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
#include "ThemeManager.h"
#include "GameTypes.h"
#include "ArrowEffects.h"


const float JUDGEMENT_Y			= CENTER_Y;
const float ARROWS_Y			= SCREEN_TOP + ARROW_SIZE * 1.5f;
const float HOLD_JUDGEMENT_Y	= ARROWS_Y + 80;
const float LIFEMETER_Y			= SCREEN_TOP + 30;
const float COMBO_Y				= CENTER_Y + 60;
const float SCORE_Y				= SCREEN_HEIGHT - 40;

const float HOLD_ARROW_NG_TIME	=	0.27f;


Player::Player()
{

	m_fSongBeat = 0;
	m_PlayerNumber = PLAYER_NONE;

	// init step elements
	for( int i=0; i<MAX_TAP_STEP_ELEMENTS; i++ )
	{
		m_TapStepsOriginal[i] = STEP_NONE;
		m_TapStepsRemaining[i] = STEP_NONE;
		m_TapStepScores[i] = TSS_NONE;
	}
	m_iNumHoldSteps = 0;


	this->AddActor( &m_GrayArrows );
	this->AddActor( &m_ColorArrowField );
	this->AddActor( &m_GhostArrows );
	this->AddActor( &m_Judgement );
	for( int c=0; c<MAX_NUM_COLUMNS; c++ )
		this->AddActor( &m_HoldJudgement[c] );
	this->AddActor( &m_Combo );
	this->AddActor( &m_LifeMeter );
	this->AddActor( &m_Score );


	// assist
	m_soundAssistTick.Load( THEME->GetPathTo(SOUND_ASSIST) );
}


void Player::Load( const Style& style, PlayerNumber player_no, const Steps& steps, const PlayerOptions& po )
{
	//RageLog( "Player::Load()", );

	m_Style = style;
	m_PlayerNumber = player_no;
	m_PlayerOptions = po;

	Steps steps2 = steps;

	if( !po.m_bAllowFreezeArrows )
		steps2.RemoveHoldSteps();

	steps2.Turn( po.m_TurnType );

	if( po.m_bLittle )
		steps2.MakeLittle();

	m_ColorArrowField.Load( style, steps2, po, 2, 10 );
	m_GrayArrows.Load( po, style );
	m_GhostArrows.Load( po, style );

	// load step elements
	for( int i=0; i<MAX_TAP_STEP_ELEMENTS; i++ )
	{
		m_TapStepsOriginal[i] = steps.m_TapSteps[i];
		m_TapStepsRemaining[i] = steps.m_TapSteps[i];
	}
	for( i=0; i<steps.m_iNumHoldSteps; i++ )
	{
		m_HoldSteps[i] = steps.m_HoldSteps[i];
	}
	m_iNumHoldSteps = steps.m_iNumHoldSteps;


	m_Combo.SetY( po.m_bReverseScroll ? SCREEN_HEIGHT - COMBO_Y : COMBO_Y );
	m_LifeMeter.SetY( LIFEMETER_Y );
	m_Judgement.SetY( po.m_bReverseScroll ? SCREEN_HEIGHT - JUDGEMENT_Y : JUDGEMENT_Y );
	for( int c=0; c<MAX_NUM_COLUMNS; c++ )
		m_HoldJudgement[c].SetY( po.m_bReverseScroll ? SCREEN_HEIGHT - HOLD_JUDGEMENT_Y : HOLD_JUDGEMENT_Y );
	m_Score.SetY( SCORE_Y );	

	m_ColorArrowField.SetY( po.m_bReverseScroll ? SCREEN_HEIGHT - ARROWS_Y : ARROWS_Y );
	m_GrayArrows.SetY( po.m_bReverseScroll ? SCREEN_HEIGHT - ARROWS_Y : ARROWS_Y );
	m_GhostArrows.SetY( po.m_bReverseScroll ? SCREEN_HEIGHT - ARROWS_Y : ARROWS_Y );
	

}

void Player::Update( float fDeltaTime, float fSongBeat, float fMaxBeatDifference )
{
	//RageLog( "Player::Update(%f, %f, %f)", fDeltaTime, fSongBeat, fMaxBeatDifference );


	//
	// Check for TapStep misses
	//
	int iNumMisses = UpdateStepsMissedOlderThan( m_fSongBeat-fMaxBeatDifference );
	if( iNumMisses > 0 )
	{
		m_Judgement.SetJudgement( TSS_MISS );
		m_Combo.EndCombo();
		for( int i=0; i<iNumMisses; i++ )
			m_LifeMeter.ChangeLife( TSS_MISS );
	}


	//
	// update HoldSteps logic
	//
	for( int i=0; i<m_iNumHoldSteps; i++ )		// for each HoldStep
	{
		HoldStep &hs = m_HoldSteps[i];
		HoldStepScore &hss = m_HoldStepScores[i];

		if( hss.m_Result != HSR_NONE )	// if this HoldStep already has a result
			continue;	// we don't need to update the logic for this one

		float fStartBeat = StepIndexToBeat( (float)hs.m_iStartIndex );
		float fEndBeat = StepIndexToBeat( (float)hs.m_iEndIndex );


		// update the life
		if( fStartBeat < m_fSongBeat && m_fSongBeat < fEndBeat )	// if the song beat is in the range of this hold
		{
			PlayerInput PlayerI = { m_PlayerNumber, hs.m_TapStep };
			bool bIsHoldingButton = GAMEINFO->IsButtonDown( PlayerI );
			if( bIsHoldingButton )
			{
				hss.m_fLife += fDeltaTime/HOLD_ARROW_NG_TIME;
				hss.m_fLife = min( hss.m_fLife, 1 );	// clamp
				int iCol = m_Style.TapStepToColumnNumber( hs.m_TapStep );
				m_GhostArrows.HoldStep( iCol );		// update the "electric ghost" effect
			}
			else	// !bIsHoldingButton
			{
				hss.m_fLife -= fDeltaTime/HOLD_ARROW_NG_TIME;
				hss.m_fLife = max( hss.m_fLife, 0 );	// clamp
			}
			m_ColorArrowField.SetHoldStepLife( i, hss.m_fLife );	// update the ColorArrowField display
		}

		// check for NG
		if( hss.m_fLife == 0 )	// the player has not pressed the button for a long time!
		{
			hss.m_Result = HSR_NG;
			int iCol = m_Style.TapStepToColumnNumber( hs.m_TapStep );
			m_HoldJudgement[iCol].SetHoldJudgement( HSR_NG );
		}

		// check for OK
		if( m_fSongBeat > fEndBeat )	// if this HoldStep is in the past
		{
			// this implies that hss.m_fLife > 0, or else we would have marked it NG above
			hss.m_fLife = 1;
			hss.m_Result = HSR_OK;
			int iCol = m_Style.TapStepToColumnNumber( hs.m_TapStep );
			m_HoldJudgement[iCol].SetHoldJudgement( HSR_OK );
			m_ColorArrowField.SetHoldStepLife( i, hss.m_fLife );	// update the ColorArrowField display
		}
	}



	ActorFrame::Update( fDeltaTime );

	m_LifeMeter.SetBeat( fSongBeat );

	m_GrayArrows.Update( fDeltaTime, fSongBeat );
	m_ColorArrowField.Update( fDeltaTime, fSongBeat );
	m_GhostArrows.Update( fDeltaTime, fSongBeat );

	m_fSongBeat = fSongBeat;	// save song beat

}

void Player::RenderPrimitives()
{
	D3DXMATRIX matOldView, matOldProj;

	if( m_PlayerOptions.m_EffectType == PlayerOptions::EFFECT_STARS )
	{
		// turn off Z Buffering
		SCREEN->GetDevice()->SetRenderState( D3DRS_ZENABLE,      FALSE );
		SCREEN->GetDevice()->SetRenderState( D3DRS_ZWRITEENABLE, FALSE );

		// save old view and projection
		SCREEN->GetDevice()->GetTransform( D3DTS_VIEW, &matOldView );
		SCREEN->GetDevice()->GetTransform( D3DTS_PROJECTION, &matOldProj );


		// construct view and project matrix
		D3DXMATRIX matNewView;
		D3DXMatrixLookAtLH( &matNewView, &D3DXVECTOR3( CENTER_X, GetY()+800.0f, 300.0f ), 
										 &D3DXVECTOR3( CENTER_X
										 , GetY()+400.0f,   0.0f ), 
										 &D3DXVECTOR3(          0.0f,         -1.0f,   0.0f ) );
		SCREEN->GetDevice()->SetTransform( D3DTS_VIEW, &matNewView );

		D3DXMATRIX matNewProj;
		D3DXMatrixPerspectiveFovLH( &matNewProj, D3DX_PI/4.0f, SCREEN_WIDTH/(float)SCREEN_HEIGHT, 0.0f, 1000.0f );
		SCREEN->GetDevice()->SetTransform( D3DTS_PROJECTION, &matNewProj );
	}

	m_GrayArrows.Draw();
	m_ColorArrowField.Draw();
	m_GhostArrows.Draw();

	if( m_PlayerOptions.m_EffectType == PlayerOptions::EFFECT_STARS )
	{
		// restire old view and projection
		SCREEN->GetDevice()->SetTransform( D3DTS_VIEW, &matOldView );
		SCREEN->GetDevice()->SetTransform( D3DTS_PROJECTION, &matOldProj );

		// turn Z Buffering back on
		SCREEN->GetDevice()->SetRenderState( D3DRS_ZENABLE,      TRUE );
		SCREEN->GetDevice()->SetRenderState( D3DRS_ZWRITEENABLE, TRUE );
	}


	m_Judgement.Draw();
	for( int c=0; c<m_Style.m_iNumColumns; c++ )
		m_HoldJudgement[c].Draw();
	m_Combo.Draw();
	m_LifeMeter.Draw();
	m_Score.Draw();


}

void Player::CrossedIndex( int iIndex )
{

}

bool Player::IsThereANoteAtIndex( int iIndex )
{
	return m_TapStepsOriginal[iIndex] != STEP_NONE;
}




void Player::HandlePlayerStep( float fSongBeat, TapStep player_step, float fMaxBeatDiff )
{
	//RageLog( "Player::HandlePlayerStep()" );

	// update gray arrows
	int iColumnNum = m_Style.TapStepToColumnNumber( player_step );
	
	m_GrayArrows.Step( iColumnNum );

	CheckForCompleteStep( fSongBeat, player_step, fMaxBeatDiff );

	//
	// check if we stepped on the TapStep part of a HoldStep
	//
	for( int i=0; i<m_iNumHoldSteps; i++ )	// for each HoldStep
	{
		HoldStep& hs		= m_HoldSteps[i];
		HoldStepScore& hss	= m_HoldStepScores[i];

		if( hss.m_Result != HSR_NONE )	// if this note already has a score
			continue;	// we don't need to update its logic

		if( hss.m_TapStepScore != TSS_NONE )	// the TapStep already has a score
			continue;	// no need to continue;
			
		if( player_step == m_HoldSteps[i].m_TapStep ) // the player's step is the same as this HoldStep
		{
			float fBeatDifference = fabsf( StepIndexToBeat(hs.m_iStartIndex) - fSongBeat );
			
			if( fBeatDifference <= fMaxBeatDiff )
			{
				float fBeatsUntilStep = StepIndexToBeat( (float)hs.m_iStartIndex ) - fSongBeat;
				float fPercentFromPerfect = fabsf( fBeatsUntilStep / fMaxBeatDiff );

				//RageLog( "fBeatsUntilStep: %f, fPercentFromPerfect: %f", 
				//		 fBeatsUntilStep, fPercentFromPerfect );

				// compute what the score should be for the note we stepped on
				TapStepScore &score = hss.m_TapStepScore;
				if(		 fPercentFromPerfect < 0.25f )	score = TSS_PERFECT;
				else if( fPercentFromPerfect < 0.50f )	score = TSS_GREAT;
				else if( fPercentFromPerfect < 0.75f )	score = TSS_GOOD;
				else									score = TSS_BOO;

				// update the judgement, score, and life
				m_Judgement.SetJudgement( score );
				m_Score.AddToScore( score, m_Combo.GetCurrentCombo() );
				m_LifeMeter.ChangeLife( score );

				// show the gray arrow ghost
				int iColNum = m_Style.TapStepToColumnNumber( player_step );
				m_GhostArrows.TapStep( iColNum, score, m_Combo.GetCurrentCombo() > 100 );

				// update the combo display
				switch( score )
				{
				case TSS_PERFECT:
				case TSS_GREAT:
					m_Combo.ContinueCombo();
					break;
				case TSS_GOOD:
				case TSS_BOO:
					m_Combo.EndCombo();
					break;
				}


			}
		}
	}

}


void Player::CheckForCompleteStep( float fSongBeat, TapStep player_step, float fMaxBeatDiff )
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
		iCurrentIndexEarlier	= clamp( iCurrentIndexEarlier, 0, MAX_TAP_STEP_ELEMENTS-1 );
		iCurrentIndexLater		= clamp( iCurrentIndexLater,   0, MAX_TAP_STEP_ELEMENTS-1 );

		////////////////////////////
		// check the step to the left of iIndexStartLookingAt
		////////////////////////////
		//RageLog( "Checking steps[%d]", iCurrentIndexEarlier );
		if( m_TapStepsRemaining[iCurrentIndexEarlier] & player_step )	// these steps overlap
		{
			m_TapStepsRemaining[iCurrentIndexEarlier] &= ~player_step;	// subtract player_step
			if( m_TapStepsRemaining[iCurrentIndexEarlier] == 0 )	{		// did this complete the step?
				OnCompleteStep( fSongBeat, player_step, fMaxBeatDiff, iCurrentIndexEarlier );
				return;
			}
		}

		////////////////////////////
		// check the step to the right of iIndexStartLookingAt
		////////////////////////////
		//RageLog( "Checking steps[%d]", iCurrentIndexLater );
		if( m_TapStepsRemaining[iCurrentIndexLater] & player_step )		// these steps overlap
		{
			m_TapStepsRemaining[iCurrentIndexLater] &= ~player_step;		// subtract player_step
			if( m_TapStepsRemaining[iCurrentIndexLater] == 0 ) {			// did this complete the step?
				OnCompleteStep( fSongBeat, player_step, fMaxBeatDiff, iCurrentIndexLater );
				return;
			}
		}
	}
}

void Player::OnCompleteStep( float fSongBeat, TapStep player_step, float fMaxBeatDiff, int iIndexThatWasSteppedOn )
{
	float fStepBeat = StepIndexToBeat( (float)iIndexThatWasSteppedOn );

 
	float fBeatsUntilStep = fStepBeat - fSongBeat;
	float fPercentFromPerfect = fabsf( fBeatsUntilStep / fMaxBeatDiff );

	//RageLog( "fBeatsUntilStep: %f, fPercentFromPerfect: %f", 
	//		 fBeatsUntilStep, fPercentFromPerfect );

	// compute what the score should be for the note we stepped on
	TapStepScore &score = m_TapStepScores[iIndexThatWasSteppedOn];

	if(		 fPercentFromPerfect < 0.25f )	score = TSS_PERFECT;
	else if( fPercentFromPerfect < 0.50f )	score = TSS_GREAT;
	else if( fPercentFromPerfect < 0.75f )	score = TSS_GOOD;
	else									score = TSS_BOO;

	// update the judgement, score, and life
	m_Judgement.SetJudgement( score );
	m_Score.AddToScore( score, m_Combo.GetCurrentCombo() );
	m_LifeMeter.ChangeLife( score );


	// remove this row from the ColorArrowField
	m_ColorArrowField.RemoveTapStepRow( iIndexThatWasSteppedOn );

	// check to see if this completes a row of notes
	for( int c=0; c<m_Style.m_iNumColumns; c++ )	// for each column
	{
		if( m_TapStepsOriginal[iIndexThatWasSteppedOn] & m_Style.m_ColumnToTapStep[c] )	// if this colum was part of the original step
			m_GhostArrows.TapStep( c, score, m_Combo.GetCurrentCombo()>100 );	// show the ghost arrow for this column
	}

	// update the combo display
	switch( score )
	{
	case TSS_PERFECT:
	case TSS_GREAT:
		m_Combo.ContinueCombo();
		break;
	case TSS_GOOD:
	case TSS_BOO:
		m_Combo.EndCombo();
		break;
	}
}


ScoreSummary Player::GetScoreSummary()
{
	ScoreSummary scoreSummary;

	for( int i=0; i<MAX_TAP_STEP_ELEMENTS; i++ ) 
	{
		switch( m_TapStepScores[i] )
		{
		case TSS_PERFECT:	scoreSummary.perfect++;		break;
		case TSS_GREAT:		scoreSummary.great++;		break;
		case TSS_GOOD:		scoreSummary.good++;		break;
		case TSS_BOO:		scoreSummary.boo++;			break;
		case TSS_MISS:		scoreSummary.miss++;		break;
		case TSS_NONE:									break;
		default:		ASSERT( false );
		}
	}
	for( i=0; i<m_iNumHoldSteps; i++ ) 
	{
		switch( m_HoldStepScores[i].m_TapStepScore )
		{
		case TSS_PERFECT:	scoreSummary.perfect++;		break;
		case TSS_GREAT:		scoreSummary.great++;		break;
		case TSS_GOOD:		scoreSummary.good++;		break;
		case TSS_BOO:		scoreSummary.boo++;			break;
		case TSS_MISS:		scoreSummary.miss++;		break;
		case TSS_NONE:									break;
		default:		ASSERT( false );
		}
		switch( m_HoldStepScores[i].m_Result )
		{
		case HSR_NG:		scoreSummary.ng++;		break;
		case HSR_OK:		scoreSummary.ok++;		break;
		case HSR_NONE:								break;
		default:		ASSERT( false );
		}
	}
	scoreSummary.max_combo = m_Combo.GetMaxCombo();
	scoreSummary.score = m_Score.GetScore();
	
	return scoreSummary;
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
		if( m_TapStepsRemaining[i] != 0  &&  m_TapStepScores[i] != TSS_MISS )
		{
			m_TapStepScores[i] = TSS_MISS;
			iNumMissesFound++;
			m_LifeMeter.ChangeLife( TSS_MISS );
		}
	}

	return iNumMissesFound;
}







