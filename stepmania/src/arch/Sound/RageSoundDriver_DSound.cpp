/* D3D implementation that uses multiple streams.
 * 
 * Each sound gets its own stream, which allows for very little startup
 * latency for each sound and lower CPU usage. */

#include "global.h"
#include "RageSoundDriver_DSound.h"
#include "DSoundHelpers.h"

#include "RageSoundManager.h"
#include "RageException.h"
#include "RageUtil.h"
#include "RageSound.h"
#include "RageLog.h"

#include "SDL.h"

const int channels = 2;
const int bytes_per_frame = 2 * channels; /* 16-bit */
const int samplerate = 44100;

/* The total write-ahead.  Don't make this *too* high; we fill the entire
 * buffer when we start playing, so it can cause frame skips.  This should be
 * high enough that sound cards won't skip. */
const int buffersize_frames = 1024*8;	/* in frames */
const int buffersize = buffersize_frames * bytes_per_frame; /* in bytes */

const int num_chunks = 8;
const int chunksize = buffersize / num_chunks;

int RageSound_DSound::MixerThread_start(void *p)
{
	((RageSound_DSound *) p)->MixerThread();
	return 0;
}

void RageSound_DSound::MixerThread()
{
	/* SOUNDMAN will be set once RageSoundManager's ctor returns and
	 * assigns it; we might get here before that happens, though. */
	while(!SOUNDMAN && !shutdown) Sleep(10);

	if(!SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_ABOVE_NORMAL))
		LOG->Warn(werr_ssprintf(GetLastError(), "Failed to set sound thread priority"));

	while(!shutdown) {
		CHECKPOINT;

		/* Sleep for the size of one chunk. */
		const int chunksize_frames = buffersize_frames / num_chunks;
		float sleep_secs = (float(chunksize_frames) / samplerate);
		Sleep(int(1000 * sleep_secs));

		CHECKPOINT;
		LockMutex L(SOUNDMAN->lock);
 		for(unsigned i = 0; i < stream_pool.size(); ++i)
		{
			if(stream_pool[i]->state == stream_pool[i]->INACTIVE)
				continue; /* inactive */

			while(stream_pool[i]->GetData(false))
				;
		}
	}

	/* I'm not sure why, but if we don't stop the stream now, then the thread will take
	 * 90ms (our buffer size) longer to close. */
	for(unsigned i = 0; i < stream_pool.size(); ++i)
		if(stream_pool[i]->state != stream_pool[i]->INACTIVE)
			stream_pool[i]->pcm->Stop();
}

void RageSound_DSound::Update(float delta)
{
	/* SoundStopped might erase sounds out from under us, so make a copy
	 * of the sound list. */
	vector<stream *> str = stream_pool;
	ASSERT(SOUNDMAN);
	LockMutex L(SOUNDMAN->lock);

	for(unsigned i = 0; i < str.size(); ++i)
	{
		if(str[i]->state != str[i]->STOPPING) continue;

		int ps = str[i]->pcm->GetPosition();
		if(ps < str[i]->flush_pos)
			continue; /* stopping but still flushing */

		/* The sound has stopped and flushed all of its buffers. */
		if(str[i]->snd != NULL)
			str[i]->snd->StopPlaying();
		str[i]->snd = NULL;

		str[i]->pcm->Stop();
		str[i]->state = str[i]->INACTIVE;
	}
}

/* If init is true, we're filling the buffer while it's stopped, so put
 * data in the current buffer (where the play cursor is); otherwise put
 * it in the opposite buffer. */
bool RageSound_DSound::stream::GetData(bool init)
{
	CHECKPOINT;

	char *locked_buf;
	unsigned len;
	const int play_pos = pcm->GetOutputPosition();
	const int cur_play_pos = pcm->GetPosition();

	if(init) {
		/* We're initializing; fill the entire buffer. The buffer is supposed to
		 * be empty, so this should never fail. */
		if(!pcm->get_output_buf(&locked_buf, &len, buffersize))
			ASSERT(0);
	} else {
		/* Just fill one chunk. */
		if(!pcm->get_output_buf(&locked_buf, &len, chunksize))
			return false;
	}

	/* It might be INACTIVE, when we're prebuffering. We just don't want to
	 * fill anything in STOPPING; in that case, we just clear the audio buffer. */
	if(state != STOPPING)
	{
		int bytes_read = 0;
		int bytes_left = len;

		/* Does the sound have a start time? */
		if( !start_time.IsZero() )
		{
			/* If the sound is supposed to start at a time past this buffer, insert silence. */
			const int iFramesUntilThisBuffer = play_pos - cur_play_pos;
			const float fSecondsBeforeStart = -start_time.Ago();
			const int iFramesBeforeStart = int(fSecondsBeforeStart * pcm->GetSampleRate());
			const int iSilentFramesInThisBuffer = iFramesBeforeStart-iFramesUntilThisBuffer;
			const int iSilentBytesInThisBuffer = clamp( iSilentFramesInThisBuffer * bytes_per_frame, 0, bytes_left );

			memset( locked_buf, 0, iSilentBytesInThisBuffer );
			bytes_read += iSilentBytesInThisBuffer;
			bytes_left -= iSilentBytesInThisBuffer;

			if( !iSilentBytesInThisBuffer )
				start_time.SetZero();
		}

		int got = snd->GetPCM( locked_buf+bytes_read, len-bytes_read, play_pos + (bytes_read/bytes_per_frame));
		bytes_read += got;

		if( bytes_read < (int) len )
		{
			/* Fill the remainder of the buffer with silence. */
			memset( locked_buf+got, 0, len-bytes_read );

			/* STOPPING tells the mixer thread to release the stream once str->flush_bufs
			 * buffers have been flushed. */
			state = STOPPING;

			/* Keep playing until the data we currently have locked has played. */
			flush_pos = pcm->GetOutputPosition();
		}
	} else {
		/* Silence the buffer. */
		memset(locked_buf, 0, len);
	}

	pcm->release_output_buf(locked_buf, len);

	return true;
}

