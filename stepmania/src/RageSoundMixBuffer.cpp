#include "global.h"
#include "RageSoundMixBuffer.h"
#include "RageUtil.h"

RageSoundMixBuffer::RageSoundMixBuffer()
{
	m_iBufSize = m_iBufUsed = 0;
	m_pMixbuf = NULL;
	m_iOffset = 0;
	SetVolume( 1.0f );
}

RageSoundMixBuffer::~RageSoundMixBuffer()
{
	free( m_pMixbuf );
}

void RageSoundMixBuffer::SetVolume( float f )
{
	m_iVolumeFactor = int(256*f);
}

/* write() will start mixing iOffset samples into the buffer.  Be careful; this is
 * measured in samples, not frames, so if the data is stereo, multiply by two. */
void RageSoundMixBuffer::SetWriteOffset( int iOffset )
{
	m_iOffset = iOffset;
}

void RageSoundMixBuffer::Extend( unsigned iSamples )
{
	const unsigned realsize = iSamples+m_iOffset;
	if( m_iBufSize < realsize )
	{
		m_pMixbuf = (int32_t *) realloc( m_pMixbuf, sizeof(int32_t) * realsize );
		m_iBufSize = realsize;
	}

	if( m_iBufUsed < realsize )
	{
		memset( m_pMixbuf + m_iBufUsed, 0, (realsize - m_iBufUsed) * sizeof(int32_t) );
		m_iBufUsed = realsize;
	}
}

void RageSoundMixBuffer::write( const int16_t *buf, unsigned size )
{
	Extend( size );

	/* Scale volume and add. */
	int32_t *pBuf = m_pMixbuf+m_iOffset;
	for( unsigned pos = 0; pos < size; ++pos )
	{
		*pBuf += buf[pos] * m_iVolumeFactor; ++pBuf;
	}
}

void RageSoundMixBuffer::read( int16_t *buf )
{
	for( unsigned pos = 0; pos < m_iBufUsed; ++pos )
	{
		int32_t out = (m_pMixbuf[pos]) / 256;
		buf[pos] = (int16_t) clamp( out, -32768, 32767 );
	}
	m_iBufUsed = 0;
}

void RageSoundMixBuffer::read( float *buf )
{
	const int Minimum = -32768 * 256;
	const int Maximum = 32767 * 256;

	for( unsigned pos = 0; pos < m_iBufUsed; ++pos )
		buf[pos] = SCALE( (float)m_pMixbuf[pos], Minimum, Maximum, -1.0f, 1.0f );

	m_iBufUsed = 0;
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
