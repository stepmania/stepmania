#include "global.h"
#include "GameState.h"
#include "Actor.h"
#include "ActorUtil.h"
#include "AdjustSync.h"
#include "AnnouncerManager.h"
#include "Bookkeeper.h"
#include "Character.h"
#include "CharacterManager.h"
#include "CommonMetrics.h"
#include "Course.h"
#include "CryptManager.h"
#include "Game.h"
#include "GameCommand.h"
#include "GameConstantsAndTypes.h"
#include "GameManager.h"
#include "GamePreferences.h"
#include "HighScore.h"
#include "LightsManager.h"
#include "LuaReference.h"
#include "MessageManager.h"
#include "MemoryCardManager.h"
#include "NoteData.h"
#include "NoteSkinManager.h"
#include "PlayerState.h"
#include "PrefsManager.h"
#include "Profile.h"
#include "ProfileManager.h"
#include "RageFile.h"
#include "RageFileManager.h"
#include "RageLog.h"
#include "RageUtil.h"
#include "Song.h"
#include "SongManager.h"
#include "SongUtil.h"
#include "StatsManager.h"
#include "StepMania.h"
#include "Steps.h"
#include "Style.h"
#include "ThemeManager.h"
#include "UnlockManager.h"
#include "ScreenManager.h"
#include "Screen.h"

#include <ctime>
#include <set>

GameState*	GAMESTATE = nullptr;	// global and accessible from anywhere in our program

#define NAME_BLACKLIST_FILE "/Data/NamesBlacklist.txt"

class GameStateMessageHandler: public MessageSubscriber
{
	void HandleMessage( const Message &msg )
	{
		if( msg.GetName() == "RefreshCreditText" )
		{
			RString sJoined;
			FOREACH_HumanPlayer( pn )
			{
				if( sJoined != "" )
					sJoined += ", ";
				sJoined += ssprintf( "P%i", pn+1 );
			}

			if( sJoined == "" )
				sJoined = "none";

			LOG->MapLog( "JOINED", "Players joined: %s", sJoined.c_str() );
		}
	}
};

struct GameStateImpl
{
	GameStateMessageHandler m_Subscriber;
	GameStateImpl()
	{
		m_Subscriber.SubscribeToMessage( "RefreshCreditText" );
	}
};
static GameStateImpl *g_pImpl = nullptr;

ThemeMetric<bool> ALLOW_LATE_JOIN("GameState","AllowLateJoin");
ThemeMetric<bool> USE_NAME_BLACKLIST("GameState","UseNameBlacklist");

ThemeMetric<RString> DEFAULT_SORT	("GameState","DefaultSort");
SortOrder GetDefaultSort()
{
	return StringToSortOrder( DEFAULT_SORT );
}
ThemeMetric<RString> DEFAULT_SONG	("GameState","DefaultSong");
Song* GameState::GetDefaultSong() const
{
	SongID sid;
	sid.FromString( DEFAULT_SONG );
	return sid.ToSong();
}

static const ThemeMetric<bool> EDIT_ALLOWED_FOR_EXTRA ("GameState","EditAllowedForExtra");
static const ThemeMetric<Difficulty> MIN_DIFFICULTY_FOR_EXTRA	("GameState","MinDifficultyForExtra");
static const ThemeMetric<Grade> GRADE_TIER_FOR_EXTRA_1	("GameState","GradeTierForExtra1");
static const ThemeMetric<bool> ALLOW_EXTRA_2		("GameState","AllowExtra2");
static const ThemeMetric<Grade> GRADE_TIER_FOR_EXTRA_2	("GameState","GradeTierForExtra2");

static ThemeMetric<bool> ARE_STAGE_PLAYER_MODS_FORCED	("GameState","AreStagePlayerModsForced");
static ThemeMetric<bool> ARE_STAGE_SONG_MODS_FORCED	("GameState","AreStageSongModsForced");

static Preference<Premium> g_Premium( "Premium", Premium_DoubleFor1Credit );
Preference<bool> GameState::m_bAutoJoin( "AutoJoin", false );

GameState::GameState() :
	processedTiming(nullptr),
	m_pCurGame(				Message_CurrentGameChanged ),
	m_pCurStyle(			Message_CurrentStyleChanged ),
	m_PlayMode(				Message_PlayModeChanged ),
	m_iCoins(				Message_CoinsChanged ),
	m_sPreferredSongGroup(	Message_PreferredSongGroupChanged ),
	m_sPreferredCourseGroup(	Message_PreferredCourseGroupChanged ),
	m_PreferredStepsType(	Message_PreferredStepsTypeChanged ),
	m_PreferredDifficulty(	Message_PreferredDifficultyP1Changed ),
	m_PreferredCourseDifficulty(	Message_PreferredCourseDifficultyP1Changed ),
	m_SortOrder(			Message_SortOrderChanged ),
	m_pCurSong(				Message_CurrentSongChanged ),
	m_pCurSteps(			Message_CurrentStepsP1Changed ),
	m_pCurCourse(			Message_CurrentCourseChanged ),
	m_pCurTrail(			Message_CurrentTrailP1Changed ),
	m_bGameplayLeadIn(		Message_GameplayLeadInChanged ),
	m_bDidModeChangeNoteSkin(	false ),
	m_bIsUsingStepTiming(		true ),
	m_bInStepEditor(		false ),
	m_stEdit(				Message_EditStepsTypeChanged ),
	m_cdEdit(				Message_EditCourseDifficultyChanged ),
	m_pEditSourceSteps(		Message_EditSourceStepsChanged ),
	m_stEditSource(			Message_EditSourceStepsTypeChanged ),
	m_iEditCourseEntryIndex(	Message_EditCourseEntryIndexChanged ),
	m_sEditLocalProfileID(		Message_EditLocalProfileIDChanged )
{
	g_pImpl = new GameStateImpl;

	m_pCurStyle.Set(nullptr);
	FOREACH_PlayerNumber(rpn)
	{
		m_SeparatedStyles[rpn]= nullptr;
	}

	m_pCurGame.Set(nullptr);
	m_iCoins.Set( 0 );
	m_timeGameStarted.SetZero();
	m_bDemonstrationOrJukebox = false;

	m_iNumTimesThroughAttract = -1;	// initial screen will bump this up to 0
	m_iStageSeed = m_iGameSeed = 0;

	m_PlayMode.Set( PlayMode_Invalid ); // used by IsPlayerEnabled before the first screen
	FOREACH_PlayerNumber( p )
		m_bSideIsJoined[p] = false; // used by GetNumSidesJoined before the first screen

	FOREACH_PlayerNumber( p )
	{
		m_pPlayerState[p] = new PlayerState;
		m_pPlayerState[p]->SetPlayerNumber(p);
	}
	FOREACH_MultiPlayer( p )
	{
		m_pMultiPlayerState[p] = new PlayerState;
		m_pMultiPlayerState[p]->SetPlayerNumber(PLAYER_1);
		m_pMultiPlayerState[p]->m_mp = p;
	}

	m_Environment = new LuaTable;

	m_bDopefish = false;

	sExpandedSectionName = "";

	// Don't reset yet; let the first screen do it, so we can use PREFSMAN and THEME.
	//Reset();

	// Register with Lua.
	{
		Lua *L = LUA->Get();
		lua_pushstring( L, "GAMESTATE" );
		this->PushSelf( L );
		lua_settable( L, LUA_GLOBALSINDEX );
		LUA->Release( L );
	}
}

GameState::~GameState()
{
	// Unregister with Lua.
	LUA->UnsetGlobal( "GAMESTATE" );

	FOREACH_PlayerNumber( p )
		SAFE_DELETE( m_pPlayerState[p] );
	FOREACH_MultiPlayer( p )
		SAFE_DELETE( m_pMultiPlayerState[p] );

	SAFE_DELETE( m_Environment );
	SAFE_DELETE( g_pImpl );
	SAFE_DELETE( processedTiming );
}

PlayerNumber GameState::GetMasterPlayerNumber() const
{
	return this->masterPlayerNumber;
}

void GameState::SetMasterPlayerNumber(const PlayerNumber p)
{
	this->masterPlayerNumber = p;
}

TimingData * GameState::GetProcessedTimingData() const
{
	return this->processedTiming;
}

void GameState::SetProcessedTimingData(TimingData * t)
{
	this->processedTiming = t;
}

void GameState::ApplyGameCommand( const RString &sCommand, PlayerNumber pn )
{
	GameCommand m;
	m.Load( 0, ParseCommands(sCommand) );

	RString sWhy;
	if( !m.IsPlayable(&sWhy) )
	{
		LuaHelpers::ReportScriptErrorFmt("Can't apply GameCommand \"%s\": %s", sCommand.c_str(), sWhy.c_str());
		return;
	}

	if( pn == PLAYER_INVALID )
		m.ApplyToAllPlayers();
	else
		m.Apply( pn );
}

void GameState::ApplyCmdline()
{
	// We need to join players before we can set the style.
	RString sPlayer;
	for( int i = 0; GetCommandlineArgument( "player", &sPlayer, i ); ++i )
	{
		int pn = StringToInt( sPlayer )-1;
		if( !IsAnInt( sPlayer ) || pn < 0 || pn >= NUM_PLAYERS )
			RageException::Throw( "Invalid argument \"--player=%s\".", sPlayer.c_str() );

		JoinPlayer( (PlayerNumber) pn );
	}

	RString sMode;
	for( int i = 0; GetCommandlineArgument( "mode", &sMode, i ); ++i )
	{
		ApplyGameCommand( sMode );
	}
}

void GameState::ResetPlayer( PlayerNumber pn )
{
	m_PreferredStepsType.Set( StepsType_Invalid );
	m_PreferredDifficulty[pn].Set( Difficulty_Invalid );
	m_PreferredCourseDifficulty[pn].Set( Difficulty_Medium );
	m_iPlayerStageTokens[pn] = 0;
	m_iAwardedExtraStages[pn] = 0;
	m_pCurSteps[pn].Set(nullptr);
	m_pCurTrail[pn].Set(nullptr);
	m_pPlayerState[pn]->Reset();
	PROFILEMAN->UnloadProfile( pn );
	ResetPlayerOptions(pn);
}

void GameState::ResetPlayerOptions( PlayerNumber pn )
{
	PlayerOptions po;
	GetDefaultPlayerOptions( po );
	m_pPlayerState[pn]->m_PlayerOptions.Assign( ModsLevel_Preferred, po );
}

void GameState::Reset()
{
	this->SetMasterPlayerNumber(PLAYER_INVALID); // must initialize for UnjoinPlayer

	FOREACH_PlayerNumber( pn )
		UnjoinPlayer( pn );

	ASSERT( THEME != nullptr );

	m_timeGameStarted.SetZero();
	SetCurrentStyle( nullptr, PLAYER_INVALID );
	FOREACH_MultiPlayer( p )
		m_MultiPlayerStatus[p] = MultiPlayerStatus_NotJoined;
	FOREACH_PlayerNumber( pn )
		MEMCARDMAN->UnlockCard( pn );
	m_bMultiplayer = false;
	m_iNumMultiplayerNoteFields = 1;
	*m_Environment = LuaTable();
	m_sPreferredSongGroup.Set( GROUP_ALL );
	m_sPreferredCourseGroup.Set( GROUP_ALL );
	m_bFailTypeWasExplicitlySet = false;
	m_SortOrder.Set( SortOrder_Invalid );
	m_PreferredSortOrder = GetDefaultSort();
	m_PlayMode.Set( PlayMode_Invalid );
	m_EditMode = EditMode_Invalid;
	m_bDemonstrationOrJukebox = false;
	m_bJukeboxUsesModifiers = false;
	m_iCurrentStageIndex = 0;

	m_bGameplayLeadIn.Set( false );
	m_iNumStagesOfThisSong = 0;
	m_bLoadingNextSong = false;

	NOTESKIN->RefreshNoteSkinData( m_pCurGame );

	m_iGameSeed = rand();
	m_iStageSeed = rand();

	m_AdjustTokensBySongCostForFinalStageCheck= true;

	m_pCurSong.Set( GetDefaultSong() );
	m_pPreferredSong = nullptr;
	m_pCurCourse.Set(nullptr);
	m_pPreferredCourse = nullptr;

	FOREACH_MultiPlayer( p )
		m_pMultiPlayerState[p]->Reset();

	m_SongOptions.Init();

	m_paused= false;
	ResetMusicStatistics();
	ResetStageStatistics();
	AdjustSync::ResetOriginalSyncData();

	SONGMAN->UpdatePopular();
	SONGMAN->UpdateShuffled();

	/* We may have cached trails from before everything was loaded (eg. from
	 * before SongManager::UpdatePopular could be called). Erase the cache. */
	SONGMAN->RegenerateNonFixedCourses();

	STATSMAN->Reset();

	FOREACH_PlayerNumber(p)
	{
		if( PREFSMAN->m_ShowDancingCharacters == SDC_Random )
			m_pCurCharacters[p] = CHARMAN->GetRandomCharacter();
		else
			m_pCurCharacters[p] = CHARMAN->GetDefaultCharacter();
		ASSERT( m_pCurCharacters[p] != nullptr );
	}

	m_bTemporaryEventMode = false;

	LIGHTSMAN->SetLightsMode( LIGHTSMODE_ATTRACT );

	m_stEdit.Set( StepsType_Invalid );
	m_pEditSourceSteps.Set(nullptr);
	m_stEditSource.Set( StepsType_Invalid );
	m_iEditCourseEntryIndex.Set( -1 );
	m_sEditLocalProfileID.Set( "" );

	m_bBackedOutOfFinalStage = false;
	m_bEarnedExtraStage = false;
	sExpandedSectionName = "";

	ApplyCmdline();
}

void GameState::JoinPlayer( PlayerNumber pn )
{
	// Make sure the join will be successful before doing it. -Kyz
	{
		int players_joined= 0;
		for(int i= 0; i < NUM_PLAYERS; ++i)
		{
			players_joined+= m_bSideIsJoined[i];
		}
		if(players_joined > 0)
		{
			const Style* cur_style= GetCurrentStyle(PLAYER_INVALID);
			if(cur_style)
			{
				const Style* new_style= GAMEMAN->GetFirstCompatibleStyle(m_pCurGame,
					players_joined + 1, cur_style->m_StepsType);
				if(new_style == nullptr)
				{
					return;
				}
			}
		}
	}
	/* If joint premium and we're not taking away a credit for the 2nd join,
	 * give the new player the same number of stage tokens that the old player
	 * has. */
	if( GetCoinMode() == CoinMode_Pay && GetPremium() == Premium_2PlayersFor1Credit && GetNumSidesJoined() == 1 )
		m_iPlayerStageTokens[pn] = m_iPlayerStageTokens[this->GetMasterPlayerNumber()];
	else
		m_iPlayerStageTokens[pn] = PREFSMAN->m_iSongsPerPlay;

	m_bSideIsJoined[pn] = true;

	if( this->GetMasterPlayerNumber() == PLAYER_INVALID )
		this->SetMasterPlayerNumber(pn);

	// if first player to join, set start time
	if( GetNumSidesJoined() == 1 )
		BeginGame();

	// Count each player join as a play.
	{
		Profile* pMachineProfile = PROFILEMAN->GetMachineProfile();
		pMachineProfile->m_iTotalSessions++;
	}

	// Set the current style to something appropriate for the new number of joined players.
	// beat gametype's versus styles use a different stepstype from its single
	// styles, so when GameCommand tries to join both players for a versus
	// style, it hits the assert when joining the first player.  So if the first
	// player is being joined and the current styletype is for two players,
	// assume that the second player will be joined immediately afterwards and
	// don't try to change the style. -Kyz
	const Style* cur_style= GetCurrentStyle(PLAYER_INVALID);
	if(cur_style != nullptr && !(pn == PLAYER_1 &&
			(cur_style->m_StyleType == StyleType_TwoPlayersTwoSides ||
				cur_style->m_StyleType == StyleType_TwoPlayersSharedSides)))
	{
		const Style *pStyle;
		// Only use one player for StyleType_OnePlayerTwoSides and StepsTypes
		// that can only be played by one player (e.g. dance-solo,
		// dance-threepanel, popn-nine). -aj
		// XXX?: still shows joined player as "Insert Card". May not be an issue? -aj
		if( cur_style->m_StyleType == StyleType_OnePlayerTwoSides ||
			cur_style->m_StepsType == StepsType_dance_solo ||
			cur_style->m_StepsType == StepsType_dance_threepanel ||
			cur_style->m_StepsType == StepsType_popn_nine )
			pStyle = GAMEMAN->GetFirstCompatibleStyle( m_pCurGame, 1, cur_style->m_StepsType );
		else
			pStyle = GAMEMAN->GetFirstCompatibleStyle( m_pCurGame, GetNumSidesJoined(), cur_style->m_StepsType );

		// use SetCurrentStyle in case of StyleType_OnePlayerTwoSides
		SetCurrentStyle( pStyle, pn );
	}

	Message msg( MessageIDToString(Message_PlayerJoined) );
	msg.SetParam( "Player", pn );
	MESSAGEMAN->Broadcast( msg );
}

