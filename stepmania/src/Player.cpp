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



#define STEP_DOWN_TIME	0.05f

#define SCORE_ADD_PERFECT	700
#define SCORE_ADD_GREAT		400
#define SCORE_ADD_GOOD		200
#define SCORE_ADD_BOO		100

#define LIFE_PERFECT	 0.015f
#define LIFE_GREAT		 0.008f
#define LIFE_GOOD		 0.000f
#define LIFE_BOO		-0.015f
#define LIFE_MISS		-0.060f

#define SPRITE_COLOR_ARROW_LEFT		"Sprites\\Color Arrow Left.sprite"
#define SPRITE_COLOR_ARROW_DOWN		"Sprites\\Color Arrow Down.sprite"
#define SPRITE_COLOR_ARROW_UP		"Sprites\\Color Arrow Up.sprite"
#define SPRITE_COLOR_ARROW_RIGHT	"Sprites\\Color Arrow Right.sprite"


const int ARROW_X_OFFSET[4] = {
	(int)(64*-1.5),
	(int)(64*-0.5),
	(int)(64* 0.5),
	(int)(64* 1.5)
};

#define GRAY_ARROW_Y						((int)(64* 1.5))
#define GRAY_ARROW_POP_UP_TIME				0.30f
#define ARROW_HEIGHT						70
#define NUM_FRAMES_IN_COLOR_ARROW_SPRITE	12

/*
const CString SPRITE_COLOR_ARROW[4] = {
	"Sprites\\Color Arrow Left.sprite",
	"Sprites\\Color Arrow Down.sprite",
	"Sprites\\Color Arrow Up.sprite",
	"Sprites\\Color Arrow Right.sprite"
};
*/
const CString SPRITE_COLOR_ARROW = "Sprites\\Color Arrow Left.sprite";
const CString SPRITE_GRAY_ARROW = "Sprites\\Gray Arrow Left.sprite";
const float SPRITE_ARROW_ROTATION[4] = {
	0,
	0-D3DX_PI/2.0f,
	0+D3DX_PI/2.0f, 
	0+D3DX_PI
};


#define JUDGEMENT_DISPLAY_TIME	1.0f
#define JUDGEMENT_SPRITE		"Sprites\\Judgement.sprite"
#define JUDGEMENT_Y				CENTER_Y

#define FONT_COMBO				"Fonts\\Font - Arial Bold numbers 30px.font"

#define COMBO_TWEEN_TIME		0.5f
#define COMBO_SPRITE			"Sprites\\Combo.sprite"
#define COMBO_Y					(CENTER_Y+60)


#define LIEFMETER_NUM_PILLS		17
#define LIFEMETER_FRAME_SPRITE	"Sprites\\Life Meter Frame.sprite"
#define LIFEMETER_PILLS_SPRITE	"Sprites\\Life Meter Pills.sprite"
#define LIFEMETER_Y				30
#define LIFEMETER_PILLS_Y		(LIFEMETER_Y+2)
const float PILL_OFFSET_Y[LIEFMETER_NUM_PILLS] = {
	0.3f, 0.7f, 1.0f, 0.7f, 0.3f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f
};

#define FONT_SCORE		"Fonts\\Font - Arial Bold numbers 30px.font"
#define SCORE_FRAME_TEXTURE	"Textures\\Score Frame 1x1.png"
#define SCORE_Y				(480-40)


