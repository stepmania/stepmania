#include "../../global.h"
#include "DSoundHelpers.h"
#include "../../RageUtil.h"
#include "../../RageLog.h"
#include "archutils/Win32/GetFileInformation.h"

#if defined(_WINDOWS)
#include <mmsystem.h>
#endif
#define DIRECTSOUND_VERSION 0x0700
#include <dsound.h>

#pragma comment(lib, "dsound.lib")

BOOL CALLBACK DSound::EnumCallback( LPGUID lpGuid, LPCSTR lpcstrDescription, LPCSTR lpcstrModule, LPVOID lpContext )
{
	CString sLine = ssprintf( "DirectSound Driver: %s", lpcstrDescription );
	if( lpcstrModule[0] )
	{
		sLine += ssprintf( " %s", lpcstrModule );

#ifndef _XBOX
		CString sPath = FindSystemFile( lpcstrModule );
		if( sPath != "" )
		{
			CString ver;
			if( GetFileVersion( sPath, ver ) )
				sLine += ssprintf(" %s", ver.c_str());
		}
#endif
	}

	LOG->Info( "%s", sLine.c_str() );

	return TRUE;
}

void DSound::SetPrimaryBufferMode()
{
#ifndef _XBOX
	DSBUFFERDESC format;
	memset( &format, 0, sizeof(format) );
	format.dwSize = sizeof(format);
	format.dwFlags = DSBCAPS_PRIMARYBUFFER;
	format.dwBufferBytes = 0;
	format.lpwfxFormat = NULL;

	IDirectSoundBuffer *buf;
	HRESULT hr = this->GetDS()->CreateSoundBuffer(&format, &buf, NULL);
	/* hr */
	if( FAILED(hr) )
	{
		LOG->Warn(hr_ssprintf(hr, "Couldn't create primary buffer"));
		return;
	}

    WAVEFORMATEX waveformat;
	memset( &waveformat, 0, sizeof(waveformat) );
    waveformat.cbSize = 0;
    waveformat.wFormatTag = WAVE_FORMAT_PCM;
	waveformat.wBitsPerSample = 16;
	waveformat.nChannels = 2;
	waveformat.nSamplesPerSec = 44100;
	waveformat.nBlockAlign = 4;
	waveformat.nAvgBytesPerSec = waveformat.nSamplesPerSec * waveformat.nBlockAlign;

	// Set the primary buffer's format
    hr = IDirectSoundBuffer_SetFormat( buf, &waveformat );
	if( FAILED(hr) )
		LOG->Warn( hr_ssprintf(hr, "SetFormat on primary buffer") );

	DWORD got;
	hr = buf->GetFormat( &waveformat, sizeof(waveformat), &got );
	if( FAILED(hr) )
		LOG->Warn( hr_ssprintf(hr, "GetFormat on primary buffer") );
	else if( waveformat.nSamplesPerSec != 44100 )
		LOG->Warn( "Primary buffer set to %i instead of 44100", waveformat.nSamplesPerSec );

	/* MS docs:
	 *
	 * When there are no sounds playing, DirectSound stops the mixer engine and halts DMA 
	 * (direct memory access) activity. If your application has frequent short intervals of
	 * silence, the overhead of starting and stopping the mixer each time a sound is played
	 * may be worse than the DMA overhead if you kept the mixer active. Also, some sound
	 * hardware or drivers may produce unwanted audible artifacts from frequent starting and
	 * stopping of playback. If your application is playing audio almost continuously with only
	 * short breaks of silence, you can force the mixer engine to remain active by calling the
	 * IDirectSoundBuffer::Play method for the primary buffer. The mixer will continue to run
	 * silently.
	 *
	 * However, I just added the above code and I don't want to change more until it's tested.
	 */
//	buf->Play(0, 0, DSBPLAY_LOOPING);

	buf->Release();
#endif
}

