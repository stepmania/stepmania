#include "stdafx.h"
/*
-----------------------------------------------------------------------------
 File: Player.cpp

 Desc: Object that accepts pad input, knocks down ColorArrows that were stepped on, 
		and keeps score for the player.

 Copyright (c) 2001 Chris Danford.  All rights reserved.
-----------------------------------------------------------------------------
*/

#include "ScreenDimensions.h"
#include "Math.h" // for fabs()
#include "Player.h"
#include "RageUtil.h"



const float LIFE_PERFECT	=	 0.015f;
const float LIFE_GREAT		=	 0.008f;
const float LIFE_GOOD		=	 0.000f;
const float LIFE_BOO		=	-0.015f;
const float LIFE_MISS		=	-0.030f;

const int ARROW_SIZE	=	 64;

const float ARROW_X_OFFSET[6] = {
	ARROW_SIZE*-2.5,
	ARROW_SIZE*-1.5,
	ARROW_SIZE*-0.5,
	ARROW_SIZE* 0.5,
	ARROW_SIZE* 1.5,
	ARROW_SIZE* 2.5
};

const float GRAY_ARROW_Y					= ARROW_SIZE * 1.5;
const float ARROW_GAP						= 70;
const int NUM_FRAMES_IN_COLOR_ARROW_SPRITE	= 12;


const float JUDGEMENT_DISPLAY_TIME	=	0.6f;
const CString JUDGEMENT_TEXTURE		=	"Textures\\judgement 1x9.png";
const float JUDGEMENT_Y_OFFSET		=	20;

const CString HOLD_JUDGEMENT_TEXTURE	=	"Textures\\judgement 1x9.png";
const float HOLD_JUDGEMENT_Y			=	GRAY_ARROW_Y + 80;


const CString SEQUENCE_COMBO_NUMBERS		=	"SpriteSequences\\MAX Numbers.seq";
const CString SEQUENCE_SCORE_NUMBERS		=	"SpriteSequences\\Bold Numbers.seq";

const float COMBO_TWEEN_TIME		=	0.5f;
const CString COMBO_TEXTURE			=	"Textures\\judgement 1x9.png";
const float COMBO_Y					=	(CENTER_Y+60);


const int LIEFMETER_NUM_PILLS		=	17;
const CString LIFEMETER_FRAME_TEXTURE=	"Textures\\Life Meter Frame.png";
const CString LIFEMETER_PILLS_TEXTURE=	"Textures\\Life Meter Pills 17x1.png";
const float LIFEMETER_Y				=	30;
const float LIFEMETER_PILLS_Y		=	LIFEMETER_Y;
const float PILL_OFFSET_Y[LIEFMETER_NUM_PILLS] = {
	0.3f, 0.7f, 1.0f, 0.7f, 0.3f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f	// kind of a sin wave
};

const CString FONT_SCORE			= "Fonts\\Font - Arial Bold numbers 30px.font";
const CString SCORE_FRAME_TEXTURE	= "Textures\\Score Frame.png";
const float SCORE_Y					= SCREEN_HEIGHT - 40;


