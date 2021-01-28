#include "global.h"
#include "ScreenGameplay.h"
#include "SongManager.h"
#include "ScreenManager.h"
#include "GameConstantsAndTypes.h"
#include "PrefsManager.h"
#include "GamePreferences.h"
#include "GameManager.h"
#include "RageFileManager.h"
#include "Steps.h"
#include "RageLog.h"
#include "LifeMeter.h"
#include "LifeMeterBar.h"
#include "GameState.h"
#include "ScoreDisplayNormal.h"
#include "ScoreDisplayPercentage.h"
#include "ScoreDisplayLifeTime.h"
#include "ScoreDisplayOni.h"
#include "ScoreDisplayRave.h"
#include "ThemeManager.h"
#include "RageTimer.h"
#include "ScoreKeeperNormal.h"
#include "ScoreKeeperRave.h"
#include "LyricsLoader.h"
#include "ActorUtil.h"
#include "ArrowEffects.h"
#include "RageSoundManager.h"
#include "RageSoundReader.h"
#include "RageTextureManager.h"
#include "GameSoundManager.h"
#include "CombinedLifeMeterTug.h"
#include "Inventory.h"
#include "Course.h"
#include "NoteDataUtil.h"
#include "UnlockManager.h"
#include "LightsManager.h"
#include "ProfileManager.h"
#include "StatsManager.h"
#include "PlayerAI.h" // for NUM_SKILL_LEVELS
#include "NetworkSyncManager.h"
#include "DancingCharacters.h"
#include "ScreenDimensions.h"
#include "ThemeMetric.h"
#include "PlayerState.h"
#include "Style.h"
#include "LuaManager.h"
#include "MemoryCardManager.h"
#include "CommonMetrics.h"
#include "InputMapper.h"
#include "Game.h"
#include "ActiveAttackList.h"
#include "Player.h"
#include "StepsDisplay.h"
#include "XmlFile.h"
#include "Background.h"
#include "Foreground.h"
#include "ScreenSaveSync.h"
#include "AdjustSync.h"
#include "SongUtil.h"
#include "Song.h"
#include "XmlFileUtil.h"
#include "Profile.h" // for replay data stuff
#include "RageDisplay.h"

// Defines
#define SHOW_LIFE_METER_FOR_DISABLED_PLAYERS	THEME->GetMetricB(m_sName,"ShowLifeMeterForDisabledPlayers")
#define SHOW_SCORE_IN_RAVE			THEME->GetMetricB(m_sName,"ShowScoreInRave")
#define SONG_POSITION_METER_WIDTH		THEME->GetMetricF(m_sName,"SongPositionMeterWidth")
#define STOP_COURSE_EARLY			THEME->GetMetricB(m_sName,"StopCourseEarly")	// evaluate this every time it's used

static ThemeMetric<float> INITIAL_BACKGROUND_BRIGHTNESS	("ScreenGameplay","InitialBackgroundBrightness");
static ThemeMetric<float> SECONDS_BETWEEN_COMMENTS	("ScreenGameplay","SecondsBetweenComments");
static ThemeMetric<RString> SCORE_KEEPER_CLASS		("ScreenGameplay","ScoreKeeperClass");
static ThemeMetric<bool> FORCE_IMMEDIATE_FAIL_FOR_BATTERY("ScreenGameplay", "ForceImmediateFailForBattery");

AutoScreenMessage( SM_PlayGo );

// received while STATE_DANCING
AutoScreenMessage( SM_LoadNextSong );
AutoScreenMessage( SM_StartLoadingNextSong );

// received while STATE_OUTRO
AutoScreenMessage( SM_DoPrevScreen );
AutoScreenMessage( SM_DoNextScreen );

// received while STATE_INTRO
AutoScreenMessage( SM_StartHereWeGo );
AutoScreenMessage( SM_StopHereWeGo );

AutoScreenMessage( SM_BattleTrickLevel1 );
AutoScreenMessage( SM_BattleTrickLevel2 );
AutoScreenMessage( SM_BattleTrickLevel3 );

static Preference<bool> g_bCenter1Player( "Center1Player", false );
static Preference<bool> g_bShowLyrics( "ShowLyrics", true );
static Preference<float> g_fNetStartOffset( "NetworkStartOffset", -3.0 );
static Preference<bool> g_bEasterEggs( "EasterEggs", true );


PlayerInfo::PlayerInfo(): m_pn(PLAYER_INVALID), m_mp(MultiPlayer_Invalid),
	m_bIsDummy(false), m_iDummyIndex(0), m_iAddToDifficulty(0),
	m_bPlayerEnabled(false), m_PlayerStateDummy(),
	m_PlayerStageStatsDummy(), m_SoundEffectControl(),
	m_vpStepsQueue(), m_asModifiersQueue(), m_pLifeMeter(nullptr),
	m_ptextCourseSongNumber(nullptr), m_ptextStepsDescription(nullptr),
	m_pPrimaryScoreDisplay(nullptr), m_pSecondaryScoreDisplay(nullptr),
	m_pPrimaryScoreKeeper(nullptr), m_pSecondaryScoreKeeper(nullptr),
	m_ptextPlayerOptions(nullptr), m_pActiveAttackList(nullptr),
	m_NoteData(), m_pPlayer(nullptr), m_pInventory(nullptr),
	m_pStepsDisplay(nullptr), m_sprOniGameOver() {}

void PlayerInfo::Load( PlayerNumber pn, MultiPlayer mp, bool bShowNoteField, int iAddToDifficulty )
{
	m_pn = pn;
	m_mp = mp;
	m_bPlayerEnabled = IsEnabled();
	m_bIsDummy = false;
	m_iAddToDifficulty = iAddToDifficulty;
	m_pLifeMeter = nullptr;
	m_ptextCourseSongNumber = nullptr;
	m_ptextStepsDescription = nullptr;

	if( !IsMultiPlayer() )
	{
		PlayMode mode = GAMESTATE->m_PlayMode;
		switch( mode )
		{
		case PLAY_MODE_REGULAR:
		case PLAY_MODE_NONSTOP:
		case PLAY_MODE_BATTLE:
		case PLAY_MODE_RAVE:
			if( PREFSMAN->m_bPercentageScoring )
				m_pPrimaryScoreDisplay = new ScoreDisplayPercentage;
			else
				m_pPrimaryScoreDisplay = new ScoreDisplayNormal;
			break;
		case PLAY_MODE_ONI:
		case PLAY_MODE_ENDLESS:
			if( GAMESTATE->m_pPlayerState[pn]->m_PlayerOptions.GetStage().m_LifeType == LifeType_Time )
				m_pPrimaryScoreDisplay = new ScoreDisplayLifeTime;
			else
				m_pPrimaryScoreDisplay = new ScoreDisplayOni;
			break;
		default:
			FAIL_M(ssprintf("Invalid PlayMode: %i", mode));
		}
	}

	PlayerState *const pPlayerState = GetPlayerState();
	PlayerStageStats *const pPlayerStageStats = GetPlayerStageStats();

	if( m_pPrimaryScoreDisplay )
		m_pPrimaryScoreDisplay->Init( pPlayerState, pPlayerStageStats );

	switch( GAMESTATE->m_PlayMode )
	{
		case PLAY_MODE_RAVE:
			m_pSecondaryScoreDisplay = new ScoreDisplayRave;
			m_pSecondaryScoreDisplay->SetName( "ScoreDisplayRave" );
		default:
			break;
	}

	if( m_pSecondaryScoreDisplay )
		m_pSecondaryScoreDisplay->Init( pPlayerState, pPlayerStageStats );

	m_pPrimaryScoreKeeper = ScoreKeeper::MakeScoreKeeper( SCORE_KEEPER_CLASS, pPlayerState, pPlayerStageStats );

	switch( GAMESTATE->m_PlayMode )
	{
		case PLAY_MODE_RAVE:
			m_pSecondaryScoreKeeper = new ScoreKeeperRave( pPlayerState, pPlayerStageStats );
		default:
			break;
	}

	m_ptextPlayerOptions = nullptr;
	m_pActiveAttackList = nullptr;
	m_pPlayer = new Player( m_NoteData, bShowNoteField );
	m_pInventory = nullptr;
	m_pStepsDisplay = nullptr;

	if( IsMultiPlayer() )
	{
		pPlayerState->m_PlayerOptions	= GAMESTATE->m_pPlayerState[PLAYER_1]->m_PlayerOptions;
	}
}

void PlayerInfo::LoadDummyP1( int iDummyIndex, int iAddToDifficulty )
{
	m_pn = PLAYER_1;
	m_bPlayerEnabled = IsEnabled();
	m_bIsDummy = true;
	m_iDummyIndex = iDummyIndex;
	m_iAddToDifficulty = iAddToDifficulty;

	// don't init any of the scoring objects
	m_pPlayer = new Player( m_NoteData, true );

	// PlayerOptions needs to be set now so that we load the correct NoteSkin.
	m_PlayerStateDummy = *GAMESTATE->m_pPlayerState[PLAYER_1];
}

PlayerInfo::~PlayerInfo()
{
	SAFE_DELETE( m_pLifeMeter );
	SAFE_DELETE( m_ptextCourseSongNumber );
	SAFE_DELETE( m_ptextStepsDescription );
	SAFE_DELETE( m_pPrimaryScoreDisplay );
	SAFE_DELETE( m_pSecondaryScoreDisplay );
	SAFE_DELETE( m_pPrimaryScoreKeeper );
	SAFE_DELETE( m_pSecondaryScoreKeeper );
	SAFE_DELETE( m_ptextPlayerOptions );
	SAFE_DELETE( m_pActiveAttackList );
	SAFE_DELETE( m_pPlayer );
	SAFE_DELETE( m_pInventory );
	SAFE_DELETE( m_pStepsDisplay );
}

void PlayerInfo::ShowOniGameOver()
{
	m_sprOniGameOver->PlayCommand( "Die" );
}

PlayerState *PlayerInfo::GetPlayerState()
{
	if( m_bIsDummy )
		return &m_PlayerStateDummy;
	return IsMultiPlayer() ?
		GAMESTATE->m_pMultiPlayerState[ GetPlayerStateAndStageStatsIndex() ] :
		GAMESTATE->m_pPlayerState[ GetPlayerStateAndStageStatsIndex() ];
}

PlayerStageStats *PlayerInfo::GetPlayerStageStats()
{
	// multiplayer chooses the PlayerStageStats with the highest score on StageFinalized
	if( m_bIsDummy  ||  IsMultiPlayer() )
		return &m_PlayerStageStatsDummy;
	return &STATSMAN->m_CurStageStats.m_player[ GetPlayerStateAndStageStatsIndex() ];
}

bool PlayerInfo::IsEnabled()
{
	if( m_pn != PLAYER_INVALID )
		return GAMESTATE->IsPlayerEnabled( m_pn );
	if( m_mp != MultiPlayer_Invalid )
		return GAMESTATE->IsMultiPlayerEnabled( m_mp );
	else if( m_bIsDummy )
		return true;
	FAIL_M("Invalid non-dummy player.");
}

vector<PlayerInfo>::iterator
GetNextEnabledPlayerInfo( vector<PlayerInfo>::iterator iter, vector<PlayerInfo> &v )
{
	for( ; iter != v.end(); ++iter )
	{
		if( !iter->m_bPlayerEnabled )
			continue;
		return iter;
	}
	return iter;
}

vector<PlayerInfo>::iterator
GetNextEnabledPlayerInfoNotDummy( vector<PlayerInfo>::iterator iter, vector<PlayerInfo> &v )
{
	for( ; iter != v.end(); iter++ )
	{
		if( iter->m_bIsDummy )
			continue;
		if( !iter->m_bPlayerEnabled )
			continue;
		return iter;
	}
	return iter;
}

vector<PlayerInfo>::iterator
GetNextEnabledPlayerNumberInfo( vector<PlayerInfo>::iterator iter, vector<PlayerInfo> &v )
{
	for( ; iter != v.end(); ++iter )
	{
		if( iter->m_bIsDummy )
			continue;
		if( !iter->m_bPlayerEnabled )
			continue;
		if( iter->m_mp != MultiPlayer_Invalid )
			continue;
		return iter;
	}
	return iter;
}

vector<PlayerInfo>::iterator
GetNextPlayerNumberInfo( vector<PlayerInfo>::iterator iter, vector<PlayerInfo> &v )
{
	for( ; iter != v.end(); ++iter )
	{
		if( iter->m_bIsDummy )
			continue;
		if( iter->m_pn == PLAYER_INVALID )
			continue;
		return iter;
	}
	return iter;
}

vector<PlayerInfo>::iterator
GetNextVisiblePlayerInfo( vector<PlayerInfo>::iterator iter, vector<PlayerInfo> &v )
{
	for( ; iter != v.end(); ++iter )
	{
		if( !iter->m_pPlayer->HasVisibleParts() )
			continue;
		return iter;
	}
	return iter;
}

////////////////////////////////////////////////////////////////////////////////

ScreenGameplay::ScreenGameplay()
{
	m_pSongBackground = nullptr;
	m_pSongForeground = nullptr;
	m_bForceNoNetwork = false;
	m_delaying_ready_announce= false;
	GAMESTATE->m_AdjustTokensBySongCostForFinalStageCheck= false;
}