DSound::DSound()
{
	HRESULT hr;

#ifndef _XBOX
    // Initialize COM
    if( FAILED( hr = CoInitialize( NULL ) ) )
		RageException::ThrowNonfatal(hr_ssprintf(hr, "CoInitialize"));
#endif

    // Create IDirectSound using the primary sound device
    if( FAILED( hr = DirectSoundCreate( NULL, &ds, NULL ) ) )
		RageException::ThrowNonfatal(hr_ssprintf(hr, "DirectSoundCreate"));

#ifndef _XBOX
	static bool bShownInfo = false;
	if( !bShownInfo )
	{
		bShownInfo = true;
		DirectSoundEnumerate(EnumCallback, 0);

		DSCAPS Caps;
		Caps.dwSize = sizeof(Caps);
		HRESULT hr;
		if( FAILED(hr = ds->GetCaps(&Caps)) )
		{
			LOG->Warn( hr_ssprintf(hr, "ds->GetCaps failed") );
		}
		else
		{
			LOG->Info( "DirectSound sample rates: %i..%i %s", Caps.dwMinSecondarySampleRate, Caps.dwMaxSecondarySampleRate,
				(Caps.dwFlags & DSCAPS_CONTINUOUSRATE)?"(continuous)":"" );
		}
	}

	/* Try to set primary mixing privileges */
	hr = ds->SetCooperativeLevel( GetDesktopWindow(), DSSCL_PRIORITY );
#endif

	SetPrimaryBufferMode();
}

DSound::~DSound()
{
	ds->Release();
#ifndef _XBOX
    CoUninitialize();
#endif
}

bool DSound::IsEmulated() const
{
#ifndef _XBOX
	/* Don't bother wasting time trying to create buffers if we're
 	 * emulated.  This also gives us better diagnostic information. */
	DSCAPS Caps;
	Caps.dwSize = sizeof(Caps);
	HRESULT hr;
	if( FAILED(hr = ds->GetCaps(&Caps)) )
	{
		LOG->Warn( hr_ssprintf(hr, "ds->GetCaps failed") );
		/* This is strange, so let's be conservative. */
		return true;
	}

	return !!(Caps.dwFlags & DSCAPS_EMULDRIVER);
#else
	return false;
#endif
}

