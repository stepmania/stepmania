#include "stdafx.h"
//-----------------------------------------------------------------------------
// File: Player.cpp
//
// Desc: Object that accepts pad input, knocks down ColorArrows that were stepped on, 
//       and keeps score for the player.
//
// Copyright (c) 2001 Chris Danford.  All rights reserved.
//-----------------------------------------------------------------------------


#include "Util.h"
#include "Math.h" // for fabs()

#include "Player.h"



#define STEP_DOWN_TIME	0.05f

#define SCORE_ADD_PERFECT	700
#define SCORE_ADD_GREAT		400
#define SCORE_ADD_GOOD		200
#define SCORE_ADD_BOO		100

#define SCORE_MULT_PERFECT	1.007f
#define SCORE_MULT_GREAT	1.004f
#define SCORE_MULT_GOOD		1.002f
#define SCORE_MULT_BOO		1.001f

#define LIFE_PERFECT	 0.015f
#define LIFE_GREAT		 0.008f
#define LIFE_GOOD		 0.000f
#define LIFE_BOO		-0.015f
#define LIFE_MISS		-0.060f


void Player::Set( GrayArrows *pGA,		ColorArrows *pCA,
				  Judgement *pJ,		Combo *pC,
				  Score *pS,			LifeMeter *pL,
				  Steps steps,			FLOAT fMaxBeatDifference )
{
	RageLog( "Player::Set()" );

	m_pGA			= pGA;
	m_pCA			= pCA;
	m_pJudgement	= pJ;
	m_pCombo		= pC;
	m_pScore		= pS;
	m_pLifeMeter	= pL;
	if( m_pLifeMeter )	m_pLifeMeter->SetLife( m_fLife );

	m_Steps = steps;

	m_StepScore.SetSize( MAX_STEP_ELEMENTS );
	for( int i=0; i<m_StepScore.GetSize(); i++ )
		m_StepScore[i] = no_score;

	m_fMaxBeatDifference = fMaxBeatDifference;
}


void Player::Update( const FLOAT &fDeltaTime )
{
	//RageLog( "Player::Update(%f)", fDeltaTime );

	for( int i=0; i<2; i++ ) {
		for( int j=0; j<4; j++ ) {
			if( m_fStepCountDown[i][j] > 0.0 )
				m_fStepCountDown[i][j] -= fDeltaTime;
		}
	}

	
	int iNumMisses = UpdateMissedStepsOlderThan( m_fSongBeat-m_fMaxBeatDifference );
	if( iNumMisses > 0 )
	{
		m_pJudgement->Miss();
		m_iCurCombo = 0;
		m_pCombo->SetCombo( 0 );
		m_fLife += LIFE_MISS * iNumMisses;
		if( m_fLife < 0.0f )
			m_fLife = 0.0f;
		m_pLifeMeter->SetLife( m_fLife );
	}
	
}

void Player::StepOn( Step player_step )
{
//	DebugLog( CString("Player::Step() ") + padStepL.ToString() + CString(" ") + padStepR.ToString() );

	// Fill the "step buffer" so that the player doesn't have to hit 2 buttons
	// at the exact same time in order to hit a two direction step
	if( player_step & STEP_P1_LEFT )	m_fStepCountDown[0][0] = STEP_DOWN_TIME;
	if( player_step & STEP_P1_DOWN )	m_fStepCountDown[0][1] = STEP_DOWN_TIME;
	if( player_step & STEP_P1_UP )		m_fStepCountDown[0][2] = STEP_DOWN_TIME;
	if( player_step & STEP_P1_RIGHT )	m_fStepCountDown[0][3] = STEP_DOWN_TIME;

	// make the gray foot steps on the screen follow the input
	m_pGA->StepOn( player_step );

	HandleStep();
}



