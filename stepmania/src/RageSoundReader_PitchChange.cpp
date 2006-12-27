/*
 * Implements properties: 
 *   "Speed" - cause the sound to play faster or slower
 *   "Pitch" - raise or lower the pitch of the sound
 *
 * Pitch changing is implemented by combining speed changing and rate changing
 * (via resampling).  The resampler needs to be changed later than the speed
 * changer; this class just controls parameters on the other two real filters.
 */

#include "global.h"
#include "RageSoundReader_PitchChange.h"
#include "RageSoundReader_SpeedChange.h"
#include "RageSoundReader_Resample_Good.h"
#include "RageLog.h"

RageSoundReader_PitchChange::RageSoundReader_PitchChange( RageSoundReader *pSource ):
	RageSoundReader_Filter( NULL )
{
	m_pSpeedChange = new RageSoundReader_SpeedChange( pSource );
	m_pResample = new RageSoundReader_Resample_Good( m_pSpeedChange, m_pSpeedChange->GetSampleRate() );
	m_pSource = m_pResample;
	m_fSpeedRatio = 1.0f;
	m_fPitchRatio = 1.0f;
}

RageSoundReader_PitchChange::RageSoundReader_PitchChange( const RageSoundReader_PitchChange &cpy ):
	RageSoundReader_Filter( cpy )
{
	/* The source tree has already been copied.  Our source is m_pResample; its source
	 * is m_pSpeedChange (and its source is a copy of the pSource we were initialized
	 * with). */
	m_pResample = dynamic_cast<RageSoundReader_Resample_Good *>( &*m_pSource );
	m_pSpeedChange = dynamic_cast<RageSoundReader_SpeedChange *>( m_pResample->GetSource() );
	m_fSpeedRatio = cpy.m_fSpeedRatio;
	m_fPitchRatio = cpy.m_fPitchRatio;
}

int RageSoundReader_PitchChange::Read( char *pBuf, int iFrames )
{
	/* m_pSpeedChange->NextReadWillStep is true if speed changes will be applied
	 * immediately on the next Read().  When this is true, apply the ratio to the
	 * resampler and the speed changer simultaneously, so they take effect as
	 * closely together as possible. */
	float fRate = GetStreamToSourceRatio();
	if( m_pSpeedChange->NextReadWillStep() )
	{
		m_pSpeedChange->SetSpeedRatio( m_fSpeedRatio / m_fPitchRatio );
		m_pResample->SetRate( m_fPitchRatio );
	}

	/* If we just applied a new speed and it caused the ratio to change, return
	 * no data, so the caller can see the new ratio. */
	if( fRate != GetStreamToSourceRatio() )
		return 0;

	return RageSoundReader_Filter::Read( pBuf, iFrames );
}

float RageSoundReader_PitchChange::GetStreamToSourceRatio() const
{
	/* If m_fSpeedRatio is 1.0f and the underlying ratio is exactly 1.0,
	 * the ratio should be exactly 1.  Rounding error prevents n * (1/n)
	 * from being exact. */
	float fRatio = m_pSource->GetStreamToSourceRatio();
	if( m_fSpeedRatio == 1.0f && fabsf(1.0f - fRatio) < 0.001f )
		fRatio = 1.0f;
	return fRatio;
}

bool RageSoundReader_PitchChange::SetProperty( const RString &sProperty, float fValue )
{
	if( sProperty == "Rate" )
	{
		/* Don't propagate this.  m_pResample will take it, but it's under
		 * our control. */
		return false;
	}
	if( sProperty == "Speed" )
	{
		SetSpeedRatio( fValue );
		return true;
	}

	if( sProperty == "Pitch" )
	{
		SetPitchRatio( fValue );
		return true;
	}

	return RageSoundReader_Filter::SetProperty( sProperty, fValue );
}

/*
 * Copyright (c) 2006 Glenn Maynard
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
