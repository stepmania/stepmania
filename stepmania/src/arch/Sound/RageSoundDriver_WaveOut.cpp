#include "global.h"
#include "RageSoundDriver_WaveOut.h"

#if defined(_MSC_VER)
#pragma comment(lib, "winmm.lib")
#endif

#include "RageTimer.h"
#include "RageLog.h"
#include "RageSound.h"
#include "RageUtil.h"
#include "RageSoundManager.h"
#include "PrefsManager.h"

const int channels = 2;
const int bytes_per_frame = channels*2;		/* 16-bit */
const int buffersize_frames = 1024*8;	/* in frames */
const int buffersize = buffersize_frames * bytes_per_frame; /* in bytes */

const int num_chunks = 8;
const int chunksize_frames = buffersize_frames / num_chunks;
const int chunksize = buffersize / num_chunks; /* in bytes */

static CString wo_ssprintf( MMRESULT err, const char *szFmt, ...)
{
	char szBuf[MAXERRORLENGTH];
	waveOutGetErrorText( err, szBuf, MAXERRORLENGTH );

    va_list	va;
    va_start( va, szFmt );
    CString s = vssprintf( szFmt, va );
    va_end( va );

	return s += ssprintf( "(%s)", szBuf );
}

int RageSound_WaveOut::MixerThread_start( void *p )
{
	((RageSound_WaveOut *) p)->MixerThread();
	return 0;
}

void RageSound_WaveOut::MixerThread()
{
	if( !SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_ABOVE_NORMAL) )
		LOG->Warn( werr_ssprintf(GetLastError(), "Failed to set sound thread priority") );

	while( !m_bShutdown )
	{
		while( GetData() )
			;

		WaitForSingleObject( m_hSoundEvent, 10 );
	}

	waveOutReset( m_hWaveOut );
}

bool RageSound_WaveOut::GetData()
{
	/* Look for a free buffer. */
	int b;
	for( b = 0; b < num_chunks; ++b )
		if( m_aBuffers[b].dwFlags & WHDR_DONE )
			break;
	if( b == num_chunks )
		return false;

	/* Call the callback. */
	this->Mix( (int16_t *) m_aBuffers[b].lpData, chunksize_frames, m_iLastCursorPos, GetPosition( NULL ) );

	MMRESULT ret = waveOutWrite( m_hWaveOut, &m_aBuffers[b], sizeof(m_aBuffers[b]) );
  	if(ret != MMSYSERR_NOERROR)
		RageException::Throw(wo_ssprintf(ret, "waveOutWrite failed"));

	/* Increment m_iLastCursorPos. */
	m_iLastCursorPos += chunksize_frames;

	return true;
}

void RageSound_WaveOut::SetupDecodingThread()
{
	if( !SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_ABOVE_NORMAL) )
		LOG->Warn( werr_ssprintf(GetLastError(), "Failed to set sound thread priority") );
}

int64_t RageSound_WaveOut::GetPosition( const RageSoundBase *pSound ) const
{
	MMTIME tm;
	tm.wType = TIME_SAMPLES;
	MMRESULT ret = waveOutGetPosition( m_hWaveOut, &tm, sizeof(tm) );
  	if( ret != MMSYSERR_NOERROR )
		RageException::Throw( wo_ssprintf(ret, "waveOutGetPosition failed") );

	return tm.u.sample;
}

RageSound_WaveOut::RageSound_WaveOut()
{
	m_bShutdown = false;
	m_iLastCursorPos = 0;

	m_hSoundEvent = CreateEvent( NULL, false, true, NULL );

	m_hWaveOut = NULL;
}

CString RageSound_WaveOut::Init()
{
	WAVEFORMATEX fmt;
	fmt.wFormatTag = WAVE_FORMAT_PCM;
    fmt.nChannels = channels;
	fmt.cbSize = 0;
	m_iSampleRate = PREFSMAN->m_iSoundPreferredSampleRate;
	fmt.nSamplesPerSec = m_iSampleRate;
	fmt.wBitsPerSample = 16;
	fmt.nBlockAlign = fmt.nChannels * fmt.wBitsPerSample / 8;
	fmt.nAvgBytesPerSec = fmt.nSamplesPerSec * fmt.nBlockAlign;

	MMRESULT ret = waveOutOpen( &m_hWaveOut, WAVE_MAPPER, &fmt, (DWORD_PTR) m_hSoundEvent, NULL, CALLBACK_EVENT );
	if( ret != MMSYSERR_NOERROR )
		return wo_ssprintf( ret, "waveOutOpen failed" );

	ZERO( m_aBuffers );
	for(int b = 0; b < num_chunks; ++b)
	{
		m_aBuffers[b].dwBufferLength = chunksize;
		m_aBuffers[b].lpData = new char[chunksize];
		ret = waveOutPrepareHeader( m_hWaveOut, &m_aBuffers[b], sizeof(m_aBuffers[b]) );
		if( ret != MMSYSERR_NOERROR )
			return wo_ssprintf( ret, "waveOutPrepareHeader failed" );
		m_aBuffers[b].dwFlags |= WHDR_DONE;
	}

	LOG->Info( "WaveOut software mixing at %i hz", m_iSampleRate );

	/* We have a very large writeahead; make sure we have a large enough decode
	 * buffer to recover cleanly from underruns. */
	SetDecodeBufferSize( buffersize_frames * 3/2 );
	StartDecodeThread();

	MixingThread.SetName( "Mixer thread" );
	MixingThread.Create( MixerThread_start, this );

	return CString();
}

RageSound_WaveOut::~RageSound_WaveOut()
{
	/* Signal the mixing thread to quit. */
	if( MixingThread.IsCreated() )
	{
		m_bShutdown = true;
		SetEvent( m_hSoundEvent );
		LOG->Trace( "Shutting down mixer thread ..." );
		MixingThread.Wait();
		LOG->Trace( "Mixer thread shut down." );
	}

	if( m_hWaveOut != NULL )
	{
		for( int b = 0; b < num_chunks && m_aBuffers[b].lpData != NULL; ++b )
		{
			waveOutUnprepareHeader( m_hWaveOut, &m_aBuffers[b], sizeof(m_aBuffers[b]) );
			delete [] m_aBuffers[b].lpData;
		}

		waveOutClose( m_hWaveOut );
	}

	CloseHandle( m_hSoundEvent );
}

float RageSound_WaveOut::GetPlayLatency() const
{
	/* If we have a 1000-byte buffer, and we fill 100 bytes at a time, we
	 * almost always have between 900 and 1000 bytes filled; on average, 950. */
	return (buffersize_frames - chunksize_frames/2) * (1.0f / m_iSampleRate);
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
