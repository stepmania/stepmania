#include "global.h"
#include "RageSoundDriver_Generic_Software.h"

#include "RageLog.h"
#include "RageSound.h"
#include "RageUtil.h"
#include "RageSoundManager.h"
#include "RageSoundMixBuffer.h"

static const int channels = 2;
static const int bytes_per_frame = channels*2; /* 16-bit */

/* When a sound has fewer than min_fill_frames buffered, buffer at maximum speed.
 * Once beyond that, fill at a limited rate. */
static const int min_fill_frames = 1024*4;
static int frames_to_buffer;
static int min_streaming_buffer_size = 1024*32;

/* 512 is about 10ms, which is big enough for the tolerance of most schedulers. */
static int chunksize() { return 512; }

static int underruns = 0, logged_underruns = 0;

RageSound_Generic_Software::Sound::Sound()
{
	m_pSound = NULL;
	m_State = AVAILABLE;
	m_bPaused = false;
}

void RageSound_Generic_Software::Sound::Allocate( int iFrames )
{
	/* Reserve enough blocks in the buffer to hold the buffer.  Add one, to account for
	 * the fact that we may have a partial block due to a previous Mix() call. */
	const int iFramesPerBlock = samples_per_block / channels;
	const int iBlocksToPrebuffer = iFrames / iFramesPerBlock;
	m_Buffer.reserve( iBlocksToPrebuffer + 1 );
}

void RageSound_Generic_Software::Sound::Deallocate()
{
	m_Buffer.reserve( 0 );
}

int RageSound_Generic_Software::DecodeThread_start( void *p )
{
	((RageSound_Generic_Software *) p)->DecodeThread();
	return 0;
}

static int g_iTotalAhead = 0;
static int g_iTotalAheadCount = 0;

RageSoundMixBuffer &RageSound_Generic_Software::MixIntoBuffer( int iFrames, int64_t iFrameNumber, int64_t iCurrentFrame )
{
	ASSERT_M( m_DecodeThread.IsCreated(), "RageSound_Generic_Software::StartDecodeThread() was never called" );

	if( iFrameNumber - iCurrentFrame + iFrames > 0 )
	{
		g_iTotalAhead += (int) (iFrameNumber - iCurrentFrame + iFrames);
		++g_iTotalAheadCount;
	}

	static RageSoundMixBuffer mix;

	for( unsigned i = 0; i < ARRAYLEN(m_Sounds); ++i )
	{
		/* s.m_pSound can not safely be accessed from here. */
		Sound &s = m_Sounds[i];
		if( s.m_State == Sound::HALTING )
		{
			/* This indicates that this stream can be reused. */
			s.m_bPaused = false;
			s.m_State = Sound::STOPPED;

//			LOG->Trace("set %p from HALTING to STOPPED", m_Sounds[i].m_pSound);
			continue;
		}

		if( s.m_State != Sound::STOPPING && s.m_State != Sound::PLAYING )
			continue;

		/* STOPPING or PLAYING.  Read sound data. */
		if( m_Sounds[i].m_bPaused )
			continue;

		int iGotFrames = 0;
		int iFramesLeft = iFrames;

		/* Does the sound have a start time? */
		if( !s.m_StartTime.IsZero() && iCurrentFrame != -1 )
		{
			/* If the sound is supposed to start at a time past this buffer, insert silence. */
			const int64_t iFramesUntilThisBuffer = iFrameNumber - iCurrentFrame;
			const float fSecondsBeforeStart = -s.m_StartTime.Ago();
			const int64_t iFramesBeforeStart = int64_t(fSecondsBeforeStart * GetSampleRate());
			const int iSilentFramesInThisBuffer = clamp( int(iFramesBeforeStart-iFramesUntilThisBuffer), 0, iFramesLeft );

			iGotFrames += iSilentFramesInThisBuffer;
			iFramesLeft -= iSilentFramesInThisBuffer;

			/* If we didn't completely fill the buffer, then we've written all of the silence. */
			if( iFramesLeft )
				s.m_StartTime.SetZero();
		}

		/* Fill actual data. */
		sound_block *p[2];
		unsigned pSize[2];
		s.m_Buffer.get_read_pointers( p, pSize );

		while( iFramesLeft && pSize[0] )
		{
			if( !p[0]->m_FramesInBuffer )
			{
				/* We've processed all of the sound in this block.  Mark it read. */
				s.m_Buffer.advance_read_pointer( 1 );
				++p[0];
				--pSize[0];

				/* If we have more data in p[0], keep going. */
				if( pSize[0] )
					continue; // more data

				/* We've used up p[0].  Try p[1]. */
				swap( p[0], p[1] );
				swap( pSize[0], pSize[1] );
				continue;
			}

			/* Note that, until we call advance_read_pointer, we can safely write to p[0]. */
			const int frames_to_read = min( iFramesLeft, p[0]->m_FramesInBuffer );
			mix.SetVolume( s.m_fVolume );
			mix.SetWriteOffset( iGotFrames*channels );
			mix.write( p[0]->m_BufferNext, frames_to_read * channels );

			SOUNDMAN->CommitPlayingPosition( s.m_iSoundID, iFrameNumber+iGotFrames, p[0]->m_iPosition, frames_to_read );

			p[0]->m_BufferNext += frames_to_read*channels;
			p[0]->m_FramesInBuffer -= frames_to_read;
			p[0]->m_iPosition += frames_to_read;

//			LOG->Trace( "incr fr rd += %i (state %i) (%p)",
//				(int) frames_to_read, s.m_State, s.m_pSound );

			iGotFrames += frames_to_read;
			iFramesLeft -= frames_to_read;
		}

		/* If we don't have enough to fill the buffer, we've underrun. */
		if( iGotFrames < iFrames && s.m_State == Sound::PLAYING )
			++underruns;
	}

	return mix;
}

