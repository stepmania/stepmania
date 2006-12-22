#include "global.h"
#include "RageSoundReader_Extend.h"
#include "RageLog.h"
#include "RageSoundUtil.h"
#include "RageUtil.h"

/*
 * Add support for negative seeks (adding a delay), extending a sound
 * beyond its end (m_LengthSeconds and M_CONTINUE), looping and fading.
 */

RageSoundReader_Extend::RageSoundReader_Extend( RageSoundReader *pSource ):
	RageSoundReader_Filter( pSource )
{
	m_iPositionFrames = pSource->GetNextSourceFrame();

	m_StopMode = M_STOP;
	m_iStartFrames = 0;
	m_iLengthFrames = -1;
	m_iFadeFrames = 0;
}

int RageSoundReader_Extend::SetPosition( int iFrame )
{
	m_iPositionFrames = iFrame;
	int iRet = m_pSource->SetPosition( max(iFrame, 0) );
	if( iRet < 0 )
		return iRet;

	if( m_iLengthFrames != -1 )
		return m_iPositionFrames < GetEndFrame();

	/* If we're in CONTINUE and we seek past the end of the file, don't return EOF. */
	if( m_StopMode == M_CONTINUE )
		return 1;

	return iRet;
}

int RageSoundReader_Extend::GetEndFrame() const
{
	if( m_iLengthFrames == -1 )
		return -1;

	return m_iStartFrames + m_iLengthFrames;
}

int RageSoundReader_Extend::GetData( char *pBuffer, int iFrames )
{
	int iFramesToRead = iFrames;
	if( m_iLengthFrames != -1 )
		iFramesToRead = min( iFramesToRead, GetEndFrame() - m_iPositionFrames );
	if( iFrames && !iFramesToRead )
		return RageSoundReader::END_OF_FILE;

	if( m_iPositionFrames < 0 )
	{
		iFramesToRead = min( iFramesToRead, -m_iPositionFrames );

		m_iPositionFrames += iFramesToRead;
		memset( pBuffer, 0, iFramesToRead * sizeof(int16_t) * this->GetNumChannels() );
		return iFramesToRead;
	}

	int iNewPositionFrames = m_pSource->GetNextSourceFrame();
	int iRet = RageSoundReader_Filter::Read( pBuffer, iFramesToRead );

	/* Update the position from the source.  If the source is at EOF, skip this,
	 * so we'll extrapolate in M_CONTINUE. */
	if( iRet != RageSoundReader::END_OF_FILE )
		m_iPositionFrames = iNewPositionFrames;
	return iRet;
}

int RageSoundReader_Extend::Read( char *pBuffer, int iFrames )
{
	int iFramesRead = GetData( pBuffer, iFrames );
	if( iFramesRead == RageSoundReader::END_OF_FILE )
	{
		if( (m_iLengthFrames != -1 && m_iPositionFrames < GetEndFrame()) ||
			m_StopMode == M_CONTINUE )
		{
			iFramesRead = iFrames;
			if( m_StopMode != M_CONTINUE )
				iFramesRead = min( GetEndFrame() - m_iPositionFrames, iFramesRead );
			memset( pBuffer, 0, iFramesRead * sizeof(int16_t) * this->GetNumChannels() );
		}
	}

	if( iFramesRead > 0 )
	{
		/* We want to fade when there's m_FadeLength seconds left, but if
		 * m_LengthFrames is -1, we don't know the length we're playing.
		 * (m_LengthFrames is the length to play, not the length of the
		 * source.)  If we don't know the length, don't fade. */
		if( m_iFadeFrames != 0 && m_iLengthFrames != -1 )
		{
			const int iFinishFadingOutAt = GetEndFrame();
			const int iStartFadingOutAt = iFinishFadingOutAt - m_iFadeFrames;
			const int iStartSecond = m_iPositionFrames;
			const int iEndSecond = m_iPositionFrames + iFramesRead;
			const float fStartVolume = SCALE( iStartSecond, iStartFadingOutAt, iFinishFadingOutAt, 1.0f, 0.0f );
			const float fEndVolume = SCALE( iEndSecond, iStartFadingOutAt, iFinishFadingOutAt, 1.0f, 0.0f );
			RageSoundUtil::Fade( (int16_t *) pBuffer, iFramesRead, fStartVolume, fEndVolume );
		}

		m_iPositionFrames += iFramesRead;
	}

	if( iFramesRead == RageSoundReader::END_OF_FILE && m_StopMode == M_LOOP )
	{
		this->SetPosition( m_iStartFrames );
		return STREAM_LOOPED;
	}

	if( iFramesRead == RageSoundReader::ERROR )
		this->SetError( m_pSource->GetError() );

	return iFramesRead;
}

int RageSoundReader_Extend::GetNextSourceFrame() const
{
	return m_iPositionFrames;
}

bool RageSoundReader_Extend::SetProperty( const RString &sProperty, float fValue )
{
	if( sProperty == "StartSecond" )
	{
		m_iStartFrames = lrintf( fValue * this->GetSampleRate() );
		return true;
	}

	if( sProperty == "LengthSeconds" )
	{
		if( fValue == -1 )
			m_iLengthFrames = -1;
		else
			m_iLengthFrames = lrintf( fValue * this->GetSampleRate() );
		return true;
	}

	if( sProperty == "Loop" )
	{
		m_StopMode = M_LOOP;
		return true;
	}

	if( sProperty == "Stop" )
	{
		m_StopMode = M_STOP;
		return true;
	}

	if( sProperty == "Continue" )
	{
		m_StopMode = M_CONTINUE;
		return true;
	}

	if( sProperty == "FadeSeconds" )
	{
		m_iFadeFrames = lrintf( fValue * this->GetSampleRate() );
		return true;
	}

	return false;
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