DSoundBuf::DSoundBuf( DSound &ds, DSoundBuf::hw hardware,
					  int channels_, int samplerate_, int samplebits_, int writeahead_ )
{
	channels = channels_;
	samplerate = samplerate_;
	samplebits = samplebits_;
	writeahead = writeahead_ * bytes_per_frame();
	volume = -1; /* unset */
	buffer_locked = false;
	write_cursor_pos = write_cursor = buffer_bytes_filled = 0;
	extra_writeahead = 0;
	LastPosition = 0;
	playing = false;

	/* The size of the actual DSound buffer.  This can be large; we generally
	 * won't fill it completely. */
	buffersize = 1024*64;
	buffersize = max( buffersize, writeahead );

	WAVEFORMATEX waveformat;
	memset( &waveformat, 0, sizeof(waveformat) );
	waveformat.cbSize = 0;
	waveformat.wFormatTag = WAVE_FORMAT_PCM;

	bool NeedCtrlFrequency = false;
	if( samplerate == DYNAMIC_SAMPLERATE )
	{
		samplerate = 44100;
		NeedCtrlFrequency = true;
	}

	int bytes = samplebits/8;
	waveformat.wBitsPerSample = WORD(samplebits);
	waveformat.nChannels = WORD(channels);
	waveformat.nSamplesPerSec = DWORD(samplerate);
	waveformat.nBlockAlign = WORD(bytes*channels);
	waveformat.nAvgBytesPerSec = samplerate * bytes*channels;

	/* Try to create the secondary buffer */
	DSBUFFERDESC format;
	memset( &format, 0, sizeof(format) );
	format.dwSize = sizeof(format);
#ifdef _XBOX
	format.dwFlags = 0;
#else
	format.dwFlags = DSBCAPS_GETCURRENTPOSITION2 | DSBCAPS_GLOBALFOCUS | DSBCAPS_CTRLVOLUME;
#endif
	
#ifndef _XBOX
	/* Don't use DSBCAPS_STATIC.  It's meant for static buffers, and we
	 * only use streaming buffers. */
	if( hardware == HW_HARDWARE )
		format.dwFlags |= DSBCAPS_LOCHARDWARE;
	else
		format.dwFlags |= DSBCAPS_LOCSOFTWARE;
#endif

	if( NeedCtrlFrequency )
		format.dwFlags |= DSBCAPS_CTRLFREQUENCY;

	format.dwBufferBytes = buffersize;
#ifndef _XBOX
	format.dwReserved = 0;
#else
	DSMIXBINVOLUMEPAIR dsmbvp[8] =
	{
		{ DSMIXBIN_FRONT_LEFT,		DSBVOLUME_MAX}, // left channel
		{ DSMIXBIN_FRONT_RIGHT,		DSBVOLUME_MAX}, // right channel
		{ DSMIXBIN_FRONT_CENTER,	DSBVOLUME_MAX}, // left channel
		{ DSMIXBIN_FRONT_CENTER,	DSBVOLUME_MAX}, // right channel
		{ DSMIXBIN_BACK_LEFT,		DSBVOLUME_MAX}, // left channel
		{ DSMIXBIN_BACK_RIGHT,		DSBVOLUME_MAX}, // right channel
		{ DSMIXBIN_LOW_FREQUENCY,	DSBVOLUME_MAX}, // left channel
		{ DSMIXBIN_LOW_FREQUENCY,	DSBVOLUME_MAX}  // right channel
	};
	DSMIXBINS dsmb;
	dsmb.dwMixBinCount = 8;
	dsmb.lpMixBinVolumePairs = dsmbvp;

	format.lpMixBins			= &dsmb;
#endif

	format.lpwfxFormat = &waveformat;

	HRESULT hr = ds.GetDS()->CreateSoundBuffer( &format, &buf, NULL );
	if( FAILED(hr) )
		RageException::ThrowNonfatal( hr_ssprintf(hr, "CreateSoundBuffer failed") );

#ifndef _XBOX
	/* I'm not sure this should ever be needed, but ... */
	DSBCAPS bcaps;
	bcaps.dwSize=sizeof(bcaps);
	hr = buf->GetCaps(&bcaps);
	if( FAILED(hr) )
		RageException::Throw(hr_ssprintf(hr, "buf->GetCaps"));
	if( int(bcaps.dwBufferBytes) != buffersize )
	{
		LOG->Warn( "bcaps.dwBufferBytes (%i) != buffersize(%i); adjusting", bcaps.dwBufferBytes, buffersize );
		buffersize = bcaps.dwBufferBytes;
		writeahead = min( writeahead, buffersize );
	}

	if( !(bcaps.dwFlags & DSBCAPS_CTRLVOLUME) )
		LOG->Warn( "Sound channel missing DSBCAPS_CTRLVOLUME" );
	if( !(bcaps.dwFlags & DSBCAPS_GETCURRENTPOSITION2) )
		LOG->Warn( "Sound channel missing DSBCAPS_GETCURRENTPOSITION2" );

	DWORD got;
	hr = buf->GetFormat( &waveformat, sizeof(waveformat), &got );
	if( FAILED(hr) )
		LOG->Warn( hr_ssprintf(hr, "GetFormat on secondary buffer") );
	else if( (int) waveformat.nSamplesPerSec != samplerate )
		LOG->Warn( "Secondary buffer set to %i instead of %i", waveformat.nSamplesPerSec, samplerate );
#endif
	
	temp_buffer = new char[buffersize];
}

void DSoundBuf::SetSampleRate(int hz)
{
	samplerate = hz;
	HRESULT hr = buf->SetFrequency( hz );
	if( FAILED(hr) )
		RageException::Throw( hr_ssprintf(hr, "buf->SetFrequency(%i)", hz) );
}