void RageSound_Generic_Software::Mix( int16_t *pBuf, int iFrames, int64_t iFrameNumber, int64_t iCurrentFrame )
{
	memset( pBuf, 0, iFrames*bytes_per_frame );
	MixIntoBuffer( iFrames, iFrameNumber, iCurrentFrame ).read( pBuf );
}

void RageSound_Generic_Software::Mix( float *pBuf, int iFrames, int64_t iFrameNumber, int64_t iCurrentFrame )
{
	memset( pBuf, 0, iFrames*bytes_per_frame*2 );
	MixIntoBuffer( iFrames, iFrameNumber, iCurrentFrame ).read( pBuf );
}

void RageSound_Generic_Software::DecodeThread()
{
	/* SOUNDMAN will be set once RageSoundManager's ctor returns and
	 * assigns it; we might get here before that happens, though. */
	while( !SOUNDMAN && !m_bShutdownDecodeThread )
		usleep( 10000 );

	SetupDecodingThread();

	while( !m_bShutdownDecodeThread )
	{
		/* Fill each playing sound, round-robin. */
		{
			int iSampleRate = GetSampleRate();
			ASSERT_M( iSampleRate > 0, ssprintf("%i", iSampleRate) );
			int iUsecs = 1000000*chunksize() / iSampleRate;
			usleep( iUsecs );
		}

		LockMut( m_Mutex );
//		LOG->Trace("begin mix");

		for( unsigned i = 0; i < ARRAYLEN(m_Sounds); ++i )
		{
			/* The volume can change while the sound is playing; update it. */
			/* XXX: We can't access m_pSound when in STOPPING; it doesn't exist anymore. */
			// if( m_Sounds[i].m_State == Sound::PLAYING || m_Sounds[i].m_State == Sound::STOPPING )
			if( m_Sounds[i].m_State == Sound::PLAYING )
				m_Sounds[i].m_fVolume = m_Sounds[i].m_pSound->GetAbsoluteVolume();
		}

		/*
		 * If a buffer is low on data, keep filling until it has a reasonable amount.
		 * However, once beyond a certain threshold, clamp the rate at which we fill
		 * it.  For example, if the threshold is 4k frames, and we have a 32k frame
		 * buffer, fill the buffer as fast as we can until it reaches 4k frames; but
		 * beyond that point, only fill it at a rate relative to realtime (for example,
		 * at 2x realtime).
		 *
		 * This allows a stream to have a large buffer, for higher reliability, without
		 * causing major CPU bursts when the stream starts or underruns.  (Filling 32k
		 * takes more CPU than filling 4k frames, and may cause a gameplay skip.)
		 */
		for( unsigned i = 0; i < ARRAYLEN(m_Sounds); ++i )
		{
			if( m_Sounds[i].m_State != Sound::PLAYING )
				continue;

			Sound *pSound = &m_Sounds[i];

			CHECKPOINT;
			int iFramesFilled = 0;
			while( pSound->m_Buffer.num_writable() )
			{
				/* If there are more than min_fill_frames available, check for
				 * rate clamping. */
				if( pSound->m_Buffer.num_readable()*samples_per_block >= unsigned(min_fill_frames) )
				{
					/* Don't write more than two chunks worth of data in one
					 * iteration.  Since we delay for one chunk period per loop,
					 * this means we'll fill at no more than 4x realtime. */
					if( iFramesFilled >= chunksize()*4 )
						break;
				}

				int iWrote = GetDataForSound( *pSound );
				if( !iWrote )
				{
					/* This sound is finishing. */
					pSound->m_State = Sound::STOPPING;
					break;
//					LOG->Trace("mixer: (#%i) eof (%p)", i, pSound->m_pSound );
				}

				iFramesFilled += iWrote;
			}
		}
//		LOG->Trace("end mix");
	}
}

