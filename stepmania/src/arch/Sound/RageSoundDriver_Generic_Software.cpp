#include "global.h"
#include "RageSoundDriver_Generic_Software.h"

#include "RageLog.h"
#include "RageSound.h"
#include "RageUtil.h"
#include "RageSoundManager.h"

static const int channels = 2;
static const int bytes_per_frame = channels*2; /* 16-bit */

static const int frames_to_buffer = 4096;

static const int num_chunks = 16;
static int chunksize() { return frames_to_buffer / num_chunks; }

RageSound_Generic_Software::sound::sound()
{
	snd = NULL;
	state = STOPPED;

	/* Reserve enough blocks in the buffer to hold the buffer, plus some extra, since blocks
	 * are partially read by Mix(). */
	const int frames_per_block = samples_per_block / channels;
	const int blocks_to_prebuffer = frames_to_buffer / frames_per_block;
	buffer.reserve( blocks_to_prebuffer + 2 );
}

int RageSound_Generic_Software::DecodeThread_start( void *p )
{
	((RageSound_Generic_Software *) p)->DecodeThread();
	return 0;
}

void RageSound_Generic_Software::Mix( int16_t *buf, int frames, int64_t frameno, int64_t current_frameno )
{
	static SoundMixBuffer mix;

	CHECKPOINT;
	for( unsigned i = 0; i < ARRAYSIZE(sounds); ++i )
	{
		/* s.snd can not safely be accessed from here. */
		sound &s = sounds[i];
		if( s.state == sound::HALTING )
		{
			/* The main thread is waiting for us. */
			s.buffer.clear();
			s.state = sound::STOPPED;
			continue;
		}

		if( s.state == sound::STOPPED )
			continue;

		/* STOPPING or PLAYING.  Read sound data. */
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
			mix.write( p[0]->p, frames_to_read * channels, s.volume, got_frames*channels );

			SOUNDMAN->CommitPlayingPosition( s.sound_id, frameno+got_frames, p[0]->position, frames_to_read );

			p[0]->p += frames_to_read*channels;
			p[0]->frames_in_buffer -= frames_to_read;
			p[0]->position += frames_to_read;
			s.frames_read += frames_to_read;
			got_frames += frames_to_read;
			frames_left -= frames_to_read;
		}

		/* If we don't have enough to fill the buffer, we've underrun. */
//		if( got_frames < frames && s.state == sound::PLAYING )
//			s.underruns++
	}

	memset( buf, 0, frames*bytes_per_frame );
	mix.read( buf );
}


void RageSound_Generic_Software::DecodeThread()
{
	/* SOUNDMAN will be set once RageSoundManager's ctor returns and
	 * assigns it; we might get here before that happens, though. */
	while( !SOUNDMAN && !shutdown_decode_thread )
		SDL_Delay( 10 );

	SetupDecodingThread();

	while( !shutdown_decode_thread )
	{
		/* Fill each playing sound, round-robin. */
		SDL_Delay( 1000*chunksize() / GetSampleRate(0) );

		LockMut(SOUNDMAN->lock);
		unsigned i;
		/* The volume can change while the sound is playing; update it. */
		for( i = 0; i < ARRAYSIZE(sounds); ++i )
			if( sounds[i].state == sound::PLAYING || sounds[i].state == sound::STOPPING )
				sounds[i].volume = sounds[i].snd->GetVolume();

		/* Fill PLAYING sounds, prioritizing sounds that have less sound buffered. */
		while( 1 )
		{
			sound *pSound = NULL;
			int64_t frames_buffered = 0;
			for( i = 0; i < ARRAYSIZE(sounds); ++i )
			{
				if( sounds[i].state != sound::PLAYING )
					continue;
				if( pSound == NULL || sounds[i].frames_buffered() < frames_buffered )
				{
					frames_buffered = sounds[i].frames_buffered();
					pSound = &sounds[i];
				}
			}

			if( pSound == NULL )
				break;

			if( frames_buffered >= frames_to_buffer )
				break;

			CHECKPOINT;
			if( !GetDataForSound( *pSound ) )
			{
				/* This sound is finishing. */
				pSound->state = sound::STOPPING;
			}
		}
	}
}

/* Buffer a block of sound data for the given sound.  If end of file is reached,
 * return false. */
