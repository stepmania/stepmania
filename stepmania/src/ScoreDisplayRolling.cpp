#include "stdafx.h"
/*
-----------------------------------------------------------------------------
 File: ScoreDisplayRolling.h

 Desc: A graphic displayed in the ScoreDisplayRolling during Dancing.

 Copyright (c) 2001-2002 by the persons listed below.  All rights reserved.
-----------------------------------------------------------------------------
*/

#include "ScoreDisplayRolling.h"
#include "RageUtil.h"
#include "RageLog.h"
#include "ThemeManager.h"


ScoreDisplayRolling::ScoreDisplayRolling()
{
	LOG->WriteLine( "ScoreDisplayRolling::ScoreDisplayRolling()" );

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


void ScoreDisplayRolling::SetScore( int iNewScore ) 
{ 
	SetScore( (float)iNewScore );
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



void ScoreDisplayRolling::AddToScore( TapNoteScore score, int iCurCombo )
{
	// The scoring system for DDR versions 1 and 2 (including the Plus remixes) is as follows: 
	// For every step: 
	//
	// Multiplier (M) = (# of NoteMetadata in your current combo / 4) rounded down 
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
	switch( score )
	{
	case TNS_MISS:											break;
	case TNS_BOO:											break;
	case TNS_GOOD:		fScoreToAdd =     M * 100 + 100;	break;
	case TNS_GREAT:		fScoreToAdd = M * M * 100 + 300;	break;
	case TNS_PERFECT:	fScoreToAdd = M * M * 300 + 500;	break;
	}
	m_fScore += fScoreToAdd;
	ASSERT( m_fScore >= 0 );

	
 /* Score implementation by Chris Gomez, February 6th, 2002  The 
 scoring system for 5th mix is much more complicated than the original 
 scoring system.  We don't have bonuses yet, so they're not included in 
 this code.  Max score is (feet + 1) * 5,000,000.  Scores are clamped 
 to integers; this is fixed on the last step (if it's perfect)

  The base step value is the max score divided by 10, divided by 
 one-half of the max combo  squared plus the max combo. This value is 
 multiplied by 10 for a perfect or 5 for a great,  and further 
 multiplied by the number of NoteMetadata currently in the combo. */ 
/*
	int iScoreToAdd = 0; int iBonus = 0;

 NoteMetadata *pSteps = GAMEINFO->m_pStepsPlayer[m_PlayerNumber];
 int iNumFeet = pSteps->m_iNumFeet;
 int iMaxCombo = pSteps->GetNumSteps();

 int iScoreMax = (iNumFeet + 1) * 5000000; //magic numbers. int 
 iBaseScore = (iScoreMax / 10) / ((iMaxCombo * (iMaxCombo + 1)) / 2);

 iCurCombo++; // looks like the current combo counter starts at 0 when 
 iCurCombo++it should start at 1
     // this was messing with the scoring, so it had to be fixed.

 switch (score)
 {
 case miss:
 case boo:
 case good:
  break;
 case great:
  iScoreToAdd = 5 * iBaseScore * iCurCombo;
  break;
 case perfect:
  iScoreToAdd = 10 * iBaseScore * iCurCombo;
  if(BeatToStepIndex(pSteps->GetLastBeat()) == iMaxCombo)
   for(int i = 1; i <= iMaxCombo; i++)
    iBonus += 10 * iBaseScore * i;
  iScoreToAdd += iBonus;
  break;
 }
*/
	this->SetScore( m_fScore );
}
