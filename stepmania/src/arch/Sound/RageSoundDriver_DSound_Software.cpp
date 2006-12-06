#include "global.h"
#include "RageSoundDriver_DSound_Software.h"
#include "DSoundHelpers.h"

#include "RageLog.h"
#include "RageUtil.h"
#include "RageSoundManager.h"
#include "PrefsManager.h"
#include "archutils/Win32/ErrorStrings.h"

REGISTER_SOUND_DRIVER_CLASS2( DirectSound-sw, DSound_Software );

static const int channels = 2;
static const int bytes_per_frame = channels*2; /* 16-bit */
static const int safe_writeahead = 1024*4; /* in frames */
static int g_iMaxWriteahead;

/* We'll fill the buffer in chunks this big. */
static const int num_chunks = 8;
static int chunksize() { return g_iMaxWriteahead / num_chunks; }

void RageSound_DSound_Software::MixerThread()
{
	if( !SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_TIME_CRITICAL) )
		if( !SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_ABOVE_NORMAL) )
			LOG->Warn(werr_ssprintf(GetLastError(), "Failed to set sound thread priority"));

	/* Fill a buffer before we start playing, so we don't play whatever junk is
	 * in the buffer. */
	char *locked_buf;
	unsigned len;
	while( m_pPCM->get_output_buf(&locked_buf, &len, chunksize()) )
	{
		memset( locked_buf, 0, len );
		m_pPCM->release_output_buf(locked_buf, len);
	}

	/* Start playing. */
	m_pPCM->Play();

	while( !m_bShutdownMixerThread )
	{
		char *pLockedBuf;
		unsigned iLen;
		const int64_t iPlayPos = m_pPCM->GetOutputPosition(); /* must be called before get_output_buf */

		if( !m_pPCM->get_output_buf(&pLockedBuf, &iLen, chunksize()) )
		{
			Sleep( chunksize()*1000 / m_iSampleRate );
			continue;
		}

		this->Mix( (int16_t *) pLockedBuf, iLen/bytes_per_frame, iPlayPos, m_pPCM->GetPosition() );

		m_pPCM->release_output_buf( pLockedBuf, iLen );
	}

	/* I'm not sure why, but if we don't stop the stream now, then the thread will take
	 * 90ms (our buffer size) longer to close. */
	m_pPCM->Stop();
}

int64_t RageSound_DSound_Software::GetPosition( const RageSoundBase *pSound ) const
{
	return m_pPCM->GetPosition();
}

int RageSound_DSound_Software::MixerThread_start(void *p)
{
	((RageSound_DSound_Software *) p)->MixerThread();
	return 0;
}

RageSound_DSound_Software::RageSound_DSound_Software()
{
	m_bShutdownMixerThread = false;
	m_pPCM = NULL;
}

RString RageSound_DSound_Software::Init()
{
	RString sError = ds.Init();
	if( sError != "" )
		return sError;

	/* If we're emulated, we're better off with the WaveOut driver; DS
	 * emulation tends to be desynced. */
	if( ds.IsEmulated() )
		return "Driver unusable (emulated device)";

	g_iMaxWriteahead = safe_writeahead;
	if( PREFSMAN->m_iSoundWriteAhead )
		g_iMaxWriteahead = PREFSMAN->m_iSoundWriteAhead;

	/* Create a DirectSound stream, but don't force it into hardware. */
	m_pPCM = new DSoundBuf;
	m_iSampleRate = PREFSMAN->m_iSoundPreferredSampleRate;
	sError = m_pPCM->Init( ds, DSoundBuf::HW_DONT_CARE, channels, m_iSampleRate, 16, g_iMaxWriteahead );
	if( sError != "" )
		return sError;

	LOG->Info( "Software mixing at %i hz", m_iSampleRate );

	StartDecodeThread();

	m_MixingThread.SetName("Mixer thread");
	m_MixingThread.Create( MixerThread_start, this );

	return RString();
}

RageSound_DSound_Software::~RageSound_DSound_Software()
{
	/* Signal the mixing thread to quit. */
	if( m_MixingThread.IsCreated() )
	{
		m_bShutdownMixerThread = true;
		LOG->Trace("Shutting down mixer thread ...");
		LOG->Flush();
		m_MixingThread.Wait();
		LOG->Trace("Mixer thread shut down.");
		LOG->Flush();
	}

	delete m_pPCM;
}

void RageSound_DSound_Software::SetupDecodingThread()
{
	if( !SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_ABOVE_NORMAL) )
		LOG->Warn( werr_ssprintf(GetLastError(), "Failed to set decoding thread priority") );
}

float RageSound_DSound_Software::GetPlayLatency() const
{
	return (1.0f / m_iSampleRate) * g_iMaxWriteahead;
}

int RageSound_DSound_Software::GetSampleRate( int rate ) const
{
	return m_iSampleRate;
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
