#include "global.h"
#include "RageSoundDriver_Null.h"

#include "RageTimer.h"
#include "RageLog.h"
#include "RageSound.h"
#include "RageUtil.h"

#include "SDL.h"

const int channels = 2;
const int samplesize = channels*2;              /* 16-bit */
const int samplerate = 44100;
const int buffersize_frames = 1024*8;   /* in frames */
const int buffersize = buffersize_frames * samplesize; /* in bytes */

const int num_chunks = 8;
const int chunksize_frames = buffersize_frames / num_chunks;
const int chunksize = buffersize / num_chunks;


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

	/* not sure if a nansleep is needed, but certainly helps
	 * when there is LOG->Trace in GetData() */
	struct timespec delay, remaining;
	delay.tv_sec = 0;
	delay.tv_nsec = 1000;

	while(!shutdown) {
		GetData();
		nanosleep(&delay, &remaining);
	}
}

bool RageSound_Null::GetData()
{
	LockMutex L(SOUNDMAN->lock);

	/* Create a 32-bit buffer to mix sounds. */
	static Sint16 *buf = NULL;
	int bufsize = chunksize_frames * channels;
	if(!buf)
		buf = new Sint16[bufsize];

	for(unsigned i = 0; i < sounds.size(); ++i)
	{
		if(sounds[i]->stopping)
			continue;

		/* Call the callback. */
		unsigned got = sounds[i]->snd->GetPCM((char *) buf, chunksize, last_cursor_pos);

		if(got < chunksize)
		{
			/* This sound is finishing. */
			sounds[i]->stopping = true;
			sounds[i]->flush_pos = last_cursor_pos + (got / samplesize);
		}
	}

	// and, do nothing! it's silence

	//XXX to make timing more accurate, nanosleep here
	//    as if playing the chunk

	/* Increment last_cursor_pos. */
	last_cursor_pos += chunksize_frames;

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

		if(GetPosition(snds[i]->snd) < sounds[i]->flush_pos)
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
	LockMutex L(SOUNDMAN->lock);

	return time(NULL)-startup_time;
}

RageSound_Null::RageSound_Null()
{
	shutdown = false;
	last_cursor_pos = 0;
	startup_time = time(NULL);

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
