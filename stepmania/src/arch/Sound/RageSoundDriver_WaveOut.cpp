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

const int channels = 2;
const int bytes_per_frame = channels*2;		/* 16-bit */
const int samplerate = 44100;
const int buffersize_frames = 1024*8;	/* in frames */
const int buffersize = buffersize_frames * bytes_per_frame; /* in bytes */

const int num_chunks = 8;
const int chunksize_frames = buffersize_frames / num_chunks;
const int chunksize = buffersize / num_chunks; /* in bytes */

static CString wo_ssprintf( MMRESULT err, const char *fmt, ...)
{
	char buf[MAXERRORLENGTH];
	waveOutGetErrorText(err, buf, MAXERRORLENGTH);

    va_list	va;
    va_start(va, fmt);
    CString s = vssprintf( fmt, va );
    va_end(va);

	return s += ssprintf( "(%s)", buf );
}

int RageSound_WaveOut::MixerThread_start(void *p)
{
	((RageSound_WaveOut *) p)->MixerThread();
	return 0;
}

void RageSound_WaveOut::MixerThread()
{
	if( !SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_ABOVE_NORMAL) )
		LOG->Warn( werr_ssprintf(GetLastError(), "Failed to set sound thread priority") );

	while( !shutdown )
	{
		while( GetData() )
			;

		WaitForSingleObject(sound_event, 10);
	}

	waveOutReset(wo);
}

bool RageSound_WaveOut::GetData()
{
	/* Look for a free buffer. */
	int b;
	for( b = 0; b < num_chunks; ++b )
		if(buffers[b].dwFlags & WHDR_DONE) break;
	if( b == num_chunks )
		return false;

	/* Call the callback. */
	this->Mix( (int16_t *) buffers[b].lpData, chunksize_frames, last_cursor_pos, GetPosition( NULL ) );

	MMRESULT ret = waveOutWrite(wo, &buffers[b], sizeof(buffers[b]));
  	if(ret != MMSYSERR_NOERROR)
		RageException::Throw(wo_ssprintf(ret, "waveOutWrite failed"));

	/* Increment last_cursor_pos. */
	last_cursor_pos += chunksize_frames;

	return true;
}

void RageSound_WaveOut::SetupDecodingThread()
{
	if( !SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_ABOVE_NORMAL) )
		LOG->Warn( werr_ssprintf(GetLastError(), "Failed to set sound thread priority") );
}

int64_t RageSound_WaveOut::GetPosition( const RageSoundBase *snd ) const
{
	MMTIME tm;
	tm.wType = TIME_SAMPLES;
	MMRESULT ret = waveOutGetPosition(wo, &tm, sizeof(tm));
  	if(ret != MMSYSERR_NOERROR)
		RageException::Throw(wo_ssprintf(ret, "waveOutGetPosition failed"));

	return tm.u.sample;
}

RageSound_WaveOut::RageSound_WaveOut()
{
	shutdown = false;
	last_cursor_pos = 0;

	sound_event = CreateEvent(NULL, false, true, NULL);

	wo = NULL;
}

CString RageSound_WaveOut::Init()
{
	WAVEFORMATEX fmt;
	fmt.wFormatTag = WAVE_FORMAT_PCM;
    fmt.nChannels = channels;
	fmt.cbSize = 0;
	fmt.nSamplesPerSec = samplerate;
	fmt.wBitsPerSample = 16;
	fmt.nBlockAlign = fmt.nChannels * fmt.wBitsPerSample / 8;
	fmt.nAvgBytesPerSec = fmt.nSamplesPerSec * fmt.nBlockAlign;

	MMRESULT ret = waveOutOpen( &wo, WAVE_MAPPER, &fmt, (DWORD_PTR) sound_event, NULL, CALLBACK_EVENT );
	if( ret != MMSYSERR_NOERROR )
		return wo_ssprintf( ret, "waveOutOpen failed" );

	ZERO( buffers );
	for(int b = 0; b < num_chunks; ++b)
	{
		buffers[b].dwBufferLength = chunksize;
		buffers[b].lpData = new char[chunksize];
		ret = waveOutPrepareHeader(wo, &buffers[b], sizeof(buffers[b]));
		if( ret != MMSYSERR_NOERROR )
			return wo_ssprintf( ret, "waveOutPrepareHeader failed" );
		buffers[b].dwFlags |= WHDR_DONE;
	}

	/* We have a very large writeahead; make sure we have a large enough decode
	 * buffer to recover cleanly from underruns. */
	SetDecodeBufferSize( buffersize_frames * 3/2 );
	StartDecodeThread();

	MixingThread.SetName("Mixer thread");
	MixingThread.Create( MixerThread_start, this );

	return "";
}

RageSound_WaveOut::~RageSound_WaveOut()
{
	/* Signal the mixing thread to quit. */
	if( MixingThread.IsCreated() )
	{
		shutdown = true;
		SetEvent( sound_event );
		LOG->Trace("Shutting down mixer thread ...");
		MixingThread.Wait();
		LOG->Trace("Mixer thread shut down.");
	}

	if( wo != NULL )
	{
		for( int b = 0; b < num_chunks && buffers[b].lpData != NULL; ++b )
		{
			waveOutUnprepareHeader( wo, &buffers[b], sizeof(buffers[b]) );
			delete [] buffers[b].lpData;
		}

		waveOutClose( wo );
	}

	CloseHandle(sound_event);
}

float RageSound_WaveOut::GetPlayLatency() const
{
	/* If we have a 1000-byte buffer, and we fill 100 bytes at a time, we
	 * almost always have between 900 and 1000 bytes filled; on average, 950. */
	return (buffersize_frames - chunksize_frames/2) * (1.0f / samplerate);
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
