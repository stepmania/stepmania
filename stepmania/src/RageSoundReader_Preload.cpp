/* This reader simply precaches all of the data from another reader. This
 * reduces CPU usage for sounds that are played several times at once. */

#include "global.h"
#include "RageSoundReader_Preload.h"
#include "RageUtil.h"
#include "RageSoundUtil.h"
#include "Preference.h"

/* If true, preloaded sounds are stored in 16-bit instead of floats.  Most
 * processing happens after preloading, and it's usually a waste to store high-
 * resolution data for sound effects. */
Preference<bool> g_bSoundPreload16bit( "SoundPreload16bit", true );

/* If a sound is smaller than this, we'll load it entirely into memory. */
Preference<int> g_iSoundPreloadMaxSamples( "SoundPreloadMaxSamples", 128*1024 );

#define samplesize (m_bBufferIs16Bit? sizeof(int16_t):sizeof(float))
#define framesize (samplesize * m_iChannels)

bool RageSoundReader_Preload::PreloadSound( RageSoundReader *&pSound )
{
	RageSoundReader_Preload *pPreload = new RageSoundReader_Preload;
	if( !pPreload->Open(pSound) )
	{
		/* Preload failed.  It read some data, so we need to rewind the reader. */
		pSound->SetPosition( 0 );
		delete pPreload;
		return false;
	}

	pSound = pPreload;
	return true;
}

RageSoundReader_Preload::RageSoundReader_Preload():
	m_Buffer( new RString )
{
	m_bBufferIs16Bit = g_bSoundPreload16bit.Get();
}

int RageSoundReader_Preload::GetTotalFrames() const
{
	return m_Buffer->size() / framesize;
}

bool RageSoundReader_Preload::Open( RageSoundReader *pSource )
{
	ASSERT( pSource );
	m_iSampleRate = pSource->GetSampleRate();
	m_iChannels = pSource->GetNumChannels();
	m_fRate = pSource->GetStreamToSourceRatio();

	int iMaxSamples = g_iSoundPreloadMaxSamples.Get();
	
	/* Check the length, and see if we think it'll fit in the buffer. */
	int iLen = pSource->GetLength_Fast();
	if( iLen != -1 )
	{
		float fSecs = iLen / 1000.f;

		int iFrames = lrintf( fSecs * m_iSampleRate ); /* seconds -> frames */
		int iSamples = unsigned( iFrames * m_iChannels ); /* frames -> samples */
		if( iSamples > iMaxSamples )
			return false; /* Don't bother trying to preload it. */

		int iBytes = unsigned( iSamples * samplesize ); /* samples -> bytes */
		m_Buffer.Get()->reserve( iBytes );
	}

	while(1)
	{
		/* If the rate changes, we won't preload it. */
		if( pSource->GetStreamToSourceRatio() != m_fRate )
			return false; /* Don't bother trying to preload it. */

		float buffer[1024];
		int iCnt = pSource->Read( buffer, ARRAYSIZE(buffer) / m_iChannels );

		if( iCnt == END_OF_FILE )
			break;
		if( iCnt < 0 )
			return false;

		/* Add the buffer. */
		if( m_bBufferIs16Bit )
		{
			int16_t buffer16[1024];
			RageSoundUtil::ConvertFloatToNativeInt16( buffer, buffer16, iCnt*m_iChannels );
			m_Buffer.Get()->append( (char *) buffer16, (char *) (buffer16+iCnt*m_iChannels) );
		}
		else
		{
			m_Buffer.Get()->append( (char *) buffer, (char *) (buffer+iCnt*m_iChannels) );
		}

		if( m_Buffer.Get()->size() > iMaxSamples * samplesize )
			return false; /* too big */
	}

	m_iPosition = 0;
	delete pSource;
	return true;
}

int RageSoundReader_Preload::GetLength() const
{
	return int(float(GetTotalFrames()) * 1000.f / m_iSampleRate);
}

int RageSoundReader_Preload::GetLength_Fast() const
{
	return GetLength();
}

int RageSoundReader_Preload::SetPosition( int iFrame )
{
	m_iPosition = iFrame;
	m_iPosition = lrintf(m_iPosition / m_fRate);

	if( m_iPosition >= int(m_Buffer->size() / framesize) )
	{
		m_iPosition = m_Buffer->size() / framesize;
		return 0;
	}

	return 1;
}

int RageSoundReader_Preload::GetNextSourceFrame() const
{
	return lrintf(m_iPosition * m_fRate);
}

int RageSoundReader_Preload::Read( float *pBuffer, int iFrames )
{
	const int iSizeFrames = m_Buffer->size() / framesize;
	const int iFramesAvail = iSizeFrames - m_iPosition;

	iFrames = min( iFrames, iFramesAvail );
	if( iFrames == 0 )
		return END_OF_FILE;
	if( m_bBufferIs16Bit )
	{
		const int16_t *pIn = (const int16_t *) (m_Buffer->data() + (m_iPosition * framesize));
		RageSoundUtil::ConvertNativeInt16ToFloat( pIn, pBuffer, iFrames * m_iChannels );
	}
	else
	{
		memcpy( pBuffer, m_Buffer->data() + (m_iPosition * framesize), iFrames * framesize );
	}
	m_iPosition += iFrames;
	
	return iFrames;
}

RageSoundReader_Preload *RageSoundReader_Preload::Copy() const
{
	return new RageSoundReader_Preload(*this);
}

int RageSoundReader_Preload::GetReferenceCount() const
{
	return m_Buffer.GetReferenceCount();
}

/*
 * Copyright (c) 2003 Glenn Maynard
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