/* Buffer a block of sound data for the given sound.  Return the number of
 * frames buffered.  If end of file is reached, return 0. */
int RageSound_Generic_Software::GetDataForSound( Sound &s )
{
	sound_block *p[2];
	unsigned psize[2];
	s.m_Buffer.get_write_pointers( p, psize );

	/* If we have no open buffer slot, we have a buffer overflow. */
	ASSERT( psize[0] > 0 );

	sound_block *pBlock = p[0];
	int size = ARRAYLEN(pBlock->m_Buffer)/channels;
	s.m_pSound->GetDataToPlay( pBlock->m_Buffer, size, pBlock->m_iPosition, pBlock->m_FramesInBuffer );
	pBlock->m_BufferNext = pBlock->m_Buffer;

	s.m_Buffer.advance_write_pointer( 1 );

//	LOG->Trace( "incr fr wr %i (state %i) (%p)",
//		(int) pBlock->m_FramesInBuffer, s.m_State, s.m_pSound );

	return pBlock->m_FramesInBuffer;
}


void RageSound_Generic_Software::Update()
{
	/* We must not lock here, since the decoder thread might hold the lock for a
	 * while at a time.  This is threadsafe, because once a sound is in STOPPING,
	 * this is the only place it'll be changed (to STOPPED). */
	for( unsigned i = 0; i < ARRAYLEN(m_Sounds); ++i )
	{
		switch( m_Sounds[i].m_State )
		{
		case Sound::STOPPED:
			m_Sounds[i].Deallocate();
			m_Sounds[i].m_State = Sound::AVAILABLE;
			continue;
		case Sound::STOPPING:
			break;
		default:
			continue;
		}

		if( m_Sounds[i].m_Buffer.num_readable() != 0 )
			continue;

//		LOG->Trace("finishing sound %i", i);

		m_Sounds[i].m_pSound->SoundIsFinishedPlaying();
		m_Sounds[i].m_pSound = NULL;

		/* This sound is done.  Set it to HALTING, since the mixer thread might
		 * be accessing it; it'll change it back to STOPPED once it's ready to
		 * be used again. */
		m_Sounds[i].m_State = Sound::HALTING;
//		LOG->Trace("set (#%i) %p from STOPPING to HALTING", i, m_Sounds[i].m_pSound);
	}

	static float fNext = 0;
	if( RageTimer::GetTimeSinceStart() >= fNext )
	{
		/* Lockless: only Mix() can write to underruns. */
		int current_underruns = underruns;
		if( current_underruns > logged_underruns )
		{
			LOG->MapLog( "GenericMixingUnderruns", "Mixing underruns: %i", current_underruns - logged_underruns );
			LOG->Trace( "Mixing underruns: %i", current_underruns - logged_underruns );
			logged_underruns = current_underruns;

			/* Don't log again for at least a second, or we'll burst output
			 * and possibly cause more underruns. */
			fNext = RageTimer::GetTimeSinceStart() + 1;
		}
	}
}

void RageSound_Generic_Software::StartMixing( RageSoundBase *pSound )
{
	/* Lock available m_Sounds[], and reserve a slot. */
	m_SoundListMutex.Lock();

	unsigned i;
	for( i = 0; i < ARRAYLEN(m_Sounds); ++i )
		if( m_Sounds[i].m_State == Sound::AVAILABLE )
			break;
	if( i == ARRAYLEN(m_Sounds) )
	{
		m_SoundListMutex.Unlock();
		return;
	}

	Sound &s = m_Sounds[i];
	s.m_State = Sound::BUFFERING;

	/* We've reserved our slot; we can safely unlock now.  Don't hold onto it longer
	 * than needed, since prebuffering might take some time. */
	m_SoundListMutex.Unlock();

	s.m_pSound = pSound;
	s.m_StartTime = pSound->GetStartTime();
	s.m_iSoundID = pSound->GetID();
	s.m_fVolume = pSound->GetAbsoluteVolume();
	s.m_Buffer.clear();

	/* Initialize the sound buffer. */
	int BufferSize = frames_to_buffer;

	/* If a sound is streaming from disk, use a bigger buffer, so we don't underrun
	 * the BGM if a drive seek takes too long.  Note that in this case, we'll still
	 * underrun any other sounds that are playing, even if they're preloaded.  This
	 * is less of a problem, since the music position isn't based on those.  It could
	 * be fixed by having two decoding threads; one for streaming sounds and one for
	 * non-streaming sounds. */
	if( s.m_pSound->IsStreamingFromDisk() )
		BufferSize = max( BufferSize, min_streaming_buffer_size );
	s.Allocate( BufferSize );

//	LOG->Trace("StartMixing(%s) (%p)", s.m_pSound->GetLoadedFilePath().c_str(), s.m_pSound );

	/* Prebuffer some frames before changing the sound to PLAYING. */
	bool ReachedEOF = false;
	int iFramesFilled = 0;
	while( !ReachedEOF && iFramesFilled < min_fill_frames && iFramesFilled < BufferSize )
	{
//		LOG->Trace("StartMixing: (#%i) buffering %i (%i writable) (%p)", i, (int) frames_to_buffer, s.buffer.num_writable(), s.m_pSound );
		int iWrote = GetDataForSound( s );
		iFramesFilled += iWrote;
		if( !iWrote )
		{
//		LOG->Trace("StartMixing: XXX hit EOF (%p)", s.m_pSound );
			ReachedEOF = true;
		}
	}

	/* If we hit EOF already, while prebuffering, then go right to STOPPING. */
	s.m_State = ReachedEOF? Sound::STOPPING: Sound::PLAYING;

//	LOG->Trace("StartMixing: (#%i) finished prebuffering(%s) (%p)", i, s.m_pSound->GetLoadedFilePath().c_str(), s.m_pSound );
}

