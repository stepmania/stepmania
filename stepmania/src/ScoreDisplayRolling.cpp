#include "stdafx.h"
/*
-----------------------------------------------------------------------------
 File: ScoreDisplayRolling.h

 Desc: A graphic displayed in the ScoreDisplayRolling during Dancing.

 Copyright (c) 2001 Chris Danford.  All rights reserved.
-----------------------------------------------------------------------------
*/

#include "ScoreDisplayRolling.h"
#include "RageUtil.h"
#include "ThemeManager.h"


ScoreDisplayRolling::ScoreDisplayRolling()
{
	RageLog( "ScoreDisplayRolling::ScoreDisplayRolling()" );

	// init the text
	Load( THEME->GetPathTo(FONT_SCORE_NUMBERS) );
	TurnShadowOff();

	// init the digits
	for( int i=0; i<NUM_SCORE_DIGITS; i++ )
	{
		m_iCurrentScoreDigits[i] = 0;
		m_iDestinationScoreDigits[i] = 0;
	}

	SetScore( 0 );
}


void ScoreDisplayRolling::SetScore( float fNewScore ) 
{ 
	// super inefficient (but isn't called very often)
	CString sFormatString = ssprintf( "%%%d.0f", NUM_SCORE_DIGITS );
	CString sScore = ssprintf( sFormatString, fNewScore );

	for( int i=0; i<NUM_SCORE_DIGITS; i++ )
	{
		m_iDestinationScoreDigits[i] = atoi( sScore.Mid(i,1) );
	}

}


void ScoreDisplayRolling::Update( float fDeltaTime )
{
	BitmapText::Update( fDeltaTime );
}

void ScoreDisplayRolling::Draw()
{
	// find the leftmost current digit that doesn't match the destination digit
	for( int i=0; i<NUM_SCORE_DIGITS; i++ )
	{
		if( m_iCurrentScoreDigits[i] != m_iDestinationScoreDigits[i] )
			break;
	}

	// and increment that digit and everything after
	for( ; i<NUM_SCORE_DIGITS; i++ )
	{
		m_iCurrentScoreDigits[i]++;
		if( m_iCurrentScoreDigits[i] > 9 )
			m_iCurrentScoreDigits[i] = 0;
	}

	int iCurScore = 0;
	int iMultiplier = 1;
	for( int d=NUM_SCORE_DIGITS-1; d>=0; d-- )		// foreach digit
	{
		iCurScore += m_iCurrentScoreDigits[d] * iMultiplier;
		iMultiplier *= 10;
	}

	CString sFormat = ssprintf( "%%%d.0d", NUM_SCORE_DIGITS );
	SetText( ssprintf(sFormat, iCurScore) );

	BitmapText::Draw();
}