void Player::HandleStep()
{
	//RageLog( "Player::HandleStep()" );

	// This is being called just after a step, so we know at 
	// least one direction is being depressed.

	// Build pad steps to check against
	Step player_step = 0x0000;
	if( m_fStepCountDown[0][0] > 0.0f )	player_step |= STEP_P1_LEFT;
	if( m_fStepCountDown[0][1] > 0.0f )	player_step |= STEP_P1_DOWN;
	if( m_fStepCountDown[0][2] > 0.0f )	player_step |= STEP_P1_UP;
	if( m_fStepCountDown[0][3] > 0.0f )	player_step |= STEP_P1_RIGHT;
	
	// find the closest step that our PadSteps cover
	//RageLog( "I ask: What step %s, %s, is near %f (within %f )", 
	//				  padStepL.ToString(), 
	//				  padStepR.ToString(), 
	//				  m_fSongBeat,
	//				  MAX_BEAT_DIFFERENCE );

	int iIndexOfClosest = 
		m_Steps.GetIndexOfClosestStep( m_fSongBeat,
									   m_fMaxBeatDifference,
									   player_step );

	if( iIndexOfClosest == -1 )	// if no steps in our specified range are covered
		return;
	
	

	FLOAT fStepBeat = StepIndexToBeat( iIndexOfClosest ); // this is the beat of the note we stepped on

	//RageLog( "GetClosestMatch returned: iIndexOfClosest = %d (%s, %s, fStepBeat: %f)", 
	//				  iIndexOfClosest, 
	//				  m_Steps.StepsLeft [iIndexOfClosest].ToString(), 
	//				  m_Steps.StepsRight[iIndexOfClosest].ToString(), 
	//				  fStepBeat );


	FLOAT fBeatsUntilStep = fStepBeat - m_fSongBeat;
	FLOAT fPercentFromPerfect = (FLOAT)fabs( fBeatsUntilStep / m_fMaxBeatDifference );

	RageLog( "fBeatsUntilStep: %f, fPercentFromPerfect: %f", 
			 fBeatsUntilStep, fPercentFromPerfect );

	// step on the note so that it can't be stepped on any more
	m_Steps.StatusArray[iIndexOfClosest] = stepped_on;

	// compute what the score should be for the note we stepped on
	StepScore &score = m_StepScore[iIndexOfClosest];

	if(		 fPercentFromPerfect < 0.20f )
		score = perfect;
	else if( fPercentFromPerfect < 0.45f )
		score = great;
	else if( fPercentFromPerfect < 0.75f )
		score = good;
	else
		score = boo;

	// update the judgement display
	switch( score )
	{
	case perfect:	m_pJudgement->Perfect();	break;
	case great:		m_pJudgement->Great();		break;
	case good:		m_pJudgement->Good();		break;
	case boo:		m_pJudgement->Boo();		break;
	}

	// update the combo display
	switch( score )
	{
	case perfect:
	case great:
		m_iCurCombo++;
		m_pCombo->SetCombo( m_iCurCombo );
		break;
	case good:
	case boo:	
		// combo stopped
		if( m_iCurCombo > m_iMaxCombo )
			m_iMaxCombo = m_iCurCombo;
		m_iCurCombo = 0;
		m_pCombo->SetCombo( m_iCurCombo );
		break;
	}

	// remove the arrows from the ColorArrow columns if the score is high enough
	if( score == perfect  ||  score == great )
	{
		m_pCA->StepOn( BeatToStepIndex(fStepBeat) );
	}

	// update running score
	switch( score )
	{
	case perfect:	m_fScore += SCORE_ADD_PERFECT;	m_fScore *= SCORE_MULT_PERFECT;	break;
	case great:		m_fScore += SCORE_ADD_GREAT;	m_fScore *= SCORE_MULT_GREAT;	break;
	case good:		m_fScore += SCORE_ADD_GOOD;		m_fScore *= SCORE_MULT_GOOD;	break;
	case boo:		m_fScore += SCORE_ADD_BOO;		m_fScore *= SCORE_MULT_BOO;		break;
	case miss:																		break;
	}

	// update life meter
	switch( score )
	{
	case perfect:	m_fLife += LIFE_PERFECT;	break;
	case great:		m_fLife += LIFE_GREAT;		break;
	case good:		m_fLife += LIFE_GOOD;		break;
	case boo:		m_fLife += LIFE_BOO;		break;
	}
	if( m_fLife < 0.0f )		m_fLife = 0.0f;
	else if( m_fLife > 1.0f )	m_fLife = 1.0f;


	m_pScore->SetScore( m_fScore );
	m_pLifeMeter->SetLife( m_fLife );
}

int Player::UpdateMissedStepsOlderThan( FLOAT iMissIfOlderThanThisBeat )
{
	//RageLog( "Steps::UpdateMissedStepsOlderThan(%f)", iMissIfOlderThanThisBeat );

	int iMissIfOlderThanThisIndex = BeatToStepIndex( iMissIfOlderThanThisBeat );

	int iNumMissesFound = 0;

	for( int i=0; i<iMissIfOlderThanThisIndex; i++ )
	{
//		RageLog( "Step %d: status == %d, score == %d", i, StatusArray[i], Score[i] );
		if( m_Steps.StatusArray[i] == not_stepped_on  &&  m_StepScore[i] == no_score )
		{
			m_StepScore[i] = miss;
			iNumMissesFound++;
		}
	}

	return iNumMissesFound;
}


ScoreSummary Player::GetScoreSummary()
{
	ScoreSummary scoreSummary;

	for( int i=0; i<m_StepScore.GetSize(); i++ ) 
	{
		switch( m_StepScore[i] )
		{
		case perfect:	scoreSummary.perfect++;		break;
		case great:		scoreSummary.great++;		break;
		case good:		scoreSummary.good++;		break;
		case boo:		scoreSummary.boo++;			break;
		case miss:		scoreSummary.miss++;		break;
		case no_score:								break;
		}
	}
	scoreSummary.max_combo = m_iMaxCombo;
	scoreSummary.score = m_fScore;
	
	return scoreSummary;
}