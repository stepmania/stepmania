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


const float TIME_BETWEEN_TICKS = 0.05f;

ScoreDisplayRolling::ScoreDisplayRolling()
{
	RageLog( "ScoreDisplayRolling::ScoreDisplayRolling()" );

	for( int i=0; i<MAX_SCORE_DIGITS; i++ )
	{
		m_textDigits[i].Load( THEME->GetPathTo(FONT_SCORE_NUMBERS) );
		m_textDigits[i].TurnShadowOff();

		// get the width of the numbers
		m_textDigits[i].SetText( "0" );
		float fCharWidth = m_textDigits[i].GetWidestLineWidthInSourcePixels();

		m_textDigits[i].SetText( " " );

		float fCharOffsetsFromCenter = i - float(MAX_SCORE_DIGITS-1)/2;

		m_textDigits[i].SetXY( fCharOffsetsFromCenter * fCharWidth, 0 );

		iCurrentScoreDigits[i] = 0;
		iDestinationScoreDigits[i] = 0;

		this->AddActor( &m_textDigits[i] );
	}

	m_fTimeUntilNextTick = 0;

	SetScore( 0 );
}


void ScoreDisplayRolling::SetScore( float fNewScore ) 
{ 
	// super inefficient (but isn't called very often)
	CString sFormatString = ssprintf( "%%%d.0f", MAX_SCORE_DIGITS );
	CString sScore = ssprintf( sFormatString, fNewScore );

	for( int i=0; i<MAX_SCORE_DIGITS; i++ )
	{
		iDestinationScoreDigits[i] = atoi( sScore.Mid(i,1) );
	}

}


void ScoreDisplayRolling::Update( float fDeltaTime )
{
	ActorFrame::Update( fDeltaTime );

	m_fTimeUntilNextTick -= fDeltaTime;

	while( m_fTimeUntilNextTick <= 0 )
	{
		m_fTimeUntilNextTick += TIME_BETWEEN_TICKS;

		// do a tick
		for( int i=0; i<MAX_SCORE_DIGITS; i++ )
		{
			if( iCurrentScoreDigits[i] != iDestinationScoreDigits[i] )
			{
				iCurrentScoreDigits[i]++;
				if( iCurrentScoreDigits[i] > 9 )
					iCurrentScoreDigits[i] = 0;

				m_textDigits[i].SetText( ssprintf("%d", iCurrentScoreDigits[i]) );
			}
		}
	}
}

