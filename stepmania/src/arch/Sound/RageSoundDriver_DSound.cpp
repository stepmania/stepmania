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

const int channels = 2;
const int samplesize = 2 * channels; /* 16-bit */
const int samplerate = 44100;
const int buffersize_frames = 4096;	/* in frames */
const int buffersize = buffersize_frames * samplesize; /* in bytes */

const int num_chunks = 8;
const int chunksize_frames = buffersize_frames / num_chunks;
const int chunksize = buffersize / num_chunks;

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

	if(!SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_ABOVE_NORMAL))
		LOG->Warn("Failed to set sound thread priority: %i", GetLastError()); /* XXX */

	while(!shutdown) {
		VDCHECKPOINT;
		Sleep(10);

		VDCHECKPOINT;
		LockMutex L(SOUNDMAN->lock);
 		for(unsigned i = 0; i < stream_pool.size(); ++i)
		{
			if(stream_pool[i]->state == stream_pool[i]->INACTIVE)
				continue; /* inactive */

			while(stream_pool[i]->GetPCM(false))
				;
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

		if(str[i]->str_ds->GetPosition() < str[i]->flush_pos)
			continue; /* stopping but still flushing */

		/* The sound has stopped and flushed all of its buffers. */
		if(str[i]->snd != NULL)
			str[i]->snd->SoundStopped();
		str[i]->snd = NULL;

		str[i]->str_ds->Stop();
		str[i]->state = str[i]->INACTIVE;
	}
}

/* If init is true, we're filling the buffer while it's stopped, so put
 * data in the current buffer (where the play cursor is); otherwise put
 * it in the opposite buffer. */
bool RageSound_DSound::stream::GetPCM(bool init)
{
	VDCHECKPOINT;

	char *locked_buf;
	unsigned len;
	int play_pos;

	if(init) {
		/* We're initializing; fill the entire buffer. The buffer is supposed to
		 * be empty, so this should never fail. */
		if(!str_ds->get_output_buf(&locked_buf, &len, &play_pos, buffersize))
			ASSERT(0);
	} else {
		/* Just fill one chunk. */
		if(!str_ds->get_output_buf(&locked_buf, &len, &play_pos, chunksize))
			return false;
	}

	/* It might be INACTIVE, when we're prebuffering. We just don't want to
	 * fill anything in STOPPING; in that case, we just clear the audio buffer. */
	if(state != STOPPING)
	{
		unsigned got = snd->GetPCM(locked_buf, len, play_pos);

		if(got < len) {
			/* Fill the remainder of the buffer with silence. */
			memset(locked_buf+got, 0, len-got);

			/* STOPPING tells the mixer thread to release the stream once str->flush_bufs
			* buffers have been flushed. */
			state = STOPPING;

			/* Flush two buffers worth of data. */
			flush_pos = str_ds->GetMaxPosition();
		}
	} else {
		/* Silence the buffer. */
		memset(locked_buf, 0, len);
	}

	str_ds->release_output_buf(locked_buf, len);

	return true;
}

RageSound_DSound::stream::~stream()
{
	delete str_ds;
}

RageSound_DSound::RageSound_DSound()
{
	shutdown = false;

	/* Don't bother wasting time trying to create buffers if we're
	 * emulated.  This also gives us better diagnostic information. */
	if(ds.IsEmulated())
		throw "Driver unusable (emulated device)";

	/* Create a bunch of streams and put them into the stream pool. */
	for(int i = 0; i < 32; ++i) {
		DSoundBuf *newbuf;
		try {
			newbuf = new DSoundBuf(ds, 
				DSoundBuf::HW_HARDWARE,
				channels, samplerate, 16, buffersize);
		} catch(const char *e) {
			/* If we didn't get at least 8, fail. */
			if(i >= 8) break; /* OK */

			/* Clean up; the dtor won't be called. */
			for(int n = 0; n < i; ++n)
				delete stream_pool[n];
			
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
	stream_pool[i]->GetPCM(true);
	stream_pool[i]->str_ds->Play();

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
	stream_pool[i]->flush_pos = stream_pool[i]->str_ds->GetMaxPosition();

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

	return stream_pool[i]->str_ds->GetPosition();
}

/*
 * Copyright (c) 2002 by the person(s) listed below.  All rights reserved.
 *
 * Glenn Maynard
 */