Player::Player( PlayerOptions po, PlayerNumber pn )
{
	m_PlayerOptions = po;
	m_PlayerNumber = pn;

	m_iCurCombo = 0;
	m_iMaxCombo = 0;
	m_fLifePercentage = 0.50f;
	m_fScore = 0;

	// init step elements
	for( int i=0; i<MAX_STEP_ELEMENTS; i++ )
	{
		m_OriginalStep[i] = 0;
		m_LeftToStepOn[i] = 0;
		m_StepScore[i] = none;
		m_iColorArrowFrameOffset[i] = 0;
	}


	CMap<Step, Step, float, float> mapStepToRotation;	// arrow facing left is rotation 0
	mapStepToRotation[STEP_PAD1_LEFT]	= 0;
	mapStepToRotation[STEP_PAD1_UPLEFT] = D3DX_PI/4.0f;
	mapStepToRotation[STEP_PAD1_DOWN]	= -D3DX_PI/2.0f;
	mapStepToRotation[STEP_PAD1_UP]		= D3DX_PI/2.0f;
	mapStepToRotation[STEP_PAD1_UPRIGHT]= D3DX_PI*3.0f/4.0f;
	mapStepToRotation[STEP_PAD1_RIGHT]	= D3DX_PI;
	mapStepToRotation[STEP_PAD2_LEFT]	= mapStepToRotation[STEP_PAD1_LEFT];
	mapStepToRotation[STEP_PAD2_UPLEFT] = mapStepToRotation[STEP_PAD1_UPLEFT];
	mapStepToRotation[STEP_PAD2_DOWN]	= mapStepToRotation[STEP_PAD1_DOWN];
	mapStepToRotation[STEP_PAD2_UP]		= mapStepToRotation[STEP_PAD1_UP];
	mapStepToRotation[STEP_PAD2_UPRIGHT]= mapStepToRotation[STEP_PAD1_UPRIGHT];
	mapStepToRotation[STEP_PAD2_RIGHT]	= mapStepToRotation[STEP_PAD1_RIGHT];
	


	switch( GAMEINFO->m_GameMode )
	{
	case single4:
	case versus4:
		m_iNumColumns = 4;		// LEFT, DOWN, UP, RIGHT
		m_StepToColumnNumber[STEP_PAD1_LEFT]	= 0;
		m_StepToColumnNumber[STEP_PAD1_DOWN]	= 1;
		m_StepToColumnNumber[STEP_PAD1_UP]		= 2;
		m_StepToColumnNumber[STEP_PAD1_RIGHT]	= 3;
		m_ColumnNumberToStep[0]		= STEP_PAD1_LEFT;
		m_ColumnNumberToStep[1]		= STEP_PAD1_DOWN;
		m_ColumnNumberToStep[2]		= STEP_PAD1_UP;
		m_ColumnNumberToStep[3]		= STEP_PAD1_RIGHT;
		m_ColumnToRotation[0] = mapStepToRotation[STEP_PAD1_LEFT];
		m_ColumnToRotation[1] = mapStepToRotation[STEP_PAD1_DOWN];
		m_ColumnToRotation[2] = mapStepToRotation[STEP_PAD1_UP];
		m_ColumnToRotation[3] = mapStepToRotation[STEP_PAD1_RIGHT];
		break;
	case single6:
		m_iNumColumns = 6;		// LEFT, UP+LEFT, DOWN, UP, UP+RIGHT, RIGHT
		m_StepToColumnNumber[STEP_PAD1_LEFT]	= 0;
		m_StepToColumnNumber[STEP_PAD1_UPLEFT]	= 1;
		m_StepToColumnNumber[STEP_PAD1_DOWN]	= 2;
		m_StepToColumnNumber[STEP_PAD1_UP]		= 3;
		m_StepToColumnNumber[STEP_PAD1_UPRIGHT]	= 4;
		m_StepToColumnNumber[STEP_PAD1_RIGHT]	= 5;
		m_ColumnNumberToStep[0]		= STEP_PAD1_LEFT;
		m_ColumnNumberToStep[1]		= STEP_PAD1_UPLEFT;
		m_ColumnNumberToStep[2]		= STEP_PAD1_DOWN;
		m_ColumnNumberToStep[3]		= STEP_PAD1_UP;
		m_ColumnNumberToStep[4]		= STEP_PAD1_UPRIGHT;
		m_ColumnNumberToStep[5]		= STEP_PAD1_RIGHT;
		m_ColumnToRotation[0] = mapStepToRotation[STEP_PAD1_LEFT];
		m_ColumnToRotation[1] = mapStepToRotation[STEP_PAD1_UPLEFT];
		m_ColumnToRotation[2] = mapStepToRotation[STEP_PAD1_DOWN];
		m_ColumnToRotation[3] = mapStepToRotation[STEP_PAD1_UP];
		m_ColumnToRotation[4] = mapStepToRotation[STEP_PAD1_UPRIGHT];
		m_ColumnToRotation[5] = mapStepToRotation[STEP_PAD1_RIGHT];
		break;
	case double4:
		m_iNumColumns = 8;		// 1_LEFT, 1_DOWN, 1_UP, 1_RIGHT, 2_LEFT, 2_DOWN, 2_UP, 2_RIGHT
		m_StepToColumnNumber[STEP_PAD1_LEFT]	= 0;
		m_StepToColumnNumber[STEP_PAD1_DOWN]	= 1;
		m_StepToColumnNumber[STEP_PAD1_UP]		= 2;
		m_StepToColumnNumber[STEP_PAD1_RIGHT]	= 3;
		m_StepToColumnNumber[STEP_PAD2_LEFT]	= 4;
		m_StepToColumnNumber[STEP_PAD2_DOWN]	= 5;
		m_StepToColumnNumber[STEP_PAD2_UP]		= 6;
		m_StepToColumnNumber[STEP_PAD2_RIGHT]	= 7;
		m_ColumnNumberToStep[0]		= STEP_PAD1_LEFT;
		m_ColumnNumberToStep[1]		= STEP_PAD1_DOWN;
		m_ColumnNumberToStep[2]		= STEP_PAD1_UP;
		m_ColumnNumberToStep[3]		= STEP_PAD1_RIGHT;
		m_ColumnNumberToStep[4]		= STEP_PAD2_LEFT;
		m_ColumnNumberToStep[5]		= STEP_PAD2_DOWN;
		m_ColumnNumberToStep[6]		= STEP_PAD2_UP;
		m_ColumnNumberToStep[7]		= STEP_PAD2_RIGHT;
		m_ColumnToRotation[0] = mapStepToRotation[STEP_PAD1_LEFT];
		m_ColumnToRotation[1] = mapStepToRotation[STEP_PAD1_DOWN];
		m_ColumnToRotation[2] = mapStepToRotation[STEP_PAD1_UP];
		m_ColumnToRotation[3] = mapStepToRotation[STEP_PAD1_RIGHT];
		m_ColumnToRotation[4] = mapStepToRotation[STEP_PAD2_LEFT];
		m_ColumnToRotation[5] = mapStepToRotation[STEP_PAD2_DOWN];
		m_ColumnToRotation[6] = mapStepToRotation[STEP_PAD2_UP];
		m_ColumnToRotation[7] = mapStepToRotation[STEP_PAD2_RIGHT];
		break;
	}


	// init arrow rotations
	for( int c=0; c < MAX_NUM_COLUMNS; c++ ) {
		m_GrayArrow[c].SetRotation( m_ColumnToRotation[c] );
		m_GhostArrow[c].SetRotation( m_ColumnToRotation[c] );
		m_HoldGhostArrow[c].SetRotation( m_ColumnToRotation[c] );
		m_ColorArrow[c].SetRotation( m_ColumnToRotation[c] );
	}


	// holder for judgement and combo displays
	m_frameJudgementAndCombo.AddActor( &m_sprJudgement );
	m_frameJudgementAndCombo.AddActor( &m_sprCombo );
	m_frameJudgementAndCombo.AddActor( &m_ComboNumber );
	m_frameJudgementAndCombo.SetXY( CENTER_X, CENTER_Y );

	// judgement
	m_fJudgementDisplayCountdown = 0;
	m_sprJudgement.LoadFromTexture( JUDGEMENT_TEXTURE );
	m_sprJudgement.StopAnimating();
	m_sprJudgement.SetXY( 0, m_PlayerOptions.m_bReverseScroll ? JUDGEMENT_Y_OFFSET : -JUDGEMENT_Y_OFFSET  );

	// combo
	m_sprCombo.LoadFromTexture( COMBO_TEXTURE );
	m_sprCombo.StopAnimating();
	m_sprCombo.SetState( 6 );
	m_sprCombo.SetXY( 40, m_PlayerOptions.m_bReverseScroll ? -JUDGEMENT_Y_OFFSET : JUDGEMENT_Y_OFFSET+2 );
	m_sprCombo.SetZoom( 1.0f );

	m_ComboNumber.LoadFromSequenceFile( SEQUENCE_COMBO_NUMBERS );
	m_ComboNumber.SetXY( -40, m_PlayerOptions.m_bReverseScroll ? -JUDGEMENT_Y_OFFSET : JUDGEMENT_Y_OFFSET );

	m_ComboNumber.SetDiffuseColor( D3DXCOLOR(1,1,1,0) );	// invisible
	m_sprCombo.SetDiffuseColor( D3DXCOLOR(1,1,1,0) );	// invisible


	// hold judgement
	for( c=0; c<MAX_NUM_COLUMNS; c++ )
	{
		m_fHoldJudgementDisplayCountdown[c] = 0;
		m_sprHoldJudgement[c].LoadFromTexture( HOLD_JUDGEMENT_TEXTURE );
		m_sprHoldJudgement[c].StopAnimating();
	}

	// life meter
	m_sprLifeMeterFrame.LoadFromTexture( LIFEMETER_FRAME_TEXTURE );
	m_sprLifeMeterPills.LoadFromTexture( LIFEMETER_PILLS_TEXTURE );
	m_sprLifeMeterFrame.StopAnimating();
	m_sprLifeMeterPills.StopAnimating();

	// score
	m_sprScoreFrame.LoadFromTexture( SCORE_FRAME_TEXTURE );
	m_ScoreNumber.LoadFromSequenceFile( SEQUENCE_SCORE_NUMBERS );
	m_ScoreNumber.SetSequence( "         " );



	SetX( CENTER_X );
}


void Player::SetX( float fX )
{
	m_fArrowsCenterX = fX;

	SetGrayArrowsX(fX); 
	SetGhostArrowsX(fX); 
	SetColorArrowsX(fX);	
	SetJudgementX(fX);	
	SetComboX(fX);	
	SetScoreX(fX);	
	SetLifeMeterX(fX);	

	m_frameJudgementAndCombo.SetX(fX);
}


