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
#include "ScreenDimensions.h"
#include "RageSoundManager.h"
#include "ThemeMetric.h"
#include "PlayerState.h"
#include "GameplayMessages.h"
#include "GameSoundManager.h"
#include "Style.h"

ThemeMetric<float> GRAY_ARROWS_Y_STANDARD		("Player","ReceptorArrowsYStandard");
ThemeMetric<float> GRAY_ARROWS_Y_REVERSE		("Player","ReceptorArrowsYReverse");
#define JUDGMENT_X( p, both_sides )				THEME->GetMetricF("Player",both_sides ? CString("JudgmentXOffsetBothSides") : ssprintf("JudgmentXOffsetOneSideP%d",p+1))
#define JUDGMENT_Y								THEME->GetMetricF("Player","JudgmentY")
#define JUDGMENT_Y_REVERSE						THEME->GetMetricF("Player","JudgmentYReverse")
#define COMBO_X( p, both_sides )				THEME->GetMetricF("Player",both_sides ? CString("ComboXOffsetBothSides") : ssprintf("ComboXOffsetOneSideP%d",p+1))
#define COMBO_Y									THEME->GetMetricF("Player","ComboY")
#define COMBO_Y_REVERSE							THEME->GetMetricF("Player","ComboYReverse")
#define ATTACK_DISPLAY_X( p, both_sides )		THEME->GetMetricF("Player",both_sides ? CString("AttackDisplayXOffsetBothSides") : ssprintf("AttackDisplayXOffsetOneSideP%d",p+1))
#define ATTACK_DISPLAY_Y						THEME->GetMetricF("Player","AttackDisplayY")
#define ATTACK_DISPLAY_Y_REVERSE				THEME->GetMetricF("Player","AttackDisplayYReverse")
ThemeMetric<float> HOLD_JUDGMENT_Y_STANDARD		("Player","HoldJudgmentYStandard");
ThemeMetric<float> HOLD_JUDGMENT_Y_REVERSE		("Player","HoldJudgmentYReverse");
ThemeMetric<int>	BRIGHT_GHOST_COMBO_THRESHOLD("Player","BrightGhostComboThreshold");
ThemeMetric<bool>	TAP_JUDGMENTS_UNDER_FIELD	("Player","TapJudgmentsUnderField");
ThemeMetric<bool>	HOLD_JUDGMENTS_UNDER_FIELD	("Player","HoldJudgmentsUnderField");
#define START_DRAWING_AT_PIXELS					THEME->GetMetricI("Player","StartDrawingAtPixels")
#define STOP_DRAWING_AT_PIXELS					THEME->GetMetricI("Player","StopDrawingAtPixels")
#define MAX_PRO_TIMING_ERROR					THEME->GetMetricI("Player","MaxProTimingError")

/* Distance to search for a note in Step(), in seconds. */
static const float StepSearchDistance = 1.0f;

#define ADJUSTED_WINDOW( judge ) ((PREFSMAN->m_fJudgeWindowSeconds##judge * PREFSMAN->m_fJudgeWindowScale) + PREFSMAN->m_fJudgeWindowAdd)


Player::Player()
{
	m_pPlayerState = NULL;
	m_pPlayerStageStats = NULL;
	m_fNoteFieldHeight = 0;

	m_pLifeMeter = NULL;
	m_pCombinedLifeMeter = NULL;
	m_pScoreDisplay = NULL;
	m_pSecondaryScoreDisplay = NULL;
	m_pPrimaryScoreKeeper = NULL;
	m_pSecondaryScoreKeeper = NULL;
	m_pInventory = NULL;
	
	m_iOffsetSample = 0;

	this->AddChild( &m_Judgment );
	this->AddChild( &m_ProTimingDisplay );
	this->AddChild( &m_Combo );
	this->AddChild( &m_AttackDisplay );
	for( int c=0; c<MAX_NOTE_TRACKS; c++ )
		this->AddChild( &m_HoldJudgment[c] );


	PlayerAI::InitFromDisk();

	m_pNoteField = new NoteField;
}

Player::~Player()
{
	delete m_pNoteField;
}

/* Init() does the expensive stuff: load sounds and note skins.  Load() just loads a NoteData. */
void Player::Init(
	PlayerState* pPlayerState, 
	PlayerStageStats* pPlayerStageStats,
	LifeMeter* pLM, 
	CombinedLifeMeter* pCombinedLM, 
	ScoreDisplay* pScoreDisplay, 
	ScoreDisplay* pSecondaryScoreDisplay, 
	Inventory* pInventory, 
	ScoreKeeper* pPrimaryScoreKeeper, 
	ScoreKeeper* pSecondaryScoreKeeper )
{
	m_pPlayerState = pPlayerState;
	m_pPlayerStageStats = pPlayerStageStats;
	m_pLifeMeter = pLM;
	m_pCombinedLifeMeter = pCombinedLM;
	m_pScoreDisplay = pScoreDisplay;
	m_pSecondaryScoreDisplay = pSecondaryScoreDisplay;
	m_pInventory = pInventory;
	m_pPrimaryScoreKeeper = pPrimaryScoreKeeper;
	m_pSecondaryScoreKeeper = pSecondaryScoreKeeper;

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

	// TODO: Remove use of PlayerNumber.
	PlayerNumber pn = m_pPlayerState->m_PlayerNumber;

	RageSoundParams p;
	GameSoundManager::SetPlayerBalance( pn, p );

	m_soundMine.SetParams( p );
	m_soundAttackLaunch.SetParams( p );
	m_soundAttackEnding.SetParams( p );
}

