#include "global.h"
#include "RageSoundDriver_ALSA9.h"

#include "RageTimer.h"
#include "RageLog.h"
#include "RageSound.h"
#include "RageSoundManager.h"
#include "RageUtil.h"

const int channels = 2;
const int samplerate = 44100;

const int samples_per_frame = channels;
const int bytes_per_frame = sizeof(Sint16) * samples_per_frame;

const unsigned max_writeahead = 1024*8;

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
		GetData();
		const float delay_ms = 1000 * float(max_writeahead) / samplerate;
		SDL_Delay( int(delay_ms) / 2);
	}
}

/* Returns the number of frames processed */
void RageSound_ALSA9::GetData()
{
	const int frames_to_fill = pcm->GetNumFramesToFill();
	if( frames_to_fill <= 0 )
		return;

	/* Sint16 represents a single sample
	 * each frame contains one sample per channel
	 */
    static Sint16 *buf = NULL;
	if (!buf)
		buf = new Sint16[max_writeahead*samples_per_frame*4];

    static SoundMixBuffer mix;
	mix.SetVolume( SOUNDMAN->GetMixVolume() );

	LockMutex L(SOUNDMAN->lock);
	for(unsigned i = 0; i < sounds.size(); ++i)
	{
		if(sounds[i]->stopping)
			continue;

		/* Call the callback.
		 * Get the units straight,
		 * <bytes> = GetPCM(<bytes*>, <bytes>, <frames>)
		 */
		unsigned got = sounds[i]->snd->GetPCM( (char *) buf, frames_to_fill*bytes_per_frame, pcm->GetPlayPos() ) / sizeof(Sint16);
		mix.write((Sint16 *) buf, got);

		if( int(got) < frames_to_fill )
		{
			/* This sound is finishing. */
			sounds[i]->stopping = true;
		}
    }
	L.Unlock();

    memset( buf, 0, sizeof(Sint16) * frames_to_fill * samples_per_frame );
	mix.read( buf );

	pcm->Write( buf, frames_to_fill );
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
	return pcm->GetPosition();
}       


RageSound_ALSA9::RageSound_ALSA9()
{
	shutdown = false;

	pcm = new Alsa9Buf( Alsa9Buf::HW_DONT_CARE, channels, samplerate, max_writeahead );
	
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
 
	delete pcm;
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
