#include "stdafx.h"
/*
-----------------------------------------------------------------------------
 File: ScoreDisplayRolling.h

 Desc: A graphic displayed in the ScoreDisplayRolling during Dancing.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "ScoreDisplayRolling.h"
#include "RageUtil.h"
#include "RageLog.h"
#include "ThemeManager.h"
#include "RageLog.h"


const float SCORE_TWEEN_TIME = 0.5f;


ScoreDisplayRolling::ScoreDisplayRolling()
{
	LOG->WriteLine( "ScoreDisplayRolling::ScoreDisplayRolling()" );

	// init the text
	Load( THEME->GetPathTo(FONT_SCORE_NUMBERS) );
	TurnShadowOff();

	m_fTrailingScore = 0;

	SetScore( 0 );
}


void ScoreDisplayRolling::Init( PlayerNumber pn, PlayerOptions po, int iOriginalNumNotes, int iNotesMeter )
{
	m_PlayerNumber = pn;
	m_PlayerOptions = po;
	m_iTotalNotes = iOriginalNumNotes;
	m_iNotesMeter = iNotesMeter;
}

void ScoreDisplayRolling::SetScore( int iNewScore ) 
{ 
	m_iScore = iNewScore;

	float fDelta = m_iScore - m_fTrailingScore;

	m_fScoreVelocity = fDelta / SCORE_TWEEN_TIME;	// in score units per second
}


int ScoreDisplayRolling::GetScore() 
{ 
	return m_iScore;
}


void ScoreDisplayRolling::Update( float fDeltaTime )
{
	BitmapText::Update( fDeltaTime );

	float fDeltaBefore = m_iScore - m_fTrailingScore;
	m_fTrailingScore += m_fScoreVelocity * fDeltaTime;
	float fDeltaAfter = m_iScore - m_fTrailingScore;

	if( fDeltaBefore * fDeltaAfter < 0 )	// the sign changed
	{
		m_fTrailingScore = (float)m_iScore;
		m_fScoreVelocity = 0;
	}

}

void ScoreDisplayRolling::Draw()
{
	if( m_iScore == 0 )
	{
		CString sFormat = ssprintf( "%%%d.0d", NUM_SCORE_DIGITS );
		SetText( ssprintf(sFormat, 0) );
	}
	else
	{
		CString sFormat = ssprintf( "%%%d.0f", NUM_SCORE_DIGITS );
		SetText( ssprintf(sFormat, m_fTrailingScore) );
	}

	BitmapText::Draw();
}


void ScoreDisplayRolling::AddToScore( TapNoteScore score, int iCurCombo )
{
//A single step's points are calculated as follows: 
//
//Let p = score multiplier (Perfect = 10, Great = 5, other = 0)
//N = total number of steps and freeze steps
//n = number of the current step or freeze step (varies from 1 to N)
//B = Base value of the song (1,000,000 X the number of feet difficulty) - All edit data is rated as 5 feet
//So, the score for one step is: 
//one_step_score = p * (B/S) * n 
//Where S = The sum of all integers from 1 to N (the total number of steps/freeze steps) 
//
//*IMPORTANT* : Double steps (U+L, D+R, etc.) count as two steps instead of one, so if you get a double L+R on the 112th step of a song, you score is calculated with a Perfect/Great/whatever for both the 112th and 113th steps. Got it? Now, through simple algebraic manipulation 
//S = 1+...+N = (1+N)*N/2 (1 through N added together) 
//Okay, time for an example: 
//
//So, for example, suppose we wanted to calculate the step score of a "Great" on the 57th step of a 441 step, 8-foot difficulty song (I'm just making this one up): 
//
//S = (1 + 441)*441 / 2
//= 194,222 / 2
//= 97,461
//StepScore = p * (B/S) * n
//= 5 * (8,000,000 / 97,461) * 57
//= 5 * (82) * 57 (The 82 is rounded down from 82.08411...)
//= 23,370
//Remember this is just the score for the step, not the cumulative score up to the 57th step. Also, please note that I am currently checking into rounding errors with the system and if there are any, how they are resolved in the system. 
//
//Note: if you got all Perfect on this song, you would get (p=10)*B, which is 80,000,000. In fact, the maximum possible score for any song is the number of feet difficulty X 10,000,000. 

	static int iNumTimesCalled = 0;
	iNumTimesCalled ++;
	LOG->WriteLine("Called %d times.",iNumTimesCalled);


	int p;	// score multiplier 
	switch( score )
	{
	case TNS_GREAT:		p = 10;		break;
	case TNS_PERFECT:	p = 5;		break;
	default:			p = 0;		break;
	}
	
	int N = m_iTotalNotes;
	int n = iCurCombo+1;
	int B = m_iNotesMeter * 1000000;
	float S = (1+N)*N/2.0f;

	int one_step_score = roundf( p * (B/S) * n );

	// HACK:  The total score is off by a factor of 0.5.  For now, multiply by two...
	one_step_score *= 2;


	m_iScore += one_step_score;
	ASSERT( m_iScore >= 0 );

	this->SetScore( m_iScore );
}
