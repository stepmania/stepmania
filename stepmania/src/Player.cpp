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
const float GRAY_ARROW_POP_UP_TIME			= 0.30f;
const float ARROW_GAP						= 70;
const int NUM_FRAMES_IN_COLOR_ARROW_SPRITE	= 12;

const CString SPRITE_COLOR_ARROW = "Sprites\\Color Arrow.sprite";
const CString SPRITE_GRAY_ARROW = "Sprites\\Gray Arrow.sprite";


const float JUDGEMENT_DISPLAY_TIME	=	0.6f;
const CString JUDGEMENT_SPRITE		=	"Sprites\\Judgement.sprite";
const float JUDGEMENT_Y				=	CENTER_Y;

const CString FONT_COMBO			=	"Fonts\\Font - Arial Bold numbers 30px.font";

const float COMBO_TWEEN_TIME		=	0.5f;
const CString COMBO_SPRITE			=	"Sprites\\Combo.sprite";
const float COMBO_Y					=	(CENTER_Y+60);


const int LIEFMETER_NUM_PILLS		=	17;
const CString LIFEMETER_FRAME_SPRITE=	"Sprites\\Life Meter Frame.sprite";
const CString LIFEMETER_PILLS_SPRITE=	"Sprites\\Life Meter Pills.sprite";
const float LIFEMETER_Y				=	30;
const float LIFEMETER_PILLS_Y		=	LIFEMETER_Y+2;
const float PILL_OFFSET_Y[LIEFMETER_NUM_PILLS] = {
	0.3f, 0.7f, 1.0f, 0.7f, 0.3f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f	// kind of a sin wave
};

const CString FONT_SCORE			= "Fonts\\Font - Arial Bold numbers 30px.font";
const CString SCORE_FRAME_TEXTURE	= "Textures\\Score Frame 1x1.png";
const float SCORE_Y					= SCREEN_HEIGHT - 40;


Player::Player()
{
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


	for( int c=0; c < MAX_NUM_COLUMNS; c++ ) {
		// gray arrows
		m_sprGrayArrow[c].LoadFromSpriteFile( SPRITE_GRAY_ARROW );
		m_sprGrayArrow[c].SetRotation( m_ColumnToRotation[c] );

		m_sprGrayArrowGhost[c].LoadFromSpriteFile( SPRITE_GRAY_ARROW );
		m_sprGrayArrowGhost[c].SetRotation( m_ColumnToRotation[c] );
		m_sprGrayArrowGhost[c].StopAnimating();
		m_sprGrayArrowGhost[c].SetState( 1 );
		m_sprGrayArrowGhost[c].SetColor( D3DXCOLOR(1,1,1,0) );

		// color arrows
		m_sprColorArrow[c].LoadFromSpriteFile( SPRITE_COLOR_ARROW );
		m_sprColorArrow[c].StopAnimating();
		m_sprColorArrow[c].SetRotation( m_ColumnToRotation[c] );
	}

	// judgement
	m_fJudgementDisplayCountdown = 0;
	m_sprJudgement.LoadFromSpriteFile( JUDGEMENT_SPRITE );

	// combo
	m_bComboVisible = FALSE;
	m_sprCombo.LoadFromSpriteFile( COMBO_SPRITE );
	m_textComboNum.LoadFromFontName( "Arial Bold" );
	m_textComboNum.SetText( "" );

	// life meter
	m_sprLifeMeterFrame.LoadFromSpriteFile( LIFEMETER_FRAME_SPRITE );
	m_sprLifeMeterPills.LoadFromSpriteFile( LIFEMETER_PILLS_SPRITE );

	// score
	m_sprScoreFrame.LoadFromTexture( SCORE_FRAME_TEXTURE );
	m_textScoreNum.LoadFromFontName( "Arial Black with Outline" );
	m_textScoreNum.SetText( "         " );


	SetX( CENTER_X );
}


void Player::SetX( int iX )
{
	m_iArrowsCenterX = iX;

	SetGrayArrowsX(iX); 
	SetColorArrowsX(iX);	
	SetJudgementX(iX);	
	SetComboX(iX);	
	SetScoreX(iX);	
	SetLifeMeterX(iX);	
}


void Player::SetSteps( const Steps& newSteps )
{ 
	for( int i=0; i<MAX_STEP_ELEMENTS; i++ ) {
		m_OriginalStep[i] = newSteps.m_steps[i];
		m_LeftToStepOn[i] = newSteps.m_steps[i];
		m_iColorArrowFrameOffset[i] = (int)( i/(FLOAT)ELEMENTS_PER_BEAT*NUM_FRAMES_IN_COLOR_ARROW_SPRITE );
	}
}

void Player::Update( const float &fDeltaTime, float fSongBeat, float fMaxBeatDifference )
{
	//RageLog( "Player::Update(%f, %f, %f)", fDeltaTime, fSongBeat, fMaxBeatDifference );

	int iNumMisses = UpdateStepsMissedOlderThan( fSongBeat-fMaxBeatDifference );
	if( iNumMisses > 0 )
	{
		SetJudgement( miss );
		m_iCurCombo = 0;
		SetCombo( 0 );
		for( int i=0; i<iNumMisses; i++ )
			ChangeLife( miss );
	}


	UpdateGrayArrows( fDeltaTime ); 
	UpdateColorArrows( fDeltaTime );
	UpdateJudgement( fDeltaTime );
	UpdateCombo( fDeltaTime );
	UpdateScore( fDeltaTime );
	UpdateLifeMeter( fDeltaTime );
}

