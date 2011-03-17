#include "global.h"
#include "SampleHistory.h"
#include "RageLog.h"
#include "RageUtil.h"
#include "Foreach.h"

SampleHistory::SampleHistory()
{
	m_iLastHistory = 0;
	m_iHistorySamplesPerSecond = 60;
	m_fHistorySeconds = 0.0f;
	m_fToSample = 1 / m_iHistorySamplesPerSecond;

	m_fHistorySeconds = 10.0f;
	int iSamples = lrintf( m_iHistorySamplesPerSecond * m_fHistorySeconds );
	m_afHistory.resize( iSamples );
}

float SampleHistory::GetSampleNum( float fSamplesAgo ) const
{
	fSamplesAgo = min( fSamplesAgo, (float) m_afHistory.size() - 1 );
	if( fSamplesAgo < 0 )
		fSamplesAgo = 0;
	if( m_afHistory.size() == 0 )
		return 0.0f;

	float fSample = m_iLastHistory - fSamplesAgo - 1;

	float f = floorf( fSample );
	int iSample = lrintf(f);
	int iNextSample = iSample + 1;
	wrap( iSample, m_afHistory.size() );
	wrap( iNextSample, m_afHistory.size() );

	float p = fSample - f;
	float fRet = lerp( p, m_afHistory[iSample], m_afHistory[iNextSample] );
//	LOG->Trace( "%.3f: %i, %i, %.3f (f %.3f, %.3f)", fSample, iSample, iNextSample, fRet, f, p );
	return fRet;
}

float SampleHistory::GetSample( float fSecondsAgo ) const
{
	float fSamplesAgo = fSecondsAgo * m_iHistorySamplesPerSecond;
	return GetSampleNum( fSamplesAgo );
}

void SampleHistory::AddSample( float fSample, float fDeltaTime )
{
	while( fDeltaTime > 0.0001f )
	{
		float fTime = min( m_fToSample, fDeltaTime );
		m_fToSample -= fTime;
		fDeltaTime -= fTime;

		if( m_fToSample < 0.0001f )
		{
			++m_iLastHistory;
			m_iLastHistory %= m_afHistory.size();
			m_fToSample += 1.0f / m_iHistorySamplesPerSecond;
		}

		m_afHistory[m_iLastHistory] = fSample;
	}
}

/*
 * (c) 2006-2007 Glenn Maynard
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
