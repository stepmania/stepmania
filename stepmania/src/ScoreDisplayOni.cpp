#include "stdafx.h"
/*
-----------------------------------------------------------------------------
 Class: ScoreDisplayOni.h

 Desc: A graphic displayed in the ScoreDisplayOni during Dancing.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "ScoreDisplayOni.h"
#include "RageUtil.h"
#include "RageLog.h"
#include "PrefsManager.h"
#include "RageLog.h"
#include "GameState.h"


const float SCORE_TWEEN_TIME = 0.5f;


ScoreDisplayOni::ScoreDisplayOni()
{
	LOG->WriteLine( "ScoreDisplayOni::ScoreDisplayOni()" );

	// init the text
	Load( THEME->GetPathTo(FONT_SCORE_NUMBERS) );
	TurnShadowOff();

	m_fTrailingScore = 0;

	SetScore( 0 );
}


void ScoreDisplayOni::Init( PlayerNumber pn, PlayerOptions po, int iOriginalNumNotes, int iNotesMeter )
{
	m_PlayerNumber = pn;
	m_PlayerOptions = po;
	m_iTotalNotes = iOriginalNumNotes;
	m_iNotesMeter = iNotesMeter;

	//for( int i=0; i<iOriginalNumNotes; i++ )
	//	AddToScore( TNS_GREAT, i );
}

void ScoreDisplayOni::SetScore( float fNewScore ) 
{ 
	m_fScore = fNewScore;

	float fDelta = (float)m_fScore - m_fTrailingScore;

	m_fScoreVelocity = fDelta / SCORE_TWEEN_TIME;	// in score units per second
}


int ScoreDisplayOni::GetScore() 
{ 
	return (int)m_fScore;
}


void ScoreDisplayOni::Update( float fDeltaTime )
{
	BitmapText::Update( fDeltaTime );

	float fDeltaBefore = (float)m_fScore - m_fTrailingScore;
	m_fTrailingScore += m_fScoreVelocity * fDeltaTime;
	float fDeltaAfter = (float)m_fScore - m_fTrailingScore;

	if( fDeltaBefore * fDeltaAfter < 0 )	// the sign changed
	{
		m_fTrailingScore = (float)m_fScore;
		m_fScoreVelocity = 0;
	}

}

void ScoreDisplayOni::Draw()
{
	float fSecsIntoPlay = 0;
	for( int i=0; i<GAMESTATE->m_aGameplayStatistics.GetSize(); i++ )
		fSecsIntoPlay += GAMESTATE->m_aGameplayStatistics[i].fSecsIntoPlay[m_PlayerNumber];
	int iMinsDisplay = (int)fSecsIntoPlay/60;
	int iSecsDisplay = (int)fSecsIntoPlay - iMinsDisplay*60;
	int iLeftoverDisplay = int( (fSecsIntoPlay - iMinsDisplay*60 - iSecsDisplay) * 100 );
	SetText( ssprintf( "%02d:%02d:%02d", iMinsDisplay, iSecsDisplay, iLeftoverDisplay ) );

	BitmapText::Draw();
}


void ScoreDisplayOni::AddToScore( TapNoteScore score, int iCurCombo )
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
	LOG->WriteLine("Called %d times - param %d.",iNumTimesCalled, iCurCombo);


	int p;	// score multiplier 
	switch( score )
	{
	case TNS_PERFECT:	p = 10;		break;
	case TNS_GREAT:		p = 5;		break;
	default:			p = 0;		break;
	}
	
	int N = m_iTotalNotes;
	int n = iCurCombo+1;
	int B = m_iNotesMeter * 1000000;
	float S = (1+N)*N/2.0f;

	int one_step_score = roundf( p * (B/S) * n );

	m_fScore += one_step_score;
	ASSERT( m_fScore >= 0 );

	// HACK:  The final total is slightly off because of rounding errors
	if( fabsf(m_fScore-B*10) < 100.0f )
		m_fScore = (float)B*10;

	this->SetScore( m_fScore );
}
