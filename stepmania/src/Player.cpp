#include "global.h"
/*
-----------------------------------------------------------------------------
 Class: Player

 Desc: See header.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "Player.h"
#include "GameConstantsAndTypes.h"
#include <math.h> // for fabs()
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
#include "ScoreDisplay.h"
#include "LifeMeter.h"
#include "PlayerAI.h"
#include "NoteFieldPositioning.h"
#include "NoteDataUtil.h"

#define GRAY_ARROWS_Y				THEME->GetMetricF("Player","GrayArrowsY")
#define JUDGMENT_X( p, both_sides )	THEME->GetMetricF("Player",both_sides ? "JudgmentXOffsetBothSides" : ssprintf("JudgmentXOffsetOneSideP%d",p+1))
#define JUDGMENT_Y					THEME->GetMetricF("Player","JudgmentY")
#define COMBO_X( p, both_sides )	THEME->GetMetricF("Player",both_sides ? "ComboXOffsetBothSides" : ssprintf("ComboXOffsetOneSideP%d",p+1))
#define COMBO_Y						THEME->GetMetricF("Player","ComboY")
#define HOLD_JUDGMENT_Y				THEME->GetMetricF("Player","HoldJudgmentY")
CachedThemeMetricI					BRIGHT_GHOST_COMBO_THRESHOLD("Player","BrightGhostComboThreshold");
#define START_DRAWING_AT_PIXELS		THEME->GetMetricI("Player","StartDrawingAtPixels")
#define STOP_DRAWING_AT_PIXELS		THEME->GetMetricI("Player","StopDrawingAtPixels")
#define MAX_PRO_TIMING_ERROR		THEME->GetMetricI("Player","MaxProTimingError")

/* Distance to search for a note in Step(). */
/* Units? */
static const float StepSearchDistanceBackwards = 1.0f;
static const float StepSearchDistanceForwards = 1.0f;



Player::Player()
{
	BRIGHT_GHOST_COMBO_THRESHOLD.Refresh();

	m_PlayerNumber = PLAYER_INVALID;
	m_bShowJudgment = true;

	m_pLifeMeter = NULL;
	m_pCombinedLifeMeter = NULL;
	m_pScore = NULL;
	m_pPrimaryScoreKeeper = NULL;
	m_pSecondaryScoreKeeper = NULL;
	m_pInventory = NULL;
	
	m_iOffsetSample = 0;

	this->AddChild( &m_ArrowBackdrop );
	this->AddChild( &m_NoteField );
	this->AddChild( &m_GrayArrowRow );
	this->AddChild( &m_GhostArrowRow );
	this->AddChild( &m_Judgment );
	this->AddChild( &m_ProTimingDisplay );
	this->AddChild( &m_Combo );
	for( int c=0; c<MAX_NOTE_TRACKS; c++ )
		this->AddChild( &m_HoldJudgment[c] );

	PlayerAI::InitFromDisk();
}

Player::~Player()
{
}