void Player::Load( const NoteData& noteData )
{
	m_iDCState = AS2D_IDLE;

	m_iRowLastCrossed = BeatToNoteRowNotRounded( GAMESTATE->m_fSongBeat ) - 1;	// why this?
	m_iMineRowLastCrossed = BeatToNoteRowNotRounded( GAMESTATE->m_fSongBeat ) - 1;	// why this?

	// TODO: Remove use of PlayerNumber.
	PlayerNumber pn = m_pPlayerState->m_PlayerNumber;

	GAMESTATE->ResetNoteSkinsForPlayer( pn );

	// init steps
	m_NoteData.Init();
	bool bOniDead = GAMESTATE->m_SongOptions.m_LifeType == SongOptions::LIFE_BATTERY  &&  
		(m_pPlayerStageStats == NULL || m_pPlayerStageStats->bFailed);
	if( !bOniDead )
		m_NoteData.CopyAll( noteData );

	/* The editor reuses Players ... so we really need to make sure everything
	 * is reset and not tweening.  Perhaps ActorFrame should recurse to subactors;
	 * then we could just this->StopTweening()? -glenn */
	m_Judgment.StopTweening();
//	m_Combo.Reset();				// don't reset combos between songs in a course!
	m_Combo.Init( pn );
	if( m_pPlayerStageStats )
		m_Combo.SetCombo( m_pPlayerStageStats->iCurCombo, m_pPlayerStageStats->iCurMissCombo );	// combo can persist between songs and games
	m_AttackDisplay.Init( m_pPlayerState );
	m_Judgment.Reset();

	/* Don't re-init this; that'll reload graphics.  Add a separate Reset() call
	 * if some ScoreDisplays need it. */
//	if( m_pScore )
//		m_pScore->Init( pn );

	/* Apply transforms. */
	NoteDataUtil::TransformNoteData( m_NoteData, GAMESTATE->m_pPlayerState[pn]->m_PlayerOptions, GAMESTATE->GetCurrentStyle()->m_StepsType );
	
	switch( GAMESTATE->m_PlayMode )
	{
	case PLAY_MODE_RAVE:
	case PLAY_MODE_BATTLE:
		{
			// ugly, ugly, ugly.  Works only w/ dance.
			NoteDataUtil::TransformNoteData( m_NoteData, GAMESTATE->m_pPlayerState[pn]->m_PlayerOptions, GAMESTATE->GetCurrentStyle()->m_StepsType );
			
			// shuffle either p1 or p2
			static int count = 0;
			switch( count )
			{
			case 0:
			case 3:
				NoteDataUtil::Turn( m_NoteData, STEPS_TYPE_DANCE_SINGLE, NoteDataUtil::left);
				break;
			case 1:
			case 2:
				NoteDataUtil::Turn( m_NoteData, STEPS_TYPE_DANCE_SINGLE, NoteDataUtil::right);
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

	float fNoteFieldMiddle = (GRAY_ARROWS_Y_STANDARD+GRAY_ARROWS_Y_REVERSE)/2;
	
	if( m_pNoteField )
		m_pNoteField->SetY( fNoteFieldMiddle );
	m_fNoteFieldHeight = GRAY_ARROWS_Y_REVERSE-GRAY_ARROWS_Y_STANDARD;
	if( m_pNoteField )
		m_pNoteField->Load( &m_NoteData, m_pPlayerState, iStartDrawingAtPixels, iStopDrawingAtPixels, m_fNoteFieldHeight );

	const bool bReverse = GAMESTATE->m_pPlayerState[pn]->m_PlayerOptions.GetReversePercentForColumn(0) == 1;
	bool bPlayerUsingBothSides = GAMESTATE->GetCurrentStyle()->m_StyleType==ONE_PLAYER_TWO_SIDES;
	m_Combo.SetX( COMBO_X(pn,bPlayerUsingBothSides) );
	m_Combo.SetY( bReverse ? COMBO_Y_REVERSE : COMBO_Y );
	m_AttackDisplay.SetX( ATTACK_DISPLAY_X(pn,bPlayerUsingBothSides) - 40 );
	m_AttackDisplay.SetY( bReverse ? ATTACK_DISPLAY_Y_REVERSE : ATTACK_DISPLAY_Y );
	m_Judgment.SetX( JUDGMENT_X(pn,bPlayerUsingBothSides) );
	m_Judgment.SetY( bReverse ? JUDGMENT_Y_REVERSE : JUDGMENT_Y );
	m_ProTimingDisplay.SetX( JUDGMENT_X(pn,bPlayerUsingBothSides) );
	m_ProTimingDisplay.SetY( bReverse ? SCREEN_BOTTOM-JUDGMENT_Y : SCREEN_TOP+JUDGMENT_Y );

	// Need to set Y positions of all these elements in Update since
	// they change depending on PlayerOptions.

	//
	// Load keysounds.  If sounds are already loaded (as in the editor), don't reload them.
	// XXX: the editor will load several duplicate copies (in each NoteField), and each
	// player will load duplicate sounds.  Does this belong somewhere else (perhaps in
	// a separate object, used alongside ScreenGameplay::m_pSoundMusic and ScreenEdit::m_pSoundMusic?)
	// We don't have to load separate copies to set player fade: always make a copy, and set the
	// fade on the copy.
	//
	const Song* pSong = GAMESTATE->m_pCurSong;
	CString sSongDir = pSong->GetSongDir();
	m_vKeysounds.resize( pSong->m_vsKeysoundFile.size() );

	RageSoundParams p;
	GameSoundManager::SetPlayerBalance( pn, p );
	for( unsigned i=0; i<m_vKeysounds.size(); i++ )
	{
		 CString sKeysoundFilePath = sSongDir + pSong->m_vsKeysoundFile[i];
		 RageSound& sound = m_vKeysounds[i];
		 if( sound.GetLoadedFilePath() != sKeysoundFilePath )
			sound.Load( sKeysoundFilePath, true );
		 sound.SetParams( p );
	}
}

void Player::Update( float fDeltaTime )
{
	//LOG->Trace( "Player::Update(%f)", fDeltaTime );

	if( GAMESTATE->m_pCurSong==NULL )
		return;

	if( m_pPlayerState->m_bAttackBeganThisUpdate )
		m_soundAttackLaunch.Play();
	if( m_pPlayerState->m_bAttackEndedThisUpdate )
		m_soundAttackEnding.Play();


	const float fSongBeat = GAMESTATE->m_fSongBeat;
	const int iSongRow = BeatToNoteRow( fSongBeat );

	if( m_pNoteField )
		m_pNoteField->Update( fDeltaTime );

	//
	// Update Y positions
	//
	{
		for( int c=0; c<GAMESTATE->GetCurrentStyle()->m_iColsPerPlayer; c++ )
		{
			float fPercentReverse = m_pPlayerState->m_CurrentPlayerOptions.GetReversePercentForColumn(c);
			float fHoldJudgeYPos = SCALE( fPercentReverse, 0.f, 1.f, HOLD_JUDGMENT_Y_STANDARD, HOLD_JUDGMENT_Y_REVERSE );
//			float fGrayYPos = SCALE( fPercentReverse, 0.f, 1.f, GRAY_ARROWS_Y_STANDARD, GRAY_ARROWS_Y_REVERSE );

			const float fX = ArrowEffects::GetXPos( m_pPlayerState, c, 0 );
			const float fZ = ArrowEffects::GetZPos( m_pPlayerState, c, 0 );

			m_HoldJudgment[c].SetX( fX );
			m_HoldJudgment[c].SetY( fHoldJudgeYPos );
			m_HoldJudgment[c].SetZ( fZ );
		}
	}

	// NoteField accounts for reverse on its own now.
	//if( m_pNoteField )
	//	m_pNoteField->SetY( fGrayYPos );

	float fMiniPercent = m_pPlayerState->m_CurrentPlayerOptions.m_fEffects[PlayerOptions::EFFECT_MINI];
	float fNoteFieldZoom = 1 - fMiniPercent*0.5f;
	float fJudgmentZoom = 1 - fMiniPercent*0.25f;
	if( m_pNoteField )
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
		// TODO: Remove use of PlayerNumber.
		PlayerNumber pn = m_pPlayerState->m_PlayerNumber;

		CHECKPOINT_M( ssprintf("%i %i", col, iNumCols) );
		const StyleInput StyleI( pn, col );
		const GameInput GameI = GAMESTATE->GetCurrentStyle()->StyleInputToGameInput( StyleI );
		bool bIsHoldingButton = INPUTMAPPER->IsButtonDown( GameI );
		// TODO: Make this work for non-human-controlled players
		if( bIsHoldingButton && !GAMESTATE->m_bDemonstrationOrJukebox && m_pPlayerState->m_PlayerController==PC_HUMAN )
			if( m_pNoteField )
				m_pNoteField->SetPressed( col );
	}
	
	//
	// update HoldNotes logic
	//
	for( int i=0; i < m_NoteData.GetNumHoldNotes(); i++ )		// for each HoldNote
	{
		HoldNote &hn = m_NoteData.GetHoldNote(i);

		// set hold flags so NoteField can do intelligent drawing
		hn.result.bHeld = false;
		hn.result.bActive = false;


		HoldNoteScore hns = hn.result.hns;
		if( hns != HNS_NONE )	// if this HoldNote already has a result
			continue;	// we don't need to update the logic for this one
		if( iSongRow < hn.iStartRow )
			continue;	// hold hasn't happened yet

		// TODO: Remove use of PlayerNumber.
		PlayerNumber pn = m_pPlayerState->m_PlayerNumber;

		const StyleInput StyleI( pn, hn.iTrack );
		const GameInput GameI = GAMESTATE->GetCurrentStyle()->StyleInputToGameInput( StyleI );

		// if they got a bad score or haven't stepped on the corresponding tap yet
		const TapNoteScore tns = m_NoteData.GetTapNote( hn.iTrack, hn.iStartRow ).result.tns;
		const bool bSteppedOnTapNote = tns != TNS_NONE  &&  tns != TNS_MISS;	// did they step on the start of this hold?

		float fLife = hn.result.fLife;

		bool bIsHoldingButton = INPUTMAPPER->IsButtonDown( GameI );
		// TODO: Make the CPU miss sometimes.
		if( m_pPlayerState->m_PlayerController != PC_HUMAN )
			bIsHoldingButton = true;

		if( bSteppedOnTapNote && fLife != 0 )
		{
			/* This hold note is not judged and we stepped on its head.  Update iLastHeldRow.
			 * Do this even if we're a little beyond the end of the hold note, to make sure
			 * iLastHeldRow is clamped to iEndRow if the hold note is held all the way. */
			hn.result.iLastHeldRow = min( iSongRow, hn.iEndRow );
		}

		// If the song beat is in the range of this hold:
		if( hn.RowIsInRange(iSongRow) )
		{
			// set hold flag so NoteField can do intelligent drawing
			hn.result.bHeld = bIsHoldingButton && bSteppedOnTapNote;
			hn.result.bActive = bSteppedOnTapNote;

			if( bSteppedOnTapNote && bIsHoldingButton )
			{
				// Increase life
				fLife = 1;

				if( m_pNoteField )
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
			if( m_pNoteField )
				m_pNoteField->DidTapNote( StyleI.col, TNS_PERFECT, true );	// bright ghost flash
		}

		if( hns != HNS_NONE )
		{
			/* this note has been judged */
			HandleHoldScore( hns, tns );
			m_HoldJudgment[hn.iTrack].SetHoldJudgment( hns );

			int ms_error = (hns == HNS_OK)? 0:MAX_PRO_TIMING_ERROR;

			if( m_pPlayerStageStats )
				m_pPlayerStageStats->iTotalError += ms_error;
			if( hns == HNS_NG ) /* don't show a 0 for an OK */
				m_ProTimingDisplay.SetJudgment( ms_error, TNS_MISS );
		}

		hn.result.fLife = fLife;
		hn.result.hns = hns;
	}

	// TODO: Remove use of PlayerNumber.
	PlayerNumber pn = m_pPlayerState->m_PlayerNumber;

	{
		// Why was this originally "BeatToNoteRowNotRounded"?  It should be rounded.  -Chris
		/* We want to send the crossed row message exactly when we cross the row--not
		 * .5 before the row.  Use a very slow song (around 2 BPM) as a test case: without
		 * rounding, autoplay steps early. -glenn */
		const int iRowNow = BeatToNoteRowNotRounded( GAMESTATE->m_fSongBeat );
		if( iRowNow >= 0 )
		{
			// for each index we crossed since the last update
			if( GAMESTATE->IsPlayerEnabled(pn) )
				FOREACH_NONEMPTY_ROW_ALL_TRACKS_RANGE( m_NoteData, r, m_iRowLastCrossed+1, iRowNow )
					CrossedRow( r );
			m_iRowLastCrossed = iRowNow;
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
			// for each index we crossed since the last update
			if( GAMESTATE->IsPlayerEnabled(pn) )
				FOREACH_NONEMPTY_ROW_ALL_TRACKS_RANGE( m_NoteData, r, m_iMineRowLastCrossed+1, iRowNow )
					CrossedMineRow( r );
			m_iMineRowLastCrossed = iRowNow;
		}
	}


	// process transforms that are waiting to be applied
	ApplyWaitingTransforms();

	ActorFrame::Update( fDeltaTime );
}

void Player::ApplyWaitingTransforms()
{
	for( unsigned j=0; j<m_pPlayerState->m_ModsToApply.size(); j++ )
	{
		const Attack &mod = m_pPlayerState->m_ModsToApply[j];
		PlayerOptions po;
		/* Should this default to "" always? need it blank so we know if mod.sModifier
		 * changes the note skin. */
		po.m_sNoteSkin = "";
		po.FromString( mod.sModifier );

		float fStartBeat, fEndBeat;
		mod.GetAttackBeats( GAMESTATE->m_pCurSong, m_pPlayerState, fStartBeat, fEndBeat );
		fEndBeat = min( fEndBeat, m_NoteData.GetLastBeat() );

		LOG->Trace( "Applying transform '%s' from %f to %f to '%s'", mod.sModifier.c_str(), fStartBeat, fEndBeat,
			GAMESTATE->m_pCurSong->GetTranslitMainTitle().c_str() );
		if( po.m_sNoteSkin != "" )
			GAMESTATE->SetNoteSkinForBeatRange( m_pPlayerState, po.m_sNoteSkin, fStartBeat, fEndBeat );

		NoteDataUtil::TransformNoteData( m_NoteData, po, GAMESTATE->GetCurrentStyle()->m_StepsType, BeatToNoteRow(fStartBeat), BeatToNoteRow(fEndBeat) );
	}
	m_pPlayerState->m_ModsToApply.clear();
}

void Player::DrawPrimitives()
{
	if( m_pNoteField == NULL )
		return;

	// TODO: Remove use of PlayerNumber.
	PlayerNumber pn = m_pPlayerState->m_PlayerNumber;

	// May have both players in doubles (for battle play); only draw primary player.
	if( GAMESTATE->GetCurrentStyle()->m_StyleType == ONE_PLAYER_TWO_SIDES  &&
		pn != GAMESTATE->m_MasterPlayerNumber )
		return;


	// Draw these below everything else.
	if( m_pPlayerState->m_PlayerOptions.m_fBlind == 0 )
		m_Combo.Draw();

	m_AttackDisplay.Draw();

	if( TAP_JUDGMENTS_UNDER_FIELD )
		DrawTapJudgments();

	if( HOLD_JUDGMENTS_UNDER_FIELD )
		DrawHoldJudgments();

	float fTilt = m_pPlayerState->m_CurrentPlayerOptions.m_fPerspectiveTilt;
	float fSkew = m_pPlayerState->m_CurrentPlayerOptions.m_fSkew;
	bool bReverse = m_pPlayerState->m_CurrentPlayerOptions.GetReversePercentForColumn(0)>0.5;


	DISPLAY->CameraPushMatrix();
	DISPLAY->PushMatrix();

	float fCenterY = this->GetY()+(GRAY_ARROWS_Y_STANDARD+GRAY_ARROWS_Y_REVERSE)/2;

	DISPLAY->LoadMenuPerspective( 45, SCALE(fSkew,0.f,1.f,this->GetX(),SCREEN_CENTER_X), fCenterY );

	float fOriginalY = 	m_pNoteField->GetY();

	float fTiltDegrees = SCALE(fTilt,-1.f,+1.f,+30,-30) * (bReverse?-1:1);


	float fZoom = SCALE( m_pPlayerState->m_CurrentPlayerOptions.m_fEffects[PlayerOptions::EFFECT_MINI], 0.f, 1.f, 1.f, 0.5f );
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


	if( !(bool)TAP_JUDGMENTS_UNDER_FIELD )
		DrawTapJudgments();

	if( !(bool)TAP_JUDGMENTS_UNDER_FIELD )
		DrawHoldJudgments();
}

void Player::DrawTapJudgments()
{
	if( m_pPlayerState->m_PlayerOptions.m_fBlind > 0 )
		return;

	if( m_pPlayerState->m_PlayerOptions.m_bProTiming )
		m_ProTimingDisplay.Draw();
	else
		m_Judgment.Draw();
}

void Player::DrawHoldJudgments()
{
	if( m_pPlayerState->m_PlayerOptions.m_fBlind > 0 )
		return;

	// TODO: Remove use of PlayerNumber.
	PlayerNumber pn = m_pPlayerState->m_PlayerNumber;

	for( int c=0; c<m_NoteData.GetNumTracks(); c++ )
	{
		NoteFieldMode::BeginDrawTrack( pn, c );

		m_HoldJudgment[c].Draw();

		NoteFieldMode::EndDrawTrack(c);
	}
}

int Player::GetClosestNoteDirectional( int col, int iStartRow, int iMaxRowsAhead, bool bAllowGraded, bool bForward ) const
{
	/* Be sure to check iStartRow itself, too. */
	int iRow = iStartRow;
	while( abs(iStartRow-iRow) <= iMaxRowsAhead )
	{
		/* Is iRow the row we want? */
		do
		{
			TapNote tn = m_NoteData.GetTapNote(col, iRow);
			if( tn.type == TapNote::empty )
				break;
			if( !bAllowGraded && tn.result.tns != TNS_NONE )
				break;
			return iRow;
		} while(0);

		if( bForward && !m_NoteData.GetNextTapNoteRowForTrack( col, iRow ) )
			return -1;
		if( !bForward && !m_NoteData.GetPrevTapNoteRowForTrack( col, iRow ) )
			return -1;
	}
	return -1;
}

/* Find the closest note to fBeat. */
int Player::GetClosestNote( int col, float fBeat, float fMaxBeatsAhead, float fMaxBeatsBehind, bool bAllowGraded ) const
{
	// look for the closest matching step
	const int iRow = BeatToNoteRow( fBeat );

	// Start at iIndexStartLookingAt and search outward.
	int iNextIndex = GetClosestNoteDirectional( col, iRow, BeatToNoteRow(fMaxBeatsAhead), bAllowGraded, true );
	int iPrevIndex = GetClosestNoteDirectional( col, iRow, BeatToNoteRow(fMaxBeatsBehind), bAllowGraded, false );

	if( iNextIndex == -1 && iPrevIndex == -1 )
		return -1;
	if( iNextIndex == -1 )
		return iPrevIndex;
	if( iPrevIndex == -1 )
		return iNextIndex;

	/* Figure out which row is closer. */
	const float DistToFwd = fabsf( fBeat-NoteRowToBeat(iNextIndex) );
	const float DistToBack = fabsf( fBeat-NoteRowToBeat(iPrevIndex) );
	
	if( DistToFwd > DistToBack )
		return iPrevIndex;
	return iNextIndex;
}


void Player::Step( int col, RageTimer tm )
{
	bool bOniDead = 
		GAMESTATE->m_SongOptions.m_LifeType == SongOptions::LIFE_BATTERY  &&  
		m_pPlayerStageStats  && 
		m_pPlayerStageStats->bFailed;
	if( bOniDead )
		return;	// do nothing

	//LOG->Trace( "Player::HandlePlayerStep()" );

	ASSERT_M( col >= 0  &&  col <= m_NoteData.GetNumTracks(), ssprintf("%i, %i", col, m_NoteData.GetNumTracks()) );


	float fPositionSeconds = GAMESTATE->m_fMusicSeconds;
	fPositionSeconds -= tm.Ago();
	const float fSongBeat = GAMESTATE->m_pCurSong ? GAMESTATE->m_pCurSong->GetBeatFromElapsedTime( fPositionSeconds ) : GAMESTATE->m_fSongBeat;

	//
	// Check for step on a TapNote
	//
	int iIndexOverlappingNote = GetClosestNote( col, fSongBeat, 
						   StepSearchDistance * GAMESTATE->m_fCurBPS * GAMESTATE->m_SongOptions.m_fMusicRate,
						   StepSearchDistance * GAMESTATE->m_fCurBPS * GAMESTATE->m_SongOptions.m_fMusicRate,
						   false );
	
	//LOG->Trace( "iIndexStartLookingAt = %d, iNumElementsToExamine = %d", iIndexStartLookingAt, iNumElementsToExamine );

	// calculate TapNoteScore
	TapNoteScore score = TNS_NONE;

	if( iIndexOverlappingNote != -1 )
	{
		// compute the score for this hit
		const float fStepBeat = NoteRowToBeat( iIndexOverlappingNote );
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


		TapNote tn = m_NoteData.GetTapNote( col, iIndexOverlappingNote );

		switch( m_pPlayerState->m_PlayerController )
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

					// TODO: Remove use of PlayerNumber.
					PlayerNumber pn = m_pPlayerState->m_PlayerNumber;

					if( m_pCombinedLifeMeter )
						m_pCombinedLifeMeter->ChangeLifeMine( pn );
					tn.result.bHidden = true;
					m_NoteData.SetTapNote( col, iIndexOverlappingNote, tn );
					if( m_pNoteField )
						m_pNoteField->DidTapNote( col, score, false );
				}
				break;

			case TapNote::attack:
				score = TNS_NONE;	// don't score this as anything
				if( fSecondsFromPerfect <= ADJUSTED_WINDOW(Attack) && !tn.result.bHidden )
				{
					m_soundAttackLaunch.Play();

					// put attack in effect
					Attack attack(
						ATTACK_LEVEL_1,
						-1,	// now
						tn.fAttackDurationSeconds,
						tn.sAttackModifiers,
						true,
						false
						);

					// TODO: Remove use of PlayerNumber
					PlayerNumber pn = m_pPlayerState->m_PlayerNumber;

					GAMESTATE->LaunchAttack( OPPOSITE_PLAYER[pn], attack );

					// remove all TapAttacks on this row
					for( int t=0; t<m_NoteData.GetNumTracks(); t++ )
					{
						TapNote tn = m_NoteData.GetTapNote(t, iIndexOverlappingNote);
						if( tn.type == TapNote::attack )
						{
							tn.result.bHidden = true;
							m_NoteData.SetTapNote( col, iIndexOverlappingNote, tn );
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
			switch( m_pPlayerState->m_PlayerController )
			{
			case PC_CPU:
				score = PlayerAI::GetTapNoteScore( m_pPlayerState );
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
					if( m_NoteData.GetTapNote(t,iIndexOverlappingNote).type == TapNote::mine )	// there's a mine to the left of us
						return;	// avoid
				}

				// The CPU hits a lot of mines.  Make it less likely to hit 
				// mines that don't have a tap note on the same row.
				bool bTapsOnRow = m_NoteData.IsThereATapOrHoldHeadAtRow( iIndexOverlappingNote );
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
					
					// Remove use of PlayerNumber.
					PlayerNumber pn = m_pPlayerState->m_PlayerNumber;

					if( m_pCombinedLifeMeter )
						m_pCombinedLifeMeter->ChangeLifeMine( pn );
					tn.result.bHidden = true;
					m_NoteData.SetTapNote( col, iIndexOverlappingNote, tn );
					if( m_pNoteField )
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
				Attack attack(
					ATTACK_LEVEL_1,
					-1,	// now
					tn.fAttackDurationSeconds,
					tn.sAttackModifiers,
					true,
					false
					);

				// TODO: Remove use of PlayerNumber.
				PlayerNumber pn = m_pPlayerState->m_PlayerNumber;

				GAMESTATE->LaunchAttack( OPPOSITE_PLAYER[pn], attack );
				
				// remove all TapAttacks on this row
				for( int t=0; t<m_NoteData.GetNumTracks(); t++ )
				{
					TapNote tn = m_NoteData.GetTapNote(t, iIndexOverlappingNote);
					if( tn.type == TapNote::attack )
					{
						tn.result.bHidden = true;
						m_NoteData.SetTapNote( col, iIndexOverlappingNote, tn );
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

			if( m_pPlayerStageStats )
				m_pPlayerStageStats->iTotalError += ms_error;
			if (!m_pPlayerState->m_PlayerOptions.m_fBlind)
				m_ProTimingDisplay.SetJudgment( ms_error, score );
		}

		if( score==TNS_MARVELOUS  &&  !GAMESTATE->ShowMarvelous())
			score = TNS_PERFECT;

		LOG->Trace("XXX: %i col %i, at %f, music at %f, step was at %f, off by %f",
			score, col, fStepSeconds, fCurrentMusicSeconds, fMusicSeconds, fNoteOffset );
//		LOG->Trace("Note offset: %f (fSecondsFromPerfect = %f), Score: %i", fNoteOffset, fSecondsFromPerfect, score);
		
		tn.result.tns = score;

		if( score != TNS_NONE )
			tn.result.fTapNoteOffset = -fNoteOffset;

		m_NoteData.SetTapNote( col, iIndexOverlappingNote, tn );


		if( m_pPlayerState->m_PlayerController == PC_HUMAN  && 
			score >= TNS_GREAT ) 
			HandleAutosync(fNoteOffset);

		// TODO: Remove use of PlayerNumber.
		PlayerNumber pn = m_pPlayerState->m_PlayerNumber;

		//Keep this here so we get the same data as Autosync
		NSMAN->ReportTiming(fNoteOffset,pn);
		
		if( m_pPrimaryScoreKeeper )
			m_pPrimaryScoreKeeper->HandleTapScore( score );
		if( m_pSecondaryScoreKeeper )
			m_pSecondaryScoreKeeper->HandleTapScore( score );

		switch( tn.type )
		{
		case TapNote::tap:
		case TapNote::hold_head:
			// don't the row if this note is a mine or tap attack
			if( NoteDataWithScoring::IsRowCompletelyJudged( m_NoteData, iIndexOverlappingNote ) )
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
		if( m_pNoteField )
			m_pNoteField->Step( col, score );
	}
	else
	{
		if( m_pNoteField )
			m_pNoteField->Step( col, TNS_NONE );
	}

	/* Search for keyed sounds separately.  If we can't find a nearby note, search
	 * backwards indefinitely, and ignore grading. */
	iIndexOverlappingNote = GetClosestNote( col, fSongBeat, 
						   999999.f,
						   StepSearchDistance * GAMESTATE->m_fCurBPS * GAMESTATE->m_SongOptions.m_fMusicRate,
						   true );
	if( iIndexOverlappingNote != -1 )
	{
		TapNote tn = m_NoteData.GetTapNote( col, iIndexOverlappingNote );
		if( tn.bKeysound )
			m_vKeysounds[tn.iKeysoundIndex].Play();
	}
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
		GAMESTATE->m_pCurSong->m_Timing.m_fBeat0OffsetInSeconds += mean;
		LOG->Trace("Offset corrected by %f. Error in steps: %f seconds.", mean, stddev);
	} else
		LOG->Trace("Offset NOT corrected. Average offset: %f seconds. Error: %f seconds.", mean, stddev);

	m_iOffsetSample = 0;
}


void Player::OnRowCompletelyJudged( int iIndexThatWasSteppedOn )
{
//	LOG->Trace( "Player::OnRowCompletelyJudged" );
	
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
//	TapNoteScore score = NoteDataWithScoring::MinTapNoteScore( m_NoteData, iIndexThatWasSteppedOn );
	TapNoteScore score = NoteDataWithScoring::LastTapNoteScore( m_NoteData, iIndexThatWasSteppedOn );
	ASSERT(score != TNS_NONE);
	ASSERT(score != TNS_HIT_MINE);

	/* If the whole row was hit with perfects or greats, remove the row
	 * from the NoteField, so it disappears. */

	for( int c=0; c<m_NoteData.GetNumTracks(); c++ )	// for each column
	{
		TapNote tn = m_NoteData.GetTapNote(c, iIndexThatWasSteppedOn);

		if( tn.type == TapNote::empty )	continue; /* no note in this col */
		if( tn.type == TapNote::mine )	continue; /* don't flash on mines b/c they're supposed to be missed */

		// If the score is great or better, remove the note from the screen to 
		// indicate success.  (Or always if blind is on.)
		if( score >= TNS_GREAT || m_pPlayerState->m_PlayerOptions.m_fBlind )
		{
			tn.result.bHidden = true;
			m_NoteData.SetTapNote( c, iIndexThatWasSteppedOn, tn );
		}

		// show the ghost arrow for this column
		if (m_pPlayerState->m_PlayerOptions.m_fBlind)
		{
			if( m_pNoteField )
				m_pNoteField->DidTapNote( c, TNS_MARVELOUS, false );
		}
		else
		{
			bool bBright = m_pPlayerStageStats && m_pPlayerStageStats->iCurCombo>(int)BRIGHT_GHOST_COMBO_THRESHOLD;
			if( m_pNoteField )
				m_pNoteField->DidTapNote( c, score, bBright );
		}
	}
		
	HandleTapRowScore( iIndexThatWasSteppedOn );	// update score

	m_Judgment.SetJudgment( score );
}


void Player::UpdateTapNotesMissedOlderThan( float fMissIfOlderThanSeconds )
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
	// Instead, only check 1 beat back.  Even 10 is overkill.
	const int iStartCheckingAt = max( 0, iMissIfOlderThanThisIndex-BeatToNoteRow(1) );

	//LOG->Trace( "iStartCheckingAt: %d   iMissIfOlderThanThisIndex:  %d", iStartCheckingAt, iMissIfOlderThanThisIndex );

	int iNumMissesFound = 0;

	FOREACH_NONEMPTY_ROW_ALL_TRACKS_RANGE( m_NoteData, r, iStartCheckingAt, iMissIfOlderThanThisIndex-1 )
	{
		bool MissedNoteOnThisRow = false;
		for( int t=0; t<m_NoteData.GetNumTracks(); t++ )
		{
			/* XXX: cleaner to pick the things we do want to apply misses to, instead of
			 * the things we don't? */
			TapNote tn = m_NoteData.GetTapNote(t, r);
			switch( tn.type )
			{
			case TapNote::empty:
			case TapNote::attack:
			case TapNote::mine:
				continue; /* no note here */
			}
			if( tn.result.tns != TNS_NONE ) /* note here is already hit */
				continue; 

			tn.result.tns =	TNS_MISS;

			// A normal note.  Penalize for not stepping on it.
			MissedNoteOnThisRow = true;

			m_NoteData.SetTapNote( t, r, tn );

			if( m_pPlayerStageStats )
			{
				m_pPlayerStageStats->iTotalError += MAX_PRO_TIMING_ERROR;
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


void Player::CrossedRow( int iNoteRow )
{
	// If we're doing random vanish, randomise notes on the fly.
	if(m_pPlayerState->m_CurrentPlayerOptions.m_fAppearances[PlayerOptions::APPEARANCE_RANDOMVANISH]==1)
		RandomizeNotes( iNoteRow );

	// check to see if there's a note at the crossed row
	RageTimer now;
	if( m_pPlayerState->m_PlayerController != PC_HUMAN )
	{
		for( int t=0; t<m_NoteData.GetNumTracks(); t++ )
		{
			TapNote tn = m_NoteData.GetTapNote(t, iNoteRow);
			if( tn.type != TapNote::empty && tn.result.tns == TNS_NONE )
				Step( t, now );
		}
	}
}

void Player::CrossedMineRow( int iNoteRow )
{
	// Hold the panel while crossing a mine will cause the mine to explode
	RageTimer now;
	for( int t=0; t<m_NoteData.GetNumTracks(); t++ )
	{
		if( m_NoteData.GetTapNote(t,iNoteRow).type == TapNote::mine )
		{
			// TODO: Remove use of PlayerNumber.
			PlayerNumber pn = m_pPlayerState->m_PlayerNumber;

			const StyleInput StyleI( pn, t );
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

void Player::RandomizeNotes( int iNoteRow )
{
	// change the row to look ahead from based upon their speed mod
	/* This is incorrect: if m_fScrollSpeed is 0.5, we'll never change
	 * any odd rows, and if it's 2, we'll shuffle each row twice. */
	int iNewNoteRow = iNoteRow + ROWS_PER_BEAT*2;
	iNewNoteRow = int( iNewNoteRow / m_pPlayerState->m_PlayerOptions.m_fScrollSpeed );

	int iNumOfTracks = m_NoteData.GetNumTracks();
	for( int t=0; t+1 < iNumOfTracks; t++ )
	{
		const int iSwapWith = RandomInt( 0, iNumOfTracks-1 );

		/* Only swap a tap and an empty. */
		const TapNote t1 = m_NoteData.GetTapNote(t, iNewNoteRow);
		if( t1.type != TapNote::tap )
			continue;

		const TapNote t2 = m_NoteData.GetTapNote( iSwapWith, iNewNoteRow );
		if( t2.type != TapNote::empty )
			continue;

		/* Make sure the destination row isn't in the middle of a hold. */
		bool bSkip = false;
		for( int i = 0; !bSkip && i < m_NoteData.GetNumHoldNotes(); ++i )
		{
			const HoldNote &hn = m_NoteData.GetHoldNote(i);
			if( hn.iTrack == iSwapWith && hn.RowIsInRange(iNewNoteRow) )
				bSkip = true;
		}
		if( bSkip )
			continue;
		
		m_NoteData.SetTapNote( t, iNewNoteRow, t2 );
		m_NoteData.SetTapNote( iSwapWith, iNewNoteRow, t1 );
	}
}

void Player::HandleTapRowScore( unsigned row )
{
	TapNoteScore scoreOfLastTap = NoteDataWithScoring::LastTapNoteScore( m_NoteData, row );
	int iNumTapsInRow = m_NoteData.GetNumTracksWithTapOrHoldHead(row);
	ASSERT(iNumTapsInRow > 0);

	bool NoCheating = true;
#ifdef DEBUG
	NoCheating = false;
#endif

	if(GAMESTATE->m_bDemonstrationOrJukebox)
		NoCheating = false;
	// don't accumulate points if AutoPlay is on.
	if( NoCheating && m_pPlayerState->m_PlayerController == PC_AUTOPLAY )
		return;

	/* Update miss combo, and handle "combo stopped" messages. */
	/* When is m_pPlayerStageStats NULL?  Would it be cleaner to pass Player a dummy
	 * PlayerStageStats in this case, instead of having to carefully check for NULL
	 * every time we use it? -glenn */
	int iDummy = 0;
	int &iCurCombo = m_pPlayerStageStats ? m_pPlayerStageStats->iCurCombo : iDummy;
	int &iCurMissCombo = m_pPlayerStageStats ? m_pPlayerStageStats->iCurMissCombo : iDummy;
	switch( scoreOfLastTap )
	{
	case TNS_MARVELOUS:
	case TNS_PERFECT:
	case TNS_GREAT:
		iCurMissCombo = 0;
		SCREENMAN->PostMessageToTopScreen( SM_MissComboAborted, 0 );
		break;

	case TNS_MISS:
		++iCurMissCombo;
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
	const int iOldCombo = iCurCombo;

	if( m_pPrimaryScoreKeeper != NULL )
		m_pPrimaryScoreKeeper->HandleTapRowScore( scoreOfLastTap, iNumTapsInRow );
	if( m_pSecondaryScoreKeeper != NULL )
		m_pSecondaryScoreKeeper->HandleTapRowScore( scoreOfLastTap, iNumTapsInRow );

	if( m_pPlayerStageStats )
		m_Combo.SetCombo( m_pPlayerStageStats->iCurCombo, m_pPlayerStageStats->iCurMissCombo );

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
	if( m_pPlayerStageStats )
		m_pPlayerStageStats->iMaxCombo = max(m_pPlayerStageStats->iMaxCombo, iCurCombo);

	/* Use the real current beat, not the beat we've been passed.  That's because we
	 * want to record the current life/combo to the current time; eg. if it's a MISS,
	 * the beat we're registering is in the past, but the life is changing now. */
	if( m_pPlayerStageStats )
		m_pPlayerStageStats->UpdateComboList( m_pPlayerStageStats->fAliveSeconds, false );

	// TODO: Remove use of PlayerNumber.
	PlayerNumber pn = m_pPlayerState->m_PlayerNumber;
	
	float life = -1;
	if( m_pLifeMeter )
	{
		life = m_pLifeMeter->GetLife();
	}
	else if( m_pCombinedLifeMeter )
	{
		life = GAMESTATE->m_fTugLifePercentP1;
		if( pn == PLAYER_2 )
			life = 1.0f - life;
	}
	if( life != -1 )
		if( m_pPlayerStageStats )
			m_pPlayerStageStats->SetLifeRecordAt( life, m_pPlayerStageStats->fAliveSeconds );

	if (m_pScoreDisplay)
		if( m_pPlayerStageStats )
			m_pScoreDisplay->SetScore(m_pPlayerStageStats->iScore);
	if (m_pSecondaryScoreDisplay)
		if( m_pPlayerStageStats )
			m_pSecondaryScoreDisplay->SetScore(m_pPlayerStageStats->iScore);

	if( m_pLifeMeter ) {
		m_pLifeMeter->ChangeLife( scoreOfLastTap );
		m_pLifeMeter->OnDancePointsChange();    // update oni life meter
	}
	if( m_pCombinedLifeMeter ) {
		m_pCombinedLifeMeter->ChangeLife( pn, scoreOfLastTap );
		m_pCombinedLifeMeter->OnDancePointsChange( pn );    // update oni life meter
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
	if( NoCheating && m_pPlayerState->m_PlayerController == PC_AUTOPLAY )
		return;

	if(m_pPrimaryScoreKeeper)
		m_pPrimaryScoreKeeper->HandleHoldScore(holdScore, tapScore );
	if(m_pSecondaryScoreKeeper)
		m_pSecondaryScoreKeeper->HandleHoldScore(holdScore, tapScore );

	// TODO: Remove use of PlayerNumber.
	PlayerNumber pn = m_pPlayerState->m_PlayerNumber;

	if (m_pScoreDisplay)
		if( m_pPlayerStageStats )
			m_pScoreDisplay->SetScore(m_pPlayerStageStats->iScore);
	if (m_pSecondaryScoreDisplay)
		if( m_pPlayerStageStats )
			m_pSecondaryScoreDisplay->SetScore(m_pPlayerStageStats->iScore);

	if( m_pLifeMeter ) 
	{
		m_pLifeMeter->ChangeLife( holdScore, tapScore );
		m_pLifeMeter->OnDancePointsChange();
	}
	if( m_pCombinedLifeMeter ) 
	{
		m_pCombinedLifeMeter->ChangeLife( pn, holdScore, tapScore );
		m_pCombinedLifeMeter->OnDancePointsChange( pn );
	}
}

float Player::GetMaxStepDistanceSeconds()
{
	return GAMESTATE->m_SongOptions.m_fMusicRate * ADJUSTED_WINDOW(Boo);
}

void Player::FadeToFail()
{
	if( m_pNoteField )
		m_pNoteField->FadeToFail();
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
