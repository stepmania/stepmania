#include "global.h"
#include "Player.h"
#include "GameConstantsAndTypes.h"
#include "RageUtil.h"
#include "PrefsManager.h"
#include "GameConstantsAndTypes.h"
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
#include "CombinedLifeMeter.h"
#include "PlayerAI.h"
#include "NoteFieldPositioning.h"
#include "NoteDataUtil.h"
#include "ScreenGameplay.h" /* for SM_ComboStopped */
#include "ScreenManager.h"
#include "StageStats.h"
#include "ArrowEffects.h"
#include "Game.h"
#include "NetworkSyncManager.h"	//used for sending timing offset
#include "DancingCharacters.h"

CachedThemeMetricF GRAY_ARROWS_Y_STANDARD		("Player","ReceptorArrowsYStandard");
CachedThemeMetricF GRAY_ARROWS_Y_REVERSE		("Player","ReceptorArrowsYReverse");
#define JUDGMENT_X( p, both_sides )				THEME->GetMetricF("Player",both_sides ? CString("JudgmentXOffsetBothSides") : ssprintf("JudgmentXOffsetOneSideP%d",p+1))
#define JUDGMENT_Y								THEME->GetMetricF("Player","JudgmentY")
#define JUDGMENT_Y_REVERSE						THEME->GetMetricF("Player","JudgmentYReverse")
#define COMBO_X( p, both_sides )				THEME->GetMetricF("Player",both_sides ? CString("ComboXOffsetBothSides") : ssprintf("ComboXOffsetOneSideP%d",p+1))
#define COMBO_Y									THEME->GetMetricF("Player","ComboY")
#define COMBO_Y_REVERSE							THEME->GetMetricF("Player","ComboYReverse")
#define ATTACK_DISPLAY_X( p, both_sides )		THEME->GetMetricF("Player",both_sides ? CString("AttackDisplayXOffsetBothSides") : ssprintf("AttackDisplayXOffsetOneSideP%d",p+1))
#define ATTACK_DISPLAY_Y						THEME->GetMetricF("Player","AttackDisplayY")
#define ATTACK_DISPLAY_Y_REVERSE				THEME->GetMetricF("Player","AttackDisplayYReverse")
CachedThemeMetricF HOLD_JUDGMENT_Y_STANDARD		("Player","HoldJudgmentYStandard");
CachedThemeMetricF HOLD_JUDGMENT_Y_REVERSE		("Player","HoldJudgmentYReverse");
CachedThemeMetricI	BRIGHT_GHOST_COMBO_THRESHOLD("Player","BrightGhostComboThreshold");
CachedThemeMetricB	TAP_JUDGMENTS_UNDER_FIELD	("Player","TapJudgmentsUnderField");
CachedThemeMetricB	HOLD_JUDGMENTS_UNDER_FIELD	("Player","HoldJudgmentsUnderField");
#define START_DRAWING_AT_PIXELS					THEME->GetMetricI("Player","StartDrawingAtPixels")
#define STOP_DRAWING_AT_PIXELS					THEME->GetMetricI("Player","StopDrawingAtPixels")
#define MAX_PRO_TIMING_ERROR					THEME->GetMetricI("Player","MaxProTimingError")

/* Distance to search for a note in Step(). */
/* Units? */
static const float StepSearchDistanceBackwards = 1.0f;
static const float StepSearchDistanceForwards = 1.0f;

#define ADJUSTED_WINDOW( judge ) ((PREFSMAN->m_fJudgeWindowSeconds##judge * PREFSMAN->m_fJudgeWindowScale) + PREFSMAN->m_fJudgeWindowAdd)


PlayerMinus::PlayerMinus()
{
	GRAY_ARROWS_Y_STANDARD.Refresh();
	GRAY_ARROWS_Y_REVERSE.Refresh();
	HOLD_JUDGMENT_Y_STANDARD.Refresh();
	HOLD_JUDGMENT_Y_REVERSE.Refresh();
	BRIGHT_GHOST_COMBO_THRESHOLD.Refresh();
	TAP_JUDGMENTS_UNDER_FIELD.Refresh();
	HOLD_JUDGMENTS_UNDER_FIELD.Refresh();

	m_PlayerNumber = PLAYER_INVALID;
	m_fNoteFieldHeight = 0;

	m_pLifeMeter = NULL;
	m_pCombinedLifeMeter = NULL;
	m_pScoreDisplay = NULL;
	m_pSecondaryScoreDisplay = NULL;
	m_pPrimaryScoreKeeper = NULL;
	m_pSecondaryScoreKeeper = NULL;
	m_pInventory = NULL;
	
	m_iOffsetSample = 0;

	this->AddChild( &m_ArrowBackdrop );
	this->AddChild( &m_Judgment );
	this->AddChild( &m_ProTimingDisplay );
	this->AddChild( &m_Combo );
	this->AddChild( &m_AttackDisplay );
	for( int c=0; c<MAX_NOTE_TRACKS; c++ )
		this->AddChild( &m_HoldJudgment[c] );


	PlayerAI::InitFromDisk();
}

PlayerMinus::~PlayerMinus()
{
}