void Player::Load( PlayerNumber pn, NoteData* pNoteData, LifeMeter* pLM, CombinedLifeMeter* pCombinedLM, ScoreDisplay* pScore, Inventory* pInventory, ScoreKeeper* pPrimaryScoreKeeper, ScoreKeeper* pSecondaryScoreKeeper )
{
	//LOG->Trace( "Player::Load()", );

	m_PlayerNumber = pn;
	m_pLifeMeter = pLM;
	m_pCombinedLifeMeter = pCombinedLM;
	m_pScore = pScore;
	m_pInventory = pInventory;
	m_pPrimaryScoreKeeper = pPrimaryScoreKeeper;
	m_pSecondaryScoreKeeper = pSecondaryScoreKeeper;
	m_iRowLastCrossed = BeatToNoteRowNotRounded( GAMESTATE->m_fSongBeat ) - 1;
	// m_iRowLastCrossed = -1;

	/* Ensure that this is up-to-date. */
	GAMESTATE->m_pPosition->Load(pn);

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
	m_Judgment.StopTweening();
//	m_Combo.Reset();				// don't reset combos between songs in a course!
	m_Combo.Init( pn );
	m_Judgment.Reset();

	if( m_pScore )
		m_pScore->Init( pn );

	/* Apply transforms. */
	NoteDataUtil::TransformNoteData( *this, GAMESTATE->m_PlayerOptions[pn], GAMESTATE->GetCurrentStyleDef()->m_StepsType );

	int iStartDrawingAtPixels = GAMESTATE->m_bEditing ? -100 : START_DRAWING_AT_PIXELS;
	int iStopDrawingAtPixels = GAMESTATE->m_bEditing ? 400 : STOP_DRAWING_AT_PIXELS;

	m_ArrowBackdrop.Unload();
	CString BackdropName = g_NoteFieldMode[pn].m_Backdrop;
	if( !BackdropName.empty() )
		m_ArrowBackdrop.LoadFromAniDir( THEME->GetPathToB( BackdropName ) );

	m_NoteField.Load( (NoteData*)this, pn, iStartDrawingAtPixels, iStopDrawingAtPixels );
	
	m_ArrowBackdrop.SetPlayer( pn );
	m_GrayArrowRow.Load( pn );
	m_GhostArrowRow.Load( pn );

	const bool bReverse = GAMESTATE->m_PlayerOptions[pn].m_fReverseScroll == 1;
	bool bPlayerUsingBothSides = GAMESTATE->GetCurrentStyleDef()->m_StyleType==StyleDef::ONE_PLAYER_TWO_CREDITS;
	m_Combo.SetX( COMBO_X(m_PlayerNumber,bPlayerUsingBothSides) );
	m_Combo.SetY( bReverse ? SCREEN_BOTTOM-COMBO_Y : SCREEN_TOP+COMBO_Y );
	m_Judgment.SetX( JUDGMENT_X(m_PlayerNumber,bPlayerUsingBothSides) );
	m_Judgment.SetY( bReverse ? SCREEN_BOTTOM-JUDGMENT_Y : SCREEN_TOP+JUDGMENT_Y );
	m_ProTimingDisplay.SetX( JUDGMENT_X(m_PlayerNumber,bPlayerUsingBothSides) );
	m_ProTimingDisplay.SetY( bReverse ? SCREEN_BOTTOM-JUDGMENT_Y : SCREEN_TOP+JUDGMENT_Y );

	/* These commands add to the above positioning, and are usually empty. */
	m_Judgment.Command( g_NoteFieldMode[pn].m_JudgmentCmd );
	m_ProTimingDisplay.Command( g_NoteFieldMode[pn].m_JudgmentCmd );
	m_Combo.Command( g_NoteFieldMode[pn].m_ComboCmd );

	int c;
	for( c=0; c<pStyleDef->m_iColsPerPlayer; c++ )
	{
		m_HoldJudgment[c].SetY( bReverse ? SCREEN_BOTTOM-HOLD_JUDGMENT_Y : SCREEN_TOP+HOLD_JUDGMENT_Y );
		m_HoldJudgment[c].SetX( (float)pStyleDef->m_ColumnInfo[pn][c].fXOffset );
		m_HoldJudgment[c].Command( g_NoteFieldMode[pn].m_HoldJudgmentCmd[c] );
	}

	m_ArrowBackdrop.SetY( bReverse ? SCREEN_BOTTOM-GRAY_ARROWS_Y : SCREEN_TOP+GRAY_ARROWS_Y );
	m_NoteField.SetY( bReverse ? SCREEN_BOTTOM-GRAY_ARROWS_Y : SCREEN_TOP+GRAY_ARROWS_Y );
	m_GrayArrowRow.SetY( bReverse ? SCREEN_BOTTOM-GRAY_ARROWS_Y : SCREEN_TOP+GRAY_ARROWS_Y );
	m_GhostArrowRow.SetY( bReverse ? SCREEN_BOTTOM-GRAY_ARROWS_Y : SCREEN_TOP+GRAY_ARROWS_Y );

	if( GAMESTATE->m_PlayerOptions[pn].m_fEffects[PlayerOptions::EFFECT_MINI] == 1 )
	{
		m_NoteField.SetZoom( 0.5f );
		m_GrayArrowRow.SetZoom( 0.5f );
		m_GhostArrowRow.SetZoom( 0.5f );
	}

	m_sLastSeenNoteSkin = GAMESTATE->m_PlayerOptions[m_PlayerNumber].m_sNoteSkin;

	m_soundMineExplosion.Load( THEME->GetPathToS("Player explosion") );
}

void Player::Update( float fDeltaTime )
{
	//LOG->Trace( "Player::Update(%f)", fDeltaTime );

	if( GAMESTATE->m_pCurSong==NULL )
		return;

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
		int iHoldStartIndex = BeatToNoteRow(hn.fStartBeat);

		m_NoteField.m_bIsHoldingHoldNote[i] = false;	// set host flag so NoteField can do intelligent drawing


		if( hns != HNS_NONE )	// if this HoldNote already has a result
			continue;	// we don't need to update the logic for this one
		const StyleInput StyleI( m_PlayerNumber, hn.iTrack );
		const GameInput GameI = GAMESTATE->GetCurrentStyleDef()->StyleInputToGameInput( StyleI );

		// if they got a bad score or haven't stepped on the corresponding tap yet
		const TapNoteScore tns = GetTapNoteScore(hn.iTrack, iHoldStartIndex);
		const bool bSteppedOnTapNote = tns != TNS_NONE  &&  tns != TNS_MISS;	// did they step on the start of this hold?

		// If the song beat is in the range of this hold:
		if( hn.fStartBeat <= fSongBeat && fSongBeat <= hn.fEndBeat )
		{
			bool bIsHoldingButton = INPUTMAPPER->IsButtonDown( GameI );
			
			// TODO: Make the CPU miss sometimes.
			if( GAMESTATE->m_PlayerController[m_PlayerNumber] != PC_HUMAN )
				bIsHoldingButton = true;

			// set hold flag so NoteField can do intelligent drawing
			m_NoteField.m_bIsHoldingHoldNote[i] = bIsHoldingButton && bSteppedOnTapNote;

			if( bSteppedOnTapNote )		// this note is not judged and we stepped on its head
				m_NoteField.GetHoldNote(i).fStartBeat = fSongBeat;	// move the start of this Hold

			if( bSteppedOnTapNote && bIsHoldingButton )
			{
				// Increase life
				fLife = 1;

				m_GhostArrowRow.HoldNote( hn.iTrack );		// update the "electric ghost" effect
			}
			else
			{
				/* What is this conditional for?  It causes a problem: if a hold note
				 * begins on a freeze, you can tap it and then release it for the
				 * duration of the freeze; life doesn't count down until we're
				 * past the first beat. */
//				if( fSongBeat-hn.fStartBeat > GAMESTATE->m_fCurBPS * GetMaxStepDistanceSeconds() )
//				{
					// Decrease life
					fLife -= fDeltaTime/PREFSMAN->m_fJudgeWindowOKSeconds;
					fLife = max( fLife, 0 );	// clamp
//				}
			}
		}

		/* check for NG.  If the head was missed completely, don't count
		 * an NG. */
		if( bSteppedOnTapNote && fLife == 0 )	// the player has not pressed the button for a long time!
			hns = HNS_NG;

		// check for OK
		if( fSongBeat >= hn.fEndBeat && bSteppedOnTapNote && fLife > 0 )	// if this HoldNote is in the past
		{
			fLife = 1;
			hns = HNS_OK;
			m_GhostArrowRow.TapNote( StyleI.col, TNS_PERFECT, true );	// bright ghost flash
		}

		if( hns != HNS_NONE )
		{
			/* this note has been judged */
			HandleHoldScore( hns, tns );
			m_HoldJudgment[hn.iTrack].SetHoldJudgment( hns );

			int ms_error = (hns == HNS_OK)? 0:MAX_PRO_TIMING_ERROR;

			GAMESTATE->m_CurStageStats.iTotalError[m_PlayerNumber] += ms_error;
			if( hns == HNS_NG ) /* don't show a 0 for an OK */
				m_ProTimingDisplay.SetJudgment( ms_error, TNS_MISS );
		}

		m_NoteField.SetHoldNoteLife(i, fLife);	// update the NoteField display
		m_NoteField.SetHoldNoteScore(i, hns);	// update the NoteField display

		SetHoldNoteLife(i, fLife);
		SetHoldNoteScore(i, hns);
	}

	// Why was this originally "BeatToNoteRowNotRounded"?  It should be rounded.  -Chris
	/* We want to send the crossed row message exactly when we cross the row--not
	 * .5 before the row.  Use a very slow song (around 2 BPM) as a test case: without
	 * rounding, autoplay steps early. -glenn */
	const int iRowNow = BeatToNoteRowNotRounded( GAMESTATE->m_fSongBeat );
	if( iRowNow >= 0 )
	{
		for( ; m_iRowLastCrossed <= iRowNow; m_iRowLastCrossed++ )  // for each index we crossed since the last update
			if( GAMESTATE->IsPlayerEnabled(m_PlayerNumber) )
				CrossedRow( m_iRowLastCrossed );
	}


	// reload noteskin if it has changed
	if( m_sLastSeenNoteSkin != GAMESTATE->m_PlayerOptions[m_PlayerNumber].m_sNoteSkin )
	{
		m_NoteField.ReloadNoteSkin();
		m_sLastSeenNoteSkin = GAMESTATE->m_PlayerOptions[m_PlayerNumber].m_sNoteSkin;
	}


	ActorFrame::Update( fDeltaTime );
}

void Player::DrawPrimitives()
{
	// May have both players in doubles (for battle play); only draw primary player.
	if( GAMESTATE->GetCurrentStyleDef()->m_StyleType == StyleDef::ONE_PLAYER_TWO_CREDITS  &&
		m_PlayerNumber != GAMESTATE->m_MasterPlayerNumber )
		return;


	// Draw these below everything else.
	m_ArrowBackdrop.Draw();
	m_Combo.Draw();

	float fTilt = GAMESTATE->m_CurrentPlayerOptions[m_PlayerNumber].m_fPerspectiveTilt;
	bool bReverse = GAMESTATE->m_CurrentPlayerOptions[m_PlayerNumber].m_fReverseScroll==1;
	float fReverseScale = bReverse ? -1.0f : 1.0f;

	if( fTilt != 0 )
	{
		DISPLAY->EnterPerspective(45, false);

		// construct view and project matrix
		RageVector3 Up( 0.0f, 1.0f, 0.0f );
		RageVector3 Eye( CENTER_X, CENTER_Y+SCALE(fTilt*fReverseScale,-1,1,-350,350), 500 );
		// give a push the receptors toward the edge of the screen so they aren't so far in the middle
		float fYOffset = SCALE(fTilt,-1,+1,10*fReverseScale,60*fReverseScale);
		RageVector3 At( CENTER_X, CENTER_Y+fYOffset, 0 );

		DISPLAY->LookAt(Eye, At, Up);
	}

	m_GrayArrowRow.Draw();
	m_NoteField.Draw();
	m_GhostArrowRow.Draw();

	if( fTilt != 0 )
		DISPLAY->ExitPerspective();

	if (m_bShowJudgment)
	{
		if( GAMESTATE->m_PlayerOptions[m_PlayerNumber].m_bProTiming )
			m_ProTimingDisplay.Draw();
		else
			m_Judgment.Draw();
	}

	for( int c=0; c<GetNumTracks(); c++ )
		m_HoldJudgment[c].Draw();
}

/* It's OK for this function to search a little more than was requested. */
int Player::GetClosestNoteDirectional( int col, float fBeat, float fMaxBeatsDistance, int iDirection  )
{
	// look for the closest matching step
	const int iIndexStartLookingAt = BeatToNoteRow( fBeat );

	/* Number of elements to examine on either end of iIndexStartLookingAt.  Make
	 * sure we always round up. */
	const int iNumElementsToExamine = BeatToNoteRow( fMaxBeatsDistance + 1 );

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

int Player::GetClosestNote( int col, float fBeat, float fMaxBeatsAhead, float fMaxBeatsBehind )
{
	int Fwd = GetClosestNoteDirectional(col, fBeat, fMaxBeatsAhead, 1);
	int Back = GetClosestNoteDirectional(col, fBeat, fMaxBeatsBehind, -1);

	if(Fwd == -1 && Back == -1) return -1;
	if(Fwd == -1) return Back;
	if(Back == -1) return Fwd;

	/* Figure out which row is closer. */
	const float DistToFwd = fabsf(fBeat-NoteRowToBeat(Fwd));
	const float DistToBack = fabsf(fBeat-NoteRowToBeat(Back));
	
	if( DistToFwd > DistToBack ) return Back;
	return Fwd;
}


void Player::Step( int col, RageTimer tm )
{
	if( GAMESTATE->m_SongOptions.m_LifeType == SongOptions::LIFE_BATTERY  &&  GAMESTATE->m_CurStageStats.bFailed[m_PlayerNumber] )	// Oni dead
		return;	// do nothing

	//LOG->Trace( "Player::HandlePlayerStep()" );

	ASSERT( col >= 0  &&  col <= GetNumTracks() );

	int iIndexOverlappingNote = GetClosestNote( col, GAMESTATE->m_fSongBeat, 
						   StepSearchDistanceForwards * GAMESTATE->m_fCurBPS * GAMESTATE->m_SongOptions.m_fMusicRate,
						   StepSearchDistanceBackwards * GAMESTATE->m_fCurBPS * GAMESTATE->m_SongOptions.m_fMusicRate );
	
	//LOG->Trace( "iIndexStartLookingAt = %d, iNumElementsToExamine = %d", iIndexStartLookingAt, iNumElementsToExamine );

	bool bGrayArrowStep = true;

	if( iIndexOverlappingNote != -1 )
	{
		// compute the score for this hit
		const float fStepBeat = NoteRowToBeat( (float)iIndexOverlappingNote );

		const float fStepSeconds = GAMESTATE->m_pCurSong->GetElapsedTimeFromBeat(fStepBeat);

		/* XXX: Double-check that we're handling m_fMusicRate correctly. */
		/* We actually stepped on the note this long ago: */
		const float fAgo = tm.Ago();
		/* ... which means it happened at this point in the music: */
		const float fMusicSeconds = GAMESTATE->m_fMusicSeconds - (fAgo / GAMESTATE->m_SongOptions.m_fMusicRate);

		// The offset from the actual step in seconds:
		const float fNoteOffset = (fStepSeconds - fMusicSeconds) / GAMESTATE->m_SongOptions.m_fMusicRate;	// account for music rate

		const float fSecondsFromPerfect = fabsf( fNoteOffset );


		TapNote tn = GetTapNote(col,iIndexOverlappingNote);

		// calculate TapNoteScore
		TapNoteScore score;

		switch( GAMESTATE->m_PlayerController[m_PlayerNumber] )
		{
		case PC_HUMAN: {
			/* 1 is normal.  2 means scoring is half as hard; .5 means it's twice as hard. */
			float fScaledSecondsFromPerfect = fSecondsFromPerfect / PREFSMAN->m_fJudgeWindowScale;
			if(		 fScaledSecondsFromPerfect <= PREFSMAN->m_fJudgeWindowMarvelousSeconds )	score = TNS_MARVELOUS;
			else if( fScaledSecondsFromPerfect <= PREFSMAN->m_fJudgeWindowPerfectSeconds )	score = TNS_PERFECT;
			else if( fScaledSecondsFromPerfect <= PREFSMAN->m_fJudgeWindowGreatSeconds )		score = TNS_GREAT;
			else if( fScaledSecondsFromPerfect <= PREFSMAN->m_fJudgeWindowGoodSeconds )		score = TNS_GOOD;
			else if( fScaledSecondsFromPerfect <= PREFSMAN->m_fJudgeWindowBooSeconds )		score = TNS_BOO;
			else	score = TNS_NONE;

			// Penalize for stepping on mines
			if( tn == TAP_MINE  &&  score > TNS_NONE )
			{
				m_soundMineExplosion.Play();
				score = TNS_MISS;
				m_GhostArrowRow.TapMine( col, TNS_MISS );
			}

			break;
		}
		case PC_CPU:
			score = PlayerAI::GetTapNoteScore( GAMESTATE->m_iCpuSkill[m_PlayerNumber], GAMESTATE->GetSumOfActiveAttackLevels(m_PlayerNumber) );			

			/* AI will generate misses here.  Don't handle a miss like a regular note because
			 * we want the judgment animation to appear delayed.  Instead, return early if
			 * AI generated a miss, and let UpdateMissedTapNotesOlderThan() detect and handle the 
			 * misses. */
			if( score == TNS_MISS )
				return;

			// Unless the computer made a very good step, they were fooled by the mine
			if( tn == TAP_MINE  &&  score <= TNS_GREAT )
			{
				m_soundMineExplosion.Play();
				score = TNS_MISS;
				m_GhostArrowRow.TapMine( col, TNS_MISS );
			}
			break;
		case PC_AUTOPLAY:
			score = TNS_MARVELOUS;

			// Don't step on mines
			if( tn == TAP_MINE )
				return;
			break;
		default:
			ASSERT(0);
			score = TNS_NONE;
			break;
		}


		if( score != TNS_NONE && score != TNS_MISS )
		{
			int ms_error = (int) roundf( fSecondsFromPerfect * 1000 );
			ms_error = min( ms_error, MAX_PRO_TIMING_ERROR );

			GAMESTATE->m_CurStageStats.iTotalError[m_PlayerNumber] += ms_error;
			m_ProTimingDisplay.SetJudgment( ms_error, score );
		}

		if( score==TNS_MARVELOUS  &&  !GAMESTATE->ShowMarvelous())
			score = TNS_PERFECT;

		bGrayArrowStep = score < TNS_GOOD;

		LOG->Trace("Note offset: %f (fSecondsFromPerfect = %f), Score: %i", fNoteOffset, fSecondsFromPerfect, score);
		
		SetTapNoteScore(col, iIndexOverlappingNote, score);

		if( score != TNS_NONE )
			SetTapNoteOffset(col, iIndexOverlappingNote, -fNoteOffset);

		if( GAMESTATE->m_PlayerController[m_PlayerNumber] == PC_HUMAN  && 
			score >= TNS_GREAT ) 
			HandleAutosync(fNoteOffset);

		if( IsRowCompletelyJudged(iIndexOverlappingNote) )
			OnRowCompletelyJudged( iIndexOverlappingNote );
	}

	if( bGrayArrowStep )
		m_GrayArrowRow.Step( col );
}

void Player::HandleAutosync(float fNoteOffset)
{
	if( !GAMESTATE->m_SongOptions.m_bAutoSync )
		return;

	m_fOffset[m_iOffsetSample++] = fNoteOffset;
	if (m_iOffsetSample < SAMPLE_COUNT) 
		return; /* need more */

	const float mean = calc_mean(m_fOffset, m_fOffset+SAMPLE_COUNT);
	const float stddev = calc_stddev(m_fOffset, m_fOffset+SAMPLE_COUNT);

	if (stddev < .03 && stddev < fabsf(mean)) { //If they stepped with less than .03 error
		GAMESTATE->m_pCurSong->m_fBeat0OffsetInSeconds += mean;
		LOG->Trace("Offset corrected by %f. Error in steps: %f seconds.", mean, stddev);
	} else
		LOG->Trace("Offset NOT corrected. Average offset: %f seconds. Error: %f seconds.", mean, stddev);

	m_iOffsetSample = 0;
}


void Player::OnRowCompletelyJudged( int iIndexThatWasSteppedOn )
{
	LOG->Trace( "Player::OnRowCompletelyJudged" );
	
	/* Find the minimum score of the row.  This will never be TNS_NONE, since this
	 * function is only called when a row is completed. */
	/* Instead, use the last tap score (ala DDR).  Using the minimum results in 
	 * slightly more harsh scoring than DDR */
//	TapNoteScore score = MinTapNoteScore(iIndexThatWasSteppedOn);
	TapNoteScore score = LastTapNoteScore(iIndexThatWasSteppedOn);
	ASSERT(score != TNS_NONE);

	/* If the whole row was hit with perfects or greats, remove the row
	 * from the NoteField, so it disappears. */

	for( int c=0; c<GetNumTracks(); c++ )	// for each column
	{
		TapNote tn = GetTapNote(c, iIndexThatWasSteppedOn);

		if( tn == TAP_EMPTY )	continue; /* no note in this col */
		if( tn == TAP_MINE )	continue; /* don't flash on mines b/c they're supposed to be missed */

		// If the score is great or better, remove the note from the screen to 
		// indicate success.
		if( score >= TNS_GREAT )
			m_NoteField.SetTapNote(c, iIndexThatWasSteppedOn, TAP_EMPTY);

		// show the ghost arrow for this column
		switch( score )
		{
		case TNS_GREAT:
		case TNS_PERFECT:
		case TNS_MARVELOUS:
			{
				bool bBright = GAMESTATE->m_CurStageStats.iCurCombo[m_PlayerNumber]>(int)BRIGHT_GHOST_COMBO_THRESHOLD;
				m_GhostArrowRow.TapNote( c, score, bBright );
			}
			break;
		}
	}
		
	HandleTapRowScore( iIndexThatWasSteppedOn );	// update score
	m_Combo.SetCombo( GAMESTATE->m_CurStageStats.iCurCombo[m_PlayerNumber] );

	m_Judgment.SetJudgment( score );
}


void Player::UpdateTapNotesMissedOlderThan( float fMissIfOlderThanSeconds )
{
	//LOG->Trace( "Steps::UpdateTapNotesMissedOlderThan(%f)", fMissIfOlderThanThisBeat );
	const float fEarliestTime = GAMESTATE->m_fMusicSeconds - fMissIfOlderThanSeconds;
	bool bFreeze;
	float fMissIfOlderThanThisBeat;
	float fThrowAway;
	GAMESTATE->m_pCurSong->GetBeatAndBPSFromElapsedTime( fEarliestTime, fMissIfOlderThanThisBeat, fThrowAway, bFreeze );

	int iMissIfOlderThanThisIndex = BeatToNoteRow( fMissIfOlderThanThisBeat );
	if( bFreeze )
	{
		/* iMissIfOlderThanThisIndex is a freeze.  Include the index of the freeze,
		 * too.  Otherwise we won't show misses for tap notes on freezes until the
		 * freeze finishes. */
		iMissIfOlderThanThisIndex++;
	}

	// Since this is being called every frame, let's not check the whole array every time.
	// Instead, only check 10 elements back.  Even 10 is overkill.
	int iStartCheckingAt = max( 0, iMissIfOlderThanThisIndex-10 );

	//LOG->Trace( "iStartCheckingAt: %d   iMissIfOlderThanThisIndex:  %d", iStartCheckingAt, iMissIfOlderThanThisIndex );

	int iNumMissesFound = 0;
	int iNumMinesMissed = 0;
	for( int r=iStartCheckingAt; r<iMissIfOlderThanThisIndex; r++ )
	{
		bool MissedNoteOnThisRow = false;
		bool MissedMineOnThisRow = false;
		for( int t=0; t<GetNumTracks(); t++ )
		{
			if( GetTapNote(t, r) == TAP_EMPTY) continue; /* no note here */
			if( GetTapNoteScore(t, r) != TNS_NONE ) continue; /* note here is already hit */
			
			if( GetTapNote(t, r) == TAP_MINE )
			{
				// A mine.  Reward for not stepping on it.
				MissedMineOnThisRow = true;
				SetTapNoteScore(t, r, TNS_MARVELOUS);
			}
			else
			{
				// A normal note.  Penalize for not stepping on it.
				MissedNoteOnThisRow = true;
				SetTapNoteScore(t, r, TNS_MISS);
				GAMESTATE->m_CurStageStats.iTotalError[m_PlayerNumber] += MAX_PRO_TIMING_ERROR;
				m_ProTimingDisplay.SetJudgment( MAX_PRO_TIMING_ERROR, TNS_MISS );
			}
		}

		if( MissedNoteOnThisRow )
		{
			iNumMissesFound++;
			HandleTapRowScore( r );
			m_Combo.SetCombo( GAMESTATE->m_CurStageStats.iCurCombo[m_PlayerNumber] );
		}
		else if( MissedMineOnThisRow )
		{
			iNumMinesMissed++;
			HandleTapRowScore( r );
			m_Combo.SetCombo( GAMESTATE->m_CurStageStats.iCurCombo[m_PlayerNumber] );
		}
	}

	if( iNumMissesFound > 0 )
		m_Judgment.SetJudgment( TNS_MISS );
	else if( iNumMinesMissed > 0 )
		m_Judgment.SetJudgment( TNS_MARVELOUS );
}


void Player::CrossedRow( int iNoteRow )
{
	// If we're doing random vanish, randomise notes on the fly.
	if(GAMESTATE->m_CurrentPlayerOptions[m_PlayerNumber].m_fAppearances[PlayerOptions::APPEARANCE_RANDOMVANISH]==1)
		RandomiseNotes( iNoteRow );

	// check to see if there's at the crossed row
	RageTimer now;
	if( GAMESTATE->m_PlayerController[m_PlayerNumber] != PC_HUMAN )
	{
		for( int t=0; t<GetNumTracks(); t++ )
			if( GetTapNote(t, iNoteRow) != TAP_EMPTY )
				if( GetTapNoteScore(t, iNoteRow) == TNS_NONE )
					Step( t, now );
	}
}

void Player::RandomiseNotes( int iNoteRow )
{
	int NewNoteRow = (int)(iNoteRow + 50 / GAMESTATE->m_PlayerOptions[m_PlayerNumber].m_fScrollSpeed); // change the row to look ahead from based upon their speed mod
	// check to see if they're at the crossed row
//	int EmptyNoteCol = -1;
//	int WaitingForEmptyColumn = -1;

	bool b_updatenotedata = false;
	int iNumOfTracks = GetNumTracks();
	for(int t=0; t<iNumOfTracks; t++)
	{
		if(t+1 < iNumOfTracks)
		{
			int iRandomTrackToSwapWith = RandomInt(0, iNumOfTracks-1);
			TapNote t1 = GetTapNote(t, NewNoteRow);
			TapNote t2 = GetTapNote(iRandomTrackToSwapWith, NewNoteRow);
			if((t1 == TAP_TAP || t1 == TAP_EMPTY) && (t2 == TAP_TAP || t2 == TAP_EMPTY) && (!(t1 == TAP_EMPTY && t2 == TAP_EMPTY) && !(t1 == TAP_TAP && t2 == TAP_TAP)))
			{
				SetTapNote(t, NewNoteRow, t2);
				SetTapNote(iRandomTrackToSwapWith, NewNoteRow, t1);
				b_updatenotedata = true;
			}
		}
	}
	if(b_updatenotedata)
	{
		m_NoteField.CopyAll((NoteData*)this);
	}
}

void Player::HandleTapRowScore( unsigned row )
{
	TapNoteScore scoreOfLastTap = LastTapNoteScore(row);

	int iNumTapsInRow = 0;
	int iNumAdditions = 0;
	for( int t=0; t<GetNumTracks(); t++ )	// for each column
	{
		TapNote tn = GetTapNote(t, row);
		if( tn != TAP_EMPTY )
			iNumTapsInRow++;
		if( tn == TAP_ADDITION )
			iNumAdditions++;
	}

	ASSERT(iNumTapsInRow > 0);

	bool NoCheating = true;
#ifdef DEBUG
	NoCheating = false;
#endif

	if(GAMESTATE->m_bDemonstrationOrJukebox)
		NoCheating = false;
	// don't accumulate points if AutoPlay is on.
	if( NoCheating && GAMESTATE->m_PlayerController[m_PlayerNumber] == PC_AUTOPLAY )
		return;

	if(m_pPrimaryScoreKeeper)
		m_pPrimaryScoreKeeper->HandleTapRowScore(scoreOfLastTap, iNumTapsInRow, iNumAdditions );

	if(m_pSecondaryScoreKeeper)
		m_pSecondaryScoreKeeper->HandleTapRowScore(scoreOfLastTap, iNumTapsInRow, iNumAdditions );

	if (m_pScore)
		m_pScore->SetScore(GAMESTATE->m_CurStageStats.iScore[m_PlayerNumber]);

	if( m_pLifeMeter ) {
		m_pLifeMeter->ChangeLife( scoreOfLastTap );
		m_pLifeMeter->OnDancePointsChange();    // update oni life meter
	}
	if( m_pCombinedLifeMeter ) {
		m_pCombinedLifeMeter->ChangeLife( m_PlayerNumber, scoreOfLastTap );
		m_pCombinedLifeMeter->OnDancePointsChange( m_PlayerNumber );    // update oni life meter
	}
}


void Player::HandleHoldScore( HoldNoteScore holdScore, TapNoteScore tapScore )
{
	bool NoCheating = true;
#ifdef DEBUG
	NoCheating = false;
#endif

	if(GAMESTATE->m_bDemonstrationOrJukebox)
		NoCheating = false;
	// don't accumulate points if AutoPlay is on.
	if( NoCheating && GAMESTATE->m_PlayerController[m_PlayerNumber] == PC_AUTOPLAY )
		return;

	if(m_pPrimaryScoreKeeper)
		m_pPrimaryScoreKeeper->HandleHoldScore(holdScore, tapScore );
	if(m_pSecondaryScoreKeeper)
		m_pSecondaryScoreKeeper->HandleHoldScore(holdScore, tapScore );

	if (m_pScore)
		m_pScore->SetScore(GAMESTATE->m_CurStageStats.iScore[m_PlayerNumber]);

	if( m_pLifeMeter ) {
		m_pLifeMeter->ChangeLife( holdScore, tapScore );
		m_pLifeMeter->OnDancePointsChange();
	}
	if( m_pCombinedLifeMeter ) {
		m_pCombinedLifeMeter->ChangeLife( m_PlayerNumber, holdScore, tapScore );
		m_pCombinedLifeMeter->OnDancePointsChange( m_PlayerNumber );
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