Player::Player()
{
	m_iCurCombo = 0;
	m_iMaxCombo = 0;
	m_fLifePercentage = 0.50f;
	m_fScore = 0.0f;

	for( int i=0; i<MAX_STEP_ELEMENTS; i++ ) {
		m_OriginalStep[i] = 0;
		m_LeftToStepOn[i] = 0;
		m_StepScore[i] = none;
		m_iColorArrowFrameOffset[i] = 0;
	}

	for( i=0; i<4; i++ ) {
		// gray arrows
		m_sprGrayArrow[i].LoadFromSpriteFile( SPRITE_GRAY_ARROW );
		m_sprGrayArrow[i].SetRotation( SPRITE_ARROW_ROTATION[i] );

		m_sprGrayArrowGhost[i].LoadFromSpriteFile( SPRITE_GRAY_ARROW );
		m_sprGrayArrowGhost[i].SetRotation( SPRITE_ARROW_ROTATION[i] );
		m_sprGrayArrowGhost[i].StopAnimating();
		m_sprGrayArrowGhost[i].SetState( 1 );
		m_sprGrayArrowGhost[i].SetColor( D3DXCOLOR(1,1,1,0) );

		// color arrows
		m_sprColorArrow[i].LoadFromSpriteFile( SPRITE_COLOR_ARROW_LEFT );
		m_sprColorArrow[i].StopAnimating();
		m_sprColorArrow[i].SetRotation( SPRITE_ARROW_ROTATION[i] );
	}

	// judgement
	m_fJudgementDisplayCountdown = 0;
	m_sprJudgement.LoadFromSpriteFile( JUDGEMENT_SPRITE );

	// combo
	m_bComboVisible = FALSE;
	m_sprCombo.LoadFromSpriteFile( COMBO_SPRITE );
	m_textComboNum.LoadFromFontFile( FONT_COMBO );
	m_textComboNum.SetText( "" );

	// life meter
	m_sprLifeMeterFrame.LoadFromSpriteFile( LIFEMETER_FRAME_SPRITE );
	m_sprLifeMeterPills.LoadFromSpriteFile( LIFEMETER_PILLS_SPRITE );

	// score
	m_sprScoreFrame.LoadFromTexture( SCORE_FRAME_TEXTURE );
	m_textScoreNum.LoadFromFontFile( FONT_SCORE );
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
	//RageLog( "Player::Update(%f)", fDeltaTime );

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
	if( player_step & STEP_P1_LEFT )	GrayArrowStep( 0 );
	if( player_step & STEP_P1_DOWN )	GrayArrowStep( 1 );
	if( player_step & STEP_P1_UP )		GrayArrowStep( 2 );
	if( player_step & STEP_P1_RIGHT )	GrayArrowStep( 3 );

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
	if( player_step & STEP_P1_LEFT )	GrayArrowGhostStep( 0 );
	if( player_step & STEP_P1_DOWN )	GrayArrowGhostStep( 1 );
	if( player_step & STEP_P1_UP )		GrayArrowGhostStep( 2 );
	if( player_step & STEP_P1_RIGHT )	GrayArrowGhostStep( 3 );

	float fBeatsUntilStep = fStepBeat - fSongBeat;
	float fPercentFromPerfect = (float)fabs( fBeatsUntilStep / fMaxBeatDiff );

	//RageLog( "fBeatsUntilStep: %f, fPercentFromPerfect: %f", 
	//		 fBeatsUntilStep, fPercentFromPerfect );

	// compute what the score should be for the note we stepped on
	StepScore &score = m_StepScore[iIndexThatWasSteppedOn];

	if(		 fPercentFromPerfect < 0.20f )	score = perfect;
	else if( fPercentFromPerfect < 0.45f )	score = great;
	else if( fPercentFromPerfect < 0.75f )	score = good;
	else									score = boo;

	// update the judgement, score, and life
	SetJudgement( score );
	ChangeScore( score );
	ChangeLife( score );

	// update the combo display
	switch( score )
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

	for( int i=iStartCheckingAt; i<iMissIfOlderThanThisIndex; i++ )
	{
//		RageLog( "Step %d: status == %d, score == %d", i, StatusArray[i], Score[i] );
		if( m_LeftToStepOn[i] != 0 )
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



void Player::UpdateGrayArrows( const float &fDeltaTime )
{
	// gray arrows
	for( int i=0; i<4; i++ ) {
		m_sprGrayArrow[i].Update( fDeltaTime );
		m_sprGrayArrowGhost[i].Update( fDeltaTime );
	}
}

void Player::DrawGrayArrows()
{
	for( int i=0; i<4; i++ )
		m_sprGrayArrow[i].Draw();
}

void Player::SetGrayArrowsX( int iNewX )
{
	for( int i=0; i<4; i++ )
		m_sprGrayArrow[i].SetXY(  iNewX + ARROW_X_OFFSET[i], GRAY_ARROW_Y );
}

void Player::SetColorArrowsX( int iNewX )
{

}

void Player::GrayArrowStep( int index )
{
	m_sprGrayArrow[index].SetZoom( 0.50 );
	m_sprGrayArrow[index].SetTweening( GRAY_ARROW_POP_UP_TIME );
	m_sprGrayArrow[index].SetTweenZoom( 1.0f );
}

void Player::GrayArrowGhostStep( int index )
{
	m_sprGrayArrowGhost[index].SetXY( m_iArrowsCenterX + ARROW_X_OFFSET[index], GRAY_ARROW_Y );
	m_sprGrayArrowGhost[index].SetZoom( 1 );
	m_sprGrayArrowGhost[index].SetColor( D3DXCOLOR(1,1,0.5f,1) );
	m_sprGrayArrowGhost[index].SetTweening( 0.3f );
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

	for( int i=iIndexFirstArrowToDraw; i<=iIndexLastArrowToDraw; i++ )
	{				
		if( m_LeftToStepOn[i] != 0 )	// if there is a step here and it hasn't been stepped 
		{		
			int iYPos = GetColorArrowYPos( i, fSongBeat );

			// calculate which frame to display
			int iFrameNo = iBaseFrameNo + m_iColorArrowFrameOffset[i];
			iFrameNo = iFrameNo % NUM_FRAMES_IN_COLOR_ARROW_SPRITE;
				
			//RageLog( "iYPos: %d, iFrameNo: %d, m_OriginalStep[i]: %d", iYPos, iFrameNo, m_OriginalStep[i] );


			if( m_OriginalStep[i] & STEP_P1_LEFT )
			{
				//RageLog( "Draw a left arrow at %d, %d", m_iArrowsCenterX + ARROW_X_OFFSET[0], iYPos );
				m_sprColorArrow[0].SetXY( m_iArrowsCenterX + ARROW_X_OFFSET[0], iYPos );
				m_sprColorArrow[0].SetState( iFrameNo );
				m_sprColorArrow[0].Draw();
			}
			if( m_OriginalStep[i] & STEP_P1_DOWN )
			{
				m_sprColorArrow[1].SetXY( m_iArrowsCenterX + ARROW_X_OFFSET[1], iYPos );
				m_sprColorArrow[1].SetState( iFrameNo );
				m_sprColorArrow[1].Draw();
			}
			if( m_OriginalStep[i] & STEP_P1_UP )
			{
				m_sprColorArrow[2].SetXY( m_iArrowsCenterX + ARROW_X_OFFSET[2], iYPos );
				m_sprColorArrow[2].SetState( iFrameNo );
				m_sprColorArrow[2].Draw();
			}
			if( m_OriginalStep[i] & STEP_P1_RIGHT )
			{
				m_sprColorArrow[3].SetXY( m_iArrowsCenterX + ARROW_X_OFFSET[3], iYPos );
				m_sprColorArrow[3].SetState( iFrameNo );
				m_sprColorArrow[3].Draw();
			}
		}	// end if there is a step
	}	// end foreach arrow to draw

	for( i=0; i<4; i++ )
	{
		m_sprGrayArrowGhost[i].Draw();
	}
}





int Player::GetColorArrowYPos( int iStepIndex, float fSongBeat )
{
	float fBeatsUntilStep = StepIndexToBeat( iStepIndex ) - fSongBeat;
	return (int)(fBeatsUntilStep * ARROW_HEIGHT) + GRAY_ARROW_Y;
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
	m_sprJudgement.TweenTo( JUDGEMENT_DISPLAY_TIME/2.0,
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


void Player::ChangeScore( StepScore score )
{
	int iScoreToAdd;
	switch( score )
	{
	case perfect:	iScoreToAdd = SCORE_ADD_PERFECT;	break;
	case great:		iScoreToAdd = SCORE_ADD_GREAT;		break;
	case good:		iScoreToAdd = SCORE_ADD_GOOD;		break;
	case boo:		iScoreToAdd = SCORE_ADD_BOO;		break;
	case miss:											break;
	}
	m_fScore += iScoreToAdd * (1 + m_iCurCombo/200.0f);

	// multiply the combo bonus
	//m_fScore *= SCORE_MULT_BOO;
	
	m_textScoreNum.SetText( ssprintf( "%9.0f", m_fScore ) );
}