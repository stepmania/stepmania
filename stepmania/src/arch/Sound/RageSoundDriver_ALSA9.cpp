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

const int samples_per_frame = channels;
const int bytes_per_frame = sizeof(Sint16) * samples_per_frame;

const unsigned max_writeahead = 1024*8;


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

	while(!shutdown)
	{
		unsigned int frames_read = GetData();
		const float delay_ms = 1000 * float(max_writeahead) / samplerate;
		SDL_Delay( int(delay_ms) / 2);
	}
}

/* Returns the number of frames processed */
int RageSound_ALSA9::GetData()
{
	LockMutex L(SOUNDMAN->lock);

	snd_pcm_sframes_t avail_frames = snd_pcm_avail_update(pcm);
	if( avail_frames < 0 && Recover(avail_frames) )
		avail_frames = snd_pcm_avail_update(pcm);

	if( avail_frames < 0 )
	{
		LOG->Trace( "RageSoundDriver_ALSA9::GetData: snd_pcm_avail_update: %s", snd_strerror(avail_frames) );
		return 0;
	}

	const snd_pcm_sframes_t filled_frames = total_frames - avail_frames;
	const snd_pcm_sframes_t frames_to_fill = (max_writeahead - filled_frames) & ~3;
	ASSERT( frames_to_fill >= 0 );
	ASSERT( frames_to_fill <= (int)max_writeahead );

//	LOG->Trace("%li avail, %li filled, %li to", avail_frames, filled_frames, frames_to_fill);
	if( frames_to_fill <= 0 )
		return 0;

	/* Sint16 represents a single sample
	 * each frame contains one sample per channel
	 */
    static Sint16 *buf = NULL;
	if (!buf)
		buf = new Sint16[max_writeahead*samples_per_frame*4];

    static SoundMixBuffer mix;
	mix.SetVolume( SOUNDMAN->GetMixVolume() );

	for(unsigned i = 0; i < sounds.size(); ++i)
	{
		if(sounds[i]->stopping)
			continue;

		/* Call the callback.
		 * Get the units straight,
		 * <bytes> = GetPCM(<bytes*>, <bytes>, <frames>)
		 */
		unsigned got = sounds[i]->snd->GetPCM( (char *) buf, frames_to_fill*bytes_per_frame, last_cursor_pos ) / sizeof(Sint16);
		mix.write((Sint16 *) buf, got);

		if( int(got) < frames_to_fill )
		{
			/* This sound is finishing. */
			sounds[i]->stopping = true;
		}
    }

    memset( buf, 0, sizeof(Sint16) * frames_to_fill * samples_per_frame );
	mix.read((Sint16*)buf);

	/* We should be able to write it all.  If we don't, treat it as an error. */
	int wrote = snd_pcm_mmap_writei( pcm, buf, frames_to_fill );
	if( wrote < 0 && Recover(wrote) )
		wrote = snd_pcm_mmap_writei( pcm, buf, frames_to_fill );

	if( wrote < 0 )
	{
		LOG->Trace( "RageSoundDriver_ALSA9::GetData: snd_pcm_mmap_writei: %s", snd_strerror(wrote) );
		return -1;
	}

	last_cursor_pos += wrote;
	if( wrote < frames_to_fill )
		LOG->Trace("Couldn't write whole buffer? (%i < %li)\n", wrote, frames_to_fill );

	return frames_to_fill;
}

/**
 * When the play buffer underruns, subsequent writes to the buffer
 * return -EPIPE.  When this happens, call Recover() to restart
 * playback.
 */
bool RageSound_ALSA9::Recover(int r)
{
	if( r == -EPIPE )
	{
		LOG->Trace("RageSound_ALSA9::Recover (prepare)");
		int err = snd_pcm_prepare(pcm);
		ALSA_ASSERT("snd_pcm_prepare (Recover)");
		return true;
	}

	if( r == -ESTRPIPE )
	{
		LOG->Trace("RageSound_ALSA9::Recover (resume)");
		int err;
		while ((err = snd_pcm_resume(pcm)) == -EAGAIN)
			SDL_Delay(10);

		ALSA_ASSERT("snd_pcm_resume (Recover)");
		return true;
	}

	return false;
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

	snd_pcm_status_t *status;
	snd_pcm_status_alloca(&status);

	int err = snd_pcm_status( pcm, status );
	
	ALSA_ASSERT("snd_pcm_status");

	snd_pcm_state_t state = snd_pcm_status_get_state( status );
	if ( state == SND_PCM_STATE_PREPARED )
		return 0;

	snd_pcm_hwsync( pcm );
	/* delay is returned in frames */
	snd_pcm_sframes_t delay = snd_pcm_status_get_delay(status);

	return last_cursor_pos - delay;
}       


RageSound_ALSA9::RageSound_ALSA9()
{
	shutdown = false;
	last_cursor_pos = 0;

	/* open the device */
	// if we instead use plughw: then our requested format WILL be provided
	int err;
	err = snd_pcm_open( &pcm, "hw:0,0", SND_PCM_STREAM_PLAYBACK, SND_PCM_NONBLOCK );
	if (err < 0)
		RageException::ThrowNonfatal("snd_pcm_open: %s", snd_strerror(err));

	/* allocate the hardware parameters structure */
	snd_pcm_hw_params_t *hwparams;
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
	snd_output_t *errout = NULL;
	snd_output_buffer_open(&errout);
	snd_pcm_dump(pcm, errout);
	snd_output_flush(errout);

	char *errstring;
	snd_output_buffer_string(errout, &errstring);
	LOG->Trace("%s", errstring);
	snd_output_close( errout );

	total_frames = snd_pcm_avail_update(pcm);

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
	return float(max_writeahead)/samplerate;
}

/*
 * Copyright (c) 2002 by the person(s) listed below.  All rights reserved.
 *
 * Glenn Maynard (RageSoundDriver_WaveOut)
 */