void Player::SetSteps( const Steps& newSteps, bool bLoadOnlyLeftSide, bool bLoadOnlyRightSide )
{ 
	Step	tempSteps[MAX_STEP_ELEMENTS];
	CArray<HoldStep, HoldStep>	tempHoldSteps;

	// copy the steps into our tempSteps where we will transform them
	for( int i=0; i<MAX_STEP_ELEMENTS; i++ ) 
	{
		tempSteps[i] = newSteps.m_Steps[i];
	
		if( bLoadOnlyLeftSide ) 
		{
			// mask off the pad2 steps
			tempSteps[i] &= ~(STEP_PAD2_LEFT | STEP_PAD2_UPLEFT | STEP_PAD2_DOWN | STEP_PAD2_UP | STEP_PAD2_UPRIGHT | STEP_PAD2_RIGHT );
		} 
		else if( bLoadOnlyRightSide ) 
		{
			// replace the step making pad2's step the new pad1 step
			tempSteps[i] =	(tempSteps[i]&STEP_PAD2_LEFT	? STEP_PAD1_LEFT	: 0) |
							(tempSteps[i]&STEP_PAD2_UPLEFT	? STEP_PAD1_UPLEFT	: 0) |
							(tempSteps[i]&STEP_PAD2_DOWN	? STEP_PAD1_DOWN	: 0) |
							(tempSteps[i]&STEP_PAD2_UP		? STEP_PAD1_UP		: 0) |
							(tempSteps[i]&STEP_PAD2_UPRIGHT ? STEP_PAD1_UPRIGHT	: 0) |
							(tempSteps[i]&STEP_PAD2_RIGHT	? STEP_PAD1_RIGHT	: 0);
		}

	}

	if( m_PlayerOptions.m_bAllowFreezeArrows )
	{
		// copy the HoldSteps and transform them later
		for( int i=0; i<newSteps.m_HoldSteps.GetSize(); i++ )
		{
			HoldStep &hs = newSteps.m_HoldSteps[i];

			if( bLoadOnlyLeftSide ) 
			{
				if( hs.m_Step & (STEP_PAD1_LEFT | STEP_PAD1_UPLEFT | STEP_PAD1_DOWN | STEP_PAD1_UP | STEP_PAD1_UPRIGHT | STEP_PAD1_RIGHT) )
					tempHoldSteps.Add( hs );
			} 
			else if( bLoadOnlyRightSide ) 
			{
				if( hs.m_Step & (STEP_PAD2_LEFT | STEP_PAD2_UPLEFT | STEP_PAD2_DOWN | STEP_PAD2_UP | STEP_PAD2_UPRIGHT | STEP_PAD2_RIGHT) )
					tempHoldSteps.Add( hs );
			} 
			else	// load both sides
			{
				tempHoldSteps.Add( hs );
			}
		}
	}


	// transform the steps
	Step oldStepToNewStep [12][2];	// 0 is old Step, 1 is new Step mapping
	oldStepToNewStep[0][0] = STEP_PAD1_LEFT;	oldStepToNewStep[0][1] = STEP_PAD1_LEFT;
	oldStepToNewStep[1][0] = STEP_PAD1_UPLEFT;	oldStepToNewStep[1][1] = STEP_PAD1_UPLEFT;
	oldStepToNewStep[2][0] = STEP_PAD1_DOWN;	oldStepToNewStep[2][1] = STEP_PAD1_DOWN;
	oldStepToNewStep[3][0] = STEP_PAD1_UP;		oldStepToNewStep[3][1] = STEP_PAD1_UP;
	oldStepToNewStep[4][0] = STEP_PAD1_UPRIGHT;	oldStepToNewStep[4][1] = STEP_PAD1_UPRIGHT;
	oldStepToNewStep[5][0] = STEP_PAD1_RIGHT;	oldStepToNewStep[5][1] = STEP_PAD1_RIGHT;
	oldStepToNewStep[6][0] = STEP_PAD2_LEFT;	oldStepToNewStep[6][1] = STEP_PAD2_LEFT;
	oldStepToNewStep[7][0] = STEP_PAD2_UPLEFT;	oldStepToNewStep[7][1] = STEP_PAD2_UPLEFT;
	oldStepToNewStep[8][0] = STEP_PAD2_DOWN;	oldStepToNewStep[8][1] = STEP_PAD2_DOWN;
	oldStepToNewStep[9][0] = STEP_PAD2_UP;		oldStepToNewStep[9][1] = STEP_PAD2_UP;
	oldStepToNewStep[10][0]= STEP_PAD2_UPRIGHT;	oldStepToNewStep[10][1]= STEP_PAD2_UPRIGHT;
	oldStepToNewStep[11][0]= STEP_PAD2_RIGHT;	oldStepToNewStep[11][1]= STEP_PAD2_RIGHT;


	switch( m_PlayerOptions.m_TurnType )
	{
	case PlayerOptions::TURN_NONE:
		break;
	case PlayerOptions::TURN_MIRROR:
		oldStepToNewStep[0][1] = STEP_PAD1_RIGHT;
		oldStepToNewStep[2][1] = STEP_PAD1_UP;
		oldStepToNewStep[3][1] = STEP_PAD1_DOWN;
		oldStepToNewStep[5][1] = STEP_PAD1_LEFT;
		oldStepToNewStep[6][1] = STEP_PAD2_RIGHT;
		oldStepToNewStep[8][1] = STEP_PAD2_UP;
		oldStepToNewStep[9][1] = STEP_PAD2_DOWN;
		oldStepToNewStep[11][1]= STEP_PAD2_LEFT;
		break;
	case PlayerOptions::TURN_LEFT:
		oldStepToNewStep[0][1] = STEP_PAD1_DOWN;
		oldStepToNewStep[2][1] = STEP_PAD1_RIGHT;
		oldStepToNewStep[3][1] = STEP_PAD1_LEFT;
		oldStepToNewStep[5][1] = STEP_PAD1_UP;
		oldStepToNewStep[6][1] = STEP_PAD2_DOWN;
		oldStepToNewStep[8][1] = STEP_PAD2_RIGHT;
		oldStepToNewStep[9][1] = STEP_PAD2_LEFT;
		oldStepToNewStep[11][1]= STEP_PAD2_UP;
		break;
	case PlayerOptions::TURN_RIGHT:
		oldStepToNewStep[0][1] = STEP_PAD1_UP;
		oldStepToNewStep[2][1] = STEP_PAD1_LEFT;
		oldStepToNewStep[3][1] = STEP_PAD1_RIGHT;
		oldStepToNewStep[5][1] = STEP_PAD1_DOWN;
		oldStepToNewStep[6][1] = STEP_PAD2_UP;
		oldStepToNewStep[8][1] = STEP_PAD2_LEFT;
		oldStepToNewStep[9][1] = STEP_PAD2_RIGHT;
		oldStepToNewStep[11][1]= STEP_PAD2_DOWN;
		break;
	case PlayerOptions::TURN_SHUFFLE:
		bool bAlreadyMapped[12];
		for( int i=0; i<12; i++ )
			bAlreadyMapped[i] = false;
		
		// hack: don't shuffle the Up+... Steps
		bAlreadyMapped[1] = true;
		bAlreadyMapped[4] = true;
		bAlreadyMapped[7] = true;
		bAlreadyMapped[10] = true;

		// shuffle left side
		for( i=0; i<6; i++ )
		{
			if( i == 1  ||  i == 4 )
				continue;

			int iMapsTo;
			do {
				iMapsTo = rand()%6;
			} while( bAlreadyMapped[iMapsTo] );
			bAlreadyMapped[iMapsTo] = true;

			oldStepToNewStep[i][1] = oldStepToNewStep[iMapsTo][0];
		}
		// shuffle right side
		for( i=6; i<12; i++ )
		{
			if( i == 7  ||  i == 10 )
				continue;

			int iMapsTo;
			do {
				iMapsTo = rand()%6+6;
			} while( bAlreadyMapped[iMapsTo] );
			bAlreadyMapped[iMapsTo] = true;

			oldStepToNewStep[i][1] = oldStepToNewStep[iMapsTo][0];
		}
		break;
	}

	// transform tempSteps and store them in the Player's structures
	for( i=0; i<MAX_STEP_ELEMENTS; i++ ) 
	{
		Step &oldStep = tempSteps[i];
		Step &newStep = m_OriginalStep[i];
		newStep = STEP_NONE;

		for( int j=0; j<12; j++ )
		{
			if( oldStep & oldStepToNewStep[j][0] )
				newStep |= oldStepToNewStep[j][1];
		}

		m_LeftToStepOn[i] = m_OriginalStep[i];
		m_iColorArrowFrameOffset[i] = (int)( i/(float)ELEMENTS_PER_BEAT*NUM_FRAMES_IN_COLOR_ARROW_SPRITE );
	}

	// transform tempHoldSteps and store them in the Player's structures
	for( i=0; i<tempHoldSteps.GetSize(); i++ ) 
	{
		HoldStep &oldHoldStep = tempHoldSteps[i];
		HoldStep newHoldStep = oldHoldStep;

		for( int j=0; j<12; j++ )
		{
			if( oldHoldStep.m_Step & oldStepToNewStep[j][0] )
				newHoldStep.m_Step |= oldStepToNewStep[j][1];
		}
		
		m_HoldSteps.Add( newHoldStep );
		m_HoldStepScores.Add( HOLD_SCORE_NONE );
	}

}