void DSoundBuf::SetVolume(float vol)
{
	ASSERT(vol >= 0);
	ASSERT(vol <= 1);
	
	if( vol == 0 )
		vol = 0.001f;		// fix log10f(0) == -INF
	float vl2 = log10f(vol) / log10f(2); /* vol log 2 */

	/* Volume is a multiplier; SetVolume wants attenuation in hundredths of a decibel. */
	const int new_volume = max( int(1000 * vl2), DSBVOLUME_MIN );

	if( volume == new_volume )
		return;

	HRESULT hr = buf->SetVolume( new_volume );
	if( FAILED(hr) )
	{
		static bool bWarned = false;
		if( !bWarned )
			LOG->Warn( hr_ssprintf(hr, "DirectSoundBuffer::SetVolume(%i) failed", volume) );
		bWarned = true;
		return;
	}

	volume = new_volume;
}

/* Determine if "pos" is between "start" and "end", for a circular buffer.  Note that
 * a start/end pos is ambiguous when start == end; it can mean that the buffer is
 * completely full or completely empty; this function treats it as completely empty. */
static bool contained( int start, int end, int pos )
{
	if( end >= start ) /* start ... pos ... end */
		return start <= pos && pos < end;
	else
		return pos >= start || pos < end;
}

DSoundBuf::~DSoundBuf()
{
	buf->Release();
	delete [] temp_buffer;
}

void round_up( int &i, int to )
{
	i += (to-1);
	i /= to;
	i *= to;
}

/* Check to make sure that, given the current writeahead and chunksize, we're
 * capable of filling the prefetch region entirely.  If we aren't, increase
 * the writeahead.  If this happens, we're underruning. */
void DSoundBuf::CheckWriteahead( int cursorstart, int cursorend )
{
	/* If we're in a recovering-from-underrun state, stop. */
	if( extra_writeahead )
		return;

	/* If the driver is requesting an unreasonably large prefetch, ignore it entirely.
	 * Some drivers seem to give broken write cursors sporadically, requesting that
	 * almost the entire buffer be filled.  There's no reason a driver should ever need
	 * more than 8k frames of writeahead. */
	int prefetch = cursorend - cursorstart;
	wrap( prefetch, buffersize );

	if( prefetch >= 1024*32 )
	{
		static bool bLogged = false;
		if( bLogged )
			return;
		bLogged = true;

		LOG->Warn("Sound driver is requesting an overly large prefetch: wants %i (cursor at %i..%i), writeahead not adjusted",
			prefetch/bytes_per_frame(), cursorstart, cursorend );
		return;
	}

	if( writeahead >= prefetch )
		return;

	/* We need to increase the writeahead. */
	LOG->Trace("insufficient writeahead: wants %i (cursor at %i..%i), writeahead adjusted from %i to %i",
		prefetch/bytes_per_frame(), cursorstart, cursorend, writeahead, prefetch );

	writeahead = prefetch;
}

/* Figure out if we've underrun, and act if appropriate. */
void DSoundBuf::CheckUnderrun( int cursorstart, int cursorend )
{
	/* If the buffer is full, we can't be underrunning. */
	if( buffer_bytes_filled >= buffersize )
		return;

	/* If nothing is expected to be filled, we can't underrun. */
	if( cursorstart == cursorend )
		return;

	/* If we're already in a recovering-from-underrun state, stop. */
	if( extra_writeahead )
		return;

	int first_byte_filled = write_cursor-buffer_bytes_filled;
	wrap( first_byte_filled, buffersize );

	/* If the end of the play cursor has data, we haven't underrun. */
	if( buffer_bytes_filled > 0 && contained(first_byte_filled, write_cursor, cursorend) )
		return;

	/* Extend the writeahead to force fill as much as required to stop underrunning.
	 * This has a major benefit: if we havn't skipped so long we've passed a whole
	 * buffer (64k = ~350ms), this doesn't break stride.  We'll skip forward, but
	 * the beat won't be lost, which is a lot easier to recover from in play. */
	/* XXX: If this happens repeatedly over a period of time, increase writeahead. */
	/* XXX: What was I doing here?  This isn't working.  We want to know the writeahead
	 * value needed to fill from the current first_byte_filled all the way to cursorend. */
	// int needed_writeahead = (cursorstart + writeahead) - write_cursor;
	int needed_writeahead = cursorend - first_byte_filled;
	wrap( needed_writeahead, buffersize );
	if( needed_writeahead > writeahead )
	{
		extra_writeahead = needed_writeahead - writeahead;
		writeahead = needed_writeahead;
	}

	int missed_by = cursorend - write_cursor;
	wrap( missed_by, buffersize );

	CString s = ssprintf( "underrun: %i..%i (%i) filled but cursor at %i..%i; missed it by %i",
		first_byte_filled, write_cursor, buffer_bytes_filled, cursorstart, cursorend, missed_by );

	if( extra_writeahead )
		s += ssprintf( "; extended writeahead by %i to %i", extra_writeahead, writeahead );

	LOG->Trace( "%s", s.c_str() );
}

