#include "global.h"
#include "RageSoundDriver_Null.h"

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


int RageSound_Null::MixerThread_start(void *p)
{
	((RageSound_Null *) p)->MixerThread();
	return 0;
}

void RageSound_Null::MixerThread()
{
	/* SOUNDMAN will be set once RageSoundManager's ctor returns and
	 * assigns it; we might get here before that happens, though. */
	while(!SOUNDMAN && !shutdown) SDL_Delay(10);

	RageTimer Speaker;

	while(!shutdown) {
		float delay_ms = float(buffersize_frames) / samplerate;
		SDL_Delay(int(delay_ms / 2));

		LockMutex L(SOUNDMAN->lock);

		/* "Play" frames. */
		const float ms_passed = Speaker.GetDeltaTime();
		const int samples_played = int(samplerate * ms_passed);
		for(unsigned i = 0; i < sounds.size(); ++i)
		{
			sounds[i]->samples_buffered -= samples_played;
			sounds[i]->samples_buffered = max(sounds[i]->samples_buffered, 0);
		}

		GetData();
	}
}

bool RageSound_Null::GetData()
{
	LockMutex L(SOUNDMAN->lock);

	/* Create a 32-bit buffer to mix sounds. */
	static Sint16 *buf = NULL;
	int bufsize = buffersize_frames * channels;
	if(!buf)
		buf = new Sint16[bufsize];

	for(unsigned i = 0; i < sounds.size(); ++i)
	{
		if(sounds[i]->stopping)
			continue;

		unsigned samples_needed = buffersize_frames - sounds[i]->samples_buffered;
		unsigned bytes_needed = samples_needed*samplesize;
		int Play_Time = int(RageTimer::GetTimeSinceStart() * samplerate);
		Play_Time += sounds[i]->samples_buffered;

		/* Call the callback. */
		unsigned got = sounds[i]->snd->GetPCM((char *) buf, bytes_needed, Play_Time);
		sounds[i]->samples_buffered += got/samplesize;

		if(got < bytes_needed)
		{
			/* This sound is finishing. */
			sounds[i]->stopping = true;
		}
	}

	// and, do nothing! it's silence

	return true;
}

void RageSound_Null::StartMixing(RageSound *snd)
{
	sound *s = new sound;
	s->snd = snd;

	SDL_LockAudio();
	sounds.push_back(s);
	SDL_UnlockAudio();
}

void RageSound_Null::Update(float delta)
{
	LockMutex L(SOUNDMAN->lock);

	/* SoundStopped might erase sounds out from under us, so make a copy
	 * of the sound list. */
	vector<sound *> snds = sounds;
	for(unsigned i = 0; i < snds.size(); ++i)
	{
		if(!sounds[i]->stopping) continue;

		if(sounds[i]->samples_buffered)
			continue; /* stopping but still flushing */

		/* This sound is done. */
		snds[i]->snd->StopPlaying();
	}
}

void RageSound_Null::StopMixing(RageSound *snd)
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

int RageSound_Null::GetPosition(const RageSound *snd) const
{
	return int(RageTimer::GetTimeSinceStart() * samplerate);
}

RageSound_Null::RageSound_Null()
{
	shutdown = false;

	MixerThreadPtr = SDL_CreateThread(MixerThread_start, this);
}

RageSound_Null::~RageSound_Null()
{
	/* Signal the mixing thread to quit. */
	shutdown = true;
	LOG->Trace("Shutting down mixer thread ...");
	SDL_WaitThread(MixerThreadPtr, NULL);
	LOG->Trace("Mixer thread shut down.");

}

float RageSound_Null::GetPlayLatency() const
{
	return 0;  /* silence is fast! */
}

/*
 * Copyright (c) 2002 by the person(s) listed below.  All rights reserved.
 *
 * Glenn Maynard (RageSoundDriver_WaveOut)
 *
 * 2003-02  RageSoundDriver_Null created from RageSoundDriver_WaveOut
 *                                                       Aaron VonderHaar
 */
