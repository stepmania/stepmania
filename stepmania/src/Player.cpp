#include "global.h"
#include "Player.h"
#include "GameConstantsAndTypes.h"
#include "RageUtil.h"
#include "RageTimer.h"
#include "PrefsManager.h"
#include "GameManager.h"
#include "InputMapper.h"
#include "SongManager.h"
#include "GameState.h"
#include "ScoreKeeperNormal.h"
#include "RageLog.h"
#include "RageDisplay.h"
#include "ThemeManager.h"
#include "ScoreDisplay.h"
#include "LifeMeter.h"
#include "CombinedLifeMeter.h"
#include "PlayerAI.h"
#include "NoteField.h"
#include "NoteDataUtil.h"
#include "ScreenMessage.h"
#include "ScreenManager.h"
#include "StageStats.h"
#include "ActorUtil.h"
#include "ArrowEffects.h"
#include "Game.h"
#include "NetworkSyncManager.h"	//used for sending timing offset
#include "DancingCharacters.h"
#include "ScreenDimensions.h"
#include "RageSoundManager.h"
#include "ThemeMetric.h"
#include "PlayerState.h"
#include "GameSoundManager.h"
#include "Style.h"
#include "MessageManager.h"
#include "ProfileManager.h"
#include "Profile.h"
#include "StatsManager.h"
#include "song.h"
#include "Steps.h"
#include "GameCommand.h"
#include "LocalizedString.h"
#include "AdjustSync.h"

RString COMBO_X_NAME( size_t p, size_t both_sides )		{ return "ComboXOffset" + (both_sides ? RString("BothSides") : ssprintf("OneSideP%d",int(p+1)) ); }
RString ATTACK_DISPLAY_X_NAME( size_t p, size_t both_sides )	{ return "AttackDisplayXOffset" + (both_sides ? RString("BothSides") : ssprintf("OneSideP%d",int(p+1)) ); }

/* Distance to search for a note in Step(), in seconds. */
static const float StepSearchDistance = 1.0f;
static const float JUMP_WINDOW_SECONDS = 0.25f;

void TimingWindowSecondsInit( size_t /*TimingWindow*/ i, RString &sNameOut, float &defaultValueOut )
{
	sNameOut = "TimingWindowSeconds" + TimingWindowToString( (TimingWindow)i );
	switch( i )
	{
	default:	ASSERT(0);
	case TW_W1:	defaultValueOut = 0.0225f;	break;
	case TW_W2:	defaultValueOut = 0.045f;	break;
	case TW_W3:	defaultValueOut = 0.090f;	break;
	case TW_W4:	defaultValueOut = 0.135f;	break;
	case TW_W5:	defaultValueOut = 0.180f;	break;
	case TW_Mine:	defaultValueOut = 0.090f;	break;	// same as great
	case TW_Hold:	defaultValueOut = 0.250f;	break;	// allow enough time to take foot off and put back on
	case TW_Roll:	defaultValueOut = 0.350f;	break;
	case TW_Attack:	defaultValueOut = 0.135f;	break;
	}
}

static Preference<float> m_fTimingWindowScale	( "TimingWindowScale",		1.0f );
static Preference<float> m_fTimingWindowAdd	( "TimingWindowAdd",		0 );
static Preference1D<float> m_fTimingWindowSeconds( TimingWindowSecondsInit, NUM_TimingWindow );

float Player::GetWindowSeconds( TimingWindow tw )
{
	float fSecs = m_fTimingWindowSeconds[tw];
	fSecs *= m_fTimingWindowScale;
	fSecs += m_fTimingWindowAdd;
	return fSecs;
}

Player::Player( NoteData &nd, bool bShowNoteField, bool bShowJudgment ) : m_NoteData(nd)
{
	m_bLoaded = false;

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
	
	m_bPaused = false;

	m_pJudgment = NULL;
	if( bShowJudgment )
	{
		m_pJudgment = new Judgment;
		this->AddChild( m_pJudgment );
	}

	m_pCombo = NULL;
	if( bShowJudgment )
	{
		m_pCombo = new Combo;
		this->AddChild( m_pCombo );
	}

	m_pAttackDisplay = NULL;
	if( bShowNoteField )
	{
		m_pAttackDisplay = new AttackDisplay;
		this->AddChild( m_pAttackDisplay );
	}

	PlayerAI::InitFromDisk();

	m_pNoteField = NULL;
	if( bShowNoteField )
	{
		m_pNoteField = new NoteField;
	}
}

Player::~Player()
{
	SAFE_DELETE( m_pJudgment );
	SAFE_DELETE( m_pCombo );
	SAFE_DELETE( m_pAttackDisplay );
	SAFE_DELETE( m_pNoteField );
	for( unsigned i = 0; i < m_vHoldJudgment.size(); ++i )
		SAFE_DELETE( m_vHoldJudgment[i] );
}

/* Init() does the expensive stuff: load sounds and note skins.  Load() just loads a NoteData. */
void Player::Init(
	const RString &sType,
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
	GRAY_ARROWS_Y_STANDARD.Load(			sType, "ReceptorArrowsYStandard" );
	GRAY_ARROWS_Y_REVERSE.Load(			sType, "ReceptorArrowsYReverse" );
	COMBO_X.Load(					sType, COMBO_X_NAME, NUM_PLAYERS, 2 );
	COMBO_Y.Load(					sType, "ComboY" );
	COMBO_Y_REVERSE.Load(				sType, "ComboYReverse" );
	COMBO_CENTERED_ADDY.Load(			sType, "ComboCenteredAddY" );
	COMBO_CENTERED_ADDY_REVERSE.Load(		sType, "ComboCenteredAddYReverse" );
	ATTACK_DISPLAY_X.Load(				sType, ATTACK_DISPLAY_X_NAME, NUM_PLAYERS, 2 );
	ATTACK_DISPLAY_Y.Load(				sType, "AttackDisplayY" );
	ATTACK_DISPLAY_Y_REVERSE.Load(			sType, "AttackDisplayYReverse" );
	HOLD_JUDGMENT_Y_STANDARD.Load(			sType, "HoldJudgmentYStandard" );
	HOLD_JUDGMENT_Y_REVERSE.Load(			sType, "HoldJudgmentYReverse" );
	BRIGHT_GHOST_COMBO_THRESHOLD.Load(		sType, "BrightGhostComboThreshold" );
	TAP_JUDGMENTS_UNDER_FIELD.Load(			sType, "TapJudgmentsUnderField" );
	HOLD_JUDGMENTS_UNDER_FIELD.Load(		sType, "HoldJudgmentsUnderField" );
	DRAW_DISTANCE_AFTER_TARGET_PIXELS.Load(		sType, "DrawDistanceAfterTargetsPixels" );
	DRAW_DISTANCE_BEFORE_TARGET_PIXELS.Load(	sType, "DrawDistanceBeforeTargetsPixels" );

	if( m_pJudgment )
	{
		//
		// Init judgment positions
		//
		LuaReference expr = THEME->GetMetricR( sType,"JudgmentTransformCommandFunction" );

		bool bPlayerUsingBothSides = GAMESTATE->GetCurrentStyle()->m_StyleType==ONE_PLAYER_TWO_SIDES;
		Actor temp;

		int iEnabledPlayerIndex = -1;
		int iNumEnabledPlayers = 0;
		if( GAMESTATE->m_bMultiplayer )
		{
			FOREACH_EnabledMultiPlayer( p )
			{
				if( p == pPlayerState->m_mp )
					iEnabledPlayerIndex = iNumEnabledPlayers;
				iNumEnabledPlayers++;
			}
		}
		else
		{
			FOREACH_EnabledPlayer( p )
			{
				if( p == pPlayerState->m_PlayerNumber )
					iEnabledPlayerIndex = iNumEnabledPlayers;
				iNumEnabledPlayers++;
			}
		}

		if( iNumEnabledPlayers == 0 )	// hack for ScreenHowToPlay where no players are joined
		{
			iEnabledPlayerIndex = 0;
			iNumEnabledPlayers = 1;
		}

		for( int i=0; i<NUM_REVERSE; i++ )
		{
			for( int j=0; j<NUM_CENTERED; j++ )
			{
				Lua *L = LUA->Get();
				expr.PushSelf( L );
				ASSERT( !lua_isnil(L, -1) );
				temp.PushSelf( L );
				LuaHelpers::Push( L, pPlayerState->m_PlayerNumber );
				LuaHelpers::Push( L, pPlayerState->m_mp );
				LuaHelpers::Push( L, iEnabledPlayerIndex );
				LuaHelpers::Push( L, iNumEnabledPlayers );
				LuaHelpers::Push( L, bPlayerUsingBothSides );
				LuaHelpers::Push( L, !!i );
				LuaHelpers::Push( L, !!j );
				lua_call( L, 8, 0 ); // 8 args, 0 results
				LUA->Release(L);
				
				m_tsJudgment[i][j] = temp.DestTweenState();
			}
		}

		// Load Judgment frame
		if( GAMESTATE->m_bMultiplayer  &&  !m_sprJudgmentFrame.IsLoaded() )	// only load the first time
		{
			GameCommand gc;
			ASSERT( pPlayerState->m_mp != MultiPlayer_Invalid );
			gc.m_MultiPlayer = pPlayerState->m_mp;
			
			{
				Lua *L = LUA->Get();		
				gc.PushSelf( L );
				lua_setglobal( L, "ThisGameCommand" );
				LUA->Release( L );
			}
			
			m_sprJudgmentFrame.Load( THEME->GetPathG(sType,"JudgmentFrame") );

			{
				Lua *L = LUA->Get();		
				expr.PushSelf( L );
				ASSERT( !lua_isnil(L, -1) );
				m_sprJudgmentFrame->PushSelf( L );
				LuaHelpers::Push( L, pPlayerState->m_PlayerNumber );
				LuaHelpers::Push( L, pPlayerState->m_mp );
				LuaHelpers::Push( L, iEnabledPlayerIndex );
				LuaHelpers::Push( L, iNumEnabledPlayers );
				LuaHelpers::Push( L, bPlayerUsingBothSides );
				LuaHelpers::Push( L, false );
				LuaHelpers::Push( L, false );
				lua_call( L, 8, 0 ); // 8 args, 0 results
				LUA->Release( L );
			}

			LUA->UnsetGlobal( "ThisGameCommand" );
			this->AddChild( m_sprJudgmentFrame );
		}
	}

	this->SortByDrawOrder();

	m_pPlayerState = pPlayerState;
	m_pPlayerStageStats = pPlayerStageStats;
	m_pLifeMeter = pLM;
	m_pCombinedLifeMeter = pCombinedLM;
	m_pScoreDisplay = pScoreDisplay;
	m_pSecondaryScoreDisplay = pSecondaryScoreDisplay;
	m_pInventory = pInventory;
	m_pPrimaryScoreKeeper = pPrimaryScoreKeeper;
	m_pSecondaryScoreKeeper = pSecondaryScoreKeeper;

	
	// set initial life
	if( m_pLifeMeter && m_pPlayerStageStats )
	{
		float fLife = m_pLifeMeter->GetLife();
		m_pPlayerStageStats->SetLifeRecordAt( fLife, STATSMAN->m_CurStageStats.m_fStepsSeconds );
	}


	// TODO: Remove use of PlayerNumber.
	PlayerNumber pn = m_pPlayerState->m_PlayerNumber;

	m_sMessageToSendOnStep = ssprintf("StepP%d",pn+1);


	RageSoundLoadParams SoundParams;
	SoundParams.m_bSupportPan = true;
	m_soundMine.Load( THEME->GetPathS(sType,"mine"), true, &SoundParams );

	/* Attacks can be launched in course modes and in battle modes.  They both come
	 * here to play, but allow loading a different sound for different modes. */
	switch( GAMESTATE->m_PlayMode )
	{
	case PLAY_MODE_RAVE:
	case PLAY_MODE_BATTLE:
		m_soundAttackLaunch.Load( THEME->GetPathS(sType,"battle attack launch"), true, &SoundParams );
		m_soundAttackEnding.Load( THEME->GetPathS(sType,"battle attack ending"), true, &SoundParams );
		break;
	default:
		m_soundAttackLaunch.Load( THEME->GetPathS(sType,"course attack launch"), true, &SoundParams );
		m_soundAttackEnding.Load( THEME->GetPathS(sType,"course attack ending"), true, &SoundParams );
		break;
	}


	float fBalance = GameSoundManager::GetPlayerBalance( pn );
	m_soundMine.SetProperty( "Pan", fBalance );
	m_soundAttackLaunch.SetProperty( "Pan", fBalance );
	m_soundAttackEnding.SetProperty( "Pan", fBalance );

	if( m_pCombo )
	{
		m_pCombo->SetName( "Combo" );
		m_pCombo->Load( m_pPlayerState, m_pPlayerStageStats );
		ActorUtil::OnCommand( m_pCombo, sType );
	}

	if( m_pJudgment )
	{
		m_pJudgment->SetName( "Judgment" );
		m_pJudgment->LoadNormal();
		ActorUtil::OnCommand( m_pJudgment, sType );
	}

	// Load HoldJudgments
	for( int i = 0; i < GAMESTATE->GetCurrentStyle()->m_iColsPerPlayer; ++i )
	{
		HoldJudgment *pJudgment = new HoldJudgment;
		pJudgment->Load( THEME->GetPathG("HoldJudgment","label 1x2") );
		m_vHoldJudgment.push_back( pJudgment );
		this->AddChild( m_vHoldJudgment[i] );
	}

	m_fNoteFieldHeight = GRAY_ARROWS_Y_REVERSE-GRAY_ARROWS_Y_STANDARD;
	if( m_pNoteField )
		m_pNoteField->Init( m_pPlayerState, m_fNoteFieldHeight );

	m_vbFretIsDown.resize( GAMESTATE->GetCurrentStyle()->m_iColsPerPlayer );
	FOREACH( bool, m_vbFretIsDown, b )
		*b = false;
}