RageSound_DSound::stream::~stream()
{
	delete pcm;
}

RageSound_DSound::RageSound_DSound()
{
	shutdown = false;

	/* Don't bother wasting time trying to create buffers if we're
	 * emulated.  This also gives us better diagnostic information. */
	if(ds.IsEmulated())
		RageException::ThrowNonfatal("Driver unusable (emulated device)");

	/* Create a bunch of streams and put them into the stream pool. */
	for(int i = 0; i < 32; ++i) {
		DSoundBuf *newbuf;
		try {
			newbuf = new DSoundBuf(ds, 
				DSoundBuf::HW_HARDWARE,
				channels, DSoundBuf::DYNAMIC_SAMPLERATE, 16, buffersize);
		} catch(const RageException &e) {
			/* If we didn't get at least 8, fail. */
			if(i >= 8) break; /* OK */

			/* Clean up; the dtor won't be called. */
			for(int n = 0; n < i; ++n)
				delete stream_pool[n];
			
			if(i)
			{
				/* We created at least one hardware buffer. */
				LOG->Trace("Could only create %i buffers; need at least 8 (failed with %s).  DirectSound driver can't be used.", i, e.what());
				RageException::ThrowNonfatal("Driver unusable (not enough hardware buffers)");
			}
			RageException::ThrowNonfatal("Driver unusable (no hardware buffers)");
		}

		stream *s = new stream;
		s->pcm = newbuf;
		stream_pool.push_back(s);
	}

	LOG->Trace("Got %i hardware buffers", stream_pool.size());

	/* Set channel volumes. */
	VolumeChanged();

	MixingThread.SetName("Mixer thread");
	MixingThread.Create( MixerThread_start, this );
}

RageSound_DSound::~RageSound_DSound()
{
	/* Signal the mixing thread to quit. */
	shutdown = true;
	LOG->Trace("Shutting down mixer thread ...");
	LOG->Flush();
	MixingThread.Wait();
	LOG->Trace("Mixer thread shut down.");
	LOG->Flush();

	for(unsigned i = 0; i < stream_pool.size(); ++i)
		delete stream_pool[i];
}

void RageSound_DSound::VolumeChanged()
{
	for(unsigned i = 0; i < stream_pool.size(); ++i)
	{
		stream_pool[i]->pcm->SetVolume(SOUNDMAN->GetMixVolume());
	}
}

void RageSound_DSound::StartMixing( RageSoundBase *snd )
{
	LockMutex L(SOUNDMAN->lock);

	/* Find an unused buffer. */
	unsigned i;
	for(i = 0; i < stream_pool.size(); ++i) {
		if(stream_pool[i]->state == stream_pool[i]->INACTIVE)
			break;
	}

	if(i == stream_pool.size()) {
		/* We don't have a free sound buffer. Fake it. */
		/* XXX: too big of a hack for too rare of a case */
		// SOUNDMAN->AddFakeSound(snd);
		return;
	}

	/* Give the stream to the playing sound and remove it from the pool. */
	stream_pool[i]->snd = snd;
	stream_pool[i]->pcm->SetSampleRate(snd->GetSampleRate());
	stream_pool[i]->start_time = snd->GetStartTime();

	/* Pre-buffer the stream. */
	/* There are two buffers of data; fill them both ahead of time so the
	 * sound can start almost immediately. */
	stream_pool[i]->GetData(true);
	stream_pool[i]->pcm->Play();

	/* Normally, at this point we should still be INACTIVE, in which case,
	 * tell the mixer thread to start mixing this channel.  However, if it's
	 * been changed to STOPPING, then we actually finished the whole file
	 * in the prebuffering GetData calls above, so leave it alone and let it
	 * finish on its own. */
	if(stream_pool[i]->state == stream_pool[i]->INACTIVE)
		stream_pool[i]->state = stream_pool[i]->PLAYING;

//	LOG->Trace("new sound assigned to channel %i", i);
}

/* Called by a RageSound; asks us to stop mixing them.  When this
 * call completes, snd->GetPCM (which runs in a separate thread) will
 * not be running and will not be called unless StartMixing is called
 * again. */
void RageSound_DSound::StopMixing( RageSoundBase *snd )
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
	stream_pool[i]->flush_pos = stream_pool[i]->pcm->GetOutputPosition();

	/* This function is called externally (by RageSound) to stop immediately.
	 * We need to prevent SoundStopped from being called; it should only be
	 * called when we stop implicitely at the end of a sound.  Set snd to NULL. */
	stream_pool[i]->snd = NULL;
}

int RageSound_DSound::GetPosition( const RageSoundBase *snd ) const
{
	LockMutex L(SOUNDMAN->lock);

	unsigned i;
	for(i = 0; i < stream_pool.size(); ++i)
		if(stream_pool[i]->snd == snd) break;

	if(i == stream_pool.size())
		RageException::Throw("GetPosition: Sound %s is not being played", snd->GetLoadedFilePath().c_str());

	ASSERT(i != stream_pool.size());

	return stream_pool[i]->pcm->GetPosition();
}

/*
 * Copyright (c) 2002-2004 by the person(s) listed below.  All rights reserved.
 *
 * Glenn Maynard
 */
