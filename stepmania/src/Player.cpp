#include "stdafx.h"
/*
-----------------------------------------------------------------------------
 Class: Player

 Desc: See header.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "GameConstantsAndTypes.h"
#include "Math.h" // for fabs()
#include "Player.h"
#include "RageUtil.h"
#include "ThemeManager.h"
#include "GameConstantsAndTypes.h"
#include "ArrowEffects.h"
#include "GameManager.h"
#include "InputMapper.h"
#include "SongManager.h"


// these two items are in the
const float FRAME_JUDGE_AND_COMBO_Y = CENTER_Y;
const float JUDGEMENT_Y_OFFSET	= -26;
const float COMBO_Y_OFFSET		= +26;

const float ARROWS_Y			= SCREEN_TOP + ARROW_SIZE * 1.5f;
const float HOLD_JUDGEMENT_Y	= ARROWS_Y + 80;

const float HOLD_ARROW_NG_TIME	=	0.18f;


Player::Player()
{

	m_fSongBeat = 0;
	m_PlayerNumber = PLAYER_INVALID;

	// init step elements
	for( int t=0; t<MAX_NOTE_TRACKS; t++ )
	{
		for( int i=0; i<MAX_TAP_NOTE_ROWS; i++ )
		{
			m_TapNotes[t][i] = '0';
			m_TapNoteScores[t][i] = TNS_NONE;
		}
	}

	m_iNumHoldNotes = 0;
	for( int i=0; i<MAX_HOLD_NOTE_ELEMENTS; i++ )
	{
		m_HoldNoteScores[i] = HNS_NONE;
		m_fHoldNoteLife[i] = 1.0f;
	}

	m_pLifeMeter = NULL;
	m_pScore = NULL;

	this->AddActor( &m_GrayArrowRow );
	this->AddActor( &m_NoteField );
	this->AddActor( &m_GhostArrowRow );

	m_frameJudgeAndCombo.AddActor( &m_Judgement );
	m_frameJudgeAndCombo.AddActor( &m_Combo );
	this->AddActor( &m_frameJudgeAndCombo );
	
	for( int c=0; c<MAX_NOTE_TRACKS; c++ )
		this->AddActor( &m_HoldJudgement[c] );
}


void Player::Load( PlayerNumber player_no, StyleDef* pStyleDef, NoteData* pNoteData, const PlayerOptions& po, LifeMeterBar* pLM, ScoreDisplayRolling* pScore, int iOriginalNumNotes )
{
	//LOG->WriteLine( "Player::Load()", );
	this->CopyAll( pNoteData );
	NoteDataWithScoring::Init();

	m_PlayerNumber = player_no;
	m_PlayerOptions = po;

	m_pLifeMeter = pLM;
	m_pScore = pScore;
	if( m_pScore )
		m_pScore->Init( player_no, m_PlayerOptions, iOriginalNumNotes, SONGMAN->GetCurrentNotes(player_no)->m_iMeter );

	if( !po.m_bHoldNotes )
		this->RemoveHoldNotes();

	this->Turn( po.m_TurnType );

	if( po.m_bLittle )
		this->MakeLittle();

	m_NoteField.Load( (NoteData*)this, player_no, pStyleDef, po, 1.5f, 5.5f, NoteField::MODE_DANCING );
	
	m_GrayArrowRow.Load( player_no, pStyleDef, po );
	m_GhostArrowRow.Load( player_no, pStyleDef, po );

	m_frameJudgeAndCombo.SetY( FRAME_JUDGE_AND_COMBO_Y );
	m_Combo.SetY( po.m_bReverseScroll ?  -COMBO_Y_OFFSET : COMBO_Y_OFFSET );
	m_Judgement.SetY( po.m_bReverseScroll ? -JUDGEMENT_Y_OFFSET : JUDGEMENT_Y_OFFSET );

	for( int c=0; c<pStyleDef->m_iColsPerPlayer; c++ )
		m_HoldJudgement[c].SetY( po.m_bReverseScroll ? SCREEN_HEIGHT - HOLD_JUDGEMENT_Y : HOLD_JUDGEMENT_Y );
	for( c=0; c<pStyleDef->m_iColsPerPlayer; c++ )
		m_HoldJudgement[c].SetX( (float)pStyleDef->m_ColumnInfo[player_no][c].fXOffset );

	m_NoteField.SetY( po.m_bReverseScroll ? SCREEN_HEIGHT - ARROWS_Y : ARROWS_Y );
	m_GrayArrowRow.SetY( po.m_bReverseScroll ? SCREEN_HEIGHT - ARROWS_Y : ARROWS_Y );
	m_GhostArrowRow.SetY( po.m_bReverseScroll ? SCREEN_HEIGHT - ARROWS_Y : ARROWS_Y );
}

void Player::Update( float fDeltaTime, float fSongBeat, float fMaxBeatDifference )
{
	//LOG->WriteLine( "Player::Update(%f, %f, %f)", fDeltaTime, fSongBeat, fMaxBeatDifference );


	//
	// Check for TapNote misses
	//
	UpdateTapNotesMissedOlderThan( m_fSongBeat-fMaxBeatDifference );


	//
	// update HoldNotes logic
	//
	for( int i=0; i<m_iNumHoldNotes; i++ )		// for each HoldNote
	{
		HoldNote &hn = m_HoldNotes[i];
		HoldNoteScore &hns = m_HoldNoteScores[i];
		float &fLife = m_fHoldNoteLife[i];

		if( hns != HNS_NONE )	// if this HoldNote already has a result
			continue;	// we don't need to update the logic for this one

		float fStartBeat = NoteRowToBeat( (float)hn.m_iStartIndex );
		float fEndBeat = NoteRowToBeat( (float)hn.m_iEndIndex );

		const StyleInput StyleI( m_PlayerNumber, hn.m_iTrack );
		const GameInput GameI = GAMEMAN->GetCurrentStyleDef()->StyleInputToGameInput( StyleI );


		// update the life
		if( fStartBeat < m_fSongBeat && m_fSongBeat < fEndBeat )	// if the song beat is in the range of this hold
		{
			const bool bIsHoldingButton = INPUTMAPPER->IsButtonDown( GameI )  ||  PREFSMAN->m_bAutoPlay;
			// if they got a bad score or haven't stepped on the corresponding tap yet
			const bool bSteppedOnTapNote = m_TapNoteScores[hn.m_iTrack][hn.m_iStartIndex] >= TNS_GREAT;


			if( bIsHoldingButton && bSteppedOnTapNote )
			{
				// Increase life
				fLife += fDeltaTime/HOLD_ARROW_NG_TIME;
				fLife = min( fLife, 1 );	// clamp

				m_NoteField.m_HoldNotes[i].m_iStartIndex = BeatToNoteRow( m_fSongBeat );	// move the start of this Hold

				m_GhostArrowRow.HoldNote( hn.m_iTrack );		// update the "electric ghost" effect
			}
			else	// !bIsHoldingButton
			{
				if( m_fSongBeat-fStartBeat > fMaxBeatDifference )
				{
					// Decrease life
					fLife -= fDeltaTime/HOLD_ARROW_NG_TIME;
					fLife = max( fLife, 0 );	// clamp
				}
			}
			m_NoteField.SetHoldNoteLife( i, fLife );	// update the NoteField display
		}

		// check for NG
		if( fLife == 0 )	// the player has not pressed the button for a long time!
		{
			hns = HNS_NG;
			m_Combo.EndCombo();
			m_HoldJudgement[hn.m_iTrack].SetHoldJudgement( HNS_NG );
		}

		// check for OK
		if( m_fSongBeat > fEndBeat )	// if this HoldNote is in the past
		{
			// At this point fLife > 0, or else we would have marked it NG above
			fLife = 1;
			hns = HNS_OK;
			m_HoldJudgement[hn.m_iTrack].SetHoldJudgement( HNS_OK );
			m_NoteField.SetHoldNoteLife( i, fLife );	// update the NoteField display
		}
	}



	ActorFrame::Update( fDeltaTime );

	m_frameJudgeAndCombo.Update( fDeltaTime );

	if( m_pLifeMeter )
		m_pLifeMeter->SetBeat( fSongBeat );

	m_GrayArrowRow.Update( fDeltaTime, fSongBeat );
	m_NoteField.Update( fDeltaTime, fSongBeat );
	m_GhostArrowRow.Update( fDeltaTime, fSongBeat );

	m_fSongBeat = fSongBeat;	// save song beat

}

void Player::DrawPrimitives()
{
	D3DXMATRIX matOldView, matOldProj;

	if( m_PlayerOptions.m_EffectType == PlayerOptions::EFFECT_SPACE )
	{
		// turn off Z Buffering
		DISPLAY->GetDevice()->SetRenderState( D3DRS_ZENABLE,      FALSE );
		DISPLAY->GetDevice()->SetRenderState( D3DRS_ZWRITEENABLE, FALSE );

		// save old view and projection
		DISPLAY->GetDevice()->GetTransform( D3DTS_VIEW, &matOldView );
		DISPLAY->GetDevice()->GetTransform( D3DTS_PROJECTION, &matOldProj );


		// construct view and project matrix
		D3DXMATRIX matNewView;
		D3DXMatrixLookAtLH( &matNewView, &D3DXVECTOR3( CENTER_X, GetY()+800.0f, 300.0f ), 
										 &D3DXVECTOR3( CENTER_X
										 , GetY()+400.0f,   0.0f ), 
										 &D3DXVECTOR3(          0.0f,         -1.0f,   0.0f ) );
		DISPLAY->GetDevice()->SetTransform( D3DTS_VIEW, &matNewView );

		D3DXMATRIX matNewProj;
		D3DXMatrixPerspectiveFovLH( &matNewProj, D3DX_PI/4.0f, SCREEN_WIDTH/(float)SCREEN_HEIGHT, 0.0f, 1000.0f );
		DISPLAY->GetDevice()->SetTransform( D3DTS_PROJECTION, &matNewProj );
	}

	m_GrayArrowRow.Draw();
	m_NoteField.Draw();
	m_GhostArrowRow.Draw();

	if( m_PlayerOptions.m_EffectType == PlayerOptions::EFFECT_SPACE )
	{
		// restire old view and projection
		DISPLAY->GetDevice()->SetTransform( D3DTS_VIEW, &matOldView );
		DISPLAY->GetDevice()->SetTransform( D3DTS_PROJECTION, &matOldProj );

		// turn Z Buffering back on
		DISPLAY->GetDevice()->SetRenderState( D3DRS_ZENABLE,      TRUE );
		DISPLAY->GetDevice()->SetRenderState( D3DRS_ZWRITEENABLE, TRUE );
	}

	m_frameJudgeAndCombo.Draw();

	for( int c=0; c<m_iNumTracks; c++ )
		m_HoldJudgement[c].Draw();
}

void Player::HandlePlayerStep( float fSongBeat, int col, float fMaxBeatDiff )
{
	//LOG->WriteLine( "Player::HandlePlayerStep()" );

	ASSERT( col >= 0  &&  col <= m_iNumTracks );

	m_GrayArrowRow.Step( col );


	// look for the closest matching step
	int iIndexStartLookingAt = BeatToNoteRow( fSongBeat );
	int iNumElementsToExamine = BeatToNoteRow( fMaxBeatDiff );	// number of elements to examine on either end of iIndexStartLookingAt
	
	//LOG->WriteLine( "iIndexStartLookingAt = %d, iNumElementsToExamine = %d", iIndexStartLookingAt, iNumElementsToExamine );

	int iIndexOverlappingNote = -1;		// leave as -1 if we don't find any

	// Start at iIndexStartLookingAt and search outward.  The first one note overlaps the player's hit (this is the closest match).
	for( int delta=0; delta <= iNumElementsToExamine; delta++ )
	{
		int iCurrentIndexEarlier = iIndexStartLookingAt - delta;
		int iCurrentIndexLater   = iIndexStartLookingAt + delta;

		// silly check to make sure we don't go out of bounds
		iCurrentIndexEarlier	= clamp( iCurrentIndexEarlier, 0, MAX_TAP_NOTE_ROWS-1 );
		iCurrentIndexLater		= clamp( iCurrentIndexLater,   0, MAX_TAP_NOTE_ROWS-1 );

		////////////////////////////
		// check the step to the left of iIndexStartLookingAt
		////////////////////////////
		//LOG->WriteLine( "Checking Notes[%d]", iCurrentIndexEarlier );
		if( m_TapNotes[col][iCurrentIndexEarlier] != '0'  &&	// there is a note here
			m_TapNoteScores[col][iCurrentIndexEarlier] == TNS_NONE )	// this note doesn't have a score
		{
			iIndexOverlappingNote = iCurrentIndexEarlier;
			break;
		}


		////////////////////////////
		// check the step to the right of iIndexStartLookingAt
		////////////////////////////
		//LOG->WriteLine( "Checking Notes[%d]", iCurrentIndexLater );
		if( m_TapNotes[col][iCurrentIndexLater] != '0'  &&	// there is a note here
			m_TapNoteScores[col][iCurrentIndexLater] == TNS_NONE )	// this note doesn't have a score
		{
			iIndexOverlappingNote = iCurrentIndexLater;
			break;
		}
	}

	if( iIndexOverlappingNote != -1 )
	{
		// compute the score for this hit
		const float fStepBeat = NoteRowToBeat( (float)iIndexOverlappingNote );
		const float fBeatsUntilStep = fStepBeat - fSongBeat;
		const float fPercentFromPerfect = fabsf( fBeatsUntilStep / fMaxBeatDiff );

		TapNoteScore &score = m_TapNoteScores[col][iIndexOverlappingNote];

		if(		 fPercentFromPerfect < 0.30f )	score = TNS_PERFECT;
		else if( fPercentFromPerfect < 0.55f )	score = TNS_GREAT;
		else if( fPercentFromPerfect < 0.78f )	score = TNS_GOOD;
		else									score = TNS_BOO;


		bool bRowDestroyed = true;
		for( int t=0; t<m_iNumTracks; t++ )			// did this complete the elminiation of the row?
		{
			if( m_TapNotes[t][iIndexOverlappingNote] != '0'  &&			// there is a note here
				m_TapNoteScores[t][iIndexOverlappingNote] == TNS_NONE )	// and it doesn't have a score
			{
				bRowDestroyed = false;
				break;	// stop searching
			}
		}
		if( bRowDestroyed )
			OnRowDestroyed( fSongBeat, col, fMaxBeatDiff, iIndexOverlappingNote );
	}
}

void Player::OnRowDestroyed( float fSongBeat, int col, float fMaxBeatDiff, int iIndexThatWasSteppedOn )
{
	//LOG->WriteLine( "fBeatsUntilStep: %f, fPercentFromPerfect: %f", 
	//		 fBeatsUntilStep, fPercentFromPerfect );
	
	// find the minimum score of the row
	TapNoteScore score = TNS_PERFECT;
	for( int t=0; t<m_iNumTracks; t++ )
		if( m_TapNoteScores[t][iIndexThatWasSteppedOn] >= TNS_BOO )
			score = min( score, m_TapNoteScores[t][iIndexThatWasSteppedOn] );

	// update the judgement, score, and life
	m_Judgement.SetJudgement( score );
	if( m_pLifeMeter )
		m_pLifeMeter->ChangeLife( score );


	// remove this row from the NoteField
	if ( ( score == TNS_PERFECT ) || ( score == TNS_GREAT ) )
		m_NoteField.RemoveTapNoteRow( iIndexThatWasSteppedOn );

	for( int c=0; c<m_iNumTracks; c++ )	// for each column
	{
		if( m_TapNotes[c][iIndexThatWasSteppedOn] != '0' )	// if there is a note in this col
		{
			m_GhostArrowRow.TapNote( c, score, m_Combo.GetCurrentCombo()>100 );	// show the ghost arrow for this column
			
			if( m_pScore )
				m_pScore->AddToScore( score, m_Combo.GetCurrentCombo() );	// update score - called once per note in this row

			// update combo - called once per note in this row
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

	// zoom the judgement and combo like a heart beat
	float fStartZoom;
	switch( score )
	{
	case TNS_PERFECT:	fStartZoom = 1.5f;	break;
	case TNS_GREAT:		fStartZoom = 1.3f;	break;
	case TNS_GOOD:		fStartZoom = 1.2f;	break;
	case TNS_BOO:		fStartZoom = 1.0f;	break;
	}
	m_frameJudgeAndCombo.SetZoom( fStartZoom );
	m_frameJudgeAndCombo.BeginTweening( 0.2f );
	m_frameJudgeAndCombo.SetTweenZoom( 1 );
}


int Player::UpdateTapNotesMissedOlderThan( float fMissIfOlderThanThisBeat )
{
	//LOG->WriteLine( "Notes::UpdateTapNotesMissedOlderThan(%f)", fMissIfOlderThanThisBeat );

	int iMissIfOlderThanThisIndex = BeatToNoteRow( fMissIfOlderThanThisBeat );

	int iNumMissesFound = 0;
	// Since this is being called every frame, let's not check the whole array every time.
	// Instead, only check 10 elements back.  Even 10 is overkill.
	int iStartCheckingAt = max( 0, iMissIfOlderThanThisIndex-10 );

	//LOG->WriteLine( "iStartCheckingAt: %d   iMissIfOlderThanThisIndex:  %d", iStartCheckingAt, iMissIfOlderThanThisIndex );
	for( int t=0; t<m_iNumTracks; t++ )
	{
		for( int r=iStartCheckingAt; r<iMissIfOlderThanThisIndex; r++ )
		{
			bool bFoundAMissInThisRow = false;
			if( m_TapNotes[t][r] != '0'  &&  m_TapNoteScores[t][r] == TNS_NONE )
			{
				m_TapNoteScores[t][r] = TNS_MISS;
				iNumMissesFound++;
				bFoundAMissInThisRow = true;
			}
			if( bFoundAMissInThisRow )
				if( m_pLifeMeter )
					m_pLifeMeter->ChangeLife( TNS_MISS );
		}
	}

	if( iNumMissesFound > 0 )
	{
		m_Judgement.SetJudgement( TNS_MISS );
		m_Combo.EndCombo();
	}

	return iNumMissesFound;
}


void Player::CrossedRow( int iNoteRow, float fSongBeat, float fMaxBeatDiff )
{
	if( PREFSMAN->m_bAutoPlay )
	{
		// check to see if there's at the crossed row
		for( int t=0; t<m_iNumTracks; t++ )
		{
			if( m_TapNotes[t][iNoteRow] != '0' )
				this->HandlePlayerStep( fSongBeat, t, fMaxBeatDiff );
		}
	}
}



GameplayStatistics Player::GetGameplayStatistics()
{
	GameplayStatistics GSreturn;

	GSreturn.pSong = SONGMAN->GetCurrentSong();
	Notes* pNotes = SONGMAN->GetCurrentNotes(m_PlayerNumber);
	GSreturn.dc = pNotes->m_DifficultyClass;
	GSreturn.meter = pNotes->m_iMeter;
	GSreturn.iPossibleDancePoints = ((NoteData*)this)->GetPossibleDancePoints();
	GSreturn.iActualDancePoints = 0;

	for( int t=0; t<MAX_NOTE_TRACKS; t++ )
	{
		for( int r=0; r<MAX_TAP_NOTE_ROWS; r++ ) 
		{
			if( m_TapNotes[t][r] == '0' )
				continue;	// no note here

			switch( m_TapNoteScores[t][r] )
			{
			case TNS_PERFECT:	GSreturn.perfect++;	break;
			case TNS_GREAT:		GSreturn.great++;	break;
			case TNS_GOOD:		GSreturn.good++;	break;
			case TNS_BOO:		GSreturn.boo++;		break;
			case TNS_MISS:		GSreturn.miss++;	break;
			case TNS_NONE:							break;
			default:		ASSERT( false );
			}
			GSreturn.iActualDancePoints += TapNoteScoreToDancePoints( m_TapNoteScores[t][r] );
		}
	}
	for( int i=0; i<m_iNumHoldNotes; i++ ) 
	{
		switch( m_HoldNoteScores[i] )
		{
		case HNS_NG:	GSreturn.ng++;	break;
		case HNS_OK:	GSreturn.ok++;	break;
		case HNS_NONE:					break;
		default:		ASSERT( false );
		}
		GSreturn.iActualDancePoints += HoldNoteScoreToDancePoints( m_HoldNoteScores[i] );
	}
	GSreturn.max_combo = m_Combo.GetMaxCombo();
	GSreturn.score = m_pScore ? m_pScore->GetScore() : 0;

	GSreturn.failed = m_pLifeMeter ? m_pLifeMeter->HasFailed() : false;


	for( int r=0; r<NUM_RADAR_VALUES; r++ )
	{
		GSreturn.fRadarPossible[r] = this->GetRadarValue( (RadarCategory)r, SONGMAN->GetCurrentSong()->m_fMusicLengthSeconds );
		GSreturn.fRadarActual[r] = this->GetActualRadarValue( (RadarCategory)r, SONGMAN->GetCurrentSong()->m_fMusicLengthSeconds );

		GSreturn.fRadarPossible[r] = clamp( GSreturn.fRadarPossible[r], 0, 1 );
		GSreturn.fRadarActual[r] = clamp( GSreturn.fRadarActual[r], 0, 1 );
	}

	return GSreturn;
}
