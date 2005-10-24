#include "global.h"
#include "RageSoundResampler.h"
#include "RageUtil.h"
#include "RageLog.h"

/*
 * This class handles sound resampling.
 *
 * This isn't very efficient; we write to a static buffer instead of a circular
 * one.  I'll optimize it if it becomes an issue.
 */

RageSoundResampler::RageSoundResampler()
{
	reset();
}

void RageSoundResampler::reset()
{
	m_bAtEof = false;
	memset( m_iPrevSample, 0, sizeof(m_iPrevSample) );
	m_iPos = 0;
	m_OutBuf.clear();
	m_iChannels = 2;
}


/* Write data to be converted. */
void RageSoundResampler::write(const void *data_, int bytes)
{
	ASSERT(!m_bAtEof);

	const int16_t *data = (const int16_t *) data_;

	const unsigned samples = bytes / sizeof(int16_t);
	const unsigned frames = samples / m_iChannels;

	if(m_iInputRate == m_iOutputRate)
	{
		/* Optimization. */
		m_OutBuf.insert(m_OutBuf.end(), data, data+samples);
		return;
	}

	/* Lerp. */
	const int FIXED_SHIFT = 14;
	const int FIXED_ONE = 1<<FIXED_SHIFT;
	int iInputSamplesPerOutputSample =
		(m_iInputRate<<FIXED_SHIFT) / m_iOutputRate;

	m_OutBuf.resize( (frames*m_iChannels*m_iOutputRate)/m_iInputRate + 10 );
	int iSize = 0;
	
	for( int c = 0; c < m_iChannels; ++c )
	{
		int iPos = m_iPos;
		const int16_t *pInBuf = &data[c];
		int16_t *pOutBuf = &m_OutBuf[c];
		int iSamplesInput = 0;
		int iSamplesOutput = 0;
		int16_t iPrevSample = m_iPrevSample[c];
		for( unsigned f = 0; f < frames; ++f )
		{
			while( iPos < FIXED_ONE )
			{
				int iSamp = iPrevSample * (FIXED_ONE-iPos) +
					pInBuf[iSamplesInput] * iPos;
				pOutBuf[iSamplesOutput] = int16_t(iSamp >> FIXED_SHIFT);
				iSamplesOutput += m_iChannels;
				iPos += iInputSamplesPerOutputSample;
			}

			iPos -= FIXED_ONE;

			iPrevSample = pInBuf[iSamplesInput];
			iSamplesInput += m_iChannels;
		}
		m_iPrevSample[c] = iPrevSample;

		if( c == m_iChannels-1 )
		{
			iSize = iSamplesOutput;
			m_iPos = iPos;
		}
	}

	m_OutBuf.erase( m_OutBuf.begin()+iSize, m_OutBuf.end() );
}


void RageSoundResampler::eof()
{
	ASSERT(!m_bAtEof);

	/* Write some silence to flush out the real data.  If we don't have any sound,
	 * don't do this, so seeking past end of file doesn't write silence. */
	bool bNeedsFlush = false;
	for( int c = 0; c < m_iChannels; ++c )
		if( m_iPrevSample[c] != 0 ) 
			bNeedsFlush = true;

	if( bNeedsFlush )
	{
		const int size = m_iChannels*16;
		int16_t *data = new int16_t[size];
		memset(data, 0, size * sizeof(int16_t));
		write(data, size * sizeof(int16_t));
		delete [] data;
	}

	m_bAtEof = true;
}


int RageSoundResampler::read(void *data, unsigned bytes)
{
	/* Don't be silly. */
	ASSERT( (bytes % sizeof(int16_t)) == 0);

	/* If no data is available, and we're m_bAtEof, return -1. */
	if(m_OutBuf.size() == 0 && m_bAtEof)
		return -1;

	/* Fill as much as we have. */
	int Avail = min(m_OutBuf.size()*sizeof(int16_t), bytes);
	memcpy(data, &m_OutBuf[0], Avail);
	m_OutBuf.erase(m_OutBuf.begin(), m_OutBuf.begin() + Avail/sizeof(int16_t));
	return Avail;
}

/*
 * Copyright (c) 2003-2005 Glenn Maynard
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