void ScreenGameplay::Init()
{
	SubscribeToMessage( "Judgment" );

	PLAYER_TYPE.Load(			m_sName, "PlayerType" );
	PLAYER_INIT_COMMAND.Load(		m_sName, "PlayerInitCommand" );
	GIVE_UP_START_TEXT.Load(		m_sName, "GiveUpStartText" );
	GIVE_UP_BACK_TEXT.Load(			m_sName, "GiveUpBackText" );
	GIVE_UP_ABORTED_TEXT.Load(		m_sName, "GiveUpAbortedText" );
	SKIP_SONG_TEXT.Load(m_sName, "SkipSongText");
	GIVE_UP_SECONDS.Load(			m_sName, "GiveUpSeconds" );
	MUSIC_FADE_OUT_SECONDS.Load(		m_sName, "MusicFadeOutSeconds" );
	OUT_TRANSITION_LENGTH.Load(		m_sName, "OutTransitionLength" );
	COURSE_TRANSITION_LENGTH.Load(		m_sName, "CourseTransitionLength" );
	BEGIN_FAILED_DELAY.Load(		m_sName, "BeginFailedDelay" );
	MIN_SECONDS_TO_STEP.Load(		m_sName, "MinSecondsToStep" );
	MIN_SECONDS_TO_MUSIC.Load(		m_sName, "MinSecondsToMusic" );
	MIN_SECONDS_TO_STEP_NEXT_SONG.Load(	m_sName, "MinSecondsToStepNextSong" );
	START_GIVES_UP.Load(			m_sName, "StartGivesUp" );
	BACK_GIVES_UP.Load(			m_sName, "BackGivesUp" );
	SELECT_SKIPS_SONG.Load(m_sName, "SelectSkipsSong");
	GIVING_UP_GOES_TO_PREV_SCREEN.Load(	m_sName, "GivingUpGoesToPrevScreen" );
	FAIL_ON_MISS_COMBO.Load(		m_sName, "FailOnMissCombo" );
	ALLOW_CENTER_1_PLAYER.Load(		m_sName, "AllowCenter1Player" );
	// configurable:
	UNPAUSE_WITH_START.Load(		m_sName, "UnpauseWithStart");
	SURVIVAL_MOD_OVERRIDE.Load(m_sName, "SurvivalModOverride");

	// Default values.  The theme can set its own through the Lua interface.
	m_HasteTurningPoints.clear();
	m_HasteTurningPoints.push_back(-1);
	m_HasteTurningPoints.push_back(0);
	m_HasteTurningPoints.push_back(0.3f);
	m_HasteTurningPoints.push_back(1);
	m_HasteAddAmounts.clear();
	m_HasteAddAmounts.push_back(-0.5f);
	m_HasteAddAmounts.push_back(0);
	m_HasteAddAmounts.push_back(0.2f);
	m_HasteAddAmounts.push_back(0.5f);
	m_fHasteTimeBetweenUpdates= 4;
	m_fHasteLifeSwitchPoint= 0.5f;
	m_fCurrHasteRate= 1; // Should this be in BeginSong?  Not sure whether it should carry over between songs.

	if( UseSongBackgroundAndForeground() )
	{
		m_pSongBackground = new Background;
		m_pSongForeground = new Foreground;
	}

	ScreenWithMenuElements::Init();

	this->FillPlayerInfo( m_vPlayerInfo );

	{
		ASSERT_M( !m_vPlayerInfo.empty(), "m_vPlayerInfo must be filled by FillPlayerInfo" );

		int iNumEnabledPlayers = 0;
		FOREACH_EnabledPlayerInfo( m_vPlayerInfo, pi )
			++iNumEnabledPlayers;

		/* If this is 0, we have no active players and havn't been initialized correctly. */
		ASSERT( iNumEnabledPlayers > 0 );
	}

	/* Pause MEMCARDMAN.  If a memory card is removed, we don't want to interrupt the
	 * player by making a noise until the game finishes. */
	if( !GAMESTATE->m_bDemonstrationOrJukebox )
		MEMCARDMAN->PauseMountingThread();

	m_pSoundMusic = nullptr;
	set_paused_internal(false);

	m_pCombinedLifeMeter = nullptr;

	if( GAMESTATE->m_pCurSong == nullptr && GAMESTATE->m_pCurCourse == nullptr )
		return;	// ScreenDemonstration will move us to the next screen.  We just need to survive for one update without crashing.

	/* Save settings to the profile now.  Don't do this on extra stages, since the
	 * user doesn't have full control; saving would force profiles to Difficulty_Hard
	 * and save over their default modifiers every time someone got an extra stage.
	 * Do this before course modifiers are set up. */
	if( !GAMESTATE->IsAnExtraStage() )
	{
		FOREACH_HumanPlayer( pn )
			GAMESTATE->SaveCurrentSettingsToProfile(pn);
	}

	/* Called once per stage (single song or single course). */
	GAMESTATE->BeginStage();

	int player = 1;
	FOREACH_EnabledPlayerInfo( m_vPlayerInfo, pi )
	{
		unsigned int count = pi->m_vpStepsQueue.size();

		for (unsigned int i = 0; i < count; i++)
		{
			Steps *curSteps = pi->m_vpStepsQueue[i];
			if (curSteps->IsNoteDataEmpty())
			{
				if (curSteps->GetNoteDataFromSimfile())
				{
					LOG->Trace("Notes should be loaded for player %d", player);
				}
				else
				{
					LOG->Trace("Error loading notes for player %d", player);
				}
			}
		}
		player++;
	}

	if(!GAMESTATE->IsCourseMode() && !GAMESTATE->m_bDemonstrationOrJukebox)
	{
		// fill in difficulty of CPU players with that of the first human player
		// this should not need to worry about step content.
		FOREACH_PotentialCpuPlayer(p)
		{
			PlayerNumber human_pn= GAMESTATE->GetFirstHumanPlayer();
			GAMESTATE->m_pCurSteps[p].Set( GAMESTATE->m_pCurSteps[human_pn] );
			if(GAMESTATE->GetCurrentGame()->m_PlayersHaveSeparateStyles)
			{
				GAMESTATE->SetCurrentStyle(GAMESTATE->GetCurrentStyle(human_pn), p);
			}
		}

		FOREACH_EnabledPlayer(p)
			ASSERT( GAMESTATE->m_pCurSteps[p].Get() != nullptr );
	}

	/* Increment the course play count. */
	if( GAMESTATE->IsCourseMode() && !GAMESTATE->m_bDemonstrationOrJukebox )
		FOREACH_EnabledPlayer(p)
			PROFILEMAN->IncrementCoursePlayCount( GAMESTATE->m_pCurCourse, GAMESTATE->m_pCurTrail[p], p );

	STATSMAN->m_CurStageStats.m_Stage = GAMESTATE->GetCurrentStage();
	STATSMAN->m_CurStageStats.m_iStageIndex = GAMESTATE->m_iCurrentStageIndex;
	STATSMAN->m_CurStageStats.m_playMode = GAMESTATE->m_PlayMode;
	FOREACH_PlayerNumber(pn)
	{
		STATSMAN->m_CurStageStats.m_player[pn].m_pStyle= GAMESTATE->GetCurrentStyle(pn);
	}
	FOREACH_MultiPlayer(pn)
	{
		STATSMAN->m_CurStageStats.m_multiPlayer[pn].m_pStyle= GAMESTATE->GetCurrentStyle(PLAYER_INVALID);
	}

	/* Record combo rollover. */
	FOREACH_EnabledPlayerInfoNotDummy( m_vPlayerInfo, pi )
		pi->GetPlayerStageStats()->UpdateComboList( 0, true );

	m_DancingState = STATE_INTRO;

	// Set this in LoadNextSong()
	//m_fTimeLeftBeforeDancingComment = SECONDS_BETWEEN_COMMENTS;

	m_bZeroDeltaOnNextUpdate = false;


	if( m_pSongBackground )
	{
		m_pSongBackground->SetName( "SongBackground" );
		m_pSongBackground->SetDrawOrder( DRAW_ORDER_BEFORE_EVERYTHING );
		ActorUtil::LoadAllCommands( *m_pSongBackground, m_sName );
		this->AddChild( m_pSongBackground );
	}

	if( m_pSongForeground )
	{
		m_pSongForeground->SetName( "SongForeground" );
		m_pSongForeground->SetDrawOrder( DRAW_ORDER_OVERLAY+1 );	// on top of the overlay, but under transitions
		ActorUtil::LoadAllCommands( *m_pSongForeground, m_sName );
		this->AddChild( m_pSongForeground );
	}

	if( PREFSMAN->m_bShowBeginnerHelper )
	{
		m_BeginnerHelper.SetDrawOrder( DRAW_ORDER_BEFORE_EVERYTHING );
		m_BeginnerHelper.SetXY( SCREEN_CENTER_X, SCREEN_CENTER_Y );
		this->AddChild( &m_BeginnerHelper );
	}

	if( !GAMESTATE->m_bDemonstrationOrJukebox )	// only load if we're going to use it
	{
		m_Toasty.Load( THEME->GetPathB(m_sName,"toasty") );
		this->AddChild( &m_Toasty );
	}

	// Use the margin function to calculate where the notefields should be and
	// what size to zoom them to.  This way, themes get margins to put cut-ins
	// in, and the engine can have players on different styles without the
	// notefields overlapping. -Kyz
	LuaReference margarine;
	float margins[NUM_PLAYERS][2];
	FOREACH_PlayerNumber(pn)
	{
		margins[pn][0]= 40;
		margins[pn][1]= 40;
	}
	THEME->GetMetric(m_sName, "MarginFunction", margarine);
	if(margarine.GetLuaType() != LUA_TFUNCTION)
	{
		LuaHelpers::ReportScriptErrorFmt("MarginFunction metric for %s must be a function.", m_sName.c_str());
	}
	else
	{
		Lua* L= LUA->Get();
		margarine.PushSelf(L);
		lua_createtable(L, 0, 0);
		int next_player_slot= 1;
		FOREACH_EnabledPlayer(pn)
		{
			Enum::Push(L, pn);
			lua_rawseti(L, -2, next_player_slot);
			++next_player_slot;
		}
		Enum::Push(L, GAMESTATE->GetCurrentStyle(PLAYER_INVALID)->m_StyleType);
		RString err= "Error running MarginFunction:  ";
		if(LuaHelpers::RunScriptOnStack(L, err, 2, 3, true))
		{
			RString marge= "Margin value must be a number.";
			margins[PLAYER_1][0]= SafeFArg(L, -3, marge, 40);
			float center= SafeFArg(L, -2, marge, 80);
			margins[PLAYER_1][1]= center / 2.0f;
			margins[PLAYER_2][0]= center / 2.0f;
			margins[PLAYER_2][1]= SafeFArg(L, -1, marge, 40);
		}
		lua_settop(L, 0);
		LUA->Release(L);
	}

	float left_edge[NUM_PLAYERS]= {0.0f, SCREEN_WIDTH / 2.0f};
	FOREACH_EnabledPlayerInfo( m_vPlayerInfo, pi )
	{
		RString sName = ssprintf("Player%s", pi->GetName().c_str());
		pi->m_pPlayer->SetName( sName );

		Style const* style= GAMESTATE->GetCurrentStyle(pi->m_pn);
		float style_width= style->GetWidth(pi->m_pn);
		float edge= left_edge[pi->m_pn];
		float screen_space;
		float field_space;
		float left_marge;
		float right_marge;
#define CENTER_PLAYER_BLOCK \
		{ \
			edge= 0.0f; \
			screen_space= SCREEN_WIDTH; \
			left_marge= margins[PLAYER_1][0]; \
			right_marge= margins[PLAYER_2][1]; \
			field_space= screen_space - left_marge - right_marge; \
		}
		// If pi->m_pn is set, then the player will be visible.  If not, then it's not
		// visible and don't bother setting its position.
		if(GAMESTATE->m_bMultiplayer && !pi->m_bIsDummy)
		CENTER_PLAYER_BLOCK
		else
		{
			screen_space= SCREEN_WIDTH / 2.0f;
			left_marge= margins[pi->m_pn][0];
			right_marge= margins[pi->m_pn][1];
			field_space= screen_space - left_marge - right_marge;
			if(Center1Player() ||
				style->m_StyleType == StyleType_TwoPlayersSharedSides ||
				(style_width > field_space && GAMESTATE->GetNumPlayersEnabled() == 1
					&& (bool)ALLOW_CENTER_1_PLAYER))
			CENTER_PLAYER_BLOCK
		}
#undef CENTER_PLAYER_BLOCK
		float player_x= edge + left_marge + (field_space / 2.0f);
		float field_zoom= field_space / style_width;
		/*
		LuaHelpers::ReportScriptErrorFmt("Positioning player %d at %.0f:  "
			"screen_space %.0f, left_edge %.0f, field_space %.0f, left_marge %.0f,"
			" right_marge %.0f, style_width %.0f, field_zoom %.2f.",
			pi->m_pn+1, player_x, screen_space, left_edge[pi->m_pn], field_space,
			left_marge, right_marge, style_width, field_zoom);
		*/
		pi->GetPlayerState()->m_NotefieldZoom= min(1.0f, field_zoom);

		pi->m_pPlayer->SetX(player_x);
		pi->m_pPlayer->RunCommands( PLAYER_INIT_COMMAND );
		//ActorUtil::LoadAllCommands(pi->m_pPlayer, m_sName);
		this->AddChild( pi->m_pPlayer );
		pi->m_pPlayer->PlayCommand( "On" );
	}

	FOREACH_EnabledPlayerInfoNotDummy( m_vPlayerInfo, pi )
	{
		if( pi->m_pPlayer->HasVisibleParts() )
		{
			pi->m_sprOniGameOver.Load( THEME->GetPathG(m_sName,"oni gameover") );
			pi->m_sprOniGameOver->SetName( ssprintf("OniGameOver%s",pi->GetName().c_str()) );
			LOAD_ALL_COMMANDS_AND_SET_XY( pi->m_sprOniGameOver );
			this->AddChild( pi->m_sprOniGameOver );
		}
	}

	m_NextSong.Load( THEME->GetPathB(m_sName,"next course song") );
	m_NextSong.SetDrawOrder( DRAW_ORDER_TRANSITIONS-1 );
	this->AddChild( &m_NextSong );

	// Add combined life meter
	switch( GAMESTATE->m_PlayMode )
	{
		case PLAY_MODE_BATTLE:
		case PLAY_MODE_RAVE:
			m_pCombinedLifeMeter = new CombinedLifeMeterTug;
			m_pCombinedLifeMeter->SetName( "CombinedLife" );
			LOAD_ALL_COMMANDS_AND_SET_XY( *m_pCombinedLifeMeter );
			this->AddChild( m_pCombinedLifeMeter );
		default:
			break;
	}

	// Before the lifemeter loads, if Networking is required
	// we need to wait, so that there is no Dead On Start issues.
	// if you wait too long at the second checkpoint, you will
	// appear dead when you begin your game.
	if( !m_bForceNoNetwork )
		NSMAN->StartRequest(0);


	// Add individual life meter
	switch( GAMESTATE->m_PlayMode )
	{
		case PLAY_MODE_REGULAR:
		case PLAY_MODE_ONI:
		case PLAY_MODE_NONSTOP:
		case PLAY_MODE_ENDLESS:
			FOREACH_PlayerNumberInfo( m_vPlayerInfo, pi )
			{
				if( !GAMESTATE->IsPlayerEnabled(pi->m_pn) && !SHOW_LIFE_METER_FOR_DISABLED_PLAYERS )
					continue;	// skip

				pi->m_pLifeMeter = LifeMeter::MakeLifeMeter( pi->GetPlayerState()->m_PlayerOptions.GetStage().m_LifeType );
				pi->m_pLifeMeter->Load( pi->GetPlayerState(), pi->GetPlayerStageStats() );
				pi->m_pLifeMeter->SetName( ssprintf("Life%s",pi->GetName().c_str()) );
				LOAD_ALL_COMMANDS_AND_SET_XY( pi->m_pLifeMeter );
				this->AddChild( pi->m_pLifeMeter );

				// HACK: When SHOW_LIFE_METER_FOR_DISABLED_PLAYERS is enabled,
				// we don't want to have any life in the disabled player's life
				// meter. I think this only happens with LifeMeterBars, but I'm
				// not 100% sure of that. -freem
				if( !GAMESTATE->IsPlayerEnabled(pi->m_pn) && SHOW_LIFE_METER_FOR_DISABLED_PLAYERS )
				{
					if(pi->GetPlayerState()->m_PlayerOptions.GetStage().m_LifeType == LifeType_Bar)
						static_cast<LifeMeterBar*>(pi->m_pLifeMeter)->ChangeLife(-1.0f);
				}
			}
			break;
		case PLAY_MODE_BATTLE:
		case PLAY_MODE_RAVE:
		default:
			break;
	}

	m_bShowScoreboard = false;

#if !defined(WITHOUT_NETWORKING)
	// Only used in SMLAN/SMOnline:
	if( !m_bForceNoNetwork && NSMAN->useSMserver && GAMESTATE->GetCurrentStyle(PLAYER_INVALID)->m_StyleType != StyleType_OnePlayerTwoSides )
	{
		m_bShowScoreboard = PREFSMAN->m_bEnableScoreboard.Get();
		PlayerNumber pn = GAMESTATE->GetFirstDisabledPlayer();
		if( pn != PLAYER_INVALID && m_bShowScoreboard )
		{
			FOREACH_NSScoreBoardColumn( col )
			{
				m_Scoreboard[col].LoadFromFont( THEME->GetPathF(m_sName,"scoreboard") );
				m_Scoreboard[col].SetShadowLength( 0 );
				m_Scoreboard[col].SetName( ssprintf("ScoreboardC%iP%i",col+1,pn+1) );
				LOAD_ALL_COMMANDS_AND_SET_XY( m_Scoreboard[col] );
				m_Scoreboard[col].SetText( NSMAN->m_Scoreboard[col] );
				m_Scoreboard[col].SetVertAlign( align_top );
				this->AddChild( &m_Scoreboard[col] );
			}
		}
	}
#endif

	FOREACH_EnabledPlayerInfo( m_vPlayerInfo, pi )
	{
		// primary score display
		if( pi->m_pPrimaryScoreDisplay )
		{
			pi->m_pPrimaryScoreDisplay->SetName( ssprintf("Score%s",pi->GetName().c_str()) );
			LOAD_ALL_COMMANDS_AND_SET_XY( pi->m_pPrimaryScoreDisplay );
			if( GAMESTATE->m_PlayMode != PLAY_MODE_RAVE || SHOW_SCORE_IN_RAVE ) /* XXX: ugly */
				this->AddChild( pi->m_pPrimaryScoreDisplay );
		}

		// secondary score display
		if( pi->m_pSecondaryScoreDisplay )
		{
			pi->m_pSecondaryScoreDisplay->SetName( ssprintf("SecondaryScore%s",pi->GetName().c_str()) );
			LOAD_ALL_COMMANDS_AND_SET_XY( pi->m_pSecondaryScoreDisplay );
			this->AddChild( pi->m_pSecondaryScoreDisplay );
		}
	}

	// Add stage / SongNumber
	FOREACH_EnabledPlayerNumberInfo( m_vPlayerInfo, pi )
	{
		if( GAMESTATE->IsCourseMode() )
		{
			ASSERT( pi->m_ptextCourseSongNumber == nullptr );
			SONG_NUMBER_FORMAT.Load( m_sName, "SongNumberFormat" );
			pi->m_ptextCourseSongNumber = new BitmapText;
			pi->m_ptextCourseSongNumber->LoadFromFont( THEME->GetPathF(m_sName,"SongNum") );
			pi->m_ptextCourseSongNumber->SetName( ssprintf("SongNumber%s",pi->GetName().c_str()) );
			LOAD_ALL_COMMANDS_AND_SET_XY( pi->m_ptextCourseSongNumber );
			pi->m_ptextCourseSongNumber->SetText( "" );
			//pi->m_ptextCourseSongNumber->SetDiffuse( RageColor(0,0.5f,1,1) );	// light blue
			this->AddChild( pi->m_ptextCourseSongNumber );
		}

		ASSERT( pi->m_ptextStepsDescription == nullptr );
		pi->m_ptextStepsDescription = new BitmapText;
		pi->m_ptextStepsDescription->LoadFromFont( THEME->GetPathF(m_sName,"StepsDescription") );
		pi->m_ptextStepsDescription->SetName( ssprintf("StepsDescription%s",pi->GetName().c_str()) );
		LOAD_ALL_COMMANDS_AND_SET_XY( pi->m_ptextStepsDescription );
		this->AddChild( pi->m_ptextStepsDescription );

		// Player/Song options
		ASSERT( pi->m_ptextPlayerOptions == nullptr );
		pi->m_ptextPlayerOptions = new BitmapText;
		pi->m_ptextPlayerOptions->LoadFromFont( THEME->GetPathF(m_sName,"player options") );
		pi->m_ptextPlayerOptions->SetName( ssprintf("PlayerOptions%s",pi->GetName().c_str()) );
		LOAD_ALL_COMMANDS_AND_SET_XY( pi->m_ptextPlayerOptions );
		this->AddChild( pi->m_ptextPlayerOptions );

		// Difficulty icon and meter
		ASSERT( pi->m_pStepsDisplay == nullptr );
		pi->m_pStepsDisplay = new StepsDisplay;
		pi->m_pStepsDisplay->Load("StepsDisplayGameplay", pi->GetPlayerState() );
		pi->m_pStepsDisplay->SetName( ssprintf("StepsDisplay%s",pi->GetName().c_str()) );
		PlayerNumber pn = pi->GetStepsAndTrailIndex();
		if( pn != PlayerNumber_Invalid )
			pi->m_pStepsDisplay->PlayCommand( "Set" + pi->GetName() );
		LOAD_ALL_COMMANDS_AND_SET_XY( pi->m_pStepsDisplay );
		this->AddChild( pi->m_pStepsDisplay );

/*
		switch( GAMESTATE->m_PlayMode )
		{
		case PLAY_MODE_BATTLE:
			pi->m_pInventory = new Inventory;
			pi->m_pInventory->Load( p );
			this->AddChild( pi->m_pInventory );
			break;
		}
*/
	}

	m_textSongOptions.LoadFromFont( THEME->GetPathF(m_sName,"song options") );
	m_textSongOptions.SetShadowLength( 0 );
	m_textSongOptions.SetName( "SongOptions" );
	LOAD_ALL_COMMANDS_AND_SET_XY( m_textSongOptions );
	m_textSongOptions.SetText( GAMESTATE->m_SongOptions.GetStage().GetLocalizedString() );
	this->AddChild( &m_textSongOptions );

	FOREACH_VisiblePlayerInfo( m_vPlayerInfo, pi )
	{
		ASSERT( pi->m_pActiveAttackList == nullptr );
		pi->m_pActiveAttackList = new ActiveAttackList;
		pi->m_pActiveAttackList->LoadFromFont( THEME->GetPathF(m_sName,"ActiveAttackList") );
		pi->m_pActiveAttackList->Init( pi->GetPlayerState() );
		pi->m_pActiveAttackList->SetName( ssprintf("ActiveAttackList%s",pi->GetName().c_str()) );
		LOAD_ALL_COMMANDS_AND_SET_XY( pi->m_pActiveAttackList );
		this->AddChild( pi->m_pActiveAttackList );
	}

	if( g_bShowLyrics )
	{
		m_LyricDisplay.SetName( "LyricDisplay" );
		LOAD_ALL_COMMANDS( m_LyricDisplay );
		this->AddChild( &m_LyricDisplay );
	}

	if( !GAMESTATE->m_bDemonstrationOrJukebox )	// only load if we're going to use it
	{
		m_Ready.Load( THEME->GetPathB(m_sName,"ready") );
		this->AddChild( &m_Ready );

		m_Go.Load( THEME->GetPathB(m_sName,"go") );
		this->AddChild( &m_Go );

		m_Failed.Load( THEME->GetPathB(m_sName,"failed") );
		m_Failed.SetDrawOrder( DRAW_ORDER_TRANSITIONS-1 ); // on top of everything else
		this->AddChild( &m_Failed );

		m_textDebug.LoadFromFont( THEME->GetPathF(m_sName,"debug") );
		m_textDebug.SetName( "Debug" );
		LOAD_ALL_COMMANDS_AND_SET_XY( m_textDebug );
		m_textDebug.SetDrawOrder( DRAW_ORDER_TRANSITIONS-1 );	// just under transitions, over the foreground
		this->AddChild( &m_textDebug );

		m_GameplayAssist.Init();

		if( GAMESTATE->IsAnExtraStage() )	// only load if we're going to use it
		{
			m_textSurviveTime.LoadFromFont( THEME->GetPathF(m_sName,"survive time") );
			m_textSurviveTime.SetShadowLength( 0 );
			m_textSurviveTime.SetName( "SurviveTime" );
			LOAD_ALL_COMMANDS_AND_SET_XY( m_textSurviveTime );
			m_textSurviveTime.SetDrawOrder( DRAW_ORDER_TRANSITIONS-1 );
			m_textSurviveTime.SetDiffuse( RageColor(1,1,1,0) );
			this->AddChild( &m_textSurviveTime );
		}

		switch( GAMESTATE->m_PlayMode )
		{
			case PLAY_MODE_BATTLE:
				m_soundBattleTrickLevel1.Load(	THEME->GetPathS(m_sName,"battle trick level1"), true );
				m_soundBattleTrickLevel2.Load(	THEME->GetPathS(m_sName,"battle trick level2"), true );
				m_soundBattleTrickLevel3.Load(	THEME->GetPathS(m_sName,"battle trick level3"), true );
			default:
				break;
		}
	}

	if( m_pSongBackground )
		m_pSongBackground->Init();

	FOREACH_EnabledPlayerInfo( m_vPlayerInfo, pi )
	{
		RString sType = PLAYER_TYPE;
		if( pi->m_bIsDummy )
			sType += "Dummy";
		pi->m_pPlayer->Init(
			sType,
			pi->GetPlayerState(),
			pi->GetPlayerStageStats(),
			pi->m_pLifeMeter,
			m_pCombinedLifeMeter,
			pi->m_pPrimaryScoreDisplay,
			pi->m_pSecondaryScoreDisplay,
			pi->m_pInventory,
			pi->m_pPrimaryScoreKeeper,
			pi->m_pSecondaryScoreKeeper );
	}

	// fill in m_apSongsQueue, m_vpStepsQueue, m_asModifiersQueue
	InitSongQueues();

	// Fill StageStats
	STATSMAN->m_CurStageStats.m_vpPossibleSongs = m_apSongsQueue;
	for (PlayerInfo &pi : m_vPlayerInfo)
	{
		if( pi.GetPlayerStageStats() )
			pi.GetPlayerStageStats()->m_vpPossibleSteps = pi.m_vpStepsQueue;
	}

	FOREACH_EnabledPlayerInfo( m_vPlayerInfo, pi )
	{
		ASSERT( !pi->m_vpStepsQueue.empty() );
		if( pi->GetPlayerStageStats() )
			pi->GetPlayerStageStats()->m_bJoined = true;
		if( pi->m_pPrimaryScoreKeeper )
			pi->m_pPrimaryScoreKeeper->Load( m_apSongsQueue, pi->m_vpStepsQueue, pi->m_asModifiersQueue );
		if( pi->m_pSecondaryScoreKeeper )
			pi->m_pSecondaryScoreKeeper->Load( m_apSongsQueue, pi->m_vpStepsQueue, pi->m_asModifiersQueue );
	}

	GAMESTATE->m_bGameplayLeadIn.Set( true );

	/* LoadNextSong first, since that positions some elements which need to be
	 * positioned before we TweenOnScreen. */
	LoadNextSong();

	m_GiveUpTimer.SetZero();
	m_SkipSongTimer.SetZero();
	m_gave_up= false;
	m_skipped_song= false;
}

