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
#include "PrefsManager.h"
#include "GameConstantsAndTypes.h"
#include "ArrowEffects.h"
#include "GameManager.h"
#include "InputMapper.h"
#include "SongManager.h"
#include "GameState.h"
#include "ScoreKeeperMAX2.h"
#include "RageLog.h"
#include "RageMath.h"


#define JUDGE_PERFECT_ZOOM_X		THEME->GetMetricF("Player","JudgePerfectZoomX")
#define JUDGE_PERFECT_ZOOM_Y		THEME->GetMetricF("Player","JudgePerfectZoomY")
#define JUDGE_GREAT_ZOOM_X			THEME->GetMetricF("Player","JudgeGreatZoomX")
#define JUDGE_GREAT_ZOOM_Y			THEME->GetMetricF("Player","JudgeGreatZoomY")
#define JUDGE_GOOD_ZOOM_X			THEME->GetMetricF("Player","JudgeGoodZoomX")
#define JUDGE_GOOD_ZOOM_Y			THEME->GetMetricF("Player","JudgeGoodZoomY")
#define JUDGE_BOO_ZOOM_X			THEME->GetMetricF("Player","JudgeBooZoomX")
#define JUDGE_BOO_ZOOM_Y			THEME->GetMetricF("Player","JudgeBooZoomY")
#define COMBO_JUDGE_TWEEN_SECONDS	THEME->GetMetricF("Player","ComboJudgeTweenSeconds")
#define BRIGHT_GHOST_THRESHOLD		THEME->GetMetricI("Player","BrightGhostThreshold")


// cache because reading from theme metrics is slow
float g_fJudgePerfectZoomX,
	  g_fJudgePerfectZoomY,
	  g_fJudgeGreatZoomX,
	  g_fJudgeGreatZoomY,
	  g_fJudgeGoodZoomX,
	  g_fJudgeGoodZoomY,
	  g_fJudgeBooZoomX,
      g_fJudgeBooZoomY,
      g_fComboJudgeTweenSeconds;
int	  g_iBrightGhostThreshold;

// these two items are in the
const float FRAME_JUDGE_AND_COMBO_Y = CENTER_Y;
const float JUDGEMENT_Y_OFFSET	= -26;
const float COMBO_Y_OFFSET		= +26;

const float FRAME_JUDGE_AND_COMBO_BEAT_TIME = 0.2f;

const float ARROWS_Y			= SCREEN_TOP + ARROW_SIZE * 1.5f;
const float HOLD_JUDGEMENT_Y	= ARROWS_Y + 80;

const float HOLD_ARROW_NG_TIME	=	0.18f;


Player::Player()
{
	// Update theme metrics cache
	g_fJudgePerfectZoomX = JUDGE_PERFECT_ZOOM_X;
	g_fJudgePerfectZoomY = JUDGE_PERFECT_ZOOM_Y;
	g_fJudgeGreatZoomX = JUDGE_GREAT_ZOOM_X;
	g_fJudgeGreatZoomY = JUDGE_GREAT_ZOOM_Y;
	g_fJudgeGoodZoomX = JUDGE_GOOD_ZOOM_X;
	g_fJudgeGoodZoomY = JUDGE_GOOD_ZOOM_Y;
	g_fJudgeBooZoomX = JUDGE_BOO_ZOOM_X;
    g_fJudgeBooZoomY = JUDGE_BOO_ZOOM_Y;
	g_fComboJudgeTweenSeconds = COMBO_JUDGE_TWEEN_SECONDS;
	g_iBrightGhostThreshold = BRIGHT_GHOST_THRESHOLD;

	m_PlayerNumber = PLAYER_INVALID;

	m_pLifeMeter = NULL;
	m_pScore = NULL;
	m_ScoreKeeper = NULL;
	
	m_iOffsetSample = 0;

	this->AddChild( &m_GrayArrowRow );
	this->AddChild( &m_NoteField );
	this->AddChild( &m_GhostArrowRow );

	m_frameJudgement.AddChild( &m_Judgement );
	this->AddChild( &m_frameJudgement );

	m_frameCombo.AddChild( &m_Combo );
	this->AddChild( &m_frameCombo );
	
	for( int c=0; c<MAX_NOTE_TRACKS; c++ )
		this->AddChild( &m_HoldJudgement[c] );
}


