#include "../../stdafx.h"
#include "RageSoundDriver_DSound_Software.h"
#include "DSoundHelpers.h"

/* Known problems:
 *
 * Skips between screen changes in the software mixer.  This is because we use
 * a smaller buffer.  The hardware mixer uses a larger buffer, so the CPU used
 * loading the new screen isn't a problem.  We can't just stop sounds; we want
 * music to keep playing while we're between screens.  Throw a few SDL_Sleep(0)s
 * in strategic places?
 */

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
const int buffersize_frames = 2048;	/* in frames */
const int buffersize = buffersize_frames * samplesize; /* in bytes */

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
	
	while(!shutdown) {
		Sleep(10);

		LockMutex L(SOUNDMAN->lock);
		GetPCM();
	}
}

void RageSound_DSound_Software::GetPCM()
{
	DWORD cursor, junk;

	HRESULT result;

	result = str_ds->GetCurrentPosition(&cursor, &junk);
	if ( result == DSERR_BUFFERLOST ) {
		str_ds->Restore();
		result = str_ds->GetCurrentPosition(&cursor, &junk);
	}
	if ( result != DS_OK ) {
		LOG->Warn(hr_ssprintf(result, "DirectSound::GetCurrentPosition failed"));
		return;
	}

	/* The DSound buffer is equal to two of our buffers.  Round cursor
	 * down to our buffer size. */
	cursor = (cursor / buffersize) * buffersize;

	/* Cursor points to the buffer that's currently playing.  We want to
	 * fill the buffer that *isn't* playing. */
	cursor += buffersize;
	cursor %= buffersize*2;

	/* If it hasn't changed, we have nothing to do yet. */
	if(int(cursor) == last_cursor_filled)
		return;
	last_cursor_filled = cursor;

	/* Increment last_cursor_pos to point at where the data we're about to
	 * ask for will actually be played. */
	last_cursor_pos += buffersize_frames;

	/* Lock the audio buffer. */
	DWORD len;
	char *locked_buf = NULL;
	result = str_ds->Lock(cursor, buffersize, (LPVOID *)&locked_buf, &len, NULL, &junk, 0);
	if ( result == DSERR_BUFFERLOST ) {
		str_ds->Restore();
		result = str_ds->Lock(cursor, buffersize, (LPVOID *)&locked_buf, &len, NULL, &junk, 0);
	}
	if ( result != DS_OK ) {
		LOG->Warn(hr_ssprintf(result, "Couldn't lock the DirectSound buffer."));
		return;
	}

	/* Silence the buffer. */
	memset(locked_buf, 0, len);

	/* Create a 32-bit buffer to mix sounds. */
	static Sint32 *mixbuf = NULL;
	static Sint16 *buf = NULL;
	int bufsize = buffersize_frames * channels;
	if(!buf)
	{
		buf = new Sint16[bufsize];
		mixbuf = new Sint32[bufsize];
	}
	memset(buf, 0, bufsize*sizeof(Uint16));
	memset(mixbuf, 0, bufsize*sizeof(Uint32));

	for(unsigned i = 0; i < sounds.size(); ++i)
	{
		if(sounds[i]->stopping)
		{
			if(sounds[i]->flush_bufs)
				sounds[i]->flush_bufs--;
		} else {
			/* Call the callback. */
			unsigned got = sounds[i]->snd->GetPCM((char *) buf, len, last_cursor_pos);

			SOUNDMAN->MixAudio(
				(Uint8 *) locked_buf, (Uint8 *) buf, got, SDL_MIX_MAXVOLUME/2);

			if(got < len)
			{
				/* This sound is finishing. */
				sounds[i]->stopping = true;
				sounds[i]->flush_bufs = 2;
			}
		}
	}

	str_ds->Unlock(locked_buf, len, NULL, 0);
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
	LockMutex L(SOUNDMAN->lock);

	/* SoundStopped might erase sounds out from under us, so make a copy
	 * of the sound list. */
	vector<sound *> snds = sounds;
	for(unsigned i = 0; i < snds.size(); ++i)
	{
		if(sounds[i]->stopping && !sounds[i]->flush_bufs)
		{
			/* This sound is done. */
			snds[i]->snd->SoundStopped();
		}
	}
}

void RageSound_DSound_Software::StopMixing(RageSound *snd)
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

	if(sounds.empty())
	{
		/* Nothing is playing.  Reset the sample count; this is just to
		 * prevent eventual overflow. */
		last_cursor_pos = LastPosition = 0;
	}
}

int RageSound_DSound_Software::GetPosition(const RageSound *snd) const
{
	LockMutex L(SOUNDMAN->lock);

	DWORD cursor, junk;
	str_ds->GetCurrentPosition(&cursor, &junk);
	int last_fill = last_cursor_filled;

	if(last_fill == 0)
	{
		/* Unwrap. */
		if(int(cursor) < last_fill + buffersize)
			cursor += buffersize*2;
		last_fill += buffersize*2;
	}

	int ret = (int(cursor) - last_fill)/samplesize +  /* bytes -> samples */
		last_cursor_pos;

	/* Failsafe: never return a value smaller than we've already returned.
	 * This can happen once in a while in underrun conditions. */
	ret = max(LastPosition, ret);
	LastPosition = ret;

	return ret;
}

RageSound_DSound_Software::RageSound_DSound_Software()
{
	shutdown = false;
	last_cursor_pos = last_cursor_filled = 0;
	LastPosition = -1;

	/* Fire up DSound. */
	int hr;
	if(FAILED(hr=DirectSoundCreate8(NULL, &ds8, NULL)))
		throw RageException(hr_ssprintf(hr, "DirectSoundCreate8"));

	/* Try to set primary mixing privileges */
	hr = ds8->SetCooperativeLevel(GetDesktopWindow(), DSSCL_PRIORITY);

	/* Create a DirectSound stream, but don't force it into hardware. */
	str_ds = CreateBuf(ds8, channels, samplerate, 16, buffersize*2, false);

	MixerThreadPtr = SDL_CreateThread(MixerThread_start, this);

	hr = str_ds->Play(0, 0, DSBPLAY_LOOPING);
}

RageSound_DSound_Software::~RageSound_DSound_Software()
{
	/* Signal the mixing thread to quit. */
	shutdown = true;
	LOG->Trace("Shutting down mixer thread ...");
	SDL_WaitThread(MixerThreadPtr, NULL);
	LOG->Trace("Mixer thread shut down.");

	str_ds->Release();
	ds8->Release();
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
