#include "global.h"

#include "PercentageDisplay.h"
#include "GameState.h"
#include "ThemeManager.h"
#include "PrefsManager.h"
#include "ActorUtil.h"

#include "RageLog.h"
#define DANCE_POINT_DIGITS			THEME->GetMetricI(m_sName,"DancePointsDigits")
#define PERCENT_DECIMAL_PLACES		THEME->GetMetricI(m_sName,"PercentDecimalPlaces")
#define PERCENT_TOTAL_SIZE			THEME->GetMetricI(m_sName,"PercentTotalSize")
#define PERCENT_USE_REMAINDER		THEME->GetMetricB(m_sName,"PercentUseRemainder")

PercentageDisplay::PercentageDisplay()
{
}

void PercentageDisplay::Load( PlayerNumber pn )
{
	m_PlayerNumber = pn;
	m_Last = -1;

	if( PREFSMAN->m_bDancePointsForOni )
		m_textPercent.SetName( ssprintf("DancePointsP%i", pn+1) );
	else
		m_textPercent.SetName( ssprintf("PercentP%i", pn+1) );

	m_textPercent.LoadFromNumbers( THEME->GetPathToN(m_sName + " text") );
	SET_XY_AND_ON_COMMAND( m_textPercent );
	this->AddChild( &m_textPercent );

	if( !PREFSMAN->m_bDancePointsForOni && PERCENT_USE_REMAINDER )
	{
		m_textPercentRemainder.SetName( ssprintf("PercentRemainderP%d",pn+1) );
		m_textPercentRemainder.LoadFromNumbers( THEME->GetPathToN(m_sName + " remainder") );
		SET_XY_AND_ON_COMMAND( m_textPercentRemainder );
		m_textPercentRemainder.SetText( "456" );
		this->AddChild( &m_textPercentRemainder );
	}

	Refresh();
}

void PercentageDisplay::TweenOffScreen()
{
	OFF_COMMAND( m_textPercent );
	if( PREFSMAN->m_bDancePointsForOni && PERCENT_USE_REMAINDER )
		OFF_COMMAND( m_textPercentRemainder );
}

void PercentageDisplay::Update( float fDeltaTime )
{
	ActorFrame::Update( fDeltaTime );

	Refresh();
}

void PercentageDisplay::Refresh()
{
	const int iActualDancePoints = GAMESTATE->m_CurStageStats.iActualDancePoints[m_PlayerNumber];
	if( iActualDancePoints == m_Last )
		return;

	m_Last = iActualDancePoints;

	CString sNumToDisplay;
	if( !PREFSMAN->m_bDancePointsForOni )
	{
		int iPossibleDancePoints = GAMESTATE->m_CurStageStats.iPossibleDancePoints[m_PlayerNumber];
		float fPercentDancePoints =  iActualDancePoints / (float)iPossibleDancePoints;
		if( iActualDancePoints == iPossibleDancePoints )
			fPercentDancePoints = 1;	// correct for rounding error
		fPercentDancePoints = clamp( fPercentDancePoints, 0, 1 );

		if( PERCENT_USE_REMAINDER )
		{
			int iPercentWhole = int(fPercentDancePoints*100);
			int iPercentRemainder = int( (fPercentDancePoints*100 - int(fPercentDancePoints*100)) * 10 );
			sNumToDisplay = ssprintf( "%02d", iPercentWhole );
			m_textPercentRemainder.SetText( ssprintf(".%01d%%", iPercentRemainder) );
		} else {
			float fNumToDisplay = max( 0, fPercentDancePoints*100 );
			sNumToDisplay = ssprintf( "%0*.*f%%", PERCENT_TOTAL_SIZE, PERCENT_DECIMAL_PLACES, fNumToDisplay );
		}
	}
	else
		sNumToDisplay = ssprintf( "%*d", DANCE_POINT_DIGITS, max( 0, iActualDancePoints ) );

	m_textPercent.SetText( sNumToDisplay );
}

/*
 * Copyright (c) 2001-2003 by the person(s) listed below.  All rights reserved.
 *  Chris Danford
 */