Player::~Player()
{
	delete m_ScoreKeeper;
}


void Player::Load( PlayerNumber pn, NoteData* pNoteData, LifeMeter* pLM, ScoreDisplay* pScore )
{
	//LOG->Trace( "Player::Load()", );

	m_PlayerNumber = pn;
	m_pLifeMeter = pLM;
	m_pScore = pScore;

	const StyleDef* pStyleDef = GAMESTATE->GetCurrentStyleDef();

	// init scoring
	NoteDataWithScoring::Init();

	// copy note data
	this->CopyAll( pNoteData );
	if( GAMESTATE->m_SongOptions.m_LifeType == SongOptions::LIFE_BATTERY  &&  GAMESTATE->m_fSecondsBeforeFail[m_PlayerNumber] != -1 )	// Oni dead
		this->ClearAll();

	/* The editor reuses Players ... so we really need to make sure everything
	 * is reset and not tweening.  Perhaps ActorFrame should recurse to subactors;
	 * then we could just this->StopTweening()? -glenn */
	m_frameJudgement.StopTweening();
//	m_Combo.Reset();		// don't reset combos between songs in a course!
	m_Judgement.Reset();

	if(m_ScoreKeeper) delete m_ScoreKeeper;
	m_ScoreKeeper = new ScoreKeeperMAX2(GAMESTATE->m_pCurNotes[m_PlayerNumber], *this, pn);

	if( m_pScore )
		m_pScore->Init( pn );

	if( !GAMESTATE->m_PlayerOptions[pn].m_bHoldNotes )
		this->RemoveHoldNotes();

	this->Turn( GAMESTATE->m_PlayerOptions[pn].m_TurnType );

	if( GAMESTATE->m_PlayerOptions[pn].m_bLittle )
		this->MakeLittle();

	int iPixelsToDrawBefore = 96;
	int iPixelsToDrawAfter = 384;

	// If both options are on, we *do* need to multiply it twice.
	if( GAMESTATE->m_PlayerOptions[pn].m_bEffects[PlayerOptions::EFFECT_MINI] )
	{
		iPixelsToDrawBefore *= 2;
		iPixelsToDrawAfter *= 2;
	}
	if( GAMESTATE->m_PlayerOptions[pn].m_bEffects[PlayerOptions::EFFECT_SPACE] )
	{
		iPixelsToDrawBefore *= 2;
		iPixelsToDrawAfter *= 2;
	}

	m_NoteField.Load( (NoteData*)this, pn, iPixelsToDrawBefore, iPixelsToDrawAfter );
	
	m_GrayArrowRow.Load( pn );
	m_GhostArrowRow.Load( pn );

	m_frameJudgement.SetY( FRAME_JUDGE_AND_COMBO_Y );
	m_frameCombo.SetY( FRAME_JUDGE_AND_COMBO_Y );
	m_Combo.SetY( GAMESTATE->m_PlayerOptions[pn].m_bReverseScroll ?  -COMBO_Y_OFFSET : COMBO_Y_OFFSET );
	m_Judgement.SetY( GAMESTATE->m_PlayerOptions[pn].m_bReverseScroll ? -JUDGEMENT_Y_OFFSET : JUDGEMENT_Y_OFFSET );

	for( int c=0; c<pStyleDef->m_iColsPerPlayer; c++ )
		m_HoldJudgement[c].SetY( GAMESTATE->m_PlayerOptions[pn].m_bReverseScroll ? SCREEN_HEIGHT - HOLD_JUDGEMENT_Y : HOLD_JUDGEMENT_Y );
	for( c=0; c<pStyleDef->m_iColsPerPlayer; c++ )
		m_HoldJudgement[c].SetX( (float)pStyleDef->m_ColumnInfo[pn][c].fXOffset );

	m_NoteField.SetY( GAMESTATE->m_PlayerOptions[pn].m_bReverseScroll ? SCREEN_HEIGHT - ARROWS_Y : ARROWS_Y );
	m_GrayArrowRow.SetY( GAMESTATE->m_PlayerOptions[pn].m_bReverseScroll ? SCREEN_HEIGHT - ARROWS_Y : ARROWS_Y );
	m_GhostArrowRow.SetY( GAMESTATE->m_PlayerOptions[pn].m_bReverseScroll ? SCREEN_HEIGHT - ARROWS_Y : ARROWS_Y );

	if( GAMESTATE->m_PlayerOptions[pn].m_bEffects[PlayerOptions::EFFECT_MINI] )
	{
		m_NoteField.SetZoom( 0.5f );
		m_GrayArrowRow.SetZoom( 0.5f );
		m_GhostArrowRow.SetZoom( 0.5f );
	}
}

