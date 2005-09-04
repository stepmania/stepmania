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
	if( !SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_TIME_CRITICAL) )
		if( !SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_ABOVE_NORMAL) )
			LOG->Warn(werr_ssprintf(GetLastError(), "Failed to set sound thread priority"));

	while( !shutdown_mixer_thread )
	{
		char *locked_buf;
		unsigned len;
		const int64_t play_pos = pcm->GetOutputPosition(); /* must be called before get_output_buf */

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
}

CString RageSound_DSound_Software::Init()
{
	CString sError = ds.Init();
	if( sError != "" )
		return sError;

	/* If we're emulated, we're better off with the WaveOut driver; DS
	 * emulation tends to be desynced. */
	if( ds.IsEmulated() )
		return "Driver unusable (emulated device)";

	max_writeahead = safe_writeahead;
	if( PREFSMAN->m_iSoundWriteAhead )
		max_writeahead = PREFSMAN->m_iSoundWriteAhead;

	/* Create a DirectSound stream, but don't force it into hardware. */
	pcm = new DSoundBuf;
	sError = pcm->Init( ds, DSoundBuf::HW_DONT_CARE, channels, samplerate, 16, max_writeahead );
	if( sError != "" )
		return sError;

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

	return CString();
}

RageSound_DSound_Software::~RageSound_DSound_Software()
{
	/* Signal the mixing thread to quit. */
	if( MixingThread.IsCreated() )
	{
		shutdown_mixer_thread = true;
		LOG->Trace("Shutting down mixer thread ...");
		LOG->Flush();
		MixingThread.Wait();
		LOG->Trace("Mixer thread shut down.");
		LOG->Flush();
	}

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
