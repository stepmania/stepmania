#include "global.h"
#include "RageSoundReader_Resample_Fast.h"

RageSoundReader_Resample_Fast::RageSoundReader_Resample_Fast()
{
	m_pSource = NULL;
	m_iInputRate = -1;
	m_iOutputRate = -1;
	m_iChannels = 2;
	Reset();
}

void RageSoundReader_Resample_Fast::Reset()
{
	m_bAtEof = false;
	memset( m_iPrevSample, 0, sizeof(m_iPrevSample) );
	m_iPos = 0;
	m_OutBuf.clear();
}

void RageSoundReader_Resample_Fast::Open( SoundReader *pSource )
{
	m_pSource = pSource;
	ASSERT(m_pSource);

	m_iInputRate = m_iOutputRate = m_pSource->GetSampleRate();
	ASSERT( m_pSource->GetNumChannels() < MAX_CHANNELS );
	m_iChannels = m_pSource->GetNumChannels();
}


RageSoundReader_Resample_Fast::~RageSoundReader_Resample_Fast()
{
	delete m_pSource;
}

void RageSoundReader_Resample_Fast::SetSampleRate( int iRate )
{
	m_iOutputRate = iRate;
}

int RageSoundReader_Resample_Fast::GetLength() const
{
	return m_pSource->GetLength();
}

int RageSoundReader_Resample_Fast::GetLength_Fast() const
{
	return m_pSource->GetLength_Fast();
}

int RageSoundReader_Resample_Fast::SetPosition_Accurate( int ms )
{
	Reset();
	return m_pSource->SetPosition_Accurate(ms);
}

int RageSoundReader_Resample_Fast::SetPosition_Fast( int ms )
{
	Reset();
	return m_pSource->SetPosition_Fast(ms);
}
static const int BUFSIZE = 1024*16;

int RageSoundReader_Resample_Fast::Read( char *pBuf, unsigned iSize )
{
	int iBytesRead = 0;
	while( iSize )
	{
		{
			ASSERT( (iSize % sizeof(int16_t)) == 0 );

			/* If no data is available, and we're m_bAtEof, stop. */
			if( m_OutBuf.size() == 0 && m_bAtEof )
				break;

			/* Fill as much as we have. */
			int iGot = min( m_OutBuf.size()*sizeof(int16_t), size_t(iSize) );
			memcpy( pBuf, &m_OutBuf[0], iGot );
			m_OutBuf.erase( m_OutBuf.begin(), m_OutBuf.begin() + iGot/sizeof(int16_t) );

			pBuf += iGot;
			iSize -= iGot;
			iBytesRead += iGot;

			if( iGot )
				continue;
		}

		{
			static char buf[BUFSIZE];
			int iGot = m_pSource->Read( buf, sizeof(buf) );

			if( iGot == -1 )
			{
				SetError( m_pSource->GetError() );
				return -1;
			}
			if( iGot == 0 )
			{
				ASSERT( !m_bAtEof );

				/* Write some silence to flush out the real data.  If we don't have any sound,
				 * don't do this, so seeking past end of file doesn't write silence. */
				bool bNeedsFlush = false;
				for( int c = 0; c < m_iChannels; ++c )
					if( m_iPrevSample[c] != 0 ) 
						bNeedsFlush = true;

				if( bNeedsFlush )
				{
					const int iSize = m_iChannels*16;
					int16_t *pData = new int16_t[iSize];
					memset( pData, 0, iSize * sizeof(int16_t) );
					WriteSamples( pData, iSize * sizeof(int16_t) );
					delete[] pData;
				}

				m_bAtEof = true;
			}
			else
				WriteSamples( buf, iGot );
		}
	}

	return iBytesRead;
}

SoundReader *RageSoundReader_Resample_Fast::Copy() const
{
	SoundReader *pNewSource = m_pSource->Copy();
	RageSoundReader_Resample_Fast *pRet = new RageSoundReader_Resample_Fast;
	pRet->Open( pNewSource );
	pRet->SetSampleRate( m_iOutputRate );
	return pRet;
}



/* Write data to be converted. */
void RageSoundReader_Resample_Fast::WriteSamples(const void *data_, int bytes)
{
	ASSERT(!m_bAtEof);

	const int16_t *data = (const int16_t *) data_;

	const unsigned samples = bytes / sizeof(int16_t);
	const unsigned frames = samples / m_iChannels;

	if( m_iInputRate == m_iOutputRate )
	{
		/* Optimization: */
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


/*
 * Copyright (c) 2003-2006 Glenn Maynard
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

