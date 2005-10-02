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

RageSound_Generic_Software::sound::sound()
{
	snd = NULL;
	state = STOPPED;
	available = true;
	paused = false;
}

void RageSound_Generic_Software::sound::Allocate( int frames )
{
	/* Reserve enough blocks in the buffer to hold the buffer.  Add one, to account for
	 * the fact that we may have a partial block due to a previous Mix() call. */
	const int frames_per_block = samples_per_block / channels;
	const int blocks_to_prebuffer = frames / frames_per_block;
	buffer.reserve( blocks_to_prebuffer + 1 );
}

void RageSound_Generic_Software::sound::Deallocate()
{
	buffer.reserve( 0 );
}

int RageSound_Generic_Software::DecodeThread_start( void *p )
{
	((RageSound_Generic_Software *) p)->DecodeThread();
	return 0;
}

static int g_TotalAhead = 0;
static int g_TotalAheadCount = 0;

void RageSound_Generic_Software::Mix( int16_t *buf, int frames, int64_t frameno, int64_t current_frameno )
{
	ASSERT_M( m_DecodeThread.IsCreated(), "RageSound_Generic_Software::StartDecodeThread() was never called" );

	if( frameno - current_frameno + frames > 0 )
	{
		g_TotalAhead += (int) (frameno - current_frameno + frames);
		++g_TotalAheadCount;
	}

	static RageSoundMixBuffer mix;

	for( unsigned i = 0; i < ARRAYSIZE(sounds); ++i )
	{
		/* s.snd can not safely be accessed from here. */
		sound &s = sounds[i];
		if( s.state == sound::HALTING )
		{
			/* This indicates that this stream can be reused. */
			s.Deallocate();
			s.state = sound::STOPPED;
			s.available = true; /* do this last */
			s.paused = false;

//			LOG->Trace("set %p from HALTING to STOPPED", sounds[i].snd);
			continue;
		}

		if( s.state != sound::STOPPING && s.state != sound::PLAYING )
			continue;

		/* STOPPING or PLAYING.  Read sound data. */
		if( sounds[i].paused )
			continue;

		int got_frames = 0;
		int frames_left = frames;

		/* Does the sound have a start time? */
		if( !s.start_time.IsZero() && current_frameno != -1 )
		{
			/* If the sound is supposed to start at a time past this buffer, insert silence. */
			const int64_t iFramesUntilThisBuffer = frameno - current_frameno;
			const float fSecondsBeforeStart = -s.start_time.Ago();
			const int64_t iFramesBeforeStart = int64_t(fSecondsBeforeStart * GetSampleRate(0));
			const int iSilentFramesInThisBuffer = clamp( int(iFramesBeforeStart-iFramesUntilThisBuffer), 0, frames_left );

			got_frames += iSilentFramesInThisBuffer;
			frames_left -= iSilentFramesInThisBuffer;

			/* If we didn't completely fill the buffer, then we've written all of the silence. */
			if( frames_left )
				s.start_time.SetZero();
		}

		/* Fill actual data. */
		sound_block *p[2];
		unsigned pSize[2];
		s.buffer.get_read_pointers( p, pSize );

		while( frames_left && pSize[0] )
		{
			if( !p[0]->frames_in_buffer )
			{
				/* We've processed all of the sound in this block.  Mark it read. */
				s.buffer.advance_read_pointer( 1 );
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
			const int frames_to_read = min( frames_left, p[0]->frames_in_buffer );
			mix.SetVolume( s.volume );
			mix.SetWriteOffset( got_frames*channels );
			mix.write( p[0]->p, frames_to_read * channels );

			SOUNDMAN->CommitPlayingPosition( s.sound_id, frameno+got_frames, p[0]->position, frames_to_read );

			p[0]->p += frames_to_read*channels;
			p[0]->frames_in_buffer -= frames_to_read;
			p[0]->position += frames_to_read;

//			LOG->Trace( "incr fr rd += %i (state %i) (%p)",
//				(int) frames_to_read, s.state, s.snd );

			got_frames += frames_to_read;
			frames_left -= frames_to_read;
		}

		/* If we don't have enough to fill the buffer, we've underrun. */
		if( got_frames < frames && s.state == sound::PLAYING )
			++underruns;
	}

	memset( buf, 0, frames*bytes_per_frame );
	mix.read( buf );
}


void RageSound_Generic_Software::DecodeThread()
{
	/* SOUNDMAN will be set once RageSoundManager's ctor returns and
	 * assigns it; we might get here before that happens, though. */
	while( !SOUNDMAN && !shutdown_decode_thread )
		usleep( 10000 );

	SetupDecodingThread();

	while( !shutdown_decode_thread )
	{
		/* Fill each playing sound, round-robin. */
		usleep( 1000000*chunksize() / GetSampleRate(0) );

		LockMut( m_Mutex );
//		LOG->Trace("begin mix");

		for( unsigned i = 0; i < ARRAYSIZE(sounds); ++i )
		{
			/* The volume can change while the sound is playing; update it. */
			if( sounds[i].state == sound::PLAYING || sounds[i].state == sound::STOPPING )
				sounds[i].volume = sounds[i].snd->GetAbsoluteVolume();
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
		for( unsigned i = 0; i < ARRAYSIZE(sounds); ++i )
		{
			if( sounds[i].state != sound::PLAYING )
				continue;

			sound *pSound = &sounds[i];

			CHECKPOINT;
			int frames_filled = 0;
			while( pSound->buffer.num_writable() )
			{
				/* If there are more than min_fill_frames available, check for
				 * rate clamping. */
				if( pSound->buffer.num_readable()*samples_per_block >= unsigned(min_fill_frames) )
				{
					/* Don't write more than two chunks worth of data in one
					 * iteration.  Since we delay for one chunk period per loop,
					 * this means we'll fill at no more than 4x realtime. */
					if( frames_filled >= chunksize()*4 )
						break;
				}

				int wrote = GetDataForSound( *pSound );
				if( !wrote )
				{
					/* This sound is finishing. */
					pSound->state = sound::STOPPING;
					break;
//					LOG->Trace("mixer: (#%i) eof (%p)", i, pSound->snd );
				}

				frames_filled += wrote;
			}
		}
//		LOG->Trace("end mix");
	}
}

/* Buffer a block of sound data for the given sound.  Return the number of
 * frames buffered.  If end of file is reached, return 0. */
int RageSound_Generic_Software::GetDataForSound( sound &s )
{
	sound_block *p[2];
	unsigned psize[2];
	s.buffer.get_write_pointers( p, psize );

	/* If we have no open buffer slot, we have a buffer overflow. */
	ASSERT( psize[0] > 0 );

	sound_block *b = p[0];
	int size = ARRAYSIZE(b->buf)/channels;
	bool eof = !s.snd->GetDataToPlay( b->buf, size, b->position, b->frames_in_buffer );
	b->p = b->buf;

	s.buffer.advance_write_pointer( 1 );

//	LOG->Trace( "incr fr wr %i (state %i) (%p)",
//		(int) b->frames_in_buffer, s.state, s.snd );

	return eof? 0:size;
}


void RageSound_Generic_Software::Update(float delta)
{
	/* We must not lock here, since the decoder thread might hold the lock for a
	 * while at a time.  This is threadsafe, because once a sound is in STOPPING,
	 * this is the only place it'll be changed (to STOPPED). */
	for( unsigned i = 0; i < ARRAYSIZE(sounds); ++i )
	{
		if( sounds[i].state != sound::STOPPING )
			continue;

		if( sounds[i].buffer.num_readable() != 0 )
			continue;

//		LOG->Trace("finishing sound %i", i);

		sounds[i].snd->SoundIsFinishedPlaying();
		sounds[i].snd = NULL;

		/* This sound is done.  Set it to HALTING, since the mixer thread might
		 * be accessing it; it'll change it back to STOPPED once it's ready to
		 * be used again. */
		sounds[i].state = sound::HALTING;
//		LOG->Trace("set (#%i) %p from STOPPING to HALTING", i, sounds[i].snd);
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

void RageSound_Generic_Software::StartMixing( RageSoundBase *snd )
{
	/* Lock available sounds[], and reserve a slot. */
	m_SoundListMutex.Lock();

	unsigned i;
	for( i = 0; i < ARRAYSIZE(sounds); ++i )
		if( sounds[i].available )
			break;
	if( i == ARRAYSIZE(sounds) )
	{
		m_SoundListMutex.Unlock();
		return;
	}

	sound &s = sounds[i];
	s.available = false;

	/* We've reserved our slot; we can safely unlock now.  Don't hold onto it longer
	 * than needed, since prebuffering might take some time. */
	m_SoundListMutex.Unlock();

	s.snd = snd;
	s.start_time = snd->GetStartTime();
	s.sound_id = snd->GetID();
	s.volume = snd->GetAbsoluteVolume();
	s.buffer.clear();

	/* Initialize the sound buffer. */
	int BufferSize = frames_to_buffer;

	/* If a sound is streaming from disk, use a bigger buffer, so we don't underrun
	 * the BGM if a drive seek takes too long.  Note that in this case, we'll still
	 * underrun any other sounds that are playing, even if they're preloaded.  This
	 * is less of a problem, since the music position isn't based on those.  It could
	 * be fixed by having two decoding threads; one for streaming sounds and one for
	 * non-streaming sounds. */
	if( s.snd->IsStreamingFromDisk() )
		BufferSize = max( BufferSize, min_streaming_buffer_size );
	s.Allocate( BufferSize );

//	LOG->Trace("StartMixing(%s) (%p)", s.snd->GetLoadedFilePath().c_str(), s.snd );

	/* Prebuffer some frames before changing the sound to PLAYING. */
	bool ReachedEOF = false;
	int frames_filled = 0;
	while( !ReachedEOF && frames_filled < min_fill_frames && frames_filled < BufferSize )
	{
//		LOG->Trace("StartMixing: (#%i) buffering %i (%i writable) (%p)", i, (int) frames_to_buffer, s.buffer.num_writable(), s.snd );
		int wrote = GetDataForSound( s );
		frames_filled += wrote;
		if( !wrote )
		{
//		LOG->Trace("StartMixing: XXX hit EOF (%p)", s.snd );
			ReachedEOF = true;
		}
	}

	/* If we hit EOF already, while prebuffering, then go right to STOPPING. */
	s.state = ReachedEOF? sound::STOPPING: sound::PLAYING;

//	LOG->Trace("StartMixing: (#%i) finished prebuffering(%s) (%p)", i, s.snd->GetLoadedFilePath().c_str(), s.snd );
}

void RageSound_Generic_Software::StopMixing( RageSoundBase *snd )
{
	/* Lock, to make sure the decoder thread isn't running on this sound while we do this. */
	LockMut( m_Mutex );

	/* Find the sound. */
	unsigned i;
	for( i = 0; i < ARRAYSIZE(sounds); ++i )
		if( !sounds[i].available && sounds[i].snd == snd )
			break;
	if( i == ARRAYSIZE(sounds) )
	{
		LOG->Trace( "not stopping a sound because it's not playing" );
		return;
	}

	/* If we're already in STOPPED, there's nothing to do. */
	if( sounds[i].state == sound::STOPPED )
	{
		LOG->Trace( "not stopping a sound because it's already in STOPPED" );
		return;
	}

//	LOG->Trace("StopMixing: set %p (%s) to HALTING", sounds[i].snd, sounds[i].snd->GetLoadedFilePath().c_str());

	/* Tell the mixing thread to flush the buffer.  We don't have to worry about
	 * the decoding thread, since we've locked m_Mutex. */
	sounds[i].state = sound::HALTING;

	/* Invalidate the snd pointer to guarantee we don't make any further references to
	 * it.  Once this call returns, the sound may no longer exist. */
	sounds[i].snd = NULL;
//	LOG->Trace("end StopMixing");
}


bool RageSound_Generic_Software::PauseMixing( RageSoundBase *snd, bool bStop )
{
	LockMut( m_Mutex );

	/* Find the sound. */
	unsigned i;
	for( i = 0; i < ARRAYSIZE(sounds); ++i )
		if( !sounds[i].available && sounds[i].snd == snd )
			break;

	/* A sound can be paused in PLAYING or STOPPING.  (STOPPING means the sound
	 * has been decoded to the end, and we're waiting for that data to finish, so
	 * externally it looks and acts like PLAYING.) */
	if( i == ARRAYSIZE(sounds) ||
		(sounds[i].state != sound::PLAYING && sounds[i].state != sound::STOPPING) )
	{
		LOG->Trace( "not pausing a sound because it's not playing" );
		return false;
	}

	sounds[i].paused = bStop;

	return true;
}

void RageSound_Generic_Software::StartDecodeThread()
{
	ASSERT( !m_DecodeThread.IsCreated() );

	m_DecodeThread.Create( DecodeThread_start, this );
}

void RageSound_Generic_Software::SetDecodeBufferSize( int frames )
{
	ASSERT( !m_DecodeThread.IsCreated() );

	frames_to_buffer = frames;
}

RageSound_Generic_Software::RageSound_Generic_Software():
	m_Mutex("RageSound_Generic_Software"),
	m_SoundListMutex("SoundListMutex")
{
	shutdown_decode_thread = false;
	SetDecodeBufferSize( 4096 );
	
	m_DecodeThread.SetName("Decode thread");
}

RageSound_Generic_Software::~RageSound_Generic_Software()
{
	/* Signal the decoding thread to quit. */
	if( m_DecodeThread.IsCreated() )
	{
		shutdown_decode_thread = true;
		LOG->Trace("Shutting down decode thread ...");
		LOG->Flush();
		m_DecodeThread.Wait();
		LOG->Trace("Decode thread shut down.");
		LOG->Flush();

		LOG->Info( "Mixing %f ahead in %i Mix() calls",
			float(g_TotalAhead) / max( g_TotalAheadCount, 1 ), g_TotalAheadCount );
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
