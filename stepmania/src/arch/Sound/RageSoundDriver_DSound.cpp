/* D3D implementation that uses multiple streams.
 * 
 * Each sound gets its own stream, which allows for very little startup
 * latency for each sound and lower CPU usage. */

#include "../../stdafx.h"
#include "RageSoundDriver_DSound.h"
#include "DSoundHelpers.h"

#include "../../RageSoundManager.h"
#include "../../RageException.h"
#include "../../RageUtil.h"
#include "../../RageSound.h"
#include "../../RageLog.h"
#include "../../tls.h"
#include "SDL.h"

#include "../../RageTimer.h"

RageSoundManager *SOUNDMAN = NULL;

#pragma comment(lib, "dsound.lib")
#pragma comment(lib, "dxguid.lib")

const int channels = 2;
const int samplesize = 2 * channels; /* 16-bit */
const int samplerate = 44100;
const int buffersize_frames = 4096;	/* in frames */
const int buffersize = buffersize_frames * samplesize; /* in bytes */

int RageSound_DSound::MixerThread_start(void *p)
{
	((RageSound_DSound *) p)->MixerThread();
	return 0;
}

void RageSound_DSound::MixerThread()
{
	InitThreadData("Mixer thread");
	VDCHECKPOINT;

	/* SOUNDMAN will be set once RageSoundManager's ctor returns and
	 * assigns it; we might get here before that happens, though. */
	while(!SOUNDMAN && !shutdown) Sleep(10);

	while(!shutdown) {
		VDCHECKPOINT;
		Sleep(10);

		VDCHECKPOINT;
		LockMutex L(SOUNDMAN->lock);
 		for(unsigned i = 0; i < stream_pool.size(); ++i)
		{
			if(stream_pool[i]->state == stream_pool[i]->INACTIVE)
				continue; /* inactive */

			stream_pool[i]->GetPCM(false);
		}
	}
}

void RageSound_DSound::Update(float delta)
{
	/* SoundStopped might erase sounds out from under us, so make a copy
	 * of the sound list. */
	vector<stream *> str = stream_pool;
	LockMutex L(SOUNDMAN->lock);

	for(unsigned i = 0; i < str.size(); ++i)
	{
		if(str[i]->state != str[i]->STOPPING) 
			continue;
		if(str[i]->flush_bufs)
			continue; /* stopping but still flushing */

		/* The sound has stopped and flushed all of its buffers. */
		if(str[i]->snd != NULL)
			str[i]->snd->SoundStopped();
		str[i]->snd = NULL;

		str[i]->str_ds->Stop();
		str[i]->str_ds->SetCurrentPosition(0);
		str[i]->LastPosition = -1;
		str[i]->state = str[i]->INACTIVE;
	}
}

/* If init is true, we're filling the buffer while it's stopped, so put
 * data in the current buffer (where the play cursor is); otherwise put
 * it in the opposite buffer. */
void RageSound_DSound::stream::GetPCM(bool init)
{
	VDCHECKPOINT;
	DWORD cursor, junk;

	HRESULT result;

	if(init) {
		cursor = 0;
		last_cursor_pos = 0;
	} else {
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

		/* Increment last_cursor_pos to point at where the data we're about to
		 * ask for will actually be played. */
		last_cursor_pos += buffersize_frames;
	}

	last_cursor_filled = cursor;

	if(state == STOPPING)
	{
		if(!flush_bufs)
			return; /* sound is finished and will be cleaned up in the main thread */

		flush_bufs--;
	}

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

	/* It might be INACTIVE, when we're prebuffering. We just don't want to
	 * fill anything in STOPPING; in that case, we just clear the audio buffer. */
	if(state != STOPPING)
		CallAudioCallback(locked_buf, len, last_cursor_pos, this);
	else
		/* Silence the buffer. */
		memset(locked_buf, 0, len);

	str_ds->Unlock(locked_buf, len, NULL, 0);
}

void RageSound_DSound::stream::CallAudioCallback(
               char *buf, unsigned long bytes,
               int outTime, stream *str )
{
	unsigned got = str->snd->GetPCM(buf, bytes, outTime);

	if(got < bytes) {
		/* Fill the remainder of the buffer with silence. */
		memset(buf+got, 0, bytes-got);

		/* STOPPING tells the mixer thread to release the stream once str->flush_bufs
		 * buffers have been flushed. */
		str->state = str->STOPPING;

		/* Flush two buffers worth of data. */
		str->flush_bufs = 2;
	}
}

RageSound_DSound::stream::~stream()
{
}