void Player::Update( float fDeltaTime )
{
	//LOG->Trace( "Player::Update(%f)", fDeltaTime );


	const float fSongBeat = GAMESTATE->m_fSongBeat;

	//
	// Check for TapNote misses
	//
	UpdateTapNotesMissedOlderThan( GAMESTATE->m_fSongBeat - GetMaxBeatDifference() );

	//
	// update HoldNotes logic
	//
	for( int i=0; i < GetNumHoldNotes(); i++ )		// for each HoldNote
	{
		const HoldNote &hn = GetHoldNote(i);
		HoldNoteScore &hns = m_HoldNoteScores[i];
		float &fLife = m_fHoldNoteLife[i];
		int iHoldStartIndex = BeatToNoteRow(hn.m_fStartBeat);

		m_NoteField.m_bIsHoldingHoldNote[i] = false;	// set host flag so NoteField can do intelligent drawing


		if( hns != HNS_NONE )	// if this HoldNote already has a result
			continue;	// we don't need to update the logic for this one

		const StyleInput StyleI( m_PlayerNumber, hn.m_iTrack );
		const GameInput GameI = GAMESTATE->GetCurrentStyleDef()->StyleInputToGameInput( StyleI );

		// if they got a bad score or haven't stepped on the corresponding tap yet
		const TapNoteScore tns = m_TapNoteScores[hn.m_iTrack][iHoldStartIndex];
		const bool bSteppedOnTapNote = tns != TNS_NONE  &&  tns != TNS_MISS;	// did they step on the start of this hold?

		if( hn.m_fStartBeat < fSongBeat && fSongBeat < hn.m_fEndBeat )	// if the song beat is in the range of this hold
		{
			bool bIsHoldingButton = INPUTMAPPER->IsButtonDown( GameI );
			if( !GAMESTATE->m_bEditing  &&  (PREFSMAN->m_bAutoPlay  ||  GAMESTATE->m_bDemonstration) )
				bIsHoldingButton = true;

			m_NoteField.m_bIsHoldingHoldNote[i] = bIsHoldingButton && bSteppedOnTapNote;	// set host flag so NoteField can do intelligent drawing

			if( bSteppedOnTapNote )		// this note is not judged and we stepped on its head
			{
				m_NoteField.GetHoldNote(i).m_fStartBeat = fSongBeat;	// move the start of this Hold
			}

			if( bSteppedOnTapNote && bIsHoldingButton )
			{
				// Increase life
				fLife += fDeltaTime/HOLD_ARROW_NG_TIME;
				fLife = min( fLife, 1 );	// clamp

				m_GhostArrowRow.HoldNote( hn.m_iTrack );		// update the "electric ghost" effect
			}
			else
			{
				if( fSongBeat-hn.m_fStartBeat > GetMaxBeatDifference() )
				{
					// Decrease life
					fLife -= fDeltaTime/HOLD_ARROW_NG_TIME;
					fLife = max( fLife, 0 );	// clamp
				}
			}
			m_NoteField.m_fHoldNoteLife[i] = fLife;	// update the NoteField display
		}

		/* check for NG.  If the head was missed completely, don't count
		 * an NG. */
		if( bSteppedOnTapNote && fLife == 0 )	// the player has not pressed the button for a long time!
		{
			hns = HNS_NG;
			HandleHoldNoteScore( hns, tns );
			m_HoldJudgement[hn.m_iTrack].SetHoldJudgement( HNS_NG );
			m_NoteField.m_HoldNoteScores[i] = HNS_NG;	// update the NoteField display
		}

		// check for OK
		if( fSongBeat >= hn.m_fEndBeat && fLife > 0 )	// if this HoldNote is in the past
		{
			fLife = 1;
			hns = HNS_OK;
			HandleHoldNoteScore( hns, tns );
			m_GhostArrowRow.TapNote( StyleI.col, TNS_PERFECT, true );	// bright ghost flash
			m_HoldJudgement[hn.m_iTrack].SetHoldJudgement( HNS_OK );
			m_NoteField.m_fHoldNoteLife[i] = fLife;		// update the NoteField display
			m_NoteField.m_HoldNoteScores[i] = HNS_OK;	// update the NoteField display
		}
	}



	ActorFrame::Update( fDeltaTime );
}

