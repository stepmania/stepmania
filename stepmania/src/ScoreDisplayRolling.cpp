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
	m_fScore = fNewScore;

	// super inefficient (but isn't called very often)
	CString sFormatString = ssprintf( "%%%d.0f", NUM_SCORE_DIGITS );
	CString sScore = ssprintf( sFormatString, fNewScore );

	for( int i=0; i<NUM_SCORE_DIGITS; i++ )
	{
		m_iDestinationScoreDigits[i] = atoi( sScore.Mid(i,1) );
	}

}


float ScoreDisplayRolling::GetScore() 
{ 
	return m_fScore;
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



void ScoreDisplayRolling::AddToScore( TapStepScore stepscore, int iCurCombo )
{
	// The scoring system for DDR versions 1 and 2 (including the Plus remixes) is as follows: 
	// For every step: 
	//
	// Multiplier (M) = (# of steps in your current combo / 4) rounded down 
	// "Good" step = M * 100 (and this ends your combo)
	// "Great" step = M * M * 100 
	// "Perfect" step = M * M * 300 
	// 
	// e.g. When you get a 259 combo, the 260th step will earn you:
	// 
	// M = (260 / 4) rounded down 
	// = 65 
	//  step = M x M X 100 
	// = 65 x 65 x 100 
	// = 422,500 
	// Perfect step = Great step score x 3 
	// = 422,500 x 3 
	// = 1,267,500

	float M = iCurCombo/4.0f;

	float fScoreToAdd = 0;
	switch( stepscore )
	{
	case TSS_MISS:											break;
	case TSS_BOO:											break;
	case TSS_GOOD:		fScoreToAdd =     M * 100 + 100;	break;
	case TSS_GREAT:		fScoreToAdd = M * M * 100 + 300;	break;
	case TSS_PERFECT:	fScoreToAdd = M * M * 300 + 500;	break;
	}
	m_fScore += fScoreToAdd;
	ASSERT( m_fScore >= 0 );

	
	this->SetScore( m_fScore );
}