bool RageSound_Generic_Software::GetDataForSound( sound &s )
{
	sound_block *p[2];
	unsigned psize[2];
	s.buffer.get_write_pointers( p, psize );

	/* If we have no open buffer slot, we have a buffer overflow. */
	RAGE_ASSERT_M( psize[0] > 0, ssprintf("%i", s.frames_buffered()) );

	sound_block *b = p[0];
	bool eof = !s.snd->GetDataToPlay( b->buf, ARRAYSIZE(b->buf), b->position, b->frames_in_buffer );
	b->p = b->buf;

	s.buffer.advance_write_pointer( 1 );

	s.frames_written += b->frames_in_buffer;

	return !eof;
}


void RageSound_Generic_Software::Update(float delta)
{
	ASSERT(SOUNDMAN);
	LockMut(SOUNDMAN->lock);

	for( unsigned i = 0; i < ARRAYSIZE(sounds); ++i )
	{
		if( sounds[i].state != sound::STOPPING )
			continue;

		if( sounds[i].buffer.num_readable() != 0 )
			continue;

		LOG->Trace("finishing sound %i", i);

		/* This sound is done. */
		sounds[i].snd->StopPlaying();
	}
}

void RageSound_Generic_Software::StartMixing( RageSoundBase *snd )
{
	unsigned i;
	for( i = 0; i < ARRAYSIZE(sounds); ++i )
		if( sounds[i].state == sound::STOPPED )
			break;
	if( i == ARRAYSIZE(sounds) )
		return;

	sound &s = sounds[i];

	s.snd = snd;
	s.start_time = snd->GetStartTime();
	s.frames_read = s.frames_written = 0;
	s.sound_id = snd->GetID();
	s.volume = snd->GetVolume();

	/* Prebuffer some frames before changing the sound to PLAYING. */
	bool ReachedEOF = false;
	while( !ReachedEOF && s.frames_buffered() < frames_to_buffer )
	{
		if( !GetDataForSound( s ) )
			ReachedEOF = true;
	}

	/* If we hit EOF already, while prebuffering, then go right to STOPPING. */
	s.state = ReachedEOF? sound::STOPPING: sound::PLAYING;

	LOG->Trace("finished prebuffering");
}

void RageSound_Generic_Software::StopMixing( RageSoundBase *snd )
{
	/* Lock, to make sure the decoder thread isn't running on this sound while we do this. */
	LockMut(SOUNDMAN->lock);

	/* Find the sound. */
	unsigned i;
	for( i = 0; i < ARRAYSIZE(sounds); ++i )
		if( sounds[i].snd == snd )
			break;
	if( i == ARRAYSIZE(sounds) )
	{
		LOG->Trace( "not stopping a sound because it's not playing" );
		return;
	}

	/* If we're already in STOPPED or HALTING, there's nothing to do. */
	if( sounds[i].state == sound::STOPPED )
	{
		LOG->Trace( "not stopping a sound because it's in STOPPED or HALTING" );
		return;
	}

	if( sounds[i].state == sound::STOPPING && sounds[i].buffer.num_readable() == 0 )
	{
		/* The sound is finished and already flushed. */
		sounds[i].state = sound::STOPPED;
	}
	else
	{
		/* Tell the mixing thread to flush the buffer. */
		sounds[i].state = sound::HALTING;
	}

	/* Invalidate the snd pointer to guarantee we don't make any further references to
	 * it.  Once this call returns, the sound may no longer exist. */
	sounds[i].snd = NULL;
}


RageSound_Generic_Software::RageSound_Generic_Software()
{
	shutdown_decode_thread = false;

	m_DecodeThread.SetName("Decode thread");
	m_DecodeThread.Create( DecodeThread_start, this );
}

RageSound_Generic_Software::~RageSound_Generic_Software()
{
	/* Signal the mixing thread to quit. */
	shutdown_decode_thread = true;
	LOG->Trace("Shutting down decode thread ...");
	LOG->Flush();
	m_DecodeThread.Wait();
	LOG->Trace("Decode thread shut down.");
	LOG->Flush();
}

/*
 * Copyright (c) 2002-2004 by the person(s) listed below.  All rights reserved.
 *
 * Glenn Maynard
 */
