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

PercentageDisplay::PercentageDisplay()
{
	this->AddChild( &m_textPercent );
}

void PercentageDisplay::Load( PlayerNumber pn )
{
	m_PlayerNumber = pn;
	m_Last = 0;

	m_textPercent.SetName( ssprintf("PercentP%i", pn+1) );
	m_textPercent.LoadFromNumbers( THEME->GetPathToN(m_sName + " text") );
	SET_XY_AND_ON_COMMAND( m_textPercent );

	if( PREFSMAN->m_bDancePointsForOni )
		m_textPercent.SetText( "     " );
	else
		m_textPercent.SetText( ssprintf( "%0*.*f%%", PERCENT_TOTAL_SIZE, PERCENT_DECIMAL_PLACES, 0 ) );
}

void PercentageDisplay::Update( float fDeltaTime )
{
	const int iActualDancePoints = GAMESTATE->m_CurStageStats.iActualDancePoints[m_PlayerNumber];
	if( iActualDancePoints == m_Last )
		return;
	m_Last = iActualDancePoints;

	CString sNumToDisplay;
	if( !PREFSMAN->m_bDancePointsForOni )
	{
		int iPossibleDancePoints = GAMESTATE->m_CurStageStats.iPossibleDancePoints[m_PlayerNumber];
		iPossibleDancePoints = max( 1, iPossibleDancePoints );
		float fPercentDancePoints =  iActualDancePoints / (float)iPossibleDancePoints + 0.00001f;	// correct for rounding errors

		float fNumToDisplay = max( 0, fPercentDancePoints*100 );
		sNumToDisplay = ssprintf( "%0*.*f%%", PERCENT_TOTAL_SIZE, PERCENT_DECIMAL_PLACES, fNumToDisplay );
	}
	else
		sNumToDisplay = ssprintf( "%*d", DANCE_POINT_DIGITS, iActualDancePoints );

	m_textPercent.SetText( sNumToDisplay );
}

/*
 * Copyright (c) 2001-2003 by the person(s) listed below.  All rights reserved.
 *  Chris Danford
 */
