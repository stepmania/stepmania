#include "global.h"
#include "RageSoundDriver_ALSA9.h"

#include "RageTimer.h"
#include "RageLog.h"
#include "RageSound.h"
#include "RageUtil.h"

#include "SDL_utils.h"

const int channels = 2;
const int samplesize = channels*2;              /* 16-bit */
const int samplerate = 44100;
const int buffersize_frames = 1024*8;   /* in frames */
const int buffersize = buffersize_frames * samplesize; /* in bytes */

/**
 * int err; must be defined before using this macro
 */
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

	while(!shutdown) {
		GetData();
		//SDL_Delay(10);
	}
}

bool RageSound_ALSA9::GetData()
{
	LockMutex L(SOUNDMAN->lock);

        int err;

        /* Create a 32-bit buffer to mix sounds. */
        static Sint16 *buf = NULL, *buf2 = NULL;
	int bufsize = buffersize_frames * channels;

	if (!buf)
	{
		buf = new Sint16[bufsize];
		buf2 = new Sint16[bufsize];
	}
        memset(buf, 0, bufsize*sizeof(Uint16));
        memset(buf2, 0, bufsize*sizeof(Uint16));
        SoundMixBuffer mix;


        for(unsigned i = 0; i < sounds.size(); ++i)
        {
                if(sounds[i]->stopping)
                        continue;

                /* Call the callback. */
                unsigned got = sounds[i]->snd->GetPCM((char *) buf, bufsize, last_cursor_pos);  

                mix.write((Sint16 *) buf, got/2);

                if(got < bufsize)
                {
                        /* This sound is finishing. */
                        sounds[i]->stopping = true;
                        sounds[i]->flush_pos = last_cursor_pos + (got / samplesize);
                }
        }

	mix.read((Sint16*)buf2);
        int r;
	Sint16 *start = buf2;

	/* it doesn't make sense to me why we must divide by two
	 * but we must */
	snd_pcm_uframes_t remaining = buffersize_frames/2;
        while (remaining > 0)
        {
		r = snd_pcm_mmap_writei(pcm, start, remaining);
                if (r == -EAGAIN) {
			continue;
		}
                if (r <= 0)
                {
                        LOG->Trace("RageSoundDriver_ALSA9::GetData: snd_pcm_mmap_writei: %s", snd_strerror(r));
			Recover(r);
			break;
                }

		start += r*channels;
		remaining -= r;
		last_cursor_pos += r;

        }

        return true;
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

	SDL_LockAudio();
	sounds.push_back(s);
	SDL_UnlockAudio();
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
	/* what's below tries to use the status time of the pcm device
	 * but since we use last_cursor_pos for the GetPCM() call, 
	 * it doesn't match up with what RageSound is expecting
	 */

        LockMutex L(SOUNDMAN->lock);

        int err;
        snd_pcm_status_t *status;
        //snd_timestamp_t now;
        snd_pcm_sframes_t delay;

        snd_pcm_status_alloca(&status);
        err = snd_pcm_status(pcm, status);
	ALSA_ASSERT("snd_pcm_status");

	snd_pcm_state_t state = snd_pcm_status_get_state(status);
	if (state == SND_PCM_STATE_PREPARED) return 0;

        //snd_pcm_status_get_tstamp(status, &now);   

        delay = snd_pcm_status_get_delay(status);

	//LOG->Trace("RageSound_ALSA9::GetPosition %d", last_cursor_pos - delay);
	return last_cursor_pos - delay;

	/* we need startup_time else we will be returning
	 * samples-since-1970, which will surely overflow int
	 */
	//float this_sec = (now.tv_sec - startup_time.tv_sec)
	//	+ ((float)now.tv_usec)/1000000;

	//LOG->Trace("RageSound_ALSA9::GetPosition %d", (int)(this_sec * samplerate)+800000);
        
	/* the +800000 was a fudge to compensate for the difference 
	 * between this and last_cursor_pos */
        //return (int)(this_sec * samplerate)+800000;


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
        ALSA_ASSERT("snd_pcm_open");

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


	gettimeofday(&startup_time, NULL);
	/* add error checking before Jan 2038 */

	MixerThreadPtr = SDL_CreateThread(MixerThread_start, this);
}

RageSound_ALSA9::~RageSound_ALSA9()
{
	/* Signal the mixing thread to quit. */
	shutdown = true;
	LOG->Trace("Shutting down mixer thread ...");
	SDL_WaitThread(MixerThreadPtr, NULL);
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


	/*XXX I'm assuming GetPlayLatency() is supposed to return seconds */
	return ((float)delay)/samplerate;
}

/*
 * Copyright (c) 2002 by the person(s) listed below.  All rights reserved.
 *
 * Glenn Maynard (RageSoundDriver_WaveOut)
 */