void GameState::UnjoinPlayer( PlayerNumber pn )
{
	/* Unjoin STATSMAN first, so steps used by this player are released
	 * and can be released by PROFILEMAN. */
	STATSMAN->UnjoinPlayer( pn );
	m_bSideIsJoined[pn] = false;
	m_iPlayerStageTokens[pn] = 0;

	ResetPlayer( pn );

	if( this->GetMasterPlayerNumber() == pn )
	{
		// We can't use GetFirstHumanPlayer() because if both players were joined, GetFirstHumanPlayer() will always return PLAYER_1, even when PLAYER_1 is the player we're unjoining.
		FOREACH_HumanPlayer( hp )
		{
			if( pn != hp )
			{
				this->SetMasterPlayerNumber(hp);
			}
		}
		if( this->GetMasterPlayerNumber() == pn )
		{
			this->SetMasterPlayerNumber(PLAYER_INVALID);
		}
	}

	Message msg( MessageIDToString(Message_PlayerUnjoined) );
	msg.SetParam( "Player", pn );
	MESSAGEMAN->Broadcast( msg );

	// If there are no players left, reset some non-player-specific stuff, too.
	if( this->GetMasterPlayerNumber() == PLAYER_INVALID )
	{
		SongOptions so;
		GetDefaultSongOptions( so );
		m_SongOptions.Assign( ModsLevel_Preferred, so );
		m_bDidModeChangeNoteSkin = false;
	}
}

/* xxx: handle multiplayer join? -aj */

namespace
{
	bool JoinInputInternal( PlayerNumber pn )
	{
		if( !GAMESTATE->PlayersCanJoin() )
			return false;

		// If this side is already in, don't re-join.
		if( GAMESTATE->m_bSideIsJoined[pn] )
			return false;

		// subtract coins
		int iCoinsNeededToJoin = GAMESTATE->GetCoinsNeededToJoin();

		if( GAMESTATE->m_iCoins < iCoinsNeededToJoin )
			return false;	// not enough coins

		GAMESTATE->m_iCoins.Set( GAMESTATE->m_iCoins - iCoinsNeededToJoin );

		GAMESTATE->JoinPlayer( pn );

		// On Join, make sure to update Coins File
		BOOKKEEPER->WriteCoinsFile(GAMESTATE->m_iCoins.Get());

		return true;
	}
};

// Handle an input that can join a player. Return true if the player joined.
bool GameState::JoinInput( PlayerNumber pn )
{
	// When AutoJoin is enabled, join all players on a single start press.
	if( GAMESTATE->m_bAutoJoin.Get() )
		return JoinPlayers();
	else
		return JoinInputInternal( pn );
}

// Attempt to join all players, as if each player pressed Start.
bool GameState::JoinPlayers()
{
	bool bJoined = false;
	FOREACH_PlayerNumber( pn )
	{
		if( JoinInputInternal(pn) )
			bJoined = true;
	}
	return bJoined;
}

int GameState::GetCoinsNeededToJoin() const
{
	int iCoinsToCharge = 0;

	if( GetCoinMode() == CoinMode_Pay )
		iCoinsToCharge = PREFSMAN->m_iCoinsPerCredit;

	// If joint premium, don't take away a credit for the second join.
	if( GetPremium() == Premium_2PlayersFor1Credit  &&
		GetNumSidesJoined() == 1 )
		iCoinsToCharge = 0;

	return iCoinsToCharge;
}

/* Game flow:
 *
 * BeginGame() - the first player has joined; the game is starting.
 *
 * PlayersFinalized() - player memory cards are loaded; later joins won't have memory cards this stage
 *
 * BeginStage() - gameplay is beginning
 *
 * optional: CancelStage() - gameplay aborted (Back pressed), undo BeginStage and back up
 *
 * CommitStageStats() - gameplay is finished
 *   Saves STATSMAN->m_CurStageStats to the profiles, so profile information
 *   is up-to-date for Evaluation.
 *
 * FinishStage() - gameplay and evaluation is finished
 *   Clears data which was stored by CommitStageStats. */
void GameState::BeginGame()
{
	m_timeGameStarted.Touch();

	m_vpsNamesThatWereFilled.clear();

	// Play attract on the ending screen, then on the ranking screen
	// even if attract sounds are set to off.
	m_iNumTimesThroughAttract = -1;

	FOREACH_PlayerNumber( pn )
		MEMCARDMAN->UnlockCard( pn );
}

void GameState::LoadProfiles( bool bLoadEdits )
{
	// Unlock any cards that we might want to load.
	FOREACH_HumanPlayer( pn )
		if( !PROFILEMAN->IsPersistentProfile(pn) )
			MEMCARDMAN->UnlockCard( pn );

	MEMCARDMAN->WaitForCheckingToComplete();

	FOREACH_HumanPlayer( pn )
	{
		// If a profile is already loaded, this was already called.
		if( PROFILEMAN->IsPersistentProfile(pn) )
			continue;

		MEMCARDMAN->MountCard( pn );
		bool bSuccess = PROFILEMAN->LoadFirstAvailableProfile( pn, bLoadEdits );	// load full profile
		MEMCARDMAN->UnmountCard( pn );

		if( !bSuccess )
			continue;

		// Lock the card on successful load, so we won't allow it to be changed.
		MEMCARDMAN->LockCard( pn );

		LoadCurrentSettingsFromProfile( pn );

		Profile* pPlayerProfile = PROFILEMAN->GetProfile( pn );
		if( pPlayerProfile )
			pPlayerProfile->m_iTotalSessions++;
	}
}

void GameState::SavePlayerProfiles()
{
	FOREACH_HumanPlayer( pn )
		SavePlayerProfile( pn );
}

void GameState::SavePlayerProfile( PlayerNumber pn )
{
	if( !PROFILEMAN->IsPersistentProfile(pn) )
		return;

	// AutoplayCPU should not save scores. -aj
	// xxx: this MAY cause issues with Multiplayer. However, without a working
	// Multiplayer build, we'll never know. -aj
	if( m_pPlayerState[pn]->m_PlayerController != PC_HUMAN )
		return;

	bool bWasMemoryCard = PROFILEMAN->ProfileWasLoadedFromMemoryCard(pn);
	if( bWasMemoryCard )
		MEMCARDMAN->MountCard( pn );
	PROFILEMAN->SaveProfile( pn );
	if( bWasMemoryCard )
		MEMCARDMAN->UnmountCard( pn );
}

bool GameState::HaveProfileToLoad()
{
	FOREACH_HumanPlayer( pn )
	{
		// We won't load this profile if it's already loaded.
		if( PROFILEMAN->IsPersistentProfile(pn) )
			continue;

		// If a memory card is inserted, we'l try to load it.
		if( MEMCARDMAN->CardInserted(pn) )
			return true;
		if( !PROFILEMAN->m_sDefaultLocalProfileID[pn].Get().empty() )
			return true;
	}

	return false;
}

bool GameState::HaveProfileToSave()
{
	FOREACH_HumanPlayer( pn )
		if( PROFILEMAN->IsPersistentProfile(pn) )
			return true;
	return false;
}

void GameState::SaveLocalData()
{
	BOOKKEEPER->WriteToDisk();
	PROFILEMAN->SaveMachineProfile();
}

int GameState::GetNumStagesMultiplierForSong( const Song* pSong )
{
	int iNumStages = 1;

	ASSERT( pSong != nullptr );
	if( pSong->IsMarathon() )
		iNumStages *= 3;
	if( pSong->IsLong() )
		iNumStages *= 2;

	return iNumStages;
}

int GameState::GetNumStagesForCurrentSongAndStepsOrCourse() const
{
	int iNumStagesOfThisSong = 1;
	if( m_pCurSong )
	{
		/* Extra stages need to only count as one stage in case a multi-stage
		 * song is chosen. */
		if( IsAnExtraStage() )
			iNumStagesOfThisSong = 1;
		else
			iNumStagesOfThisSong = GameState::GetNumStagesMultiplierForSong( m_pCurSong );
	}
	else if( m_pCurCourse )
		iNumStagesOfThisSong = PREFSMAN->m_iSongsPerPlay;
	else
		return -1;

	iNumStagesOfThisSong = max( iNumStagesOfThisSong, 1 );

	return iNumStagesOfThisSong;
}

// Called by ScreenGameplay. Set the length of the current song.
void GameState::BeginStage()
{
	if( m_bDemonstrationOrJukebox )
		return;

	// This should only be called once per stage.
	if( m_iNumStagesOfThisSong != 0 )
		LOG->Warn( "XXX: m_iNumStagesOfThisSong == %i?", m_iNumStagesOfThisSong );

	ResetStageStatistics();
	AdjustSync::ResetOriginalSyncData();

	if( !ARE_STAGE_PLAYER_MODS_FORCED )
	{
		FOREACH_PlayerNumber( p )
		{
			ModsGroup<PlayerOptions> &po = m_pPlayerState[p]->m_PlayerOptions;
			po.Assign(ModsLevel_Stage,
					  m_pPlayerState[p]->m_PlayerOptions.GetPreferred());
		}
	}
	if( !ARE_STAGE_SONG_MODS_FORCED )
		m_SongOptions.Assign( ModsLevel_Stage, m_SongOptions.GetPreferred() );

	STATSMAN->m_CurStageStats.m_fMusicRate = m_SongOptions.GetSong().m_fMusicRate;
	m_iNumStagesOfThisSong = GetNumStagesForCurrentSongAndStepsOrCourse();
	ASSERT( m_iNumStagesOfThisSong != -1 );
	FOREACH_EnabledPlayer( p )
	{
		// only do this check with human players, assume CPU players (Rave)
		// always have tokens. -aj (this could probably be moved below, even.)
		if( !IsEventMode() && !IsCpuPlayer(p) )
		{
			if(m_iPlayerStageTokens[p] < m_iNumStagesOfThisSong)
			{
				LuaHelpers::ReportScriptErrorFmt("Player %d only has %d stage tokens, but needs %d.", p, m_iPlayerStageTokens[p], m_iNumStagesOfThisSong);
			}
		}
		m_iPlayerStageTokens[p] -= m_iNumStagesOfThisSong;
	}
	FOREACH_HumanPlayer( pn )
		if( CurrentOptionsDisqualifyPlayer(pn) )
			STATSMAN->m_CurStageStats.m_player[pn].m_bDisqualified = true;
	m_bEarnedExtraStage = false;
	m_sStageGUID = CryptManager::GenerateRandomUUID();
}

void GameState::CancelStage()
{
	FOREACH_CpuPlayer( p )
	{
		switch( m_PlayMode )
		{
			case PLAY_MODE_BATTLE:
			case PLAY_MODE_RAVE:
				m_iPlayerStageTokens[p] = PREFSMAN->m_iSongsPerPlay;
			default:
				break;
		}
	}

	FOREACH_EnabledPlayer( p )
		m_iPlayerStageTokens[p] += m_iNumStagesOfThisSong;
	m_iNumStagesOfThisSong = 0;
	ResetStageStatistics();
}

void GameState::CommitStageStats()
{
	if( m_bDemonstrationOrJukebox )
		return;

	STATSMAN->CommitStatsToProfiles( &STATSMAN->m_CurStageStats );

	// Update TotalPlaySeconds.
	int iPlaySeconds = max( 0, (int) m_timeGameStarted.GetDeltaTime() );

	Profile* pMachineProfile = PROFILEMAN->GetMachineProfile();
	pMachineProfile->m_iTotalSessionSeconds += iPlaySeconds;

	FOREACH_HumanPlayer( p )
	{
		Profile* pPlayerProfile = PROFILEMAN->GetProfile( p );
		if( pPlayerProfile )
			pPlayerProfile->m_iTotalSessionSeconds += iPlaySeconds;
	}
}

/* Called by ScreenSelectMusic (etc). Increment the stage counter if we just
 * played a song. Might be called more than once. */
void GameState::FinishStage()
{
	// Increment the stage counter.
	const int iOldStageIndex = m_iCurrentStageIndex;
	++m_iCurrentStageIndex;

	m_iNumStagesOfThisSong = 0;

	EarnedExtraStage e = CalculateEarnedExtraStage();
	STATSMAN->m_CurStageStats.m_EarnedExtraStage = e;
	if( e != EarnedExtraStage_No )
	{
		LOG->Trace( "awarded extra stage" );
		FOREACH_HumanPlayer( p )
		{
			// todo: unhardcode the extra stage limit? -aj
			if( m_iAwardedExtraStages[p] < 2 )
			{
				++m_iAwardedExtraStages[p];
				++m_iPlayerStageTokens[p];
				m_bEarnedExtraStage = true;
			}
		}
	}

	// Save the current combo to the profiles so it can be used for ComboContinuesBetweenSongs.
	FOREACH_HumanPlayer( p )
	{
		Profile* pProfile = PROFILEMAN->GetProfile(p);
		pProfile->m_iCurrentCombo = STATSMAN->m_CurStageStats.m_player[p].m_iCurCombo;
	}

	if( m_bDemonstrationOrJukebox )
		return;

	// todo: simplify. profile saving is accomplished in ScreenProfileSave
	// now; all this code does differently is save machine profile as well. -aj
	if( IsEventMode() )
	{
		const int iSaveProfileEvery = 3;
		if( iOldStageIndex/iSaveProfileEvery < m_iCurrentStageIndex/iSaveProfileEvery )
		{
			LOG->Trace( "Played %i stages; saving profiles ...", iSaveProfileEvery );
			PROFILEMAN->SaveMachineProfile();
			this->SavePlayerProfiles();
		}
	}
}

void GameState::LoadCurrentSettingsFromProfile( PlayerNumber pn )
{
	if( !PROFILEMAN->IsPersistentProfile(pn) )
		return;

	const Profile *pProfile = PROFILEMAN->GetProfile(pn);

	// apply saved default modifiers if any
	RString sModifiers;
	if( pProfile->GetDefaultModifiers( m_pCurGame, sModifiers ) )
	{
		/* We don't save negative preferences (eg. "no reverse"). If the theme
		 * sets a default of "reverse", and the player turns it off, we should
		 * set it off. However, don't reset modifiers that aren't saved by the
		 * profile, so we don't ignore unsaved modifiers when a profile is in use. */
		PO_GROUP_CALL( m_pPlayerState[pn]->m_PlayerOptions, ModsLevel_Preferred, ResetSavedPrefs );
		ApplyPreferredModifiers( pn, sModifiers );
	}
	// Only set the sort order if it wasn't already set by a GameCommand (or by an earlier profile)
	if( m_PreferredSortOrder == SortOrder_Invalid  &&  pProfile->m_SortOrder != SortOrder_Invalid )
		m_PreferredSortOrder = pProfile->m_SortOrder;
	if( pProfile->m_LastDifficulty != Difficulty_Invalid )
		m_PreferredDifficulty[pn].Set( pProfile->m_LastDifficulty );
	if( pProfile->m_LastCourseDifficulty != Difficulty_Invalid )
		m_PreferredCourseDifficulty[pn].Set( pProfile->m_LastCourseDifficulty );
	// Only set the PreferredStepsType if it wasn't already set by a GameCommand (or by an earlier profile)
	if( m_PreferredStepsType == StepsType_Invalid  &&  pProfile->m_LastStepsType != StepsType_Invalid )
		m_PreferredStepsType.Set( pProfile->m_LastStepsType );
	if( m_pPreferredSong == nullptr )
		m_pPreferredSong = pProfile->m_lastSong.ToSong();
	if( m_pPreferredCourse == nullptr )
		m_pPreferredCourse = pProfile->m_lastCourse.ToCourse();
}