void Player::Draw( float fSongBeat )
{
	DrawGrayArrows(); 
	DrawColorArrows( fSongBeat );
	DrawJudgement();
	DrawCombo();
	DrawScore();
	DrawLifeMeter( fSongBeat );	
}


void Player::HandlePlayerStep( float fSongBeat, Step player_step, float fMaxBeatDiff )
{
	//RageLog( "Player::HandlePlayerStep()" );

	// update gray arrows
	int iColumnNum = m_StepToColumnNumber[player_step];
	GrayArrowStep( iColumnNum );

	CheckForCompleteStep( fSongBeat, player_step, fMaxBeatDiff );
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

	// show the gray arrow ghost
	int iColumnNum = m_StepToColumnNumber[player_step];
	GrayArrowGhostStep( iColumnNum );

	float fBeatsUntilStep = fStepBeat - fSongBeat;
	float fPercentFromPerfect = (float)fabs( fBeatsUntilStep / fMaxBeatDiff );

	//RageLog( "fBeatsUntilStep: %f, fPercentFromPerfect: %f", 
	//		 fBeatsUntilStep, fPercentFromPerfect );

	// compute what the score should be for the note we stepped on
	StepScore &stepscore = m_StepScore[iIndexThatWasSteppedOn];

	if(		 fPercentFromPerfect < 0.20f )	stepscore = perfect;
	else if( fPercentFromPerfect < 0.45f )	stepscore = great;
	else if( fPercentFromPerfect < 0.75f )	stepscore = good;
	else									stepscore = boo;

	// update the judgement, score, and life
	SetJudgement( stepscore );
	ChangeScore( stepscore, m_iCurCombo );
	ChangeLife( stepscore );

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


int Player::GetArrowColumnX( int iColNum ) 
{ 
	return m_iArrowsCenterX + (iColNum - (m_iNumColumns-1)/2) * ARROW_SIZE;
}

void Player::UpdateGrayArrows( const float &fDeltaTime )
{
	for( int i=0; i < m_iNumColumns; i++ ) {
		m_sprGrayArrow[i].Update( fDeltaTime );
		m_sprGrayArrowGhost[i].Update( fDeltaTime );
	}
}

void Player::DrawGrayArrows()
{
	for( int i=0; i<m_iNumColumns; i++ )
		m_sprGrayArrow[i].Draw();
}

void Player::SetGrayArrowsX( int iNewX )
{
	for( int i=0; i<m_iNumColumns; i++ )
		m_sprGrayArrow[i].SetXY(  GetArrowColumnX(i), GRAY_ARROW_Y );
}

void Player::SetColorArrowsX( int iNewX )
{
	for( int i=0; i<m_iNumColumns; i++ )
		m_sprColorArrow[i].SetX( GetArrowColumnX(i) );

}

void Player::GrayArrowStep( int index )
{
	m_sprGrayArrow[index].SetZoom( 0.50 );
	m_sprGrayArrow[index].BeginTweening( GRAY_ARROW_POP_UP_TIME );
	m_sprGrayArrow[index].SetTweenZoom( 1.0f );
}

void Player::GrayArrowGhostStep( int index )
{
	m_sprGrayArrowGhost[index].SetXY( GetArrowColumnX(index), GRAY_ARROW_Y );
	m_sprGrayArrowGhost[index].SetZoom( 1 );
	m_sprGrayArrowGhost[index].SetColor( D3DXCOLOR(1,1,0.5f,1) );
	m_sprGrayArrowGhost[index].BeginTweening( 0.3f );
	m_sprGrayArrowGhost[index].SetTweenZoom( 1.5 );
	m_sprGrayArrowGhost[index].SetTweenColor( D3DXCOLOR(1,1,0.5f,0) );		
}

void Player::UpdateColorArrows( const float &fDeltaTime )
{

}

void Player::DrawColorArrows( float fSongBeat )
{
	//RageLog( "ColorArrows::Draw(%f)", fSongBeat );

	int iBaseFrameNo = (int)(fSongBeat*2.5) % 12;	// 2.5 is a "fudge number" :-)  This should be based on BPM

	int iIndexFirstArrowToDraw = BeatToStepIndex( fSongBeat - 2.0f );	// 2 beats earlier
	if( iIndexFirstArrowToDraw < 0 ) iIndexFirstArrowToDraw = 0;
	int iIndexLastArrowToDraw  = BeatToStepIndex( fSongBeat + 7.0f );	// 7 beats later

	//RageLog( "Drawing elements %d through %d", iIndexFirstArrowToDraw, iIndexLastArrowToDraw );

	for( int i=iIndexFirstArrowToDraw; i<=iIndexLastArrowToDraw; i++ )	//	 for each row
	{				
		if( m_LeftToStepOn[i] != 0 )	// this step is not yet complete 
		{		
			int iYPos = GetColorArrowYPos( i, fSongBeat );

			// calculate which frame to display
			int iFrameNo = iBaseFrameNo + m_iColorArrowFrameOffset[i];
			iFrameNo = iFrameNo % NUM_FRAMES_IN_COLOR_ARROW_SPRITE;
				
			//RageLog( "iYPos: %d, iFrameNo: %d, m_OriginalStep[i]: %d", iYPos, iFrameNo, m_OriginalStep[i] );

			for( int c=0; c < m_iNumColumns; c++ ) {	// for each arrow column
				if( m_OriginalStep[i] & m_ColumnNumberToStep[c] ) {	// this column is still unstepped on?
					m_sprColorArrow[c].SetY( iYPos );
					m_sprColorArrow[c].SetState( iFrameNo );
					m_sprColorArrow[c].Draw();
				}
			}


		}	// end if there is a step
	}	// end foreach arrow to draw

	for( i=0; i<m_iNumColumns; i++ ) {
		m_sprGrayArrowGhost[i].Draw();
	}
}





int Player::GetColorArrowYPos( int iStepIndex, float fSongBeat )
{
	float fBeatsUntilStep = StepIndexToBeat( iStepIndex ) - fSongBeat;
	return (int)(fBeatsUntilStep * ARROW_GAP) + GRAY_ARROW_Y;
}






void Player::SetJudgementX( int iNewX )
{
	m_sprJudgement.SetXY(  iNewX,  CENTER_Y );
}

void Player::UpdateJudgement( const float &fDeltaTime )
{
	if( m_fJudgementDisplayCountdown > 0.0 )
		m_fJudgementDisplayCountdown -= fDeltaTime;
	m_sprJudgement.Update( fDeltaTime );
}

void Player::DrawJudgement()
{
	if( m_fJudgementDisplayCountdown > 0.0 )
		m_sprJudgement.Draw();
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

	m_sprJudgement.SetZoom( 1.5f );
	m_sprJudgement.TweenTo( JUDGEMENT_DISPLAY_TIME/3.0,
							m_sprJudgement.GetX(), 
							m_sprJudgement.GetY()  );
}

void Player::SetComboX( int iNewX )
{
	m_sprCombo.SetXY( iNewX+40, COMBO_Y );
	m_textComboNum.SetXY(  iNewX-50, COMBO_Y );
}

void Player::UpdateCombo( const float &fDeltaTime )
{
	m_sprCombo.Update( fDeltaTime );
	m_textComboNum.Update( fDeltaTime );
}

void Player::DrawCombo()
{
	if( m_bComboVisible )
	{
		m_textComboNum.Draw();
		m_sprCombo.Draw();
	}
}


void Player::SetCombo( int iNewCombo )
{
	// new max combo
	if( iNewCombo > m_iMaxCombo )
		m_iMaxCombo = iNewCombo;

	m_iCurCombo = iNewCombo;

	if( iNewCombo <= 4 )
	{
		m_bComboVisible = FALSE;
	}
	else
	{
		m_bComboVisible = TRUE;

		m_textComboNum.SetText( ssprintf("%d", iNewCombo) );
		m_textComboNum.SetZoom( 1.0f + iNewCombo/200.0f ); 
		m_textComboNum.TweenTo( COMBO_TWEEN_TIME, m_textComboNum.GetX(), m_textComboNum.GetY() );
	}

}



void Player::SetLifeMeterX( int iNewX )
{
	m_sprLifeMeterFrame.SetXY( iNewX, LIFEMETER_Y );
	m_sprLifeMeterPills.SetXY( iNewX, LIFEMETER_PILLS_Y );
}

void Player::UpdateLifeMeter( const float &fDeltaTime )
{
	m_sprLifeMeterFrame.Update( fDeltaTime );
	m_sprLifeMeterPills.Update( fDeltaTime );
}

void Player::DrawLifeMeter( float fSongBeat )
{
	float fBeatPercentage = fSongBeat - (int)fSongBeat;
	int iOffsetStart = roundf( LIEFMETER_NUM_PILLS*fBeatPercentage );

	m_sprLifeMeterFrame.Draw();

	float iX = m_sprLifeMeterFrame.GetLeftEdge() + 27;
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
	m_textScoreNum.SetXY(  iNewX, SCORE_Y );
}

void Player::UpdateScore( const float &fDeltaTime )
{
	m_sprScoreFrame.Update( fDeltaTime );
	m_textScoreNum.Update( fDeltaTime );
}

void Player::DrawScore()
{
	m_textScoreNum.Draw();
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

	int iScoreToAdd = 0;
	switch( score )
	{
	case miss:											break;
	case boo:											break;
	case good:		iScoreToAdd = M * 100     + 100;	break;
	case great:		iScoreToAdd = M * M * 100 + 300;	break;
	case perfect:	iScoreToAdd = M * M * 300 + 500;	break;
	}
	m_fScore += iScoreToAdd;
	ASSERT( m_fScore > 0 );

	
	m_textScoreNum.SetText( ssprintf( "%9.0f", m_fScore ) );
}