bool DSoundBuf::get_output_buf( char **buffer, unsigned *bufsiz, int chunksize )
{
	ASSERT(!buffer_locked);

	chunksize *= bytes_per_frame();

	DWORD cursorstart, cursorend;

	HRESULT result;

	/* It's easiest to think of the cursor as a block, starting and ending at
	 * the two values returned by GetCurrentPosition, that we can't write to. */
	result = buf->GetCurrentPosition( &cursorstart, &cursorend );
#ifndef _XBOX
	if ( result == DSERR_BUFFERLOST )
	{
		buf->Restore();
		result = buf->GetCurrentPosition( &cursorstart, &cursorend );
	}
	if ( result != DS_OK )
	{
		LOG->Warn( hr_ssprintf(result, "DirectSound::GetCurrentPosition failed") );
		return false;
	}
#endif

	/* Some cards (Creative AudioPCI) have a no-write area even when not playing.  I'm not
	 * sure what that means, but it breaks the assumption that we can fill the whole writeahead
	 * when prebuffering. */
	if( !playing )
		cursorend = cursorstart;

	/*
	 * Some cards (Game Theater XP 7.1 hercwdm.sys 5.12.01.4101 [466688b, 01-10-2003])
	 * have odd behavior when starting a sound: the start/end cursors go:
	 *
	 * 0,0             end cursor forced equal to start above (normal)
	 * 4608, 1764      end cursor trailing the write cursor; except with old emulated
	 *                   WaveOut devices, this shouldn't happen; it indicates that the
	 *                   driver expects almost the whole buffer to be filled.  Also, the
	 *                   play cursor is too far ahead from the last call for the amount
	 *                   of actual time passed.
	 * 704, XXX        start cursor moves back to where it should be.  I don't have an exact
	 *                   end cursor position, but in general from now on it stays about 5kb
	 *                   ahead of start (which is where it should be).
	 *
	 * The second call is completely wrong; both the start and end cursors are meaningless.
	 * Detect this: if the end cursor is close behind the start cursor, don't do anything.
	 * (We can't; we have no idea what the cursors actually are.)
	 */
	{
		int prefetch = cursorend - cursorstart;
		wrap( prefetch, buffersize );

		if( buffersize-prefetch < 1024*4 )
		{
			LOG->Trace( "Strange DirectSound cursor ignored: %i..%i", cursorstart, cursorend );
			return false;
		}
	}

	/* Update buffer_bytes_filled. */
	{
		int first_byte_filled = write_cursor-buffer_bytes_filled;
		wrap( first_byte_filled, buffersize );

		/* The number of bytes that have been played since the last time we got here: */
		int bytes_played = cursorstart - first_byte_filled;
		wrap( bytes_played, buffersize );

		buffer_bytes_filled -= bytes_played;
		buffer_bytes_filled = max( 0, buffer_bytes_filled );

		if( extra_writeahead )
		{
			int used = min( extra_writeahead, bytes_played );
			LOG->Trace("used %i of %i", used, extra_writeahead);
			writeahead -= used;
			extra_writeahead -= used;
		}
	}

	CheckWriteahead( cursorstart, cursorend );
	CheckUnderrun( cursorstart, cursorend );

	/* If we already have enough bytes written ahead, stop. */
	if( buffer_bytes_filled > writeahead )
		return false;

	int num_bytes_empty = writeahead-buffer_bytes_filled;

	/* num_bytes_empty is the amount of free buffer space.  If it's
	 * too small, come back later. */
	if( num_bytes_empty < chunksize )
		return false;

//	LOG->Trace("gave %i at %i (%i, %i) %i filled", num_bytes_empty, write_cursor, cursor, write, buffer_bytes_filled);

	/* Lock the audio buffer. */
	result = buf->Lock( write_cursor, num_bytes_empty, (LPVOID *)&locked_buf1, (DWORD *) &locked_size1, (LPVOID *)&locked_buf2, (DWORD *) &locked_size2, 0 );

#ifndef _XBOX
	if ( result == DSERR_BUFFERLOST )
	{
		buf->Restore();
		result = buf->Lock( write_cursor, num_bytes_empty, (LPVOID *)&locked_buf1, (DWORD *) &locked_size1, (LPVOID *)&locked_buf2, (DWORD *) &locked_size2, 0 );
	}
#endif
	if ( result != DS_OK )
	{
		LOG->Warn( hr_ssprintf(result, "Couldn't lock the DirectSound buffer.") );
		return false;
	}

	*buffer = temp_buffer;
	*bufsiz = locked_size1 + locked_size2;

	write_cursor += num_bytes_empty;
	if( write_cursor >= buffersize )
		write_cursor -= buffersize;

	buffer_bytes_filled += num_bytes_empty;
	write_cursor_pos += num_bytes_empty / bytes_per_frame();

	buffer_locked = true;

	return true;
}