void RageSound_Generic_Software::StopMixing( RageSoundBase *pSound )
{
	/* Lock, to make sure the decoder thread isn't running on this sound while we do this. */
	LockMut( m_Mutex );

	/* Find the sound. */
	unsigned i;
	for( i = 0; i < ARRAYLEN(m_Sounds); ++i )
		if( m_Sounds[i].m_State != Sound::AVAILABLE && m_Sounds[i].m_pSound == pSound )
			break;
	if( i == ARRAYLEN(m_Sounds) )
	{
		LOG->Trace( "not stopping a sound because it's not playing" );
		return;
	}

	/* If we're already in STOPPED, there's nothing to do. */
	if( m_Sounds[i].m_State == Sound::STOPPED )
	{
		LOG->Trace( "not stopping a sound because it's already in STOPPED" );
		return;
	}

//	LOG->Trace("StopMixing: set %p (%s) to HALTING", m_Sounds[i].m_pSound, m_Sounds[i].m_pSound->GetLoadedFilePath().c_str());

	/* Tell the mixing thread to flush the buffer.  We don't have to worry about
	 * the decoding thread, since we've locked m_Mutex. */
	m_Sounds[i].m_State = Sound::HALTING;

	/* Invalidate the m_pSound pointer to guarantee we don't make any further references to
	 * it.  Once this call returns, the sound may no longer exist. */
	m_Sounds[i].m_pSound = NULL;
//	LOG->Trace("end StopMixing");
}


bool RageSound_Generic_Software::PauseMixing( RageSoundBase *pSound, bool bStop )
{
	LockMut( m_Mutex );

	/* Find the sound. */
	unsigned i;
	for( i = 0; i < ARRAYLEN(m_Sounds); ++i )
		if( m_Sounds[i].m_State != Sound::AVAILABLE && m_Sounds[i].m_pSound == pSound )
			break;

	/* A sound can be paused in PLAYING or STOPPING.  (STOPPING means the sound
	 * has been decoded to the end, and we're waiting for that data to finish, so
	 * externally it looks and acts like PLAYING.) */
	if( i == ARRAYLEN(m_Sounds) ||
		(m_Sounds[i].m_State != Sound::PLAYING && m_Sounds[i].m_State != Sound::STOPPING) )
	{
		LOG->Trace( "not pausing a sound because it's not playing" );
		return false;
	}

	m_Sounds[i].m_bPaused = bStop;

	return true;
}

void RageSound_Generic_Software::StartDecodeThread()
{
	ASSERT( !m_DecodeThread.IsCreated() );

	m_DecodeThread.Create( DecodeThread_start, this );
}

void RageSound_Generic_Software::SetDecodeBufferSize( int iFrames )
{
	ASSERT( !m_DecodeThread.IsCreated() );

	frames_to_buffer = iFrames;
}

RageSound_Generic_Software::RageSound_Generic_Software():
	m_Mutex("RageSound_Generic_Software"),
	m_SoundListMutex("SoundListMutex")
{
	m_bShutdownDecodeThread = false;
	SetDecodeBufferSize( 4096 );
	
	m_DecodeThread.SetName("Decode thread");
}

RageSound_Generic_Software::~RageSound_Generic_Software()
{
	/* Signal the decoding thread to quit. */
	if( m_DecodeThread.IsCreated() )
	{
		m_bShutdownDecodeThread = true;
		LOG->Trace("Shutting down decode thread ...");
		LOG->Flush();
		m_DecodeThread.Wait();
		LOG->Trace("Decode thread shut down.");
		LOG->Flush();

		LOG->Info( "Mixing %f ahead in %i Mix() calls",
			float(g_iTotalAhead) / max( g_iTotalAheadCount, 1 ), g_iTotalAheadCount );
	}
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