void Player::Load()
{
	m_bLoaded = true;

	m_LastTapNoteScore = TNS_None;
	// The editor can start playing in the middle of the song.
	const int iNoteRow = BeatToNoteRowNotRounded( GAMESTATE->m_fSongBeat );
	m_iRowLastCrossed     = iNoteRow - 1;
	m_iMineRowLastCrossed = iNoteRow - 1;
	m_iRowLastJudged      = iNoteRow - 1;
	m_iMineRowLastJudged  = iNoteRow - 1;
	m_JudgedRows.Reset( iNoteRow );

	// TODO: Remove use of PlayerNumber.
	PlayerNumber pn = m_pPlayerState->m_PlayerNumber;

	bool bOniDead = GAMESTATE->m_SongOptions.GetStage().m_LifeType == SongOptions::LIFE_BATTERY  &&  
		(m_pPlayerStageStats == NULL || m_pPlayerStageStats->m_bFailed);

	/* The editor reuses Players ... so we really need to make sure everything
	 * is reset and not tweening.  Perhaps ActorFrame should recurse to subactors;
	 * then we could just this->StopTweening()? -glenn */
	if( m_pJudgment )
		m_pJudgment->StopTweening();
//	m_pCombo->Reset();				// don't reset combos between songs in a course!
	if( m_pPlayerStageStats )
	{
		if( m_pCombo ) 
			m_pCombo->SetCombo( m_pPlayerStageStats->m_iCurCombo, m_pPlayerStageStats->m_iCurMissCombo );	// combo can persist between songs and games
	}
	if( m_pAttackDisplay )
		m_pAttackDisplay->Init( m_pPlayerState );
	if( m_pJudgment )
		m_pJudgment->Reset();

	/* Don't re-init this; that'll reload graphics.  Add a separate Reset() call
	 * if some ScoreDisplays need it. */
//	if( m_pScore )
//		m_pScore->Init( pn );

	/* Apply transforms. */
	NoteDataUtil::TransformNoteData( m_NoteData, m_pPlayerState->m_PlayerOptions.GetStage(), GAMESTATE->GetCurrentStyle()->m_StepsType );
	
	const Song* pSong = GAMESTATE->m_pCurSong;
	if( GAMESTATE->m_pCurGame->m_bAllowHopos )
		NoteDataUtil::SetHopoPossibleFlags( pSong, m_NoteData );
	
	switch( GAMESTATE->m_PlayMode )
	{
	case PLAY_MODE_RAVE:
	case PLAY_MODE_BATTLE:
		{
			// ugly, ugly, ugly.  Works only w/ dance.
			NoteDataUtil::TransformNoteData( m_NoteData, m_pPlayerState->m_PlayerOptions.GetStage(), GAMESTATE->GetCurrentStyle()->m_StepsType );
			
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

	int iDrawDistanceAfterTargetsPixels = GAMESTATE->IsEditing() ? -100 : DRAW_DISTANCE_AFTER_TARGET_PIXELS;
	int iDrawDistanceBeforeTargetsPixels = GAMESTATE->IsEditing() ? 400 : DRAW_DISTANCE_BEFORE_TARGET_PIXELS;

	float fNoteFieldMiddle = (GRAY_ARROWS_Y_STANDARD+GRAY_ARROWS_Y_REVERSE)/2;
	
	if( m_pNoteField && !bOniDead )
	{
		m_pNoteField->SetY( fNoteFieldMiddle );
		m_pNoteField->Load( &m_NoteData, iDrawDistanceAfterTargetsPixels, iDrawDistanceBeforeTargetsPixels );
	}

	const bool bReverse = m_pPlayerState->m_PlayerOptions.GetStage().GetReversePercentForColumn( 0 ) == 1;
	bool bPlayerUsingBothSides = GAMESTATE->GetCurrentStyle()->m_StyleType==ONE_PLAYER_TWO_SIDES;
	if( m_pCombo )
	{
		m_pCombo->SetX( COMBO_X.GetValue(pn, bPlayerUsingBothSides) );
		m_pCombo->SetY( bReverse ? COMBO_Y_REVERSE : COMBO_Y );
	}
	if( m_pAttackDisplay )
		m_pAttackDisplay->SetX( ATTACK_DISPLAY_X.GetValue(pn, bPlayerUsingBothSides) - 40 );
	// set this in Update //m_pAttackDisplay->SetY( bReverse ? ATTACK_DISPLAY_Y_REVERSE : ATTACK_DISPLAY_Y );

	// set this in Update 
	//m_pJudgment->SetX( JUDGMENT_X.GetValue(pn,bPlayerUsingBothSides) );
	//m_pJudgment->SetY( bReverse ? JUDGMENT_Y_REVERSE : JUDGMENT_Y );

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
	RString sSongDir = pSong->GetSongDir();
	m_vKeysounds.resize( pSong->m_vsKeysoundFile.size() );

	RageSoundLoadParams SoundParams;
	SoundParams.m_bSupportPan = true;

	float fBalance = GameSoundManager::GetPlayerBalance( pn );
	for( unsigned i=0; i<m_vKeysounds.size(); i++ )
	{
		RString sKeysoundFilePath = sSongDir + pSong->m_vsKeysoundFile[i];
		RageSound& sound = m_vKeysounds[i];
		if( sound.GetLoadedFilePath() != sKeysoundFilePath )
			sound.Load( sKeysoundFilePath, true, &SoundParams );
		sound.SetProperty( "Pan", fBalance );
	}

	if( m_pPlayerStageStats )
		SendComboMessage( m_pPlayerStageStats->m_iCurCombo, m_pPlayerStageStats->m_iCurMissCombo );
}

void Player::SendComboMessage( int iOldCombo, int iOldMissCombo )
{
	Message msg( "ComboChanged" );
	msg.SetParam( "Player", m_pPlayerState->m_PlayerNumber );
	msg.SetParam( "OldCombo", iOldCombo );
	msg.SetParam( "OldMissCombo", iOldMissCombo );
	msg.SetParam( "PlayerState", LuaReference::CreateFromPush(*m_pPlayerState) );
	msg.SetParam( "PlayerStageStats", LuaReference::CreateFromPush(*m_pPlayerStageStats) );
	MESSAGEMAN->Broadcast( msg );
}

void Player::Update( float fDeltaTime )
{
	const RageTimer now;
	// Don't update if we havn't been loaded yet.
	if( !m_bLoaded )
		return;

	//LOG->Trace( "Player::Update(%f)", fDeltaTime );

	if( GAMESTATE->m_pCurSong==NULL )
		return;

	ActorFrame::Update( fDeltaTime );

	if( m_pPlayerState->m_bAttackBeganThisUpdate )
		m_soundAttackLaunch.Play();
	if( m_pPlayerState->m_bAttackEndedThisUpdate )
		m_soundAttackEnding.Play();


	const float fSongBeat = GAMESTATE->m_fSongBeat;
	const int iSongRow = BeatToNoteRow( fSongBeat );

	if( m_pNoteField )
		m_pNoteField->Update( fDeltaTime );

	float fMiniPercent = m_pPlayerState->m_PlayerOptions.GetCurrent().m_fEffects[PlayerOptions::EFFECT_MINI];
	float fTinyPercent = m_pPlayerState->m_PlayerOptions.GetCurrent().m_fEffects[PlayerOptions::EFFECT_TINY];
	float fJudgmentZoom = min( powf(0.5f, fMiniPercent+fTinyPercent), 1.0f );
	
	//
	// Update Y positions
	//
	{
		for( int c=0; c<GAMESTATE->GetCurrentStyle()->m_iColsPerPlayer; c++ )
		{
			float fPercentReverse = m_pPlayerState->m_PlayerOptions.GetCurrent().GetReversePercentForColumn(c);
			float fHoldJudgeYPos = SCALE( fPercentReverse, 0.f, 1.f, HOLD_JUDGMENT_Y_STANDARD, HOLD_JUDGMENT_Y_REVERSE );
//			float fGrayYPos = SCALE( fPercentReverse, 0.f, 1.f, GRAY_ARROWS_Y_STANDARD, GRAY_ARROWS_Y_REVERSE );

			const float fX = ArrowEffects::GetXPos( m_pPlayerState, c, 0 );
			const float fZ = ArrowEffects::GetZPos( m_pPlayerState, c, 0 );

			m_vHoldJudgment[c]->SetX( fX );
			m_vHoldJudgment[c]->SetY( fHoldJudgeYPos );
			m_vHoldJudgment[c]->SetZ( fZ );
			m_vHoldJudgment[c]->SetZoom( fJudgmentZoom );
		}
	}

	// NoteField accounts for reverse on its own now.
	//if( m_pNoteField )
	//	m_pNoteField->SetY( fGrayYPos );

	const bool bReverse = m_pPlayerState->m_PlayerOptions.GetCurrent().GetReversePercentForColumn(0) == 1;
	float fPercentCentered = m_pPlayerState->m_PlayerOptions.GetCurrent().m_fScrolls[PlayerOptions::SCROLL_CENTERED];
	if( m_pCombo )
	{
		m_pCombo->SetY( 
			bReverse ? 
			COMBO_Y_REVERSE + fPercentCentered * COMBO_CENTERED_ADDY_REVERSE : 
			COMBO_Y + fPercentCentered * COMBO_CENTERED_ADDY );
	}
	if( m_pJudgment )
	{
		const Actor::TweenState &ts1 = m_tsJudgment[bReverse?1:0][0];
		const Actor::TweenState &ts2 = m_tsJudgment[bReverse?1:0][1];
		Actor::TweenState::MakeWeightedAverage( m_pJudgment->DestTweenState(), ts1, ts2, fPercentCentered );
		if( m_sprJudgmentFrame.IsLoaded() )
			Actor::TweenState::MakeWeightedAverage( m_sprJudgmentFrame->DestTweenState(), ts1, ts2, fPercentCentered );
	}


	float fNoteFieldZoom = 1 - fTinyPercent*0.5f;
	if( m_pNoteField )
		m_pNoteField->SetZoom( fNoteFieldZoom );
	if( m_pJudgment )
		m_pJudgment->SetZoom( m_pJudgment->GetZoom() * fJudgmentZoom );
	if( m_sprJudgmentFrame.IsLoaded() )
		m_sprJudgmentFrame->SetZoom( m_sprJudgmentFrame->GetZoom() * fJudgmentZoom );

	// If we're paused, don't update tap or hold note logic, so hold notes can be released
	// during pause.
	if( m_bPaused )
		return;
	
	//
	// Check for TapNote misses
	//
	UpdateTapNotesMissedOlderThan( GetMaxStepDistanceSeconds() );

	//
	// update pressed flag
	//
	const int iNumCols = GAMESTATE->GetCurrentStyle()->m_iColsPerPlayer;
	ASSERT_M( iNumCols <= MAX_COLS_PER_PLAYER, ssprintf("%i > %i", iNumCols, MAX_COLS_PER_PLAYER) );
	for( int col=0; col < iNumCols; ++col )
	{
		ASSERT( m_pPlayerState );

		// TODO: Remove use of PlayerNumber.
		GameInput GameI = GAMESTATE->GetCurrentStyle()->StyleInputToGameInput( col, m_pPlayerState->m_PlayerNumber );

		bool bIsHoldingButton = INPUTMAPPER->IsBeingPressed( GameI );
		// TODO: Make this work for non-human-controlled players
		if( bIsHoldingButton && !GAMESTATE->m_bDemonstrationOrJukebox && m_pPlayerState->m_PlayerController==PC_HUMAN )
			if( m_pNoteField )
				m_pNoteField->SetPressed( col );
	}
	

	//
	// handle Autoplay for rolls
	//
	if( m_pPlayerState->m_PlayerController != PC_HUMAN )
	{
		for( int iTrack=0; iTrack<m_NoteData.GetNumTracks(); ++iTrack )
		{
			// TODO: Make the CPU miss sometimes.
			int iHeadRow;
			if( !m_NoteData.IsHoldNoteAtRow(iTrack, iSongRow, &iHeadRow) )
				continue;

			const TapNote &tn = m_NoteData.GetTapNote( iTrack, iHeadRow );
			if( tn.type != TapNote::hold_head || tn.subType != TapNote::hold_head_roll )
				continue;
			if( tn.HoldResult.hns != HNS_None )
				continue;
			if( tn.HoldResult.fLife >= 0.5f )
				continue;

			Step( iTrack, iSongRow, now, false, false );
			if( m_pPlayerState->m_PlayerController == PC_AUTOPLAY )
				STATSMAN->m_CurStageStats.m_bUsedAutoplay = true;
		}
	}


	//
	// update HoldNotes logic
	//
	for( int iTrack=0; iTrack<m_NoteData.GetNumTracks(); ++iTrack )
	{
		// Since this is being called every frame, let's not check the whole array every time.
		// Instead, only check 1 beat back.  Even 1 is overkill.
		const int iStartCheckingAt = max( 0, iSongRow-BeatToNoteRow(1) );
		NoteData::iterator begin, end;
		m_NoteData.GetTapNoteRangeInclusive( iTrack, iStartCheckingAt, iSongRow+1, begin, end );
		for( ; begin != end; ++begin )
		{
			TapNote &tn = begin->second;
			if( tn.type != TapNote::hold_head )
				continue;
			int iRow = begin->first;

			// set hold flags so NoteField can do intelligent drawing
			tn.HoldResult.bHeld = false;
			tn.HoldResult.bActive = false;

			HoldNoteScore hns = tn.HoldResult.hns;
			if( hns != HNS_None )	// if this HoldNote already has a result
				continue;	// we don't need to update the logic for this one

			// TODO: Remove use of PlayerNumber.
			PlayerNumber pn = m_pPlayerState->m_PlayerNumber;

			// if they got a bad score or haven't stepped on the corresponding tap yet
			const TapNoteScore tns = tn.result.tns;
			const bool bSteppedOnTapNote = tns != TNS_None  &&  tns != TNS_Miss;	// did they step on the start of this hold?

			bool bIsHoldingButton = false;
			if( m_pPlayerState->m_PlayerController != PC_HUMAN )
			{
				// TODO: Make the CPU miss sometimes.
				bIsHoldingButton = true;
				if( m_pPlayerState->m_PlayerController == PC_AUTOPLAY )
					STATSMAN->m_CurStageStats.m_bUsedAutoplay = true;
			}
			else
			{
				GameInput GameI = GAMESTATE->GetCurrentStyle()->StyleInputToGameInput( iTrack, pn );
				bIsHoldingButton = INPUTMAPPER->IsBeingPressed( GameI, m_pPlayerState->m_mp );
			}

			int iEndRow = iRow + tn.iDuration;

			float fLife = tn.HoldResult.fLife;
			if( bSteppedOnTapNote && fLife != 0 )
			{
				/* This hold note is not judged and we stepped on its head.  Update iLastHeldRow.
				 * Do this even if we're a little beyond the end of the hold note, to make sure
				 * iLastHeldRow is clamped to iEndRow if the hold note is held all the way. */
				tn.HoldResult.iLastHeldRow = min( iSongRow, iEndRow );
			}

			// If the song beat is in the range of this hold:
			if( iRow <= iSongRow && iRow <= iEndRow )
			{
				switch( tn.subType )
				{
				case TapNote::hold_head_hold:
					// set hold flag so NoteField can do intelligent drawing
					tn.HoldResult.bHeld = bIsHoldingButton && bSteppedOnTapNote;
					tn.HoldResult.bActive = bSteppedOnTapNote;

					if( bSteppedOnTapNote && bIsHoldingButton )
					{
						// Increase life
						fLife = 1;
					}
					else
					{
						// Decrease life
						fLife -= fDeltaTime/GetWindowSeconds(TW_Hold);
						fLife = max( fLife, 0 );	// clamp
					}
					break;
				case TapNote::hold_head_roll:
					tn.HoldResult.bHeld = true;
					tn.HoldResult.bActive = bSteppedOnTapNote;

					// give positive life in Step(), not here.

					// Decrease life
					fLife -= fDeltaTime/GetWindowSeconds(TW_Roll);
					fLife = max( fLife, 0 );	// clamp
					break;
				default:
					ASSERT(0);
				}
			}

			/* check for LetGo.  If the head was missed completely, don't count an LetGo. */
			if( bSteppedOnTapNote && fLife == 0 )	// the player has not pressed the button for a long time!
				hns = HNS_LetGo;

			// check for OK
			if( iSongRow >= iEndRow && bSteppedOnTapNote && fLife > 0 )	// if this HoldNote is in the past
			{
				fLife = 1;
				hns = HNS_Held;
				bool bBright = m_pPlayerStageStats && m_pPlayerStageStats->m_iCurCombo>(int)BRIGHT_GHOST_COMBO_THRESHOLD;
				if( m_pNoteField )
					m_pNoteField->DidHoldNote( iTrack, HNS_Held, bBright );	// bright ghost flash
			}
			
			tn.HoldResult.fLife = fLife;
			tn.HoldResult.hns = hns;
			
			if( hns != HNS_None )
			{
				/* this note has been judged */
				HandleHoldScore( tn );
				
				if( m_pPlayerStageStats != NULL )
					m_pPlayerStageStats->m_hnsLast = hns;
				if( m_pPlayerState->m_mp != MultiPlayer_Invalid )
					MESSAGEMAN->Broadcast( enum_add2(Message_ShowHoldJudgmentMuliPlayerP1,m_pPlayerState->m_mp) );

				m_vHoldJudgment[iTrack]->SetHoldJudgment( hns );
			}
		}
	}
	
	{
		// Why was this originally "BeatToNoteRowNotRounded"?  It should be rounded.  -Chris
		/* We want to send the crossed row message exactly when we cross the row--not
		 * .5 before the row.  Use a very slow song (around 2 BPM) as a test case: without
		 * rounding, autoplay steps early. -glenn */
		const int iRowNow = BeatToNoteRowNotRounded( GAMESTATE->m_fSongBeat );
		if( iRowNow >= 0 )
		{
			// for each index we crossed since the last update
			if( GAMESTATE->IsPlayerEnabled(m_pPlayerState) )
				FOREACH_NONEMPTY_ROW_ALL_TRACKS_RANGE( m_NoteData, r, m_iRowLastCrossed, iRowNow+1 )
					CrossedRow( r, now );
			m_iRowLastCrossed = iRowNow+1;
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
			if( GAMESTATE->IsPlayerEnabled(m_pPlayerState) )
				FOREACH_NONEMPTY_ROW_ALL_TRACKS_RANGE( m_NoteData, r, m_iMineRowLastCrossed, iRowNow+1 )
					CrossedMineRow( r, now );
			m_iMineRowLastCrossed = iRowNow+1;
		}
	}
	
	// Check for completely judged rows.
	UpdateJudgedRows();

	// process transforms that are waiting to be applied
	ApplyWaitingTransforms();
}

void Player::ProcessMessages( float fDeltaTime )
{
	ActorFrame::ProcessMessages( fDeltaTime );

	if( m_pNoteField )
		m_pNoteField->ProcessMessages( fDeltaTime );
}

void Player::ApplyWaitingTransforms()
{
	for( unsigned j=0; j<m_pPlayerState->m_ModsToApply.size(); j++ )
	{
		const Attack &mod = m_pPlayerState->m_ModsToApply[j];
		PlayerOptions po;
		po.FromString( mod.sModifiers );

		float fStartBeat, fEndBeat;
		mod.GetRealtimeAttackBeats( GAMESTATE->m_pCurSong, m_pPlayerState, fStartBeat, fEndBeat );
		fEndBeat = min( fEndBeat, m_NoteData.GetLastBeat() );

		LOG->Trace( "Applying transform '%s' from %f to %f to '%s'", mod.sModifiers.c_str(), fStartBeat, fEndBeat,
			GAMESTATE->m_pCurSong->GetTranslitMainTitle().c_str() );

		NoteDataUtil::TransformNoteData( m_NoteData, po, GAMESTATE->GetCurrentStyle()->m_StepsType, BeatToNoteRow(fStartBeat), BeatToNoteRow(fEndBeat) );
	}
	m_pPlayerState->m_ModsToApply.clear();
}

void Player::DrawPrimitives()
{
	// TODO: Remove use of PlayerNumber.
	PlayerNumber pn = m_pPlayerState->m_PlayerNumber;

	// May have both players in doubles (for battle play); only draw primary player.
	if( GAMESTATE->GetCurrentStyle()->m_StyleType == ONE_PLAYER_TWO_SIDES  &&
		pn != GAMESTATE->m_MasterPlayerNumber )
		return;


	// Draw these below everything else.
	if( m_pPlayerState->m_PlayerOptions.GetCurrent().m_fBlind == 0 )
	{
		if( m_pCombo )
			m_pCombo->Draw();
	}

	if( m_pAttackDisplay )
		m_pAttackDisplay->Draw();

	if( TAP_JUDGMENTS_UNDER_FIELD )
		DrawTapJudgments();

	if( HOLD_JUDGMENTS_UNDER_FIELD )
		DrawHoldJudgments();

	float fTilt = m_pPlayerState->m_PlayerOptions.GetCurrent().m_fPerspectiveTilt;
	float fSkew = m_pPlayerState->m_PlayerOptions.GetCurrent().m_fSkew;
	bool bReverse = m_pPlayerState->m_PlayerOptions.GetCurrent().GetReversePercentForColumn(0)>0.5;


	DISPLAY->CameraPushMatrix();
	DISPLAY->PushMatrix();

	float fCenterY = this->GetY()+(GRAY_ARROWS_Y_STANDARD+GRAY_ARROWS_Y_REVERSE)/2;

	DISPLAY->LoadMenuPerspective( 45, SCREEN_WIDTH, SCREEN_HEIGHT, SCALE(fSkew,0.f,1.f,this->GetX(),SCREEN_CENTER_X), fCenterY );

	if( m_pNoteField )
	{
		float fOriginalY = 	m_pNoteField->GetY();

		float fTiltDegrees = SCALE(fTilt,-1.f,+1.f,+30,-30) * (bReverse?-1:1);

		float fZoom = SCALE( m_pPlayerState->m_PlayerOptions.GetCurrent().m_fEffects[PlayerOptions::EFFECT_TINY], 0.f, 1.f, 1.f, 0.5f );
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
	}

	DISPLAY->CameraPopMatrix();
	DISPLAY->PopMatrix();


	if( !(bool)TAP_JUDGMENTS_UNDER_FIELD )
		DrawTapJudgments();

	if( !(bool)HOLD_JUDGMENTS_UNDER_FIELD )
		DrawHoldJudgments();
}

void Player::DrawTapJudgments()
{
	if( m_pPlayerState->m_PlayerOptions.GetCurrent().m_fBlind > 0 )
		return;

	if( m_sprJudgmentFrame.IsLoaded() )
		m_sprJudgmentFrame->Draw();
	if( m_pJudgment )
		m_pJudgment->Draw();
}

void Player::DrawHoldJudgments()
{
	if( m_pPlayerState->m_PlayerOptions.GetCurrent().m_fBlind > 0 )
		return;

	for( int c=0; c<m_NoteData.GetNumTracks(); c++ )
		m_vHoldJudgment[c]->Draw();
}


void Player::ChangeLife( TapNoteScore tns )
{
	PlayerNumber pn = m_pPlayerState->m_PlayerNumber;
	if( m_pLifeMeter )
		m_pLifeMeter->ChangeLife( tns );
	if( m_pCombinedLifeMeter )
		m_pCombinedLifeMeter->ChangeLife( pn, tns );

	ChangeLifeRecord();
}

void Player::ChangeLife( HoldNoteScore hns, TapNoteScore tns )
{
	PlayerNumber pn = m_pPlayerState->m_PlayerNumber;
	if( m_pLifeMeter )
		m_pLifeMeter->ChangeLife( hns, tns );
	if( m_pCombinedLifeMeter )
		m_pCombinedLifeMeter->ChangeLife( pn, hns, tns );

	ChangeLifeRecord();
}

void Player::ChangeLifeRecord()
{
	PlayerNumber pn = m_pPlayerState->m_PlayerNumber;
	float fLife = -1;
	if( m_pLifeMeter )
	{
		fLife = m_pLifeMeter->GetLife();
	}
	else if( m_pCombinedLifeMeter )
	{
		fLife = GAMESTATE->m_fTugLifePercentP1;
		if( pn == PLAYER_2 )
			fLife = 1.0f - fLife;
	}
	if( fLife != -1 )
		if( m_pPlayerStageStats )
			m_pPlayerStageStats->SetLifeRecordAt( fLife, STATSMAN->m_CurStageStats.m_fStepsSeconds );
}

int Player::GetClosestNoteDirectional( int col, int iStartRow, int iEndRow, bool bAllowGraded, bool bForward ) const
{
	NoteData::const_iterator begin, end;
	m_NoteData.GetTapNoteRange( col, iStartRow, iEndRow, begin, end );

	if( !bForward )
		swap( begin, end );

	while( begin != end )
	{
		if( !bForward )
			--begin;

		/* Is this the row we want? */
		do {
			const TapNote &tn = begin->second;
			if( tn.type == TapNote::empty )
				break;
			if( !bAllowGraded && tn.result.tns != TNS_None )
				break;

			return begin->first;
		} while(0);

		if( bForward )
			++begin;
	}

	return -1;
}

/* Find the closest note to fBeat. */
int Player::GetClosestNote( int col, int iNoteRow, int iMaxRowsAhead, int iMaxRowsBehind, bool bAllowGraded ) const
{
	// Start at iIndexStartLookingAt and search outward.
	int iNextIndex = GetClosestNoteDirectional( col, iNoteRow, iNoteRow+iMaxRowsAhead, bAllowGraded, true );
	int iPrevIndex = GetClosestNoteDirectional( col, iNoteRow-iMaxRowsBehind, iNoteRow, bAllowGraded, false );

	if( iNextIndex == -1 && iPrevIndex == -1 )
		return -1;
	if( iNextIndex == -1 )
		return iPrevIndex;
	if( iPrevIndex == -1 )
		return iNextIndex;

	/* Figure out which row is closer. */
	if( abs(iNoteRow-iNextIndex) > abs(iNoteRow-iPrevIndex) )
		return iPrevIndex;
	else
		return iNextIndex;
}

int Player::GetClosestNonEmptyRowDirectional( int iStartRow, int iEndRow, bool bAllowGraded, bool bForward ) const
{
	if( bForward )
	{
		NoteData::all_tracks_iterator iter = m_NoteData.GetTapNoteRangeAllTracks( iStartRow, iEndRow );

		while( !iter.IsAtEnd() )
		{
			if( NoteDataWithScoring::IsRowCompletelyJudged(m_NoteData, iter.Row(), m_pPlayerState->m_PlayerNumber) )
			{
				++iter;
				continue;
			}
			return iter.Row();
		}
	}
	else
	{
		NoteData::all_tracks_reverse_iterator iter = m_NoteData.GetTapNoteRangeAllTracksReverse( iStartRow, iEndRow );

		while( !iter.IsAtEnd() )
		{
			if( NoteDataWithScoring::IsRowCompletelyJudged(m_NoteData, iter.Row(), m_pPlayerState->m_PlayerNumber) )
			{
				++iter;
				continue;
			}
			return iter.Row();
		}
	}

	return -1;
}

/* Find the closest note to fBeat. */
int Player::GetClosestNonEmptyRow( int iNoteRow, int iMaxRowsAhead, int iMaxRowsBehind, bool bAllowGraded ) const
{
	// Start at iIndexStartLookingAt and search outward.
	int iNextRow = GetClosestNonEmptyRowDirectional( iNoteRow, iNoteRow+iMaxRowsAhead, bAllowGraded, true );
	int iPrevRow = GetClosestNonEmptyRowDirectional( iNoteRow-iMaxRowsBehind, iNoteRow, bAllowGraded, false );

	if( iNextRow == -1 && iPrevRow == -1 )
		return -1;
	if( iNextRow == -1 )
		return iPrevRow;
	if( iPrevRow == -1 )
		return iNextRow;

	/* Figure out which row is closer. */
	if( abs(iNoteRow-iNextRow) > abs(iNoteRow-iPrevRow) )
		return iPrevRow;
	else
		return iNextRow;
}

bool Player::IsOniDead() const
{
	// If we're playing on oni and we've died, do nothing.
	return GAMESTATE->m_SongOptions.GetCurrent().m_LifeType == SongOptions::LIFE_BATTERY && m_pPlayerStageStats  && m_pPlayerStageStats->m_bFailed;
}

void Player::Fret( int col, int row, const RageTimer &tm, bool bHeld, bool bRelease )
{
	if( IsOniDead() )
		return;
	
	DEBUG_ASSERT_M( col >= 0  &&  col <= m_NoteData.GetNumTracks(), ssprintf("%i, %i", col, m_NoteData.GetNumTracks()) );

	m_vbFretIsDown[ col ] = !bRelease;

	// Handle hammer-ons and pull-offs
	const float fPositionSeconds = GAMESTATE->m_fMusicSeconds - tm.Ago();
	int iHopoCol = -1;
	bool bDoHopo = 
		m_pPlayerState->m_fLastHopoNoteMusicSeconds != -1  &&
		fPositionSeconds <= m_pPlayerState->m_fLastHopoNoteMusicSeconds + HOPO_CHAIN_SECONDS;
	if( bDoHopo )
	{
		// do a Hopo:
		//  - on pressed fret is no higher fret is held
		//  - on next lowest held fret when the highest held fret is released
		bool bHigherFretIsDown = false;
		for( int i=col+1; i<m_NoteData.GetNumTracks(); i++ )
		{
			if( m_vbFretIsDown[i] )
			{
				bHigherFretIsDown = true;
				break;
			}
		}
		if( bHigherFretIsDown )
			bDoHopo = false;
	}

	if( bDoHopo )
	{
		if( !bRelease )
		{
			// hammer-on
			iHopoCol = col;
		}
		else
		{
			// pull-off
			// find next lowest fret that is held
			for( int i=col-1; i>=0; i-- )
			{
				if( m_vbFretIsDown[i] )
				{
					iHopoCol = i;
					break;
				}
			}
			if( iHopoCol == -1 )
				bDoHopo = false;
		}
	}

	if( bDoHopo )
		Hopo( iHopoCol, row, tm, bHeld, bRelease );
}

void Player::StepStrumHopo( int col, int row, const RageTimer &tm, bool bHeld, bool bRelease, Player::ButtonType pbt )
{
	if( IsOniDead() )
		return;

	// Do everything that depends on a RageTime here and set your breakpoints somewhere after this block
	const float fLastBeatUpdate = GAMESTATE->m_LastBeatUpdate.Ago();
	const float fPositionSeconds = GAMESTATE->m_fMusicSeconds - tm.Ago();
	const float fTimeSinceStep = tm.Ago();

	switch( pbt )
	{
	DEFAULT_FAIL(pbt);
	case ButtonType_Strum:
		if( bRelease )
			return;	
		break;
	case ButtonType_Hopo:
		bRelease = false;
		break;
	case ButtonType_Step:
		break;
	}

	const float fSongBeat = GAMESTATE->m_pCurSong ? GAMESTATE->m_pCurSong->GetBeatFromElapsedTime( fPositionSeconds ) : GAMESTATE->m_fSongBeat;
	const int iSongRow = row == -1 ? BeatToNoteRow( fSongBeat ) : row;
	
	if( col != -1 && !bRelease )
	{
		// Update roll life
		// Let's not check the whole array every time.
		// Instead, only check 1 beat back.  Even 1 is overkill.
		// Just update the life here and let Update judge the roll.
		const int iStartCheckingAt = max( 0, iSongRow-BeatToNoteRow(1) );
		NoteData::iterator begin, end;
		m_NoteData.GetTapNoteRangeInclusive( col, iStartCheckingAt, iSongRow+1, begin, end );
		for( ; begin != end; ++begin )
		{
			TapNote &tn = begin->second;
			if( tn.type != TapNote::hold_head )
				continue;
			const int iRow = begin->first;

			HoldNoteScore hns = tn.HoldResult.hns;
			if( hns != HNS_None )	// if this HoldNote already has a result
				continue;	// we don't need to update the logic for this one

			// if they got a bad score or haven't stepped on the corresponding tap yet
			const TapNoteScore tns = tn.result.tns;
			const bool bSteppedOnTapNote = tns != TNS_None  &&  tns != TNS_Miss;	// did they step on the start of this roll?
			const int iEndRow = iRow + tn.iDuration;

			if( bSteppedOnTapNote && tn.HoldResult.fLife != 0 )
			{
				/* This hold note is not judged and we stepped on its head.  Update iLastHeldRow.
				 * Do this even if we're a little beyond the end of the hold note, to make sure
				 * iLastHeldRow is clamped to iEndRow if the hold note is held all the way. */
				tn.HoldResult.iLastHeldRow = min( iSongRow, iEndRow );
			}

			// If the song beat is in the range of this hold:
			if( iRow <= iSongRow && iRow <= iEndRow )
			{
				switch( tn.subType )
				{
				case TapNote::hold_head_hold:
					// this is handled in Update
					break;
				case TapNote::hold_head_roll:
					if( bSteppedOnTapNote )
					{
						// Increase life
						tn.HoldResult.fLife = 1;

						bool bBright = m_pPlayerStageStats && m_pPlayerStageStats->m_iCurCombo>(int)BRIGHT_GHOST_COMBO_THRESHOLD;
						if( m_pNoteField )
							m_pNoteField->DidHoldNote( col, HNS_Held, bBright );
					}
					break;
				default:
					ASSERT(0);
				}
			}
		}
	}



	//
	// Count calories for this step, unless we're being called because a button is
	// held over a mine or being released.
	//
	if( m_pPlayerStageStats && m_pPlayerState && !bHeld && !bRelease )
	{
		// TODO: remove use of PlayerNumber
		PlayerNumber pn = m_pPlayerState->m_PlayerNumber;
		Profile *pProfile = PROFILEMAN->GetProfile( pn );
		if( pProfile )
		{
			int iNumTracksHeld = 0;
			for( int t=0; t<m_NoteData.GetNumTracks(); t++ )
			{
				GameInput GameI = GAMESTATE->GetCurrentStyle()->StyleInputToGameInput( t, pn );
				const float fSecsHeld = INPUTMAPPER->GetSecsHeld( GameI );
				if( fSecsHeld > 0  && fSecsHeld < JUMP_WINDOW_SECONDS )
					iNumTracksHeld++;
			}

			float fCals = 0;
			switch( iNumTracksHeld )
			{
			case 0:
				// autoplay is on, or this is a computer player
				iNumTracksHeld = 1;
				// fall through
			default:
				{
					float fCalsFor100Lbs = SCALE( iNumTracksHeld, 1, 2, 0.023f, 0.077f );
					float fCalsFor200Lbs = SCALE( iNumTracksHeld, 1, 2, 0.041f, 0.133f );
					fCals = SCALE( pProfile->GetCalculatedWeightPounds(), 100.f, 200.f, fCalsFor100Lbs, fCalsFor200Lbs );
				}
				break;
			}

			m_pPlayerStageStats->m_fCaloriesBurned += fCals;
		}
	}




	//
	// Check for step on a TapNote
	//
	/* XXX: This seems wrong. If a player steps twice quickly and two notes are close together in the same column then
	 * it is possible for the two notes to be graded out of order. Two possible fixes:
	 * 1. Adjust the fSongBeat (or the resulting note row) backward by iStepSearchRows and search forward two iStepSearchRows
	 * lengths, disallowing graded. This doesn't seem right because if a second note has passed, an earlier one should not
	 * be graded.
	 * 2. Clamp the distance searched backward to the previous row graded.
	 * Either option would fundamentally change the grading of two quick notes "jack hammers." Hmm.
	 */
	const int iStepSearchRows = BeatToNoteRow( StepSearchDistance * GAMESTATE->m_fCurBPS * GAMESTATE->m_SongOptions.GetCurrent().m_fMusicRate );
	int iRowOfOverlappingNoteOrRow = row;
	if( row == -1 )
	{
		switch( pbt )
		{
		DEFAULT_FAIL(pbt);
		case ButtonType_Strum:
			iRowOfOverlappingNoteOrRow = GetClosestNonEmptyRow( BeatToNoteRow(fSongBeat), iStepSearchRows, iStepSearchRows, false );
			break;
		case ButtonType_Hopo:
		case ButtonType_Step:
			iRowOfOverlappingNoteOrRow = GetClosestNote( col, BeatToNoteRow(fSongBeat), iStepSearchRows, iStepSearchRows, false );
			break;
		}
	}
	
	// calculate TapNoteScore
	TapNoteScore score = TNS_None;

	if( iRowOfOverlappingNoteOrRow != -1 )
	{
		// compute the score for this hit
		float fNoteOffset = 0.0f;
		// we need this later if we are autosyncing
		const float fStepBeat = NoteRowToBeat( iRowOfOverlappingNoteOrRow );
		const float fStepSeconds = GAMESTATE->m_pCurSong->GetElapsedTimeFromBeat(fStepBeat);

		if( row == -1 )
		{
			/* We actually stepped on the note this long ago: */
			//fTimeSinceStep
		
			/* GAMESTATE->m_fMusicSeconds is the music time as of GAMESTATE->m_LastBeatUpdate. Figure
			 * out what the music time is as of now. */
			const float fCurrentMusicSeconds = GAMESTATE->m_fMusicSeconds + (fLastBeatUpdate*GAMESTATE->m_SongOptions.GetCurrent().m_fMusicRate);

			/* ... which means it happened at this point in the music: */
			const float fMusicSeconds = fCurrentMusicSeconds - fTimeSinceStep * GAMESTATE->m_SongOptions.GetCurrent().m_fMusicRate;

			// The offset from the actual step in seconds:
			fNoteOffset = (fStepSeconds - fMusicSeconds) / GAMESTATE->m_SongOptions.GetCurrent().m_fMusicRate;	// account for music rate
//			LOG->Trace("step was %.3f ago, music is off by %f: %f vs %f, step was %f off", 
//				fTimeSinceStep, GAMESTATE->m_LastBeatUpdate.Ago()/GAMESTATE->m_SongOptions.m_fMusicRate,
//				fStepSeconds, fMusicSeconds, fNoteOffset );
		}

		const float fSecondsFromExact = fabsf( fNoteOffset );

		TapNote tnDummy = TAP_ORIGINAL_TAP;
		TapNote *pTN = NULL;
		switch( pbt )
		{
		DEFAULT_FAIL(pbt);
		case ButtonType_Strum:
			pTN = &tnDummy;
			break;
		case ButtonType_Hopo:
		case ButtonType_Step:
			NoteData::iterator iter = m_NoteData.FindTapNote( col, iRowOfOverlappingNoteOrRow );
			DEBUG_ASSERT( iter!= m_NoteData.end(col) );
			pTN = &iter->second;
			break;
		}

		switch( m_pPlayerState->m_PlayerController )
		{
		case PC_HUMAN:
			switch( pTN->type )
			{
			case TapNote::mine:
				// Stepped too close to mine?
				if( !bRelease && fSecondsFromExact <= GetWindowSeconds(TW_Mine) )
					score = TNS_HitMine;
				break;

			case TapNote::attack:
				if( !bRelease && fSecondsFromExact <= GetWindowSeconds(TW_Attack) && !pTN->result.bHidden )
					score = TNS_W2; /* sentinel */
				break;

			default:
				if( (pTN->type == TapNote::lift) == bRelease )
				{
					if(	 fSecondsFromExact <= GetWindowSeconds(TW_W1) )	score = TNS_W1;
					else if( fSecondsFromExact <= GetWindowSeconds(TW_W2) )	score = TNS_W2;
					else if( fSecondsFromExact <= GetWindowSeconds(TW_W3) )	score = TNS_W3;
					else if( fSecondsFromExact <= GetWindowSeconds(TW_W4) )	score = TNS_W4;
					else if( fSecondsFromExact <= GetWindowSeconds(TW_W5) )	score = TNS_W5;
				}
				break;
			}
			break;
		
		case PC_CPU:
		case PC_AUTOPLAY:
			score = PlayerAI::GetTapNoteScore( m_pPlayerState );

			/* XXX: This doesn't make sense. Step should only be called in autoplay for hit notes */
#if 0
			// GetTapNoteScore always returns TNS_W1 in autoplay.
			// If the step is far away, don't judge it.
			if( m_pPlayerState->m_PlayerController == PC_AUTOPLAY &&
				fSecondsFromExact > GetWindowSeconds(TW_W5) )
			{
				score = TNS_None;
				break;
			}
#endif

			// TRICKY:  We're asking the AI to judge mines.  consider TNS_W4 and below
			// as "mine was hit" and everything else as "mine was avoided"
			if( pTN->type == TapNote::mine )
			{
				// The CPU hits a lot of mines.  Only consider hitting the 
				// first mine for a row.  We know we're the first mine if 
				// there are are no mines to the left of us.
				for( int t=0; t<col; t++ )
				{
					if( m_NoteData.GetTapNote(t,iRowOfOverlappingNoteOrRow).type == TapNote::mine )	// there's a mine to the left of us
						return;	// avoid
				}

				// The CPU hits a lot of mines.  Make it less likely to hit 
				// mines that don't have a tap note on the same row.
				bool bTapsOnRow = m_NoteData.IsThereATapOrHoldHeadAtRow( iRowOfOverlappingNoteOrRow );
				TapNoteScore get_to_avoid = bTapsOnRow ? TNS_W3 : TNS_W4;

				if( score >= get_to_avoid )
					return;	// avoided
				else
					score = TNS_HitMine;
			}

			if( pTN->type == TapNote::attack && score > TNS_W4 )
				score = TNS_W2; /* sentinel */

			/* AI will generate misses here.  Don't handle a miss like a regular note because
			 * we want the judgment animation to appear delayed.  Instead, return early if
			 * AI generated a miss, and let UpdateMissedTapNotesOlderThan() detect and handle the 
			 * misses. */
			if( score == TNS_Miss )
				return;

			// Put some small, random amount in fNoteOffset so that demonstration 
			// show a mix of late and early.
			fNoteOffset = randomf( -0.1f, 0.1f );

			break;

		default:
			ASSERT(0);
			score = TNS_None;
			break;
		}

		
		switch( pbt )
		{
		DEFAULT_FAIL(pbt);
		case ButtonType_Strum:
			{
				bool bNoteRowMatchesFrets = true;
				int iFirstNoteCol = -1;
				for( int i=0; i<GAMESTATE->GetCurrentStyle()->m_iColsPerPlayer; i++ )
				{
					const TapNote &tn = m_NoteData.GetTapNote( i, iRowOfOverlappingNoteOrRow );
					bool bIsNote = (tn.type != TapNote::empty);
					if( iFirstNoteCol == -1  &&  bIsNote )
						iFirstNoteCol = i;

					// Extra notes to the left (higher up on the string) can be held without penalty.  It's necessary to hold
					// the extra frets or pull-offs.
					if( iFirstNoteCol == -1 )
						continue;

					bool bNoteMatchesFret = m_vbFretIsDown[i] == bIsNote;
					if( !bNoteMatchesFret )
					{
						bNoteRowMatchesFrets = false;
						break;
					}
				}
				ASSERT( iFirstNoteCol != -1 );
				if( !bNoteRowMatchesFrets )
				{
					score = TNS_None;
				}
				else
				{
					int iLastNoteCol = -1;
					for( int i=GAMESTATE->GetCurrentStyle()->m_iColsPerPlayer-1; i>=0; i-- )
					{
						const TapNote &tn = m_NoteData.GetTapNote( i, iRowOfOverlappingNoteOrRow );
						bool bIsNote = (tn.type != TapNote::empty);
						if( bIsNote )
						{
							iLastNoteCol = i;
							break;
						}
					}

					m_pPlayerState->m_fLastHopoNoteMusicSeconds = fStepSeconds;
					m_pPlayerState->m_iLastHopoNoteCol = iLastNoteCol; 
				}
			}
			break;
		case ButtonType_Hopo:
			{
				bool bDidHopo = true;

				{
					// only can hopo on a row with one note
					if( m_NoteData.GetNumTapNotesInRow(iRowOfOverlappingNoteOrRow) != 1 )
					{
						bDidHopo = false;
						goto done_checking_hopo;
					}

					// con't hopo on the same note 2x in a row
					if( col == m_pPlayerState->m_iLastHopoNoteCol )
					{
						bDidHopo = false;
						goto done_checking_hopo;
					}

					const TapNote &tn = m_NoteData.GetTapNote( col, iRowOfOverlappingNoteOrRow );
					ASSERT( tn.type != TapNote::empty );

					int iRowsAgoLastNote = 100000;	// TODO: find more reasonable value based on HOPO_CHAIN_SECONDS?
					NoteData::all_tracks_reverse_iterator iter = m_NoteData.GetTapNoteRangeAllTracksReverse( iRowsAgoLastNote-iRowsAgoLastNote, iRowOfOverlappingNoteOrRow-1 );
					ASSERT( !iter.IsAtEnd() );	// there must have been a note that started the hopo
					if( !NoteDataWithScoring::IsRowCompletelyJudged(m_NoteData, iter.Row(), m_pPlayerState->m_PlayerNumber) )
					{
						bDidHopo = false;
						goto done_checking_hopo;
					}

					const TapNoteResult &lastTNR = NoteDataWithScoring::LastTapNoteWithResult( m_NoteData, iter.Row(), m_pPlayerState->m_PlayerNumber ).result;
					if( lastTNR.tns <= TNS_Miss )
					{
						bDidHopo = false;
						goto done_checking_hopo;
					}
				}
done_checking_hopo:
				if( !bDidHopo )
				{
					score = TNS_None;
				}
				else
				{
					m_pPlayerState->m_fLastHopoNoteMusicSeconds = fStepSeconds;
					m_pPlayerState->m_iLastHopoNoteCol = col;
				}
			}
			break;
		case ButtonType_Step:
			break;
		}


		if( pTN->type == TapNote::attack && score == TNS_W2 )
		{
			score = TNS_None;	// don't score this as anything

			m_soundAttackLaunch.Play();

			// put attack in effect
			Attack attack(
				ATTACK_LEVEL_1,
				-1,	// now
				pTN->fAttackDurationSeconds,
				pTN->sAttackModifiers,
				true,
				false
				);

			// TODO: Remove use of PlayerNumber
			PlayerNumber pnToAttack = OPPOSITE_PLAYER[m_pPlayerState->m_PlayerNumber];
			PlayerState *pPlayerStateToAttack = GAMESTATE->m_pPlayerState[pnToAttack];
			pPlayerStateToAttack->LaunchAttack( attack );

			// remove all TapAttacks on this row
			for( int t=0; t<m_NoteData.GetNumTracks(); t++ )
			{
				const TapNote &tn = m_NoteData.GetTapNote( t, iRowOfOverlappingNoteOrRow );
				if( tn.type == TapNote::attack )
					HideNote( t, iRowOfOverlappingNoteOrRow );
			}
		}

		if( m_pPlayerState->m_PlayerController == PC_HUMAN && score >= TNS_W3 ) 
			AdjustSync::HandleAutosync( fNoteOffset, fStepSeconds );

		// Do game-specific and mode-specific score mapping.
		score = GAMESTATE->GetCurrentGame()->MapTapNoteScore( score );
		if( score == TNS_W1 && !GAMESTATE->ShowW1() )
			score = TNS_W2;


		if( score != TNS_None )
		{
			switch( pbt )
			{
			DEFAULT_FAIL(pbt);
			case ButtonType_Strum:
				for( int t=0; t<m_NoteData.GetNumTracks(); t++ )
				{
					TapNote tn = m_NoteData.GetTapNote( t, iRowOfOverlappingNoteOrRow );
					if( tn.type != TapNote::empty )
					{
						tn.result.tns = score;
						tn.result.fTapNoteOffset = -fNoteOffset;
						m_NoteData.SetTapNote( t, iRowOfOverlappingNoteOrRow, tn );
					}
				}
				break;
			case ButtonType_Hopo:
			case ButtonType_Step:
				pTN->result.tns = score;
				pTN->result.fTapNoteOffset = -fNoteOffset;
				break;
			}
		}

		PlayerNumber pn = pTN->pn == PLAYER_INVALID ? m_pPlayerState->m_PlayerNumber : pTN->pn;
		m_LastTapNoteScore = score;
		if( GAMESTATE->GetCurrentGame()->m_bCountNotesSeparately )
		{
			if( pTN->type != TapNote::mine )
			{
				const bool bBlind = (m_pPlayerState->m_PlayerOptions.GetCurrent().m_fBlind != 0);
				// XXX This is the wrong combo for shared players. STATSMAN->m_CurStageStats.m_Player[pn] might work but could be wrong.
				const bool bBright = m_pPlayerStageStats && m_pPlayerStageStats->m_iCurCombo > int(BRIGHT_GHOST_COMBO_THRESHOLD) || bBlind;			
				if( m_pNoteField )
					m_pNoteField->DidTapNote( col, score, bBright );
				if( score >= TNS_W3 || bBlind )
					HideNote( col, iRowOfOverlappingNoteOrRow );
			}
		}
		else if( NoteDataWithScoring::IsRowCompletelyJudged(m_NoteData, iRowOfOverlappingNoteOrRow, pn) )
		{
			FlashGhostRow( iRowOfOverlappingNoteOrRow, pn );
		}
	}

	// check for hopo end
	if( score <= TNS_Miss )
	{
		m_pPlayerState->ClearHopoState();
	}


	if( score != TNS_None )
	{
		/* Search for keyed sounds separately.  If we can't find a nearby note, search
		 * backwards indefinitely, and ignore grading. */
		/* XXX: This isn't quite right. As per the above XXX for iRowOfOverlappingNote, if iRowOfOverlappingNote
		 * is set to a previous note, the keysound could have changed and this would cause the wrong one to play,
		 * in essence playing two sounds in the opposite order. Maybe this should always perform the search. Still,
		 * even that doesn't seem quite right since it would then play the same (new) keysound twice which would
		 * sound wrong even though the notes were judged as being correct, above. Fixing the above problem would
		 * fix this one as well. */
		if( iRowOfOverlappingNoteOrRow == -1 )
			iRowOfOverlappingNoteOrRow = GetClosestNote( col, BeatToNoteRow(fSongBeat),
								iStepSearchRows, MAX_NOTE_ROW, true );
		if( iRowOfOverlappingNoteOrRow != -1 )
		{
			switch( pbt )
			{
			DEFAULT_FAIL(pbt);
			case ButtonType_Strum:
				for( int i=0; i<m_NoteData.GetNumTracks(); i++ )
				{
					const TapNote &tn = m_NoteData.GetTapNote( i, iRowOfOverlappingNoteOrRow );
					if( tn.iKeysoundIndex >= 0 && tn.iKeysoundIndex < (int) m_vKeysounds.size() )
						m_vKeysounds[tn.iKeysoundIndex].Play();
				}
				break;
			case ButtonType_Step:
			case ButtonType_Hopo:
				const TapNote &tn = m_NoteData.GetTapNote( col, iRowOfOverlappingNoteOrRow );
				if( tn.iKeysoundIndex >= 0 && tn.iKeysoundIndex < (int) m_vKeysounds.size() )
					m_vKeysounds[tn.iKeysoundIndex].Play();
				break;
			}
		}
	}
	// XXX:
	if( !bRelease )
	{
		if( m_pNoteField )
		{
			switch( pbt )
			{
			DEFAULT_FAIL(pbt);
			case ButtonType_Strum:
				{
					// only pulse the shortest string fret
					int iLastFret = -1;
					for( int i=0; i<m_NoteData.GetNumTracks(); i++ )
					{
						if( m_vbFretIsDown[i] )
							iLastFret = i;
					}
					if( iLastFret != -1 ) 
						m_pNoteField->Step( iLastFret, score );
				}
				break;
			case ButtonType_Step:
				m_pNoteField->Step( col, score );
				break;
			case ButtonType_Hopo:
				// no animation
				break;
			}
		}
		MESSAGEMAN->Broadcast( m_sMessageToSendOnStep );
	}
}

static bool Unjudged( const TapNote &tn )
{
	if( tn.result.tns != TNS_None )
		return false;
	switch( tn.type )
	{
	case TapNote::tap:
	case TapNote::hold_head:
	case TapNote::mine:
	case TapNote::lift:
		return true;
	}
	return false;
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
		GAMESTATE->m_pCurSong->m_Timing.GetBeatAndBPSFromElapsedTime( fEarliestTime, fMissIfOlderThanThisBeat, fThrowAway, bFreeze );

		iMissIfOlderThanThisIndex = BeatToNoteRow( fMissIfOlderThanThisBeat );
		if( bFreeze )
		{
			/* If there is a freeze on iMissIfOlderThanThisIndex, include this index too.
			 * Otherwise we won't show misses for tap notes on freezes until the
			 * freeze finishes. */
			iMissIfOlderThanThisIndex++;
		}
	}


	// m_iRowLastJudged and m_iMineRowLastJudged have already been judged.
	const int iStartIndex = min( m_iRowLastJudged, m_iMineRowLastJudged ) + 1;
	bool bMisses = false;
	
	NoteData::all_tracks_iterator iter = m_NoteData.GetTapNoteRangeAllTracks( iStartIndex, iMissIfOlderThanThisIndex-1, Unjudged );
	
	for( ; !iter.IsAtEnd(); ++iter )
	{
		TapNote &tn = *iter;
		
		if( tn.pn != PLAYER_INVALID && tn.pn != m_pPlayerState->m_PlayerNumber )
			continue;
		if( tn.type == TapNote::mine )
		{
			tn.result.tns = TNS_AvoidMine;
			
			/* The only real way to tell if a mine has been scored is if it has disappeared
			 * but this only works for hit mines so update the scores for avoided mines here. */
			if( m_pPrimaryScoreKeeper )
				m_pPrimaryScoreKeeper->HandleTapScore( tn );
			if( m_pSecondaryScoreKeeper )
				m_pSecondaryScoreKeeper->HandleTapScore( tn );
		}
		else
		{
			bMisses = true;
			tn.result.tns = TNS_Miss;
		}
	}
		
	if( bMisses )
		SetJudgment( TNS_Miss, false );
}

static bool MinesNotHidden( const TapNote &tn )
{
	return tn.type == TapNote::mine && !tn.result.bHidden;
}

void Player::UpdateJudgedRows()
{
	const int iEndRow = BeatToNoteRow( GAMESTATE->m_fSongBeat );
	PlayerNumber pn = m_pPlayerState->m_PlayerNumber;
	bool bAllJudged = true;
	const bool bSeparately = GAMESTATE->GetCurrentGame()->m_bCountNotesSeparately;
	
	for( int iRow = m_iRowLastJudged+1; iRow <= iEndRow; ++iRow )
	{
		if( !NoteDataWithScoring::IsRowCompletelyJudged(m_NoteData, iRow, pn) )
		{
			bAllJudged = false;
			continue;
		}
		if( bAllJudged )
			++m_iRowLastJudged;
		if( m_JudgedRows[iRow] )
			continue;
		const TapNoteResult &lastTNR = NoteDataWithScoring::LastTapNoteWithResult( m_NoteData, iRow, pn ).result;
		
		if( lastTNR.tns < TNS_Miss )
			continue;
		if( bSeparately )
		{
			for( int iTrack = 0; iTrack < m_NoteData.GetNumTracks(); ++iTrack )
			{
				const TapNote &tn = m_NoteData.GetTapNote( iTrack, iRow );
				if( tn.type == TapNote::empty || tn.type == TapNote::mine ) continue;
				if( tn.pn != PLAYER_INVALID && tn.pn != pn ) continue;
				SetJudgment( tn.result.tns, tn.result.fTapNoteOffset < 0.0f );
			}
		}
		else
		{
			SetJudgment( lastTNR.tns, lastTNR.fTapNoteOffset < 0.0f );
		}
		HandleTapRowScore( iRow );
	}
	
	NoteData::all_tracks_iterator iter = m_NoteData.GetTapNoteRangeAllTracks( m_iMineRowLastJudged+1, iEndRow, MinesNotHidden );
	
	bAllJudged = true;
	for( ; !iter.IsAtEnd(); ++iter )
	{
		TapNote &tn = *iter;
		
		switch( tn.result.tns )
		{
		case TNS_None:		bAllJudged = false;
		case TNS_AvoidMine:	continue;
		case TNS_HitMine:	break;
		DEFAULT_FAIL( tn.result.tns );
		}
		if( m_pNoteField )
			m_pNoteField->DidTapNote( iter.Track(), tn.result.tns, false );
		
		if( tn.pn != PLAYER_INVALID && tn.pn != pn )
			continue;
		if( tn.iKeysoundIndex >= 0 && tn.iKeysoundIndex < (int) m_vKeysounds.size() )
			m_vKeysounds[tn.iKeysoundIndex].Play();
		else
			m_soundMine.Play();
		
		ChangeLife( tn.result.tns );
		if( m_pScoreDisplay )
			m_pScoreDisplay->OnJudgment( tn.result.tns );
		if( m_pSecondaryScoreDisplay )
			m_pSecondaryScoreDisplay->OnJudgment( tn.result.tns );
		
		// Make sure hit mines affect the dance points.
		if( m_pPrimaryScoreKeeper )
			m_pPrimaryScoreKeeper->HandleTapScore( tn );
		if( m_pSecondaryScoreKeeper )
			m_pSecondaryScoreKeeper->HandleTapScore( tn );
		tn.result.bHidden = true;
		// Subtract one to ensure that we have actually completed the row.
		if( bAllJudged && iter.Row() - 1 > m_iMineRowLastJudged )
			m_iMineRowLastJudged = iter.Row() - 1;
	}
	if( bAllJudged )
		m_iMineRowLastJudged = iEndRow;
}

void Player::FlashGhostRow( int iRow, PlayerNumber pn )
{
	TapNoteScore lastTNS = NoteDataWithScoring::LastTapNoteWithResult( m_NoteData, iRow, pn ).result.tns;
	const bool bBlind = (m_pPlayerState->m_PlayerOptions.GetCurrent().m_fBlind != 0);
	// XXX This is the wrong combo for shared players. STATSMAN->m_CurStageStats.m_Player[pn] might work but could be wrong.
	const bool bBright = m_pPlayerStageStats && m_pPlayerStageStats->m_iCurCombo > int(BRIGHT_GHOST_COMBO_THRESHOLD) || bBlind;
		
	for( int iTrack = 0; iTrack < m_NoteData.GetNumTracks(); ++iTrack )
	{
		const TapNote &tn = m_NoteData.GetTapNote( iTrack, iRow );
		
		if( tn.type == TapNote::empty || tn.type == TapNote::mine )
			continue;
		if( tn.pn != PLAYER_INVALID && tn.pn != pn )
			continue;
		if( m_pNoteField )
			m_pNoteField->DidTapNote( iTrack, lastTNS, bBright );
		if( lastTNS >= TNS_W3 || bBlind )
			HideNote( iTrack, iRow );
	}
}

void Player::CrossedRow( int iNoteRow, const RageTimer &now )
{
	// If we're doing random vanish, randomise notes on the fly.
	if( m_pPlayerState->m_PlayerOptions.GetCurrent().m_fAppearances[PlayerOptions::APPEARANCE_RANDOMVANISH]==1 )
		RandomizeNotes( iNoteRow );

	// check to see if there's a note at the crossed row
	if( m_pPlayerState->m_PlayerController != PC_HUMAN )
	{
		for( int t=0; t<m_NoteData.GetNumTracks(); t++ )
		{
			const TapNote &tn = m_NoteData.GetTapNote( t, iNoteRow );
			if( tn.type != TapNote::empty && tn.result.tns == TNS_None )
			{
				Step( t, iNoteRow, now, false, false );
				if( m_pPlayerState->m_PlayerController == PC_AUTOPLAY )
					STATSMAN->m_CurStageStats.m_bUsedAutoplay = true;
			}
		}
	}
}

void Player::CrossedMineRow( int iNoteRow, const RageTimer &now )
{
	// Hold the panel while crossing a mine will cause the mine to explode
	for( int t=0; t<m_NoteData.GetNumTracks(); t++ )
	{
		if( m_NoteData.GetTapNote(t,iNoteRow).type == TapNote::mine )
		{
			// TODO: Remove use of PlayerNumber.
			PlayerNumber pn = m_pPlayerState->m_PlayerNumber;

			GameInput GameI = GAMESTATE->GetCurrentStyle()->StyleInputToGameInput( t, pn );
			if( PREFSMAN->m_fPadStickSeconds > 0 )
			{
				float fSecsHeld = INPUTMAPPER->GetSecsHeld( GameI, m_pPlayerState->m_mp );
				if( fSecsHeld >= PREFSMAN->m_fPadStickSeconds )
					Step( t, -1, now - PREFSMAN->m_fPadStickSeconds, true, false );
			}
			else
			{
				bool bIsDown = INPUTMAPPER->IsBeingPressed( GameI, m_pPlayerState->m_mp );
				if( bIsDown )
					Step( t, iNoteRow, now, true, false );
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
	iNewNoteRow = int( iNewNoteRow / m_pPlayerState->m_PlayerOptions.GetCurrent().m_fScrollSpeed );

	int iNumOfTracks = m_NoteData.GetNumTracks();
	for( int t=0; t+1 < iNumOfTracks; t++ )
	{
		const int iSwapWith = RandomInt( iNumOfTracks );

		/* Only swap a tap and an empty. */
		NoteData::iterator iter = m_NoteData.FindTapNote( t, iNewNoteRow );
		if( iter == m_NoteData.end(t) || iter->second.type != TapNote::tap )
			continue;

		// Make sure this is empty.
		if( m_NoteData.FindTapNote(iSwapWith, iNewNoteRow) != m_NoteData.end(iSwapWith) )
			continue;

		/* Make sure the destination row isn't in the middle of a hold. */
		if( m_NoteData.IsHoldNoteAtRow(iSwapWith, iNoteRow) )
			continue;
		
		m_NoteData.SetTapNote( iSwapWith, iNewNoteRow, iter->second );
		m_NoteData.RemoveTapNote( t, iter );
	}
}

void Player::HandleTapRowScore( unsigned row )
{
	bool bNoCheating = true;
#ifdef DEBUG
	bNoCheating = false;
#endif

	if( GAMESTATE->m_bDemonstrationOrJukebox )
		bNoCheating = false;
	// don't accumulate points if AutoPlay is on.
	if( bNoCheating && m_pPlayerState->m_PlayerController == PC_AUTOPLAY )
		return;

	PlayerNumber pn = m_pPlayerState->m_PlayerNumber;
	TapNoteScore scoreOfLastTap = NoteDataWithScoring::LastTapNoteWithResult(m_NoteData, row, pn).result.tns;
	const int iOldCombo = m_pPlayerStageStats ? m_pPlayerStageStats->m_iCurCombo : 0;
	const int iOldMissCombo = m_pPlayerStageStats ? m_pPlayerStageStats->m_iCurMissCombo : 0;

	if( scoreOfLastTap == TNS_Miss )
		m_LastTapNoteScore = TNS_Miss;

	for( int track = 0; track < m_NoteData.GetNumTracks(); ++track )
	{
		const TapNote &tn = m_NoteData.GetTapNote( track, row );
		// Mines cannot be handled here.
		if( tn.type == TapNote::empty || tn.type == TapNote::mine )
			continue;
		if( tn.pn != PLAYER_INVALID && tn.pn != pn )
			continue;
		if( m_pPrimaryScoreKeeper )
			m_pPrimaryScoreKeeper->HandleTapScore( tn );
		if( m_pSecondaryScoreKeeper )
			m_pSecondaryScoreKeeper->HandleTapScore( tn );
	}		

	if( m_pPrimaryScoreKeeper != NULL )
		m_pPrimaryScoreKeeper->HandleTapRowScore( m_NoteData, row );
	if( m_pSecondaryScoreKeeper != NULL )
		m_pSecondaryScoreKeeper->HandleTapRowScore( m_NoteData, row );

	const int iCurCombo = m_pPlayerStageStats ? m_pPlayerStageStats->m_iCurCombo : 0;
	const int iCurMissCombo = m_pPlayerStageStats ? m_pPlayerStageStats->m_iCurMissCombo : 0;

	if( iOldCombo > 50 && iCurCombo < 50 )
		SCREENMAN->PostMessageToTopScreen( SM_ComboStopped, 0 );

	SendComboMessage( iOldCombo, iOldMissCombo );

	if( m_pPlayerStageStats && m_pCombo )
	{
		m_pCombo->SetCombo( iCurCombo, iCurMissCombo );
	}

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
		m_pPlayerStageStats->m_iMaxCombo = max(m_pPlayerStageStats->m_iMaxCombo, iCurCombo);

	/*
	 * Use the real current beat, not the beat we've been passed.  That's because we
	 * want to record the current life/combo to the current time; eg. if it's a MISS,
	 * the beat we're registering is in the past, but the life is changing now.
	 *
	 * We need to include time from previous songs in a course, so we can't use
	 * GAMESTATE->m_fMusicSeconds.  Use fStepsSeconds instead.
	 */
	if( m_pPlayerStageStats )
		m_pPlayerStageStats->UpdateComboList( STATSMAN->m_CurStageStats.m_fStepsSeconds, false );

	if( m_pScoreDisplay )
	{
		if( m_pPlayerStageStats )
			m_pScoreDisplay->SetScore( m_pPlayerStageStats->m_iScore );
		m_pScoreDisplay->OnJudgment( scoreOfLastTap );
	}
	if( m_pSecondaryScoreDisplay )
	{
		if( m_pPlayerStageStats )
			m_pSecondaryScoreDisplay->SetScore( m_pPlayerStageStats->m_iScore );
		m_pSecondaryScoreDisplay->OnJudgment( scoreOfLastTap );
	}

	ChangeLife( scoreOfLastTap );
}


void Player::HandleHoldScore( const TapNote &tn )
{
	HoldNoteScore holdScore = tn.HoldResult.hns;
	TapNoteScore tapScore = tn.result.tns;
	bool NoCheating = true;
#ifdef DEBUG
	NoCheating = false;
#endif

	if( GAMESTATE->m_bDemonstrationOrJukebox )
		NoCheating = false;
	// don't accumulate points if AutoPlay is on.
	if( NoCheating && m_pPlayerState->m_PlayerController == PC_AUTOPLAY )
		return;

	if( m_pPrimaryScoreKeeper )
		m_pPrimaryScoreKeeper->HandleHoldScore( tn );
	if( m_pSecondaryScoreKeeper )
		m_pSecondaryScoreKeeper->HandleHoldScore( tn );

	if( m_pScoreDisplay )
	{
		if( m_pPlayerStageStats ) 
			m_pScoreDisplay->SetScore( m_pPlayerStageStats->m_iScore );
		m_pScoreDisplay->OnJudgment( holdScore, tapScore );
	}
	if( m_pSecondaryScoreDisplay )
	{
		if( m_pPlayerStageStats ) 
			m_pSecondaryScoreDisplay->SetScore( m_pPlayerStageStats->m_iScore );
		m_pSecondaryScoreDisplay->OnJudgment( holdScore, tapScore );
	}

	ChangeLife( holdScore, tapScore );
}

float Player::GetMaxStepDistanceSeconds()
{
	return GAMESTATE->m_SongOptions.GetCurrent().m_fMusicRate * GetWindowSeconds(TW_W5);
}

void Player::FadeToFail()
{
	if( m_pNoteField )
		m_pNoteField->FadeToFail();

	// clear miss combo
	if( m_pCombo )
		m_pCombo->SetCombo( 0, 0 );
}

void Player::CacheAllUsedNoteSkins()
{
	if( m_pNoteField )
		m_pNoteField->CacheAllUsedNoteSkins();
}

bool Player::IsPlayingBeginner() const
{
	if( m_pPlayerStageStats != NULL && !m_pPlayerStageStats->m_vpPossibleSteps.empty() )
	{
		Steps *pSteps = m_pPlayerStageStats->m_vpPossibleSteps[0];
		return pSteps->GetDifficulty() == DIFFICULTY_BEGINNER;
	}

	if( m_pPlayerState == NULL )
		return false;

	if( m_pPlayerState->m_PlayerNumber == PLAYER_INVALID )
		return false;
	Steps *pSteps = GAMESTATE->m_pCurSteps[ m_pPlayerState->m_PlayerNumber ];
	return pSteps && pSteps->GetDifficulty() == DIFFICULTY_BEGINNER;
}

void Player::SetJudgment( TapNoteScore tns, bool bEarly )
{
	if( m_pPlayerStageStats )
		m_pPlayerStageStats->m_tnsLast = tns;
	if( m_pPlayerState->m_mp != MultiPlayer_Invalid )
		MESSAGEMAN->Broadcast( enum_add2(Message_ShowJudgmentMuliPlayerP1,m_pPlayerState->m_mp) );

	if( m_pJudgment )
		m_pJudgment->SetJudgment( tns, bEarly );
}

/*
 * (c) 2001-2006 Chris Danford, Steve Checkoway
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