RageSound_DSound::RageSound_DSound()
{
	shutdown = false;

	/* Fire up DSound. */
	int hr;
	if(FAILED(hr=DirectSoundCreate8(NULL, &ds8, NULL)))
		throw RageException(hr_ssprintf(hr, "DirectSoundCreate8"));

	/* Try to set primary mixing privileges */
	hr = ds8->SetCooperativeLevel(GetDesktopWindow(), DSSCL_PRIORITY);
	{
		/* Don't bother wasting time trying to create buffers if we're
		 * emulated.  This also gives us better diagnostic information. */
		DSCAPS Caps;
		Caps.dwSize = sizeof(Caps);
		HRESULT hr;
		if(FAILED(hr = ds8->GetCaps(&Caps)))
			LOG->Warn(hr_ssprintf(hr, "ds8->GetCaps failed"));
		else if(Caps.dwFlags & DSCAPS_EMULDRIVER)
		{
			ds8->Release();
			throw "Driver unusable (emulated device)";
		}
	}

	/* Create a bunch of streams and put them into the stream pool. */
	for(int i = 0; i < 32; ++i) {
		IDirectSoundBuffer8 *newbuf;
		try {
			newbuf = CreateBuf(ds8, channels, samplerate, 16, buffersize*2, true);
		} catch(const char *e) {
			/* If we didn't get at least 8, fail. */
			if(i >= 8) break; /* OK */

			/* Clean up; the dtor won't be called. */
			for(int n = 0; n < i; ++n)
				delete stream_pool[n];
			ds8->Release();
			
			if(i)
			{
				/* We created at least one hardware buffer. */
				LOG->Trace("Could only create %i buffers; need at least 8 (failed with %s).  DirectSound driver can't be used.", i, e);
				throw "Driver unusable (not enough hardware buffers)";
			}
			throw "Driver unusable (no hardware buffers)";
		}

		stream *s = new stream;
		s->str_ds = newbuf;
		stream_pool.push_back(s);
	}
	LOG->Trace("Got %i hardware buffers", stream_pool.size());

	MixerThreadPtr = SDL_CreateThread(MixerThread_start, this);
}

RageSound_DSound::~RageSound_DSound()
{
	/* Signal the mixing thread to quit. */
	shutdown = true;
	SDL_WaitThread(MixerThreadPtr, NULL);

	ds8->Release();

	for(unsigned i = 0; i < stream_pool.size(); ++i)
		delete stream_pool[i];
}

void RageSound_DSound::StartMixing(RageSound *snd)
{
	LockMutex L(SOUNDMAN->lock);

	/* Find an unused buffer. */
	unsigned i;
	for(i = 0; i < stream_pool.size(); ++i) {
		if(stream_pool[i]->state == stream_pool[i]->INACTIVE)
			break;
	}

	if(i == stream_pool.size()) {
		/* We don't have a free sound buffer. XXX fake it */
		SOUNDMAN->AddFakeSound(snd);
		return;
	}

	/* Give the stream to the playing sound and remove it from the pool. */
	stream_pool[i]->snd = snd;

	/* Pre-buffer the stream. */
	/* There are two buffers of data; fill them both ahead of time so the
	 * sound can start almost immediately. */
	stream_pool[i]->GetPCM(true); /* first call = true */
	stream_pool[i]->GetPCM(false);
	HRESULT hr = stream_pool[i]->str_ds->Play(0, 0, DSBPLAY_LOOPING);
	if ( hr != DS_OK ) {
		return;
	}

	/* Normally, at this point we should still be INACTIVE, in which case,
	 * tell the mixer thread to start mixing this channel.  However, if it's
	 * been changed to STOPPING, then we actually finished the whole file
	 * in the prebuffering GetPCM calls above, so leave it alone and let it
	 * finish on its own. */
	if(stream_pool[i]->state == stream_pool[i]->INACTIVE)
		stream_pool[i]->state = stream_pool[i]->PLAYING;

	LOG->Trace("new sound assigned to channel %i", i);
}

/* Called by a RageSound; asks us to stop mixing them.  When this
 * call completes, snd->GetPCM (which runs in a separate thread) will
 * not be running and will not be called unless StartMixing is called
 * again. */
void RageSound_DSound::StopMixing(RageSound *snd)
{
	ASSERT(snd != NULL);
	LockMutex L(SOUNDMAN->lock);

	unsigned i;
	for(i = 0; i < stream_pool.size(); ++i)
		if(stream_pool[i]->snd == snd) break;

	if(i == stream_pool.size()) {
		LOG->Trace("not stopping a sound because it's not playing");
		return;
	}

	/* STOPPING tells the mixer thread to release the stream once str->flush_bufs
	 * buffers have been flushed. */
	stream_pool[i]->state = stream_pool[i]->STOPPING;

	/* Flush two buffers worth of data. */
	stream_pool[i]->flush_bufs = 2;

	/* This function is called externally (by RageSound) to stop immediately.
	 * We need to prevent SoundStopped from being called; it should only be
	 * called when we stop implicitely at the end of a sound.  Set snd to NULL. */
	stream_pool[i]->snd = NULL;
}

int RageSound_DSound::GetPosition(const RageSound *snd) const
{
	LockMutex L(SOUNDMAN->lock);

	unsigned i;
	for(i = 0; i < stream_pool.size(); ++i)
		if(stream_pool[i]->snd == snd) break;

	if(i == stream_pool.size())
		throw RageException("GetPosition: Sound %s is not being played", snd->GetLoadedFilePath());

	ASSERT(i != stream_pool.size());

	DWORD cursor, junk;
	stream_pool[i]->str_ds->GetCurrentPosition(&cursor, &junk);
	int last_fill = stream_pool[i]->last_cursor_filled;

	if(last_fill == 0)
	{
		if(int(cursor) < last_fill + buffersize)
			cursor += buffersize*2;
		last_fill += buffersize*2;
	}

	int last_pos = stream_pool[i]->last_cursor_pos;
	int ret = (int(cursor) - last_fill)/samplesize +  /* bytes -> samples */
		last_pos;

	/* Failsafe: never return a value smaller than we've already returned.
	 * This can happen once in a while in underrun conditions. */
	ret = max(stream_pool[i]->LastPosition, ret);
	stream_pool[i]->LastPosition = ret;

	return ret;
}

/*
 * Copyright (c) 2002 by the person(s) listed below.  All rights reserved.
 *
 * Glenn Maynard
 */