void Player::Update( float fDeltaTime, float fSongBeat, float fMaxBeatDifference )
{
	//RageLog( "Player::Update(%f, %f, %f)", fDeltaTime, fSongBeat, fMaxBeatDifference );

	m_fSongBeat = fSongBeat;	// save song beat

	// check for misses
	int iNumMisses = UpdateStepsMissedOlderThan( m_fSongBeat-fMaxBeatDifference );
	if( iNumMisses > 0 )
	{
		SetJudgement( miss );
		m_iCurCombo = 0;
		SetCombo( 0 );
		for( int i=0; i<iNumMisses; i++ )
			ChangeLife( miss );
	}


	// check for HoldStep misses
	for( int i=0; i<m_HoldSteps.GetSize(); i++ )
	{
		HoldStep &hs = m_HoldSteps[i];
		float fStartBeat = StepIndexToBeat( hs.m_iStartIndex );
		float fEndBeat = StepIndexToBeat( hs.m_iEndIndex );

		// update the "electric arrow"
		if( m_HoldStepScores[i] == HOLD_STEPPED_ON  &&	// this hold doesn't already have a score
			fStartBeat < m_fSongBeat && m_fSongBeat < fEndBeat )	// if the song beat is in the range of this hold
		{
			PlayerInput PlayerI = { m_PlayerNumber, hs.m_Step };
			if( GAMEINFO->IsButtonDown( PlayerI ) )		// they're holding the button down
			{
				int iCol = m_StepToColumnNumber[ hs.m_Step ];
				m_HoldGhostArrow[iCol].Step();
			}
		}

		// update the logic
		if( (m_HoldStepScores[i] == HOLD_SCORE_NONE  ||  m_HoldStepScores[i] == HOLD_STEPPED_ON)  &&	// this hold doesn't already have a score
			fStartBeat+fMaxBeatDifference/2 < m_fSongBeat && m_fSongBeat < fEndBeat-fMaxBeatDifference/2 )	// if the song beat is in the range of this hold
		{
			PlayerInput PlayerI = { m_PlayerNumber, hs.m_Step };
			if( !GAMEINFO->IsButtonDown( PlayerI ) )		// they're not holding the button down
			{
				m_HoldStepScores[i] = HOLD_SCORE_NG;
				int iCol = m_StepToColumnNumber[ hs.m_Step ];
				SetHoldJudgement( iCol, HOLD_SCORE_NG );
			}
		}
	}

	// check for HoldStep completes
	for( i=0; i<m_HoldSteps.GetSize(); i++ )
	{
		HoldStep &hs = m_HoldSteps[i];
		float fEndBeat = StepIndexToBeat( hs.m_iEndIndex );

		if(  m_fSongBeat > fEndBeat  &&					// if this hold step is in the past
			m_HoldStepScores[i] == HOLD_STEPPED_ON )	// and it doesn't yet have a score
		{
			m_HoldStepScores[i] = HOLD_SCORE_OK;
			int iCol = m_StepToColumnNumber[ hs.m_Step ];
			SetHoldJudgement( iCol, HOLD_SCORE_OK );
		}
	}



	UpdateGrayArrows( fDeltaTime ); 
	UpdateColorArrows( fDeltaTime );
	UpdateGhostArrows( fDeltaTime );
	UpdateJudgement( fDeltaTime );
	UpdateCombo( fDeltaTime );
	UpdateScore( fDeltaTime );
	UpdateLifeMeter( fDeltaTime );

	m_frameJudgementAndCombo.Update( fDeltaTime );
}

void Player::Draw()
{
	DrawGrayArrows(); 
	DrawColorArrows();
	DrawGhostArrows();
	DrawJudgement();
	DrawCombo();
	DrawScore();
	DrawLifeMeter();
	
	m_frameJudgementAndCombo.Draw();
}


void Player::HandlePlayerStep( float fSongBeat, Step player_step, float fMaxBeatDiff )
{
	//RageLog( "Player::HandlePlayerStep()" );

	// update gray arrows
	int iColumnNum = m_StepToColumnNumber[player_step];
	GrayArrowStep( iColumnNum, perfect );

	CheckForCompleteStep( fSongBeat, player_step, fMaxBeatDiff );

	// check if we stepped on the beginning of a HoldStep
	int iCurrentIndex = BeatToStepIndex( fSongBeat );
	for( int i=0; i<m_HoldSteps.GetSize(); i++ )	// for each HoldStep
	{
		if( m_HoldStepScores[i] == HOLD_SCORE_NONE  &&	// this has not already been stepped on
			player_step == m_HoldSteps[i].m_Step )		// player steps in the same note as the hold
		{
			HoldStep &hs = m_HoldSteps[i];
			int iIndexDifference = abs( hs.m_iStartIndex - iCurrentIndex );
			int iMaxIndexDiff = BeatToStepIndex( fMaxBeatDiff );
			
			if( iIndexDifference <= iMaxIndexDiff )
				m_HoldStepScores[i] = HOLD_STEPPED_ON;
		}
	}

}