bool ScreenGameplay::Center1Player() const
{
	/* Perhaps this should be handled better by defining a new
	 * StyleType for ONE_PLAYER_ONE_CREDIT_AND_ONE_COMPUTER,
	 * but for now just ignore Center1Player when it's Battle or Rave
	 * Mode. This doesn't begin to address two-player solo (6 arrows) */
	return g_bCenter1Player &&
		(bool)ALLOW_CENTER_1_PLAYER &&
		GAMESTATE->m_PlayMode != PLAY_MODE_BATTLE &&
		GAMESTATE->m_PlayMode != PLAY_MODE_RAVE &&
		GAMESTATE->GetCurrentStyle(PLAYER_INVALID)->m_StyleType == StyleType_OnePlayerOneSide;
}

// fill in m_apSongsQueue, m_vpStepsQueue, m_asModifiersQueue
void ScreenGameplay::InitSongQueues()
{
	if( GAMESTATE->IsCourseMode() )
	{
		Course* pCourse = GAMESTATE->m_pCurCourse;
		ASSERT( pCourse != nullptr );

		m_apSongsQueue.clear();
		PlayerNumber pnMaster = GAMESTATE->GetMasterPlayerNumber();
		Trail *pTrail = GAMESTATE->m_pCurTrail[pnMaster];
		ASSERT( pTrail != nullptr );
		for (TrailEntry const &e : pTrail->m_vEntries)
		{
			ASSERT( e.pSong != nullptr );
			m_apSongsQueue.push_back( e.pSong );
		}

		FOREACH_EnabledPlayerInfo( m_vPlayerInfo, pi )
		{
			Trail *lTrail = GAMESTATE->m_pCurTrail[ pi->GetStepsAndTrailIndex() ];
			ASSERT( lTrail != nullptr );

			pi->m_vpStepsQueue.clear();
			pi->m_asModifiersQueue.clear();
			for (TrailEntry const &e : lTrail->m_vEntries)
			{
				ASSERT( e.pSteps != nullptr );
				pi->m_vpStepsQueue.push_back( e.pSteps );
				AttackArray a;
				e.GetAttackArray( a );
				pi->m_asModifiersQueue.push_back( a );
			}

			// In a survival course, override stored mods
			if( pCourse->GetCourseType() == COURSE_TYPE_SURVIVAL && SURVIVAL_MOD_OVERRIDE )
			{
				pi->GetPlayerState()->m_PlayerOptions.FromString( ModsLevel_Stage,
										 "clearall,"
										 + CommonMetrics::DEFAULT_NOTESKIN_NAME.GetValue()
										 + ","
										 + CommonMetrics::DEFAULT_MODIFIERS.GetValue() );
				pi->GetPlayerState()->RebuildPlayerOptionsFromActiveAttacks();
			}
		}
	}
	else
	{
		m_apSongsQueue.push_back( GAMESTATE->m_pCurSong );
		FOREACH_EnabledPlayerInfo( m_vPlayerInfo, pi )
		{
			Steps *pSteps = GAMESTATE->m_pCurSteps[ pi->GetStepsAndTrailIndex() ];
			pi->m_vpStepsQueue.push_back( pSteps );
			const PlayerOptions &p = pi->GetPlayerState()->m_PlayerOptions.GetCurrent();

			if (p.m_fNoAttack == 0 && p.m_fRandAttack == 0 &&
			    pSteps->m_Attacks.size() > 0 )
			{
				pi->m_asModifiersQueue.push_back( pSteps->m_Attacks );
			}
			else
			{
				AttackArray aa;
				pi->m_asModifiersQueue.push_back( aa );
			}
		}
	}

	if( GAMESTATE->m_bMultiplayer )
	{
		for( int i=0; i<(int)m_apSongsQueue.size(); i++ )
		{
			Song *pSong = m_apSongsQueue[i];

			FOREACH_EnabledPlayerInfo( m_vPlayerInfo, pi )
			{
				Steps *pOldSteps = pi->m_vpStepsQueue[i];

				vector<Steps*> vpSteps;
				SongUtil::GetSteps( pSong, vpSteps, pOldSteps->m_StepsType );
				StepsUtil::SortNotesArrayByDifficulty( vpSteps );
				vector<Steps*>::iterator iter = find( vpSteps.begin(), vpSteps.end(), pOldSteps );
				int iIndexBase = 0;
				if( iter != vpSteps.end() )
				{
					iIndexBase = iter - vpSteps.begin();
					CLAMP( iIndexBase, 0, vpSteps.size() - GAMESTATE->m_iNumMultiplayerNoteFields );
				}

				int iIndexToUse = iIndexBase + pi->m_iAddToDifficulty;
				CLAMP( iIndexToUse, 0, vpSteps.size()-1 );

				Steps *pSteps = vpSteps[iIndexToUse];
				pi->m_vpStepsQueue[i] = pSteps;
			}
		}
	}
}

ScreenGameplay::~ScreenGameplay()
{
	GAMESTATE->m_AdjustTokensBySongCostForFinalStageCheck= true;
	if( this->IsFirstUpdate() )
	{
		/* We never received any updates. That means we were deleted without being
		 * used, and never actually played. (This can happen when backing out of
		 * ScreenStage.) Cancel the stage. */
		GAMESTATE->CancelStage();
	}

	LOG->Trace( "ScreenGameplay::~ScreenGameplay()" );

	SAFE_DELETE( m_pSongBackground );
	SAFE_DELETE( m_pSongForeground );

	if( !GAMESTATE->m_bDemonstrationOrJukebox )
		MEMCARDMAN->UnPauseMountingThread();

	SAFE_DELETE( m_pCombinedLifeMeter );
	if( m_pSoundMusic )
		m_pSoundMusic->StopPlaying();

	m_GameplayAssist.StopPlaying();

	if( !m_bForceNoNetwork )
		NSMAN->ReportSongOver();
}

bool ScreenGameplay::IsLastSong()
{
	if( GAMESTATE->m_pCurCourse )
	{
		if( GAMESTATE->m_pCurCourse->m_fGoalSeconds > 0  &&  GAMESTATE->m_bWorkoutGoalComplete )
			return true;
	}
	if( GAMESTATE->m_pCurCourse  &&  GAMESTATE->m_pCurCourse->m_bRepeat )
		return false;
	return GAMESTATE->GetCourseSongIndex() >= (int)m_apSongsQueue.size()-1; // GetCourseSongIndex() is 0-based
}

void ScreenGameplay::SetupSong( int iSongIndex )
{
	FOREACH_EnabledPlayerInfo( m_vPlayerInfo, pi )
	{
		/* This is the first beat that can be changed without it being visible.
		 * Until we draw for the first time, any beat can be changed. */
		pi->GetPlayerState()->m_fLastDrawnBeat = -100;

		Steps *pSteps = pi->m_vpStepsQueue[iSongIndex];
 		GAMESTATE->m_pCurSteps[ pi->GetStepsAndTrailIndex() ].Set( pSteps );

		/* Load new NoteData into Player. Do this before
		 * RebuildPlayerOptionsFromActiveAttacks or else transform mods will get
		 * propagated to GAMESTATE->m_pPlayerOptions too early and be double-applied
		 * to the NoteData:
		 * once in Player::Load, then again in Player::ApplyActiveAttacks.
		 * This is very bad for transforms like AddMines.
		 */
		NoteData originalNoteData;
		pSteps->GetNoteData( originalNoteData );

		const Style* pStyle = GAMESTATE->GetCurrentStyle(pi->m_pn);
		NoteData ndTransformed;
		pStyle->GetTransformedNoteDataForStyle( pi->GetStepsAndTrailIndex(), originalNoteData, ndTransformed );

		// HACK: Apply NoteSkins from global course options. Do this before
		// Player::Load, since it needs to know which note skin to load.
		pi->GetPlayerState()->m_ModsToApply.clear();
		for( unsigned i=0; i<pi->m_asModifiersQueue[iSongIndex].size(); ++i )
		{
			Attack a = pi->m_asModifiersQueue[iSongIndex][i];
			if( a.fStartSecond != 0 )
				continue;
			a.fStartSecond = ATTACK_STARTS_NOW;	// now

			PlayerOptions po;
			po.FromString( a.sModifiers );
			if( po.m_sNoteSkin.empty() )
				continue;
			a.sModifiers = po.m_sNoteSkin;

			pi->GetPlayerState()->LaunchAttack( a );
		}

		/* Update attack bOn flags, and rebuild Current-level options
		 * from Song-level options. The current NoteSkin could have changed
		 * because of an attack ending. */
		pi->GetPlayerState()->Update( 0 );

		// load player
		{
			pi->m_NoteData = ndTransformed;
			NoteDataUtil::RemoveAllTapsOfType( pi->m_NoteData, TapNoteType_AutoKeysound );
			pi->m_pPlayer->Load();
		}

		// load auto keysounds
		{
			NoteData nd = ndTransformed;
			NoteDataUtil::RemoveAllTapsExceptForType( nd, TapNoteType_AutoKeysound );
			m_AutoKeysounds.Load( pi->GetStepsAndTrailIndex(), nd );
		}

		{
			RString sType;
			switch( GAMESTATE->m_SongOptions.GetCurrent().m_SoundEffectType )
			{
				case SoundEffectType_Off:	sType = "SoundEffectControl_Off";	break;
				case SoundEffectType_Speed:	sType = "SoundEffectControl_Speed";	break;
				case SoundEffectType_Pitch:	sType = "SoundEffectControl_Pitch";	break;
				default: break;
			}

			pi->m_SoundEffectControl.Load( sType, pi->GetPlayerState(), &pi->m_NoteData );
		}

		// Put course options into effect.  Do this after Player::Load so
		// that mods aren't double-applied.
		pi->GetPlayerState()->m_ModsToApply.clear();
		for( unsigned i=0; i<pi->m_asModifiersQueue[iSongIndex].size(); ++i )
		{
			Attack a = pi->m_asModifiersQueue[iSongIndex][i];
			if( a.fStartSecond == 0 )
				a.fStartSecond = ATTACK_STARTS_NOW;	// now

			pi->GetPlayerState()->LaunchAttack( a );
			GAMESTATE->m_SongOptions.FromString( ModsLevel_Song, a.sModifiers );
		}

		// Update attack bOn flags.
		pi->GetPlayerState()->Update( 0 );

		// Hack: Course modifiers that are set to start immediately shouldn't tween on.
		pi->GetPlayerState()->m_PlayerOptions.SetCurrentToLevel( ModsLevel_Stage );
	}
}

void ScreenGameplay::ReloadCurrentSong()
{
	FOREACH_EnabledPlayerInfoNotDummy( m_vPlayerInfo, pi )
		pi->GetPlayerStageStats()->m_iSongsPlayed--;

	LoadNextSong();
}