void PlayerMinus::Load( PlayerNumber pn, const NoteData* pNoteData, LifeMeter* pLM, CombinedLifeMeter* pCombinedLM, ScoreDisplay* pScoreDisplay, ScoreDisplay* pSecondaryScoreDisplay, Inventory* pInventory, ScoreKeeper* pPrimaryScoreKeeper, ScoreKeeper* pSecondaryScoreKeeper, NoteField* pNoteField )
{
	m_iDCState = AS2D_IDLE;
	//LOG->Trace( "PlayerMinus::Load()", );

	GAMESTATE->ResetNoteSkinsForPlayer( pn );
	
	m_PlayerNumber = pn;
	m_pLifeMeter = pLM;
	m_pCombinedLifeMeter = pCombinedLM;
	m_pScoreDisplay = pScoreDisplay;
	m_pSecondaryScoreDisplay = pSecondaryScoreDisplay;
	m_pInventory = pInventory;
	m_pPrimaryScoreKeeper = pPrimaryScoreKeeper;
	m_pSecondaryScoreKeeper = pSecondaryScoreKeeper;
	m_pNoteField = pNoteField;
	m_iRowLastCrossed = BeatToNoteRowNotRounded( GAMESTATE->m_fSongBeat ) - 1;	// why this?
	m_iMineRowLastCrossed = BeatToNoteRowNotRounded( GAMESTATE->m_fSongBeat ) - 1;	// why this?

	/* Ensure that this is up-to-date. */
	GAMESTATE->m_pPosition->Load(pn);

	const Style* pStyle = GAMESTATE->GetCurrentStyle();

	// init scoring
	NoteDataWithScoring::Init();

	// copy note data
	this->CopyAll( pNoteData );
	if( GAMESTATE->m_SongOptions.m_LifeType == SongOptions::LIFE_BATTERY  &&  g_CurStageStats.bFailed[pn] )	// Oni dead
		this->ClearAll();

	/* The editor reuses Players ... so we really need to make sure everything
	 * is reset and not tweening.  Perhaps ActorFrame should recurse to subactors;
	 * then we could just this->StopTweening()? -glenn */
	m_Judgment.StopTweening();
//	m_Combo.Reset();				// don't reset combos between songs in a course!
	m_Combo.Init( pn );
	m_Combo.SetCombo( g_CurStageStats.iCurCombo[m_PlayerNumber], g_CurStageStats.iCurMissCombo[m_PlayerNumber]  );	// combo can persist between songs and games
	m_AttackDisplay.Init( pn );
	m_Judgment.Reset();

	/* Don't re-init this; that'll reload graphics.  Add a separate Reset() call
	 * if some ScoreDisplays need it. */
//	if( m_pScore )
//		m_pScore->Init( pn );

	/* Apply transforms. */
	NoteDataUtil::TransformNoteData( *this, GAMESTATE->m_PlayerOptions[pn], GAMESTATE->GetCurrentStyle()->m_StepsType );
	
	switch( GAMESTATE->m_PlayMode )
	{
		case PLAY_MODE_RAVE:
		case PLAY_MODE_BATTLE:
			{
				// ugly, ugly, ugly.  Works only w/ dance.
				NoteDataUtil::TransformNoteData( *this, GAMESTATE->m_PlayerOptions[pn], GAMESTATE->GetCurrentStyle()->m_StepsType );
				
				// shuffle either p1 or p2
				static int count = 0;
				switch( count )
				{
				case 0:
				case 3:
					NoteDataUtil::Turn( *this, STEPS_TYPE_DANCE_SINGLE, NoteDataUtil::left);
					break;
				case 1:
				case 2:
					NoteDataUtil::Turn( *this, STEPS_TYPE_DANCE_SINGLE, NoteDataUtil::right);
					break;
				default:
					ASSERT(0);
				}
				count++;
				count %= 4;
			}
			break;
	}

	int iStartDrawingAtPixels = GAMESTATE->m_bEditing ? -100 : START_DRAWING_AT_PIXELS;
	int iStopDrawingAtPixels = GAMESTATE->m_bEditing ? 400 : STOP_DRAWING_AT_PIXELS;

	m_ArrowBackdrop.Unload();
	CString BackdropName = g_NoteFieldMode[pn].m_Backdrop;
	if( !BackdropName.empty() )
		m_ArrowBackdrop.LoadFromAniDir( THEME->GetPathToB( BackdropName ) );

	float fNoteFieldMidde = (GRAY_ARROWS_Y_STANDARD+GRAY_ARROWS_Y_REVERSE)/2;
	m_pNoteField->SetY( fNoteFieldMidde );
	m_fNoteFieldHeight = GRAY_ARROWS_Y_REVERSE-GRAY_ARROWS_Y_STANDARD;
	m_pNoteField->Load( this, pn, iStartDrawingAtPixels, iStopDrawingAtPixels, m_fNoteFieldHeight );
	m_ArrowBackdrop.SetPlayer( pn );

	const bool bReverse = GAMESTATE->m_PlayerOptions[pn].GetReversePercentForColumn(0) == 1;
	bool bPlayerUsingBothSides = GAMESTATE->GetCurrentStyle()->m_StyleType==Style::ONE_PLAYER_TWO_CREDITS;
	m_Combo.SetX( COMBO_X(m_PlayerNumber,bPlayerUsingBothSides) );
	m_Combo.SetY( bReverse ? COMBO_Y_REVERSE : COMBO_Y );
	m_AttackDisplay.SetX( ATTACK_DISPLAY_X(m_PlayerNumber,bPlayerUsingBothSides) );
	m_AttackDisplay.SetY( bReverse ? ATTACK_DISPLAY_Y_REVERSE : ATTACK_DISPLAY_Y );
	m_Judgment.SetX( JUDGMENT_X(m_PlayerNumber,bPlayerUsingBothSides) );
	m_Judgment.SetY( bReverse ? JUDGMENT_Y_REVERSE : JUDGMENT_Y );
	m_ProTimingDisplay.SetX( JUDGMENT_X(m_PlayerNumber,bPlayerUsingBothSides) );
	m_ProTimingDisplay.SetY( bReverse ? SCREEN_BOTTOM-JUDGMENT_Y : SCREEN_TOP+JUDGMENT_Y );

	/* These commands add to the above positioning, and are usually empty. */
	m_Judgment.Command( g_NoteFieldMode[pn].m_JudgmentCmd );
	m_ProTimingDisplay.Command( g_NoteFieldMode[pn].m_JudgmentCmd );
	m_Combo.Command( g_NoteFieldMode[pn].m_ComboCmd );
	m_AttackDisplay.Command( g_NoteFieldMode[pn].m_AttackDisplayCmd );

	int c;
	for( c=0; c<pStyle->m_iColsPerPlayer; c++ )
	{
		NoteFieldMode &mode = g_NoteFieldMode[pn];
		m_HoldJudgment[c].Command( mode.m_HoldJudgmentCmd[c] );
	}

	// Need to set Y positions of all these elements in Update since
	// they change depending on PlayerOptions.

	RageSoundParams p;
	m_soundMine.Load( THEME->GetPathToS("Player mine"), true );

	/* Attacks can be launched in course modes and in battle modes.  They both come
	 * here to play, but allow loading a different sound for different modes. */
	switch( GAMESTATE->m_PlayMode )
	{
	case PLAY_MODE_RAVE:
	case PLAY_MODE_BATTLE:
		m_soundAttackLaunch.Load( THEME->GetPathToS("Player battle attack launch"), true );
		m_soundAttackEnding.Load( THEME->GetPathToS("Player battle attack ending"), true );
		break;
	default:
		m_soundAttackLaunch.Load( THEME->GetPathToS("Player course attack launch"), true );
		m_soundAttackEnding.Load( THEME->GetPathToS("Player course attack ending"), true );
		break;
	}

	if( GAMESTATE->GetNumPlayersEnabled() == 2 )
	{
		/* Two players are active.  Play sounds on this player's side. */
		p.m_Balance = (m_PlayerNumber == PLAYER_1)? -1.0f:1.0f;
	}
	m_soundMine.SetParams( p );
	m_soundAttackLaunch.SetParams( p );
	m_soundAttackEnding.SetParams( p );
}

void PlayerMinus::Update( float fDeltaTime )
{
	//LOG->Trace( "PlayerMinus::Update(%f)", fDeltaTime );

	if( GAMESTATE->m_pCurSong==NULL )
		return;

	if( GAMESTATE->m_bAttackBeganThisUpdate[m_PlayerNumber] )
		m_soundAttackLaunch.Play();
	if( GAMESTATE->m_bAttackEndedThisUpdate[m_PlayerNumber] )
		m_soundAttackEnding.Play();


	const float fSongBeat = GAMESTATE->m_fSongBeat;
	const int iSongRow = BeatToNoteRow( fSongBeat );

	m_pNoteField->Update( fDeltaTime );

	//
	// Update Y positions
	//
	{
		for( int c=0; c<GAMESTATE->GetCurrentStyle()->m_iColsPerPlayer; c++ )
		{
			float fPercentReverse = GAMESTATE->m_CurrentPlayerOptions[m_PlayerNumber].GetReversePercentForColumn(c);
			float fHoldJudgeYPos = SCALE( fPercentReverse, 0.f, 1.f, HOLD_JUDGMENT_Y_STANDARD, HOLD_JUDGMENT_Y_REVERSE );
//			float fGrayYPos = SCALE( fPercentReverse, 0.f, 1.f, GRAY_ARROWS_Y_STANDARD, GRAY_ARROWS_Y_REVERSE );

			const float fX = ArrowGetXPos( m_PlayerNumber, c, 0 );
			const float fZ = ArrowGetZPos( m_PlayerNumber, c, 0 );

			m_HoldJudgment[c].SetX( fX );
			m_HoldJudgment[c].SetY( fHoldJudgeYPos );
			m_HoldJudgment[c].SetZ( fZ );
		}
	}

	float fPercentReverse = GAMESTATE->m_CurrentPlayerOptions[m_PlayerNumber].GetReversePercentForColumn(0);
	float fGrayYPos = SCALE( fPercentReverse, 0.f, 1.f, GRAY_ARROWS_Y_STANDARD, GRAY_ARROWS_Y_REVERSE );
	m_ArrowBackdrop.SetY( fGrayYPos );

	// NoteField accounts for reverse on its own now.
//	m_pNoteField->SetY( fGrayYPos );

	float fMiniPercent = GAMESTATE->m_CurrentPlayerOptions[m_PlayerNumber].m_fEffects[PlayerOptions::EFFECT_MINI];
	float fNoteFieldZoom = 1 - fMiniPercent*0.5f;
	float fJudgmentZoom = 1 - fMiniPercent*0.25f;
	m_pNoteField->SetZoom( fNoteFieldZoom );
	m_Judgment.SetZoom( fJudgmentZoom );

	//
	// Check for TapNote misses
	//
	UpdateTapNotesMissedOlderThan( GetMaxStepDistanceSeconds() );

	//
	// update pressed flag
	//
	const int iNumCols = GAMESTATE->GetCurrentStyle()->m_iColsPerPlayer;
	ASSERT_M( iNumCols < MAX_COLS_PER_PLAYER, ssprintf("%i >= %i", iNumCols, MAX_COLS_PER_PLAYER) );
	for( int col=0; col < iNumCols; ++col )
	{
		CHECKPOINT_M( ssprintf("%i %i", col, iNumCols) );
		const StyleInput StyleI( m_PlayerNumber, col );
		const GameInput GameI = GAMESTATE->GetCurrentStyle()->StyleInputToGameInput( StyleI );
		bool bIsHoldingButton = INPUTMAPPER->IsButtonDown( GameI );
		// TODO: Make this work for non-human-controlled players
		if( bIsHoldingButton && !GAMESTATE->m_bDemonstrationOrJukebox && GAMESTATE->m_PlayerController[m_PlayerNumber]==PC_HUMAN )
			m_pNoteField->SetPressed( col );
	}
	
	//
	// update HoldNotes logic
	//
	for( int i=0; i < GetNumHoldNotes(); i++ )		// for each HoldNote
	{
		const HoldNote &hn = GetHoldNote(i);
		HoldNoteScore hns = GetHoldNoteScore(hn);

		m_pNoteField->m_HeldHoldNotes[hn] = false;	// set hold flag so NoteField can do intelligent drawing
		m_pNoteField->m_ActiveHoldNotes[hn] = false;	// set hold flag so NoteField can do intelligent drawing


		if( hns != HNS_NONE )	// if this HoldNote already has a result
			continue;	// we don't need to update the logic for this one
		if( iSongRow < hn.iStartRow )
			continue;	// hold hasn't happened yet

		const StyleInput StyleI( m_PlayerNumber, hn.iTrack );
		const GameInput GameI = GAMESTATE->GetCurrentStyle()->StyleInputToGameInput( StyleI );

		// if they got a bad score or haven't stepped on the corresponding tap yet
		const TapNoteScore tns = GetTapNoteScore( hn.iTrack, hn.iStartRow );
		const bool bSteppedOnTapNote = tns != TNS_NONE  &&  tns != TNS_MISS;	// did they step on the start of this hold?

		float fLife = GetHoldNoteLife(hn);

		// If the song beat is in the range of this hold:
		if( hn.iStartRow <= iSongRow && iSongRow <= hn.iEndRow )
		{
			bool bIsHoldingButton = INPUTMAPPER->IsButtonDown( GameI );
			
			// TODO: Make the CPU miss sometimes.
			if( GAMESTATE->m_PlayerController[m_PlayerNumber] != PC_HUMAN )
				bIsHoldingButton = true;

			// set hold flag so NoteField can do intelligent drawing
			m_pNoteField->m_HeldHoldNotes[hn] = bIsHoldingButton && bSteppedOnTapNote;
			m_pNoteField->m_ActiveHoldNotes[hn] = bSteppedOnTapNote;

			if( bSteppedOnTapNote )
			{
				/* This hold note is not judged and we stepped on its head.  Update
				 * iLastHeldRow. */
				HoldNoteResult *hnr = m_pNoteField->CreateHoldNoteResult( hn );
				hnr->iLastHeldRow = min( iSongRow, hn.iEndRow );

				hnr = this->CreateHoldNoteResult( hn );
				hnr->iLastHeldRow = min( iSongRow, hn.iEndRow );
			}


			if( bSteppedOnTapNote && bIsHoldingButton )
			{
				// Increase life
				fLife = 1;

				m_pNoteField->DidHoldNote( hn.iTrack );		// update the "electric ghost" effect
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
					fLife -= fDeltaTime/ADJUSTED_WINDOW(OK);
					fLife = max( fLife, 0 );	// clamp
//				}
			}
		}

		/* check for NG.  If the head was missed completely, don't count
		 * an NG. */
		if( bSteppedOnTapNote && fLife == 0 )	// the player has not pressed the button for a long time!
			hns = HNS_NG;

		// check for OK
		if( iSongRow >= hn.iEndRow && bSteppedOnTapNote && fLife > 0 )	// if this HoldNote is in the past
		{
			fLife = 1;
			hns = HNS_OK;
			m_pNoteField->DidTapNote( StyleI.col, TNS_PERFECT, true );	// bright ghost flash
		}

		if( hns != HNS_NONE )
		{
			/* this note has been judged */
			HandleHoldScore( hns, tns );
			m_HoldJudgment[hn.iTrack].SetHoldJudgment( hns );

			int ms_error = (hns == HNS_OK)? 0:MAX_PRO_TIMING_ERROR;

			g_CurStageStats.iTotalError[m_PlayerNumber] += ms_error;
			if( hns == HNS_NG ) /* don't show a 0 for an OK */
				m_ProTimingDisplay.SetJudgment( ms_error, TNS_MISS );
		}

		m_pNoteField->SetHoldNoteLife(hn, fLife);	// update the NoteField display
		m_pNoteField->SetHoldNoteScore(hn, hns);	// update the NoteField display

		SetHoldNoteLife(hn, fLife);
		SetHoldNoteScore(hn, hns);
	}

	{
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
	}

	{
		// TRICKY: 
		float fPositionSeconds = GAMESTATE->m_fMusicSeconds;
		fPositionSeconds -= PREFSMAN->m_fPadStickSeconds;
		const float fSongBeat = GAMESTATE->m_pCurSong ? GAMESTATE->m_pCurSong->GetBeatFromElapsedTime( fPositionSeconds ) : 0;
		const int iRowNow = BeatToNoteRowNotRounded( fSongBeat );
		if( iRowNow >= 0 )
		{
			for( ; m_iMineRowLastCrossed <= iRowNow; m_iMineRowLastCrossed++ )  // for each index we crossed since the last update
				if( GAMESTATE->IsPlayerEnabled(m_PlayerNumber) )
					CrossedMineRow( m_iMineRowLastCrossed );
		}
	}


	// process transforms that are waiting to be applied
	ApplyWaitingTransforms();

	/* Cache any newly-used note skins.  Normally, the only new skins cached now are
	 * when we're adding course modifiers at the start of a song.  If this is spending
	 * time loading skins in the middle of a song, something is wrong. */
	m_pNoteField->CacheAllUsedNoteSkins();

	ActorFrame::Update( fDeltaTime );
}

void PlayerMinus::ApplyWaitingTransforms()
{
	for( unsigned j=0; j<GAMESTATE->m_ModsToApply[m_PlayerNumber].size(); j++ )
	{
		const Attack &mod = GAMESTATE->m_ModsToApply[m_PlayerNumber][j];
		PlayerOptions po;
		/* Should this default to "" always? need it blank so we know if mod.sModifier
		 * changes the note skin. */
		po.m_sNoteSkin = "";
		po.FromString( mod.sModifier );

		float fStartBeat, fEndBeat;
		mod.GetAttackBeats( GAMESTATE->m_pCurSong, m_PlayerNumber, fStartBeat, fEndBeat );
		fEndBeat = min( fEndBeat, GetNumBeats() );

		LOG->Trace( "Applying transform '%s' from %f to %f to '%s'", mod.sModifier.c_str(), fStartBeat, fEndBeat,
			GAMESTATE->m_pCurSong->GetTranslitMainTitle().c_str() );
		if( po.m_sNoteSkin != "" )
			GAMESTATE->SetNoteSkinForBeatRange( m_PlayerNumber, po.m_sNoteSkin, fStartBeat, fEndBeat );

		NoteDataUtil::TransformNoteData( *this, po, GAMESTATE->GetCurrentStyle()->m_StepsType, fStartBeat, fEndBeat );
		m_pNoteField->CopyRange( this, BeatToNoteRow(fStartBeat), BeatToNoteRow(fEndBeat), BeatToNoteRow(fStartBeat) );
	}
	GAMESTATE->m_ModsToApply[m_PlayerNumber].clear();
}

void PlayerMinus::DrawPrimitives()
{
	// May have both players in doubles (for battle play); only draw primary player.
	if( GAMESTATE->GetCurrentStyle()->m_StyleType == Style::ONE_PLAYER_TWO_CREDITS  &&
		m_PlayerNumber != GAMESTATE->m_MasterPlayerNumber )
		return;


	// Draw these below everything else.
	m_ArrowBackdrop.Draw();
	if( GAMESTATE->m_PlayerOptions[m_PlayerNumber].m_fBlind == 0 )
		m_Combo.Draw();

	m_AttackDisplay.Draw();

	if( TAP_JUDGMENTS_UNDER_FIELD )
		DrawTapJudgments();

	if( HOLD_JUDGMENTS_UNDER_FIELD )
		DrawHoldJudgments();

	float fTilt = GAMESTATE->m_CurrentPlayerOptions[m_PlayerNumber].m_fPerspectiveTilt;
	float fSkew = GAMESTATE->m_CurrentPlayerOptions[m_PlayerNumber].m_fSkew;
	bool bReverse = GAMESTATE->m_CurrentPlayerOptions[m_PlayerNumber].GetReversePercentForColumn(0)>0.5;


	DISPLAY->CameraPushMatrix();
	DISPLAY->PushMatrix();

	float fCenterY = (GRAY_ARROWS_Y_STANDARD+GRAY_ARROWS_Y_REVERSE)/2;
//	float fHeight = GRAY_ARROWS_Y_REVERSE-GRAY_ARROWS_Y_STANDARD;

	DISPLAY->LoadMenuPerspective( 45, SCALE(fSkew,0.f,1.f,this->GetX(),CENTER_X), fCenterY );

	float fOriginalY = 	m_pNoteField->GetY();

	float fTiltDegrees = SCALE(fTilt,-1.f,+1.f,+30,-30) * (bReverse?-1:1);


	float fZoom = SCALE( GAMESTATE->m_CurrentPlayerOptions[m_PlayerNumber].m_fEffects[PlayerOptions::EFFECT_MINI], 0.f, 1.f, 1.f, 0.5f );
	if( fTilt > 0 )
		fZoom *= SCALE( fTilt, 0.f, 1.f, 1.f, 0.9f );
	else
		fZoom *= SCALE( fTilt, 0.f, -1.f, 1.f, 0.9f );

	float fYOffset;
	if( fTilt > 0 )
		fYOffset = SCALE( fTilt, 0.f, 1.f, 0.f, -45.f ) * (bReverse?-1:1);
	else
		fYOffset = SCALE( fTilt, 0.f, -1.f, 0.f, -20.f ) * (bReverse?-1:1);

	m_pNoteField->SetY( fOriginalY + fYOffset );
	m_pNoteField->SetZoom( fZoom );
	m_pNoteField->SetRotationX( fTiltDegrees );
	m_pNoteField->Draw();

	m_pNoteField->SetY( fOriginalY );

	DISPLAY->CameraPopMatrix();
	DISPLAY->PopMatrix();


	if( !TAP_JUDGMENTS_UNDER_FIELD )
		DrawTapJudgments();

	if( !TAP_JUDGMENTS_UNDER_FIELD )
		DrawHoldJudgments();
}

void PlayerMinus::DrawTapJudgments()
{
	if( GAMESTATE->m_PlayerOptions[m_PlayerNumber].m_fBlind > 0 )
		return;

	if( GAMESTATE->m_PlayerOptions[m_PlayerNumber].m_bProTiming )
		m_ProTimingDisplay.Draw();
	else
		m_Judgment.Draw();
}

void PlayerMinus::DrawHoldJudgments()
{
	if( GAMESTATE->m_PlayerOptions[m_PlayerNumber].m_fBlind > 0 )
		return;

	for( int c=0; c<GetNumTracks(); c++ )
	{
		g_NoteFieldMode[m_PlayerNumber].BeginDrawTrack(c);

		m_HoldJudgment[c].Draw();

		g_NoteFieldMode[m_PlayerNumber].EndDrawTrack(c);
	}
}

/* It's OK for this function to search a little more than was requested. */
int PlayerMinus::GetClosestNoteDirectional( int col, float fBeat, float fMaxBeatsDistance, int iDirection  ) const
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
		if( GetTapNote(col, iCurrentIndex).type == TapNote::empty) continue; /* no note here */
		if( GetTapNoteScore(col, iCurrentIndex) != TNS_NONE ) continue;	/* this note has a score already */

		return iCurrentIndex;
	}

	return -1;
}

int PlayerMinus::GetClosestNote( int col, float fBeat, float fMaxBeatsAhead, float fMaxBeatsBehind ) const
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


void PlayerMinus::Step( int col, RageTimer tm )
{
	if( GAMESTATE->m_SongOptions.m_LifeType == SongOptions::LIFE_BATTERY  &&  g_CurStageStats.bFailed[m_PlayerNumber] )	// Oni dead
		return;	// do nothing

	//LOG->Trace( "PlayerMinus::HandlePlayerStep()" );

	ASSERT( col >= 0  &&  col <= GetNumTracks() );

	float fPositionSeconds = GAMESTATE->m_fMusicSeconds;
	fPositionSeconds -= tm.Ago();
	const float fSongBeat = GAMESTATE->m_pCurSong ? GAMESTATE->m_pCurSong->GetBeatFromElapsedTime( fPositionSeconds ) : GAMESTATE->m_fSongBeat;

	//
	// Check for step on a TapNote
	//
	int iIndexOverlappingNote = GetClosestNote( col, fSongBeat, 
						   StepSearchDistanceForwards * GAMESTATE->m_fCurBPS * GAMESTATE->m_SongOptions.m_fMusicRate,
						   StepSearchDistanceBackwards * GAMESTATE->m_fCurBPS * GAMESTATE->m_SongOptions.m_fMusicRate );
	
	//LOG->Trace( "iIndexStartLookingAt = %d, iNumElementsToExamine = %d", iIndexStartLookingAt, iNumElementsToExamine );

	// calculate TapNoteScore
	TapNoteScore score = TNS_NONE;

	if( iIndexOverlappingNote != -1 )
	{
		// compute the score for this hit
		const float fStepBeat = NoteRowToBeat( (float)iIndexOverlappingNote );
		const float fStepSeconds = GAMESTATE->m_pCurSong->GetElapsedTimeFromBeat(fStepBeat);

		/* We actually stepped on the note this long ago: */
		const float fTimeSinceStep = tm.Ago();

		/* GAMESTATE->m_fMusicSeconds is the music time as of GAMESTATE->m_LastBeatUpdate. Figure
		 * out what the music time is as of now. */
		const float fCurrentMusicSeconds = GAMESTATE->m_fMusicSeconds + (GAMESTATE->m_LastBeatUpdate.Ago()*GAMESTATE->m_SongOptions.m_fMusicRate);

		/* ... which means it happened at this point in the music: */
		const float fMusicSeconds = fCurrentMusicSeconds - fTimeSinceStep * GAMESTATE->m_SongOptions.m_fMusicRate;

		// The offset from the actual step in seconds:
		const float fNoteOffset = (fStepSeconds - fMusicSeconds) / GAMESTATE->m_SongOptions.m_fMusicRate;	// account for music rate
//		LOG->Trace("step was %.3f ago, music is off by %f: %f vs %f, step was %f off", 
//			fTimeSinceStep, GAMESTATE->m_LastBeatUpdate.Ago()/GAMESTATE->m_SongOptions.m_fMusicRate,
//			fStepSeconds, fMusicSeconds, fNoteOffset );

		const float fSecondsFromPerfect = fabsf( fNoteOffset );


		TapNote tn = GetTapNote(col,iIndexOverlappingNote);

		switch( GAMESTATE->m_PlayerController[m_PlayerNumber] )
		{
		case PC_HUMAN:

			switch( tn.type )
			{
			case TapNote::mine:
				// stepped too close to mine?
				if( fSecondsFromPerfect <= ADJUSTED_WINDOW(Mine) )
				{
					m_soundMine.Play();
					score = TNS_HIT_MINE;

					if( m_pLifeMeter )
						m_pLifeMeter->ChangeLifeMine();
					if( m_pCombinedLifeMeter )
						m_pCombinedLifeMeter->ChangeLifeMine(m_PlayerNumber);
					m_pNoteField->SetTapNote(col, iIndexOverlappingNote, TAP_EMPTY);	// remove from NoteField
					m_pNoteField->DidTapNote( col, score, false );
				}
				break;

			case TapNote::attack:
				score = TNS_NONE;	// don't score this as anything
				if( fSecondsFromPerfect <= ADJUSTED_WINDOW(Attack) )
				{
					m_soundAttackLaunch.Play();
					// put attack in effect
					Attack attack = this->GetAttackAt( col, iIndexOverlappingNote );
					GAMESTATE->LaunchAttack( OPPOSITE_PLAYER[m_PlayerNumber], attack );

					// remove all TapAttacks on this row
					for( int t=0; t<this->GetNumTracks(); t++ )
					{
						TapNote tn = this->GetTapNote(t, iIndexOverlappingNote);
						if( tn.type == TapNote::attack )
						{
							this->SetTapNote(col, iIndexOverlappingNote, TAP_EMPTY);	// remove from NoteField
							m_pNoteField->SetTapNote(col, iIndexOverlappingNote, TAP_EMPTY);	// remove from NoteField
						}
					}
				}
				break;
			default:
				if(		 fSecondsFromPerfect <= ADJUSTED_WINDOW(Marvelous) )	score = TNS_MARVELOUS;
				else if( fSecondsFromPerfect <= ADJUSTED_WINDOW(Perfect) )	score = TNS_PERFECT;
				else if( fSecondsFromPerfect <= ADJUSTED_WINDOW(Great) )		score = TNS_GREAT;
				else if( fSecondsFromPerfect <= ADJUSTED_WINDOW(Good) )		score = TNS_GOOD;
				else if( fSecondsFromPerfect <= ADJUSTED_WINDOW(Boo) )		score = TNS_BOO;
				else	score = TNS_NONE;
				break;
			}
			break;
		
		case PC_CPU:
		case PC_AUTOPLAY:
			switch( GAMESTATE->m_PlayerController[m_PlayerNumber] )
			{
			case PC_CPU:
				score = PlayerAI::GetTapNoteScore( m_PlayerNumber );
				break;
			case PC_AUTOPLAY:
				score = TNS_MARVELOUS;
				break;
			}

			// TRICKY:  We're asking the AI to judge mines.  consider TNS_GOOD and below
			// as "mine was hit" and everything else as "mine was avoided"
			if( tn.type == TapNote::mine )
			{
				// The CPU hits a lot of mines.  Only consider hitting the 
				// first mine for a row.  We know we're the first mine if 
				// there are are no mines to the left of us.
				for( int t=0; t<col; t++ )
				{
					if( GetTapNote(t,iIndexOverlappingNote).type == TapNote::mine )	// there's a mine to the left of us
						return;	// avoid
				}

				// The CPU hits a lot of mines.  Make it less likely to hit 
				// mines that don't have a tap note on the same row.
				bool bTapsOnRow = IsThereATapOrHoldHeadAtRow( iIndexOverlappingNote );
				TapNoteScore get_to_avoid = bTapsOnRow ? TNS_GREAT : TNS_GOOD;

				if( score >= get_to_avoid )
				{
					return;	// avoided
				}
				else
				{
					score = TNS_HIT_MINE;
					m_soundMine.Play();
					if( m_pLifeMeter )
						m_pLifeMeter->ChangeLifeMine();
					if( m_pCombinedLifeMeter )
						m_pCombinedLifeMeter->ChangeLifeMine(m_PlayerNumber);
					m_pNoteField->SetTapNote(col, iIndexOverlappingNote, TAP_EMPTY);	// remove from NoteField
					m_pNoteField->DidTapNote( col, score, false );
				}
			}

			/* AI will generate misses here.  Don't handle a miss like a regular note because
			 * we want the judgment animation to appear delayed.  Instead, return early if
			 * AI generated a miss, and let UpdateMissedTapNotesOlderThan() detect and handle the 
			 * misses. */
			if( score == TNS_MISS )
				return;


			if( tn.type == TapNote::attack  &&  score > TNS_GOOD )
			{
				m_soundAttackLaunch.Play();
				score = TNS_NONE;	// don't score this as anything
				
				// put attack in effect
				Attack attack = this->GetAttackAt( col, iIndexOverlappingNote );
				GAMESTATE->LaunchAttack( OPPOSITE_PLAYER[m_PlayerNumber], attack );
				
				// remove all TapAttacks on this row
				for( int t=0; t<this->GetNumTracks(); t++ )
				{
					TapNote tn = this->GetTapNote(t, iIndexOverlappingNote);
					if( tn.type == TapNote::attack )
					{
						this->SetTapNote(col, iIndexOverlappingNote, TAP_EMPTY);	// remove from NoteField
						m_pNoteField->SetTapNote(col, iIndexOverlappingNote, TAP_EMPTY);	// remove from NoteField
					}
				}
			}

			break;

		default:
			ASSERT(0);
			score = TNS_NONE;
			break;
		}
		

		// Do game-specific score mapping.
		const Game* pGame = GAMESTATE->GetCurrentGame();
		if( score == TNS_MARVELOUS )	score = pGame->m_mapMarvelousTo;
		if( score == TNS_PERFECT )		score = pGame->m_mapPerfectTo;
		if( score == TNS_GREAT )		score = pGame->m_mapGreatTo;
		if( score == TNS_GOOD )			score = pGame->m_mapGoodTo;
		if( score == TNS_BOO )			score = pGame->m_mapBooTo;



		if( score != TNS_NONE && score != TNS_MISS )
		{
			int ms_error = (int) roundf( fSecondsFromPerfect * 1000 );
			ms_error = min( ms_error, MAX_PRO_TIMING_ERROR );

			g_CurStageStats.iTotalError[m_PlayerNumber] += ms_error;
			if (!GAMESTATE->m_PlayerOptions[m_PlayerNumber].m_fBlind)
				m_ProTimingDisplay.SetJudgment( ms_error, score );
		}

		if( score==TNS_MARVELOUS  &&  !GAMESTATE->ShowMarvelous())
			score = TNS_PERFECT;

		LOG->Trace("XXX: %i col %i, at %f, music at %f, step was at %f, off by %f",
			score, col, fStepSeconds, fCurrentMusicSeconds, fMusicSeconds, fNoteOffset );
//		LOG->Trace("Note offset: %f (fSecondsFromPerfect = %f), Score: %i", fNoteOffset, fSecondsFromPerfect, score);
		
		SetTapNoteScore(col, iIndexOverlappingNote, score);

		if( score != TNS_NONE )
			SetTapNoteOffset(col, iIndexOverlappingNote, -fNoteOffset);

		if( GAMESTATE->m_PlayerController[m_PlayerNumber] == PC_HUMAN  && 
			score >= TNS_GREAT ) 
			HandleAutosync(fNoteOffset);

		//Keep this here so we get the same data as Autosync
		NSMAN->ReportTiming(fNoteOffset,m_PlayerNumber);
		
		if( m_pPrimaryScoreKeeper )
			m_pPrimaryScoreKeeper->HandleTapScore( score );
		if( m_pSecondaryScoreKeeper )
			m_pSecondaryScoreKeeper->HandleTapScore( score );

		switch( tn.type )
		{
		case TapNote::tap:
		case TapNote::hold_head:
			// don't the row if this note is a mine or tap attack
			if( IsRowCompletelyJudged(iIndexOverlappingNote) )
				OnRowCompletelyJudged( iIndexOverlappingNote );
		}

		if( score == TNS_MISS || score == TNS_BOO )
		{
			m_iDCState = AS2D_MISS;
		}
		if( score == TNS_GOOD || score == TNS_GREAT )
		{
			m_iDCState = AS2D_GOOD;
		}
		if( score == TNS_PERFECT || score == TNS_MARVELOUS )
		{
			m_iDCState = AS2D_GREAT;
			if( m_pLifeMeter && m_pLifeMeter->GetLife() == 1.0f) // full life
			{
				m_iDCState = AS2D_FEVER; // super celebrate time :)
			}
		}
		m_pNoteField->Step( col, score );
	}
	else
		m_pNoteField->Step( col, TNS_NONE );
}

void PlayerMinus::HandleAutosync(float fNoteOffset)
{
	if( !GAMESTATE->m_SongOptions.m_bAutoSync )
		return;

	m_fOffset[m_iOffsetSample++] = fNoteOffset;
	if (m_iOffsetSample < SAMPLE_COUNT) 
		return; /* need more */

	const float mean = calc_mean(m_fOffset, m_fOffset+SAMPLE_COUNT);
	const float stddev = calc_stddev(m_fOffset, m_fOffset+SAMPLE_COUNT);

	if (stddev < .03 && stddev < fabsf(mean)) { //If they stepped with less than .03 error
		GAMESTATE->m_pCurSong->m_Timing.m_fBeat0OffsetInSeconds += mean;
		LOG->Trace("Offset corrected by %f. Error in steps: %f seconds.", mean, stddev);
	} else
		LOG->Trace("Offset NOT corrected. Average offset: %f seconds. Error: %f seconds.", mean, stddev);

	m_iOffsetSample = 0;
}


void PlayerMinus::OnRowCompletelyJudged( int iIndexThatWasSteppedOn )
{
//	LOG->Trace( "PlayerMinus::OnRowCompletelyJudged" );
	
	/* Find the minimum score of the row.  This will never be TNS_NONE, since this
	 * function is only called when a row is completed. */
	/* Instead, use the last tap score (ala DDR).  Using the minimum results in 
	 * slightly more harsh scoring than DDR */
	/* I'm not sure this is right, either.  Can you really jump a boo and a perfect
	 * and get scored for a perfect?  (That's so loose, you can gallop jumps.) -glenn */
	/* Instead of grading individual columns, DDR sets a "was pressed recently" 
	 * countdown every time you step on a column.  When you step on the first note of 
	 * the jump, it sets the first "was pressed recently" timer.  Then, when you do 
	 * the 2nd step of the jump, it sets another column's timer then AND's the jump 
	 * columns with the "was pressed recently" columns to see whether or not you hit 
	 * all the columns of the jump.  -Chris */
//	TapNoteScore score = MinTapNoteScore(iIndexThatWasSteppedOn);
	TapNoteScore score = LastTapNoteScore(iIndexThatWasSteppedOn);
	ASSERT(score != TNS_NONE);
	ASSERT(score != TNS_HIT_MINE);

	/* If the whole row was hit with perfects or greats, remove the row
	 * from the NoteField, so it disappears. */

	for( int c=0; c<GetNumTracks(); c++ )	// for each column
	{
		TapNote tn = GetTapNote(c, iIndexThatWasSteppedOn);

		if( tn.type == TapNote::empty )	continue; /* no note in this col */
		if( tn.type == TapNote::mine )	continue; /* don't flash on mines b/c they're supposed to be missed */

		// If the score is great or better, remove the note from the screen to 
		// indicate success.  (Or always if blind is on.)
		if( score >= TNS_GREAT || GAMESTATE->m_PlayerOptions[m_PlayerNumber].m_fBlind )
			m_pNoteField->SetTapNote(c, iIndexThatWasSteppedOn, TAP_EMPTY);

		// show the ghost arrow for this column
		if (GAMESTATE->m_PlayerOptions[m_PlayerNumber].m_fBlind)
			m_pNoteField->DidTapNote( c, TNS_MARVELOUS, false );
		else
		{
			switch( score )
			{
			case TNS_GREAT:
			case TNS_PERFECT:
			case TNS_MARVELOUS:
				{
					bool bBright = g_CurStageStats.iCurCombo[m_PlayerNumber]>(int)BRIGHT_GHOST_COMBO_THRESHOLD;
					m_pNoteField->DidTapNote( c, score, bBright );
				}
				break;
			}
		}
	}
		
	HandleTapRowScore( iIndexThatWasSteppedOn );	// update score

	m_Judgment.SetJudgment( score );
}


void PlayerMinus::UpdateTapNotesMissedOlderThan( float fMissIfOlderThanSeconds )
{
	//LOG->Trace( "Steps::UpdateTapNotesMissedOlderThan(%f)", fMissIfOlderThanThisBeat );
	int iMissIfOlderThanThisIndex;
	{
		const float fEarliestTime = GAMESTATE->m_fMusicSeconds - fMissIfOlderThanSeconds;
		bool bFreeze;
		float fMissIfOlderThanThisBeat;
		float fThrowAway;
		GAMESTATE->m_pCurSong->GetBeatAndBPSFromElapsedTime( fEarliestTime, fMissIfOlderThanThisBeat, fThrowAway, bFreeze );

		iMissIfOlderThanThisIndex = BeatToNoteRow( fMissIfOlderThanThisBeat );
		if( bFreeze )
		{
			/* If there is a freeze on iMissIfOlderThanThisIndex, include this index too.
			 * Otherwise we won't show misses for tap notes on freezes until the
			 * freeze finishes. */
			iMissIfOlderThanThisIndex++;
		}
	}

	// Since this is being called every frame, let's not check the whole array every time.
	// Instead, only check 10 elements back.  Even 10 is overkill.
	const int iStartCheckingAt = max( 0, iMissIfOlderThanThisIndex-10 );

	//LOG->Trace( "iStartCheckingAt: %d   iMissIfOlderThanThisIndex:  %d", iStartCheckingAt, iMissIfOlderThanThisIndex );

	int iNumMissesFound = 0;
	for( int r=iStartCheckingAt; r<iMissIfOlderThanThisIndex; r++ )
	{
		bool MissedNoteOnThisRow = false;
		for( int t=0; t<GetNumTracks(); t++ )
		{
			switch( GetTapNote(t, r).type )
			{
			case TapNote::empty:
			case TapNote::attack:
				continue; /* no note here */
			}

			if( GetTapNoteScore(t, r) != TNS_NONE ) /* note here is already hit */
				continue; 
			
			if( GetTapNote(t, r).type != TapNote::mine )
			{
				// A normal note.  Penalize for not stepping on it.
				MissedNoteOnThisRow = true;
				SetTapNoteScore(t, r, TNS_MISS);
				g_CurStageStats.iTotalError[m_PlayerNumber] += MAX_PRO_TIMING_ERROR;
				m_ProTimingDisplay.SetJudgment( MAX_PRO_TIMING_ERROR, TNS_MISS );
			}
		}

		if( MissedNoteOnThisRow )
		{
			iNumMissesFound++;
			HandleTapRowScore( r );
		}
	}

	if( iNumMissesFound > 0 )
		m_Judgment.SetJudgment( TNS_MISS );
}


void PlayerMinus::CrossedRow( int iNoteRow )
{
	// If we're doing random vanish, randomise notes on the fly.
	if(GAMESTATE->m_CurrentPlayerOptions[m_PlayerNumber].m_fAppearances[PlayerOptions::APPEARANCE_RANDOMVANISH]==1)
		RandomiseNotes( iNoteRow );

	// check to see if there's at the crossed row
	RageTimer now;
	if( GAMESTATE->m_PlayerController[m_PlayerNumber] != PC_HUMAN )
	{
		for( int t=0; t<GetNumTracks(); t++ )
			if( GetTapNote(t, iNoteRow).type != TapNote::empty )
				if( GetTapNoteScore(t, iNoteRow) == TNS_NONE )
					Step( t, now );
	}
}

void PlayerMinus::CrossedMineRow( int iNoteRow )
{
	// Hold the panel while crossing a mine will cause the mine to explode
	RageTimer now;
	for( int t=0; t<GetNumTracks(); t++ )
	{
		if( GetTapNote(t,iNoteRow).type == TapNote::mine )
		{
			const StyleInput StyleI( m_PlayerNumber, t );
			const GameInput GameI = GAMESTATE->GetCurrentStyle()->StyleInputToGameInput( StyleI );
			if( PREFSMAN->m_fPadStickSeconds > 0 )
			{
				float fSecsHeld = INPUTMAPPER->GetSecsHeld( GameI );
				if( fSecsHeld >= PREFSMAN->m_fPadStickSeconds )
					Step( t, now+(-PREFSMAN->m_fPadStickSeconds) );
			}
			else
			{
				bool bIsDown = INPUTMAPPER->IsButtonDown( GameI );
				if( bIsDown )
					Step( t, now );
			}
		}
	}
}

void PlayerMinus::RandomiseNotes( int iNoteRow )
{
	// change the row to look ahead from based upon their speed mod
	/* This is incorrect: if m_fScrollSpeed is 0.5, we'll never change
	 * any odd rows, and if it's 2, we'll shuffle each row twice. */
	int iNewNoteRow = iNoteRow + ROWS_PER_BEAT*2;
	iNewNoteRow = int( iNewNoteRow / GAMESTATE->m_PlayerOptions[m_PlayerNumber].m_fScrollSpeed );

	int iNumOfTracks = GetNumTracks();
	for( int t=0; t+1 < iNumOfTracks; t++ )
	{
		const int iSwapWith = RandomInt( 0, iNumOfTracks-1 );

		/* Only swap a tap and an empty. */
		const TapNote t1 = GetTapNote(t, iNewNoteRow);
		if( t1.type != TapNote::tap )
			continue;

		const TapNote t2 = GetTapNote( iSwapWith, iNewNoteRow );
		if( t2.type != TapNote::empty )
			continue;

		/* Make sure the destination row isn't in the middle of a hold. */
		bool bSkip = false;
		for( int i = 0; !bSkip && i < GetNumHoldNotes(); ++i )
		{
			const HoldNote &hn = GetHoldNote(i);
			if( hn.iTrack == iSwapWith && hn.RowIsInRange(iNewNoteRow) )
				bSkip = true;
		}
		if( bSkip )
			continue;
		
		SetTapNote( t, iNewNoteRow, t2 );
		SetTapNote( iSwapWith, iNewNoteRow, t1 );

		const TapNote nft1 = m_pNoteField->GetTapNote( t, iNewNoteRow );
		const TapNote nft2 = m_pNoteField->GetTapNote( iSwapWith, iNewNoteRow );
		m_pNoteField->SetTapNote( t, iNewNoteRow, nft2 );
		m_pNoteField->SetTapNote( iSwapWith, iNewNoteRow, nft1 );
	}
}

void PlayerMinus::HandleTapRowScore( unsigned row )
{
	TapNoteScore scoreOfLastTap = LastTapNoteScore(row);
	int iNumTapsInRow = this->GetNumTracksWithTapOrHoldHead(row);
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

	/* Update miss combo, and handle "combo stopped" messages. */
	int &iCurCombo = g_CurStageStats.iCurCombo[m_PlayerNumber];
	switch( scoreOfLastTap )
	{
	case TNS_MARVELOUS:
	case TNS_PERFECT:
	case TNS_GREAT:
		g_CurStageStats.iCurMissCombo[m_PlayerNumber] = 0;
		SCREENMAN->PostMessageToTopScreen( SM_MissComboAborted, 0 );
		break;

	case TNS_MISS:
		++g_CurStageStats.iCurMissCombo[m_PlayerNumber];
		m_iDCState = AS2D_MISS; // update dancing 2d characters that may have missed a note
	case TNS_GOOD:
	case TNS_BOO:
		if( iCurCombo > 50 )
			SCREENMAN->PostMessageToTopScreen( SM_ComboStopped, 0 );

		iCurCombo = 0;
		break;
	default:
		ASSERT( 0 );
	}

	/* The score keeper updates the hit combo.  Remember the old combo for handling announcers. */
	const int iOldCombo = g_CurStageStats.iCurCombo[m_PlayerNumber];

	if(m_pPrimaryScoreKeeper)
		m_pPrimaryScoreKeeper->HandleTapRowScore(scoreOfLastTap, iNumTapsInRow );
	if(m_pSecondaryScoreKeeper)
		m_pSecondaryScoreKeeper->HandleTapRowScore(scoreOfLastTap, iNumTapsInRow );

	m_Combo.SetCombo( g_CurStageStats.iCurCombo[m_PlayerNumber], g_CurStageStats.iCurMissCombo[m_PlayerNumber] );

#define CROSSED( x ) (iOldCombo<x && iCurCombo>=x)
	if ( CROSSED(100) )	
		SCREENMAN->PostMessageToTopScreen( SM_100Combo, 0 );
	else if( CROSSED(200) )	
		SCREENMAN->PostMessageToTopScreen( SM_200Combo, 0 );
	else if( CROSSED(300) )	
		SCREENMAN->PostMessageToTopScreen( SM_300Combo, 0 );
	else if( CROSSED(400) )	
		SCREENMAN->PostMessageToTopScreen( SM_400Combo, 0 );
	else if( CROSSED(500) )	
		SCREENMAN->PostMessageToTopScreen( SM_500Combo, 0 );
	else if( CROSSED(600) )	
		SCREENMAN->PostMessageToTopScreen( SM_600Combo, 0 );
	else if( CROSSED(700) )	
		SCREENMAN->PostMessageToTopScreen( SM_700Combo, 0 );
	else if( CROSSED(800) )	
		SCREENMAN->PostMessageToTopScreen( SM_800Combo, 0 );
	else if( CROSSED(900) )	
		SCREENMAN->PostMessageToTopScreen( SM_900Combo, 0 );
	else if( CROSSED(1000))	
		SCREENMAN->PostMessageToTopScreen( SM_1000Combo, 0 );
	else if( (iOldCombo / 100) < (iCurCombo / 100) && iCurCombo > 1000 )
		SCREENMAN->PostMessageToTopScreen( SM_ComboContinuing, 0 );
#undef CROSSED

	// new max combo
	g_CurStageStats.iMaxCombo[m_PlayerNumber] = max(g_CurStageStats.iMaxCombo[m_PlayerNumber], iCurCombo);

	/* Use the real current beat, not the beat we've been passed.  That's because we
	 * want to record the current life/combo to the current time; eg. if it's a MISS,
	 * the beat we're registering is in the past, but the life is changing now. */
	g_CurStageStats.UpdateComboList( m_PlayerNumber, g_CurStageStats.fAliveSeconds[m_PlayerNumber], false );

	float life = -1;
	if( m_pLifeMeter )
		life = m_pLifeMeter->GetLife();
	else if( m_pCombinedLifeMeter )
	{
		life = GAMESTATE->m_fTugLifePercentP1;
		if( m_PlayerNumber == PLAYER_2 )
			life = 1.0f - life;
	}
	if( life != -1 )
		g_CurStageStats.SetLifeRecordAt( m_PlayerNumber, life, g_CurStageStats.fAliveSeconds[m_PlayerNumber] );

	if (m_pScoreDisplay)
		m_pScoreDisplay->SetScore(g_CurStageStats.iScore[m_PlayerNumber]);
	if (m_pSecondaryScoreDisplay)
		m_pSecondaryScoreDisplay->SetScore(g_CurStageStats.iScore[m_PlayerNumber]);

	if( m_pLifeMeter ) {
		m_pLifeMeter->ChangeLife( scoreOfLastTap );
		m_pLifeMeter->OnDancePointsChange();    // update oni life meter
	}
	if( m_pCombinedLifeMeter ) {
		m_pCombinedLifeMeter->ChangeLife( m_PlayerNumber, scoreOfLastTap );
		m_pCombinedLifeMeter->OnDancePointsChange( m_PlayerNumber );    // update oni life meter
	}
}


void PlayerMinus::HandleHoldScore( HoldNoteScore holdScore, TapNoteScore tapScore )
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

	if (m_pScoreDisplay)
		m_pScoreDisplay->SetScore(g_CurStageStats.iScore[m_PlayerNumber]);
	if (m_pSecondaryScoreDisplay)
		m_pSecondaryScoreDisplay->SetScore(g_CurStageStats.iScore[m_PlayerNumber]);

	if( m_pLifeMeter ) {
		m_pLifeMeter->ChangeLife( holdScore, tapScore );
		m_pLifeMeter->OnDancePointsChange();
	}
	if( m_pCombinedLifeMeter ) {
		m_pCombinedLifeMeter->ChangeLife( m_PlayerNumber, holdScore, tapScore );
		m_pCombinedLifeMeter->OnDancePointsChange( m_PlayerNumber );
	}
}

float PlayerMinus::GetMaxStepDistanceSeconds()
{
	return GAMESTATE->m_SongOptions.m_fMusicRate * ADJUSTED_WINDOW(Boo);
}

void PlayerMinus::FadeToFail()
{
	m_pNoteField->FadeToFail();
}

/* XXX: Why's m_NoteField in a separate class, again?  Is that still needed? */
void Player::Load( PlayerNumber player_no, const NoteData* pNoteData, LifeMeter* pLM, CombinedLifeMeter* pCombinedLM, ScoreDisplay* pScoreDisplay, ScoreDisplay* pSecondaryScoreDisplay, Inventory* pInventory, ScoreKeeper* pPrimaryScoreKeeper, ScoreKeeper* pSecondaryScoreKeeper )
{
	PlayerMinus::Load( player_no, pNoteData, pLM, pCombinedLM, pScoreDisplay, pSecondaryScoreDisplay, pInventory, pPrimaryScoreKeeper, pSecondaryScoreKeeper, &m_NoteField );
}

/*
 * (c) 2001-2004 Chris Danford
 * All rights reserved.
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, and/or sell copies of the Software, and to permit persons to
 * whom the Software is furnished to do so, provided that the above
 * copyright notice(s) and this permission notice appear in all copies of
 * the Software and that both the above copyright notice(s) and this
 * permission notice appear in supporting documentation.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT OF
 * THIRD PARTY RIGHTS. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR HOLDERS
 * INCLUDED IN THIS NOTICE BE LIABLE FOR ANY CLAIM, OR ANY SPECIAL INDIRECT
 * OR CONSEQUENTIAL DAMAGES, OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS
 * OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
 * OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */
