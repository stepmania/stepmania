#include "global.h"
/*
-----------------------------------------------------------------------------
 Class: Player

 Desc: See header.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "GameConstantsAndTypes.h"
#include <math.h> // for fabs()
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
#include "RageDisplay.h"
#include "ThemeManager.h"
#include "Combo.h"

#define JUDGE_MARVELOUS_ZOOM_X		THEME->GetMetricF("Player","JudgeMarvelousZoomX")
#define JUDGE_MARVELOUS_ZOOM_Y		THEME->GetMetricF("Player","JudgeMarvelousZoomY")
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
float 
	g_fJudgeMarvelousZoomX,
	g_fJudgeMarvelousZoomY,
	g_fJudgePerfectZoomX,
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
	g_fJudgeMarvelousZoomX = JUDGE_MARVELOUS_ZOOM_X;
	g_fJudgeMarvelousZoomY = JUDGE_MARVELOUS_ZOOM_Y;
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


int Player::GetPlayersMaxCombo()
{
	return(	m_Combo.GetMaxCombo() );
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
	if( GAMESTATE->m_SongOptions.m_LifeType == SongOptions::LIFE_BATTERY  &&  GAMESTATE->m_CurStageStats.bFailed[pn] )	// Oni dead
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
		NoteDataUtil::RemoveHoldNotes(*this);

	NoteDataUtil::Turn( *this, GAMESTATE->m_PlayerOptions[pn].m_TurnType );

	if( GAMESTATE->m_PlayerOptions[pn].m_bLittle )
		NoteDataUtil::MakeLittle(*this);

	int iPixelsToDrawBefore = -60;
	int iPixelsToDrawAfter = 350;

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

	int c;
	for( c=0; c<pStyleDef->m_iColsPerPlayer; c++ )
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
	UpdateTapNotesMissedOlderThan( GetMaxStepDistanceSeconds() );

	//
	// update HoldNotes logic
	//
	for( int i=0; i < GetNumHoldNotes(); i++ )		// for each HoldNote
	{
		const HoldNote &hn = GetHoldNote(i);
		HoldNoteScore hns = GetHoldNoteScore(i);
		float fLife = GetHoldNoteLife(i);
		int iHoldStartIndex = BeatToNoteRow(hn.m_fStartBeat);

		m_NoteField.m_bIsHoldingHoldNote[i] = false;	// set host flag so NoteField can do intelligent drawing


		if( hns != HNS_NONE )	// if this HoldNote already has a result
			continue;	// we don't need to update the logic for this one
		const StyleInput StyleI( m_PlayerNumber, hn.m_iTrack );
		const GameInput GameI = GAMESTATE->GetCurrentStyleDef()->StyleInputToGameInput( StyleI );

		// if they got a bad score or haven't stepped on the corresponding tap yet
		const TapNoteScore tns = GetTapNoteScore(hn.m_iTrack, iHoldStartIndex);
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
				if( fSongBeat-hn.m_fStartBeat > GAMESTATE->m_fCurBPS * GetMaxStepDistanceSeconds() )
				{
					// Decrease life
					fLife -= fDeltaTime/HOLD_ARROW_NG_TIME;
					fLife = max( fLife, 0 );	// clamp
				}
			}
		}

		/* check for NG.  If the head was missed completely, don't count
		 * an NG. */
		if( bSteppedOnTapNote && fLife == 0 )	// the player has not pressed the button for a long time!
			hns = HNS_NG;

		// check for OK
		if( fSongBeat >= hn.m_fEndBeat && bSteppedOnTapNote && fLife > 0 )	// if this HoldNote is in the past
		{
			fLife = 1;
			hns = HNS_OK;
			m_GhostArrowRow.TapNote( StyleI.col, TNS_PERFECT, true );	// bright ghost flash
		}

		if( hns != HNS_NONE )
		{
			/* this note's been judged */
			HandleHoldNoteScore( hns, tns );
			m_HoldJudgement[hn.m_iTrack].SetHoldJudgement( hns );
		}

		m_NoteField.SetHoldNoteLife(i, fLife);	// update the NoteField display
		m_NoteField.SetHoldNoteScore(i, hns);	// update the NoteField display

		SetHoldNoteLife(i, fLife);
		SetHoldNoteScore(i, hns);
	}



	ActorFrame::Update( fDeltaTime );
}

void Player::DrawPrimitives()
{
	m_frameCombo.Draw();	// draw this below everything else

	if( GAMESTATE->m_PlayerOptions[m_PlayerNumber].m_bEffects[PlayerOptions::EFFECT_SPACE] )
	{
		DISPLAY->PushMatrix();
		DISPLAY->EnterPerspective(45, false);

		// construct view and project matrix
		RageVector3 Eye, At, Up( 0.0f, 1.0f, 0.0f );
		if( GAMESTATE->m_PlayerOptions[m_PlayerNumber].m_bReverseScroll ) {
			Eye = RageVector3( CENTER_X, -300.0f, 400.0f );
			At = RageVector3( CENTER_X, 100.0f, 0.0f );
		} else {
			Eye = RageVector3( CENTER_X, 800, 400 );
			At = RageVector3( CENTER_X, 400, 0.0f );
		}

		DISPLAY->LookAt(Eye, At, Up);
	}

	m_GrayArrowRow.Draw();
	m_NoteField.Draw();
	m_GhostArrowRow.Draw();

	if( GAMESTATE->m_PlayerOptions[m_PlayerNumber].m_bEffects[PlayerOptions::EFFECT_SPACE] )
	{
		DISPLAY->ExitPerspective();
		DISPLAY->PopMatrix();
	}

	m_frameJudgement.Draw();

	for( int c=0; c<GetNumTracks(); c++ )
		m_HoldJudgement[c].Draw();
}

int Player::GetClosestBeatDirectional( int col, float fBeat, float fMaxSecondsDistance, int iDirection  )
{
	// look for the closest matching step
	const int iIndexStartLookingAt = BeatToNoteRow( fBeat );

	// number of elements to examine on either end of iIndexStartLookingAt
	const int iNumElementsToExamine = BeatToNoteRow( fMaxSecondsDistance );

	// Start at iIndexStartLookingAt and search outward.
	for( int delta=0; delta < iNumElementsToExamine; delta++ )
	{
		int iCurrentIndex = iIndexStartLookingAt + (iDirection * delta);

		if( iCurrentIndex < 0) continue;
		if( GetTapNote(col, iCurrentIndex) == TAP_EMPTY) continue; /* no note here */
		if( GetTapNoteScore(col, iCurrentIndex) != TNS_NONE ) continue;	/* this note has a score already */

		return iCurrentIndex;
	}

	return -1;
}

int Player::GetClosestBeat( int col, float fBeat, float fMaxBeatsAhead, float fMaxBeatsBehind )
{
	int Fwd = GetClosestBeatDirectional(col, fBeat, fMaxBeatsAhead, 1);
	int Back = GetClosestBeatDirectional(col, fBeat, fMaxBeatsBehind, -1);

	if(Fwd == -1 && Back == -1) return -1;
	if(Fwd == -1) return Back;
	if(Back == -1) return Fwd;

	/* Figure out which row is closer. */
	const float DistToFwd = fabsf(fBeat-NoteRowToBeat(Fwd));
	const float DistToBack = fabsf(fBeat-NoteRowToBeat(Back));
	
	if( DistToFwd > DistToBack ) return Back;
	return Fwd;
}

void Player::Step( int col )
{
	if( GAMESTATE->m_SongOptions.m_LifeType == SongOptions::LIFE_BATTERY  &&  GAMESTATE->m_CurStageStats.bFailed[m_PlayerNumber] )	// Oni dead
		return;	// do nothing

	//LOG->Trace( "Player::HandlePlayerStep()" );

	ASSERT( col >= 0  &&  col <= GetNumTracks() );

	int iIndexOverlappingNote = GetClosestBeat( col, GAMESTATE->m_fSongBeat, 
						   GAMESTATE->m_fCurBPS * GAMESTATE->m_SongOptions.m_fMusicRate,
						   GAMESTATE->m_fCurBPS * GAMESTATE->m_SongOptions.m_fMusicRate );
	
	//LOG->Trace( "iIndexStartLookingAt = %d, iNumElementsToExamine = %d", iIndexStartLookingAt, iNumElementsToExamine );

	bool bDestroyedNote = false;

	if( iIndexOverlappingNote != -1 )
	{
		// compute the score for this hit
		const float fStepBeat = NoteRowToBeat( (float)iIndexOverlappingNote );
		// const float fBeatsUntilStep = fStepBeat - fSongBeat;
		const float fStepSeconds = GAMESTATE->m_pCurSong->GetElapsedTimeFromBeat(fStepBeat);
		// The offset from the actual step in seconds:
		const float fNoteOffset = fStepSeconds - GAMESTATE->m_fMusicSeconds;
		// const float fNoteOffset = fBeatsUntilStep / GAMESTATE->m_fCurBPS;
		const float fSecondsFromPerfect = fabsf( fNoteOffset );


		TapNoteScore score;

		LOG->Trace("fSecondsFromPerfect = %f", fSecondsFromPerfect);
		if(		 fSecondsFromPerfect <= PREFSMAN->m_fJudgeWindowMarvelousSeconds )	score = TNS_MARVELOUS;
		else if( fSecondsFromPerfect <= PREFSMAN->m_fJudgeWindowPerfectSeconds )	score = TNS_PERFECT;
		else if( fSecondsFromPerfect <= PREFSMAN->m_fJudgeWindowGreatSeconds )		score = TNS_GREAT;
		else if( fSecondsFromPerfect <= PREFSMAN->m_fJudgeWindowGoodSeconds )		score = TNS_GOOD;
		else if( fSecondsFromPerfect <= PREFSMAN->m_fJudgeWindowBooSeconds )		score = TNS_BOO;
		else																		score = TNS_NONE;

		if( !GAMESTATE->m_bEditing && (GAMESTATE->m_bDemonstration  ||  PREFSMAN->m_bAutoPlay) )
			score = TNS_MARVELOUS;

		if( score==TNS_MARVELOUS  &&  !PREFSMAN->m_bMarvelousTiming )
			score = TNS_PERFECT;

		bDestroyedNote = (score >= TNS_GOOD);

		LOG->Trace("(%2d/%2d)Note offset: %f, Score: %i", m_iOffsetSample, SAMPLE_COUNT, fNoteOffset, score);
		SetTapNoteScore(col, iIndexOverlappingNote, score);

		if (GAMESTATE->m_SongOptions.m_bAutoSync ) 
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
			for( int t=0; t<GetNumTracks(); t++ )			// did this complete the elimination of the row?
			{
				if( GetTapNote(t, iIndexOverlappingNote) != TAP_EMPTY  &&			// there is a note here
					GetTapNoteScore(t, iIndexOverlappingNote) == TNS_NONE )			// and it doesn't have a score
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
	TapNoteScore score = TNS_MARVELOUS;
	for( int t=0; t<GetNumTracks(); t++ )
	{
		TapNoteScore tns = GetTapNoteScore(t, iIndexThatWasSteppedOn);
		if( tns >= TNS_BOO )
			score = min( score, tns );
	}

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
	switch ( score )
	{
	case TNS_MARVELOUS:
	case TNS_PERFECT:
	case TNS_GREAT:
		m_NoteField.RemoveTapNoteRow( iIndexThatWasSteppedOn );
		break;
	}

	int iNumNotesInThisRow = 0;
	for( int c=0; c<GetNumTracks(); c++ )	// for each column
	{
		if( GetTapNote(c, iIndexThatWasSteppedOn) != TAP_EMPTY )	// if there is a note in this col
		{
			iNumNotesInThisRow++;

			// show the ghost arrow for this column
			if(score == TNS_GREAT || score == TNS_PERFECT || score == TNS_MARVELOUS)
				m_GhostArrowRow.TapNote( c, score, m_Combo.GetCurrentCombo()>g_iBrightGhostThreshold );
		}
	}
		
	if( iNumNotesInThisRow > 0 )
	{
		HandleNoteScore( score, iNumNotesInThisRow );	// update score
		m_Combo.UpdateScore( score, iNumNotesInThisRow );
		GAMESTATE->m_CurStageStats.iMaxCombo[m_PlayerNumber] = max( GAMESTATE->m_CurStageStats.iMaxCombo[m_PlayerNumber], m_Combo.GetCurrentCombo() );
	}

	// update the judgement, score, and life
	m_Judgement.SetJudgement( score );

	// zoom the judgement and combo like a heart beat
	float fStartZoomX=0.f, fStartZoomY=0.f;
	switch( score )
	{
	case TNS_MARVELOUS:	fStartZoomX = g_fJudgeMarvelousZoomX;	fStartZoomY = g_fJudgeMarvelousZoomY;	break;
	case TNS_PERFECT:	fStartZoomX = g_fJudgePerfectZoomX;		fStartZoomY = g_fJudgePerfectZoomY;		break;
	case TNS_GREAT:		fStartZoomX = g_fJudgeGreatZoomX;		fStartZoomY = g_fJudgeGreatZoomY;		break;
	case TNS_GOOD:		fStartZoomX = g_fJudgeGoodZoomX;		fStartZoomY = g_fJudgeGoodZoomY;		break;
	case TNS_BOO:		fStartZoomX = g_fJudgeBooZoomX;			fStartZoomY = g_fJudgeBooZoomY;			break;
	default:
		ASSERT(0);
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


int Player::UpdateTapNotesMissedOlderThan( float fMissIfOlderThanSeconds )
{
	//LOG->Trace( "Notes::UpdateTapNotesMissedOlderThan(%f)", fMissIfOlderThanThisBeat );
	const float fEarliestTime = GAMESTATE->m_fMusicSeconds - fMissIfOlderThanSeconds;
	const float fMissIfOlderThanThisBeat = GAMESTATE->m_pCurSong->GetBeatFromElapsedTime(fEarliestTime);

	int iMissIfOlderThanThisIndex = BeatToNoteRow( fMissIfOlderThanThisBeat );

	// Since this is being called every frame, let's not check the whole array every time.
	// Instead, only check 10 elements back.  Even 10 is overkill.
	int iStartCheckingAt = max( 0, iMissIfOlderThanThisIndex-10 );

	//LOG->Trace( "iStartCheckingAt: %d   iMissIfOlderThanThisIndex:  %d", iStartCheckingAt, iMissIfOlderThanThisIndex );

	/* If we're on a freeze, and the freeze has been running for fMissIfOlderThanSeconds,
	 * then iMissIfOlderThanThisIndex will be the freeze itself, in which case we do
	 * want to update the row of the freeze itself; otherwise we won't show misses
	 * for tap notes on freezes until the freeze finishes. */
	int iNumMissesFound = 0;
	for( int r=iStartCheckingAt; r<=iMissIfOlderThanThisIndex; r++ )
	{
		int iNumMissesThisRow = 0;
		for( int t=0; t<GetNumTracks(); t++ )
		{
			if( GetTapNote(t, r) != TAP_EMPTY  &&  GetTapNoteScore(t, r) == TNS_NONE )
			{
				SetTapNoteScore(t, r, TNS_MISS);
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
		for( int t=0; t<GetNumTracks(); t++ )
		{
			if( GetTapNote(t, iNoteRow) != TAP_EMPTY )
				this->Step( t );
		}
	}
}


void Player::HandleNoteScore( TapNoteScore score, int iNumTapsInRow )
{
	ASSERT( iNumTapsInRow >= 1 );

#ifndef DEBUG
	// don't accumulate points if AutoPlay is on.
	if( PREFSMAN->m_bAutoPlay  &&  !GAMESTATE->m_bDemonstration )
		return;
#endif //DEBUG

	if(m_ScoreKeeper)
		m_ScoreKeeper->HandleNoteScore(score, iNumTapsInRow);

	if (m_pScore)
		m_pScore->SetScore(GAMESTATE->m_CurStageStats.fScore[m_PlayerNumber]);

	if( m_pLifeMeter ) {
		m_pLifeMeter->ChangeLife( score );
		m_pLifeMeter->OnDancePointsChange();    // update oni life meter
	}
}


void Player::HandleHoldNoteScore( HoldNoteScore score, TapNoteScore TapNoteScore )
{
#ifndef DEBUG
	// don't accumulate points if AutoPlay is on.
	if( PREFSMAN->m_bAutoPlay  &&  !GAMESTATE->m_bDemonstration )
		return;
#endif //DEBUG

	if(m_ScoreKeeper) {
		m_ScoreKeeper->HandleHoldNoteScore(score, TapNoteScore);
	}

	if (m_pScore)
		m_pScore->SetScore(GAMESTATE->m_CurStageStats.fScore[m_PlayerNumber]);

	if( m_pLifeMeter ) {
		if( score == HNS_NG ) {
			m_pLifeMeter->ChangeLife( score, TapNoteScore );
		
			// refresh Oni life meter
			m_pLifeMeter->OnDancePointsChange();
		}
	}
}

float Player::GetMaxStepDistanceSeconds()
{
	return GAMESTATE->m_SongOptions.m_fMusicRate * PREFSMAN->m_fJudgeWindowBooSeconds * PREFSMAN->m_fJudgeWindowScale;
}

void Player::FadeToFail()
{
	m_NoteField.FadeToFail();
}
