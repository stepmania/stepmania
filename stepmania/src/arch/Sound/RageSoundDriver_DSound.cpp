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

const int channels = 2;
const int bytes_per_frame = 2 * channels; /* 16-bit */
const int samplerate = 44100;

/* The total write-ahead.  Don't make this *too* high; we fill the entire
 * buffer when we start playing, so it can cause frame skips.  This should be
 * high enough that sound cards won't skip. */
const int buffersize = 1024*8;	/* in frames */

const int num_chunks = 8;
const int chunksize = buffersize / num_chunks;

int RageSound_DSound::MixerThread_start(void *p)
{
	((RageSound_DSound *) p)->MixerThread();
	return 0;
}

void RageSound_DSound::MixerThread()
{
	if( !SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_ABOVE_NORMAL) )
		LOG->Warn(werr_ssprintf(GetLastError(), "Failed to set sound thread priority"));

	while( !shutdown )
	{
		CHECKPOINT;

		/* Sleep for the size of one chunk. */
		const int chunksize_frames = buffersize / num_chunks;
		float sleep_secs = (float(chunksize_frames) / samplerate);
		Sleep(int(1000 * sleep_secs));

		CHECKPOINT;
		LockMut( m_Mutex );

 		for( unsigned i = 0; i < stream_pool.size(); ++i )
		{
			/* We're only interested in PLAYING and FLUSHING sounds. */
			while( stream_pool[i]->state == stream::PLAYING ||
				   stream_pool[i]->state == stream::FLUSHING )
			{
				/* Don't mix paused sounds. */
				if( stream_pool[i]->bPaused )
					break;

				bool bEOF;
				if( !stream_pool[i]->GetData( false, bEOF ) )
					break;

				if( bEOF )
				{
					/* FLUSHING tells the mixer thread to release the stream once str->flush_bufs
					 * buffers have been flushed. */
					stream_pool[i]->state = stream_pool[i]->FLUSHING;

					/* Keep playing until the data we currently have locked has played. */
					stream_pool[i]->flush_pos = stream_pool[i]->pcm->GetOutputPosition();
				}
			}
		}

		/* When sounds are in FLUSHING, and we've finished flushing, stop the sound
		 * and move the sound to FINISHED.  Once we do this, it's owned by the main
		 * thread and we can't touch it anymore. */
		for( unsigned i = 0; i < stream_pool.size(); ++i )
		{
			if( stream_pool[i]->state != stream_pool[i]->FLUSHING )
				continue;

			const int64_t ps = stream_pool[i]->pcm->GetPosition();
			if( ps < stream_pool[i]->flush_pos )
				continue; /* still flushing */

			stream_pool[i]->bPaused = false;
			stream_pool[i]->pcm->Stop();

			stream_pool[i]->state = stream::FINISHED;
		}
	}

	/* I'm not sure why, but if we don't stop streams now, then the thread will take
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

	for(unsigned i = 0; i < str.size(); ++i)
	{
		if( str[i]->state != stream::FINISHED )
			continue;

		/* The sound has stopped and flushed all of its buffers. */
		str[i]->snd->SoundIsFinishedPlaying();
		str[i]->snd = NULL;

		/* Once we do this, the sound is once available for use; we must lock
		 * m_InactiveSoundMutex to take it out of INACTIVE again. */
		str[i]->state = str[i]->INACTIVE;
	}
}

/* If init is true, we're filling the buffer while it's stopped, so fill the
 * entire buffer.  If false, fill only one chunk.  If EOF is reached, set bEOF. */
