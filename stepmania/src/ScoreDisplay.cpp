#include "stdafx.h"
/*
-----------------------------------------------------------------------------
 File: ScoreDisplay.h

 Desc: A graphic displayed in the ScoreDisplay during Dancing.

 Copyright (c) 2001 Chris Danford.  All rights reserved.
-----------------------------------------------------------------------------
*/

#include "ScoreDisplay.h"
#include "RageUtil.h"
#include "ThemeManager.h"




ScoreDisplay::ScoreDisplay()
{
	RageLog( "ScoreDisplay::ScoreDisplay()" );

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

		this->AddActor( &m_textDigits[i] );
	}

	SetScore( 0 );
}


void ScoreDisplay::SetScore( float fNewScore ) 
{ 
	CString sFormatString = ssprintf( "%%%d.0f", MAX_SCORE_DIGITS );
	CString sScore = ssprintf( sFormatString, fNewScore );

	for( int i=0; i<MAX_SCORE_DIGITS; i++ )
	{
		m_textDigits[i].SetText( sScore[i] );

		m_textDigits[i].StopTweening();
		m_textDigits[i].SetZoomX( 0 );
		m_textDigits[i].BeginTweeningQueued( 0.2f * (MAX_SCORE_DIGITS-i) / MAX_SCORE_DIGITS + 0.1f );
		// do nothing - this tween is a wait
		m_textDigits[i].BeginTweeningQueued( 0.2f );
		m_textDigits[i].SetTweenZoomX( 1 );
	}

}

