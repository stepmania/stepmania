#include "../../stdafx.h"
#include "RageSoundDriver_DSound_Software.h"
#include "DSoundHelpers.h"

#include "../../RageTimer.h"
#include "../../RageLog.h"
#include "../../RageSound.h"
#include "../../RageUtil.h"
#include "../../tls.h"

#include "SDL.h"

/* samples */
const int channels = 2;
const int samplesize = channels*2; /* 16-bit */
const int samplerate = 44100;
const int buffersize_frames = 1024*4;	/* in frames */
const int buffersize = buffersize_frames * samplesize; /* in bytes */

/* We'll fill the buffer in chunks this big.  This should evenly divide the
 * buffer size. */
const int num_chunks = 8;
const int chunksize_frames = buffersize_frames / num_chunks;
const int chunksize = buffersize / num_chunks;

int RageSound_DSound_Software::MixerThread_start(void *p)
{
	((RageSound_DSound_Software *) p)->MixerThread();
	return 0;
}

void RageSound_DSound_Software::MixerThread()
{
	InitThreadData("Mixer thread");
	VDCHECKPOINT;

	/* SOUNDMAN will be set once RageSoundManager's ctor returns and
	 * assigns it; we might get here before that happens, though. */
	while(!SOUNDMAN && !shutdown) Sleep(10);

	if(!SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_ABOVE_NORMAL))
		LOG->Warn(werr_ssprintf(GetLastError(), "Failed to set sound thread priority"));

	while(!shutdown) {
		Sleep(10);
		while(GetData())
			;
	}
}

bool RageSound_DSound_Software::GetData()
{
	LockMut(SOUNDMAN->lock);

	char *locked_buf;
	unsigned len;
	int play_pos;

	if(!str_ds->get_output_buf(&locked_buf, &len, &play_pos, chunksize))
		return false;

	/* Silence the buffer. */
	memset(locked_buf, 0, len);

	static Sint16 *buf = NULL;
	int bufsize = buffersize_frames * channels;
	if(!buf)
	{
		buf = new Sint16[bufsize];
	}
	memset(buf, 0, bufsize*sizeof(Uint16));

	SoundMixBuffer mix;

	for(unsigned i = 0; i < sounds.size(); ++i)
	{
		if(sounds[i]->stopping)
			continue;

		/* Call the callback. */
		unsigned got = sounds[i]->snd->GetPCM((char *) buf, len, play_pos);

		mix.write((Sint16 *) buf, got/2);

		if(got < len)
		{
			/* This sound is finishing. */
			sounds[i]->stopping = true;
			sounds[i]->flush_pos = str_ds->GetMaxPosition();
		}
	}

	mix.read((Sint16 *) locked_buf);

	str_ds->release_output_buf(locked_buf, len);

	return true;
}


void RageSound_DSound_Software::StartMixing(RageSound *snd)
{
	sound *s = new sound;
	s->snd = snd;

	SDL_LockAudio();
	sounds.push_back(s);
	SDL_UnlockAudio();
}

void RageSound_DSound_Software::Update(float delta)
{
	LockMut(SOUNDMAN->lock);

	/* SoundStopped might erase sounds out from under us, so make a copy
	 * of the sound list. */
	vector<sound *> snds = sounds;
	for(unsigned i = 0; i < snds.size(); ++i)
	{
		if(!snds[i]->stopping)  continue;

		if(GetPosition(snds[i]->snd) < snds[i]->flush_pos)
			continue; /* stopping but still flushing */

		/* This sound is done. */
		snds[i]->snd->StopPlaying();
	}
}

void RageSound_DSound_Software::StopMixing(RageSound *snd)
{
	LockMut(SOUNDMAN->lock);

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

	/* If nothing is playing, reset the sample count; this is just to
     * prevent eventual overflow. */
	if(sounds.empty())
		str_ds->Reset();
}

int RageSound_DSound_Software::GetPosition(const RageSound *snd) const
{
	LockMut(SOUNDMAN->lock);
	return str_ds->GetPosition();
}

RageSound_DSound_Software::RageSound_DSound_Software()
{
	shutdown = false;

	/* If we're emulated, we're better off with the WaveOut driver; DS
	 * emulation tends to be desynced. */
	if(ds.IsEmulated())
		RageException::ThrowNonfatal("Driver unusable (emulated device)");

	/* Create a DirectSound stream, but don't force it into hardware. */
	str_ds = new DSoundBuf(ds, 
		DSoundBuf::HW_DONT_CARE, 
		channels, samplerate, 16, buffersize);
	MixerThreadPtr = SDL_CreateThread(MixerThread_start, this);

	str_ds->Play();
}

RageSound_DSound_Software::~RageSound_DSound_Software()
{
	/* Signal the mixing thread to quit. */
	shutdown = true;
	LOG->Trace("Shutting down mixer thread %p ...", MixerThreadPtr);
	LOG->Flush();
	SDL_WaitThread(MixerThreadPtr, NULL);
	LOG->Trace("Mixer thread shut down.");
	LOG->Flush();

	delete str_ds;
}

float RageSound_DSound_Software::GetPlayLatency() const
{
	return (1.0f / samplerate) * buffersize_frames;
}

/*
 * Copyright (c) 2002 by the person(s) listed below.  All rights reserved.
 *
 * Glenn Maynard
 */