void GameState::SaveCurrentSettingsToProfile( PlayerNumber pn )
{
	if( !PROFILEMAN->IsPersistentProfile(pn) )
		return;
	if( m_bDemonstrationOrJukebox )
		return;

	Profile* pProfile = PROFILEMAN->GetProfile(pn);

	pProfile->SetDefaultModifiers( m_pCurGame, m_pPlayerState[pn]->m_PlayerOptions.GetPreferred().GetSavedPrefsString() );
	if( IsSongSort(m_PreferredSortOrder) )
		pProfile->m_SortOrder = m_PreferredSortOrder;
	if( m_PreferredDifficulty[pn] != Difficulty_Invalid )
		pProfile->m_LastDifficulty = m_PreferredDifficulty[pn];
	if( m_PreferredCourseDifficulty[pn] != Difficulty_Invalid )
		pProfile->m_LastCourseDifficulty = m_PreferredCourseDifficulty[pn];
	if( m_PreferredStepsType != StepsType_Invalid )
		pProfile->m_LastStepsType = m_PreferredStepsType;
	if( m_pPreferredSong )
		pProfile->m_lastSong.FromSong( m_pPreferredSong );
	if( m_pPreferredCourse )
		pProfile->m_lastCourse.FromCourse( m_pPreferredCourse );
}

bool GameState::CanSafelyEnterGameplay(RString& reason)
{
	if(!IsCourseMode())
	{
		Song const* song= m_pCurSong;
		if(song == nullptr)
		{
			reason= "Current song is null.";
			return false;
		}
	}
	else
	{
		Course const* song= m_pCurCourse;
		if(song == nullptr)
		{
			reason= "Current course is null.";
			return false;
		}
	}
	FOREACH_EnabledPlayer(pn)
	{
		Style const* style= GetCurrentStyle(pn);
		if(style == nullptr)
		{
			reason= ssprintf("Style for player %d is null.", pn+1);
			return false;
		}
		if(!IsCourseMode())
		{
			Steps const* steps= m_pCurSteps[pn];
			if(steps == nullptr)
			{
				reason= ssprintf("Steps for player %d is null.", pn+1);
				return false;
			}
			if(steps->m_StepsType != style->m_StepsType)
			{
				reason= ssprintf("Player %d StepsType %s for steps does not equal "
					"StepsType %s for style.", pn+1,
					GAMEMAN->GetStepsTypeInfo(steps->m_StepsType).szName,
					GAMEMAN->GetStepsTypeInfo(style->m_StepsType).szName);
				return false;
			}
			if(steps->m_pSong != m_pCurSong)
			{
				reason= ssprintf("Steps for player %d are not for the current song.",
					pn+1);
				return false;
			}
			NoteData ndtemp;
			steps->GetNoteData(ndtemp);
			if(ndtemp.GetNumTracks() != style->m_iColsPerPlayer)
			{
				reason= ssprintf("Steps for player %d have %d columns, style has %d "
					"columns.", pn+1, ndtemp.GetNumTracks(), style->m_iColsPerPlayer);
				return false;
			}
		}
		else
		{
			Trail const* steps= m_pCurTrail[pn];
			if(steps == nullptr)
			{
				reason= ssprintf("Steps for player %d is null.", pn+1);
				return false;
			}
			if(steps->m_StepsType != style->m_StepsType)
			{
				reason= ssprintf("Player %d StepsType %s for steps does not equal "
					"StepsType %s for style.", pn+1,
					GAMEMAN->GetStepsTypeInfo(steps->m_StepsType).szName,
					GAMEMAN->GetStepsTypeInfo(style->m_StepsType).szName);
				return false;
			}
		}
	}
	return true;
}

void GameState::SetCompatibleStylesForPlayers()
{
	bool style_set= false;
	if(IsCourseMode())
	{
		if(m_pCurCourse != nullptr)
		{
			const Style* style= m_pCurCourse->GetCourseStyle(m_pCurGame, GetNumSidesJoined());
			if(style != nullptr)
			{
				style_set= true;
				SetCurrentStyle(style, PLAYER_INVALID);
			}
		}
		else if(GetCurrentStyle(PLAYER_INVALID) == nullptr)
		{
			vector<StepsType> vst;
			GAMEMAN->GetStepsTypesForGame(m_pCurGame, vst);
			const Style *style = GAMEMAN->GetFirstCompatibleStyle(
				m_pCurGame, GetNumSidesJoined(), vst[0]);
			SetCurrentStyle(style, PLAYER_INVALID);
		}
	}
	if(!style_set)
	{
		FOREACH_EnabledPlayer(pn)
		{
			StepsType st= StepsType_Invalid;
			if(m_pCurSteps[pn] != nullptr)
			{
				st= m_pCurSteps[pn]->m_StepsType;
			}
			else if(m_pCurTrail[pn] != nullptr)
			{
				st= m_pCurTrail[pn]->m_StepsType;
			}
			else
			{
				vector<StepsType> vst;
				GAMEMAN->GetStepsTypesForGame(m_pCurGame, vst);
				st= vst[0];
			}
			const Style *style = GAMEMAN->GetFirstCompatibleStyle(
				m_pCurGame, GetNumSidesJoined(), st);
			SetCurrentStyle(style, pn);
		}
	}
}

void GameState::ForceSharedSidesMatch()
{
	PlayerNumber pn_with_shared= PLAYER_INVALID;
	const Style* shared_style= nullptr;
	FOREACH_EnabledPlayer(pn)
	{
		const Style* style= GetCurrentStyle(pn);
		ASSERT_M(style != nullptr, "Style being null should not be possible.");
		if(style->m_StyleType == StyleType_TwoPlayersSharedSides)
		{
			pn_with_shared= pn;
			shared_style= style;
		}
	}
	if(pn_with_shared != PLAYER_INVALID)
	{
		ASSERT_M(GetNumPlayersEnabled() == 2, "2 players must be enabled for shared sides.");
		PlayerNumber other_pn= OPPOSITE_PLAYER[pn_with_shared];
		const Style* other_style= GetCurrentStyle(other_pn);
		ASSERT_M(other_style != nullptr, "Other player's style being null should not be possible.");
		if(other_style->m_StyleType != StyleType_TwoPlayersSharedSides)
		{
			SetCurrentStyle(shared_style, other_pn);
			if(IsCourseMode())
			{
				m_pCurTrail[other_pn].Set(m_pCurTrail[pn_with_shared]);
			}
			else
			{
				m_pCurSteps[other_pn].Set(m_pCurSteps[pn_with_shared]);
			}
		}
	}
}

void GameState::ForceOtherPlayersToCompatibleSteps(PlayerNumber main)
{
	if(IsCourseMode())
	{
		Trail* steps_to_match= m_pCurTrail[main].Get();
		if(steps_to_match == nullptr) { return; }
		int num_players= GAMESTATE->GetNumPlayersEnabled();
		StyleType styletype_to_match= GAMEMAN->GetFirstCompatibleStyle(
			GAMESTATE->GetCurrentGame(), num_players, steps_to_match->m_StepsType)
			->m_StyleType;
		FOREACH_EnabledPlayer(pn)
		{
			Trail* pn_steps= m_pCurTrail[pn].Get();
			bool match_failed= pn_steps == nullptr;
			if(steps_to_match != pn_steps && pn_steps != nullptr)
			{
				StyleType pn_styletype= GAMEMAN->GetFirstCompatibleStyle(
					GAMESTATE->GetCurrentGame(), num_players, pn_steps->m_StepsType)
					->m_StyleType;
				if(styletype_to_match == StyleType_TwoPlayersSharedSides ||
					pn_styletype == StyleType_TwoPlayersSharedSides)
				{
					match_failed= true;
				}
			}
			if(match_failed)
			{
				m_pCurTrail[pn].Set(steps_to_match);
			}
		}
	}
	else
	{
		Steps* steps_to_match= m_pCurSteps[main].Get();
		if(steps_to_match == nullptr) { return; }
		int num_players= GAMESTATE->GetNumPlayersEnabled();
		StyleType styletype_to_match= GAMEMAN->GetFirstCompatibleStyle(
			GAMESTATE->GetCurrentGame(), num_players, steps_to_match->m_StepsType)
			->m_StyleType;
		RString music_to_match= steps_to_match->GetMusicFile();
		FOREACH_EnabledPlayer(pn)
		{
			Steps* pn_steps= m_pCurSteps[pn].Get();
			bool match_failed= pn_steps == nullptr;
			if(steps_to_match != pn_steps && pn_steps != nullptr)
			{
				StyleType pn_styletype= GAMEMAN->GetFirstCompatibleStyle(
					GAMESTATE->GetCurrentGame(), num_players, pn_steps->m_StepsType)
					->m_StyleType;
				if(styletype_to_match == StyleType_TwoPlayersSharedSides ||
					pn_styletype == StyleType_TwoPlayersSharedSides)
				{
					match_failed= true;
				}
				if(music_to_match != pn_steps->GetMusicFile())
				{
					match_failed= true;
				}
			}
			if(match_failed)
			{
				m_pCurSteps[pn].Set(steps_to_match);
			}
		}
	}
}

void GameState::Update( float fDelta )
{
	m_SongOptions.Update( fDelta );

	FOREACH_PlayerNumber( p )
	{
		m_pPlayerState[p]->Update( fDelta );

		if( !m_bGoalComplete[p] && IsGoalComplete(p) )
		{
			m_bGoalComplete[p] = true;
			MESSAGEMAN->Broadcast( (MessageID)(Message_GoalCompleteP1+p) );
		}
	}

	if( GAMESTATE->m_pCurCourse )
	{
		if( GAMESTATE->m_pCurCourse->m_fGoalSeconds > 0  &&  !m_bWorkoutGoalComplete )
		{
			const StageStats &ssCurrent = STATSMAN->m_CurStageStats;
			bool bGoalComplete = ssCurrent.m_fGameplaySeconds > GAMESTATE->m_pCurCourse->m_fGoalSeconds;
			if( bGoalComplete )
			{
				MESSAGEMAN->Broadcast( "WorkoutGoalComplete" );
				m_bWorkoutGoalComplete = true;
			}
		}
	}
}

void GameState::SetCurGame( const Game *pGame )
{
	m_pCurGame.Set( pGame );
	RString sGame = pGame ? RString(pGame->m_szName) : RString();
	PREFSMAN->SetCurrentGame( sGame );
}

const float GameState::MUSIC_SECONDS_INVALID = -5000.0f;

void GameState::ResetMusicStatistics()
{
	m_Position.Reset();
	m_LastPositionTimer.Touch();
	m_LastPositionSeconds = 0.0f;

	Actor::SetBGMTime( 0, 0, 0, 0 );

	FOREACH_PlayerNumber( p )
	{
		m_pPlayerState[p]->m_Position.Reset();
	}
}

void GameState::ResetStageStatistics()
{
	StageStats OldStats = STATSMAN->m_CurStageStats;
	STATSMAN->m_CurStageStats = StageStats();
	if( PREFSMAN->m_bComboContinuesBetweenSongs )
	{
		FOREACH_PlayerNumber( p )
		{
			bool FirstSong = m_iCurrentStageIndex == 0;
			if( FirstSong )
			{
				Profile* pProfile = PROFILEMAN->GetProfile(p);
				STATSMAN->m_CurStageStats.m_player[p].m_iCurCombo = pProfile->m_iCurrentCombo;
			}
			else
			{
				STATSMAN->m_CurStageStats.m_player[p].m_iCurCombo = OldStats.m_player[p].m_iCurCombo;
			}
		}
	}

	RemoveAllActiveAttacks();
	FOREACH_PlayerNumber( p )
		m_pPlayerState[p]->RemoveAllInventory();
	m_fOpponentHealthPercent = 1;
	m_fHasteRate = 0;
	m_fLastHasteUpdateMusicSeconds = 0;
	m_fAccumulatedHasteSeconds = 0;
	m_fTugLifePercentP1 = 0.5f;
	FOREACH_PlayerNumber( p )
	{
		m_pPlayerState[p]->m_fSuperMeter = 0;
		m_pPlayerState[p]->m_HealthState = HealthState_Alive;

		m_pPlayerState[p]->m_iLastPositiveSumOfAttackLevels = 0;
		m_pPlayerState[p]->m_fSecondsUntilAttacksPhasedOut = 0;	// PlayerAI not affected

		m_bGoalComplete[p] = false;
	}
	m_bWorkoutGoalComplete = false;


	FOREACH_PlayerNumber( p )
	{
		m_vLastStageAwards[p].clear();
		m_vLastPeakComboAwards[p].clear();
	}

	// Reset the round seed. Do this here and not in FinishStage so that players
	// get new shuffle patterns if they Back out of gameplay and play again.
	m_iStageSeed = rand();
}

void GameState::UpdateSongPosition( float fPositionSeconds, const TimingData &timing, const RageTimer &timestamp )
{
	/* It's not uncommon to get a lot of duplicated positions from the sound
	 * driver, like so: 13.120953,13.130975,13.130975,13.130975,13.140998,...
	 * This causes visual stuttering of the arrows. To compensate, keep a
	 * RageTimer since the last change. */
	if(fPositionSeconds == m_LastPositionSeconds && !m_paused)
		fPositionSeconds += m_LastPositionTimer.Ago();
	else
	{
		//LOG->Info("Time difference: %+f",
		//	m_LastPositionTimer.Ago() - (fPositionSeconds - m_LastPositionSeconds)
		//);
		m_LastPositionTimer.Touch();
		m_LastPositionSeconds = fPositionSeconds;
	}

	m_Position.UpdateSongPosition( fPositionSeconds, timing, timestamp );

	FOREACH_EnabledPlayer( pn )
	{
		if( m_pCurSteps[pn] )
		{
			m_pPlayerState[pn]->m_Position.UpdateSongPosition( fPositionSeconds, *m_pCurSteps[pn]->GetTimingData(), timestamp );
			Actor::SetPlayerBGMBeat( pn, m_pPlayerState[pn]->m_Position.m_fSongBeatVisible, m_pPlayerState[pn]->m_Position.m_fSongBeatNoOffset );
		}
	}
	Actor::SetBGMTime( GAMESTATE->m_Position.m_fMusicSecondsVisible, GAMESTATE->m_Position.m_fSongBeatVisible, fPositionSeconds, GAMESTATE->m_Position.m_fSongBeatNoOffset );
//	LOG->Trace( "m_fMusicSeconds = %f, m_fSongBeat = %f, m_fCurBPS = %f, m_bFreeze = %f", m_fMusicSeconds, m_fSongBeat, m_fCurBPS, m_bFreeze );
}

/*
update player position code goes here
 */

float GameState::GetSongPercent( float beat ) const
{
	// 0 = first step; 1 = last step
	float curTime = this->m_pCurSong->m_SongTiming.GetElapsedTimeFromBeat(beat);
	return (curTime - m_pCurSong->GetFirstSecond()) / m_pCurSong->GetLastSecond();
}

int GameState::GetNumStagesLeft( PlayerNumber pn ) const
{
	return m_iPlayerStageTokens[pn];
}

int GameState::GetSmallestNumStagesLeftForAnyHumanPlayer() const
{
	if( IsEventMode() )
		return 999;
	int iSmallest = INT_MAX;
	FOREACH_HumanPlayer( p )
		iSmallest = min( iSmallest, m_iPlayerStageTokens[p] );
	return iSmallest;
}

bool GameState::IsFinalStageForAnyHumanPlayer() const
{
	return GetSmallestNumStagesLeftForAnyHumanPlayer() == 1;
}

bool GameState::IsFinalStageForEveryHumanPlayer() const
{
	int song_cost= 1;
	if(m_pCurSong != nullptr)
	{
		if(m_pCurSong->IsLong())
		{
			song_cost= 2;
		}
		else if(m_pCurSong->IsMarathon())
		{
			song_cost= 3;
		}
	}
	// If we're on gameplay or evaluation, they set this to false because those
	// screens have already had the stage tokens subtracted.
	song_cost*= m_AdjustTokensBySongCostForFinalStageCheck;
	int num_on_final= 0;
	int num_humans= 0;
	FOREACH_HumanPlayer(p)
	{
		if(m_iPlayerStageTokens[p] - song_cost <= 0)
		{
			++num_on_final;
		}
		++num_humans;
	}
	return num_on_final >= num_humans;
}

bool GameState::IsAnExtraStage() const
{
	if( this->GetMasterPlayerNumber() == PlayerNumber_Invalid )
		return false;
	return !IsEventMode() && !IsCourseMode() && m_iAwardedExtraStages[this->GetMasterPlayerNumber()] > 0;
}

static ThemeMetric<bool> LOCK_EXTRA_STAGE_SELECTION("GameState","LockExtraStageSelection");
bool GameState::IsAnExtraStageAndSelectionLocked() const
{
	return IsAnExtraStage() && LOCK_EXTRA_STAGE_SELECTION;
}

bool GameState::IsExtraStage() const
{
	if( this->GetMasterPlayerNumber() == PlayerNumber_Invalid )
		return false;
	return !IsEventMode() && !IsCourseMode() && m_iAwardedExtraStages[this->GetMasterPlayerNumber()] == 1;
}

bool GameState::IsExtraStage2() const
{
	if( this->GetMasterPlayerNumber() == PlayerNumber_Invalid )
		return false;
	return !IsEventMode() && !IsCourseMode() && m_iAwardedExtraStages[this->GetMasterPlayerNumber()] == 2;
}

Stage GameState::GetCurrentStage() const
{
	if( m_bDemonstrationOrJukebox )			return Stage_Demo;
	// "event" has precedence
	else if( IsEventMode() )			return Stage_Event;
	else if( m_PlayMode == PLAY_MODE_ONI )		return Stage_Oni;
	else if( m_PlayMode == PLAY_MODE_NONSTOP )	return Stage_Nonstop;
	else if( m_PlayMode == PLAY_MODE_ENDLESS )	return Stage_Endless;
	else if( IsExtraStage() )			return Stage_Extra1;
	else if( IsExtraStage2() )			return Stage_Extra2;
	// Previous logic did not factor in current song length, or the fact that
	// players aren't allowed to start a song with 0 tokens.  This new
	// function also has logic for handling the Gameplay and Evaluation cases
	// which used to require workarounds on the theme side. -Kyz
	else if(IsFinalStageForEveryHumanPlayer()) return Stage_Final;
	else
	{
		switch( this->m_iCurrentStageIndex )
		{
		case 0:	return Stage_1st;
		case 1:	return Stage_2nd;
		case 2:	return Stage_3rd;
		case 3:	return Stage_4th;
		case 4:	return Stage_5th;
		case 5:	return Stage_6th;
		default:	return Stage_Next;
		}
	}
}

int GameState::GetCourseSongIndex() const
{
	// iSongsPlayed includes the current song, so it's 1-based; subtract one.
	if( GAMESTATE->m_bMultiplayer )
	{
		FOREACH_EnabledMultiPlayer(mp)
			return STATSMAN->m_CurStageStats.m_multiPlayer[mp].m_iSongsPlayed-1;
		FAIL_M("At least one MultiPlayer must be joined.");
	}
	else
	{
		return STATSMAN->m_CurStageStats.m_player[this->GetMasterPlayerNumber()].m_iSongsPlayed-1;
	}
}

/* Hack: when we're loading a new course song, we want to display the new song
 * number, even though we haven't started that song yet. */
int GameState::GetLoadingCourseSongIndex() const
{
	int iIndex = GetCourseSongIndex();
	if( m_bLoadingNextSong )
		++iIndex;
	return iIndex;
}

static char const* prepare_song_failures[]= {
	"success",
	"no_current_song",
	"card_mount_failed",
	"load_interrupted",
};

int GameState::prepare_song_for_gameplay()
{
	Song* curr= m_pCurSong;
	if(curr == nullptr)
	{
		return 1;
	}
	if(curr->m_LoadedFromProfile == ProfileSlot_Invalid)
	{
		return 0;
	}
	ProfileSlot prof_slot= curr->m_LoadedFromProfile;
	PlayerNumber slot_as_pn= PlayerNumber(prof_slot);
	if(!PROFILEMAN->ProfileWasLoadedFromMemoryCard(slot_as_pn))
	{
		return 0;
	}
	if(!MEMCARDMAN->MountCard(slot_as_pn))
	{
		return 2;
	}
	RString prof_dir= PROFILEMAN->GetProfileDir(prof_slot);
	// Song loading changes its paths to point to the cache area. -Kyz
	RString to_dir= curr->GetSongDir();
	RString from_dir= curr->GetPreCustomifyDir();
	// The problem of what files to copy is complicated by steps being able to
	// specify their own music file, and the variety of step file formats.
	// Complex logic to figure out what files the song actually uses would be
	// bug prone.  Just copy all audio files and step files. -Kyz
	vector<RString> copy_exts= ActorUtil::GetTypeExtensionList(FT_Sound);
	copy_exts.push_back("sm");
	copy_exts.push_back("ssc");
	copy_exts.push_back("lrc");
	vector<RString> files_in_dir;
	FILEMAN->GetDirListingWithMultipleExtensions(from_dir, copy_exts, files_in_dir);
	for(size_t i= 0; i < files_in_dir.size(); ++i)
	{
		RString& fname= files_in_dir[i];
		if(!FileCopy(from_dir + fname, to_dir + fname))
		{
			return 3;
		}
	}
	MEMCARDMAN->UnmountCard(slot_as_pn);
	return 0;
}

static LocalizedString PLAYER1	("GameState","Player 1");
static LocalizedString PLAYER2	("GameState","Player 2");
static LocalizedString CPU		("GameState","CPU");
RString GameState::GetPlayerDisplayName( PlayerNumber pn ) const
{
	ASSERT( IsPlayerEnabled(pn) );
	const LocalizedString *pDefaultNames[] = { &PLAYER1, &PLAYER2 };
	if( IsHumanPlayer(pn) )
	{
		if( !PROFILEMAN->GetPlayerName(pn).empty() )
			return PROFILEMAN->GetPlayerName(pn);
		else
			return pDefaultNames[pn]->GetValue();
	}
	else
	{
		return CPU.GetValue();
	}
}

bool GameState::PlayersCanJoin() const
{
	if(GetNumSidesJoined() == 0)
	{
		return true;
	}
	// If we check the style and it comes up nullptr, either the style has not been
	// chosen, or we're on ScreenSelectMusic with AutoSetStyle.
	// If the style does not come up nullptr, we might be on a screen in a custom
	// theme that wants to allow joining after the style is set anyway.
	// Either way, we can't use the existence of a style to decide.
	// -Kyz
	if( ALLOW_LATE_JOIN.IsLoaded()  &&  ALLOW_LATE_JOIN )
	{
		Screen *pScreen = SCREENMAN->GetTopScreen();
		if(pScreen)
		{
			if(!pScreen->AllowLateJoin())
			{
				return false;
			}
		}
		// We can't use FOREACH_EnabledPlayer because that uses PlayersCanJoin
		// in part of its logic chain. -Kyz
		FOREACH_PlayerNumber(pn)
		{
			const Style* style= GetCurrentStyle(pn);
			if(style)
			{
				const Style* compat_style= GAMEMAN->GetFirstCompatibleStyle(
					m_pCurGame, 2, style->m_StepsType);
				if(compat_style == nullptr)
				{
					return false;
				}
			}
		}
		return true;
	}
	return false;
}

int GameState::GetNumSidesJoined() const
{
	int iNumSidesJoined = 0;
	FOREACH_PlayerNumber( p )
		if( m_bSideIsJoined[p] )
			iNumSidesJoined++;	// left side, and right side
	return iNumSidesJoined;
}

const Game* GameState::GetCurrentGame() const
{
	ASSERT( m_pCurGame != nullptr );	// the game must be set before calling this
	return m_pCurGame;
}

const Style* GameState::GetCurrentStyle(PlayerNumber pn) const
{
	if(GetCurrentGame() == nullptr) { return nullptr; }
	if(!GetCurrentGame()->m_PlayersHaveSeparateStyles)
	{
		return m_pCurStyle;
	}
	else
	{
		if(pn >= NUM_PLAYERS)
		{
			return m_SeparatedStyles[PLAYER_1] == nullptr ? m_SeparatedStyles[PLAYER_2]
				: m_SeparatedStyles[PLAYER_1];
		}
		return m_SeparatedStyles[pn];
	}
}

void GameState::SetCurrentStyle(const Style *style, PlayerNumber pn)
{
	if(!GetCurrentGame()->m_PlayersHaveSeparateStyles)
	{
		m_pCurStyle.Set(style);
	}
	else
	{
		if(pn == PLAYER_INVALID)
		{
			FOREACH_PlayerNumber(rpn)
			{
				m_SeparatedStyles[rpn]= style;
			}
		}
		else
		{
			m_SeparatedStyles[pn]= style;
		}
	}
	if(INPUTMAPPER)
	{
		if(GetCurrentStyle(pn) && GetCurrentStyle(pn)->m_StyleType == StyleType_OnePlayerTwoSides)
		{
			// If the other player is joined, unjoin them because this style only
			// allows one player.
			PlayerNumber other_pn= OPPOSITE_PLAYER[this->GetMasterPlayerNumber()];
			if(GetNumSidesJoined() > 1)
			{
				UnjoinPlayer(other_pn);
			}
			INPUTMAPPER->SetJoinControllers(this->GetMasterPlayerNumber());
		}
		else
			INPUTMAPPER->SetJoinControllers(PLAYER_INVALID);
	}
}

bool GameState::SetCompatibleStyle(StepsType stype, PlayerNumber pn)
{
	bool style_incompatible= false;
	if(!GetCurrentStyle(pn))
	{
		style_incompatible= true;
	}
	else
	{
		style_incompatible= stype != GetCurrentStyle(pn)->m_StepsType;
	}
	if(CommonMetrics::AUTO_SET_STYLE && style_incompatible)
	{
		const Style* compatible_style= GAMEMAN->GetFirstCompatibleStyle(
			m_pCurGame, GetNumSidesJoined(), stype);
		if(!compatible_style)
		{
			return false;
		}
		SetCurrentStyle(compatible_style, pn);
	}
	return stype == GetCurrentStyle(pn)->m_StepsType;
}

bool GameState::IsPlayerEnabled( PlayerNumber pn ) const
{
	// In rave, all players are present.  Non-human players are CPU controlled.
	switch( m_PlayMode )
	{
		case PLAY_MODE_BATTLE:
		case PLAY_MODE_RAVE:
			return true;
		default:
			return IsHumanPlayer(pn);
	}
}

bool GameState::IsMultiPlayerEnabled( MultiPlayer mp ) const
{
	return m_MultiPlayerStatus[ mp ] == MultiPlayerStatus_Joined;
}

bool GameState::IsPlayerEnabled( const PlayerState* pPlayerState ) const
{
	if( pPlayerState->m_mp != MultiPlayer_Invalid )
		return IsMultiPlayerEnabled( pPlayerState->m_mp );
	if( pPlayerState->m_PlayerNumber != PLAYER_INVALID )
		return IsPlayerEnabled( pPlayerState->m_PlayerNumber );
	return false;
}

int	GameState::GetNumPlayersEnabled() const
{
	int count = 0;
	FOREACH_EnabledPlayer( pn )
		count++;
	return count;
}

bool GameState::IsHumanPlayer( PlayerNumber pn ) const
{
	if( pn == PLAYER_INVALID )
		return false;

	if(GetCurrentGame()->m_PlayersHaveSeparateStyles)
	{
		if( GetCurrentStyle(pn) == nullptr )	// no style chosen
		{
			return m_bSideIsJoined[pn];
		}
		else
		{
			StyleType type = GetCurrentStyle(pn)->m_StyleType;
			switch( type )
			{
				case StyleType_TwoPlayersTwoSides:
				case StyleType_TwoPlayersSharedSides:
					return true;
				case StyleType_OnePlayerOneSide:
				case StyleType_OnePlayerTwoSides:
					return pn == this->GetMasterPlayerNumber();
				default:
					FAIL_M(ssprintf("Invalid style type: %i", type));
			}
		}
	}
	if( GetCurrentStyle(pn) == nullptr )	// no style chosen
	{
		return m_bSideIsJoined[pn];	// only allow input from sides that have already joined
	}

	StyleType type = GetCurrentStyle(pn)->m_StyleType;
	switch( type )
	{
	case StyleType_TwoPlayersTwoSides:
	case StyleType_TwoPlayersSharedSides:
		return true;
	case StyleType_OnePlayerOneSide:
	case StyleType_OnePlayerTwoSides:
		return pn == this->GetMasterPlayerNumber();
	default:
		FAIL_M(ssprintf("Invalid style type: %i", type));
	}
}

int GameState::GetNumHumanPlayers() const
{
	int count = 0;
	FOREACH_HumanPlayer( pn )
		count++;
	return count;
}

PlayerNumber GameState::GetFirstHumanPlayer() const
{
	FOREACH_HumanPlayer( pn )
		return pn;
	return PLAYER_INVALID;
}

PlayerNumber GameState::GetFirstDisabledPlayer() const
{
	FOREACH_PlayerNumber( pn )
		if( !IsPlayerEnabled(pn) )
			return pn;
	return PLAYER_INVALID;
}

bool GameState::IsCpuPlayer( PlayerNumber pn ) const
{
	return IsPlayerEnabled(pn) && !IsHumanPlayer(pn);
}

bool GameState::AnyPlayersAreCpu() const
{
	FOREACH_CpuPlayer( pn )
		return true;
	return false;
}


bool GameState::IsCourseMode() const
{
	switch(m_PlayMode)
	{
	case PLAY_MODE_ONI:
	case PLAY_MODE_NONSTOP:
	case PLAY_MODE_ENDLESS:
		return true;
	default:
		return false;
	}
}

bool GameState::IsBattleMode() const
{
	switch( m_PlayMode )
	{
	case PLAY_MODE_BATTLE:
		return true;
	default:
		return false;
	}
}

EarnedExtraStage GameState::CalculateEarnedExtraStage() const
{
	if( IsEventMode() )
		return EarnedExtraStage_No;

	if( !PREFSMAN->m_bAllowExtraStage )
		return EarnedExtraStage_No;

	if( m_PlayMode != PLAY_MODE_REGULAR )
		return EarnedExtraStage_No;

	if( m_bBackedOutOfFinalStage )
		return EarnedExtraStage_No;

	if( GetSmallestNumStagesLeftForAnyHumanPlayer() > 0 )
		return EarnedExtraStage_No;

	if( m_iAwardedExtraStages[this->GetMasterPlayerNumber()] >= 2 )
		return EarnedExtraStage_No;

	FOREACH_EnabledPlayer( pn )
	{
		Difficulty dc = m_pCurSteps[pn]->GetDifficulty();
		switch( dc )
		{
		case Difficulty_Edit:
			if( !EDIT_ALLOWED_FOR_EXTRA )
				continue; // can't use edit steps
			break;
		default:
			if( dc < MIN_DIFFICULTY_FOR_EXTRA )
				continue; // not hard enough!
			break;
		}

		if( IsExtraStage() )
		{
			if( ALLOW_EXTRA_2  &&  STATSMAN->m_CurStageStats.m_player[pn].GetGrade() <= GRADE_TIER_FOR_EXTRA_2 )
				return EarnedExtraStage_Extra2;
		}
		else if( STATSMAN->m_CurStageStats.m_player[pn].GetGrade() <= GRADE_TIER_FOR_EXTRA_1 )
		{
			return EarnedExtraStage_Extra1;
		}
	}

	return EarnedExtraStage_No;
}

PlayerNumber GameState::GetBestPlayer() const
{
	FOREACH_PlayerNumber( pn )
		if( GetStageResult(pn) == RESULT_WIN )
			return pn;
	return PLAYER_INVALID;	// draw
}

StageResult GameState::GetStageResult( PlayerNumber pn ) const
{
	switch( m_PlayMode )
	{
		case PLAY_MODE_BATTLE:
		case PLAY_MODE_RAVE:
			if( fabsf(m_fTugLifePercentP1 - 0.5f) < 0.0001f )
				return RESULT_DRAW;
			switch( pn )
			{
			case PLAYER_1:	return (m_fTugLifePercentP1>=0.5f)?RESULT_WIN:RESULT_LOSE;
			case PLAYER_2:	return (m_fTugLifePercentP1<0.5f)?RESULT_WIN:RESULT_LOSE;
			default:	FAIL_M("Invalid player for battle! Aborting..."); return RESULT_LOSE;
			}
		default: break;
	}

	StageResult win = RESULT_WIN;
	FOREACH_PlayerNumber( p )
	{
		if( p == pn )
			continue;

		// If anyone did just as well, at best it's a draw.
		if( STATSMAN->m_CurStageStats.m_player[p].m_iActualDancePoints == STATSMAN->m_CurStageStats.m_player[pn].m_iActualDancePoints )
			win = RESULT_DRAW;

		// If anyone did better, we lost.
		if( STATSMAN->m_CurStageStats.m_player[p].m_iActualDancePoints > STATSMAN->m_CurStageStats.m_player[pn].m_iActualDancePoints )
			return RESULT_LOSE;
	}
	return win;
}

void GameState::GetDefaultPlayerOptions( PlayerOptions &po )
{
	po.Init();
	po.FromString( PREFSMAN->m_sDefaultModifiers );
	po.FromString( CommonMetrics::DEFAULT_MODIFIERS );
	if( po.m_sNoteSkin.empty() )
		po.m_sNoteSkin = CommonMetrics::DEFAULT_NOTESKIN_NAME;
}

void GameState::GetDefaultSongOptions( SongOptions &so )
{
	so.Init();
	so.FromString( PREFSMAN->m_sDefaultModifiers );
	so.FromString( CommonMetrics::DEFAULT_MODIFIERS );
}

void GameState::ResetToDefaultSongOptions( ModsLevel l )
{
	SongOptions so;
	GetDefaultSongOptions( so );
	m_SongOptions.Assign( l, so );
}

void GameState::ApplyPreferredModifiers( PlayerNumber pn, RString sModifiers )
{
	m_pPlayerState[pn]->m_PlayerOptions.FromString( ModsLevel_Preferred, sModifiers );
	m_SongOptions.FromString( ModsLevel_Preferred, sModifiers );
}

void GameState::ApplyStageModifiers( PlayerNumber pn, RString sModifiers )
{
	m_pPlayerState[pn]->m_PlayerOptions.FromString( ModsLevel_Stage, sModifiers );
	m_SongOptions.FromString( ModsLevel_Stage, sModifiers );
}

void GameState::ClearStageModifiersIllegalForCourse()
{
	FOREACH_EnabledPlayer( pn )
		PO_GROUP_CALL( m_pPlayerState[pn]->m_PlayerOptions, ModsLevel_Stage, ResetSavedPrefsInvalidForCourse );
}

bool GameState::CurrentOptionsDisqualifyPlayer( PlayerNumber pn )
{
	if( !PREFSMAN->m_bDisqualification )
		return false;

	if( !IsHumanPlayer(pn) )
		return false;

	const PlayerOptions &po = m_pPlayerState[pn]->m_PlayerOptions.GetPreferred();
	const SongOptions &so = m_SongOptions.GetPreferred();

	// Playing a song/course at a slower music rate should disqualify score. -x0rbl
	if (so.m_fMusicRate < 1.0) {
		return true;
	}

	// Check the stored player options for disqualify.  Don't disqualify because
	// of mods that were forced.
	if( IsCourseMode() )
		return po.IsEasierForCourseAndTrail(  m_pCurCourse, m_pCurTrail[pn] );
	else
		return po.IsEasierForSongAndSteps(  m_pCurSong, m_pCurSteps[pn], pn);
}

/* reset noteskins (?)
 * GameState::ResetNoteSkins()
 * GameState::ResetNoteSkinsForPlayer( PlayerNumber pn )
 *
 */

void GameState::GetAllUsedNoteSkins( vector<RString> &out ) const
{
	FOREACH_EnabledPlayer( pn )
	{
		out.push_back( m_pPlayerState[pn]->m_PlayerOptions.GetCurrent().m_sNoteSkin );

		// Add noteskins that are used in courses.
		if( IsCourseMode() )
		{
			const Trail *pTrail = m_pCurTrail[pn];
			ASSERT( pTrail != nullptr );

			for (TrailEntry const &e : pTrail->m_vEntries)
			{
				PlayerOptions po;
				po.FromString( e.Modifiers );
				if( !po.m_sNoteSkin.empty() )
					out.push_back( po.m_sNoteSkin );
			}
		}
	}

	// Remove duplicates.
	sort( out.begin(), out.end() );
	out.erase( unique( out.begin(), out.end() ), out.end() );
}

void GameState::RemoveAllActiveAttacks()	// called on end of song
{
	FOREACH_PlayerNumber( p )
		m_pPlayerState[p]->RemoveActiveAttacks();
}

void GameState::AddStageToPlayer( PlayerNumber pn )
{
	// Add one stage more to player (bonus) -cerbo
	++m_iPlayerStageTokens[pn];
}

template<class T>
void setmin( T &a, const T &b )
{
	a = min(a, b);
}

template<class T>
void setmax( T &a, const T &b )
{
	a = max(a, b);
}

FailType GameState::GetPlayerFailType( const PlayerState *pPlayerState ) const
{
	PlayerNumber pn = pPlayerState->m_PlayerNumber;
	FailType ft = pPlayerState->m_PlayerOptions.GetCurrent().m_FailType;

	// If the player changed the fail mode explicitly, leave it alone.
	if( m_bFailTypeWasExplicitlySet )
		return ft;

	if( IsCourseMode() )
	{
		if( PREFSMAN->m_bMinimum1FullSongInCourses && GetCourseSongIndex()==0 )
			ft = max( ft, FailType_ImmediateContinue );	// take the least harsh of the two FailTypes
	}
	else
	{
		Difficulty dc = Difficulty_Invalid;
		if( m_pCurSteps[pn] )
			dc = m_pCurSteps[pn]->GetDifficulty();

		bool bFirstStage = false;
		if( !IsEventMode() )
			bFirstStage |= m_iPlayerStageTokens[pPlayerState->m_PlayerNumber] == PREFSMAN->m_iSongsPerPlay-1; // HACK; -1 because this is called during gameplay

		// Easy and beginner are never harder than FAIL_IMMEDIATE_CONTINUE.
		if( dc <= Difficulty_Easy )
			setmax( ft, FailType_ImmediateContinue );

		if( dc <= Difficulty_Easy && bFirstStage && PREFSMAN->m_bFailOffForFirstStageEasy )
			setmax( ft, FailType_Off );

		/* If beginner's steps were chosen, and this is the first stage,
		 * turn off failure completely. */
		if( dc == Difficulty_Beginner && bFirstStage )
			setmax( ft, FailType_Off );

		if( dc == Difficulty_Beginner && PREFSMAN->m_bFailOffInBeginner )
			setmax( ft, FailType_Off );
	}

	return ft;
}

bool GameState::ShowW1() const
{
	AllowW1 pref = PREFSMAN->m_AllowW1;
	switch( pref )
	{
	case ALLOW_W1_NEVER:		return false;
	case ALLOW_W1_COURSES_ONLY:	return IsCourseMode();
	case ALLOW_W1_EVERYWHERE:	return true;
	default:
		FAIL_M(ssprintf("Invalid AllowW1 preference: %i", pref));
	}
}


static ThemeMetric<bool> PROFILE_RECORD_FEATS("GameState","ProfileRecordFeats");
static ThemeMetric<bool> CATEGORY_RECORD_FEATS("GameState","CategoryRecordFeats");
void GameState::GetRankingFeats( PlayerNumber pn, vector<RankingFeat> &asFeatsOut ) const
{
	if( !IsHumanPlayer(pn) )
		return;

	Profile *pProf = PROFILEMAN->GetProfile(pn);

	// Check for feats even if the PlayMode is rave or battle because the player
	// may have made high scores then switched modes.
	PlayMode mode = m_PlayMode.Get();
	char const *modeStr = PlayModeToString(mode).c_str();
	
	CHECKPOINT_M( ssprintf("Getting the feats for %s", modeStr));
	switch( mode )
	{
	case PLAY_MODE_REGULAR:
	case PLAY_MODE_BATTLE:
	case PLAY_MODE_RAVE:
		{
			StepsType st = GetCurrentStyle(pn)->m_StepsType;

			// Find unique Song and Steps combinations that were played.
			// We must keep only the unique combination or else we'll double-count
			// high score markers.
			vector<SongAndSteps> vSongAndSteps;

			for( unsigned i=0; i<STATSMAN->m_vPlayedStageStats.size(); i++ )
			{
				CHECKPOINT_M( ssprintf("%u/%i", i, (int)STATSMAN->m_vPlayedStageStats.size() ) );
				SongAndSteps sas;
				ASSERT( !STATSMAN->m_vPlayedStageStats[i].m_vpPlayedSongs.empty() );
				sas.pSong = STATSMAN->m_vPlayedStageStats[i].m_vpPlayedSongs[0];
				ASSERT( sas.pSong != nullptr );
				if( STATSMAN->m_vPlayedStageStats[i].m_player[pn].m_vpPossibleSteps.empty() )
					continue;
				sas.pSteps = STATSMAN->m_vPlayedStageStats[i].m_player[pn].m_vpPossibleSteps[0];
				ASSERT( sas.pSteps != nullptr );
				vSongAndSteps.push_back( sas );
			}
			CHECKPOINT_M( ssprintf("All songs/steps from %s gathered", modeStr));

			sort( vSongAndSteps.begin(), vSongAndSteps.end() );

			vector<SongAndSteps>::iterator toDelete = unique( vSongAndSteps.begin(), vSongAndSteps.end() );
			vSongAndSteps.erase(toDelete, vSongAndSteps.end());

			CHECKPOINT_M( "About to find records from the gathered.");
			for( unsigned i=0; i<vSongAndSteps.size(); i++ )
			{
				Song* pSong = vSongAndSteps[i].pSong;
				Steps* pSteps = vSongAndSteps[i].pSteps;

				// Find Machine Records
				{
					HighScoreList &hsl = PROFILEMAN->GetMachineProfile()->GetStepsHighScoreList(pSong,pSteps);
					for( unsigned j=0; j<hsl.vHighScores.size(); j++ )
					{
						HighScore &hs = hsl.vHighScores[j];

						if( hs.GetName() != RANKING_TO_FILL_IN_MARKER[pn] )
							continue;

						RankingFeat feat;
						feat.Type = RankingFeat::SONG;
						feat.pSong = pSong;
						feat.pSteps = pSteps;
						feat.Feat = ssprintf("MR #%d in %s %s", j+1, pSong->GetTranslitMainTitle().c_str(), DifficultyToString(pSteps->GetDifficulty()).c_str() );
						feat.pStringToFill = hs.GetNameMutable();
						feat.grade = hs.GetGrade();
						feat.fPercentDP = hs.GetPercentDP();
						feat.iScore = hs.GetScore();

						if( pSong->HasBanner() )
							feat.Banner = pSong->GetBannerPath();

						asFeatsOut.push_back( feat );
					}
				}

				// Find Personal Records
				if( pProf && PROFILE_RECORD_FEATS )
				{
					HighScoreList &hsl = pProf->GetStepsHighScoreList(pSong,pSteps);
					for( unsigned j=0; j<hsl.vHighScores.size(); j++ )
					{
						HighScore &hs = hsl.vHighScores[j];

						if( hs.GetName() != RANKING_TO_FILL_IN_MARKER[pn] )
							continue;

						RankingFeat feat;
						feat.pSong = pSong;
						feat.pSteps = pSteps;
						feat.Type = RankingFeat::SONG;
						feat.Feat = ssprintf("PR #%d in %s %s", j+1, pSong->GetTranslitMainTitle().c_str(), DifficultyToString(pSteps->GetDifficulty()).c_str() );
						feat.pStringToFill = hs.GetNameMutable();
						feat.grade = hs.GetGrade();
						feat.fPercentDP = hs.GetPercentDP();
						feat.iScore = hs.GetScore();

						// XXX: temporary hack
						// Why is this here? -aj
						if( pSong->HasBackground() )
							feat.Banner = pSong->GetBackgroundPath();
		//					if( pSong->HasBanner() )
		//						feat.Banner = pSong->GetBannerPath();

						asFeatsOut.push_back( feat );
					}
				}
			}

			CHECKPOINT_M("Getting the final evaluation stage stats.");
			StageStats stats;
			STATSMAN->GetFinalEvalStageStats( stats );


			// Find Machine Category Records
			FOREACH_ENUM( RankingCategory, rc )
			{
				if( !CATEGORY_RECORD_FEATS )
					continue;
				HighScoreList &hsl = PROFILEMAN->GetMachineProfile()->GetCategoryHighScoreList( st, rc );
				for( unsigned j=0; j<hsl.vHighScores.size(); j++ )
				{
					HighScore &hs = hsl.vHighScores[j];
					if( hs.GetName() != RANKING_TO_FILL_IN_MARKER[pn] )
						continue;

					RankingFeat feat;
					feat.Type = RankingFeat::CATEGORY;
					feat.Feat = ssprintf("MR #%d in Type %c (%d)", j+1, 'A'+rc, stats.GetAverageMeter(pn) );
					feat.pStringToFill = hs.GetNameMutable();
					feat.grade = Grade_NoData;
					feat.iScore = hs.GetScore();
					feat.fPercentDP = hs.GetPercentDP();
					asFeatsOut.push_back( feat );
				}
			}

			// Find Personal Category Records
			FOREACH_ENUM( RankingCategory, rc )
			{
				if( !CATEGORY_RECORD_FEATS )
					continue;

				if( pProf && PROFILE_RECORD_FEATS )
				{
					HighScoreList &hsl = pProf->GetCategoryHighScoreList( st, rc );
					for( unsigned j=0; j<hsl.vHighScores.size(); j++ )
					{
						HighScore &hs = hsl.vHighScores[j];
						if( hs.GetName() != RANKING_TO_FILL_IN_MARKER[pn] )
							continue;

						RankingFeat feat;
						feat.Type = RankingFeat::CATEGORY;
						feat.Feat = ssprintf("PR #%d in Type %c (%d)", j+1, 'A'+rc, stats.GetAverageMeter(pn) );
						feat.pStringToFill = hs.GetNameMutable();
						feat.grade = Grade_NoData;
						feat.iScore = hs.GetScore();
						feat.fPercentDP = hs.GetPercentDP();
						asFeatsOut.push_back( feat );
					}
				}
			}
		}
		break;
	case PLAY_MODE_NONSTOP:
	case PLAY_MODE_ONI:
	case PLAY_MODE_ENDLESS:
		{
			Course* pCourse = m_pCurCourse;
			ASSERT( pCourse != nullptr );
			Trail *pTrail = m_pCurTrail[pn];
			ASSERT( pTrail != nullptr );
			CourseDifficulty cd = pTrail->m_CourseDifficulty;

			// Find Machine Records
			{
				Profile* pProfile = PROFILEMAN->GetMachineProfile();
				HighScoreList &hsl = pProfile->GetCourseHighScoreList( pCourse, pTrail );
				for( unsigned i=0; i<hsl.vHighScores.size(); i++ )
				{
					HighScore &hs = hsl.vHighScores[i];
					if( hs.GetName() != RANKING_TO_FILL_IN_MARKER[pn] )
							continue;

					RankingFeat feat;
					feat.Type = RankingFeat::COURSE;
					feat.pCourse = pCourse;
					feat.Feat = ssprintf("MR #%d in %s", i+1, pCourse->GetDisplayFullTitle().c_str() );
					if( cd != Difficulty_Medium )
						feat.Feat += " " + CourseDifficultyToLocalizedString(cd);
					feat.pStringToFill = hs.GetNameMutable();
					feat.grade = Grade_NoData;
					feat.iScore = hs.GetScore();
					feat.fPercentDP = hs.GetPercentDP();
					if( pCourse->HasBanner() )
						feat.Banner = pCourse->GetBannerPath();
					asFeatsOut.push_back( feat );
				}
			}

			// Find Personal Records
			if( PROFILE_RECORD_FEATS && PROFILEMAN->IsPersistentProfile(pn) )
			{
				HighScoreList &hsl = pProf->GetCourseHighScoreList( pCourse, pTrail );
				for( unsigned i=0; i<hsl.vHighScores.size(); i++ )
				{
					HighScore& hs = hsl.vHighScores[i];
					if( hs.GetName() != RANKING_TO_FILL_IN_MARKER[pn] )
							continue;

					RankingFeat feat;
					feat.Type = RankingFeat::COURSE;
					feat.pCourse = pCourse;
					feat.Feat = ssprintf("PR #%d in %s", i+1, pCourse->GetDisplayFullTitle().c_str() );
					feat.pStringToFill = hs.GetNameMutable();
					feat.grade = Grade_NoData;
					feat.iScore = hs.GetScore();
					feat.fPercentDP = hs.GetPercentDP();
					if( pCourse->HasBanner() )
						feat.Banner = pCourse->GetBannerPath();
					asFeatsOut.push_back( feat );
				}
			}
		}
		break;
	default:
		FAIL_M(ssprintf("Invalid play mode: %i", int(m_PlayMode)));
	}
}

bool GameState::AnyPlayerHasRankingFeats() const
{
	vector<RankingFeat> vFeats;
	FOREACH_PlayerNumber( p )
	{
		GetRankingFeats( p, vFeats );
		if( !vFeats.empty() )
			return true;
	}
	return false;
}

void GameState::StoreRankingName( PlayerNumber pn, RString sName )
{
	// The theme can upper it if desired. -Kyz
	// sName.MakeUpper();

	if( USE_NAME_BLACKLIST )
	{
		RageFile file;
		if( file.Open(NAME_BLACKLIST_FILE) )
		{
			RString sLine;

			while( !file.AtEOF() )
			{
				if( file.GetLine(sLine) == -1 )
				{
					LuaHelpers::ReportScriptErrorFmt( "Error reading \"%s\": %s", NAME_BLACKLIST_FILE, file.GetError().c_str() );
					break;
				}

				sLine.MakeUpper();
				if( !sLine.empty() && sName.find(sLine) != string::npos )	// name contains a bad word
				{
					LOG->Trace( "entered '%s' matches blacklisted item '%s'", sName.c_str(), sLine.c_str() );
					sName = "";
					break;
				}
			}
		}
	}

	vector<RankingFeat> aFeats;
	GetRankingFeats( pn, aFeats );

	for( unsigned i=0; i<aFeats.size(); i++ )
	{
		*aFeats[i].pStringToFill = sName;

		// save name pointers as we fill them
		m_vpsNamesThatWereFilled.push_back( aFeats[i].pStringToFill );
	}


	Profile *pProfile = PROFILEMAN->GetMachineProfile();

	if( !PREFSMAN->m_bAllowMultipleHighScoreWithSameName )
	{
		// erase all but the highest score for each name
		for (auto &songIter : pProfile->m_SongHighScores)
			for (auto &stepIter : songIter.second.m_StepsHighScores)
				stepIter.second.hsl.RemoveAllButOneOfEachName();

		for (auto &courseIter : pProfile->m_CourseHighScores)
			for (auto &trailIter : courseIter.second.m_TrailHighScores)
				trailIter.second.hsl.RemoveAllButOneOfEachName();
	}

	// clamp high score sizes
	for (auto &songIter : pProfile->m_SongHighScores)
		for (auto &stepIter : songIter.second.m_StepsHighScores)
			stepIter.second.hsl.ClampSize( true );

	for (auto &courseIter : pProfile->m_CourseHighScores)
		for (auto &trailIter : courseIter.second.m_TrailHighScores)
			trailIter.second.hsl.ClampSize( true );
}

bool GameState::AllAreInDangerOrWorse() const
{
	FOREACH_EnabledPlayer( p )
		if( m_pPlayerState[p]->m_HealthState < HealthState_Danger )
			return false;
	return true;
}

bool GameState::OneIsHot() const
{
	FOREACH_EnabledPlayer( p )
		if( m_pPlayerState[p]->m_HealthState == HealthState_Hot )
			return true;
	return false;
}

bool GameState::IsTimeToPlayAttractSounds() const
{
	// m_iNumTimesThroughAttract will be -1 from the first attract screen after
	// the end of a game until the next time FIRST_ATTRACT_SCREEN is reached.
	// Play attract sounds for this sort span of time regardless of
	// m_AttractSoundFrequency because it's awkward to have the machine go
	// silent immediately after the end of a game.
	if( m_iNumTimesThroughAttract == -1 )
		return true;

	if( PREFSMAN->m_AttractSoundFrequency == ASF_NEVER )
		return false;

	// play attract sounds once every m_iAttractSoundFrequency times through
	if( (m_iNumTimesThroughAttract % PREFSMAN->m_AttractSoundFrequency)==0 )
		return true;

	return false;
}

void GameState::VisitAttractScreen( const RString sScreenName )
{
	if( sScreenName == CommonMetrics::FIRST_ATTRACT_SCREEN.GetValue() )
		m_iNumTimesThroughAttract++;
}

bool GameState::DifficultiesLocked() const
{
	if( m_PlayMode == PLAY_MODE_RAVE )
		return true;
	if( IsCourseMode() )
		return PREFSMAN->m_bLockCourseDifficulties;
	if( GetCurrentStyle(PLAYER_INVALID)->m_bLockDifficulties )
		return true;
	return false;
}

bool GameState::ChangePreferredDifficultyAndStepsType( PlayerNumber pn, Difficulty dc, StepsType st )
{
	m_PreferredDifficulty[pn].Set( dc );
	m_PreferredStepsType.Set( st );
	if( DifficultiesLocked() )
		FOREACH_PlayerNumber( p )
			if( p != pn )
				m_PreferredDifficulty[p].Set( m_PreferredDifficulty[pn] );

	return true;
}

/* When only displaying difficulties in DIFFICULTIES_TO_SHOW, use GetClosestShownDifficulty
 * to find which difficulty to show, and ChangePreferredDifficulty(pn, dir) to change
 * difficulty. */
bool GameState::ChangePreferredDifficulty( PlayerNumber pn, int dir )
{
	const vector<Difficulty> &v = CommonMetrics::DIFFICULTIES_TO_SHOW.GetValue();

	Difficulty d = GetClosestShownDifficulty(pn);
	for(;;)
	{
		d = enum_add2( d, dir );
		if( d < 0 || d >= NUM_Difficulty )
		{
			return false;
		}
		if( find(v.begin(), v.end(), d) != v.end() )
		{
			break; // found
		}
	}
	m_PreferredDifficulty[pn].Set( d );
	return true;
}

/* The user may be set to prefer a difficulty that isn't always shown; typically,
 * Difficulty_Edit. Return the closest shown difficulty <= m_PreferredDifficulty. */
Difficulty GameState::GetClosestShownDifficulty( PlayerNumber pn ) const
{
	const vector<Difficulty> &v = CommonMetrics::DIFFICULTIES_TO_SHOW.GetValue();

	Difficulty iClosest = (Difficulty) 0;
	int iClosestDist = -1;
	for (Difficulty const &dc : v)
	{
		int iDist = m_PreferredDifficulty[pn] - dc;
		if( iDist < 0 )
			continue;
		if( iClosestDist != -1 && iDist > iClosestDist )
			continue;
		iClosestDist = iDist;
		iClosest = dc;
	}

	return iClosest;
}

bool GameState::ChangePreferredCourseDifficultyAndStepsType( PlayerNumber pn, CourseDifficulty cd, StepsType st )
{
	m_PreferredCourseDifficulty[pn].Set( cd );
	m_PreferredStepsType.Set( st );
	if( PREFSMAN->m_bLockCourseDifficulties )
		FOREACH_PlayerNumber( p )
			if( p != pn )
				m_PreferredCourseDifficulty[p].Set( m_PreferredCourseDifficulty[pn] );

	return true;
}

bool GameState::ChangePreferredCourseDifficulty( PlayerNumber pn, int dir )
{
	/* If we have a course selected, only choose among difficulties available in the course. */
	const Course *pCourse = m_pCurCourse;

	const vector<CourseDifficulty> &v = CommonMetrics::COURSE_DIFFICULTIES_TO_SHOW.GetValue();

	CourseDifficulty cd = m_PreferredCourseDifficulty[pn];
	for(;;)
	{
		cd = enum_add2( cd, dir );
		if( cd < 0 || cd >= NUM_Difficulty )
		{
			return false;
		}
		if( find(v.begin(),v.end(),cd) == v.end() )
		{
			continue; /* not available */
		}
		if( !pCourse || pCourse->GetTrail( GetCurrentStyle(pn)->m_StepsType, cd ) )
		{
			break;
		}
	}
	return ChangePreferredCourseDifficulty( pn, cd );
}

bool GameState::IsCourseDifficultyShown( CourseDifficulty cd )
{
	const vector<CourseDifficulty> &v = CommonMetrics::COURSE_DIFFICULTIES_TO_SHOW.GetValue();
	return find(v.begin(), v.end(), cd) != v.end();
}

Difficulty GameState::GetEasiestStepsDifficulty() const
{
	Difficulty dc = Difficulty_Invalid;
	FOREACH_HumanPlayer( p )
	{
		if( m_pCurSteps[p] == nullptr )
		{
			LuaHelpers::ReportScriptErrorFmt( "GetEasiestStepsDifficulty called but p%i hasn't chosen notes", p+1 );
			continue;
		}
		dc = min( dc, m_pCurSteps[p]->GetDifficulty() );
	}
	return dc;
}

Difficulty GameState::GetHardestStepsDifficulty() const
{
	Difficulty dc = Difficulty_Beginner;
	FOREACH_HumanPlayer( p )
	{
		if( m_pCurSteps[p] == nullptr )
		{
			LuaHelpers::ReportScriptErrorFmt( "GetHardestStepsDifficulty called but p%i hasn't chosen notes", p+1 );
			continue;
		}
		dc = max( dc, m_pCurSteps[p]->GetDifficulty() );
	}
	return dc;
}

void GameState::SetNewStageSeed()
{
	m_iStageSeed= rand();
}

bool GameState::IsEventMode() const
{
	return m_bTemporaryEventMode || PREFSMAN->m_bEventMode;
}

CoinMode GameState::GetCoinMode() const
{
	// XXX: Event mode shouldn't be valid if CoinMode_Pay
	if( IsEventMode() && GamePreferences::m_CoinMode == CoinMode_Pay )
		return CoinMode_Free;
	else
		return GamePreferences::m_CoinMode;
}

ThemeMetric<bool> DISABLE_PREMIUM_IN_EVENT_MODE("GameState","DisablePremiumInEventMode");
Premium	GameState::GetPremium() const
{
	return DISABLE_PREMIUM_IN_EVENT_MODE ? Premium_Off : g_Premium;
}

float GameState::GetGoalPercentComplete( PlayerNumber pn )
{
	const Profile *pProfile = PROFILEMAN->GetProfile(pn);
	const StageStats &ssCurrent = STATSMAN->m_CurStageStats;
	const PlayerStageStats &pssCurrent = ssCurrent.m_player[pn];

	float fActual = 0;
	float fGoal = 0;
	switch( pProfile->m_GoalType )
	{
	case GoalType_Calories:
		fActual = pssCurrent.m_fCaloriesBurned;
		fGoal = (float)pProfile->m_iGoalCalories;
		break;
	case GoalType_Time:
		fActual = ssCurrent.m_fGameplaySeconds;
		fGoal = (float)pProfile->m_iGoalSeconds;
		break;
	case GoalType_None:
		return 0;	// never complete
	default:
		FAIL_M(ssprintf("Invalid GoalType: %i", pProfile->m_GoalType));
	}
	if( fGoal == 0 )
		return 0;
	else
		return fActual / fGoal;
}

bool GameState::PlayerIsUsingModifier( PlayerNumber pn, const RString &sModifier )
{
	PlayerOptions po = m_pPlayerState[pn]->m_PlayerOptions.GetCurrent();
	SongOptions so = m_SongOptions.GetCurrent();
	po.FromString( sModifier );
	so.FromString( sModifier );

	return po == m_pPlayerState[pn]->m_PlayerOptions.GetCurrent()  &&  so == m_SongOptions.GetCurrent();
}

Profile* GameState::GetEditLocalProfile()
{
	if( m_sEditLocalProfileID.Get().empty() )
		return nullptr;
	return PROFILEMAN->GetLocalProfile( m_sEditLocalProfileID );
}


PlayerNumber GetNextHumanPlayer( PlayerNumber pn )
{
	for( enum_add(pn, 1); pn < NUM_PLAYERS; enum_add(pn, 1) )
		if( GAMESTATE->IsHumanPlayer(pn) )
			return pn;
	return PLAYER_INVALID;
}

PlayerNumber GetNextEnabledPlayer( PlayerNumber pn )
{
	for( enum_add(pn, 1); pn < NUM_PLAYERS; enum_add(pn, 1) )
		if( GAMESTATE->IsPlayerEnabled(pn) )
			return pn;
	return PLAYER_INVALID;
}

PlayerNumber GetNextCpuPlayer( PlayerNumber pn )
{
	for( enum_add(pn, 1); pn < NUM_PLAYERS; enum_add(pn, 1) )
		if( GAMESTATE->IsCpuPlayer(pn) )
			return pn;
	return PLAYER_INVALID;
}

PlayerNumber GetNextPotentialCpuPlayer( PlayerNumber pn )
{
	for( enum_add(pn, 1); pn < NUM_PLAYERS; enum_add(pn, 1) )
		if( !GAMESTATE->IsHumanPlayer(pn) )
			return pn;
	return PLAYER_INVALID;
}

MultiPlayer GetNextEnabledMultiPlayer( MultiPlayer mp )
{
	for( enum_add(mp, 1); mp < NUM_MultiPlayer; enum_add(mp, 1) )
		if( GAMESTATE->IsMultiPlayerEnabled(mp) )
			return mp;
	return MultiPlayer_Invalid;
}



// lua start
#include "LuaBinding.h"
#include "Game.h"

/** @brief Allow Lua to have access to the GameState. */
class LunaGameState: public Luna<GameState>
{
public:
	DEFINE_METHOD( IsPlayerEnabled,			IsPlayerEnabled(Enum::Check<PlayerNumber>(L, 1)) )
	DEFINE_METHOD( IsHumanPlayer,			IsHumanPlayer(Enum::Check<PlayerNumber>(L, 1)) )
	DEFINE_METHOD( GetPlayerDisplayName,		GetPlayerDisplayName(Enum::Check<PlayerNumber>(L, 1)) )
	DEFINE_METHOD( GetMasterPlayerNumber,		GetMasterPlayerNumber() )
	DEFINE_METHOD( GetMultiplayer,			m_bMultiplayer )
	static int SetMultiplayer( T* p, lua_State *L )
	{
		p->m_bMultiplayer = BArg(1);
		COMMON_RETURN_SELF;
	}
	DEFINE_METHOD( InStepEditor,			m_bInStepEditor );
	DEFINE_METHOD( GetNumMultiplayerNoteFields,	m_iNumMultiplayerNoteFields )
	DEFINE_METHOD( ShowW1,				ShowW1() )

	static int SetNumMultiplayerNoteFields( T* p, lua_State *L )
	{
		p->m_iNumMultiplayerNoteFields = IArg(1);
		COMMON_RETURN_SELF;
	}
	static int GetPlayerState( T* p, lua_State *L )
	{
		PlayerNumber pn = Enum::Check<PlayerNumber>(L, 1);
		p->m_pPlayerState[pn]->PushSelf(L);
		return 1;
	}
	static int GetMultiPlayerState( T* p, lua_State *L )
	{
		MultiPlayer mp = Enum::Check<MultiPlayer>(L, 1);
		p->m_pMultiPlayerState[mp]->PushSelf(L);
		return 1;
	}
	static int ApplyGameCommand( T* p, lua_State *L )
	{
		PlayerNumber pn = PLAYER_INVALID;
		if( lua_gettop(L) >= 2 && !lua_isnil(L,2) ) {
			// Legacy behavior: if an old-style numerical argument
			// is given, decrement it before trying to parse
			if( lua_isnumber(L,2) ) {
				int arg = (int) lua_tonumber( L, 2 );
				arg--;
				LuaHelpers::Push( L, arg );
				lua_replace( L, -2 );
			}
			pn = Enum::Check<PlayerNumber>(L, 2);
		}
		p->ApplyGameCommand(SArg(1),pn);
		COMMON_RETURN_SELF;
	}
	static int GetCurrentSong( T* p, lua_State *L )			{ if(p->m_pCurSong) p->m_pCurSong->PushSelf(L); else lua_pushnil(L); return 1; }
	static int SetCurrentSong( T* p, lua_State *L )
	{
		if( lua_isnil(L,1) ) { p->m_pCurSong.Set(nullptr); }
		else { Song *pS = Luna<Song>::check( L, 1, true ); p->m_pCurSong.Set( pS ); }
		COMMON_RETURN_SELF;
	}
	static int CanSafelyEnterGameplay(T* p, lua_State* L)
	{
		RString reason;
		bool can= p->CanSafelyEnterGameplay(reason);
		lua_pushboolean(L, can);
		LuaHelpers::Push(L, reason);
		return 2;
	}
	static void SetCompatibleStyleOrError(T* p, lua_State* L, StepsType stype, PlayerNumber pn)
	{
		if(!p->SetCompatibleStyle(stype, pn))
		{
			luaL_error(L, "No compatible style for steps/trail.");
		}
		if(!p->GetCurrentStyle(pn))
		{
			luaL_error(L, "No style set and AutoSetStyle is false, cannot set steps/trail.");
		}
	}
	static int GetCurrentSteps( T* p, lua_State *L )
	{
		PlayerNumber pn = Enum::Check<PlayerNumber>(L, 1);
		Steps *pSteps = p->m_pCurSteps[pn];
		if( pSteps ) { pSteps->PushSelf(L); }
		else		 { lua_pushnil(L); }
		return 1;
	}
	static int SetCurrentSteps( T* p, lua_State *L )
	{
		PlayerNumber pn = Enum::Check<PlayerNumber>(L, 1);
		if(lua_isnil(L,2))
		{
			p->m_pCurSteps[pn].Set(nullptr);
		}
		else
		{
			Steps *pS = Luna<Steps>::check(L,2);
			SetCompatibleStyleOrError(p, L, pS->m_StepsType, pn);
			p->m_pCurSteps[pn].Set(pS);
			p->ForceOtherPlayersToCompatibleSteps(pn);
		}
		COMMON_RETURN_SELF;
	}
	static int GetCurrentCourse( T* p, lua_State *L )		{ if(p->m_pCurCourse) p->m_pCurCourse->PushSelf(L); else lua_pushnil(L); return 1; }
	static int SetCurrentCourse( T* p, lua_State *L )
	{
		if( lua_isnil(L,1) ) { p->m_pCurCourse.Set(nullptr); }
		else { Course *pC = Luna<Course>::check(L,1); p->m_pCurCourse.Set( pC ); }
		COMMON_RETURN_SELF;
	}
	static int GetCurrentTrail( T* p, lua_State *L )
	{
		PlayerNumber pn = Enum::Check<PlayerNumber>(L, 1);
		Trail *pTrail = p->m_pCurTrail[pn];
		if( pTrail ) { pTrail->PushSelf(L); }
		else		 { lua_pushnil(L); }
		return 1;
	}
	static int SetCurrentTrail( T* p, lua_State *L )
	{
		PlayerNumber pn = Enum::Check<PlayerNumber>(L, 1);
		if(lua_isnil(L,2))
		{
			p->m_pCurTrail[pn].Set(nullptr);
		}
		else
		{
			Trail *pS = Luna<Trail>::check(L,2);
			SetCompatibleStyleOrError(p, L, pS->m_StepsType, pn);
			p->m_pCurTrail[pn].Set(pS);
			p->ForceOtherPlayersToCompatibleSteps(pn);
		}
		COMMON_RETURN_SELF;
	}
	static int GetPreferredSong( T* p, lua_State *L )		{ if(p->m_pPreferredSong) p->m_pPreferredSong->PushSelf(L); else lua_pushnil(L); return 1; }
	static int SetPreferredSong( T* p, lua_State *L )
	{
		if( lua_isnil(L,1) ) { p->m_pPreferredSong = nullptr; }
		else { Song *pS = Luna<Song>::check(L,1); p->m_pPreferredSong = pS; }
		COMMON_RETURN_SELF;
	}
	static int SetTemporaryEventMode( T* p, lua_State *L )	{ p->m_bTemporaryEventMode = BArg(1); COMMON_RETURN_SELF; }
	static int Env( T* p, lua_State *L )	{ p->m_Environment->PushSelf(L); return 1; }
	static int GetEditSourceSteps( T* p, lua_State *L )
	{
		Steps *pSteps = p->m_pEditSourceSteps;
		if( pSteps ) { pSteps->PushSelf(L); }
		else		 { lua_pushnil(L); }
		return 1;
	}
	static int SetPreferredDifficulty( T* p, lua_State *L )
	{
		PlayerNumber pn = Enum::Check<PlayerNumber>( L, 1 );
		Difficulty dc = Enum::Check<Difficulty>( L, 2 );
		p->m_PreferredDifficulty[pn].Set( dc );
		COMMON_RETURN_SELF;
	}
	DEFINE_METHOD( GetPreferredDifficulty,		m_PreferredDifficulty[Enum::Check<PlayerNumber>(L, 1)] )
	DEFINE_METHOD( AnyPlayerHasRankingFeats,	AnyPlayerHasRankingFeats() )
	DEFINE_METHOD( IsCourseMode,			IsCourseMode() )
	DEFINE_METHOD( IsBattleMode,			IsBattleMode() )
	DEFINE_METHOD( IsDemonstration,			m_bDemonstrationOrJukebox )
	DEFINE_METHOD( GetPlayMode,			m_PlayMode )
	DEFINE_METHOD( GetSortOrder,			m_SortOrder )
	DEFINE_METHOD( GetCurrentStageIndex,		m_iCurrentStageIndex )
	DEFINE_METHOD( IsGoalComplete,			IsGoalComplete(Enum::Check<PlayerNumber>(L, 1)) )
	DEFINE_METHOD( PlayerIsUsingModifier,		PlayerIsUsingModifier(Enum::Check<PlayerNumber>(L, 1), SArg(2)) )
	DEFINE_METHOD( GetCourseSongIndex,		GetCourseSongIndex() )
	DEFINE_METHOD( GetLoadingCourseSongIndex,	GetLoadingCourseSongIndex() )
	DEFINE_METHOD( GetSmallestNumStagesLeftForAnyHumanPlayer, GetSmallestNumStagesLeftForAnyHumanPlayer() )
	DEFINE_METHOD( IsAnExtraStage,			IsAnExtraStage() )
	DEFINE_METHOD( IsExtraStage,			IsExtraStage() )
	DEFINE_METHOD( IsExtraStage2,			IsExtraStage2() )
	DEFINE_METHOD( GetCurrentStage,			GetCurrentStage() )
	DEFINE_METHOD( HasEarnedExtraStage,		HasEarnedExtraStage() )
	DEFINE_METHOD( GetEarnedExtraStage,		GetEarnedExtraStage() )
	DEFINE_METHOD( GetEasiestStepsDifficulty,	GetEasiestStepsDifficulty() )
	DEFINE_METHOD( GetHardestStepsDifficulty,	GetHardestStepsDifficulty() )
	DEFINE_METHOD( IsEventMode,			IsEventMode() )
	DEFINE_METHOD( GetNumPlayersEnabled,		GetNumPlayersEnabled() )
	/*DEFINE_METHOD( GetSongBeat,			m_Position.m_fSongBeat )
	DEFINE_METHOD( GetSongBeatVisible,		m_Position.m_fSongBeatVisible )
	DEFINE_METHOD( GetSongBPS,			m_Position.m_fCurBPS )
	DEFINE_METHOD( GetSongFreeze,			m_Position.m_bFreeze )
	DEFINE_METHOD( GetSongDelay,			m_Position.m_bDelay )*/
	static int GetSongPosition( T* p, lua_State *L )
	{
		p->m_Position.PushSelf(L);
		return 1;
	}
	DEFINE_METHOD( GetLastGameplayDuration, m_DanceDuration )
	DEFINE_METHOD( GetGameplayLeadIn,		m_bGameplayLeadIn )
	DEFINE_METHOD( GetCoins,			m_iCoins )
	DEFINE_METHOD( IsSideJoined,			m_bSideIsJoined[Enum::Check<PlayerNumber>(L, 1)] )
	DEFINE_METHOD( GetCoinsNeededToJoin,		GetCoinsNeededToJoin() )
	DEFINE_METHOD( EnoughCreditsToJoin,		EnoughCreditsToJoin() )
	DEFINE_METHOD( PlayersCanJoin,			PlayersCanJoin() )
	DEFINE_METHOD( GetNumSidesJoined,		GetNumSidesJoined() )
	DEFINE_METHOD( GetCoinMode,			GetCoinMode() )
	DEFINE_METHOD( GetPremium,			GetPremium() )
	DEFINE_METHOD( GetSongOptionsString,		m_SongOptions.GetCurrent().GetString() )
	static int GetSongOptions( T* p, lua_State *L )
	{
		ModsLevel m = Enum::Check<ModsLevel>( L, 1 );
		RString s = p->m_SongOptions.Get(m).GetString();
		LuaHelpers::Push( L, s );
		return 1;
	}
	static int GetSongOptionsObject( T* p, lua_State *L )
	{
		ModsLevel m = Enum::Check<ModsLevel>( L, 1 );
		p->m_SongOptions.Get(m).PushSelf(L);
		return 1;
	}
	static int GetDefaultSongOptions( T* p, lua_State *L )
	{
		SongOptions so;
		p->GetDefaultSongOptions( so );
		lua_pushstring(L, so.GetString());
		return 1;
	}
	static int ApplyPreferredSongOptionsToOtherLevels(T* p, lua_State* L)
	{
		p->m_SongOptions.Assign(ModsLevel_Preferred,
			p->m_SongOptions.Get(ModsLevel_Preferred));
		return 0;
	}
	static int ApplyStageModifiers( T* p, lua_State *L )
	{
		p->ApplyStageModifiers( Enum::Check<PlayerNumber>(L, 1), SArg(2) );
		COMMON_RETURN_SELF;
	}
	static int ApplyPreferredModifiers( T* p, lua_State *L )
	{
		p->ApplyPreferredModifiers( Enum::Check<PlayerNumber>(L, 1), SArg(2) );
		COMMON_RETURN_SELF;
	}
	static int ClearStageModifiersIllegalForCourse( T* p, lua_State *L )
	{
		p->ClearStageModifiersIllegalForCourse();
		COMMON_RETURN_SELF;
	}
	static int SetSongOptions( T* p, lua_State *L )
	{
		ModsLevel m = Enum::Check<ModsLevel>( L, 1 );

		SongOptions so;

		so.FromString( SArg(2) );
		p->m_SongOptions.Assign( m, so );
		COMMON_RETURN_SELF;
	}
	static int GetStageResult( T* p, lua_State *L )
	{
		PlayerNumber pn = Enum::Check<PlayerNumber>(L, 1);
		LuaHelpers::Push( L, p->GetStageResult(pn) );
		return 1;
	}
	static int IsWinner( T* p, lua_State *L )
	{
		PlayerNumber pn = Enum::Check<PlayerNumber>(L, 1);
		lua_pushboolean(L, p->GetStageResult(pn)==RESULT_WIN); return 1;
	}
	static int IsDraw( T* p, lua_State *L )
	{
		lua_pushboolean(L, p->GetStageResult(PLAYER_1)==RESULT_DRAW); return 1;
	}
	static int GetCurrentGame( T* p, lua_State *L )			{ const_cast<Game*>(p->GetCurrentGame())->PushSelf( L ); return 1; }
	DEFINE_METHOD( GetEditCourseEntryIndex,		m_iEditCourseEntryIndex )
	DEFINE_METHOD( GetEditLocalProfileID,		m_sEditLocalProfileID.Get() )
	static int GetEditLocalProfile( T* p, lua_State *L )
	{
		Profile *pProfile = p->GetEditLocalProfile();
		if( pProfile )
			pProfile->PushSelf(L);
		else
			lua_pushnil( L );
		return 1;
	}

	static int GetCurrentStepsCredits( T* t, lua_State *L )
	{
		const Song* pSong = t->m_pCurSong;
		if( pSong == nullptr )
			return 0;

		// use a vector and not a set so that ordering is maintained
		vector<const Steps*> vpStepsToShow;
		FOREACH_HumanPlayer( p )
		{
			const Steps* pSteps = GAMESTATE->m_pCurSteps[p];
			if( pSteps == nullptr )
				return 0;
			bool bAlreadyAdded = find( vpStepsToShow.begin(), vpStepsToShow.end(), pSteps ) != vpStepsToShow.end();
			if( !bAlreadyAdded )
				vpStepsToShow.push_back( pSteps );
		}

		for( unsigned i=0; i<vpStepsToShow.size(); i++ )
		{
			const Steps* pSteps = vpStepsToShow[i];
			RString sDifficulty = CustomDifficultyToLocalizedString( GetCustomDifficulty( pSteps->m_StepsType, pSteps->GetDifficulty(), CourseType_Invalid ) );

			lua_pushstring( L, sDifficulty );
			lua_pushstring( L, pSteps->GetDescription() );
		}

		return vpStepsToShow.size()*2;
	}

	static int SetPreferredSongGroup( T* p, lua_State *L ) { p->m_sPreferredSongGroup.Set( SArg(1) ); COMMON_RETURN_SELF; }
	DEFINE_METHOD( GetPreferredSongGroup, m_sPreferredSongGroup.Get() );
	static int GetHumanPlayers( T* p, lua_State *L )
	{
		vector<PlayerNumber> vHP;
		FOREACH_HumanPlayer( pn )
			vHP.push_back( pn );
		LuaHelpers::CreateTableFromArray( vHP, L );
		return 1;
	}
	static int GetEnabledPlayers(T* , lua_State *L )
	{
		vector<PlayerNumber> vEP;
		FOREACH_EnabledPlayer( pn )
			vEP.push_back( pn );
		LuaHelpers::CreateTableFromArray( vEP, L );
		return 1;
	}
	static int GetCurrentStyle( T* p, lua_State *L )
	{
		PlayerNumber pn= Enum::Check<PlayerNumber>(L, 1, true, true);
		Style *pStyle = const_cast<Style *> (p->GetCurrentStyle(pn));
		LuaHelpers::Push( L, pStyle );
		return 1;
	}
	static int IsAnyHumanPlayerUsingMemoryCard( T* , lua_State *L )
	{
		bool bUsingMemoryCard = false;
		FOREACH_HumanPlayer( pn )
		{
			if( MEMCARDMAN->GetCardState(pn) == MemoryCardState_Ready )
				bUsingMemoryCard = true;
		}
		lua_pushboolean(L, bUsingMemoryCard );
		return 1;
	}
	static int GetNumStagesForCurrentSongAndStepsOrCourse( T* , lua_State *L )
	{
		lua_pushnumber(L, GAMESTATE->GetNumStagesForCurrentSongAndStepsOrCourse() );
		return 1;
	}
	static int GetNumStagesLeft( T* p, lua_State *L )
	{
		PlayerNumber pn = Enum::Check<PlayerNumber>(L, 1);
		lua_pushnumber(L, p->GetNumStagesLeft(pn));
		return 1;
	}
	static int GetGameSeed( T* p, lua_State *L )			{ LuaHelpers::Push( L, p->m_iGameSeed ); return 1; }
	static int GetStageSeed( T* p, lua_State *L )			{ LuaHelpers::Push( L, p->m_iStageSeed ); return 1; }
	static int SaveLocalData( T* p, lua_State *L )			{ p->SaveLocalData(); COMMON_RETURN_SELF; }

	static int SetJukeboxUsesModifiers( T* p, lua_State *L )
	{
		p->m_bJukeboxUsesModifiers = BArg(1); COMMON_RETURN_SELF;
	}
	static int Reset( T* p, lua_State *L )				{ p->Reset(); COMMON_RETURN_SELF; }
	static int JoinPlayer( T* p, lua_State *L )				{ p->JoinPlayer(Enum::Check<PlayerNumber>(L, 1)); COMMON_RETURN_SELF; }
	static int UnjoinPlayer( T* p, lua_State *L )				{ p->UnjoinPlayer(Enum::Check<PlayerNumber>(L, 1)); COMMON_RETURN_SELF; }
	static int JoinInput( T* p, lua_State *L )
	{
		lua_pushboolean(L, p->JoinInput(Enum::Check<PlayerNumber>(L, 1)));
		return 1;
	}
	static int GetSongPercent( T* p, lua_State *L )				{ lua_pushnumber(L, p->GetSongPercent(FArg(1))); return 1; }
	DEFINE_METHOD( GetCurMusicSeconds,	m_Position.m_fMusicSeconds )

	DEFINE_METHOD( GetWorkoutGoalComplete,		m_bWorkoutGoalComplete )
	static int GetCharacter( T* p, lua_State *L )				{ p->m_pCurCharacters[Enum::Check<PlayerNumber>(L, 1)]->PushSelf(L); return 1; }
	static int SetCharacter( T* p, lua_State *L ){
		Character* c = CHARMAN->GetCharacterFromID(SArg(2));
		if (c)
			p->m_pCurCharacters[Enum::Check<PlayerNumber>(L, 1)] = c;
		COMMON_RETURN_SELF;
	}
	static int GetExpandedSectionName( T* p, lua_State *L )				{ lua_pushstring(L, p->sExpandedSectionName); return 1; }
	static int AddStageToPlayer( T* p, lua_State *L )				{ p->AddStageToPlayer(Enum::Check<PlayerNumber>(L, 1)); COMMON_RETURN_SELF; }
	static int InsertCoin( T* p, lua_State *L )
	{
		int numCoins = IArg(1);
		if (GAMESTATE->m_iCoins + numCoins >= 0)
		{
			StepMania::InsertCoin(numCoins, false);
		} else {
			// Warn themers if they attempt to set credits to a negative value.
			luaL_error( L, "Credits may not be negative." );
		}
		COMMON_RETURN_SELF;
	}
	static int InsertCredit( T* p, lua_State *L )
	{
		StepMania::InsertCredit();
		COMMON_RETURN_SELF;
	}
	static int CurrentOptionsDisqualifyPlayer( T* p, lua_State *L )	{ lua_pushboolean(L, p->CurrentOptionsDisqualifyPlayer(Enum::Check<PlayerNumber>(L, 1))); return 1; }

	static int ResetPlayerOptions( T* p, lua_State *L )
	{
		p->ResetPlayerOptions(Enum::Check<PlayerNumber>(L, 1));
		COMMON_RETURN_SELF;
	}

	static int RefreshNoteSkinData( T* p, lua_State *L )
	{
		NOTESKIN->RefreshNoteSkinData(p->m_pCurGame);
		COMMON_RETURN_SELF;
	}

	static int Dopefish( T* p, lua_State *L )
	{
		lua_pushboolean(L, p->m_bDopefish);
		return 1;
	}

	static int LoadProfiles( T* p, lua_State *L )
	{
		bool LoadEdits = true;
		if(lua_isboolean(L, 1))
		{
			LoadEdits = BArg(1);
		}
		p->LoadProfiles( LoadEdits );
		SCREENMAN->ZeroNextUpdate();
		COMMON_RETURN_SELF;
	}

	static int SaveProfiles( T* p, lua_State *L )
	{
		p->SavePlayerProfiles();
		SCREENMAN->ZeroNextUpdate();
		COMMON_RETURN_SELF;
	}

	static int SetFailTypeExplicitlySet(T* p, lua_State* L)
	{
		p->m_bFailTypeWasExplicitlySet= true;
		COMMON_RETURN_SELF;
	}

	static int StoreRankingName( T* p, lua_State *L )
	{
		p->StoreRankingName(Enum::Check<PlayerNumber>(L, 1), SArg(2));
		COMMON_RETURN_SELF;
	}

	DEFINE_METHOD( HaveProfileToLoad, HaveProfileToLoad() )
	DEFINE_METHOD( HaveProfileToSave, HaveProfileToSave() )

	static bool AreStyleAndPlayModeCompatible( T* p, lua_State* L, const Style *style, PlayMode pm )
	{
		if( pm != PLAY_MODE_BATTLE && pm != PLAY_MODE_RAVE )
		{
			return true;
		}

		// Do not allow styles with StepsTypes with shared sides or that are one player only with Battle or Rave.
		if( style->m_StyleType != StyleType_TwoPlayersSharedSides )
		{
			vector<const Style*> vpStyles;
			GAMEMAN->GetCompatibleStyles( p->m_pCurGame, 2, vpStyles );
			for (const Style *s : vpStyles)
			{
				if( s->m_StepsType == style->m_StepsType )
				{
					return true;
				}
			}
		}
		luaL_error( L, "Style %s is incompatible with PlayMode %s",
			style->m_szName, PlayModeToString( pm ).c_str() );
		return false;
	}

	static void ClearIncompatibleStepsAndTrails( T *p, lua_State* L )
	{
		FOREACH_HumanPlayer( pn )
		{
			const Style *style = p->GetCurrentStyle(pn);
			if( p->m_pCurSteps[pn] && ( !style || style->m_StepsType != p->m_pCurSteps[pn]->m_StepsType ) )
			{
				p->m_pCurSteps[pn].Set( nullptr );
			}
			if( p->m_pCurTrail[pn] && ( !style || style->m_StepsType != p->m_pCurTrail[pn]->m_StepsType ) )
			{
				p->m_pCurTrail[pn].Set( nullptr );
			}
		}
	}

	static int SetCurrentStyle( T* p, lua_State *L )
	{
		const Style* pStyle = nullptr;
		if( lua_isstring(L,1) )
		{
			RString style = SArg(1);
			pStyle = GAMEMAN->GameAndStringToStyle( GAMESTATE->m_pCurGame, style );
			if( !pStyle )
			{
				luaL_error( L, "SetCurrentStyle: %s is not a valid style.", style.c_str() );
			}
		}
		else
		{
			pStyle = Luna<Style>::check(L,1);
		}

		StyleType st = pStyle->m_StyleType;
		if( p->GetNumSidesJoined() == 2 &&
			( st == StyleType_OnePlayerOneSide || st == StyleType_OnePlayerTwoSides ) )
		{
			luaL_error( L, "Too many sides joined for style %s", pStyle->m_szName );
		}
		else if( p->GetNumSidesJoined() == 1 &&
			( st == StyleType_TwoPlayersTwoSides || st == StyleType_TwoPlayersSharedSides ) )
		{
			luaL_error( L, "Too few sides joined for style %s", pStyle->m_szName );
		}

		if( !AreStyleAndPlayModeCompatible( p, L, pStyle, p->m_PlayMode ) )
		{
			COMMON_RETURN_SELF;
		}
		PlayerNumber pn= Enum::Check<PlayerNumber>(L, 2, true, true);

		p->SetCurrentStyle(pStyle, pn);
		ClearIncompatibleStepsAndTrails( p, L );

		COMMON_RETURN_SELF;
	}

	static int SetCurrentPlayMode( T* p, lua_State *L )
	{
		PlayMode pm = Enum::Check<PlayMode>( L, 1 );
		if( AreStyleAndPlayModeCompatible( p, L, p->GetCurrentStyle(PLAYER_INVALID), pm ) )
		{
			p->m_PlayMode.Set( pm );
		}
		COMMON_RETURN_SELF;
	}

	static int SetStepsForEditMode(T* p, lua_State *L)
	{
		// Arg forms:
		// 1.  Edit existing steps:
		//    song, steps
		// 2.  Create new steps to edit:
		//    song, nil, stepstype, difficulty
		// 3.  Copy steps to new difficulty to edit:
		//    song, steps, stepstype, difficulty
		Song* song= Luna<Song>::check(L, 1);
		Steps* steps= nullptr;
		if(!lua_isnil(L, 2))
		{
			steps= Luna<Steps>::check(L, 2);
		}
		// Form 1.
		if(steps != nullptr && lua_gettop(L) == 2)
		{
			p->m_pCurSong.Set(song);
			p->m_pCurSteps[PLAYER_1].Set(steps);
			p->SetCurrentStyle(GAMEMAN->GetEditorStyleForStepsType(
					steps->m_StepsType), PLAYER_INVALID);
			p->m_pCurCourse.Set(nullptr);
			return 0;
		}
		StepsType stype= Enum::Check<StepsType>(L, 3);
		Difficulty diff= Enum::Check<Difficulty>(L, 4);
		Steps* new_steps= song->CreateSteps();
		RString edit_name;
		// Form 2.
		if(steps == nullptr)
		{
			new_steps->CreateBlank(stype);
			new_steps->SetMeter(1);
			edit_name= "";
		}
		// Form 3.
		else
		{
			new_steps->CopyFrom(steps, stype, song->m_fMusicLengthSeconds);
			edit_name= steps->GetDescription();
		}
		SongUtil::MakeUniqueEditDescription(song, stype, edit_name);
		steps->SetDescription(edit_name);
		song->AddSteps(new_steps);
		p->m_pCurSong.Set(song);
		p->m_pCurSteps[PLAYER_1].Set(steps);
		p->SetCurrentStyle(GAMEMAN->GetEditorStyleForStepsType(
				steps->m_StepsType), PLAYER_INVALID);
		p->m_pCurCourse.Set(nullptr);
		return 0;
	}

	static int GetAutoGenFarg(T* p, lua_State *L)
	{
		int i= IArg(1) - 1;
		if(i < 0) { lua_pushnil(L); return 1; }
		size_t si= static_cast<size_t>(i);
		if(si >= p->m_autogen_fargs.size()) { lua_pushnil(L); return 1; }
		lua_pushnumber(L, p->GetAutoGenFarg(si));
		return 1;
	}
	static int SetAutoGenFarg(T* p, lua_State* L)
	{
		int i= IArg(1) - 1;
		if(i < 0)
		{
			luaL_error(L, "%i is not a valid autogen arg index.", i);
		}
		float v= FArg(2);
		size_t si= static_cast<size_t>(i);
		while(si >= p->m_autogen_fargs.size())
		{
			p->m_autogen_fargs.push_back(0.0f);
		}
		p->m_autogen_fargs[si]= v;
		COMMON_RETURN_SELF;
	}
	static int prepare_song_for_gameplay(T* p, lua_State* L)
	{
		int result= p->prepare_song_for_gameplay();
		lua_pushstring(L, prepare_song_failures[result]);
		return 1;
	}

	LunaGameState()
	{
		ADD_METHOD( IsPlayerEnabled );
		ADD_METHOD( IsHumanPlayer );
		ADD_METHOD( GetPlayerDisplayName );
		ADD_METHOD( GetMasterPlayerNumber );
		ADD_METHOD( GetMultiplayer );
		ADD_METHOD( SetMultiplayer );
		ADD_METHOD( InStepEditor );
		ADD_METHOD( GetNumMultiplayerNoteFields );
		ADD_METHOD( SetNumMultiplayerNoteFields );
		ADD_METHOD( ShowW1 );
		ADD_METHOD( GetPlayerState );
		ADD_METHOD( GetMultiPlayerState );
		ADD_METHOD( ApplyGameCommand );
		ADD_METHOD( CanSafelyEnterGameplay );
		ADD_METHOD( GetCurrentSong );
		ADD_METHOD( SetCurrentSong );
		ADD_METHOD( GetCurrentSteps );
		ADD_METHOD( SetCurrentSteps );
		ADD_METHOD( GetCurrentCourse );
		ADD_METHOD( SetCurrentCourse );
		ADD_METHOD( GetCurrentTrail );
		ADD_METHOD( SetCurrentTrail );
		ADD_METHOD( SetPreferredSong );
		ADD_METHOD( GetPreferredSong );
		ADD_METHOD( SetTemporaryEventMode );
		ADD_METHOD( Env );
		ADD_METHOD( GetEditSourceSteps );
		ADD_METHOD( SetPreferredDifficulty );
		ADD_METHOD( GetPreferredDifficulty );
		ADD_METHOD( AnyPlayerHasRankingFeats );
		ADD_METHOD( IsCourseMode );
		ADD_METHOD( IsBattleMode );
		ADD_METHOD( IsDemonstration );
		ADD_METHOD( GetPlayMode );
		ADD_METHOD( GetSortOrder );
		ADD_METHOD( GetCurrentStageIndex );
		ADD_METHOD( IsGoalComplete );
		ADD_METHOD( PlayerIsUsingModifier );
		ADD_METHOD( GetCourseSongIndex );
		ADD_METHOD( GetLoadingCourseSongIndex );
		ADD_METHOD( GetSmallestNumStagesLeftForAnyHumanPlayer );
		ADD_METHOD( IsAnExtraStage );
		ADD_METHOD( IsExtraStage );
		ADD_METHOD( IsExtraStage2 );
		ADD_METHOD( GetCurrentStage );
		ADD_METHOD( HasEarnedExtraStage );
		ADD_METHOD( GetEarnedExtraStage );
		ADD_METHOD( GetEasiestStepsDifficulty );
		ADD_METHOD( GetHardestStepsDifficulty );
		ADD_METHOD( IsEventMode );
		ADD_METHOD( GetNumPlayersEnabled );
		/*ADD_METHOD( GetSongBeat );
		ADD_METHOD( GetSongBeatVisible );
		ADD_METHOD( GetSongBPS );
		ADD_METHOD( GetSongFreeze );
		ADD_METHOD( GetSongDelay );*/
		ADD_METHOD( GetSongPosition );
		ADD_METHOD( GetLastGameplayDuration );
		ADD_METHOD( GetGameplayLeadIn );
		ADD_METHOD( GetCoins );
		ADD_METHOD( IsSideJoined );
		ADD_METHOD( GetCoinsNeededToJoin );
		ADD_METHOD( EnoughCreditsToJoin );
		ADD_METHOD( PlayersCanJoin );
		ADD_METHOD( GetNumSidesJoined );
		ADD_METHOD( GetCoinMode );
		ADD_METHOD( GetPremium );
		ADD_METHOD( GetSongOptionsString );
		ADD_METHOD( GetSongOptions );
		ADD_METHOD( GetSongOptionsObject );
		ADD_METHOD( GetDefaultSongOptions );
		ADD_METHOD( ApplyPreferredSongOptionsToOtherLevels );
		ADD_METHOD( ApplyPreferredModifiers );
		ADD_METHOD( ApplyStageModifiers );
		ADD_METHOD( ClearStageModifiersIllegalForCourse );
		ADD_METHOD( SetSongOptions );
		ADD_METHOD( GetStageResult );
		ADD_METHOD( IsWinner );
		ADD_METHOD( IsDraw );
		ADD_METHOD( GetCurrentGame );
		ADD_METHOD( GetEditCourseEntryIndex );
		ADD_METHOD( GetEditLocalProfileID );
		ADD_METHOD( GetEditLocalProfile );
		ADD_METHOD( GetCurrentStepsCredits );
		ADD_METHOD( SetPreferredSongGroup );
		ADD_METHOD( GetPreferredSongGroup );
		ADD_METHOD( GetHumanPlayers );
		ADD_METHOD( GetEnabledPlayers );
		ADD_METHOD( GetCurrentStyle );
		ADD_METHOD( IsAnyHumanPlayerUsingMemoryCard );
		ADD_METHOD( GetNumStagesForCurrentSongAndStepsOrCourse );
		ADD_METHOD( GetNumStagesLeft );
		ADD_METHOD( GetGameSeed );
		ADD_METHOD( GetStageSeed );
		ADD_METHOD( SaveLocalData );
		ADD_METHOD( SetJukeboxUsesModifiers );
		ADD_METHOD( GetWorkoutGoalComplete );
		ADD_METHOD( Reset );
		ADD_METHOD( JoinPlayer );
		ADD_METHOD( UnjoinPlayer );
		ADD_METHOD( JoinInput );
		ADD_METHOD( GetSongPercent );
		ADD_METHOD( GetCurMusicSeconds );
		ADD_METHOD( GetCharacter );
		ADD_METHOD( SetCharacter );
		ADD_METHOD( GetExpandedSectionName );
		ADD_METHOD( AddStageToPlayer );
		ADD_METHOD( InsertCoin );
		ADD_METHOD( InsertCredit );
		ADD_METHOD( CurrentOptionsDisqualifyPlayer );
		ADD_METHOD( ResetPlayerOptions );
		ADD_METHOD( RefreshNoteSkinData );
		ADD_METHOD( Dopefish );
		ADD_METHOD( LoadProfiles );
		ADD_METHOD( SaveProfiles );
		ADD_METHOD( HaveProfileToLoad );
		ADD_METHOD( HaveProfileToSave );
		ADD_METHOD( SetFailTypeExplicitlySet );
		ADD_METHOD( StoreRankingName );
		ADD_METHOD( SetCurrentStyle );
		ADD_METHOD( SetCurrentPlayMode );
		ADD_METHOD( SetStepsForEditMode );
		ADD_METHOD( GetAutoGenFarg );
		ADD_METHOD( SetAutoGenFarg );
		ADD_METHOD(prepare_song_for_gameplay);
	}
};

LUA_REGISTER_CLASS( GameState )
// lua end

/*
 * (c) 2001-2004 Chris Danford, Glenn Maynard, Chris Gomez
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