void Player::CheckForCompleteStep( float fSongBeat, Step player_step, float fMaxBeatDiff )
{
	//RageLog( "Player::CheckForCompleteStep()" );

	// look for the closest matching step
	int iIndexStartLookingAt = BeatToStepIndex( fSongBeat );
	int iNumElementsToExamine = BeatToStepIndex( fMaxBeatDiff );	// number of elements to examine on either end of iIndexStartLookingAt
	
	//RageLog( "iIndexStartLookingAt = %d, iNumElementsToExamine = %d", iIndexStartLookingAt, iNumElementsToExamine );

	// Start at iIndexStartLookingAt and search outward.  The first one that overlaps the player's step is the closest match.
	for( int delta=0; delta <= iNumElementsToExamine; delta++ )
	{
		int iCurrentIndexEarlier = iIndexStartLookingAt - delta;
		int iCurrentIndexLater   = iIndexStartLookingAt + delta;

		// silly check to make sure we don't go out of bounds
		iCurrentIndexEarlier	= clamp( iCurrentIndexEarlier, 0, MAX_STEP_ELEMENTS-1 );
		iCurrentIndexLater		= clamp( iCurrentIndexLater,   0, MAX_STEP_ELEMENTS-1 );

		////////////////////////////
		// check the step to the left of iIndexStartLookingAt
		////////////////////////////
		//RageLog( "Checking steps[%d]", iCurrentIndexEarlier );
		if( m_LeftToStepOn[iCurrentIndexEarlier] & player_step )	// these steps overlap
		{
			m_LeftToStepOn[iCurrentIndexEarlier] &= ~player_step;	// subtract player_step
			if( m_LeftToStepOn[iCurrentIndexEarlier] == 0 )	{		// did this complete the step?
				OnCompleteStep( fSongBeat, player_step, fMaxBeatDiff, iCurrentIndexEarlier );
				return;
			}
		}

		////////////////////////////
		// check the step to the right of iIndexStartLookingAt
		////////////////////////////
		//RageLog( "Checking steps[%d]", iCurrentIndexLater );
		if( m_LeftToStepOn[iCurrentIndexLater] & player_step )		// these steps overlap
		{
			m_LeftToStepOn[iCurrentIndexLater] &= ~player_step;		// subtract player_step
			if( m_LeftToStepOn[iCurrentIndexLater] == 0 ) {			// did this complete the step?
				OnCompleteStep( fSongBeat, player_step, fMaxBeatDiff, iCurrentIndexLater );
				return;
			}
		}
	}
}

void Player::OnCompleteStep( float fSongBeat, Step player_step, float fMaxBeatDiff, int iIndexThatWasSteppedOn )
{
	float fStepBeat = StepIndexToBeat( iIndexThatWasSteppedOn );

 
	float fBeatsUntilStep = fStepBeat - fSongBeat;
	float fPercentFromPerfect = (float)fabs( fBeatsUntilStep / fMaxBeatDiff );

	//RageLog( "fBeatsUntilStep: %f, fPercentFromPerfect: %f", 
	//		 fBeatsUntilStep, fPercentFromPerfect );

	// compute what the score should be for the note we stepped on
	StepScore &stepscore = m_StepScore[iIndexThatWasSteppedOn];

	if(		 fPercentFromPerfect < 0.25f )	stepscore = perfect;
	else if( fPercentFromPerfect < 0.50f )	stepscore = great;
	else if( fPercentFromPerfect < 0.75f )	stepscore = good;
	else									stepscore = boo;

	// update the judgement, score, and life
	SetJudgement( stepscore );
	ChangeScore( stepscore, m_iCurCombo );
	ChangeLife( stepscore );

	// show the gray arrow ghost
	for( int c=0; c < m_iNumColumns; c++ ) {	// for each arrow column
		if( m_OriginalStep[iIndexThatWasSteppedOn] & m_ColumnNumberToStep[c] ) {	// this column is still unstepped on?
			GhostArrowStep( c, stepscore );
		}
	}

	// update the combo display
	switch( stepscore )
	{
	case perfect:
	case great:
		SetCombo( m_iCurCombo+1 );	// combo continuing
		break;
	case good:
	case boo:	
		SetCombo( 0 );		// combo stopped
		break;
	}
}


int Player::UpdateStepsMissedOlderThan( float fMissIfOlderThanThisBeat )
{
	//RageLog( "Steps::UpdateStepsMissedOlderThan(%f)", fMissIfOlderThanThisBeat );

	int iMissIfOlderThanThisIndex = BeatToStepIndex( fMissIfOlderThanThisBeat );

	int iNumMissesFound = 0;
	// Since this is being called frame, let's not check the whole array every time.
	// Instead, only check 10 elements back.  Even 10 is overkill.
	int iStartCheckingAt = max( 0, iMissIfOlderThanThisIndex-10 );

	//RageLog( "iStartCheckingAt: %d   iMissIfOlderThanThisIndex:  %d", iStartCheckingAt, iMissIfOlderThanThisIndex );
	for( int i=iStartCheckingAt; i<iMissIfOlderThanThisIndex; i++ )
	{
		//RageLog( "Checking for miss:  %d: lefttostepon == %d, score == %d", i, m_LeftToStepOn[i], m_StepScore[i] );
		if( m_LeftToStepOn[i] != 0  &&  m_StepScore[i] != miss)
		{
			m_StepScore[i] = miss;
			iNumMissesFound++;
			ChangeLife( miss );
		}
	}

	return iNumMissesFound;
}


ScoreSummary Player::GetScoreSummary()
{
	ScoreSummary scoreSummary;

	for( int i=0; i<MAX_STEP_ELEMENTS; i++ ) 
	{
		switch( m_StepScore[i] )
		{
		case perfect:	scoreSummary.perfect++;		break;
		case great:		scoreSummary.great++;		break;
		case good:		scoreSummary.good++;		break;
		case boo:		scoreSummary.boo++;			break;
		case miss:		scoreSummary.miss++;		break;
		case none:									break;
		}
	}
	scoreSummary.max_combo = m_iMaxCombo;
	scoreSummary.score = m_fScore;
	
	return scoreSummary;
}


float Player::GetArrowColumnX( int iColNum ) 
{
	float fColOffsetFromCenter = iColNum - (m_iNumColumns-1)/2.0f;
	return m_fArrowsCenterX + fColOffsetFromCenter * ARROW_SIZE;
}

void Player::UpdateGrayArrows( float fDeltaTime )
{
	for( int i=0; i < m_iNumColumns; i++ ) {
		m_GrayArrow[i].Update( fDeltaTime );
	}
}

void Player::UpdateGhostArrows( float fDeltaTime )
{
	for( int i=0; i < m_iNumColumns; i++ ) {
		m_GhostArrow[i].Update( fDeltaTime );
		m_HoldGhostArrow[i].Update( fDeltaTime );
	}
}

void Player::DrawGrayArrows()
{
	for( int i=0; i<m_iNumColumns; i++ )
	{
		m_GrayArrow[i].SetBeat( m_fSongBeat );
		m_GrayArrow[i].Draw();
	}
}

void Player::DrawGhostArrows()
{
	for( int i=0; i<m_iNumColumns; i++ )
	{
		m_GhostArrow[i].SetBeat( m_fSongBeat );
		m_GhostArrow[i].Draw();
		m_HoldGhostArrow[i].SetBeat( m_fSongBeat );
		m_HoldGhostArrow[i].Draw();
	}
}

void Player::SetGrayArrowsX( int iNewX )
{
	for( int i=0; i<m_iNumColumns; i++ )
	{
		float fY = GetGrayArrowYPos();
		if( m_PlayerOptions.m_bReverseScroll )	fY = SCREEN_HEIGHT - fY;
		m_GrayArrow[i].SetXY(  GetArrowColumnX(i), fY );
	}
}

void Player::SetGhostArrowsX( int iNewX )
{
	for( int i=0; i<m_iNumColumns; i++ )
	{
		float fY = GetGrayArrowYPos();
		if( m_PlayerOptions.m_bReverseScroll )	fY = SCREEN_HEIGHT - fY;
		m_GhostArrow[i].SetXY(  GetArrowColumnX(i), fY );
		m_HoldGhostArrow[i].SetXY(  GetArrowColumnX(i), fY );
	}
}

void Player::SetColorArrowsX( int iNewX )
{
	for( int i=0; i<m_iNumColumns; i++ )
		m_ColorArrow[i].SetX( GetArrowColumnX(i) );
}

void Player::GrayArrowStep( int index, StepScore score )
{
	m_GrayArrow[index].Step( perfect );
}