void DSoundBuf::release_output_buf( char *buffer, unsigned bufsiz )
{
	memcpy( locked_buf1, buffer, locked_size1 );
	memcpy( locked_buf2, buffer+locked_size1, locked_size2 );
	buf->Unlock( locked_buf1, locked_size1, locked_buf2, locked_size2 );
	buffer_locked = false;
}

int64_t DSoundBuf::GetPosition() const
{
	DWORD cursor, junk;
	HRESULT hr = buf->GetCurrentPosition( &cursor, &junk );
	ASSERT_M( SUCCEEDED(hr), hr_ssprintf(hr, "GetCurrentPosition") );

	/* This happens occasionally on "Realtek AC97 Audio". */
	if( (int) cursor == buffersize )
		cursor = 0;
	ASSERT_M( (int) cursor < buffersize, ssprintf("%i, %i", cursor, buffersize) );

	int cursor_frames = int(cursor) / bytes_per_frame();
	int write_cursor_frames = write_cursor / bytes_per_frame();

	int frames_behind = write_cursor_frames - cursor_frames;
	/* frames_behind will be 0 if we're called before the buffer starts playing:
	 * both write_cursor_frames and cursor_frames will be 0. */
	if( frames_behind < 0 )
		frames_behind += buffersize_frames(); /* unwrap */

	int64_t ret = write_cursor_pos - frames_behind;

	/* Failsafe: never return a value smaller than we've already returned.
	 * This can happen once in a while in underrun conditions. */
	ret = max( LastPosition, ret );
	LastPosition = ret;

	return ret;
}

void DSoundBuf::Play()
{
	if( playing )
		return;
	buf->Play( 0, 0, DSBPLAY_LOOPING );
	playing = true;
}

void DSoundBuf::Stop()
{
	if( !playing )
		return;

	buf->Stop();
	buf->SetCurrentPosition(0);

	write_cursor_pos = write_cursor = buffer_bytes_filled = 0;
	LastPosition = 0;

	writeahead -= extra_writeahead;
	extra_writeahead = 0;

	/* When stopped and rewound, the play and write cursors should both be 0. */
	/* This isn't true on some broken cards. */
//	DWORD play, write;
//	buf->GetCurrentPosition( &play, &write );
//	ASSERT_M( play == 0 && write == 0, ssprintf("%i, %i", play, write) );

	playing = false;
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
