#include "global.h"
#include "RageSoundDriver_ALSA9.h"

#include "RageTimer.h"
#include "RageLog.h"
#include "RageSound.h"
#include "RageSoundManager.h"
#include "RageUtil.h"

#include "SDL_utils.h"

const int channels = 2;
const int samplerate = 44100;
const int bytes_per_sample = 2;  /* 16-bit */

const int samples_per_frame = channels;
const int bytes_per_frame = bytes_per_sample * samples_per_frame;

const int buffersize_frames = 1024*2;
const int buffersize_samples = buffersize_frames * samples_per_frame;
const int buffersize_bytes = buffersize_frames * bytes_per_frame;


/* int err; must be defined before using this macro */
#define ALSA_ASSERT(x) \
        if (err < 0) \
        { \
                LOG->Trace("RageSound_ALSA9: ASSERT %s: %s", \
                        x, snd_strerror(err)); \
        }



int RageSound_ALSA9::MixerThread_start(void *p)
{
	((RageSound_ALSA9 *) p)->MixerThread();
	return 0;
}

void RageSound_ALSA9::MixerThread()
{
	/* SOUNDMAN will be set once RageSoundManager's ctor returns and
	 * assigns it; we might get here before that happens, though. */
	while(!SOUNDMAN && !shutdown) SDL_Delay(10);

	RageTimer Speaker;

	float delay_ms, passed_ms;
	unsigned int frames_read;

	while(!shutdown) {
		frames_read = GetData();
		delay_ms = 1000 * float(frames_read) / samplerate;

		passed_ms = Speaker.GetDeltaTime();

		//LOG->Trace("@@@@@ DELAY %f - %f = %f ms", delay_ms, passed_ms, delay_ms - passed_ms);
		
		if (delay_ms > passed_ms)
			/* If we sleep for the full delay, we will just
			 * miss the deadline, the ALSA pipe will underrun,
			 * we will have to recover, lots of bad things to
			 * make the sound start skipping.
			 * Instead, just sleep for half the time
			 * so we wake up with time to spare
			 */
			SDL_Delay(int(delay_ms-passed_ms) / 2);
		else
			LOG->Trace("RageSound_ALSA9: Missed deadline by %f ms", (delay_ms - passed_ms) );

	}
}

/* Returns the number of frames processed */
unsigned int RageSound_ALSA9::GetData()
{
	LockMutex L(SOUNDMAN->lock);

    int err;

	/* Sint16 represents a single sample
	 * each frame contains one sample per channel
	 */
    static Sint16 *buf = NULL, *buf2 = NULL;

	if (!buf)
	{
		buf = new Sint16[buffersize_samples];
		buf2 = new Sint16[buffersize_samples];
	}
    memset(buf, 0, buffersize_bytes);
    memset(buf2, 0, buffersize_bytes);
    SoundMixBuffer mix;

	for(unsigned i = 0; i < sounds.size(); ++i)
	{
		if(sounds[i]->stopping)
			continue;

		/* Call the callback.
		 * Get the units straight,
		 * <bytes> = GetPCM(<bytes*>, <bytes>, <frames>)
		 */
		unsigned got = sounds[i]->snd->GetPCM((char *) buf, buffersize_bytes, last_cursor_pos);  

		mix.write((Sint16 *) buf, got/bytes_per_sample);

		if(got < buffersize_bytes)
		{
			/* This sound is finishing. */
			sounds[i]->stopping = true;
		}
    }

	mix.read((Sint16*)buf2);
	int r;
	Sint16 *start = buf2;
	int new_cursor = last_cursor_pos;

	snd_pcm_uframes_t remaining = buffersize_frames;
	while (remaining > 0)
	{
		r = snd_pcm_mmap_writei(pcm, start, remaining);
		if (r == -EAGAIN)
			continue;

		if (r <= 0)
		{
			LOG->Trace("RageSoundDriver_ALSA9::GetData: snd_pcm_mmap_writei: %s", snd_strerror(r));
			Recover(r);
			break;
		}

		start += r*samples_per_frame;
		remaining -= r;
		new_cursor += r;
	}

	last_cursor_pos = new_cursor;
	//last_cursor_pos += buffersize_frames;

	return buffersize_frames;
}

/**
 * When the play buffer underruns, subsequent writes to the buffer
 * return -EPIPE.  When this happens, call Recover() to restart
 * playback.
 */
void RageSound_ALSA9::Recover(int r)
{
	int err;

	if (r == -EPIPE)
	{
		LOG->Trace("RageSound_ALSA9::Recover (prepare)");
		err = snd_pcm_prepare(pcm);
		ALSA_ASSERT("snd_pcm_prepare (Recover)");
	}
	else /* r == -ESTRPIPE */
	{
		LOG->Trace("RageSound_ALSA9::Recover (resume)");
		while ((err = snd_pcm_resume(pcm)) == -EAGAIN)
			SDL_Delay(10);

		ALSA_ASSERT("snd_pcm_resume (Recover)");
	}

}


void RageSound_ALSA9::StartMixing(RageSound *snd)
{
	sound *s = new sound;
	s->snd = snd;

	LockMutex L(SOUNDMAN->lock);
	sounds.push_back(s);
}