bool RageSound_DSound::stream::GetData( bool init, bool &bEOF )
{
	CHECKPOINT;

	bEOF = false;

	char *locked_buf;
	unsigned len;
	const int64_t play_pos = pcm->GetOutputPosition();
	const int64_t cur_play_pos = pcm->GetPosition();

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
	 * fill anything in FLUSHING; in that case, we just clear the audio buffer. */
	if(state != FLUSHING)
	{
		pcm->SetVolume( snd->GetAbsoluteVolume() );

		int bytes_read = 0;
		int bytes_left = len;

		/* Does the sound have a start time? */
		if( !start_time.IsZero() )
		{
			/* If the sound is supposed to start at a time past this buffer, insert silence. */
			const int64_t iFramesUntilThisBuffer = play_pos - cur_play_pos;
			const float fSecondsBeforeStart = -start_time.Ago();
			const int64_t iFramesBeforeStart = int64_t(fSecondsBeforeStart * pcm->GetSampleRate());
			const int64_t iSilentFramesInThisBuffer = iFramesBeforeStart-iFramesUntilThisBuffer;
			const int iSilentBytesInThisBuffer = clamp( int(iSilentFramesInThisBuffer * bytes_per_frame), 0, bytes_left );

			memset( locked_buf, 0, iSilentBytesInThisBuffer );
			bytes_read += iSilentBytesInThisBuffer;
			bytes_left -= iSilentBytesInThisBuffer;

			/* If we didn't completely fill the buffer, then we've written all of the silence. */
			if( bytes_left )
				start_time.SetZero();
		}

		int got = snd->GetPCM( locked_buf+bytes_read, len-bytes_read, play_pos + (bytes_read/bytes_per_frame));
		bytes_read += got;

		if( bytes_read < (int) len )
		{
			/* Fill the remainder of the buffer with silence. */
			memset( locked_buf+got, 0, len-bytes_read );

			bEOF = true;
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

RageSound_DSound::RageSound_DSound():
	m_Mutex("DSoundMutex"),
	m_InactiveSoundMutex("InactiveSoundMutex")
{
	shutdown = false;
}

RString RageSound_DSound::Init()
{
	RString sError = ds.Init();
	if( sError != "" )
		return sError;

	/* Don't bother wasting time trying to create buffers if we're
	 * emulated.  This also gives us better diagnostic information. */
	if( ds.IsEmulated() )
		return "Driver unusable (emulated device)";

	/* Create a bunch of streams and put them into the stream pool. */
	for( int i = 0; i < 32; ++i )
	{
		DSoundBuf *newbuf = new DSoundBuf;
		RString sError = newbuf->Init( ds, DSoundBuf::HW_HARDWARE,
			channels, DSoundBuf::DYNAMIC_SAMPLERATE, 16, buffersize );
		if( sError != "" )
		{
			/* The channel failed to create.  We may be out of hardware streams, or we
			 * may not have hardware mixing at all.  If we didn't get at least 8, fail. */
			delete newbuf;

			if( i >= 8 )
				break; /* OK */

			if( i )
			{
				/* We created at least one hardware buffer. */
				LOG->Trace( "Could only create %i buffers; need at least 8 (failed with %s).  Hardware DirectSound driver can't be used.", i, sError.c_str() );
				return "not enough hardware buffers available";
			}

			return "no hardware buffers available";
		}

		stream *s = new stream;
		s->pcm = newbuf;
		stream_pool.push_back(s);
	}

	LOG->Trace( "Got %i hardware buffers", stream_pool.size() );

	MixingThread.SetName("Mixer thread");
	MixingThread.Create( MixerThread_start, this );

	return RString();
}

RageSound_DSound::~RageSound_DSound()
{
	/* Signal the mixing thread to quit. */
	if( MixingThread.IsCreated() )
	{
		shutdown = true;
		LOG->Trace("Shutting down mixer thread ...");
		LOG->Flush();
		MixingThread.Wait();
		LOG->Trace("Mixer thread shut down.");
		LOG->Flush();
	}

	for(unsigned i = 0; i < stream_pool.size(); ++i)
		delete stream_pool[i];
}

void RageSound_DSound::StartMixing( RageSoundBase *snd )
{
	/* Lock INACTIVE sounds[], and reserve a slot. */
	m_InactiveSoundMutex.Lock();

	/* Find an unused buffer. */
	unsigned i;
	for( i = 0; i < stream_pool.size(); ++i )
		if( stream_pool[i]->state == stream::INACTIVE )
			break;

	if( i == stream_pool.size() )
	{
		/* We don't have a free sound buffer. */
		m_InactiveSoundMutex.Unlock();
		return;
	}

	/* Place the sound in SETUP, where nobody else will touch it, until we put it
	 * in FLUSHING or PLAYING below. */
	stream_pool[i]->state = stream::SETUP;
	m_InactiveSoundMutex.Unlock();

	/* Give the stream to the playing sound and remove it from the pool. */
	stream_pool[i]->snd = snd;
	stream_pool[i]->pcm->SetSampleRate(snd->GetSampleRate());
	stream_pool[i]->start_time = snd->GetStartTime();

	/* Pre-buffer the stream, and start it immediately. */
	bool bEOF;
	stream_pool[i]->GetData( true, bEOF );
	stream_pool[i]->pcm->Play();

	/* If bEOF is true, we actually finished the whole file in the prebuffering
	 * GetData call above, and the sound should go straight to FLUSHING.  Otherwise,
	 * set PLAYING. */
	if( bEOF )
	{
		stream_pool[i]->state = stream_pool[i]->FLUSHING;
		stream_pool[i]->flush_pos = stream_pool[i]->pcm->GetOutputPosition();
	}
	else
		stream_pool[i]->state = stream_pool[i]->PLAYING;

//	LOG->Trace("new sound assigned to channel %i", i);
}

/* Called by a RageSound; asks us to stop mixing them.  When this
 * call completes, snd->GetPCM (which runs in a separate thread) will
 * not be running and will not be called unless StartMixing is called
 * again. */
void RageSound_DSound::StopMixing( RageSoundBase *snd )
{
	/* Lock, to make sure the decoder thread isn't running on this sound while we do this. */
	LockMut( m_Mutex );

	ASSERT(snd != NULL);

	unsigned i;
	for(i = 0; i < stream_pool.size(); ++i)
		if(stream_pool[i]->snd == snd) break;

	if(i == stream_pool.size()) {
		LOG->Trace("not stopping a sound because it's not playing");
		return;
	}

	/* We need to make sure the sound stream is actually stopped before we return;
	 * don't wait for the buffered frames to finish.  This is because exiting a
	 * thread before stopping its sounds causes glitches or worse.  Other drivers
	 * don't need to guarantee this; this is just a peculiarity of DirectSound. */
	stream_pool[i]->pcm->Stop();
	stream_pool[i]->state = stream_pool[i]->INACTIVE;
}

bool RageSound_DSound::PauseMixing( RageSoundBase *snd, bool bStop )
{
	unsigned i;
	for( i = 0; i < stream_pool.size(); ++i )
		if( stream_pool[i]->snd == snd )
			break;

	/* A sound can be paused in PLAYING or FLUSHING.  (FLUSHING means the sound
	 * has been decoded to the end, and we're waiting for that data to finish, so
	 * externally it looks and acts like PLAYING.) */
	if( i == stream_pool.size() ||
		(stream_pool[i]->state != stream::PLAYING && stream_pool[i]->state != stream::FLUSHING) )
	{
		LOG->Trace( "not pausing a sound because it's not playing" );
		return false;
	}

	stream_pool[i]->bPaused = bStop;

	return true;
}

int64_t RageSound_DSound::GetPosition( const RageSoundBase *snd ) const
{
	unsigned i;
	for(i = 0; i < stream_pool.size(); ++i)
		if(stream_pool[i]->snd == snd) break;

	if(i == stream_pool.size())
		RageException::Throw("GetPosition: Sound %s is not being played", snd->GetLoadedFilePath().c_str());

	ASSERT(i != stream_pool.size());

	/* XXX: This isn't quite threadsafe: GetPosition uses two variables, and we might
	 * be caught in the middle.  We don't want to lock, though ... */
	return stream_pool[i]->pcm->GetPosition();
}

/*
 * (c) 2002-2004 Glenn Maynard
 * All rights reserved.
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, and/or sell copies of the Software, and to permit persons to
 * whom the Software is furnished to do so, provided that the above
 * copyright notice(s) and this permission notice appear in all copies of
 * the Software and that both the above copyright notice(s) and this
 * permission notice appear in supporting documentation.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT OF
 * THIRD PARTY RIGHTS. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR HOLDERS
 * INCLUDED IN THIS NOTICE BE LIABLE FOR ANY CLAIM, OR ANY SPECIAL INDIRECT
 * OR CONSEQUENTIAL DAMAGES, OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS
 * OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
 * OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */
