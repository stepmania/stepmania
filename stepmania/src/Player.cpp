#include "stdafx.h"
/*
-----------------------------------------------------------------------------
 Class: Pattern

 Desc: See header.

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
	for( int i=0; i<MAX_TAP_NOTE_ELEMENTS; i++ )
	{
		m_TapNotesOriginal[i] = NOTE_NONE;
		m_TapNotesRemaining[i] = NOTE_NONE;
		m_TapNoteScores[i] = TNS_NONE;
	}
	m_iNumHoldNotes = 0;


	this->AddActor( &m_GrayArrows );
	this->AddActor( &m_ColorArrowField );
	this->AddActor( &m_GhostArrows );
	this->AddActor( &m_Judgement );
	for( int c=0; c<MAX_NUM_COLUMNS; c++ )
		this->AddActor( &m_HoldJudgement[c] );
	this->AddActor( &m_Combo );
	this->AddActor( &m_LifeMeter );
	this->AddActor( &m_Score );
}


void Player::Load( const StyleDef& StyleDef, PlayerNumber player_no, const Pattern& pattern, const PlayerOptions& po )
{
	//HELPER.Log( "Player::Load()", );

	m_Style = StyleDef;
	m_PlayerNumber = player_no;
	m_PlayerOptions = po;

	Pattern pattern2 = pattern;

	if( !po.m_bAllowFreezeArrows )
		pattern2.RemoveHoldNotes();

	pattern2.Turn( po.m_TurnType );

	if( po.m_bLittle )
		pattern2.MakeLittle();

	m_ColorArrowField.Load( StyleDef, pattern2, po, 1.5f, 5.5f, ColorArrowField::MODE_DANCING );
	m_GrayArrows.Load( po, StyleDef );
	m_GhostArrows.Load( po, StyleDef );

	// load step elements
	for( int i=0; i<MAX_TAP_NOTE_ELEMENTS; i++ )
	{
		m_TapNotesOriginal[i] = pattern2.m_TapNotes[i];
		m_TapNotesRemaining[i] = pattern2.m_TapNotes[i];
	}
	for( i=0; i<pattern2.m_iNumHoldNotes; i++ )
	{
		m_HoldNotes[i] = pattern2.m_HoldNotes[i];
	}
	m_iNumHoldNotes = pattern2.m_iNumHoldNotes;


	m_Combo.SetY( po.m_bReverseScroll ? SCREEN_HEIGHT - COMBO_Y : COMBO_Y );
	m_LifeMeter.SetY( LIFEMETER_Y );
	m_Judgement.SetY( po.m_bReverseScroll ? SCREEN_HEIGHT - JUDGEMENT_Y : JUDGEMENT_Y );
	for( int c=0; c<MAX_NUM_COLUMNS; c++ )
		m_HoldJudgement[c].SetY( po.m_bReverseScroll ? SCREEN_HEIGHT - HOLD_JUDGEMENT_Y : HOLD_JUDGEMENT_Y );
	m_Score.SetY( SCORE_Y );	

	m_ColorArrowField.SetY( po.m_bReverseScroll ? SCREEN_HEIGHT - ARROWS_Y : ARROWS_Y );
	m_GrayArrows.SetY( po.m_bReverseScroll ? SCREEN_HEIGHT - ARROWS_Y : ARROWS_Y );
	m_GhostArrows.SetY( po.m_bReverseScroll ? SCREEN_HEIGHT - ARROWS_Y : ARROWS_Y );
	
	// Load options into Life Meter
	m_LifeMeter.SetPlayerOptions(po);
}

void Player::Update( float fDeltaTime, float fSongBeat, float fMaxBeatDifference )
{
	//HELPER.Log( "Player::Update(%f, %f, %f)", fDeltaTime, fSongBeat, fMaxBeatDifference );


	//
	// Check for TapNote misses
	//
	int iNumMisses = UpdateStepsMissedOlderThan( m_fSongBeat-fMaxBeatDifference );
	if( iNumMisses > 0 )
	{
		m_Judgement.SetJudgement( TNS_MISS );
		m_Combo.EndCombo();
		for( int i=0; i<iNumMisses; i++ )
			m_LifeMeter.ChangeLife( TNS_MISS );
	}


	//
	// update HoldNotes logic
	//
	for( int i=0; i<m_iNumHoldNotes; i++ )		// for each HoldNote
	{
		HoldNote &hn = m_HoldNotes[i];
		HoldNoteScore &hns = m_HoldNoteScores[i];

		if( hns.m_Result != HNR_NONE )	// if this HoldNote already has a result
			continue;	// we don't need to update the logic for this one

		float fStartBeat = NoteIndexToBeat( (float)hn.m_iStartIndex );
		float fEndBeat = NoteIndexToBeat( (float)hn.m_iEndIndex );


		// update the life
		if( fStartBeat < m_fSongBeat && m_fSongBeat < fEndBeat )	// if the song beat is in the range of this hold
		{
			PlayerInput PlayerI( m_PlayerNumber, hn.m_TapNote );
			bool bIsHoldingButton = PREFS->IsButtonDown( PlayerI );
			if( bIsHoldingButton )
			{
				hns.m_fLife += fDeltaTime/HOLD_ARROW_NG_TIME;
				hns.m_fLife = min( hns.m_fLife, 1 );	// clamp
				int iCol = m_Style.TapNoteToColumnNumber( hn.m_TapNote );
				m_GhostArrows.HoldNote( iCol );		// update the "electric ghost" effect
			}
			else	// !bIsHoldingButton
			{
				hns.m_fLife -= fDeltaTime/HOLD_ARROW_NG_TIME;
				hns.m_fLife = max( hns.m_fLife, 0 );	// clamp
			}
			m_ColorArrowField.SetHoldNoteLife( i, hns.m_fLife );	// update the ColorArrowField display
		}

		// check for NG
		if( hns.m_fLife == 0 )	// the player has not pressed the button for a long time!
		{
			hns.m_Result = HNR_NG;
			int iCol = m_Style.TapNoteToColumnNumber( hn.m_TapNote );
			m_HoldJudgement[iCol].SetHoldJudgement( HNR_NG );
		}

		// check for OK
		if( m_fSongBeat > fEndBeat )	// if this HoldNote is in the past
		{
			// this implies that hns.m_fLife > 0, or else we would have marked it NG above
			hns.m_fLife = 1;
			hns.m_Result = HNR_OK;
			int iCol = m_Style.TapNoteToColumnNumber( hn.m_TapNote );
			m_HoldJudgement[iCol].SetHoldJudgement( HNR_OK );
			m_ColorArrowField.SetHoldNoteLife( i, hns.m_fLife );	// update the ColorArrowField display
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

	if( m_PlayerOptions.m_EffectType == PlayerOptions::EFFECT_SPACE )
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

	if( m_PlayerOptions.m_EffectType == PlayerOptions::EFFECT_SPACE )
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

bool Player::IsThereANoteAtIndex( int iIndex )
{
	if( m_TapNotesOriginal[iIndex] != NOTE_NONE )
		return true;

	for( int i=0; i<m_iNumHoldNotes; i++ )	// for each HoldNote
	{
		if( m_HoldNotes[i].m_iStartIndex == iIndex )
			return true;
	}

	return false;
}




void Player::HandlePlayerStep( float fSongBeat, TapNote player_step, float fMaxBeatDiff )
{
	//HELPER.Log( "Player::HandlePlayerStep()" );

	int iColumnNum = m_Style.TapNoteToColumnNumber( player_step );
	if( iColumnNum == -1 )	// if this TapNote is not used in the current StyleDef
		return;		// ignore the step

	m_GrayArrows.Step( iColumnNum );

	CheckForCompleteStep( fSongBeat, player_step, fMaxBeatDiff );

	//
	// check if we stepped on the TapNote part of a HoldNote
	//
	for( int i=0; i<m_iNumHoldNotes; i++ )	// for each HoldNote
	{
		HoldNote& hn		= m_HoldNotes[i];
		HoldNoteScore& hns	= m_HoldNoteScores[i];

		if( hns.m_Result != HNR_NONE )	// if this note already has a score
			continue;	// we don't need to update its logic

		if( hns.m_TapNoteScore != TNS_NONE )	// the TapNote already has a score
			continue;	// no need to continue;
			
		if( player_step == m_HoldNotes[i].m_TapNote ) // the player's step is the same as this HoldNote
		{
			float fBeatDifference = fabsf( NoteIndexToBeat(hn.m_iStartIndex) - fSongBeat );
			
			if( fBeatDifference <= fMaxBeatDiff )
			{
				float fBeatsUntilStep = NoteIndexToBeat( (float)hn.m_iStartIndex ) - fSongBeat;
				float fPercentFromPerfect = fabsf( fBeatsUntilStep / fMaxBeatDiff );

				//HELPER.Log( "fBeatsUntilStep: %f, fPercentFromPerfect: %f", 
				//		 fBeatsUntilStep, fPercentFromPerfect );

				// compute what the score should be for the note we stepped on
				TapNoteScore &score = hns.m_TapNoteScore;
				if(		 fPercentFromPerfect < 0.25f )	score = TNS_PERFECT;
				else if( fPercentFromPerfect < 0.50f )	score = TNS_GREAT;
				else if( fPercentFromPerfect < 0.75f )	score = TNS_GOOD;
				else									score = TNS_BOO;

				// update the judgement, score, and life
				m_Judgement.SetJudgement( score );
				m_Score.AddToScore( score, m_Combo.GetCurrentCombo() );
				m_LifeMeter.ChangeLife( score );

				// show the gray arrow ghost
				int iColNum = m_Style.TapNoteToColumnNumber( player_step );
				m_GhostArrows.TapNote( iColNum, score, m_Combo.GetCurrentCombo() > 100 );

				// update the combo display
				switch( score )
				{
				case TNS_PERFECT:
				case TNS_GREAT:
					m_Combo.ContinueCombo();
					break;
				case TNS_GOOD:
				case TNS_BOO:
					m_Combo.EndCombo();
					break;
				}


			}
		}
	}

}


void Player::CheckForCompleteStep( float fSongBeat, TapNote player_step, float fMaxBeatDiff )
{
	//HELPER.Log( "Player::CheckForCompleteStep()" );

	// look for the closest matching step
	int iIndexStartLookingAt = BeatToNoteIndex( fSongBeat );
	int iNumElementsToExamine = BeatToNoteIndex( fMaxBeatDiff );	// number of elements to examine on either end of iIndexStartLookingAt
	
	//HELPER.Log( "iIndexStartLookingAt = %d, iNumElementsToExamine = %d", iIndexStartLookingAt, iNumElementsToExamine );

	// Start at iIndexStartLookingAt and search outward.  The first one that overlaps the player's step is the closest match.
	for( int delta=0; delta <= iNumElementsToExamine; delta++ )
	{
		int iCurrentIndexEarlier = iIndexStartLookingAt - delta;
		int iCurrentIndexLater   = iIndexStartLookingAt + delta;

		// silly check to make sure we don't go out of bounds
		iCurrentIndexEarlier	= clamp( iCurrentIndexEarlier, 0, MAX_TAP_NOTE_ELEMENTS-1 );
		iCurrentIndexLater		= clamp( iCurrentIndexLater,   0, MAX_TAP_NOTE_ELEMENTS-1 );

		////////////////////////////
		// check the step to the left of iIndexStartLookingAt
		////////////////////////////
		//HELPER.Log( "Checking Pattern[%d]", iCurrentIndexEarlier );
		if( m_TapNotesRemaining[iCurrentIndexEarlier] & player_step )	// these Pattern overlap
		{
			m_TapNotesRemaining[iCurrentIndexEarlier] &= ~player_step;	// subtract player_step
			if( m_TapNotesRemaining[iCurrentIndexEarlier] == 0 )	{		// did this complete the step?
				OnCompleteStep( fSongBeat, player_step, fMaxBeatDiff, iCurrentIndexEarlier );
				return;
			}
		}

		////////////////////////////
		// check the step to the right of iIndexStartLookingAt
		////////////////////////////
		//HELPER.Log( "Checking Pattern[%d]", iCurrentIndexLater );
		if( m_TapNotesRemaining[iCurrentIndexLater] & player_step )		// these Pattern overlap
		{
			m_TapNotesRemaining[iCurrentIndexLater] &= ~player_step;		// subtract player_step
			if( m_TapNotesRemaining[iCurrentIndexLater] == 0 ) {			// did this complete the step?
				OnCompleteStep( fSongBeat, player_step, fMaxBeatDiff, iCurrentIndexLater );
				return;
			}
		}
	}
}

void Player::OnCompleteStep( float fSongBeat, TapNote player_step, float fMaxBeatDiff, int iIndexThatWasSteppedOn )
{
	float fStepBeat = NoteIndexToBeat( (float)iIndexThatWasSteppedOn );

 
	float fBeatsUntilStep = fStepBeat - fSongBeat;
	float fPercentFromPerfect = fabsf( fBeatsUntilStep / fMaxBeatDiff );

	//HELPER.Log( "fBeatsUntilStep: %f, fPercentFromPerfect: %f", 
	//		 fBeatsUntilStep, fPercentFromPerfect );

	// compute what the score should be for the note we stepped on
	TapNoteScore &score = m_TapNoteScores[iIndexThatWasSteppedOn];

	if(		 fPercentFromPerfect < 0.25f )	score = TNS_PERFECT;
	else if( fPercentFromPerfect < 0.50f )	score = TNS_GREAT;
	else if( fPercentFromPerfect < 0.75f )	score = TNS_GOOD;
	else									score = TNS_BOO;

	// update the judgement, score, and life
	m_Judgement.SetJudgement( score );
	m_Score.AddToScore( score, m_Combo.GetCurrentCombo() );
	m_LifeMeter.ChangeLife( score );


	// remove this row from the ColorArrowField
	m_ColorArrowField.RemoveTapNoteRow( iIndexThatWasSteppedOn );

	// check to see if this completes a row of notes
	for( int c=0; c<m_Style.m_iNumColumns; c++ )	// for each column
	{
		if( m_TapNotesOriginal[iIndexThatWasSteppedOn] & m_Style.m_ColumnToTapNote[c] )	// if this colum was part of the original step
			m_GhostArrows.TapNote( c, score, m_Combo.GetCurrentCombo()>100 );	// show the ghost arrow for this column
	}

	// update the combo display
	switch( score )
	{
	case TNS_PERFECT:
	case TNS_GREAT:
		m_Combo.ContinueCombo();
		break;
	case TNS_GOOD:
	case TNS_BOO:
		m_Combo.EndCombo();
		break;
	}
}


ScoreSummary Player::GetScoreSummary()
{
	ScoreSummary scoreSummary;

	for( int i=0; i<MAX_TAP_NOTE_ELEMENTS; i++ ) 
	{
		switch( m_TapNoteScores[i] )
		{
		case TNS_PERFECT:	scoreSummary.perfect++;		break;
		case TNS_GREAT:		scoreSummary.great++;		break;
		case TNS_GOOD:		scoreSummary.good++;		break;
		case TNS_BOO:		scoreSummary.boo++;			break;
		case TNS_MISS:		scoreSummary.miss++;		break;
		case TNS_NONE:									break;
		default:		ASSERT( false );
		}
	}
	for( i=0; i<m_iNumHoldNotes; i++ ) 
	{
		switch( m_HoldNoteScores[i].m_TapNoteScore )
		{
		case TNS_PERFECT:	scoreSummary.perfect++;		break;
		case TNS_GREAT:		scoreSummary.great++;		break;
		case TNS_GOOD:		scoreSummary.good++;		break;
		case TNS_BOO:		scoreSummary.boo++;			break;
		case TNS_MISS:		scoreSummary.miss++;		break;
		case TNS_NONE:									break;
		default:		ASSERT( false );
		}
		switch( m_HoldNoteScores[i].m_Result )
		{
		case HNR_NG:		scoreSummary.ng++;		break;
		case HNR_OK:		scoreSummary.ok++;		break;
		case HNR_NONE:								break;
		default:		ASSERT( false );
		}
	}
	scoreSummary.max_combo = m_Combo.GetMaxCombo();
	scoreSummary.score = m_Score.GetScore();
	
	return scoreSummary;
}


int Player::UpdateStepsMissedOlderThan( float fMissIfOlderThanThisBeat )
{
	//HELPER.Log( "Pattern::UpdateStepsMissedOlderThan(%f)", fMissIfOlderThanThisBeat );

	int iMissIfOlderThanThisIndex = BeatToNoteIndex( fMissIfOlderThanThisBeat );

	int iNumMissesFound = 0;
	// Since this is being called frame, let's not check the whole array every time.
	// Instead, only check 10 elements back.  Even 10 is overkill.
	int iStartCheckingAt = max( 0, iMissIfOlderThanThisIndex-10 );

	//HELPER.Log( "iStartCheckingAt: %d   iMissIfOlderThanThisIndex:  %d", iStartCheckingAt, iMissIfOlderThanThisIndex );
	for( int i=iStartCheckingAt; i<iMissIfOlderThanThisIndex; i++ )
	{
		//HELPER.Log( "Checking for miss:  %d: lefttostepon == %d, score == %d", i, m_LeftToStepOn[i], m_StepScore[i] );
		if( m_TapNotesRemaining[i] != 0  &&  m_TapNoteScores[i] != TNS_MISS )
		{
			m_TapNoteScores[i] = TNS_MISS;
			iNumMissesFound++;
			m_LifeMeter.ChangeLife( TNS_MISS );
		}
	}

	return iNumMissesFound;
}







