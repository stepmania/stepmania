#include "global.h"
#include "RageSoundMixBuffer.h"
#include "RageSoundManager.h"
#include "RageUtil.h"

RageSoundMixBuffer::RageSoundMixBuffer()
{
	bufsize = used = 0;
	mixbuf = NULL;
	if( SOUNDMAN != NULL )
		SetVolume( SOUNDMAN->GetMixVolume() );
	else
		SetVolume( 1.0f );
}

RageSoundMixBuffer::~RageSoundMixBuffer()
{
	free(mixbuf);
}

void RageSoundMixBuffer::SetVolume( float f )
{
	vol = int(256*f);
}

void RageSoundMixBuffer::write( const int16_t *buf, unsigned size, float volume, int offset )
{
	int factor = vol;
	if( volume != -1 )
		factor = int( 256*volume );

	const unsigned realsize = size+offset;
	if( bufsize < realsize )
	{
		mixbuf = (int32_t *) realloc( mixbuf, sizeof(int32_t) * realsize );
		bufsize = realsize;
	}

	if( used < realsize )
	{
		memset( mixbuf + used, 0, (realsize - used) * sizeof(int32_t) );
		used = realsize;
	}

	/* Scale volume and add. */
	for(unsigned pos = 0; pos < size; ++pos)
		mixbuf[pos+offset] += buf[pos] * factor;
}

void RageSoundMixBuffer::read(int16_t *buf)
{
	for( unsigned pos = 0; pos < used; ++pos )
	{
		int32_t out = (mixbuf[pos]) / 256;
		buf[pos] = (int16_t) clamp( out, -32768, 32767 );
	}
	used = 0;
}

void RageSoundMixBuffer::read( float *buf )
{
	const int Minimum = -32768 * 256;
	const int Maximum = 32767 * 256;

	for( unsigned pos = 0; pos < used; ++pos )
		buf[pos] = SCALE( (float)mixbuf[pos], Minimum, Maximum, -1.0f, 1.0f );

	used = 0;
}

/*
 * Copyright (c) 2002-2004 Glenn Maynard
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
