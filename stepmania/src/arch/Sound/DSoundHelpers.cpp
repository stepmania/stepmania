#include "../../stdafx.h"
#include "DSoundHelpers.h"

/* Stuff shared between the two DirectSound drivers. */
static IDirectSoundBuffer8 *CreateBuf(IDirectSound8 *ds8, WAVEFORMATEX *wavefmt, Uint32 buffersize, bool Hardware)
{
	/* Try to create the secondary buffer */
	DSBUFFERDESC format;
	memset(&format, 0, sizeof(format));
	format.dwSize = sizeof(format);
	format.dwFlags = DSBCAPS_GETCURRENTPOSITION2 | DSBCAPS_GLOBALFOCUS;
	if(Hardware)
		format.dwFlags |= DSBCAPS_LOCHARDWARE;
	format.dwBufferBytes = buffersize;
	format.dwReserved = 0;
	format.lpwfxFormat = wavefmt;

	IDirectSoundBuffer *sndbuf_buf;
	HRESULT hr = ds8->CreateSoundBuffer(&format, &sndbuf_buf, NULL);
	if (FAILED(hr))
		throw "CreateSoundBuffer failed";

	IDirectSoundBuffer8 *buf;
	sndbuf_buf->QueryInterface(IID_IDirectSoundBuffer8, (LPVOID*) &buf);

	return buf;
}

IDirectSoundBuffer8 *CreateBuf(IDirectSound8 *ds8, 
							  int channels, int samplerate, int bits,
							  Uint32 buffersize, bool Hardware)
{
	WAVEFORMATEX waveformat;
	memset(&waveformat, 0, sizeof(waveformat));
	waveformat.cbSize = sizeof(waveformat);
	waveformat.wFormatTag = WAVE_FORMAT_PCM;

	int bytes = bits/8;
	waveformat.wBitsPerSample = WORD(bits);
	waveformat.nChannels = WORD(channels);
	waveformat.nSamplesPerSec = DWORD(samplerate);
	waveformat.nBlockAlign = WORD(bytes*channels);
	waveformat.nAvgBytesPerSec = samplerate * bytes*channels;

	return CreateBuf(ds8, &waveformat, buffersize, Hardware);
}

/*
 * Copyright (c) 2002 by the person(s) listed below.  All rights reserved.
 *
 * Glenn Maynard
 */