void ScreenGameplay::LoadNextSong()
{
	GAMESTATE->ResetMusicStatistics();

	FOREACH_EnabledPlayerInfo( m_vPlayerInfo, pi )
	{
		pi->GetPlayerStageStats()->m_iSongsPlayed++;
		if( pi->m_ptextCourseSongNumber )
			pi->m_ptextCourseSongNumber->SetText( ssprintf(SONG_NUMBER_FORMAT.GetValue(), pi->GetPlayerStageStats()->m_iSongsPassed+1) );
	}

	if( GAMESTATE->m_bMultiplayer )
	{
		FOREACH_ENUM( MultiPlayer, mp )
			this->UpdateStageStats( mp );
	}

	int iPlaySongIndex = GAMESTATE->GetCourseSongIndex();
	iPlaySongIndex %= m_apSongsQueue.size();
	GAMESTATE->m_pCurSong.Set( m_apSongsQueue[iPlaySongIndex] );
	STATSMAN->m_CurStageStats.m_vpPlayedSongs.push_back( GAMESTATE->m_pCurSong );

	// No need to do this here.  We do it in SongFinished().
	//GAMESTATE->RemoveAllActiveAttacks();

	// Force immediate fail behavior changed to theme metric by Kyz.
	if(FORCE_IMMEDIATE_FAIL_FOR_BATTERY)
	{
		FOREACH_EnabledPlayerInfo( m_vPlayerInfo, pi )
		{
			if(pi->GetPlayerState()->m_PlayerOptions.GetStage().m_LifeType == LifeType_Battery)
			{
				PO_GROUP_ASSIGN(pi->GetPlayerState()->m_PlayerOptions, ModsLevel_Song, m_FailType, FailType_Immediate);
			}
		}
	}

	m_textSongOptions.SetText( GAMESTATE->m_SongOptions.GetCurrent().GetString() );

	SetupSong( iPlaySongIndex );

	Song* pSong = GAMESTATE->m_pCurSong;
	FOREACH_EnabledPlayerInfo( m_vPlayerInfo, pi )
	{
		Steps* pSteps = GAMESTATE->m_pCurSteps[ pi->GetStepsAndTrailIndex() ];
		++pi->GetPlayerStageStats()->m_iStepsPlayed;

		ASSERT( GAMESTATE->m_pCurSteps[ pi->GetStepsAndTrailIndex() ] != nullptr );
		if( pi->m_ptextStepsDescription )
			pi->m_ptextStepsDescription->SetText( pSteps->GetDescription() );

		/* Increment the play count even if the player fails.  (It's still popular,
		 * even if the people playing it aren't good at it.) */
		if( !GAMESTATE->m_bDemonstrationOrJukebox )
		{
			if( pi->m_pn != PLAYER_INVALID )
				PROFILEMAN->IncrementStepsPlayCount( pSong, pSteps, pi->m_pn );
		}

		if( pi->m_ptextPlayerOptions )
			pi->m_ptextPlayerOptions->SetText( pi->GetPlayerState()->m_PlayerOptions.GetCurrent().GetString() );
		if( pi->m_pActiveAttackList )
			pi->m_pActiveAttackList->Refresh();

		// reset oni game over graphic
		SET_XY_AND_ON_COMMAND( pi->m_sprOniGameOver );

		if(pi->GetPlayerState()->m_PlayerOptions.GetStage().m_LifeType==LifeType_Battery && pi->GetPlayerStageStats()->m_bFailed)	// already failed
			pi->ShowOniGameOver();

		if(pi->GetPlayerState()->m_PlayerOptions.GetStage().m_LifeType==LifeType_Bar && pi->m_pLifeMeter )
			pi->m_pLifeMeter->UpdateNonstopLifebar();

		if( pi->m_pStepsDisplay )
			pi->m_pStepsDisplay->SetFromSteps( pSteps );

		/* The actual note data for scoring is the base class of Player.  This includes
		 * transforms, like Wide.  Otherwise, the scoring will operate on the wrong data. */
		if( pi->m_pPrimaryScoreKeeper )
			pi->m_pPrimaryScoreKeeper->OnNextSong( GAMESTATE->GetCourseSongIndex(), pSteps, &pi->m_pPlayer->GetNoteData() );
		if( pi->m_pSecondaryScoreKeeper )
			pi->m_pSecondaryScoreKeeper->OnNextSong( GAMESTATE->GetCourseSongIndex(), pSteps, &pi->m_pPlayer->GetNoteData() );

		// Don't mess with the PlayerController of the Dummy player
		if( !pi->m_bIsDummy )
		{
			if( GAMESTATE->m_bDemonstrationOrJukebox )
			{
				pi->GetPlayerState()->m_PlayerController = PC_CPU;
				pi->GetPlayerState()->m_iCpuSkill = 5;
			}
			else if( GAMESTATE->IsCpuPlayer(pi->GetStepsAndTrailIndex()) )
			{
				pi->GetPlayerState()->m_PlayerController = PC_CPU;
				int iMeter = pSteps->GetMeter();
				int iNewSkill = SCALE( iMeter, MIN_METER, MAX_METER, 0, NUM_SKILL_LEVELS-1 );
				/* Watch out: songs aren't actually bound by MAX_METER. */
				iNewSkill = clamp( iNewSkill, 0, NUM_SKILL_LEVELS-1 );
				pi->GetPlayerState()->m_iCpuSkill = iNewSkill;
			}
			else
			{
				if( pi->GetPlayerState()->m_PlayerOptions.GetCurrent().m_fPlayerAutoPlay != 0 )
					pi->GetPlayerState()->m_PlayerController = PC_AUTOPLAY;
				else
					pi->GetPlayerState()->m_PlayerController = GamePreferences::m_AutoPlay;
			}
		}
	}

	bool bAllReverse = true;
	bool bAtLeastOneReverse = false;
	FOREACH_EnabledPlayerInfo( m_vPlayerInfo, pi )
	{
		if( pi->GetPlayerState()->m_PlayerOptions.GetCurrent().m_fScrolls[PlayerOptions::SCROLL_REVERSE] == 1 )
			bAtLeastOneReverse = true;
		else
			bAllReverse = false;
	}

	FOREACH_EnabledPlayerInfo( m_vPlayerInfo, pi )
	{
		bool bReverse = pi->GetPlayerState()->m_PlayerOptions.GetCurrent().m_fScrolls[PlayerOptions::SCROLL_REVERSE] == 1;

		if( pi->m_pStepsDisplay )
			pi->m_pStepsDisplay->PlayCommand( bReverse? "SetReverse":"SetNoReverse" );
	}

	m_LyricDisplay.PlayCommand( bAllReverse? "SetReverse": bAtLeastOneReverse? "SetOneReverse": "SetNoReverse" );

	// Load lyrics
	// XXX: don't load this here (who and why? -aj)
	LyricsLoader LL;
	if( GAMESTATE->m_pCurSong->HasLyrics()  )
		LL.LoadFromLRCFile(GAMESTATE->m_pCurSong->GetLyricsPath(), *GAMESTATE->m_pCurSong);

	// Set up song-specific graphics.

	// Check to see if any players are in beginner mode.
	// Note: steps can be different if turn modifiers are used.
	if( PREFSMAN->m_bShowBeginnerHelper )
	{
		FOREACH_EnabledPlayerNumberInfo( m_vPlayerInfo, pi )
		{
			PlayerNumber pn = pi->GetStepsAndTrailIndex();
			if( GAMESTATE->IsHumanPlayer(pn) && GAMESTATE->m_pCurSteps[pn]->GetDifficulty() == Difficulty_Beginner )
				m_BeginnerHelper.AddPlayer( pn, pi->m_pPlayer->GetNoteData() );
		}
	}

	if( m_pSongBackground )
		m_pSongBackground->Unload();

	if( m_pSongForeground )
		m_pSongForeground->Unload();

	if( !PREFSMAN->m_bShowBeginnerHelper || !m_BeginnerHelper.Init(2) )
	{
		m_BeginnerHelper.SetVisible( false );

		// BeginnerHelper disabled, or failed to load.
		if( m_pSongBackground )
			m_pSongBackground->LoadFromSong( GAMESTATE->m_pCurSong );

		if( !GAMESTATE->m_bDemonstrationOrJukebox )
		{
			/* This will fade from a preset brightness to the actual brightness
			 * (based on prefs and "cover"). The preset brightness may be 0 (to
			 * fade from black), or it might be 1, if the stage screen has the
			 * song BG and we're coming from it (like Pump). This used to be done
			 * in SM_PlayReady, but that means it's impossible to snap to the
			 * new brightness immediately. */
			if( m_pSongBackground )
			{
				m_pSongBackground->SetBrightness( INITIAL_BACKGROUND_BRIGHTNESS );
				m_pSongBackground->FadeToActualBrightness();
			}
		}
	}
	else
	{
		m_BeginnerHelper.SetVisible( true );
	}

	FOREACH_EnabledPlayerInfo( m_vPlayerInfo, pi )
	{
		if( !pi->GetPlayerStageStats()->m_bFailed )
		{
			// give a little life back between stages
			if( pi->m_pLifeMeter )
				pi->m_pLifeMeter->OnLoadSong();
			if( pi->m_pPrimaryScoreDisplay )
				pi->m_pPrimaryScoreDisplay->OnLoadSong();
			if( pi->m_pSecondaryScoreDisplay )
				pi->m_pSecondaryScoreDisplay->OnLoadSong();
		}
	}
	if( m_pCombinedLifeMeter )
		m_pCombinedLifeMeter->OnLoadSong();

	if( m_pSongForeground )
		m_pSongForeground->LoadFromSong( GAMESTATE->m_pCurSong );

	m_fTimeSinceLastDancingComment = 0;

	/* m_soundMusic and m_pSongBackground take a very long time to load,
	 * so cap fDelta at 0 so m_NextSong will show up on screen.
	 * -Chris */
	m_bZeroDeltaOnNextUpdate = true;
	SCREENMAN->ZeroNextUpdate();

	// Load cabinet lights data
	LoadLights();

	/* Load the music last, since it may start streaming and we don't want the music
	 * to compete with other loading. */
	m_AutoKeysounds.FinishLoading();
	m_pSoundMusic = m_AutoKeysounds.GetSound();

	/* Give SoundEffectControls the new RageSoundReaders. */
	FOREACH_EnabledPlayerInfo( m_vPlayerInfo, pi )
	{
		RageSoundReader *pPlayerSound = m_AutoKeysounds.GetPlayerSound(pi->m_pn);
		if( pPlayerSound == nullptr && pi->m_pn == GAMESTATE->GetMasterPlayerNumber() )
			pPlayerSound = m_AutoKeysounds.GetSharedSound();
		pi->m_SoundEffectControl.SetSoundReader( pPlayerSound );
	}

	MESSAGEMAN->Broadcast("DoneLoadingNextSong");
}

void ScreenGameplay::LoadLights()
{
	if( !LIGHTSMAN->IsEnabled() )
		return;

	// First, check if the song has explicit lights
	m_CabinetLightsNoteData.Init();
	ASSERT( GAMESTATE->m_pCurSong != nullptr );

	const Steps *pSteps = SongUtil::GetClosestNotes( GAMESTATE->m_pCurSong, StepsType_lights_cabinet, Difficulty_Medium );
	if( pSteps != nullptr )
	{
		pSteps->GetNoteData( m_CabinetLightsNoteData );
		return;
	}

	// No explicit lights.  Create autogen lights.
	RString sDifficulty = PREFSMAN->m_sLightsStepsDifficulty;
	vector<RString> asDifficulties;
	split( sDifficulty, ",", asDifficulties );

	// Always use the steps from the primary steps type so that lights are consistent over single and double styles.
	StepsType st = GAMEMAN->GetHowToPlayStyleForGame( GAMESTATE->m_pCurGame )->m_StepsType;

	Difficulty d1 = Difficulty_Invalid;
	if( asDifficulties.size() > 0 )
	{
		if( asDifficulties[0].CompareNoCase("selected") == 0 )
		{
			// Base lights off current difficulty of active player
			// Can be either P1 or P2 if they're individual or P1 if both are active
			FOREACH_EnabledPlayerNumberInfo( m_vPlayerInfo, pi )
			{
				PlayerNumber pn = pi->GetStepsAndTrailIndex();

				if( GAMESTATE->IsPlayerEnabled(pn) )
				{
					d1 = GAMESTATE->m_pCurSteps[pn]->GetDifficulty();
					break;
				}
			}
		}
		else
			d1 = StringToDifficulty( asDifficulties[0] );
	}

	pSteps = SongUtil::GetClosestNotes( GAMESTATE->m_pCurSong, st, d1 );

	// If we can't find anything at all, stop.
	if( pSteps == nullptr )
		return;

	NoteData TapNoteData1;
	pSteps->GetNoteData( TapNoteData1 );

	//taken from oitg, restores arrow -> marquee/bass light mapping.
	if( asDifficulties.size() > 1 )
	{
		Difficulty d2 = StringToDifficulty( asDifficulties[1] );

		Steps *pSteps2;

		pSteps2 = SongUtil::GetClosestNotes( GAMESTATE->m_pCurSong, st, d2 );

		if(pSteps2 != nullptr)
		{
			NoteData TapNoteData2;
			pSteps2->GetNoteData( TapNoteData2 );

			NoteDataUtil::LoadTransformedLightsFromTwo( TapNoteData1, TapNoteData2, m_CabinetLightsNoteData );
			return;
		}

		/* fall through */
	}

	NoteDataUtil::LoadTransformedLights( TapNoteData1, m_CabinetLightsNoteData, GAMEMAN->GetStepsTypeInfo(StepsType_lights_cabinet).iNumTracks );
}

void ScreenGameplay::StartPlayingSong( float fMinTimeToNotes, float fMinTimeToMusic )
{
	ASSERT( fMinTimeToNotes >= 0 );
	ASSERT( fMinTimeToMusic >= 0 );

	m_pSoundMusic->SetProperty( "AccurateSync", true );

	RageSoundParams p;
	p.m_fSpeed = GAMESTATE->m_SongOptions.GetCurrent().m_fMusicRate;
	p.StopMode = RageSoundParams::M_CONTINUE;

	{
		const float fFirstSecond = GAMESTATE->m_pCurSong->GetFirstSecond();
		float fStartDelay = fMinTimeToNotes - fFirstSecond;
		fStartDelay = max( fStartDelay, fMinTimeToMusic );
		p.m_StartSecond = -fStartDelay;
	}

	ASSERT( !m_pSoundMusic->IsPlaying() );
	{
		float fSecondsToStartFadingOutMusic, fSecondsToStartTransitioningOut;
		GetMusicEndTiming( fSecondsToStartFadingOutMusic, fSecondsToStartTransitioningOut );

		if( fSecondsToStartFadingOutMusic < GAMESTATE->m_pCurSong->m_fMusicLengthSeconds )
		{
			p.m_fFadeOutSeconds = MUSIC_FADE_OUT_SECONDS;
			p.m_LengthSeconds = fSecondsToStartFadingOutMusic + MUSIC_FADE_OUT_SECONDS - p.m_StartSecond;
		}
	}
	m_pSoundMusic->Play(false, &p);
	if( m_bPaused )
		m_pSoundMusic->Pause( true );

	/* Make sure GAMESTATE->m_fMusicSeconds is set up. */
	GAMESTATE->m_Position.m_fMusicSeconds = -5000;
	UpdateSongPosition(0);

	ASSERT( GAMESTATE->m_Position.m_fMusicSeconds > -4000 ); /* make sure the "fake timer" code doesn't trigger */
	FOREACH_EnabledPlayer(pn)
	{
		if(GAMESTATE->m_pCurSteps[pn])
		{
			GAMESTATE->m_pCurSteps[pn]->GetTimingData()->PrepareLookup();
		}
	}
}

void ScreenGameplay::set_paused_internal(bool p)
{
	m_bPaused= p;
	GAMESTATE->SetPaused(p);
}

void ScreenGameplay::PauseGame( bool bPause, GameController gc )
{
	if( m_bPaused == bPause )
	{
		LOG->Trace( "ScreenGameplay::PauseGame(%i) received, but already in that state; ignored", bPause );
		return;
	}

	// Don't pause if we're already tweening out.
	if( bPause && m_DancingState == STATE_OUTRO )
		return;

	ResetGiveUpTimers(false);

	set_paused_internal(bPause);
	m_PauseController = gc;

	m_pSoundMusic->Pause( bPause );
	if( bPause )
		this->PlayCommand( "Pause" );
	else
		this->PlayCommand( "Unpause" );

	FOREACH_EnabledPlayerInfo( m_vPlayerInfo, pi )
	{
		pi->m_pPlayer->SetPaused( m_bPaused );
	}
}

// play assist ticks
void ScreenGameplay::PlayTicks()
{
	/* TODO: Allow all players to have ticks. Not as simple as it looks.
	 * If a loop takes place, it could make one player's ticks come later
	 * than intended. Any help here would be appreciated. -Wolfman2000 */
	Player &player = *m_vPlayerInfo[GAMESTATE->GetMasterPlayerNumber()].m_pPlayer;
	const NoteData &nd = player.GetNoteData();
	m_GameplayAssist.PlayTicks( nd, player.GetPlayerState() );
}

/* Play announcer "type" if it's been at least fSeconds since the last announcer. */
void ScreenGameplay::PlayAnnouncer( const RString &type, float fSeconds, float *fDeltaSeconds )
{
	if( GAMESTATE->m_fOpponentHealthPercent == 0 )
		return; // Shut the announcer up

	/* Don't play in demonstration. */
	if( GAMESTATE->m_bDemonstrationOrJukebox )
		return;

	/* Don't play before the first beat, or after we're finished. */
	if( m_DancingState != STATE_DANCING )
		return;
	if(GAMESTATE->m_pCurSong == nullptr  ||	// this will be true on ScreenDemonstration sometimes
	   GAMESTATE->m_Position.m_fSongBeat < GAMESTATE->m_pCurSong->GetFirstBeat())
		return;

	if( *fDeltaSeconds < fSeconds )
		return;
	*fDeltaSeconds = 0;

	SOUND->PlayOnceFromAnnouncer( type );
}

void ScreenGameplay::UpdateSongPosition( float fDeltaTime )
{
	if( !m_pSoundMusic->IsPlaying() )
		return;

	RageTimer tm;
	const float fSeconds = m_pSoundMusic->GetPositionSeconds( nullptr, &tm );
	const float fAdjust = SOUND->GetFrameTimingAdjustment( fDeltaTime );
	GAMESTATE->UpdateSongPosition( fSeconds+fAdjust, GAMESTATE->m_pCurSong->m_SongTiming, tm+fAdjust );
}