void Player::GhostArrowStep( int index, StepScore score )
{
	m_GhostArrow[index].Step( score );		
}

void Player::UpdateColorArrows( float fDeltaTime )
{
	int iIndexFirstArrowToDraw = BeatToStepIndex( m_fSongBeat - 2.0f );	// 2 beats earlier
	if( iIndexFirstArrowToDraw < 0 ) iIndexFirstArrowToDraw = 0;
	int iIndexLastArrowToDraw  = BeatToStepIndex( m_fSongBeat + 7.0f );	// 7 beats later

	for( int c=0; c < m_iNumColumns; c++ ) 
		m_ColorArrow[c].Update( fDeltaTime );
		
}


// 
// modified to add color shifting to the arrow texture
//
void Player::DrawColorArrows()
{
	//RageLog( "ColorArrows::Draw(%f)", fSongBeat );

	int iBaseFrameNo = (int)(m_fSongBeat*2.5) % 12;	// 2.5 is a "fudge number" :-)  This should be based on BPM
	if( iBaseFrameNo < 0 )
		iBaseFrameNo = 0;
	
	if( m_fSongBeat < 0 )
		m_fSongBeat = 0;
	int iIndexFirstArrowToDraw = BeatToStepIndex( m_fSongBeat - 2.0f );	// 2 beats earlier
	if( iIndexFirstArrowToDraw < 0 ) iIndexFirstArrowToDraw = 0;
	int iIndexLastArrowToDraw  = BeatToStepIndex( m_fSongBeat + 7.0f );	// 7 beats later

	//RageLog( "Drawing elements %d through %d", iIndexFirstArrowToDraw, iIndexLastArrowToDraw );

	for( int i=iIndexFirstArrowToDraw; i<=iIndexLastArrowToDraw; i++ )	//	 for each row
	{				
		if( m_LeftToStepOn[i] != 0  || 	// this step is not yet complete 
			m_StepScore[i] == good  ||  m_StepScore[i] == boo  ||  m_StepScore[i] == miss )
		{		
			float fYOffset = GetColorArrowYOffset( i, m_fSongBeat );
			float fYPos = GetColorArrowYPos( i, m_fSongBeat );
			if( m_PlayerOptions.m_bReverseScroll )	fYPos = SCREEN_HEIGHT - fYPos;

			float fAlpha = GetColorArrowAlphaFromYOffset( fYOffset );

			// beats until the note is stepped on.
			float fBeatsTilStep = StepIndexToBeat( i ) - m_fSongBeat;

			//RageLog( "iYPos: %d, iFrameNo: %d, m_OriginalStep[i]: %d", iYPos, iFrameNo, m_OriginalStep[i] );

			for( int c=0; c < m_iNumColumns; c++ ) {	// for each arrow column
				if( m_OriginalStep[i] & m_ColumnNumberToStep[c] ) {	// this column is still unstepped on?
					m_ColorArrow[c].SetY( fYPos );
					m_ColorArrow[c].SetIndexAndBeat( i, m_fSongBeat );
					m_ColorArrow[c].SetAlpha( fAlpha );
					m_ColorArrow[c].Draw();
				}
			}


		}	// end if there is a step
	}	// end foreach arrow to draw

	// draw all freeze arrows
	for( i=0; i<m_HoldSteps.GetSize(); i++ )
	{
		HoldStep &hs = m_HoldSteps[i];

		if( !( iIndexFirstArrowToDraw <= hs.m_iEndIndex && hs.m_iEndIndex <= iIndexLastArrowToDraw  ||
			   iIndexFirstArrowToDraw <= hs.m_iStartIndex  && hs.m_iStartIndex <= iIndexLastArrowToDraw  ||
			   hs.m_iStartIndex < iIndexFirstArrowToDraw && hs.m_iEndIndex > iIndexLastArrowToDraw ) )
		{
			continue;
		}

		HoldStepScore &score = m_HoldStepScores[i];

		int iColNum = m_StepToColumnNumber[ hs.m_Step ];

		switch( score )
		{
		case HOLD_SCORE_OK:
			continue;	// don't draw
		case HOLD_SCORE_NONE:
		case HOLD_SCORE_NG:
			m_ColorArrow[iColNum].SetGrayPartClear();
			break;
		case HOLD_STEPPED_ON:
			m_ColorArrow[iColNum].SetGrayPartFull();
			break;
		}

		// draw the gray parts
		for( int j=hs.m_iEndIndex; j>=hs.m_iStartIndex; j-- )	// for each arrow in this freeze
		{
			float fYOffset = GetColorArrowYOffset( j, m_fSongBeat );
			float fAlpha = GetColorArrowAlphaFromYOffset( fYOffset );

			float fYPos = GetColorArrowYPos( j, m_fSongBeat );
			if( score == HOLD_STEPPED_ON  ||  score == HOLD_SCORE_OK )
				fYPos = max( fYPos, GetGrayArrowYPos() );
			if( m_PlayerOptions.m_bReverseScroll )	fYPos = SCREEN_HEIGHT - fYPos;
			m_ColorArrow[iColNum].SetY( fYPos );
			m_ColorArrow[iColNum].SetAlpha( fAlpha );
			m_ColorArrow[iColNum].DrawGrayPart();
		}

		// draw the color parts
		for( j=hs.m_iEndIndex; j>=hs.m_iStartIndex; j-- )	// for each arrow in this freeze
		{
			float fYOffset = GetColorArrowYOffset( j, m_fSongBeat );
			float fAlpha = GetColorArrowAlphaFromYOffset( fYOffset );
	
			float fYPos = GetColorArrowYPos( j, m_fSongBeat );
			m_ColorArrow[iColNum].SetY( fYPos );
			if( score == HOLD_STEPPED_ON  ||  score == HOLD_SCORE_OK )
				fYPos = max( fYPos, GetGrayArrowYPos() );
			if( m_PlayerOptions.m_bReverseScroll )	fYPos = SCREEN_HEIGHT - fYPos;
			m_ColorArrow[iColNum].SetY( fYPos );
			D3DXCOLOR color( (float)(j-hs.m_iStartIndex)/(float)(hs.m_iEndIndex-hs.m_iStartIndex), 1, 0, 1 );	// color shifts from green to yellow
			m_ColorArrow[iColNum].SetDiffuseColor( color );	
			m_ColorArrow[iColNum].SetAlpha( fAlpha );
			m_ColorArrow[iColNum].DrawColorPart();
		}

		// draw the first arrow on top of the others
		j = hs.m_iStartIndex;

		float fYOffset = GetColorArrowYOffset( j, m_fSongBeat );
		float fAlpha = GetColorArrowAlphaFromYOffset( fYOffset );
		float fYPos = GetColorArrowYPos( j, m_fSongBeat );
		if( score == HOLD_STEPPED_ON  ||  score == HOLD_SCORE_OK )
			fYPos = max( fYPos, GetGrayArrowYPos() );
		if( m_PlayerOptions.m_bReverseScroll )	fYPos = SCREEN_HEIGHT - fYPos;
		m_ColorArrow[iColNum].SetY( fYPos );
		m_ColorArrow[iColNum].SetIndexAndBeat( i, m_fSongBeat );
		D3DXCOLOR color( 0, 1, 0, 1 );	// color shifts from green to yellow
		m_ColorArrow[iColNum].SetDiffuseColor( color );
		m_ColorArrow[iColNum].SetAlpha( fAlpha );
		m_ColorArrow[iColNum].Draw();

	}

}



