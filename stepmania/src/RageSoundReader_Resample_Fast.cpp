#include "global.h"
#include "RageSoundReader_Resample_Fast.h"

RageSoundReader_Resample_Fast::RageSoundReader_Resample_Fast()
{
	m_pSource = NULL;
	m_iOutputSampleRate = -1;
}

void RageSoundReader_Resample_Fast::Open( SoundReader *pSource )
{
	m_pSource = pSource;
	ASSERT(m_pSource);

	m_iOutputSampleRate = m_pSource->GetSampleRate();
	m_Resamp.SetInputSampleRate( m_iOutputSampleRate );
	m_Resamp.SetChannels( m_pSource->GetNumChannels() );
}


RageSoundReader_Resample_Fast::~RageSoundReader_Resample_Fast()
{
	delete m_pSource;
}

void RageSoundReader_Resample_Fast::SetSampleRate( int hz )
{
	m_iOutputSampleRate = hz;
	m_Resamp.SetOutputSampleRate( m_iOutputSampleRate );
}

int RageSoundReader_Resample_Fast::GetLength() const
{
	m_Resamp.reset();
	return m_pSource->GetLength();
}

int RageSoundReader_Resample_Fast::GetLength_Fast() const
{
	m_Resamp.reset();
	return m_pSource->GetLength_Fast();
}

int RageSoundReader_Resample_Fast::SetPosition_Accurate( int ms )
{
	m_Resamp.reset();
	return m_pSource->SetPosition_Accurate(ms);
}

int RageSoundReader_Resample_Fast::SetPosition_Fast( int ms )
{
	m_Resamp.reset();
	return m_pSource->SetPosition_Fast(ms);
}
static const int BUFSIZE = 1024*16;

int RageSoundReader_Resample_Fast::Read( char *pBuf, unsigned iSize )
{
	int iBytesRead = 0;
	while( iSize )
	{
		{
			int iGot = m_Resamp.read( pBuf, iSize );

			if( iGot == -1 )
				break;

			pBuf += iGot;
			iSize -= iGot;
			iBytesRead += iGot;

			if( iGot )
				continue;
		}

		{
			static char buf[BUFSIZE];
			int iGot = m_pSource->Read(buf, sizeof(buf));

			if( iGot == -1 )
			{
				SetError( m_pSource->GetError() );
				return -1;
			}
			if( iGot == 0 )
				m_Resamp.eof();
			else
				m_Resamp.write( buf, iGot );
		}
	}

	return iBytesRead;
}

SoundReader *RageSoundReader_Resample_Fast::Copy() const
{
	SoundReader *pNewSource = m_pSource->Copy();
	RageSoundReader_Resample_Fast *pRet = new RageSoundReader_Resample_Fast;
	pRet->Open( pNewSource );
	pRet->SetSampleRate( m_iOutputSampleRate );
	return pRet;
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

