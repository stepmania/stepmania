#include "global.h"

#include "PercentageDisplay.h"
#include "GameState.h"
#include "ThemeManager.h"
#include "PrefsManager.h"
#include "ActorUtil.h"
#include "RageLog.h"
#include "StageStats.h"

#define DANCE_POINT_DIGITS			THEME->GetMetricI(m_sName,"DancePointsDigits")
#define PERCENT_DECIMAL_PLACES		THEME->GetMetricI(m_sName,"PercentDecimalPlaces")
#define PERCENT_TOTAL_SIZE			THEME->GetMetricI(m_sName,"PercentTotalSize")
#define PERCENT_USE_REMAINDER		THEME->GetMetricB(m_sName,"PercentUseRemainder")

PercentageDisplay::PercentageDisplay()
{
	m_pSource = NULL;
}

void PercentageDisplay::Load( PlayerNumber pn, StageStats* pSource, bool bAutoRefresh )
{
	ASSERT( m_sName != "" ); // set this!
	m_PlayerNumber = pn;
	m_pSource = pSource;
	m_bAutoRefresh = bAutoRefresh;
	m_Last = -1;

	if( PREFSMAN->m_bDancePointsForOni )
		m_textPercent.SetName( ssprintf("DancePointsP%i", pn+1) );
	else
		m_textPercent.SetName( ssprintf("PercentP%i", pn+1) );

	m_textPercent.LoadFromFont( THEME->GetPathF(m_sName,"text") );
	SET_XY_AND_ON_COMMAND( m_textPercent );
	this->AddChild( &m_textPercent );

	if( !PREFSMAN->m_bDancePointsForOni && PERCENT_USE_REMAINDER )
	{
		m_textPercentRemainder.SetName( ssprintf("PercentRemainderP%d",pn+1) );
		m_textPercentRemainder.LoadFromFont( THEME->GetPathF(m_sName,"remainder") );
		SET_XY_AND_ON_COMMAND( m_textPercentRemainder );
		m_textPercentRemainder.SetText( "456" );
		this->AddChild( &m_textPercentRemainder );
	}

	Refresh();
}

void PercentageDisplay::TweenOffScreen()
{
	OFF_COMMAND( m_textPercent );
	if( !PREFSMAN->m_bDancePointsForOni && PERCENT_USE_REMAINDER )
		OFF_COMMAND( m_textPercentRemainder );
}

void PercentageDisplay::Update( float fDeltaTime )
{
	ActorFrame::Update( fDeltaTime );

	if( m_bAutoRefresh )
		Refresh();
}

void PercentageDisplay::Refresh()
{
	const int iActualDancePoints = m_pSource->iActualDancePoints[m_PlayerNumber];
	if( iActualDancePoints == m_Last )
		return;

	m_Last = iActualDancePoints;

	CString sNumToDisplay;
	if( !PREFSMAN->m_bDancePointsForOni )
	{
		float fPercentDancePoints = m_pSource->GetPercentDancePoints( m_PlayerNumber );

		// clamp percentage - feedback is that negative numbers look weird here.
		CLAMP( fPercentDancePoints, 0.f, 1.f );

		if( PERCENT_USE_REMAINDER )
		{
			int iPercentWhole = int(fPercentDancePoints*100);
			int iPercentRemainder = int( (fPercentDancePoints*100 - int(fPercentDancePoints*100)) * 10 );
			sNumToDisplay = ssprintf( "%2d", iPercentWhole );
			m_textPercentRemainder.SetText( ssprintf(".%01d%%", iPercentRemainder) );
		}
		else
		{
			float fNumToDisplay = fPercentDancePoints*100;
			sNumToDisplay = ssprintf( "%*.*f%%", PERCENT_TOTAL_SIZE, PERCENT_DECIMAL_PLACES, fNumToDisplay );
			
			// HACK: Use the last frame in the numbers texture as '-'
			sNumToDisplay.Replace('-','x');
		}
	}
	else
		sNumToDisplay = ssprintf( "%*d", DANCE_POINT_DIGITS, max( 0, iActualDancePoints ) );

	m_textPercent.SetText( sNumToDisplay );
}

/*
 * (c) 2001-2003 Chris Danford
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