void RageSound_ALSA9::Update(float delta)
{
	LockMutex L(SOUNDMAN->lock);

	/* SoundStopped might erase sounds out from under us, so make a copy
	 * of the sound list. */
	vector<sound *> snds = sounds;
	for(unsigned i = 0; i < snds.size(); ++i)
	{
		if(!sounds[i]->stopping) continue;

		if(GetPosition(snds[i]->snd) < sounds[i]->flush_pos)
			continue; /* stopping but still flushing */

		/* This sound is done. */
		snds[i]->snd->StopPlaying();
	}
}

void RageSound_ALSA9::StopMixing(RageSound *snd)
{
	LockMutex L(SOUNDMAN->lock);

	/* Find the sound. */
	unsigned i;
	for(i = 0; i < sounds.size(); ++i)
		if(sounds[i]->snd == snd) break;
	if(i == sounds.size())
	{
		LOG->Trace("not stopping a sound because it's not playing");
		return;
	}

	delete sounds[i];
	sounds.erase(sounds.begin()+i, sounds.begin()+i+1);
}


int RageSound_ALSA9::GetPosition(const RageSound *snd) const
{
	LockMutex L(SOUNDMAN->lock);

	int err;
	snd_pcm_status_t *status;
	snd_pcm_sframes_t delay;

	snd_pcm_status_alloca(&status);
	err = snd_pcm_status(pcm, status);
	
	ALSA_ASSERT("snd_pcm_status");

	snd_pcm_state_t state = snd_pcm_status_get_state(status);
	if (state == SND_PCM_STATE_PREPARED) return 0;

	/* delay is returned in frames */
	delay = snd_pcm_status_get_delay(status);

	return last_cursor_pos - delay;
}       


RageSound_ALSA9::RageSound_ALSA9()
{
	shutdown = false;
	int err;

	last_cursor_pos = 0;

	snd_pcm_hw_params_t *hwparams;
	snd_pcm_sw_params_t *swparams;

	/* open the device */
	// if we instead use plughw: then our requested format WILL be provided
	err = snd_pcm_open(&pcm, "hw:0,0", SND_PCM_STREAM_PLAYBACK, 0);  
	if (err < 0)
		RageException::ThrowNonfatal("snd_pcm_open: %s", snd_strerror(err));

	/* allocate the hardware parameters structure */
	err = snd_pcm_hw_params_malloc(&hwparams);
	ALSA_ASSERT("snd_pcm_hw_params_malloc");

	/* not exactly sure what this does */
	err = snd_pcm_hw_params_any(pcm, hwparams);
	ALSA_ASSERT("snd_pcm_hw_params_any");

	/* set to mmap mode (with channels interleaved) */
	err = snd_pcm_hw_params_set_access(pcm, hwparams, SND_PCM_ACCESS_MMAP_INTERLEAVED);
	ALSA_ASSERT("snd_pcm_hw_params_set_access");

	/* set PCM format (signed 16bit, little endian) */
	err = snd_pcm_hw_params_set_format(pcm, hwparams, SND_PCM_FORMAT_S16_LE);
	ALSA_ASSERT("snd_pcm_hw_params_set_format");

	/* set number of channels */
	err = snd_pcm_hw_params_set_channels(pcm, hwparams, 2);
	ALSA_ASSERT("snd_pcm_hw_params_set_channels");

	unsigned int rate = samplerate;
	err = snd_pcm_hw_params_set_rate_near(pcm, hwparams, &rate, 0);
	// check if we got the rate we desire

	/* write the hardware parameters to the device */
	err = snd_pcm_hw_params(pcm, hwparams);
	ALSA_ASSERT("snd_pcm_hw_params");

	snd_pcm_hw_params_free(hwparams);

	/* prepare the device to reveice data */
	err = snd_pcm_prepare(pcm);
	ALSA_ASSERT("snd_pcm_prepare");

	//XXX should RageException::ThrowNonfatal if something went wrong


	/* prepare a snd_output_t for use with LOG->Trace */
	errout = NULL;
	snd_output_buffer_open(&errout);
	snd_pcm_dump(pcm, errout);
	char *errstring;
	snd_output_buffer_string(errout, &errstring);
	LOG->Trace("%s", errstring);
	snd_output_flush(errout);

	MixingThread.SetName( "RageSound_ALSA9" );
	MixingThread.Create( MixerThread_start, this );
}

RageSound_ALSA9::~RageSound_ALSA9()
{
	/* Signal the mixing thread to quit. */
	shutdown = true;
	LOG->Trace("Shutting down mixer thread ...");
	MixingThread.Wait();
	LOG->Trace("Mixer thread shut down.");
 
	snd_pcm_close(pcm);
}

float RageSound_ALSA9::GetPlayLatency() const
{
	int err;
	snd_pcm_status_t *status;
	snd_pcm_sframes_t delay;

	snd_pcm_status_alloca(&status);
	err = snd_pcm_status(pcm, status);
	ALSA_ASSERT("snd_pcm_status");

	delay = snd_pcm_status_get_delay(status);
	/*XXX not sure if delay is the thing to use for 
	 * calculating latency
	 *
	 * delay is the number of frames between where
	 * the next write will begin and what is being
	 * played now
	 */

	/* this returns seconds */
	return ((float)delay)/samplerate;
}

/*
 * Copyright (c) 2002 by the person(s) listed below.  All rights reserved.
 *
 * Glenn Maynard (RageSoundDriver_WaveOut)
 */