void ScreenGameplay::BeginScreen()
{
	if( GAMESTATE->m_pCurSong == nullptr  )
		return;

	ScreenWithMenuElements::BeginScreen();

	SOUND->PlayOnceFromAnnouncer( "gameplay intro" );	// crowd cheer

	// Get the transitions rolling
	if( !m_bForceNoNetwork && NSMAN->useSMserver )
	{
		// If we're using networking, we must not have any delay. If we do,
		// this can cause inconsistency on different computers and
		// different themes.

		StartPlayingSong( 0, 0 );
		m_pSoundMusic->Stop();

		float startOffset = g_fNetStartOffset;

		NSMAN->StartRequest(1);

		RageSoundParams p;
		p.m_fSpeed = 1.0f;	// Force 1.0 playback speed
		p.StopMode = RageSoundParams::M_CONTINUE;
		p.m_StartSecond = startOffset;
		m_pSoundMusic->SetProperty( "AccurateSync", true );
		m_pSoundMusic->Play(false, &p);

		UpdateSongPosition(0);
	}
	else
	{
		StartPlayingSong( MIN_SECONDS_TO_STEP, MIN_SECONDS_TO_MUSIC );
	}
}

bool ScreenGameplay::AllAreFailing()
{
	FOREACH_EnabledPlayerInfo( m_vPlayerInfo, pi )
	{
		if( pi->m_pLifeMeter && !pi->m_pLifeMeter->IsFailing() )
			return false;
	}
	return true;
}

void ScreenGameplay::GetMusicEndTiming( float &fSecondsToStartFadingOutMusic, float &fSecondsToStartTransitioningOut )
{
	float fLastStepSeconds = GAMESTATE->m_pCurSong->GetLastSecond();
	fLastStepSeconds += Player::GetMaxStepDistanceSeconds();

	float fTransitionLength;
	if( !GAMESTATE->IsCourseMode() || IsLastSong() )
		fTransitionLength = OUT_TRANSITION_LENGTH;
	else
		fTransitionLength = COURSE_TRANSITION_LENGTH;

	fSecondsToStartTransitioningOut = fLastStepSeconds;

	// Align the end of the music fade to the end of the transition.
	float fSecondsToFinishFadingOutMusic = fSecondsToStartTransitioningOut + fTransitionLength;
	if( fSecondsToFinishFadingOutMusic < GAMESTATE->m_pCurSong->m_fMusicLengthSeconds )
		fSecondsToStartFadingOutMusic = fSecondsToFinishFadingOutMusic - MUSIC_FADE_OUT_SECONDS;
	else
		fSecondsToStartFadingOutMusic = GAMESTATE->m_pCurSong->m_fMusicLengthSeconds; // don't fade

	/* Make sure we keep going long enough to register a miss for the last note, and
	 * never start fading before the last note. */
	fSecondsToStartFadingOutMusic = max( fSecondsToStartFadingOutMusic, fLastStepSeconds );
	fSecondsToStartTransitioningOut = max( fSecondsToStartTransitioningOut, fLastStepSeconds );

	/* Make sure the fade finishes before the transition finishes. */
	fSecondsToStartTransitioningOut = max( fSecondsToStartTransitioningOut, fSecondsToStartFadingOutMusic + MUSIC_FADE_OUT_SECONDS - fTransitionLength );
}

void ScreenGameplay::Update( float fDeltaTime )
{
	if( GAMESTATE->m_pCurSong == nullptr  )
	{
		/* ScreenDemonstration will move us to the next screen.  We just need to
		 * survive for one update without crashing.  We need to call Screen::Update
		 * to make sure we receive the next-screen message. */
		Screen::Update( fDeltaTime );
		return;
	}

	UpdateSongPosition( fDeltaTime );

	if( m_bZeroDeltaOnNextUpdate )
	{
		Screen::Update( 0 );
		m_bZeroDeltaOnNextUpdate = false;
	}
	else
	{
		const float fRate = PREFSMAN->m_bRateModsAffectTweens ? GAMESTATE->m_SongOptions.GetCurrent().m_fMusicRate : 1.0f;
		Screen::Update( fDeltaTime * fRate );
	}

	/* This happens if ScreenDemonstration::HandleScreenMessage sets a new screen when
	 * PREFSMAN->m_bDelayedScreenLoad. */
	if( GAMESTATE->m_pCurSong == nullptr )
		return;
	/* This can happen if ScreenDemonstration::HandleScreenMessage sets a new screen when
	 * !PREFSMAN->m_bDelayedScreenLoad.  (The new screen was loaded when we called Screen::Update,
	 * and the ctor might set a new GAMESTATE->m_pCurSong, so the above check can fail.) */
	if( SCREENMAN->GetTopScreen() != this )
		return;

	/* Update actors when paused, but never move on to another state. */
	if( m_bPaused )
		return;

	//LOG->Trace( "m_fOffsetInBeats = %f, m_fBeatsPerSecond = %f, m_Music.GetPositionSeconds = %f", m_fOffsetInBeats, m_fBeatsPerSecond, m_Music.GetPositionSeconds() );

	m_AutoKeysounds.Update(fDeltaTime);

	// update GameState HealthState
	FOREACH_EnabledPlayerInfo( m_vPlayerInfo, pi )
	{
		HealthState &hs = pi->GetPlayerState()->m_HealthState;
		HealthState OldHealthState = hs;
		if( GAMESTATE->GetPlayerFailType(pi->GetPlayerState()) != FailType_Off &&
			pi->m_pLifeMeter && pi->m_pLifeMeter->IsFailing() )
		{
			hs = HealthState_Dead;
		}
		else if( pi->m_pLifeMeter && pi->m_pLifeMeter->IsHot() )
		{
			hs = HealthState_Hot;
		}
		else if( GAMESTATE->GetPlayerFailType(pi->GetPlayerState()) != FailType_Off &&
			pi->m_pLifeMeter && pi->m_pLifeMeter->IsInDanger() )
		{
			hs = HealthState_Danger;
		}
		else
		{
			hs = HealthState_Alive;
		}

		if( hs != OldHealthState )
		{
			Message msg( "HealthStateChanged" );
			msg.SetParam( "PlayerNumber", pi->m_pn );
			msg.SetParam( "HealthState", hs );
			msg.SetParam( "OldHealthState", OldHealthState );
			MESSAGEMAN->Broadcast( msg );
		}

		pi->m_SoundEffectControl.Update( fDeltaTime );
	}

	{
		float fSpeed = GAMESTATE->m_SongOptions.GetCurrent().m_fMusicRate;
		if( GAMESTATE->m_SongOptions.GetCurrent().m_fHaste != 0.0f )
			fSpeed *= GetHasteRate();

		RageSoundParams p = m_pSoundMusic->GetParams();
		if( fabsf(p.m_fSpeed - fSpeed) > 0.01f && fSpeed >= 0.0f)
		{
			p.m_fSpeed = fSpeed;
			m_pSoundMusic->SetParams( p );
		}
	}

	switch( m_DancingState )
	{
		case STATE_DANCING:
		{
			/* Set STATSMAN->m_CurStageStats.bFailed for failed players.  In, FAIL_IMMEDIATE, send
			 * SM_BeginFailed if all players failed, and kill dead Oni players. */
			FOREACH_EnabledPlayerInfo( m_vPlayerInfo, pi )
			{
				PlayerNumber pn = pi->GetStepsAndTrailIndex();

				FailType ft = GAMESTATE->GetPlayerFailType( pi->GetPlayerState() );
				LifeType lt = pi->GetPlayerState()->m_PlayerOptions.GetStage().m_LifeType;
				if( ft == FailType_Off || ft == FailType_EndOfSong )
					continue;

				// check for individual fail
				if( pi->m_pLifeMeter == nullptr || !pi->m_pLifeMeter->IsFailing() )
					continue; /* isn't failing */
				if( pi->GetPlayerStageStats()->m_bFailed )
					continue; /* failed and is already dead */

				LOG->Trace("Player %d failed", (int)pn);
				pi->GetPlayerStageStats()->m_bFailed = true;	// fail

				{
					Message msg("PlayerFailed");
					msg.SetParam( "PlayerNumber", pi->m_pn );
					MESSAGEMAN->Broadcast( msg );
				}

				// Check for and do Oni die.
				bool bAllowOniDie = false;
				switch( lt )
				{
					case LifeType_Battery:
						bAllowOniDie = true;
					default:
						break;
				}
				if( bAllowOniDie && ft == FailType_Immediate )
				{
					if( !STATSMAN->m_CurStageStats.AllFailed() )	// if not the last one to fail
					{
						// kill them!
						FailFadeRemovePlayer(&*pi);
					}
				}
			}

			bool bAllFailed = true;
			FOREACH_EnabledPlayerInfo( m_vPlayerInfo, pi )
			{
				FailType ft = GAMESTATE->GetPlayerFailType( pi->GetPlayerState() );
				switch( ft )
				{
				case FailType_Immediate:
					if( pi->m_pLifeMeter == nullptr  ||  (pi->m_pLifeMeter && !pi->m_pLifeMeter->IsFailing()) )
						bAllFailed = false;
					break;
				case FailType_ImmediateContinue:
				case FailType_EndOfSong:
					bAllFailed = false;	// wait until the end of the song to fail.
					break;
				case FailType_Off:
					bAllFailed = false;	// never fail.
					break;
				default:
					FAIL_M("Invalid fail type! Aborting...");
				}
			}

			if( bAllFailed )
			{
				m_pSoundMusic->StopPlaying();
				SCREENMAN->PostMessageToTopScreen( SM_NotesEnded, 0 );
				m_LyricDisplay.Stop();
			}

			// Update living players' alive time
			// HACK: Don't scale alive time when using tab/tilde.  Instead of accumulating time from a timer,
			// this time should instead be tied to the music position.
			float fUnscaledDeltaTime = m_timerGameplaySeconds.GetDeltaTime();

			FOREACH_EnabledPlayerInfo( m_vPlayerInfo, pi )
				if( !pi->GetPlayerStageStats()->m_bFailed )
					pi->GetPlayerStageStats()->m_fAliveSeconds += fUnscaledDeltaTime * GAMESTATE->m_SongOptions.GetCurrent().m_fMusicRate;

			// update fGameplaySeconds
			STATSMAN->m_CurStageStats.m_fGameplaySeconds += fUnscaledDeltaTime;
			float curBeat = GAMESTATE->m_Position.m_fSongBeat;
			Song &s = *GAMESTATE->m_pCurSong;

			if( curBeat >= s.GetFirstBeat() && curBeat < s.GetLastBeat() )
			{
				STATSMAN->m_CurStageStats.m_fStepsSeconds += fUnscaledDeltaTime;

				UpdateHasteRate();

				if( GAMESTATE->m_SongOptions.GetCurrent().m_fHaste != 0.0f )
				{
					float fHasteRate = GetHasteRate();
					// For negative haste, accumulate seconds while the song is slowed down.
					if(GAMESTATE->m_SongOptions.GetCurrent().m_fHaste < 0)
					{
						GAMESTATE->m_fAccumulatedHasteSeconds -= (fUnscaledDeltaTime * fHasteRate) - fUnscaledDeltaTime;
					}
					// For positive haste, accumulate seconds while the song is sped up.
					else
					{
						GAMESTATE->m_fAccumulatedHasteSeconds += (fUnscaledDeltaTime * fHasteRate) - fUnscaledDeltaTime;
					}
				}
			}

			// Check for end of song
			{
				float fSecondsToStartFadingOutMusic, fSecondsToStartTransitioningOut;
				GetMusicEndTiming( fSecondsToStartFadingOutMusic, fSecondsToStartTransitioningOut );

				bool bAllReallyFailed = STATSMAN->m_CurStageStats.AllFailed();
				if( bAllReallyFailed )
					fSecondsToStartTransitioningOut += BEGIN_FAILED_DELAY;

				if( GAMESTATE->m_Position.m_fMusicSeconds >= fSecondsToStartTransitioningOut && !m_NextSong.IsTransitioning() )
					this->PostScreenMessage( SM_NotesEnded, 0 );
			}

			// update 2d dancing characters
			FOREACH_EnabledPlayerNumberInfo( m_vPlayerInfo, pi )
			{
				DancingCharacters *pCharacter = nullptr;
				if( m_pSongBackground )
					pCharacter = m_pSongBackground->GetDancingCharacters();
				if( pCharacter != nullptr )
				{
					TapNoteScore tns = pi->m_pPlayer->GetLastTapNoteScore();

					ANIM_STATES_2D state = AS2D_MISS;

					switch( tns )
					{
					case TNS_W4:
					case TNS_W3:
						state = AS2D_GOOD;
						break;
					case TNS_W2:
					case TNS_W1:
						state = AS2D_GREAT;
						break;
					default:
						state = AS2D_MISS;
						break;
					}

					if( state == AS2D_GREAT && pi->GetPlayerState()->m_HealthState == HealthState_Hot )
						state = AS2D_FEVER;

					pCharacter->Change2DAnimState( pi->m_pn, state );
				}
			}

			// Check for enemy death in enemy battle
			static float fLastSeenEnemyHealth = 1;
			if( fLastSeenEnemyHealth != GAMESTATE->m_fOpponentHealthPercent )
			{
				fLastSeenEnemyHealth = GAMESTATE->m_fOpponentHealthPercent;

				if( GAMESTATE->m_fOpponentHealthPercent == 0 )
				{
					// HACK:  Load incorrect directory on purpose for now.
					PlayAnnouncer( "gameplay battle damage level3", 0 );

					GAMESTATE->RemoveAllActiveAttacks();

					FOREACH_EnabledPlayerNumberInfo( m_vPlayerInfo, pi )
					{
						if( !GAMESTATE->IsCpuPlayer(pi->m_pn) )
							continue;

						FailFadeRemovePlayer(&*pi);
					}
				}
			}

			// update give up
			bool bGiveUpTimerFired = false;
			bGiveUpTimerFired= !m_GiveUpTimer.IsZero() && m_GiveUpTimer.Ago() > GIVE_UP_SECONDS;
			m_gave_up= bGiveUpTimerFired;
			m_skipped_song= !m_SkipSongTimer.IsZero() && m_SkipSongTimer.Ago() > GIVE_UP_SECONDS;


			bool bAllHumanHaveBigMissCombo = true;
			FOREACH_EnabledPlayerNumberInfo( m_vPlayerInfo, pi )
			{
				if (pi->GetPlayerState()->m_PlayerOptions.GetCurrent().m_FailType == FailType_Off ||
					pi->GetPlayerState()->m_HealthState < HealthState_Dead )
				{
					bAllHumanHaveBigMissCombo = false;
					break;
				}
			}
			if (bAllHumanHaveBigMissCombo) // possible to get in here.
			{
				bAllHumanHaveBigMissCombo = FAIL_ON_MISS_COMBO.GetValue() != -1 && STATSMAN->m_CurStageStats.GetMinimumMissCombo() >= (unsigned int)FAIL_ON_MISS_COMBO;
			}
			if(bGiveUpTimerFired || bAllHumanHaveBigMissCombo || m_skipped_song)
			{
				STATSMAN->m_CurStageStats.m_bGaveUp = true;
				FOREACH_EnabledPlayerNumberInfo( m_vPlayerInfo, pi )
				{
					pi->GetPlayerStageStats()->m_bFailed |= bAllHumanHaveBigMissCombo;
					pi->GetPlayerStageStats()->m_bDisqualified |= bGiveUpTimerFired;    // Don't disqualify if failing for miss combo.  The player should still be eligable for a high score on courses.
				}
				ResetGiveUpTimers(false);
				if(GIVING_UP_GOES_TO_PREV_SCREEN && !m_skipped_song)
				{
					BeginBackingOutFromGameplay();
				}
				else
				{
					m_pSoundMusic->StopPlaying();
					this->PostScreenMessage( SM_NotesEnded, 0 );
				}
				return;
			}

			// Check to see if it's time to play a ScreenGameplay comment
			m_fTimeSinceLastDancingComment += fDeltaTime;

			PlayMode mode = GAMESTATE->m_PlayMode;
			switch( mode )
			{
				case PLAY_MODE_REGULAR:
				case PLAY_MODE_BATTLE:
				case PLAY_MODE_RAVE:
					if( GAMESTATE->OneIsHot() )
						PlayAnnouncer( "gameplay comment hot", SECONDS_BETWEEN_COMMENTS );
					else if( GAMESTATE->AllAreInDangerOrWorse() )
						PlayAnnouncer( "gameplay comment danger", SECONDS_BETWEEN_COMMENTS );
					else
						PlayAnnouncer( "gameplay comment good", SECONDS_BETWEEN_COMMENTS );
					break;
				case PLAY_MODE_NONSTOP:
				case PLAY_MODE_ONI:
				case PLAY_MODE_ENDLESS:
					PlayAnnouncer( "gameplay comment oni", SECONDS_BETWEEN_COMMENTS );
					break;
				default:
					FAIL_M(ssprintf("Invalid PlayMode: %i", mode));
			}
		}
		default: break;
	}

	PlayTicks();
	UpdateLights();
	SendCrossedMessages();

	if( !m_bForceNoNetwork && NSMAN->useSMserver )
	{
		FOREACH_EnabledPlayerNumberInfo( m_vPlayerInfo, pi )
			if( pi->m_pLifeMeter )
				NSMAN->m_playerLife[pi->m_pn] = int(pi->m_pLifeMeter->GetLife()*10000);

		if( m_bShowScoreboard )
			FOREACH_NSScoreBoardColumn(cn)
				if( m_bShowScoreboard && NSMAN->ChangedScoreboard(cn) && GAMESTATE->GetFirstDisabledPlayer() != PLAYER_INVALID )
					m_Scoreboard[cn].SetText( NSMAN->m_Scoreboard[cn] );
	}
	// ArrowEffects::Update call moved because having it happen once per
	// NoteField (which means twice in two player) seemed wasteful. -Kyz
	ArrowEffects::Update();
}