float Player::GetColorArrowYPos( int iStepIndex, float fSongBeat )
{
	float fBeatsUntilStep = StepIndexToBeat( iStepIndex ) - fSongBeat;
	float fYOffset = fBeatsUntilStep * ARROW_GAP * m_PlayerOptions.m_fArrowScrollSpeed;
	switch( m_PlayerOptions.m_EffectType )
	{
	case PlayerOptions::EFFECT_BOOST:
		fYOffset *= 1.4f / ((fYOffset+SCREEN_HEIGHT/1.6f)/SCREEN_HEIGHT); 
		break;
	case PlayerOptions::EFFECT_WAVE:
		fYOffset += 15*sin( fYOffset/38 ); 
		break;
	}
	float fYPos = fYOffset + GRAY_ARROW_Y;

	return fYPos;

}

float Player::GetColorArrowYOffset( int iStepIndex, float fSongBeat )
{
	float fBeatsUntilStep = StepIndexToBeat( iStepIndex ) - fSongBeat;
	float fYOffset = fBeatsUntilStep * ARROW_GAP * m_PlayerOptions.m_fArrowScrollSpeed;
	switch( m_PlayerOptions.m_EffectType )
	{
	case PlayerOptions::EFFECT_BOOST:
		fYOffset *= 1.4f / ((fYOffset+SCREEN_HEIGHT/1.6f)/SCREEN_HEIGHT); 
		break;
	case PlayerOptions::EFFECT_WAVE:
		fYOffset += 15*sin( fYOffset/38 ); 
		break;
	}
	return fYOffset;
}

float Player::GetColorArrowAlphaFromYOffset( float fYOffset )
{
	float fAlpha;
	switch( m_PlayerOptions.m_AppearanceType )
	{ 
	case PlayerOptions::APPEARANCE_VISIBLE:
		fAlpha = 1;
		break;
	case PlayerOptions::APPEARANCE_HIDDEN:
//						fAlpha = m_PlayerOptions.m_bReverseScroll ? ((SCREEN_HEIGHT-(fYPos-200))/200) : ((fYPos-200)/200);
		fAlpha = (fYOffset-100)/200;
		break;
	case PlayerOptions::APPEARANCE_SUDDEN:
//						fAlpha = m_PlayerOptions.m_bReverseScroll ? ((SCREEN_HEIGHT-(fYPos+200))/200) : ((fYPos+200)/200);
		fAlpha = ((SCREEN_HEIGHT-fYOffset)-280)/200;
		break;
	case PlayerOptions::APPEARANCE_STEALTH:
		fAlpha = 0;
		break;
	};
	if( fYOffset < 0 )
		fAlpha = 1;

	return fAlpha;
};

float Player::GetGrayArrowYPos()
{
	return GRAY_ARROW_Y;
}






void Player::SetJudgementX( int iNewX )
{
	// the frame will do this for us
	//float fY = JUDGEMENT_Y;
	//if( m_PlayerOptions.m_bReverseScroll )	fY = SCREEN_HEIGHT - fY;
	//m_sprJudgement.SetXY( iNewX, fY );

	float fY = HOLD_JUDGEMENT_Y;
	if( m_PlayerOptions.m_bReverseScroll )	fY = SCREEN_HEIGHT - fY;
	for( int c=0; c<MAX_NUM_COLUMNS; c++ )
	{
		m_sprHoldJudgement[c].SetXY( GetArrowColumnX(c), fY );
	}
}

void Player::UpdateJudgement( float fDeltaTime )
{
	m_fJudgementDisplayCountdown -= fDeltaTime;
	if( m_fJudgementDisplayCountdown < 0 )
	{
		m_sprJudgement.SetDiffuseColor( D3DXCOLOR(1,1,1,0) );	// invisible
		m_fJudgementDisplayCountdown = 0;
	}

	// m_sprJudgement.Update( fDeltaTime );	// the frame will take care of this for us.

	for( int c=0; c<MAX_NUM_COLUMNS; c++ )
	{
		if( m_fHoldJudgementDisplayCountdown[c] > 0 )
			m_fHoldJudgementDisplayCountdown[c] -= fDeltaTime;
		m_sprHoldJudgement[c].Update( fDeltaTime );
	}

}

void Player::DrawJudgement()
{
	//if( m_fJudgementDisplayCountdown > 0.0 )
	//	m_sprJudgement.Draw();		// the frame will take care of this for us

	for( int c=0; c<MAX_NUM_COLUMNS; c++ )
	{
		if( m_fHoldJudgementDisplayCountdown[c] > 0.0 )
			m_sprHoldJudgement[c].Draw();
	}
}

void Player::SetJudgement( StepScore score )
{
	//RageLog( "Judgement::SetJudgement()" );

	switch( score )
	{
	case perfect:	m_sprJudgement.SetState( 0 );	break;
	case great:		m_sprJudgement.SetState( 1 );	break;
	case good:		m_sprJudgement.SetState( 2 );	break;
	case boo:		m_sprJudgement.SetState( 3 );	break;
	case miss:		m_sprJudgement.SetState( 4 );	break;
	}

	m_fJudgementDisplayCountdown = JUDGEMENT_DISPLAY_TIME;

	if( score == miss ) {	// falling down
		m_sprJudgement.SetY( (m_PlayerOptions.m_bReverseScroll ? JUDGEMENT_Y_OFFSET : -JUDGEMENT_Y_OFFSET) - 30 );
		m_sprJudgement.SetZoom( 1.0f );
		m_sprJudgement.SetDiffuseColor( D3DXCOLOR(1,1,1,1) );	// visible
		m_sprJudgement.BeginTweening( JUDGEMENT_DISPLAY_TIME );
		m_sprJudgement.SetTweenY( (m_PlayerOptions.m_bReverseScroll ? JUDGEMENT_Y_OFFSET : -JUDGEMENT_Y_OFFSET) + 30 );

	} else {		// zooming out
		m_sprJudgement.StopTweening();
		m_sprJudgement.SetY( (m_PlayerOptions.m_bReverseScroll ? JUDGEMENT_Y_OFFSET : -JUDGEMENT_Y_OFFSET) );
		m_sprJudgement.SetDiffuseColor( D3DXCOLOR(1,1,1,1) );	// visible
		
		m_frameJudgementAndCombo.SetZoom( 1.35f );
		m_frameJudgementAndCombo.BeginTweening( JUDGEMENT_DISPLAY_TIME/5.0 );
		m_frameJudgementAndCombo.SetTweenZoom( 1.0f );
		
/*		m_sprJudgement.SetZoom( 1.5f );
		m_sprJudgement.SetDiffuseColor( D3DXCOLOR(1,1,1,1) );	// visible
		m_sprJudgement.BeginTweening( JUDGEMENT_DISPLAY_TIME/3.0 );
		m_sprJudgement.SetTweenZoom( 1.0f );
		*/
	}
}

