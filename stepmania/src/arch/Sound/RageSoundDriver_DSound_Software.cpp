#include "global.h"
#include "RageSoundDriver_DSound_Software.h"
#include "DSoundHelpers.h"

#include "RageTimer.h"
#include "RageLog.h"
#include "RageSound.h"
#include "RageUtil.h"
#include "RageSoundManager.h"
#include "PrefsManager.h"

static const int channels = 2;
static const int bytes_per_frame = channels*2; /* 16-bit */
static const int samplerate = 44100;
static const int safe_writeahead = 1024*4; /* in frames */
static int max_writeahead;

/* We'll fill the buffer in chunks this big. */
static const int num_chunks = 8;
static int chunksize() { return max_writeahead / num_chunks; }

void RageSound_DSound_Software::MixerThread()
{
	/* SOUNDMAN will be set once RageSoundManager's ctor returns and
	 * assigns it; we might get here before that happens, though. */
	while( !SOUNDMAN && !shutdown_mixer_thread )
		SDL_Delay( 10 );

	if( !SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_TIME_CRITICAL) )
		if( !SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_ABOVE_NORMAL) )
			LOG->Warn(werr_ssprintf(GetLastError(), "Failed to set sound thread priority"));

	while( !shutdown_mixer_thread )
	{
		char *locked_buf;
		unsigned len;
		const int64_t play_pos = pcm->GetOutputPosition();

		if( !pcm->get_output_buf(&locked_buf, &len, chunksize()) )
		{
			Sleep( chunksize()*1000 / samplerate );
			continue;
		}

		this->Mix( (int16_t *) locked_buf, len/bytes_per_frame, play_pos, pcm->GetPosition() );

		pcm->release_output_buf(locked_buf, len);
	}

	/* I'm not sure why, but if we don't stop the stream now, then the thread will take
	 * 90ms (our buffer size) longer to close. */
	pcm->Stop();
}

int64_t RageSound_DSound_Software::GetPosition( const RageSoundBase *snd ) const
{
	return pcm->GetPosition();
}

int RageSound_DSound_Software::MixerThread_start(void *p)
{
	((RageSound_DSound_Software *) p)->MixerThread();
	return 0;
}

RageSound_DSound_Software::RageSound_DSound_Software()
{
	shutdown_mixer_thread = false;
	pcm = NULL;

	/* If we're emulated, we're better off with the WaveOut driver; DS
	 * emulation tends to be desynced. */
	if( ds.IsEmulated() )
		RageException::ThrowNonfatal( "Driver unusable (emulated device)" );

	max_writeahead = safe_writeahead;
	if( PREFSMAN->m_iSoundWriteAhead )
		max_writeahead = PREFSMAN->m_iSoundWriteAhead;

	/* Create a DirectSound stream, but don't force it into hardware. */
	pcm = new DSoundBuf( ds, DSoundBuf::HW_DONT_CARE, channels, samplerate, 16, max_writeahead );

	/* Fill a buffer before we start playing, so we don't play whatever junk is
	 * in the buffer. */
	char *locked_buf;
	unsigned len;
	while( pcm->get_output_buf(&locked_buf, &len, chunksize()) )
	{
		memset( locked_buf, 0, len );
		pcm->release_output_buf(locked_buf, len);
	}

	StartDecodeThread();

	/* Start playing. */
	pcm->Play();

	MixingThread.SetName("Mixer thread");
	MixingThread.Create( MixerThread_start, this );
}

RageSound_DSound_Software::~RageSound_DSound_Software()
{
	/* Signal the mixing thread to quit. */
	shutdown_mixer_thread = true;
	LOG->Trace("Shutting down mixer thread ...");
	LOG->Flush();
	MixingThread.Wait();
	LOG->Trace("Mixer thread shut down.");
	LOG->Flush();

	delete pcm;
}

void RageSound_DSound_Software::SetupDecodingThread()
{
	if( !SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_ABOVE_NORMAL) )
		LOG->Warn( werr_ssprintf(GetLastError(), "Failed to set decoding thread priority") );
}

float RageSound_DSound_Software::GetPlayLatency() const
{
	return (1.0f / samplerate) * max_writeahead;
}

int RageSound_DSound_Software::GetSampleRate( int rate ) const
{
	return samplerate;
}

/*
 * Copyright (c) 2002-2004 by the person(s) listed below.  All rights reserved.
 *
 * Glenn Maynard
 */