void ScreenGameplay::DrawPrimitives()
{
	// ScreenGameplay::DrawPrimitives exists so that the notefield board can be
	// above the song background and underneath everything else.  This way, a
	// theme can put a screen filter in the notefield board and not have it
	// obscure custom elements on the screen.  Putting the screen filter in the
	// notefield board simplifies placement because it ensures that the filter
	// is in the same place as the notefield, instead of forcing the filter to
	// check conditions and metrics that affect the position of the notefield.
	// This also solves the problem of the ComboUnderField metric putting the
	// combo underneath the opaque notefield board.
	// -Kyz
	if(m_pSongBackground)
	{
		m_pSongBackground->m_disable_draw= false;
		m_pSongBackground->Draw();
		m_pSongBackground->m_disable_draw= true;
	}
	FOREACH_EnabledPlayerNumberInfo(m_vPlayerInfo, pi)
	{
		pi->m_pPlayer->DrawNoteFieldBoard();
	}
	// Clear the z buffer so 3D notes aren't hidden by anything in the underlay using masking. -Kyz
	DISPLAY->ClearZBuffer();
	ScreenWithMenuElements::DrawPrimitives();
}

void ScreenGameplay::FailFadeRemovePlayer(PlayerInfo* pi)
{
	SOUND->PlayOnceFromDir( THEME->GetPathS(m_sName,"oni die") );
	pi->ShowOniGameOver();
	int tracks = pi->m_NoteData.GetNumTracks();
	pi->m_NoteData.Init();		// remove all notes and scoring
	pi->m_NoteData.SetNumTracks(tracks); // reset the number of tracks.
	pi->m_pPlayer->FadeToFail();	// tell the NoteField to fade to white
}

float ScreenGameplay::GetHasteRate()
{
	return m_fCurrHasteRate;
}

void ScreenGameplay::UpdateHasteRate()
{
	if( GAMESTATE->m_Position.m_fMusicSeconds < GAMESTATE->m_fLastHasteUpdateMusicSeconds || // new song
		GAMESTATE->m_Position.m_fMusicSeconds > GAMESTATE->m_fLastHasteUpdateMusicSeconds + m_fHasteTimeBetweenUpdates )
	{
		bool bAnyPlayerHitAllNotes = false;
		FOREACH_EnabledPlayerInfo( m_vPlayerInfo, pi )
		{
			if( !GAMESTATE->IsHumanPlayer(pi->m_pn) )
				continue;

			PlayerState *pPS = pi->GetPlayerState();
			if( pPS->m_iTapsHitSinceLastHasteUpdate > 0 &&
				pPS->m_iTapsMissedSinceLastHasteUpdate == 0 )
				bAnyPlayerHitAllNotes = true;

			pPS->m_iTapsHitSinceLastHasteUpdate = 0;
			pPS->m_iTapsMissedSinceLastHasteUpdate = 0;
		}

		if( bAnyPlayerHitAllNotes )
			GAMESTATE->m_fHasteRate += 0.1f;
		CLAMP( GAMESTATE->m_fHasteRate, -1.0f, +1.0f );

		GAMESTATE->m_fLastHasteUpdateMusicSeconds = GAMESTATE->m_Position.m_fMusicSeconds;
	}

	/* If the life meter is less than half full, push the haste rate down to let
	 * the player use his accumulated haste time. */
	float fMaxLife = 0;
	FOREACH_EnabledPlayerInfo( m_vPlayerInfo, pi )
	{
		if( !GAMESTATE->IsHumanPlayer(pi->m_pn) )
			continue;
		// In Battle/Rave mode, the players don't have life meters.
		if(pi->m_pLifeMeter)
		{
			fMaxLife= max(fMaxLife, pi->m_pLifeMeter->GetLife());
		}
		else
		{
			fMaxLife= 1;
		}
	}
	if( fMaxLife <= m_fHasteLifeSwitchPoint )
		GAMESTATE->m_fHasteRate = SCALE( fMaxLife, 0.0f, m_fHasteLifeSwitchPoint, -1.0f, 0.0f );
	CLAMP( GAMESTATE->m_fHasteRate, -1.0f, +1.0f );

	float fSpeed = 1.0f;
	// If there are no turning points or no add amounts, the bad themer probably thinks that's a way to disable haste.
	// Since we're outside a lua function, crashing (asserting) won't point back to the source of the problem.
	if(m_HasteTurningPoints.size() < 2 || m_HasteAddAmounts.size() < 2 ||
		m_HasteTurningPoints.size() != m_HasteAddAmounts.size())
	{
		m_fCurrHasteRate= fSpeed;
		return;
	}
	float options_haste= GAMESTATE->m_SongOptions.GetCurrent().m_fHaste;
	float scale_from_low= -1;
	float scale_from_high= 1;
	float scale_to_low= 0;
	float scale_to_high=0;
	for(size_t turning_point= 0; turning_point < m_HasteTurningPoints.size();
			++turning_point)
	{
		float curr_turning_point= m_HasteTurningPoints[turning_point];
		scale_from_high= curr_turning_point;
		scale_to_high= m_HasteAddAmounts[turning_point];
		if(GAMESTATE->m_fHasteRate < curr_turning_point)
		{
			break;
		}
		scale_from_low= curr_turning_point;
		scale_to_low= m_HasteAddAmounts[turning_point];
	}
	// If negative haste is being used, the game instead slows down when the player does well.
	float speed_add= SCALE(GAMESTATE->m_fHasteRate, scale_from_low, scale_from_high, scale_to_low, scale_to_high) * options_haste;
	if(scale_from_low == scale_from_high)
	{
		speed_add= scale_to_high * options_haste;
	}
	CLAMP(speed_add, -1.0f, 1.0f);

	// Only adjust speed_add by AccumulatedHasteSeconds when the player is losing seconds.  Otherwise, gaining the first second is interfered with.
	bool losing_seconds= false;
	if(options_haste > 0)
	{
		losing_seconds= speed_add < 0;
	}
	else
	{
		losing_seconds= speed_add > 0;
	}
	if( losing_seconds && GAMESTATE->m_fAccumulatedHasteSeconds <= 1 )
	{
		/* Only allow slowing down the song while the players have accumulated
		 * haste. This prevents dragging on the song by keeping the life meter
		 * nearly empty. */
		/* In positive haste mode, the player accumulates seconds while the song
		 * is sped up, and loses them while the song is slowed down.  "<= 1"
		 * means that the player is only eligible to slow the song down when
		 * they are down to their last accumulated second. -Kyz */
		// 1 second left is full speed_add, 0 seconds left is no speed_add.
		float clamp_secs= max(0, GAMESTATE->m_fAccumulatedHasteSeconds);
		speed_add = speed_add * clamp_secs;
	}
	fSpeed += speed_add;
	m_fCurrHasteRate= fSpeed;
}

void ScreenGameplay::UpdateLights()
{
	if( !LIGHTSMAN->IsEnabled() )
		return;
	if( m_CabinetLightsNoteData.GetNumTracks() == 0 )	// light data wasn't loaded
		return;

	bool bBlinkCabinetLight[NUM_CabinetLight];
	bool bBlinkGameButton[NUM_GameController][NUM_GameButton];
	ZERO( bBlinkCabinetLight );
	ZERO( bBlinkGameButton );
	{
		const float fSongBeat = GAMESTATE->m_Position.m_fLightSongBeat;
		const int iSongRow = BeatToNoteRowNotRounded( fSongBeat );

		static int iRowLastCrossed = 0;

		FOREACH_CabinetLight( cl )
		{
			// for each index we crossed since the last update:
			FOREACH_NONEMPTY_ROW_IN_TRACK_RANGE( m_CabinetLightsNoteData, cl, r, iRowLastCrossed+1, iSongRow+1 )
			{
				if( m_CabinetLightsNoteData.GetTapNote( cl, r ).type != TapNoteType_Empty )
					bBlinkCabinetLight[cl] = true;
			}

			if( m_CabinetLightsNoteData.IsHoldNoteAtRow( cl, iSongRow ) )
				bBlinkCabinetLight[cl] = true;
		}

		FOREACH_EnabledPlayerNumberInfo( m_vPlayerInfo, pi )
		{
			const Style* pStyle = GAMESTATE->GetCurrentStyle(pi->m_pn);
			const NoteData &nd = pi->m_pPlayer->GetNoteData();
			for( int t=0; t<nd.GetNumTracks(); t++ )
			{
				bool bBlink = false;

				// for each index we crossed since the last update:
				FOREACH_NONEMPTY_ROW_IN_TRACK_RANGE( nd, t, r, iRowLastCrossed+1, iSongRow+1 )
				{
					const TapNote &tn = nd.GetTapNote( t, r );
					if( tn.type != TapNoteType_Mine )
						bBlink = true;
				}

				// check if a hold should be active
				if( nd.IsHoldNoteAtRow( t, iSongRow ) )
					bBlink = true;

				if( bBlink )
				{
					vector<GameInput> gi;
					pStyle->StyleInputToGameInput( t, pi->m_pn, gi );
					for(size_t i= 0; i < gi.size(); ++i)
					{
						bBlinkGameButton[gi[i].controller][gi[i].button] = true;
					}
				}
			}
		}

		iRowLastCrossed = iSongRow;
	}

	// Before the first beat of the song, all cabinet lights solid on (except for menu buttons).
	Song &s = *GAMESTATE->m_pCurSong;
	bool bOverrideCabinetBlink = (GAMESTATE->m_Position.m_fSongBeat < s.GetFirstBeat());
	FOREACH_CabinetLight( cl )
		bBlinkCabinetLight[cl] |= bOverrideCabinetBlink;

	// Send blink data.
	FOREACH_CabinetLight( cl )
	{
		if( bBlinkCabinetLight[cl] )
			LIGHTSMAN->BlinkCabinetLight( cl );
	}

	FOREACH_ENUM( GameController,  gc )
	{
		FOREACH_ENUM( GameButton,  gb )
		{
			if( bBlinkGameButton[gc][gb] )
				LIGHTSMAN->BlinkGameButton( GameInput(gc,gb) );
		}
	}
}

void ScreenGameplay::SendCrossedMessages()
{
	{
		static int iRowLastCrossed = 0;

		float fPositionSeconds = GAMESTATE->m_Position.m_fMusicSeconds;
		float fSongBeat = GAMESTATE->m_pCurSong->m_SongTiming.GetBeatFromElapsedTime( fPositionSeconds );

		int iRowNow = BeatToNoteRowNotRounded( fSongBeat );
		iRowNow = max( 0, iRowNow );

		for( int r=iRowLastCrossed+1; r<=iRowNow; r++ )
		{
			if( GetNoteType( r ) == NOTE_TYPE_4TH )
				MESSAGEMAN->Broadcast( Message_BeatCrossed );
		}

		iRowLastCrossed = iRowNow;
	}

	{
		const int NUM_MESSAGES_TO_SEND = 4;
		const float MESSAGE_SPACING_SECONDS = 0.4f;

		PlayerNumber pn = PLAYER_INVALID;
		FOREACH_EnabledPlayerNumberInfo( m_vPlayerInfo, pi )
		{
			if( GAMESTATE->m_pCurSteps[ pi->m_pn ]->GetDifficulty() == Difficulty_Beginner )
			{
				pn = pi->m_pn;
				break;
			}
		}
		if( pn == PLAYER_INVALID )
			return;

		const NoteData &nd = m_vPlayerInfo[pn].m_pPlayer->GetNoteData();

		static int iRowLastCrossedAll[NUM_MESSAGES_TO_SEND] = { 0, 0, 0, 0 };
		for( int i=0; i<NUM_MESSAGES_TO_SEND; i++ )
		{
			float fNoteWillCrossInSeconds = MESSAGE_SPACING_SECONDS * i;

			float fPositionSeconds = GAMESTATE->m_Position.m_fMusicSeconds + fNoteWillCrossInSeconds;
			float fSongBeat = GAMESTATE->m_pCurSong->m_SongTiming.GetBeatFromElapsedTime( fPositionSeconds );

			int iRowNow = BeatToNoteRowNotRounded( fSongBeat );
			iRowNow = max( 0, iRowNow );
			int &iRowLastCrossed = iRowLastCrossedAll[i];

			FOREACH_NONEMPTY_ROW_ALL_TRACKS_RANGE( nd, r, iRowLastCrossed+1, iRowNow+1 )
			{
				int iNumTracksWithTapOrHoldHead = 0;
				for( int t=0; t<nd.GetNumTracks(); t++ )
				{
					if( nd.GetTapNote(t,r).type == TapNoteType_Empty )
						continue;

					iNumTracksWithTapOrHoldHead++;

					// send crossed message
					if(GAMESTATE->GetCurrentGame()->m_PlayersHaveSeparateStyles)
					{
						FOREACH_EnabledPlayerNumberInfo(m_vPlayerInfo, pi)
						{
							const Style *pStyle = GAMESTATE->GetCurrentStyle(pi->m_pn);
							RString sButton = pStyle->ColToButtonName( t );
							Message msg( i == 0 ? "NoteCrossed" : "NoteWillCross" );
							msg.SetParam( "ButtonName", sButton );
							msg.SetParam( "NumMessagesFromCrossed", i );
							msg.SetParam("PlayerNumber", pi->m_pn);
							MESSAGEMAN->Broadcast( msg );
						}
					}
					else
					{
						const Style *pStyle = GAMESTATE->GetCurrentStyle(PLAYER_INVALID);
						RString sButton = pStyle->ColToButtonName( t );
						Message msg( i == 0 ? "NoteCrossed" : "NoteWillCross" );
						msg.SetParam( "ButtonName", sButton );
						msg.SetParam( "NumMessagesFromCrossed", i );
						MESSAGEMAN->Broadcast( msg );
					}
				}

				if( iNumTracksWithTapOrHoldHead > 0 )
					MESSAGEMAN->Broadcast( (MessageID)(Message_NoteCrossed + i) );
				if( i == 0  &&  iNumTracksWithTapOrHoldHead >= 2 )
				{
					RString sMessageName = "NoteCrossedJump";
					MESSAGEMAN->Broadcast( sMessageName );
				}
			}

			iRowLastCrossed = iRowNow;
		}
	}
}

void ScreenGameplay::BeginBackingOutFromGameplay()
{
	m_DancingState = STATE_OUTRO;
	ResetGiveUpTimers(false);

	m_pSoundMusic->StopPlaying();
	m_GameplayAssist.StopPlaying(); // Stop any queued assist ticks.

	this->ClearMessageQueue();

	// If this is the final stage, don't allow extra stage
	if( GAMESTATE->GetSmallestNumStagesLeftForAnyHumanPlayer() == 0 )
		GAMESTATE->m_bBackedOutOfFinalStage = true;
	// Disallow backing out of extra stage
	if( GAMESTATE->IsAnExtraStage() )
		SCREENMAN->PostMessageToTopScreen( SM_BeginFailed, 0 );
	else
		m_Cancel.StartTransitioning( SM_DoPrevScreen );
}

void ScreenGameplay::AbortGiveUpText(bool show_abort_text)
{
	m_textDebug.StopTweening();
	if(show_abort_text)
	{
		m_textDebug.SetText(GIVE_UP_ABORTED_TEXT);
	}
	// otherwise tween out the text that's there

	m_textDebug.BeginTweening(1/2.f);
	m_textDebug.SetDiffuse(RageColor(1,1,1,0));
}

void ScreenGameplay::AbortSkipSong(bool show_text)
{
	if(m_SkipSongTimer.IsZero())
	{
		return;
	}
	AbortGiveUpText(show_text);
	m_SkipSongTimer.SetZero();
}

void ScreenGameplay::AbortGiveUp( bool bShowText )
{
	if( m_GiveUpTimer.IsZero() )
	{
		return;
	}
	AbortGiveUpText(bShowText);
	m_GiveUpTimer.SetZero();
}

void ScreenGameplay::ResetGiveUpTimers(bool show_text)
{
	AbortSkipSong(show_text);
	AbortGiveUp(show_text);
}


bool ScreenGameplay::Input( const InputEventPlus &input )
{
	//LOG->Trace( "ScreenGameplay::Input()" );

	Message msg("");
	if( m_Codes.InputMessage(input, msg) )
		this->HandleMessage( msg );

	if( m_bPaused )
	{
		/* If we're paused, only accept GAME_BUTTON_START to unpause. */
		if( GAMESTATE->IsHumanPlayer(input.pn) && input.MenuI == GAME_BUTTON_START && input.type == IET_FIRST_PRESS )
		{
			if( m_PauseController == GameController_Invalid || m_PauseController == input.GameI.controller )
			{
				// IMO, it's better to have this configurable. -DaisuMaster
				if( UNPAUSE_WITH_START )
				{
					this->PauseGame( false );
					return true;
				}
			}
		}
		return false;
	}

	if(m_DancingState != STATE_OUTRO  &&
		GAMESTATE->IsHumanPlayer(input.pn)  &&
		!m_Cancel.IsTransitioning() )
	{
		/* Allow bailing out by holding any START button.
		 * This gives a way to "give up" when a back button isn't available.
		 * If this is also a style button, don't do this; pump center is start.
		 */
		bool bHoldingGiveUp = false;
		if( GAMESTATE->GetCurrentStyle(input.pn)->GameInputToColumn(input.GameI) == Column_Invalid )
		{
			bHoldingGiveUp |= ( START_GIVES_UP && input.MenuI == GAME_BUTTON_START );
			bHoldingGiveUp |= ( BACK_GIVES_UP && input.MenuI == GAME_BUTTON_BACK );
		}
		// Allow holding SELECT to skip the current song in course mode. -Kyz
		if(GAMESTATE->IsCourseMode() && SELECT_SKIPS_SONG &&
			input.MenuI == GAME_BUTTON_SELECT)
		{
			if(input.type == IET_RELEASE)
			{
				AbortSkipSong(true);
			}
			else if(input.type == IET_FIRST_PRESS && m_SkipSongTimer.IsZero())
			{
				m_textDebug.SetText(SKIP_SONG_TEXT);
				m_textDebug.PlayCommand("StartOn");
				m_SkipSongTimer.Touch();
			}
			return true;
		}

		if( bHoldingGiveUp )
		{
			// No PREFSMAN->m_bDelayedEscape; always delayed.
			if( input.type==IET_RELEASE )
			{
				AbortGiveUp( true );
			}
			else if( input.type==IET_FIRST_PRESS && m_GiveUpTimer.IsZero() )
			{
				m_textDebug.SetText( GIVE_UP_START_TEXT );
				m_textDebug.PlayCommand( "StartOn" );
				m_GiveUpTimer.Touch(); // start the timer
			}

			return true;
		}

		/* Only handle GAME_BUTTON_BACK as a regular BACK button if BACK_GIVES_UP is
		 * disabled. */
		bool bHoldingBack = false;
		if( GAMESTATE->GetCurrentStyle(input.pn)->GameInputToColumn(input.GameI) == Column_Invalid )
		{
			bHoldingBack |= input.MenuI == GAME_BUTTON_BACK && !BACK_GIVES_UP;
		}

		if( bHoldingBack )
		{
			if( ((!PREFSMAN->m_bDelayedBack && input.type==IET_FIRST_PRESS) ||
				(input.DeviceI.device==DEVICE_KEYBOARD && input.type==IET_REPEAT) ||
				(input.DeviceI.device!=DEVICE_KEYBOARD && INPUTFILTER->GetSecsHeld(input.DeviceI) >= 1.0f)) )
			{
				LOG->Trace("Player %i went back", input.pn+1);
				BeginBackingOutFromGameplay();
			}
			else if( PREFSMAN->m_bDelayedBack && input.type==IET_FIRST_PRESS )
			{
				m_textDebug.SetText( GIVE_UP_BACK_TEXT );
				m_textDebug.PlayCommand( "BackOn" );
			}
			else if( PREFSMAN->m_bDelayedBack && input.type==IET_RELEASE )
			{
				m_textDebug.PlayCommand( "TweenOff" );
			}

			return true;
		}
	}

	bool bRelease = input.type == IET_RELEASE;
	if( !input.GameI.IsValid() )
		return false;

	int iCol = GAMESTATE->GetCurrentStyle(input.pn)->GameInputToColumn( input.GameI );

	// Don't pass on any inputs to Player that aren't a press or a release.
	switch( input.type )
	{
	case IET_FIRST_PRESS:
	case IET_RELEASE:
		break;
	default:
		return false;
	}

	if( GAMESTATE->m_bMultiplayer )
	{
		if( input.mp != MultiPlayer_Invalid  &&  GAMESTATE->IsMultiPlayerEnabled(input.mp)  &&  iCol != -1 )
		{
			for (PlayerInfo const &pi : m_vPlayerInfo)
			{
				if( input.mp == pi.m_mp )
					pi.m_pPlayer->Step( iCol, -1, input.DeviceI.ts, false, bRelease );
			}
			return true;
		}
	}
	else
	{
		// handle a step or battle item activate
		if( GAMESTATE->IsHumanPlayer( input.pn ) )
		{
			ResetGiveUpTimers(true);

			if( GamePreferences::m_AutoPlay == PC_HUMAN && GAMESTATE->m_pPlayerState[input.pn]->m_PlayerOptions.GetCurrent().m_fPlayerAutoPlay == 0 )
			{
				PlayerInfo& pi = GetPlayerInfoForInput( input );

				ASSERT( input.GameI.IsValid() );

				GameButtonType gbt = GAMESTATE->m_pCurGame->GetPerButtonInfo(input.GameI.button)->m_gbt;
				switch( gbt )
				{
				case GameButtonType_Menu:
					return false;
				case GameButtonType_Step:
					if( iCol != -1 )
						pi.m_pPlayer->Step( iCol, -1, input.DeviceI.ts, false, bRelease );
					return true;
				}
			}
		}
	}
	return false;
}


/* Saving StageStats that are affected by the note pattern is a little tricky:
 *
 * Stats are cumulative for course play.
 *
 * For regular songs, it doesn't matter how we do it; the pattern doesn't change
 * during play.
 *
 * The pattern changes during play in battle and course mode. We want to include
 * these changes, so run stats for a song after the song finishes.
 *
 * If we fail, be sure to include the current song in stats,
 * with the current modifier set. So:
 * 1. At the end of a song in any mode, pass or fail, add stats for that song
 *    (from m_pPlayer).
 * 2. At the end of gameplay in course mode, add stats for any songs that weren't
 *    played, applying the modifiers the song would have been played with.
 *    This doesn't include songs that were played but failed; that was done in #1.
 */
void ScreenGameplay::SaveStats()
{
	float fMusicLen = GAMESTATE->m_pCurSong->m_fMusicLengthSeconds;

	FOREACH_EnabledPlayerInfo( m_vPlayerInfo, pi )
	{
		/* Note that adding stats is only meaningful for the counters (eg. RadarCategory_Jumps),
		 * not for the percentages (RadarCategory_Air). */
		RadarValues rv;
		PlayerStageStats &pss = *pi->GetPlayerStageStats();
		const NoteData &nd = pi->m_pPlayer->GetNoteData();
		PlayerNumber pn = pi->m_pn;

		GAMESTATE->SetProcessedTimingData(GAMESTATE->m_pCurSteps[pn]->GetTimingData());
		NoteDataUtil::CalculateRadarValues( nd, fMusicLen, rv );
		pss.m_radarPossible += rv;
		NoteDataWithScoring::GetActualRadarValues( nd, pss, fMusicLen, rv );
		pss.m_radarActual += rv;
		GAMESTATE->SetProcessedTimingData(nullptr);
	}
}

void ScreenGameplay::SongFinished()
{
	FOREACH_EnabledPlayer(pn)
	{
		if(GAMESTATE->m_pCurSteps[pn])
		{
			GAMESTATE->m_pCurSteps[pn]->GetTimingData()->ReleaseLookup();
		}
	}
	AdjustSync::HandleSongEnd();
	SaveStats(); // Let subclasses save the stats.
	/* Extremely important: if we don't remove attacks before moving on to the next
	 * screen, they'll still be turned on eventually. */
	GAMESTATE->RemoveAllActiveAttacks();
	FOREACH_VisiblePlayerInfo( m_vPlayerInfo, pi )
		pi->m_pActiveAttackList->Refresh();
}

void ScreenGameplay::StageFinished( bool bBackedOut )
{
	if( GAMESTATE->IsCourseMode() && GAMESTATE->m_PlayMode != PLAY_MODE_ENDLESS )
	{
		LOG->Trace("Stage finished at index %i/%i", GAMESTATE->GetCourseSongIndex(), (int) m_apSongsQueue.size() );
		// +1 to skip the current song; that song has already passed.
		for( unsigned i = GAMESTATE->GetCourseSongIndex()+1; i < m_apSongsQueue.size(); ++i )
		{
			LOG->Trace("Running stats for %i", i );
			SetupSong( i );
			FOREACH_EnabledPlayerInfo( m_vPlayerInfo, pi )
				pi->m_pPlayer->ApplyWaitingTransforms();
			SongFinished();
		}
	}

	if( bBackedOut )
	{
		GAMESTATE->CancelStage();
		return;
	}

	// If all players failed, kill.
	if( STATSMAN->m_CurStageStats.AllFailed() )
	{
		FOREACH_HumanPlayer( p )
			GAMESTATE->m_iPlayerStageTokens[p] = 0;
	}

	FOREACH_HumanPlayer( pn )
		STATSMAN->m_CurStageStats.m_player[pn].CalcAwards( pn, STATSMAN->m_CurStageStats.m_bGaveUp, STATSMAN->m_CurStageStats.m_bUsedAutoplay );
	STATSMAN->m_CurStageStats.FinalizeScores( false );

	GAMESTATE->CommitStageStats();

	// save current stage stats
	STATSMAN->m_vPlayedStageStats.push_back( STATSMAN->m_CurStageStats );

	STATSMAN->CalcAccumPlayedStageStats();
	GAMESTATE->FinishStage();
}

void ScreenGameplay::HandleScreenMessage( const ScreenMessage SM )
{
	CHECKPOINT_M( ssprintf("HandleScreenMessage(%s)", ScreenMessageHelpers::ScreenMessageToString(SM).c_str()) );
	if( SM == SM_DoneFadingIn )
	{
		// If the ready animation is zero length, then playing the sound will
		// make it overlap with the go sound.
		// If the Ready animation is zero length, and the Go animation is not,
		// only play the Go sound.
		// If they're both zero length, only play the Ready sound.
		// Otherwise, play both sounds.
		// -Kyz
		m_Ready.StartTransitioning( SM_PlayGo );
		if(m_Ready.GetTweenTimeLeft() <= .0f)
		{
			m_delaying_ready_announce= true;
		}
		else
		{
			m_delaying_ready_announce= false;
			SOUND->PlayOnceFromAnnouncer("gameplay ready");
		}
	}
	else if( SM == SM_PlayGo )
	{
		m_Go.StartTransitioning( SM_None );
		bool should_play_go= true;
		if(m_delaying_ready_announce)
		{
			if(m_Go.GetTweenTimeLeft() <= .0f)
			{
				SOUND->PlayOnceFromAnnouncer("gameplay ready");
				should_play_go= false;
			}
			else
			{
				should_play_go= true;
			}
		}
		if(should_play_go)
		{
			if( GAMESTATE->IsAnExtraStage() )
			{
				SOUND->PlayOnceFromAnnouncer( "gameplay here we go extra" );
			}
			else if( GAMESTATE->GetSmallestNumStagesLeftForAnyHumanPlayer() == 0 )
			{
				SOUND->PlayOnceFromAnnouncer( "gameplay here we go final" );
			}
			else
			{
				SOUND->PlayOnceFromAnnouncer( "gameplay here we go normal" );
			}
		}

		GAMESTATE->m_DanceStartTime.Touch();

		GAMESTATE->m_bGameplayLeadIn.Set( false );
		m_DancingState = STATE_DANCING; // STATE CHANGE!  Now the user is allowed to press Back
	}
	else if( SM == SM_NotesEnded )	// received while STATE_DANCING
	{
		ResetGiveUpTimers(false); // don't allow giveup while the next song is loading

		/* Do this in LoadNextSong, so we don't tween off old attacks until
		 * m_NextSong finishes. */
		// GAMESTATE->RemoveAllActiveAttacks();

		FOREACH_EnabledPlayerInfo( m_vPlayerInfo, pi )
		{
			// Mark failure.
			if(GAMESTATE->GetPlayerFailType(pi->GetPlayerState()) != FailType_Off
					&& (pi->m_pLifeMeter && pi->m_pLifeMeter->IsFailing()))
			{
				pi->GetPlayerStageStats()->m_bFailed = true;
			}

			if( !pi->GetPlayerStageStats()->m_bFailed )
			{
				pi->GetPlayerStageStats()->m_iSongsPassed++;
			}

			// set a life record at the point of failure
			if( pi->GetPlayerStageStats()->m_bFailed )
			{
				pi->GetPlayerStageStats()->SetLifeRecordAt(
					0, STATSMAN->m_CurStageStats.m_fGameplaySeconds );
			}
		}

		/* If all players have *really* failed (bFailed, not the life meter or
		 * bFailedEarlier): */
		const bool bAllReallyFailed = STATSMAN->m_CurStageStats.AllFailed();
		const bool bStopCourseEarly = STOP_COURSE_EARLY;
		const bool bIsLastSong = IsLastSong();

		LOG->Trace( "bAllReallyFailed = %d, bStopCourseEarly = %d, "
			"bIsLastSong = %d, m_gave_up = %d, m_skipped_song = %d",
			bAllReallyFailed, bStopCourseEarly, bIsLastSong, m_gave_up,
			m_skipped_song);

		if(!bIsLastSong && m_skipped_song)
		{
			// Load the next song in the course.
			HandleScreenMessage( SM_StartLoadingNextSong );
			return;
		}
		if( bStopCourseEarly || bAllReallyFailed || bIsLastSong || m_gave_up )
		{
			// Time to leave from ScreenGameplay
			HandleScreenMessage( SM_LeaveGameplay );
		}
		else
		{
			// Load the next song in the course.
			HandleScreenMessage( SM_StartLoadingNextSong );
			return;
		}
	}
	else if( SM == SM_LeaveGameplay )
	{
		GAMESTATE->m_DanceDuration= GAMESTATE->m_DanceStartTime.Ago();
		// update dancing characters for win / lose
		DancingCharacters *pDancers = nullptr;
		if( m_pSongBackground )
			pDancers = m_pSongBackground->GetDancingCharacters();
		if( pDancers )
		{
			FOREACH_EnabledPlayerNumberInfo( m_vPlayerInfo, pi )
			{
				// XXX: In battle modes, switch( GAMESTATE->GetStageResult(p) ).
				if( pi->GetPlayerStageStats()->m_bFailed )
					pDancers->Change2DAnimState( pi->m_pn, AS2D_FAIL ); // fail anim
				else if( pi->m_pLifeMeter && pi->GetPlayerState()->m_HealthState == HealthState_Hot )
					pDancers->Change2DAnimState( pi->m_pn, AS2D_WINFEVER ); // full life pass anim
				else
					pDancers->Change2DAnimState( pi->m_pn, AS2D_WIN ); // pass anim
			}
		}

		// End round.
		if( m_DancingState == STATE_OUTRO )	// ScreenGameplay already ended
			return;		// ignore
		m_DancingState = STATE_OUTRO;
		ResetGiveUpTimers(false);

		GAMESTATE->RemoveAllActiveAttacks();
		FOREACH_EnabledPlayerInfo( m_vPlayerInfo, pi )
		{
			if( pi->m_pActiveAttackList )
				pi->m_pActiveAttackList->Refresh();
		}

		LIGHTSMAN->SetLightsMode( LIGHTSMODE_ALL_CLEARED );

		bool bAllReallyFailed = STATSMAN->m_CurStageStats.AllFailed();

		if( bAllReallyFailed )
		{
			this->PostScreenMessage( SM_BeginFailed, 0 );
			return;
		}

		// todo: add GameplayCleared, StartTransitioningCleared commands -aj
		TweenOffScreen();

		m_Out.StartTransitioning( SM_DoNextScreen );

		// do they deserve an extra stage?
		if( GAMESTATE->HasEarnedExtraStage() )
			SOUND->PlayOnceFromAnnouncer( "gameplay extra" );
		else
			SOUND->PlayOnceFromAnnouncer( "gameplay cleared" );
	}
	else if( SM == SM_StartLoadingNextSong )
	{
		// Next song.
		// give a little life back between stages
		FOREACH_EnabledPlayerInfo( m_vPlayerInfo, pi )
		{
			if( pi->m_pLifeMeter )
				pi->m_pLifeMeter->OnSongEnded();
		}

		// If this is a repeating course, and we're at the end of it, repick and
		// add new songs to the players' step and song queues.
		if(GAMESTATE->IsCourseMode() && GAMESTATE->m_pCurCourse &&
			GAMESTATE->m_pCurCourse->m_bRepeat &&
			GAMESTATE->GetCourseSongIndex() >= (int)m_apSongsQueue.size()-1)
		{
			Course* course= GAMESTATE->m_pCurCourse;
			ASSERT(course != nullptr);
			// Need to store these so they can be used to refetch the players'
			// trails after they're invalidated.
			vector<StepsType> trail_sts;
			vector<CourseDifficulty> trail_cds;
			FOREACH_EnabledPlayerInfo(m_vPlayerInfo, pi)
			{
				Trail* trail= GAMESTATE->m_pCurTrail[pi->GetStepsAndTrailIndex()];
				ASSERT(trail != nullptr);
				trail_sts.push_back(trail->m_StepsType);
				trail_cds.push_back(trail->m_CourseDifficulty);
			}
			// Set a new stage seed so the order will be different.
			GAMESTATE->SetNewStageSeed();
			course->InvalidateTrailCache();
			course->RegenerateNonFixedTrails();
			size_t info_id= 0; // Can't use the player number in the playerinfo
			// because it won't match up in 2-player.
			FOREACH_EnabledPlayerInfo(m_vPlayerInfo, pi)
			{
				Trail* trail= course->GetTrail(trail_sts[info_id], trail_cds[info_id]);
				ASSERT(trail != nullptr);
				GAMESTATE->m_pCurTrail[pi->m_pn].Set(trail);
				++info_id;
			}
			PlayerNumber master_pn = GAMESTATE->GetMasterPlayerNumber();
			Trail *master_trail= GAMESTATE->m_pCurTrail[master_pn];
			ASSERT(master_trail != nullptr);
			for (TrailEntry const &entry : master_trail->m_vEntries)
			{
				ASSERT(entry.pSong != nullptr);
				m_apSongsQueue.push_back(entry.pSong);
				STATSMAN->m_CurStageStats.m_vpPossibleSongs.push_back(entry.pSong);
			}
			FOREACH_EnabledPlayerInfo(m_vPlayerInfo, pi)
			{
				Trail* trail= GAMESTATE->m_pCurTrail[pi->GetStepsAndTrailIndex()];
				ASSERT(trail != nullptr);
				for (TrailEntry const &entry : master_trail->m_vEntries)
				{
					ASSERT(entry.pSteps != nullptr);
					pi->m_vpStepsQueue.push_back(entry.pSteps);
					AttackArray a;
					entry.GetAttackArray(a);
					pi->m_asModifiersQueue.push_back(a);
					pi->GetPlayerStageStats()->m_vpPossibleSteps.push_back(entry.pSteps);
				}
				// In a survival course, override stored mods
				if(course->GetCourseType() == COURSE_TYPE_SURVIVAL &&
					SURVIVAL_MOD_OVERRIDE)
				{
					pi->GetPlayerState()->m_PlayerOptions.FromString(ModsLevel_Stage,
						"clearall," + CommonMetrics::DEFAULT_NOTESKIN_NAME.GetValue() +
						"," + CommonMetrics::DEFAULT_MODIFIERS.GetValue());
					pi->GetPlayerState()->RebuildPlayerOptionsFromActiveAttacks();
				}
			}
		}

		GAMESTATE->m_bLoadingNextSong = true;
		MESSAGEMAN->Broadcast( "BeforeLoadingNextCourseSong" );
		m_NextSong.Reset();
		m_NextSong.PlayCommand( "Start" );
		m_NextSong.StartTransitioning( SM_LoadNextSong );
		MESSAGEMAN->Broadcast( "ChangeCourseSongIn" );
	}
	else if( SM == SM_LoadNextSong )
	{
		m_pSoundMusic->Stop();
		SongFinished();

		MESSAGEMAN->Broadcast( "ChangeCourseSongOut" );

		GAMESTATE->m_bLoadingNextSong = false;
		LoadNextSong();

		m_NextSong.Reset();
		m_NextSong.PlayCommand( "Finish" );
		m_NextSong.StartTransitioning( SM_None );

		StartPlayingSong( MIN_SECONDS_TO_STEP_NEXT_SONG, 0 );
	}
	else if( SM == SM_PlayToasty )
	{
		if(g_bEasterEggs)
		{
			if(PREFSMAN->m_AllowMultipleToasties ||
				m_Toasty.IsWaiting())
			{
				m_Toasty.Reset();
				m_Toasty.StartTransitioning();
			}
		}
	}
	else if( ScreenMessageHelpers::ScreenMessageToString(SM).find("0Combo") != string::npos )
	{
		int iCombo;
		RString sCropped = ScreenMessageHelpers::ScreenMessageToString(SM).substr(3);
		sscanf(sCropped.c_str(),"%d%*s",&iCombo);
		PlayAnnouncer( ssprintf("gameplay %d combo",iCombo), 2 );
	}
	else if( SM == SM_ComboStopped )
	{
		PlayAnnouncer( "gameplay combo stopped", 2 );
	}
	else if( SM == SM_ComboContinuing )
	{
		PlayAnnouncer( "gameplay combo overflow", 2 );
	}
	else if( SM >= SM_BattleTrickLevel1 && SM <= SM_BattleTrickLevel3 )
	{
		int iTrickLevel = SM-SM_BattleTrickLevel1+1;
		PlayAnnouncer( ssprintf("gameplay battle trick level%d",iTrickLevel), 3 );
		if( SM == SM_BattleTrickLevel1 ) m_soundBattleTrickLevel1.Play(false);
		else if( SM == SM_BattleTrickLevel2 ) m_soundBattleTrickLevel2.Play(false);
		else if( SM == SM_BattleTrickLevel3 ) m_soundBattleTrickLevel3.Play(false);
	}
	else if( SM >= SM_BattleDamageLevel1 && SM <= SM_BattleDamageLevel3 )
	{
		int iDamageLevel = SM-SM_BattleDamageLevel1+1;
		PlayAnnouncer( ssprintf("gameplay battle damage level%d",iDamageLevel), 3 );
	}
	else if( SM == SM_DoPrevScreen )
	{
		SongFinished();
		this->StageFinished( true );

		m_sNextScreen = GetPrevScreen();

		if( AdjustSync::IsSyncDataChanged() )
			ScreenSaveSync::PromptSaveSync( SM_GoToPrevScreen );
		else
			HandleScreenMessage( SM_GoToPrevScreen );
	}
	else if( SM == SM_DoNextScreen )
	{
		SongFinished();
		this->StageFinished( false );
		// only save replays if the player chose to
		if( GAMESTATE->m_SongOptions.GetCurrent().m_bSaveReplay )
			SaveReplay();

		if( AdjustSync::IsSyncDataChanged() )
			ScreenSaveSync::PromptSaveSync( SM_GoToNextScreen );
		else
			HandleScreenMessage( SM_GoToNextScreen );
	}
	else if( SM == SM_GainFocus )
	{
		// We do this ourself.
		SOUND->HandleSongTimer( false );
	}
	else if( SM == SM_LoseFocus )
	{
		// We might have turned the song timer off. Be sure to turn it back on.
		SOUND->HandleSongTimer( true );
	}
	else if( SM == SM_BeginFailed )
	{
		m_DancingState = STATE_OUTRO;
		ResetGiveUpTimers(false);
		m_GameplayAssist.StopPlaying(); // Stop any queued assist ticks.
		TweenOffScreen();
		m_Failed.StartTransitioning( SM_DoNextScreen );

		// show the survive time if extra stage
		if( GAMESTATE->IsAnExtraStage() )
		{
			float fMaxAliveSeconds = 0;
			FOREACH_EnabledPlayer(p)
				fMaxAliveSeconds = max( fMaxAliveSeconds, STATSMAN->m_CurStageStats.m_player[p].m_fAliveSeconds );
			m_textSurviveTime.SetText( "TIME: " + SecondsToMMSSMsMs(fMaxAliveSeconds) );
			ON_COMMAND( m_textSurviveTime );
		}

		if( GAMESTATE->IsCourseMode() )
		{
			if( GAMESTATE->GetCourseSongIndex() >= int(m_apSongsQueue.size() / 2) )
				SOUND->PlayOnceFromAnnouncer( "gameplay oni failed halfway" );
			else
				SOUND->PlayOnceFromAnnouncer( "gameplay oni failed" );
		}
		else
		{
			SOUND->PlayOnceFromAnnouncer( "gameplay failed" );
		}
	}
	else if( SM == SM_Pause )
	{
		// Ignore SM_Pause when in demonstration.
		if( GAMESTATE->m_bDemonstrationOrJukebox )
			return;

		if( !m_bPaused )
			PauseGame( true );
	}

	ScreenWithMenuElements::HandleScreenMessage( SM );
}

void ScreenGameplay::HandleMessage( const Message &msg )
{
	if( msg == "Judgment" )
	{
		PlayerNumber pn;
		msg.GetParam( "Player", pn );

		FOREACH_EnabledPlayerNumberInfo( m_vPlayerInfo, pi )
		{
			if( pi->m_pn != pn )
				continue;
			if( !pi->GetPlayerState()->m_PlayerOptions.GetCurrent().m_bMuteOnError )
				continue;

			RageSoundReader *pSoundReader = m_AutoKeysounds.GetPlayerSound( pn );
			if( pSoundReader == nullptr )
				pSoundReader = m_AutoKeysounds.GetSharedSound();

			HoldNoteScore hns;
			msg.GetParam( "HoldNoteScore", hns );
			TapNoteScore tns;
			msg.GetParam( "TapNoteScore", tns );

			bool bOn = false;
			if( hns != HoldNoteScore_Invalid )
				bOn = hns != HNS_LetGo;
			else
				bOn = tns != TNS_Miss;

			if( pSoundReader )
				pSoundReader->SetProperty( "Volume", bOn? 1.0f:0.0f );
		}
	}

	ScreenWithMenuElements::HandleMessage( msg );
}

void ScreenGameplay::Cancel( ScreenMessage smSendWhenDone )
{
	m_pSoundMusic->Stop();

	ScreenWithMenuElements::Cancel( smSendWhenDone );
}

Song *ScreenGameplay::GetNextCourseSong() const
{
	ASSERT( GAMESTATE->IsCourseMode() );

	int iPlaySongIndex = GAMESTATE->GetCourseSongIndex()+1;
	iPlaySongIndex %= m_apSongsQueue.size();

	return m_apSongsQueue[iPlaySongIndex];
}

PlayerInfo *ScreenGameplay::GetPlayerInfo( PlayerNumber pn )
{
	FOREACH_EnabledPlayerNumberInfo( m_vPlayerInfo, pi )
	{
		if( pi->m_pn == pn )
			return &*pi;
	}
	return nullptr;
}

PlayerInfo *ScreenGameplay::GetDummyPlayerInfo( int iDummyIndex )
{
	FOREACH_EnabledPlayerInfo( m_vPlayerInfo, pi )
	{
		if( pi->m_bIsDummy  &&  pi->m_iDummyIndex == iDummyIndex )
			return &*pi;
	}
	return nullptr;
}

void ScreenGameplay::SaveReplay()
{
	/* Replay data TODO:
	 * Add more player information (?)
	 * Add AutoGen flag if steps were autogen?
	 * Add proper steps hash?
	 * Add modifiers used
	 * Add date played, machine played on, etc.
	 * Hash of some stuff to validate data (see Profile)
	 */
	FOREACH_HumanPlayer( pn )
	{
		FOREACH_EnabledPlayerInfo( m_vPlayerInfo, pi )
		{
			Profile *pTempProfile = PROFILEMAN->GetProfile(pn);

			XNode *p = new XNode("ReplayData");
			// append version number (in case the format changes)
			p->AppendAttr("Version",0);

			// song information node
			SongID songID;
			songID.FromSong(GAMESTATE->m_pCurSong);
			XNode *pSongInfoNode = songID.CreateNode();
			pSongInfoNode->AppendChild("Title", GAMESTATE->m_pCurSong->GetDisplayFullTitle());
			pSongInfoNode->AppendChild("Artist", GAMESTATE->m_pCurSong->GetDisplayArtist());
			p->AppendChild(pSongInfoNode);

			// steps information
			StepsID stepsID;
			stepsID.FromSteps( GAMESTATE->m_pCurSteps[pn] );
			XNode *pStepsInfoNode = stepsID.CreateNode();
			// hashing = argh
			//pStepsInfoNode->AppendChild("StepsHash", stepsID.ToSteps(GAMESTATE->m_pCurSong,false)->GetHash());
			p->AppendChild(pStepsInfoNode);

			// player information node (rival data sup)
			XNode *pPlayerInfoNode = new XNode("Player");
			pPlayerInfoNode->AppendChild("DisplayName", pTempProfile->m_sDisplayName);
			pPlayerInfoNode->AppendChild("Guid", pTempProfile->m_sGuid);
			p->AppendChild(pPlayerInfoNode);

			// the timings.
			p->AppendChild( pi->m_pPlayer->GetNoteData().CreateNode() );

			// Find a file name for the replay
			vector<RString> files;
			GetDirListing( "Save/Replays/replay*", files, false, false );
			sort( files.begin(), files.end() );

			// Files should be of the form "replay#####.xml".
			int iIndex = 0;

			for( int i = files.size()-1; i >= 0; --i )
			{
				static Regex re( "^replay([0-9]{5})\\....$" );
				vector<RString> matches;
				if( !re.Compare( files[i], matches ) )
					continue;

				ASSERT( matches.size() == 1 );
				iIndex = std::stoi( matches[0] )+1;
				break;
			}

			RString sFileName = ssprintf( "replay%05d.xml", iIndex );

			XmlFileUtil::SaveToFile( p, "Save/Replays/"+sFileName );
			SAFE_DELETE( p );
			return;
		}
	}
}

/*
bool ScreenGameplay::LoadReplay()
{
	// Load replay which was selected via options
}
*/

// lua start
#include "LuaBinding.h"
#include "OptionsBinding.h"

/** @brief Allow Lua to have access to the ScreenGameplay. */
class LunaScreenGameplay: public Luna<ScreenGameplay>
{
public:
	static int GetNextCourseSong( T* p, lua_State *L ) { p->GetNextCourseSong()->PushSelf(L); return 1; }
	static int Center1Player( T* p, lua_State *L ) { lua_pushboolean( L, p->Center1Player() ); return 1; }
	static int GetLifeMeter( T* p, lua_State *L )
	{
		PlayerNumber pn = Enum::Check<PlayerNumber>( L, 1 );

		PlayerInfo *pi = p->GetPlayerInfo(pn);
		if( pi == nullptr )
			return 0;
		LifeMeter *pLM = pi->m_pLifeMeter;
		if( pLM == nullptr )
			return 0;

		pLM->PushSelf( L );
		return 1;
	}
	static int GetPlayerInfo( T* p, lua_State *L )
	{
		PlayerNumber pn = Enum::Check<PlayerNumber>( L, 1 );

		PlayerInfo *pi = p->GetPlayerInfo(pn);
		if( pi == nullptr )
			return 0;

		pi->PushSelf( L );
		return 1;
	}
	static int GetDummyPlayerInfo( T* p, lua_State *L )
	{
		int iDummyIndex = IArg(1);
		PlayerInfo *pi = p->GetDummyPlayerInfo(iDummyIndex);
		if( pi == nullptr )
			return 0;
		pi->PushSelf( L );
		return 1;
	}
	static int PauseGame( T* p, lua_State *L )		{ p->Pause( BArg(1)); return 0; }
	static int IsPaused( T* p, lua_State *L )		{ lua_pushboolean( L, p->IsPaused() ); return 1; }
	static int GetHasteRate( T* p, lua_State *L )    { lua_pushnumber( L, p->GetHasteRate() ); return 1; }
	static bool TurningPointsValid(lua_State* L, int index)
	{
		size_t size= lua_objlen(L, index);
		if(size < 2)
		{
			luaL_error(L, "Invalid number of entries %zu", size);
		}
		float prev_turning= -1;
		for(size_t n= 1; n < size; ++n)
		{
			lua_pushnumber(L, n);
			lua_gettable(L, index);
			float v= FArg(-1);
			if(v < prev_turning || v > 1)
			{
				luaL_error(L, "Invalid value %f", v);
			}
			lua_pop(L, 1);
		}
		return true;
	}
	static bool AddAmountsValid(lua_State* L, int index)
	{
		return TurningPointsValid(L, index);
	}
	FLOAT_TABLE_INTERFACE(HasteTurningPoints, HasteTurningPoints, TurningPointsValid);
	FLOAT_TABLE_INTERFACE(HasteAddAmounts, HasteAddAmounts, AddAmountsValid);
	FLOAT_NO_SPEED_INTERFACE(HasteTimeBetweenUpdates, HasteTimeBetweenUpdates, (v > 0));
	FLOAT_NO_SPEED_INTERFACE(HasteLifeSwitchPoint, HasteLifeSwitchPoint, (v >= 0 && v <= 1));
	static int begin_backing_out(T* p, lua_State* L)
	{
		p->BeginBackingOutFromGameplay();
		COMMON_RETURN_SELF;
	}
	static int GetTrueBPS(T* p, lua_State* L)
	{
		PlayerNumber pn= Enum::Check<PlayerNumber>(L, 1);
		float haste= p->GetHasteRate();
		float rate= GAMESTATE->m_SongOptions.GetCurrent().m_fMusicRate;
		float bps= GAMESTATE->m_pPlayerState[pn]->m_Position.m_fCurBPS;
		float true_bps= haste * rate * bps;
		lua_pushnumber(L, true_bps);
		return 1;
	}

	LunaScreenGameplay()
	{
		ADD_METHOD( GetNextCourseSong );
		ADD_METHOD( Center1Player );
		ADD_METHOD( GetLifeMeter );
		ADD_METHOD( GetPlayerInfo );
		ADD_METHOD( GetDummyPlayerInfo );
		// sm-ssc additions:
		ADD_METHOD( PauseGame );
		ADD_METHOD( IsPaused );
		ADD_METHOD( GetHasteRate );
		ADD_METHOD( HasteTurningPoints );
		ADD_METHOD( HasteAddAmounts );
		ADD_METHOD( HasteTimeBetweenUpdates );
		ADD_METHOD( HasteLifeSwitchPoint );
		ADD_METHOD(begin_backing_out);
		ADD_METHOD( GetTrueBPS );
	}
};

LUA_REGISTER_DERIVED_CLASS( ScreenGameplay, ScreenWithMenuElements )


/** @brief Allow Lua to have access to the PlayerInfo. */
class LunaPlayerInfo: public Luna<PlayerInfo>
{
public:
	static int GetLifeMeter( T* p, lua_State *L )
	{
		if(p->m_pLifeMeter)
		{
			p->m_pLifeMeter->PushSelf(L);
			return 1;
		}
		return 0;
	}

	static int GetStepsQueueWrapped( T* p, lua_State *L )
	{
		int iIndex = IArg(1);
		iIndex %= p->m_vpStepsQueue.size();
		Steps *pSteps = p->m_vpStepsQueue[iIndex];
		pSteps->PushSelf(L);
		return 1;
	}

	LunaPlayerInfo()
	{
		ADD_METHOD( GetLifeMeter );
		ADD_METHOD( GetStepsQueueWrapped );
	}
};

LUA_REGISTER_CLASS( PlayerInfo )
// lua end

/*
 * (c) 2001-2004 Chris Danford, Glenn Maynard
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