void Player::SetHoldJudgement( int iCol, HoldStepScore score )
{
	//RageLog( "Judgement::SetJudgement()" );

	switch( score )
	{
	case HOLD_SCORE_NONE:	m_sprHoldJudgement[iCol].SetState( 0 );	break;	// freeze!
	case HOLD_SCORE_OK:		m_sprHoldJudgement[iCol].SetState( 7 );	break;
	case HOLD_SCORE_NG:		m_sprHoldJudgement[iCol].SetState( 8 );	break;
	}

	m_fHoldJudgementDisplayCountdown[iCol] = JUDGEMENT_DISPLAY_TIME;

	if( score == HOLD_SCORE_NG ) {	// falling down
		float fY = HOLD_JUDGEMENT_Y;
		if( m_PlayerOptions.m_bReverseScroll )	fY = SCREEN_HEIGHT - fY;
		m_sprHoldJudgement[iCol].SetY( fY - 10 );
		m_sprHoldJudgement[iCol].SetZoom( 1.0f );
		m_sprHoldJudgement[iCol].BeginTweening( JUDGEMENT_DISPLAY_TIME );
		m_sprHoldJudgement[iCol].SetTweenY( fY + 10 );
	} else {		// zooming out
		float fY = HOLD_JUDGEMENT_Y;
		if( m_PlayerOptions.m_bReverseScroll )	fY = SCREEN_HEIGHT - fY;
		m_sprHoldJudgement[iCol].SetY( fY );
		m_sprHoldJudgement[iCol].SetZoom( 1.5f );
		m_sprHoldJudgement[iCol].BeginTweening( JUDGEMENT_DISPLAY_TIME/3.0 );
		m_sprHoldJudgement[iCol].SetTweenZoom( 1.0f );
	}
}

void Player::SetComboX( int iNewX )
{
	float fY;

	fY = COMBO_Y;
	if( m_PlayerOptions.m_bReverseScroll )	fY = SCREEN_HEIGHT - fY;

	//m_sprCombo.SetXY( iNewX+40, fY );		// the frame will do this for us
	//m_ComboNumber.SetXY(  iNewX-50, fY );
}

void Player::UpdateCombo( float fDeltaTime )
{
	//m_sprCombo.Update( fDeltaTime );		// the frame will do this for us
	//m_ComboNumber.Update( fDeltaTime );	// the frame will do this for us
}

void Player::DrawCombo()
{
// the frame will do this for us
//	if( m_bComboVisible )
//	{
//		m_ComboNumber.Draw();
//		m_sprCombo.Draw();
//	}
}


void Player::SetCombo( int iNewCombo )
{
	// new max combo
	if( iNewCombo > m_iMaxCombo )
		m_iMaxCombo = iNewCombo;

	m_iCurCombo = iNewCombo;

	if( iNewCombo <= 4 )
	{
		m_ComboNumber.SetDiffuseColor( D3DXCOLOR(1,1,1,0) );	// invisible
		m_sprCombo.SetDiffuseColor( D3DXCOLOR(1,1,1,0) );	// invisible
	}
	else
	{
		m_ComboNumber.SetDiffuseColor( D3DXCOLOR(1,1,1,1) );	// visible
		m_sprCombo.SetDiffuseColor( D3DXCOLOR(1,1,1,1) );	// visible

		m_ComboNumber.SetSequence( ssprintf("%d", iNewCombo) );
		float fNewZoom = 0.5f + iNewCombo/800.0f;
		m_ComboNumber.SetZoom( fNewZoom ); 
		m_ComboNumber.SetX( -40 - (fNewZoom-1)*30 ); 
		m_ComboNumber.SetY( m_PlayerOptions.m_bReverseScroll ? -JUDGEMENT_Y_OFFSET : JUDGEMENT_Y_OFFSET ); 
		
		//m_ComboNumber.BeginTweening( COMBO_TWEEN_TIME );
		//m_ComboNumber.SetTweenZoom( 1 );
	}

}



void Player::SetLifeMeterX( int iNewX )
{
	m_sprLifeMeterFrame.SetXY( iNewX, LIFEMETER_Y );
	m_sprLifeMeterPills.SetXY( iNewX, LIFEMETER_PILLS_Y );
}

void Player::UpdateLifeMeter( float fDeltaTime )
{
	m_sprLifeMeterFrame.Update( fDeltaTime );
	m_sprLifeMeterPills.Update( fDeltaTime );
}

void Player::DrawLifeMeter()
{
	float fBeatPercentage = m_fSongBeat - (int)m_fSongBeat;
	int iOffsetStart = roundf( LIEFMETER_NUM_PILLS*fBeatPercentage );

	m_sprLifeMeterFrame.Draw();

	float iX = m_sprLifeMeterFrame.GetX() - m_sprLifeMeterFrame.GetZoomedWidth()/2 + 27;
	int iNumPills = (int)(m_sprLifeMeterPills.GetNumStates() * m_fLifePercentage);
	int iPillWidth = m_sprLifeMeterPills.GetZoomedWidth();

	for( int i=0; i<iNumPills; i++ )
	{
		m_sprLifeMeterPills.SetState( i );
		m_sprLifeMeterPills.SetX( iX );

		int iOffsetNum = (iOffsetStart - i + LIEFMETER_NUM_PILLS) % LIEFMETER_NUM_PILLS;
		int iOffset = roundf( PILL_OFFSET_Y[iOffsetNum] * m_fLifePercentage * 8.0f );
		m_sprLifeMeterPills.SetY( LIFEMETER_PILLS_Y - iOffset );

		m_sprLifeMeterPills.Draw();

		iX += iPillWidth;
	}
}

void Player::ChangeLife( StepScore score )
{
	switch( score )
	{
	case perfect:	m_fLifePercentage += LIFE_PERFECT;	break;
	case great:		m_fLifePercentage += LIFE_GREAT;	break;
	case good:		m_fLifePercentage += LIFE_GOOD;		break;
	case boo:		m_fLifePercentage += LIFE_BOO;		break;
	case miss:		m_fLifePercentage += LIFE_MISS;		break;
	}

	m_fLifePercentage = clamp( m_fLifePercentage, 0, 1 );
	
	if( m_fLifePercentage == 1 )
		m_sprLifeMeterFrame.SetEffectCamelion( 5, D3DXCOLOR(0.2f,0.2f,0.2f,1), D3DXCOLOR(1,1,1,1) );
	else if( m_fLifePercentage < 0.25f )
		m_sprLifeMeterFrame.SetEffectCamelion( 5, D3DXCOLOR(1,0.8f,0.8f,1), D3DXCOLOR(1,0.2f,0.2f,1) );
	else
		m_sprLifeMeterFrame.SetEffectNone();

}






void Player::SetScoreX( int iNewX )
{
	m_sprScoreFrame.SetXY( iNewX, SCORE_Y );
	m_ScoreNumber.SetXY(  iNewX, SCORE_Y );
}

void Player::UpdateScore( float fDeltaTime )
{
	m_sprScoreFrame.Update( fDeltaTime );
	m_ScoreNumber.Update( fDeltaTime );
}

void Player::DrawScore()
{
	m_ScoreNumber.Draw();
	m_sprScoreFrame.Draw();
}


void Player::ChangeScore( StepScore score, int iCurCombo )
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
	switch( score )
	{
	case miss:											break;
	case boo:											break;
	case good:		fScoreToAdd = M * 100     + 100;	break;
	case great:		fScoreToAdd = M * M * 100 + 300;	break;
	case perfect:	fScoreToAdd = M * M * 300 + 500;	break;
	}
	m_fScore += fScoreToAdd;
	ASSERT( m_fScore >= 0 );

	
	m_ScoreNumber.SetSequence( ssprintf( "%9.0f", m_fScore ) );
}