void Player::DrawPrimitives()
{
	m_frameCombo.Draw();	// draw this below everything else

	RageMatrix matOldView, matOldProj;

	if( GAMESTATE->m_PlayerOptions[m_PlayerNumber].m_bEffects[PlayerOptions::EFFECT_SPACE] )
	{
		// save old view and projection
		DISPLAY->GetViewTransform( &matOldView );
		DISPLAY->GetProjectionTransform( &matOldProj );

		// construct view and project matrix
		RageMatrix matNewView;
		RageVector3 Eye, At, Up( 0.0f, -1.0f, 0.0f );
		if( GAMESTATE->m_PlayerOptions[m_PlayerNumber].m_bReverseScroll ) {
			Eye = RageVector3( CENTER_X, GetY()-300.0f, 400.0f );
			At = RageVector3( CENTER_X, GetY()+100.0f, 0.0f );
		} else {
			Eye = RageVector3( CENTER_X, GetY()+800.0f, 400.0f );
			At = RageVector3( CENTER_X, GetY()+400.0f, 0.0f );
		}
		RageMatrixLookAtLH( &matNewView, &Eye, &At, &Up );

		DISPLAY->SetViewTransform( &matNewView );

		RageMatrix matNewProj;
		RageMatrixPerspectiveFovLH( &matNewProj, D3DX_PI/4.0f, SCREEN_WIDTH/(float)SCREEN_HEIGHT, 0.0f, 1000.0f );
		DISPLAY->SetProjectionTransform( &matNewProj );
	}

	m_GrayArrowRow.Draw();
	m_NoteField.Draw();
	m_GhostArrowRow.Draw();

	if( GAMESTATE->m_PlayerOptions[m_PlayerNumber].m_bEffects[PlayerOptions::EFFECT_SPACE] )
	{
		// restire old view and projection
		DISPLAY->SetViewTransform( &matOldView );
		DISPLAY->SetProjectionTransform( &matOldProj );
	}

	m_frameJudgement.Draw();

	for( int c=0; c<m_iNumTracks; c++ )
		m_HoldJudgement[c].Draw();
}

void Player::Step( int col )
{
	if( GAMESTATE->m_SongOptions.m_LifeType == SongOptions::LIFE_BATTERY  &&  GAMESTATE->m_fSecondsBeforeFail[m_PlayerNumber] != -1 )	// Oni dead
		return;	// do nothing

	//LOG->Trace( "Player::HandlePlayerStep()" );

	ASSERT( col >= 0  &&  col <= m_iNumTracks );

	const float fSongBeat = GAMESTATE->m_fSongBeat;

	// look for the closest matching step
	int iIndexStartLookingAt = BeatToNoteRow( GAMESTATE->m_fSongBeat );
	// number of elements to examine on either end of iIndexStartLookingAt
	// 3dfsux: I expanded this window so that beat correction will look
	int iNumElementsToExamine = BeatToNoteRow( GAMESTATE->m_fCurBPS * GAMESTATE->m_SongOptions.m_fMusicRate );
	
	//LOG->Trace( "iIndexStartLookingAt = %d, iNumElementsToExamine = %d", iIndexStartLookingAt, iNumElementsToExamine );

	int iIndexOverlappingNote = -1;		// leave as -1 if we don't find any

	// Start at iIndexStartLookingAt and search outward.  The first one note 
	// overlaps the player's hit (this is the closest match).
	for( int delta=0; delta <= iNumElementsToExamine; delta++ )
	{
		int iCurrentIndexEarlier = iIndexStartLookingAt - delta;
		int iCurrentIndexLater   = iIndexStartLookingAt + delta;

		////////////////////////////
		// check the step to the left of iIndexStartLookingAt
		////////////////////////////
		//LOG->Trace( "Checking Notes[%d]", iCurrentIndexEarlier );
		if( iCurrentIndexEarlier >= 0  &&
			GetTapNote(col, iCurrentIndexEarlier) != TAP_EMPTY  &&	// there is a note here
			m_TapNoteScores[col][iCurrentIndexEarlier] == TNS_NONE )	// this note doesn't have a score
		{
			iIndexOverlappingNote = iCurrentIndexEarlier;
			break;
		}


		////////////////////////////
		// check the step to the right of iIndexStartLookingAt
		////////////////////////////
		//LOG->Trace( "Checking Notes[%d]", iCurrentIndexLater );
		if( iCurrentIndexLater >= 0  &&
			GetTapNote(col, iCurrentIndexLater) != TAP_EMPTY  &&	// there is a note here
			m_TapNoteScores[col][iCurrentIndexLater] == TNS_NONE )	// this note doesn't have a score
		{
			iIndexOverlappingNote = iCurrentIndexLater;
			break;
		}
	}

	bool bDestroyedNote = false;

	if( iIndexOverlappingNote != -1 )
	{
		// compute the score for this hit
		const float fStepBeat = NoteRowToBeat( (float)iIndexOverlappingNote );
		const float fBeatsUntilStep = fStepBeat - fSongBeat;
		const float fPercentFromPerfect = fabsf( fBeatsUntilStep / GetMaxBeatDifference() );
		const float fNoteOffset = fBeatsUntilStep / GAMESTATE->m_fCurBPS; //the offset from the actual step in seconds


		TapNoteScore &score = m_TapNoteScores[col][iIndexOverlappingNote];

		if(		 fPercentFromPerfect < PREFSMAN->m_fJudgeWindowPerfectPercent )	score = TNS_PERFECT;
		else if( fPercentFromPerfect < PREFSMAN->m_fJudgeWindowGreatPercent )	score = TNS_GREAT;
		else if( fPercentFromPerfect < PREFSMAN->m_fJudgeWindowGoodPercent )	score = TNS_GOOD;
		//we have to mark boo's as better than MISS's now that the window is expanded
		else if( fPercentFromPerfect < 1.0f )									score = TNS_BOO;
		else																	score = TNS_NONE;

		if( GAMESTATE->m_bDemonstration  ||  PREFSMAN->m_bAutoPlay )
			score = TNS_PERFECT;

		bDestroyedNote = (score >= TNS_GOOD);

		LOG->Trace("(%2d/%2d)Note offset: %f, Score: %i", m_iOffsetSample, SAMPLE_COUNT, fNoteOffset, score);
		if (GAMESTATE->m_SongOptions.m_AutoAdjust == SongOptions::ADJUST_ON) 
		{
			m_fOffset[m_iOffsetSample++] = fNoteOffset;
			if (m_iOffsetSample >= SAMPLE_COUNT) 
			{
				float mean = calc_mean(m_fOffset, m_fOffset+SAMPLE_COUNT);
				float stddev = calc_stddev(m_fOffset, m_fOffset+SAMPLE_COUNT);

				if (stddev < .03 && stddev < fabsf(mean)) { //If they stepped with less than .03 error
					GAMESTATE->m_pCurSong->m_fBeat0OffsetInSeconds += mean;
					LOG->Trace("Offset corrected by %f. Error in steps: %f seconds.", mean, stddev);
				} else
					LOG->Trace("Offset NOT corrected. Average offset: %f seconds. Error: %f seconds.", mean, stddev);
				m_iOffsetSample = 0;
			}
		}




		if (score > TNS_NONE) {
			bool bRowDestroyed = true;
			for( int t=0; t<m_iNumTracks; t++ )			// did this complete the elminiation of the row?
			{
				if( GetTapNote(t, iIndexOverlappingNote) != TAP_EMPTY  &&			// there is a note here
					m_TapNoteScores[t][iIndexOverlappingNote] == TNS_NONE )	// and it doesn't have a score
				{
					bRowDestroyed = false;
					break;	// stop searching
				}
			}
			if( bRowDestroyed )
				OnRowDestroyed( iIndexOverlappingNote );
		}
	}

	if( !bDestroyedNote )
		m_GrayArrowRow.Step( col );
}

void Player::OnRowDestroyed( int iIndexThatWasSteppedOn )
{
	LOG->Trace( "Player::OnRowDestroyed" );
	
	// find the minimum score of the row
	TapNoteScore score = TNS_PERFECT;
	for( int t=0; t<m_iNumTracks; t++ )
		if( m_TapNoteScores[t][iIndexThatWasSteppedOn] >= TNS_BOO )
			score = min( score, m_TapNoteScores[t][iIndexThatWasSteppedOn] );

	// remove this row from the NoteField
//	bool bHoldNoteOnThisBeat = false;
//	for( int j=0; j<GetNumHoldNotes(); j++ )
//	{
//		if( GetHoldNote(j).m_iStartIndex == iIndexThatWasSteppedOn )
//		{
//			bHoldNoteOnThisBeat = true;
//			break;
//		}
//	}


	/* If the whole row was hit with perfects or greats, remove the row
	 * from the NoteField, so it disappears. */
	if ( score==TNS_PERFECT  ||  score == TNS_GREAT )
		m_NoteField.RemoveTapNoteRow( iIndexThatWasSteppedOn );

	int iNumNotesInThisRow = 0;
	for( int c=0; c<m_iNumTracks; c++ )	// for each column
	{
		if( GetTapNote(c, iIndexThatWasSteppedOn) != TAP_EMPTY )	// if there is a note in this col
		{
			iNumNotesInThisRow++;

			// show the ghost arrow for this column
			m_GhostArrowRow.TapNote( c, score, m_Combo.GetCurrentCombo()>g_iBrightGhostThreshold );
		}
	}
		
	if( iNumNotesInThisRow > 0 )
	{
		HandleNoteScore( score, iNumNotesInThisRow );	// update score
		m_Combo.UpdateScore( score, iNumNotesInThisRow );
		GAMESTATE->m_iMaxCombo[m_PlayerNumber] = max( GAMESTATE->m_iMaxCombo[m_PlayerNumber], m_Combo.GetCurrentCombo() );
	}

	// update the judgement, score, and life
	m_Judgement.SetJudgement( score );

	// zoom the judgement and combo like a heart beat
	float fStartZoomX=0.f, fStartZoomY=0.f;
	switch( score )
	{
	case TNS_PERFECT:	fStartZoomX = g_fJudgePerfectZoomX;	fStartZoomY = g_fJudgePerfectZoomY;	break;
	case TNS_GREAT:		fStartZoomX = g_fJudgeGreatZoomX;	fStartZoomY = g_fJudgeGreatZoomY;	break;
	case TNS_GOOD:		fStartZoomX = g_fJudgeGoodZoomX;	fStartZoomY = g_fJudgeGoodZoomY;	break;
	case TNS_BOO:		fStartZoomX = g_fJudgeBooZoomX;		fStartZoomY = g_fJudgeBooZoomY;		break;
	}
	m_frameJudgement.StopTweening();
	m_frameJudgement.SetZoomX( fStartZoomX );
	m_frameJudgement.SetZoomY( fStartZoomY );
	m_frameJudgement.BeginTweening( g_fComboJudgeTweenSeconds );
	m_frameJudgement.SetTweenZoom( 1 );

	m_frameCombo.StopTweening();
	m_frameCombo.SetZoomX( fStartZoomX );
	m_frameCombo.SetZoomY( fStartZoomY );
	m_frameCombo.BeginTweening( g_fComboJudgeTweenSeconds );
	m_frameCombo.SetTweenZoom( 1 );
}


int Player::UpdateTapNotesMissedOlderThan( float fMissIfOlderThanThisBeat )
{
	//LOG->Trace( "Notes::UpdateTapNotesMissedOlderThan(%f)", fMissIfOlderThanThisBeat );

	int iMissIfOlderThanThisIndex = BeatToNoteRow( fMissIfOlderThanThisBeat );

	int iNumMissesFound = 0;
	// Since this is being called every frame, let's not check the whole array every time.
	// Instead, only check 10 elements back.  Even 10 is overkill.
	int iStartCheckingAt = max( 0, iMissIfOlderThanThisIndex-10 );

	//LOG->Trace( "iStartCheckingAt: %d   iMissIfOlderThanThisIndex:  %d", iStartCheckingAt, iMissIfOlderThanThisIndex );
	for( int r=iStartCheckingAt; r<iMissIfOlderThanThisIndex; r++ )
	{
		int iNumMissesThisRow = 0;
		for( int t=0; t<m_iNumTracks; t++ )
		{
			if( GetTapNote(t, r) != TAP_EMPTY  &&  m_TapNoteScores[t][r] == TNS_NONE )
			{
				m_TapNoteScores[t][r] = TNS_MISS;
				iNumMissesFound++;
				iNumMissesThisRow++;
			}
		}
		if( iNumMissesThisRow > 0 )
		{
			HandleNoteScore( TNS_MISS, iNumMissesThisRow );
			m_Combo.UpdateScore( TNS_MISS, iNumMissesThisRow );
		}
	}

	if( iNumMissesFound > 0 )
	{
		m_Judgement.SetJudgement( TNS_MISS );
	}

	return iNumMissesFound;
}


void Player::CrossedRow( int iNoteRow )
{
	if( PREFSMAN->m_bAutoPlay  ||  GAMESTATE->m_bDemonstration )
	{
		// check to see if there's at the crossed row
		for( int t=0; t<m_iNumTracks; t++ )
		{
			if( GetTapNote(t, iNoteRow) != TAP_EMPTY )
				this->Step( t );
		}
	}
}


void Player::HandleNoteScore( TapNoteScore score, int iNumTapsInRow )
{
	ASSERT( iNumTapsInRow >= 1 );

	// don't accumulate points if AutoPlay is on.
	if( PREFSMAN->m_bAutoPlay  &&  !GAMESTATE->m_bDemonstration )
		return;

	if(m_ScoreKeeper)
		m_ScoreKeeper->HandleNoteScore(score, iNumTapsInRow);

	if (m_pScore)
		m_pScore->SetScore(GAMESTATE->m_fScore[m_PlayerNumber]);

	if( m_pLifeMeter ) {
		m_pLifeMeter->ChangeLife( score );
		m_pLifeMeter->OnDancePointsChange();    // update oni life meter
	}
}


void Player::HandleHoldNoteScore( HoldNoteScore score, TapNoteScore TapNoteScore )
{
	// don't accumulate points if AutoPlay is on.
	if( PREFSMAN->m_bAutoPlay  &&  !GAMESTATE->m_bDemonstration )
		return;

	if(m_ScoreKeeper) {
		m_ScoreKeeper->HandleHoldNoteScore(score, TapNoteScore);
	}

	if (m_pScore)
		m_pScore->SetScore(GAMESTATE->m_fScore[m_PlayerNumber]);

	if( m_pLifeMeter ) {
		if( score == HNS_NG ) {
			m_pLifeMeter->ChangeLife( score, TapNoteScore );
		
			// refresh Oni life meter
			m_pLifeMeter->OnDancePointsChange();
		}
	}
}

float Player::GetMaxBeatDifference()
{
	return GAMESTATE->m_fCurBPS * PREFSMAN->m_fJudgeWindowSeconds * GAMESTATE->m_SongOptions.m_fMusicRate;
}

void Player::FadeToFail()
{
	m_NoteField.FadeToFail();
}
