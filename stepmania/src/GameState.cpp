#include "global.h"
#include "GameState.h"
#include "IniFile.h"
#include "GameManager.h"
#include "PrefsManager.h"
#include "InputMapper.h"
#include "song.h"
#include "Course.h"
#include "RageLog.h"
#include "RageUtil.h"
#include "SongManager.h"
#include "Steps.h"
#include "NoteSkinManager.h"
#include "ModeChoice.h"
#include "NoteFieldPositioning.h"
#include "Character.h"
#include "UnlockSystem.h"
#include "AnnouncerManager.h"
#include "ProfileManager.h"
#include "arch/arch.h"
#include "ThemeManager.h"
#include "LightsManager.h"
#include "RageFile.h"
#include "Bookkeeper.h"
#include "MemoryCardManager.h"
#include "StageStats.h"
#include "GameConstantsAndTypes.h"
#include "StepMania.h"
#include "CommonMetrics.h"

#include <ctime>
#include <set>


GameState*	GAMESTATE = NULL;	// global and accessable from anywhere in our program

#define CHARACTERS_DIR "Characters/"
#define NAMES_BLACKLIST_FILE "Data/NamesBlacklist.dat"

GameState::GameState()
{
	m_pPosition = NULL;
	m_pCurStyle = NULL;

	m_pCurGame = NULL;
	m_iCoins = 0;
	m_timeGameStarted.SetZero();
	m_bIsOnSystemMenu = false;

	ReloadCharacters();

	m_iNumTimesThroughAttract = -1;	// initial screen will bump this up to 0
	m_iRoundSeed = m_iGameSeed = 0;

	m_PlayMode = PLAY_MODE_INVALID; // used by IsPlayerEnabled before the first screen
	FOREACH_PlayerNumber( p )
		m_bSideIsJoined[p] = false; // used by GetNumSidesJoined before the first screen

	/* Don't reset yet; let the first screen do it, so we can
	 * use PREFSMAN and THEME. */
//	Reset();
}

GameState::~GameState()
{
	delete m_pPosition;
	for( unsigned i=0; i<m_pCharacters.size(); i++ )
		delete m_pCharacters[i];
}

void GameState::ApplyCmdline()
{
	int i;

	/* We need to join players before we can set the style. */
	CString sPlayer;
	for( i = 0; GetCommandlineArgument( "player", &sPlayer, i ); ++i )
	{
		int pn = atoi( sPlayer )-1;
		if( !IsAnInt( sPlayer ) || pn < 0 || pn >= NUM_PLAYERS )
			RageException::Throw( "Invalid argument \"--player=%s\"", sPlayer.c_str() );

		this->JoinPlayer( (PlayerNumber) pn );
	}

	CString sMode;
	for( i = 0; GetCommandlineArgument( "mode", &sMode, i ); ++i )
	{
		ModeChoice m;
		m.Load( 0, sMode );
		CString why;
		if( !m.IsPlayable(&why) )
			RageException::Throw( "Can't apply mode \"%s\": %s", sMode.c_str(), why.c_str() );

		m.ApplyToAllPlayers();
	}
}

void GameState::Reset()
{
	EndGame();
	
	MEMCARDMAN->LockCards( true );
	
	ASSERT( THEME );

	m_timeGameStarted.SetZero();
	m_pCurStyle = NULL;
	FOREACH_PlayerNumber( p )
		m_bSideIsJoined[p] = false;
	m_bPlayersFinalized = false;
//	m_iCoins = 0;	// don't reset coin count!
	m_MasterPlayerNumber = PLAYER_INVALID;
	m_mapEnv.clear();
	m_sPreferredGroup	= GROUP_ALL_MUSIC;
	m_bChangedFailType = false;
	FOREACH_PlayerNumber( p )
	{
		m_PreferredDifficulty[p] = DIFFICULTY_INVALID;
		m_PreferredCourseDifficulty[p] = DIFFICULTY_MEDIUM;
	}
	m_SortOrder = SORT_INVALID;
	m_PlayMode = PLAY_MODE_INVALID;
	m_bEditing = false;
	m_bDemonstrationOrJukebox = false;
	m_bJukeboxUsesModifiers = false;
	m_iCurrentStageIndex = 0;
	m_bAllow2ndExtraStage = true;
	m_BeatToNoteSkinRev = 0;
	m_iNumStagesOfThisSong = 0;

	NOTESKIN->RefreshNoteSkinData( this->m_pCurGame );

	m_iGameSeed = rand();
	m_iRoundSeed = rand();

	m_pCurSong = NULL;
	m_pPreferredSong = NULL;
	FOREACH_PlayerNumber( p )
		m_pCurSteps[p] = NULL;
	m_pCurCourse = NULL;
	m_pPreferredCourse = NULL;
	FOREACH_PlayerNumber( p )
		m_pCurTrail[p] = NULL;

	SAFE_DELETE( m_pPosition );
	m_pPosition = new NoteFieldPositioning("Positioning.ini");

	FOREACH_PlayerNumber( p )
		m_bAttackBeganThisUpdate[p] = false;

	ResetMusicStatistics();
	ResetStageStatistics();
	SONGMAN->FreeAllLoadedFromProfiles();
	SONGMAN->UpdateBest();
	SONGMAN->UpdateShuffled();

	/* We may have cached trails from before everything was loaded (eg. from before
	 * SongManager::UpdateBest could be called).  Erase the cache. */
	SONGMAN->RegenerateNonFixedCourses();

	g_vPlayedStageStats.clear();

	FOREACH_PlayerNumber( p )
	{	
		m_CurrentPlayerOptions[p].Init();
		m_PlayerOptions[p].Init();
		m_StoredPlayerOptions[p].Init();
	}
	m_SongOptions.Init();
	
	FOREACH_PlayerNumber(p)
	{
		// I can't think of a good reason to have both game-specific
		// default mods and theme specific default mods.  We should choose 
		// one or the other. -Chris
		// Having default modifiers in prefs is needed for several things.
		// The theme setting is for eg. BM being reverse by default.  (This
		// could be done in the title menu ModeChoice, but then it wouldn't
		// affect demo, and other non-gameplay things ...) -glenn
		ApplyModifiers( p, DEFAULT_MODIFIERS );
		ApplyModifiers( p, PREFSMAN->m_sDefaultModifiers );
	}

	FOREACH_PlayerNumber(p)
	{
		if( PREFSMAN->m_ShowDancingCharacters == PrefsManager::CO_RANDOM)
			m_pCurCharacters[p] = GetRandomCharacter();
		else
			m_pCurCharacters[p] = GetDefaultCharacter();
		ASSERT( m_pCurCharacters[p] );
	}

	FOREACH_PlayerNumber(p)
	{
		m_fSuperMeterGrowthScale[p] = 1;
		m_iCpuSkill[p] = 5;
	}


	LIGHTSMAN->SetLightsMode( LIGHTSMODE_ATTRACT );

	ApplyCmdline();
}

void GameState::JoinPlayer( PlayerNumber pn )
{
	this->m_bSideIsJoined[pn] = true;
	if( this->m_MasterPlayerNumber == PLAYER_INVALID )
		this->m_MasterPlayerNumber = pn;

	// if first player to join, set start time
	if( this->GetNumSidesJoined() == 1 )
		this->BeginGame();
}

void GameState::BeginGame()
{
	m_timeGameStarted.Touch();

	m_vpsNamesThatWereFilled.clear();

	// Play attract on the ending screen, then on the ranking screen
	// even if attract sounds are set to off.
	m_iNumTimesThroughAttract = -1;

	MEMCARDMAN->LockCards( false );
}

void GameState::PlayersFinalized()
{
	if( m_bPlayersFinalized )
		return;

	m_bPlayersFinalized = true;

	MEMCARDMAN->LockCards( true );


	// apply saved default modifiers if any
	FOREACH_HumanPlayer( pn )
	{
		PROFILEMAN->LoadFirstAvailableProfile( pn );	// load full profile

		if( !PROFILEMAN->IsUsingProfile(pn) )
			continue;	// skip

		Profile* pProfile = PROFILEMAN->GetProfile(pn);

		if( pProfile->m_bUsingProfileDefaultModifiers )
		{
			GAMESTATE->m_PlayerOptions[pn].Init();
			GAMESTATE->ApplyModifiers( pn, pProfile->m_sDefaultModifiers );
		}
		// Only set the sort order if it wasn't already set by a ModeChoice (or by an earlier profile)
		if( m_SortOrder == SORT_INVALID && pProfile->m_SortOrder != SORT_INVALID )
			m_SortOrder = pProfile->m_SortOrder;
		if( pProfile->m_LastDifficulty != DIFFICULTY_INVALID )
			m_PreferredDifficulty[pn] = pProfile->m_LastDifficulty;
		if( pProfile->m_LastCourseDifficulty != DIFFICULTY_INVALID )
			m_PreferredCourseDifficulty[pn] = pProfile->m_LastCourseDifficulty;
		if( m_pPreferredSong == NULL )
			m_pPreferredSong = pProfile->m_lastSong.ToSong();
		if( m_pPreferredCourse == NULL )
			m_pPreferredCourse = pProfile->m_lastCourse.ToCourse();
	}

	SONGMAN->LoadAllFromProfiles();

	FOREACH_PlayerNumber( pn )
	{
		if( !IsHumanPlayer(pn) )
			ApplyModifiers( pn, DEFAULT_CPU_MODIFIERS );
	}
}

/* This data is added to each player profile, and to the machine profile per-player. */
void AddPlayerStatsToProfile( Profile *pProfile, const StageStats &ss, PlayerNumber pn )
{
	ss.AssertValid( pn );
	CHECKPOINT;

	StyleID sID;
	sID.FromStyle( ss.pStyle );

	ASSERT( ss.vpSongs.size() == ss.vpSteps[pn].size() );
	for( unsigned i=0; i<ss.vpSongs.size(); i++ )
	{
		Steps *pSteps = ss.vpSteps[pn][i];

		pProfile->m_iNumSongsPlayedByPlayMode[ss.playMode]++;
		pProfile->m_iNumSongsPlayedByStyle[sID] ++;
		pProfile->m_iNumSongsPlayedByDifficulty[pSteps->GetDifficulty()] ++;

		int iMeter = clamp( pSteps->GetMeter(), 0, MAX_METER );
		pProfile->m_iNumSongsPlayedByMeter[iMeter] ++;
	}
	
	pProfile->m_iTotalDancePoints += ss.iActualDancePoints[pn];

	if( ss.StageType == StageStats::STAGE_EXTRA || ss.StageType == StageStats::STAGE_EXTRA2 )
	{
		if( ss.bFailed[pn] )
			++pProfile->m_iNumExtraStagesFailed;
		else
			++pProfile->m_iNumExtraStagesPassed;
	}

	// If you fail in a course, you passed all but the final song.
	// FIXME: Not true.  If playing with 2 players, one player could have failed earlier.
	if( !ss.bFailed[pn] )
	{
		pProfile->m_iNumStagesPassedByPlayMode[ss.playMode] ++;
		pProfile->m_iNumStagesPassedByGrade[ss.GetGrade(pn)] ++;
	}
}

void GameState::EndGame()
{
	LOG->Trace( "GameState::EndGame" );

	if( m_bDemonstrationOrJukebox )
		return;
	if( m_timeGameStarted.IsZero() || !g_vPlayedStageStats.size() )	// we were in the middle of a game and played at least one song
		return;

	/* Finish the final stage. */
	FinishStage();


	// Update totalPlaySeconds stat
	int iPlaySeconds = max( 0, (int) m_timeGameStarted.PeekDeltaTime() );

	Profile* pMachineProfile = PROFILEMAN->GetMachineProfile();
	pMachineProfile->m_iTotalPlaySeconds += iPlaySeconds;
	pMachineProfile->m_iTotalPlays++;

	FOREACH_HumanPlayer( p )
	{
		Profile* pPlayerProfile = PROFILEMAN->GetProfile( p );
		if( pPlayerProfile )
		{
			pPlayerProfile->m_iTotalPlaySeconds += iPlaySeconds;
			pPlayerProfile->m_iTotalPlays++;
		}
	}



	BOOKKEEPER->WriteToDisk();
	PROFILEMAN->SaveAllProfiles();

	FOREACH_HumanPlayer( pn )
	{
		if( !PROFILEMAN->IsUsingProfile(pn) )
			continue;

		PROFILEMAN->UnloadProfile( pn );
	}

	// Reset the USB storage device numbers -after- saving
	CHECKPOINT;
	MEMCARDMAN->FlushAndReset();
	CHECKPOINT;

	SONGMAN->FreeAllLoadedFromProfiles();

	// make sure we don't execute EndGame twice.
	m_timeGameStarted.SetZero();
}

void GameState::SaveCurrentSettingsToProfile( PlayerNumber pn )
{
	if( !PROFILEMAN->IsUsingProfile(pn) )
		return;
	if( m_bDemonstrationOrJukebox )
		return;

	Profile* pProfile = PROFILEMAN->GetProfile(pn);

	pProfile->m_bUsingProfileDefaultModifiers = true;
	pProfile->m_sDefaultModifiers = m_PlayerOptions[pn].GetSavedPrefsString();
	if( IsSongSort(m_SortOrder) )
		pProfile->m_SortOrder = m_SortOrder;
	if( m_PreferredDifficulty[pn] != DIFFICULTY_INVALID )
		pProfile->m_LastDifficulty = m_PreferredDifficulty[pn];
	if( m_PreferredCourseDifficulty[pn] != DIFFICULTY_INVALID )
		pProfile->m_LastCourseDifficulty = m_PreferredCourseDifficulty[pn];
	if( m_pPreferredSong )
		pProfile->m_lastSong.FromSong( m_pPreferredSong );
	if( m_pPreferredCourse )
		pProfile->m_lastCourse.FromCourse( m_pPreferredCourse );
}

void GameState::Update( float fDelta )
{
	FOREACH_PlayerNumber( p )
	{
		m_CurrentPlayerOptions[p].Approach( m_PlayerOptions[p], fDelta );

		// TRICKY: GAMESTATE->Update is run before any of the Screen update's,
		// so we'll clear these flags here and let them get turned on later
		m_bAttackBeganThisUpdate[p] = false;
		m_bAttackEndedThisUpdate[p] = false;

		bool bRebuildPlayerOptions = false;

		/* See if any delayed attacks are starting or ending. */
		for( unsigned s=0; s<m_ActiveAttacks[p].size(); s++ )
		{
			Attack &attack = m_ActiveAttacks[p][s];
			
			// -1 is the "starts now" sentinel value.  You must add the attack
			// by calling GameState::LaunchAttack, or else the -1 won't be 
			// converted into the current music time.  
			ASSERT( attack.fStartSecond != -1 );

			bool bCurrentlyEnabled =
				attack.bGlobal ||
				( attack.fStartSecond < this->m_fMusicSeconds &&
				m_fMusicSeconds < attack.fStartSecond+attack.fSecsRemaining );

			if( m_ActiveAttacks[p][s].bOn == bCurrentlyEnabled )
				continue; /* OK */

			if( m_ActiveAttacks[p][s].bOn && !bCurrentlyEnabled )
				m_bAttackEndedThisUpdate[p] = true;
			else if( !m_ActiveAttacks[p][s].bOn && bCurrentlyEnabled )
				m_bAttackBeganThisUpdate[p] = true;

			bRebuildPlayerOptions = true;

			m_ActiveAttacks[p][s].bOn = bCurrentlyEnabled;
		}

		if( bRebuildPlayerOptions )
			RebuildPlayerOptionsFromActiveAttacks( (PlayerNumber)p );

		if( m_fSecondsUntilAttacksPhasedOut[p] > 0 )
			m_fSecondsUntilAttacksPhasedOut[p] = max( 0, m_fSecondsUntilAttacksPhasedOut[p] - fDelta );
	}
}

void GameState::ReloadCharacters()
{
	unsigned i;

	for( i=0; i<m_pCharacters.size(); i++ )
		delete m_pCharacters[i];
	m_pCharacters.clear();

	FOREACH_PlayerNumber( p )
		m_pCurCharacters[p] = NULL;

	CStringArray as;
	GetDirListing( CHARACTERS_DIR "*", as, true, true );
	bool FoundDefault = false;
	for( i=0; i<as.size(); i++ )
	{
		CString sCharName, sDummy;
		splitpath(as[i], sDummy, sCharName, sDummy);
		sCharName.MakeLower();

		if( sCharName == "cvs" )	// the directory called "CVS"
			continue;		// ignore it

		if( sCharName.CompareNoCase("default")==0 )
			FoundDefault = true;

		Character* pChar = new Character;
		if( pChar->Load( as[i] ) )
			m_pCharacters.push_back( pChar );
		else
			delete pChar;
	}
	
	if( !FoundDefault )
		RageException::Throw( "'Characters/default' is missing." );

	// If FoundDefault, then we're not empty. -Chris
//	if( m_pCharacters.empty() )
//		RageException::Throw( "Couldn't find any character definitions" );
}

const float GameState::MUSIC_SECONDS_INVALID = -5000.0f;

void GameState::ResetMusicStatistics()
{	
	m_fMusicSeconds = 0; // MUSIC_SECONDS_INVALID;
	m_fSongBeat = 0;
	m_fCurBPS = 10;
	m_bFreeze = false;
	m_bPastHereWeGo = false;
	Actor::SetBGMTime( 0 );
}

void GameState::ResetStageStatistics()
{
	StageStats OldStats = g_CurStageStats;
	g_CurStageStats = StageStats();
	if( PREFSMAN->m_bComboContinuesBetweenSongs )
	{
		if( GetStageIndex() == 0 )
		{
			FOREACH_PlayerNumber( p )
			{
				Profile* pProfile = PROFILEMAN->GetProfile((PlayerNumber)p);
				if( pProfile )
					g_CurStageStats.iCurCombo[p] = pProfile->m_iCurrentCombo;
			}
		}
		else	// GetStageIndex() > 0
		{
			memcpy( g_CurStageStats.iCurCombo, OldStats.iCurCombo,  sizeof(OldStats.iCurCombo) );
		}
	}

	RemoveAllActiveAttacks();
	RemoveAllInventory();
	m_fOpponentHealthPercent = 1;
	m_fTugLifePercentP1 = 0.5f;
	FOREACH_PlayerNumber( p )
	{
		m_fSuperMeter[p] = 0;
		m_HealthState[p] = ALIVE;

		m_iLastPositiveSumOfAttackLevels[p] = 0;
		m_fSecondsUntilAttacksPhasedOut[p] = 0;	// PlayerAI not affected
	}


	FOREACH_PlayerNumber( p )
	{
		m_vLastPerDifficultyAwards[p].clear();
		m_vLastPeakComboAwards[p].clear();
	}
}

void GameState::UpdateSongPosition( float fPositionSeconds, const TimingData &timing, const RageTimer &timestamp )
{
	if( !timestamp.IsZero() )
		m_LastBeatUpdate = timestamp;
	else
		m_LastBeatUpdate.Touch();
	timing.GetBeatAndBPSFromElapsedTime( fPositionSeconds, m_fSongBeat, m_fCurBPS, m_bFreeze );
	ASSERT_M( m_fSongBeat > -2000, ssprintf("%f %f", m_fSongBeat, fPositionSeconds) );

	m_fMusicSeconds = fPositionSeconds;

	Actor::SetBGMTime( m_fSongBeat );
	
//	LOG->Trace( "m_fMusicSeconds = %f, m_fSongBeat = %f, m_fCurBPS = %f, m_bFreeze = %f", m_fMusicSeconds, m_fSongBeat, m_fCurBPS, m_bFreeze );
}

float GameState::GetSongPercent( float beat ) const
{
	/* 0 = first step; 1 = last step */
	return (beat - m_pCurSong->m_fFirstBeat) / m_pCurSong->m_fLastBeat;
}

static int GetNumStagesForCurrentSong()
{
	int iNumStagesOfThisSong = 1;
	if( GAMESTATE->m_pCurSong )
		iNumStagesOfThisSong = SongManager::GetNumStagesForSong( GAMESTATE->m_pCurSong );
	else if( GAMESTATE->m_pCurCourse )
		iNumStagesOfThisSong = 1;
	else
		return -1;

	ASSERT( iNumStagesOfThisSong >= 1 && iNumStagesOfThisSong <= 3 );

	/* Never increment more than one past final stage.  That is, if the current
	 * stage is the final stage, and we picked a stage that takes two songs, it
	 * only counts as one stage (so it doesn't bump us all the way to Ex2).
	 * One case where this happens is a long/marathon extra stage.  Another is
	 * if a long/marathon song is selected explicitly in the theme with a ModeChoice,
	 * and PREFSMAN->m_iNumArcadeStages is less than the number of stages that
	 * song takes. */
	int iNumStagesLeft = PREFSMAN->m_iNumArcadeStages - GAMESTATE->m_iCurrentStageIndex;
	iNumStagesOfThisSong = min( iNumStagesOfThisSong, iNumStagesLeft );
	iNumStagesOfThisSong = max( iNumStagesOfThisSong, 1 );

	return iNumStagesOfThisSong;
}

/* Called by ScreenGameplay.  Set the length of the current song. */
void GameState::BeginStage()
{
	if( m_bDemonstrationOrJukebox )
		return;

	/* This should only be called once per stage. */
	if( m_iNumStagesOfThisSong != 0 )
		LOG->Warn( "XXX: m_iNumStagesOfThisSong == %i?", m_iNumStagesOfThisSong );

	/* Finish the last stage (if any), if we havn't already.  (For example, we might
	 * have, for some reason, gone from gameplay to evaluation straight back to gameplay.) */
	FinishStage();

	m_iNumStagesOfThisSong = GetNumStagesForCurrentSong();
	ASSERT( m_iNumStagesOfThisSong != -1 );
}

void GameState::CancelStage()
{
	m_iNumStagesOfThisSong = 0;
}

/* Called by ScreenSelectMusic (etc).  Increment the stage counter if we just played a
 * song.  Might be called more than once. */
void GameState::FinishStage()
{
	/* If m_iNumStagesOfThisSong is 0, we've been called more than once before calling
	 * BeginStage.  This can happen when backing out of the player options screen. */
	if( m_iNumStagesOfThisSong == 0 )
		return;

	// Increment the stage counter.
	ASSERT( m_iNumStagesOfThisSong >= 1 && m_iNumStagesOfThisSong <= 3 );
	const int iOldStageIndex = m_iCurrentStageIndex;
	m_iCurrentStageIndex += m_iNumStagesOfThisSong;

	m_iNumStagesOfThisSong = 0;

	// The round has ended; change the seed.
	GAMESTATE->m_iRoundSeed = rand();

	if( m_bDemonstrationOrJukebox )
		return;

	//
	// Add step totals.  Use radarActual, since the player might have failed part way
	// through the song, in which case we don't want to give credit for the rest of the
	// song.
	//
	FOREACH_HumanPlayer( pn )
	{
		int iNumTapsAndHolds	= (int) g_CurStageStats.radarActual[pn][RADAR_NUM_TAPS_AND_HOLDS];
		int iNumJumps			= (int) g_CurStageStats.radarActual[pn][RADAR_NUM_JUMPS];
		int iNumHolds			= (int) g_CurStageStats.radarActual[pn][RADAR_NUM_HOLDS];
		int iNumMines			= (int) g_CurStageStats.radarActual[pn][RADAR_NUM_MINES];
		int iNumHands			= (int) g_CurStageStats.radarActual[pn][RADAR_NUM_HANDS];
		PROFILEMAN->AddStepTotals( pn, iNumTapsAndHolds, iNumJumps, iNumHolds, iNumMines, iNumHands );
	}


	// Update profile stats
	Profile* pMachineProfile = PROFILEMAN->GetMachineProfile();

	int iGameplaySeconds = (int)truncf(g_CurStageStats.fGameplaySeconds);

	pMachineProfile->m_iTotalGameplaySeconds += iGameplaySeconds;
	pMachineProfile->m_iCurrentCombo = 0;

	CHECKPOINT;
	FOREACH_HumanPlayer( p )
	{
		CHECKPOINT;

		Profile* pPlayerProfile = PROFILEMAN->GetProfile( p );
		if( pPlayerProfile )
		{
			pPlayerProfile->m_iTotalGameplaySeconds += iGameplaySeconds;
			pPlayerProfile->m_iCurrentCombo = 
				PREFSMAN->m_bComboContinuesBetweenSongs ? 
				g_CurStageStats.iCurCombo[p] : 
				0;
		}

		const StageStats& ss = g_CurStageStats;
		AddPlayerStatsToProfile( pMachineProfile, ss, p );

		if( pPlayerProfile )
			AddPlayerStatsToProfile( pPlayerProfile, ss, p );

		CHECKPOINT;
	}



	if( PREFSMAN->m_bEventMode )
	{
		const int iSaveProfileEvery = 3;
		if( iOldStageIndex/iSaveProfileEvery < m_iCurrentStageIndex/iSaveProfileEvery )
		{
			LOG->Trace( "Played %i stages; saving profiles ...", iSaveProfileEvery );
			PROFILEMAN->SaveAllProfiles();
		}
	}
}

int GameState::GetStageIndex() const
{
	return m_iCurrentStageIndex;
}

int GameState::GetNumStagesLeft() const
{
	if( IsExtraStage() || IsExtraStage2() )
		return 1;
	if( PREFSMAN->m_bEventMode )
		return 999;
	return PREFSMAN->m_iNumArcadeStages - m_iCurrentStageIndex;
}

bool GameState::IsFinalStage() const
{
	if( PREFSMAN->m_bEventMode )
		return false;

	if( this->IsCourseMode() )
		return true;

	/* This changes dynamically on ScreenSelectMusic as the wheel turns. */
	int iPredictedStageForCurSong = GetNumStagesForCurrentSong();
	if( iPredictedStageForCurSong == -1 )
		iPredictedStageForCurSong = 1;
	return m_iCurrentStageIndex + iPredictedStageForCurSong == PREFSMAN->m_iNumArcadeStages;
}

bool GameState::IsExtraStage() const
{
	if( PREFSMAN->m_bEventMode )
		return false;
	return m_iCurrentStageIndex == PREFSMAN->m_iNumArcadeStages;
}

bool GameState::IsExtraStage2() const
{
	if( PREFSMAN->m_bEventMode )
		return false;
	return m_iCurrentStageIndex == PREFSMAN->m_iNumArcadeStages+1;
}

CString GameState::GetStageText() const
{
	if( m_bDemonstrationOrJukebox )				return "demo";
	else if( m_PlayMode == PLAY_MODE_ONI )		return "oni";
	else if( m_PlayMode == PLAY_MODE_NONSTOP )	return "nonstop";
	else if( m_PlayMode == PLAY_MODE_ENDLESS )	return "endless";
	else if( PREFSMAN->m_bEventMode )			return "event";
	else if( IsFinalStage() )					return "final";
	else if( IsExtraStage() )					return "extra1";
	else if( IsExtraStage2() )					return "extra2";
	else										return ssprintf("%d",m_iCurrentStageIndex+1);
}

void GameState::GetAllStageTexts( CStringArray &out ) const
{
	out.clear();
	out.push_back( "demo" );
	out.push_back( "oni" );
	out.push_back( "nonstop" );
	out.push_back( "endless" );
	out.push_back( "event" );
	out.push_back( "final" );
	out.push_back( "extra1" );
	out.push_back( "extra2" );
	for( int stage = 0; stage < PREFSMAN->m_iNumArcadeStages; ++stage )
		out.push_back( ssprintf("%d",stage+1) );
}

int GameState::GetCourseSongIndex() const
{
	int iSongIndex = 0;
	/* iSongsPlayed includes the current song, so it's 1-based; subtract one. */
	FOREACH_PlayerNumber( p )
		if( IsPlayerEnabled(p) )
			iSongIndex = max( iSongIndex, g_CurStageStats.iSongsPlayed[p]-1 );
	return iSongIndex;
}

CString GameState::GetPlayerDisplayName( PlayerNumber pn ) const
{
	ASSERT( IsPlayerEnabled(pn) );
	const CString defaultnames[NUM_PLAYERS] = { "Player 1", "Player 2" };
	if( IsHumanPlayer(pn) )
	{
		if( !PROFILEMAN->GetPlayerName(pn).empty() )
			return PROFILEMAN->GetPlayerName(pn);
		else
			return defaultnames[pn];
	}
	else
	{
		return "CPU";
	}
}

bool GameState::PlayersCanJoin() const
{
	return GetNumSidesJoined() == 0 || this->m_pCurStyle == NULL;	// selecting a style finalizes the players
}

bool GameState::EnoughCreditsToJoin() const
{
	switch( PREFSMAN->GetCoinMode() )
	{
	case COIN_PAY:
		return GAMESTATE->m_iCoins >= PREFSMAN->m_iCoinsPerCredit;
	case COIN_HOME:
	case COIN_FREE:
		return true;
	default:
		ASSERT(0);
		return false;
	}
}

int GameState::GetNumSidesJoined() const
{ 
	int iNumSidesJoined = 0;
	for( int c=0; c<NUM_PLAYERS; c++ )
		if( m_bSideIsJoined[c] )
			iNumSidesJoined++;	// left side, and right side
	return iNumSidesJoined;
}

const Game* GameState::GetCurrentGame()
{
	ASSERT( m_pCurGame != NULL );	// the game must be set before calling this
	return m_pCurGame;
}

const Style* GameState::GetCurrentStyle() const
{
	return m_pCurStyle;
}


bool GameState::IsPlayerEnabled( PlayerNumber pn ) const
{
	// In rave, all players are present.  Non-human players are CPU controlled.
	switch( m_PlayMode )
	{
	case PLAY_MODE_BATTLE:
	case PLAY_MODE_RAVE:
		return true;
	}

	return IsHumanPlayer( pn );
}

int	GameState::GetNumPlayersEnabled() const
{
	int count = 0;
	FOREACH_PlayerNumber( p )
		if( IsPlayerEnabled(p) )
			count++;
	return count;
}

bool GameState::PlayerUsingBothSides() const
{
	return this->GetCurrentStyle()->m_StyleType==Style::ONE_PLAYER_TWO_CREDITS;
}

bool GameState::IsHumanPlayer( PlayerNumber pn ) const
{
	if( m_pCurStyle == NULL )	// no style chosen
	{
		if( this->PlayersCanJoin() )	
			return m_bSideIsJoined[pn];	// only allow input from sides that have already joined
		else
			return true;	// if we can't join, then we're on a screen like MusicScroll or GameOver
	}

	switch( GetCurrentStyle()->m_StyleType )
	{
	case Style::TWO_PLAYERS_TWO_CREDITS:
		return true;
	case Style::ONE_PLAYER_ONE_CREDIT:
	case Style::ONE_PLAYER_TWO_CREDITS:
		return pn == m_MasterPlayerNumber;
	default:
		ASSERT(0);		// invalid style type
		return false;
	}
}

int GameState::GetNumHumanPlayers() const
{
	int count = 0;
	FOREACH_PlayerNumber( p )
		if( IsHumanPlayer(p) )
			count++;
	return count;
}

PlayerNumber GameState::GetFirstHumanPlayer() const
{
	FOREACH_PlayerNumber( p )
		if( IsHumanPlayer(p) )
			return p;
	ASSERT(0);	// there must be at least 1 human player
	return PLAYER_INVALID;
}

bool GameState::IsCpuPlayer( PlayerNumber pn ) const
{
	return IsPlayerEnabled(pn) && !IsHumanPlayer(pn);
}

bool GameState::AnyPlayersAreCpu() const
{ 
	FOREACH_PlayerNumber( p )
		if( IsCpuPlayer(p) )
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
	switch( this->m_PlayMode )
	{
	case PLAY_MODE_BATTLE:
		return true;
	default:
		return false;
	}
}

bool GameState::HasEarnedExtraStage() const
{
	if( PREFSMAN->m_bEventMode )
		return false;

	if( !PREFSMAN->m_bAllowExtraStage )
		return false;

	if( this->m_PlayMode != PLAY_MODE_REGULAR )
		return false;

	if( (this->IsFinalStage() || this->IsExtraStage()) )
	{
		FOREACH_PlayerNumber( p )
		{
			if( !this->IsPlayerEnabled(p) )
				continue;	// skip

			if( this->m_pCurSteps[p]->GetDifficulty() != DIFFICULTY_HARD && 
				this->m_pCurSteps[p]->GetDifficulty() != DIFFICULTY_CHALLENGE )
				continue; /* not hard enough! */

			/* If "choose EX" is enabled, then we should only grant EX2 if the chosen
			 * stage was the EX we would have chosen (m_bAllow2ndExtraStage is true). */
			if( PREFSMAN->m_bPickExtraStage && this->IsExtraStage() && !this->m_bAllow2ndExtraStage )
				continue;

			if( g_CurStageStats.GetGrade((PlayerNumber)p) <= GRADE_TIER_3 )
				return true;
		}
	}
	return false;
}

PlayerNumber GameState::GetBestPlayer() const
{
	for( int p=PLAYER_1; p<NUM_PLAYERS; p++ )
		if( GetStageResult( (PlayerNumber)p ) == RESULT_WIN )
			return (PlayerNumber)p;
	return PLAYER_INVALID;	// draw
}

StageResult GameState::GetStageResult( PlayerNumber pn ) const
{
	switch( this->m_PlayMode )
	{
	case PLAY_MODE_BATTLE:
	case PLAY_MODE_RAVE:
		if( fabsf(m_fTugLifePercentP1 - 0.5f) < 0.0001f )
			return RESULT_DRAW;
		switch( pn )
		{
		case PLAYER_1:	return (m_fTugLifePercentP1>=0.5f)?RESULT_WIN:RESULT_LOSE;
		case PLAYER_2:	return (m_fTugLifePercentP1<0.5f)?RESULT_WIN:RESULT_LOSE;
		default:	ASSERT(0); return RESULT_LOSE;
		}
	}

	StageResult win = RESULT_WIN;
	for( int p=PLAYER_1; p<NUM_PLAYERS; p++ )
	{
		if( p == pn )
			continue;

		/* If anyone did just as well, at best it's a draw. */
		if( g_CurStageStats.iActualDancePoints[p] == g_CurStageStats.iActualDancePoints[pn] )
			win = RESULT_DRAW;

		/* If anyone did better, we lost. */
		if( g_CurStageStats.iActualDancePoints[p] > g_CurStageStats.iActualDancePoints[pn] )
			return RESULT_LOSE;
	}
	return win;
}

void GameState::GetFinalEvalStats( StageStats& statsOut ) const
{
	statsOut.Init();

	// Show stats only for the latest 3 normal songs + passed extra stages
	int PassedRegularSongsLeft = 3;
	for( int i = (int)g_vPlayedStageStats.size()-1; i >= 0; --i )
	{
		const StageStats &s = g_vPlayedStageStats[i];

		if( !s.OnePassed() )
			continue;

		if( s.StageType == StageStats::STAGE_NORMAL )
		{
			if( PassedRegularSongsLeft == 0 )
				break;

			--PassedRegularSongsLeft;
		}

		statsOut.AddStats( s );
	}

	if( statsOut.vpSongs.empty() ) return;	// don't divide by 0 below

	/* Scale radar percentages back down to roughly 0..1.  Don't scale RADAR_NUM_TAPS_AND_HOLDS
	 * and the rest, which are counters. */
	// FIXME: Weight each song by the number of stages it took to account for 
	// long, marathon.
	FOREACH_EnabledPlayer( p )
	{
		for( int r = 0; r < RADAR_NUM_TAPS_AND_HOLDS; r++)
		{
			statsOut.radarPossible[p][r] /= statsOut.vpSongs.size();
			statsOut.radarActual[p][r] /= statsOut.vpSongs.size();
		}
	}
}


void GameState::ApplyModifiers( PlayerNumber pn, CString sModifiers )
{
	const SongOptions::FailType ft = this->m_SongOptions.m_FailType;

	m_PlayerOptions[pn].FromString( sModifiers );
	m_SongOptions.FromString( sModifiers );

	if( ft != this->m_SongOptions.m_FailType )
		this->m_bChangedFailType = true;
}

/* Store the player's preferred options.  This is called at the very beginning
 * of gameplay. */
void GameState::StoreSelectedOptions()
{
	FOREACH_PlayerNumber( p )
		this->m_StoredPlayerOptions[p] = this->m_PlayerOptions[p];
	m_StoredSongOptions = m_SongOptions;
}

/* Restore the preferred options.  This is called after a song ends, before
 * setting new course options, so options from one song don't carry into the
 * next and we default back to the preferred options.  This is also called
 * at the end of gameplay to restore options. */
void GameState::RestoreSelectedOptions()
{
	FOREACH_PlayerNumber( p )
		this->m_PlayerOptions[p] = this->m_StoredPlayerOptions[p];
	m_SongOptions = m_StoredSongOptions;
}

bool GameState::IsDisqualified( PlayerNumber pn )
{
	if( !PREFSMAN->m_bDisqualification )
		return false;

	if( GAMESTATE->IsCourseMode() )
	{
		return GAMESTATE->m_PlayerOptions[pn].IsEasierForCourseAndTrail( 
			GAMESTATE->m_pCurCourse, 
			GAMESTATE->m_pCurTrail[pn] );
	}
	else
	{
		return GAMESTATE->m_PlayerOptions[pn].IsEasierForSongAndSteps( 
			GAMESTATE->m_pCurSong, 
			GAMESTATE->m_pCurSteps[pn] );
	}
}

void GameState::ResetNoteSkins()
{
	FOREACH_PlayerNumber( pn )
		ResetNoteSkinsForPlayer( (PlayerNumber) pn );

	++m_BeatToNoteSkinRev;
}

void GameState::ResetNoteSkinsForPlayer( PlayerNumber pn )
{
	m_BeatToNoteSkin[pn].clear();
	m_BeatToNoteSkin[pn][-1000] = this->m_PlayerOptions[pn].m_sNoteSkin;

	++m_BeatToNoteSkinRev;
}

void GameState::GetAllUsedNoteSkins( vector<CString> &out ) const
{
	FOREACH_EnabledPlayer( pn )
	{
		out.push_back( this->m_PlayerOptions[pn].m_sNoteSkin );

		switch( this->m_PlayMode )
		{
		case PLAY_MODE_BATTLE:
		case PLAY_MODE_RAVE:
			for( int al=0; al<NUM_ATTACK_LEVELS; al++ )
			{
				const Character *ch = this->m_pCurCharacters[pn];
				ASSERT( ch );
				const CString* asAttacks = ch->m_sAttacks[al];
				for( int att = 0; att < NUM_ATTACKS_PER_LEVEL; ++att )
				{
					PlayerOptions po;
					po.FromString( asAttacks[att] );
					/* Hack: NoteSkin "default" is never applied as an attack,
					 * so don't waste memory preloading it. */
					if( po.m_sNoteSkin != "" && po.m_sNoteSkin.CompareNoCase("default") )
						out.push_back( po.m_sNoteSkin );
				}
			}
		}

		for( map<float,CString>::const_iterator it = m_BeatToNoteSkin[pn].begin(); 
			it != m_BeatToNoteSkin[pn].end(); ++it )
			out.push_back( it->second );
	}
}

/* From NoteField: */

void GameState::GetUndisplayedBeats( PlayerNumber pn, float TotalSeconds, float &StartBeat, float &EndBeat ) const
{
	/* If reasonable, push the attack forward so notes on screen don't change suddenly. */
	StartBeat = min( this->m_fSongBeat+BEATS_PER_MEASURE*2, m_fLastDrawnBeat[pn] );
	StartBeat = truncf(StartBeat)+1;

	const float StartSecond = this->m_pCurSong->GetElapsedTimeFromBeat( StartBeat );
	const float EndSecond = StartSecond + TotalSeconds;
	EndBeat = this->m_pCurSong->GetBeatFromElapsedTime( EndSecond );
	EndBeat = truncf(EndBeat)+1;
}


void GameState::SetNoteSkinForBeatRange( PlayerNumber pn, CString sNoteSkin, float StartBeat, float EndBeat )
{
	map<float,CString> &BeatToNoteSkin = m_BeatToNoteSkin[pn];

	/* Erase any other note skin settings in this range. */
	map<float,CString>::iterator it = BeatToNoteSkin.lower_bound( StartBeat );
	map<float,CString>::iterator end = BeatToNoteSkin.upper_bound( EndBeat );
	while( it != end )
	{
		map<float,CString>::iterator next = it;
		++next;

		BeatToNoteSkin.erase( it );

		it = next;
	}

	/* Add the skin to m_BeatToNoteSkin.  */
	BeatToNoteSkin[StartBeat] = sNoteSkin;

	/* Return to the default note skin after the duration. */
	BeatToNoteSkin[EndBeat] = m_StoredPlayerOptions[pn].m_sNoteSkin;

	++m_BeatToNoteSkinRev;
}

/* This is called to launch an attack, or to queue an attack if a.fStartSecond
 * is set.  This is also called by GameState::Update when activating a queued attack. */
void GameState::LaunchAttack( PlayerNumber target, Attack a )
{
	LOG->Trace( "Launch attack '%s' against P%d at %f", a.sModifier.c_str(), target+1, a.fStartSecond );

	/* If fStartSecond is -1, it means "launch as soon as possible".  For m_ActiveAttacks,
	 * mark the real time it's starting (now), so Update() can know when the attack started
	 * so it can be removed later.  For m_ModsToApply, leave the -1 in, so Player::Update
	 * knows to apply attack transforms correctly.  (yuck) */
	m_ModsToApply[target].push_back( a );
	if( a.fStartSecond == -1 )
		a.fStartSecond = this->m_fMusicSeconds;
	m_ActiveAttacks[target].push_back( a );

	this->RebuildPlayerOptionsFromActiveAttacks( target );
}

void GameState::RemoveActiveAttacksForPlayer( PlayerNumber pn, AttackLevel al )
{
	for( unsigned s=0; s<m_ActiveAttacks[pn].size(); s++ )
	{
		if( al != NUM_ATTACK_LEVELS && al != m_ActiveAttacks[pn][s].level )
			continue;
		m_ActiveAttacks[pn].erase( m_ActiveAttacks[pn].begin()+s, m_ActiveAttacks[pn].begin()+s+1 );
		--s;
	}
	RebuildPlayerOptionsFromActiveAttacks( pn );
}

void GameState::RemoveAllInventory()
{
	FOREACH_PlayerNumber( p )
		for( int s=0; s<NUM_INVENTORY_SLOTS; s++ )
		{
			m_Inventory[p][s].fSecsRemaining = 0;
			m_Inventory[p][s].sModifier = "";
		}
}

void GameState::RebuildPlayerOptionsFromActiveAttacks( PlayerNumber pn )
{
	// rebuild player options
	PlayerOptions po = m_StoredPlayerOptions[pn];
	for( unsigned s=0; s<m_ActiveAttacks[pn].size(); s++ )
	{
		if( !m_ActiveAttacks[pn][s].bOn )
			continue; /* hasn't started yet */
		po.FromString( m_ActiveAttacks[pn][s].sModifier );
	}
	m_PlayerOptions[pn] = po;


	int iSumOfAttackLevels = GetSumOfActiveAttackLevels( pn );
	if( iSumOfAttackLevels > 0 )
	{
		m_iLastPositiveSumOfAttackLevels[pn] = iSumOfAttackLevels;
		m_fSecondsUntilAttacksPhasedOut[pn] = 10000;	// any positive number that won't run out before the attacks
	}
	else
	{
		// don't change!  m_iLastPositiveSumOfAttackLevels[p] = iSumOfAttackLevels;
		m_fSecondsUntilAttacksPhasedOut[pn] = 2;	// 2 seconds to phase out
	}
}

void GameState::RemoveAllActiveAttacks()	// called on end of song
{
	FOREACH_PlayerNumber( p )
		RemoveActiveAttacksForPlayer( (PlayerNumber)p );
}

int GameState::GetSumOfActiveAttackLevels( PlayerNumber pn ) const
{
	int iSum = 0;

	for( unsigned s=0; s<m_ActiveAttacks[pn].size(); s++ )
		if( m_ActiveAttacks[pn][s].fSecsRemaining > 0 && m_ActiveAttacks[pn][s].level != NUM_ATTACK_LEVELS )
			iSum += m_ActiveAttacks[pn][s].level;

	return iSum;
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

/* Adjust the fail mode based on the chosen difficulty.  This must be called
 * after the difficulty has been finalized (usually in ScreenSelectMusic or
 * ScreenPlayerOptions), and before the fail mode is displayed or used (usually
 * in ScreenSongOptions). */
void GameState::AdjustFailType()
{
	/* Single song mode only. */
	if( this->IsCourseMode() )
		return;

	/* If the player changed the fail mode explicitly, leave it alone. */
	if( this->m_bChangedFailType )
		return;

	/* Find the easiest difficulty notes selected by either player. */
	const Difficulty dc = GetEasiestNotesDifficulty();

	/* Reset the fail type to the default. */
	SongOptions so;
	so.FromString( PREFSMAN->m_sDefaultModifiers );
	this->m_SongOptions.m_FailType = so.m_FailType;

    /* Easy and beginner are never harder than FAIL_END_OF_SONG. */
	if(dc <= DIFFICULTY_EASY)
		setmax(this->m_SongOptions.m_FailType, SongOptions::FAIL_END_OF_SONG);

	/* If beginner's steps were chosen, and this is the first stage,
		* turn off failure completely--always give a second try. */
	if(dc == DIFFICULTY_BEGINNER &&
		!PREFSMAN->m_bEventMode && /* stage index is meaningless in event mode */
		this->m_iCurrentStageIndex == 0)
		setmax(this->m_SongOptions.m_FailType, SongOptions::FAIL_OFF);
}

bool GameState::ShowMarvelous() const
{
	if (PREFSMAN->m_iMarvelousTiming == 2)
		return true;

	if (PREFSMAN->m_iMarvelousTiming == 1)
		if (IsCourseMode())
			return true;

	return false;
}

void GameState::GetCharacters( vector<Character*> &apCharactersOut )
{
	for( unsigned i=0; i<m_pCharacters.size(); i++ )
		if( m_pCharacters[i]->m_sName.CompareNoCase("default")!=0 )
			apCharactersOut.push_back( m_pCharacters[i] );
}

Character* GameState::GetRandomCharacter()
{
	vector<Character*> apCharacters;
	GetCharacters( apCharacters );
	if( apCharacters.size() )
		return apCharacters[rand()%apCharacters.size()];
	else
		return GetDefaultCharacter();
}

Character* GameState::GetDefaultCharacter()
{
	for( unsigned i=0; i<m_pCharacters.size(); i++ )
	{
		if( m_pCharacters[i]->m_sName.CompareNoCase("default")==0 )
			return m_pCharacters[i];
	}

	/* We always have the default character. */
	ASSERT(0);
	return NULL;
}

struct SongAndSteps
{
	Song* pSong;
	Steps* pSteps;
	bool operator==( const SongAndSteps& other ) const { return pSong==other.pSong && pSteps==other.pSteps; }
	bool operator<( const SongAndSteps& other ) const { return pSong<=other.pSong && pSteps<=other.pSteps; }
};

void GameState::GetRankingFeats( PlayerNumber pn, vector<RankingFeat> &asFeatsOut ) const
{
	if( !IsHumanPlayer(pn) )
		return;

	Profile *pProf = PROFILEMAN->GetProfile(pn);

	CHECKPOINT_M(ssprintf("PlayMode %i",this->m_PlayMode));
	switch( this->m_PlayMode )
	{
	case PLAY_MODE_REGULAR:
		{
			CHECKPOINT;

			StepsType st = this->GetCurrentStyle()->m_StepsType;

			//
			// Find unique Song and Steps combinations that were played.
			// We must keep only the unique combination or else we'll double-count
			// high score markers.
			//
			vector<SongAndSteps> vSongAndSteps;

			for( unsigned i=0; i<g_vPlayedStageStats.size(); i++ )
			{
				CHECKPOINT_M( ssprintf("%u/%i", i, (int)g_vPlayedStageStats.size() ) );
				SongAndSteps sas;
				sas.pSong = g_vPlayedStageStats[i].vpSongs[0];
				ASSERT( sas.pSong );
				sas.pSteps = g_vPlayedStageStats[i].vpSteps[pn][0];
				ASSERT( sas.pSteps );
				vSongAndSteps.push_back( sas );
			}
			CHECKPOINT;

			sort( vSongAndSteps.begin(), vSongAndSteps.end() );

			vector<SongAndSteps>::iterator toDelete = unique( vSongAndSteps.begin(), vSongAndSteps.end() );
			vSongAndSteps.erase(toDelete, vSongAndSteps.end());

			CHECKPOINT;
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

						if( hs.sName != RANKING_TO_FILL_IN_MARKER[pn] )
							continue;

						RankingFeat feat;
						feat.Type = RankingFeat::SONG;
						feat.pSong = pSong;
						feat.pSteps = pSteps;
						feat.Feat = ssprintf("MR #%d in %s %s", j+1, pSong->GetTranslitMainTitle().c_str(), DifficultyToString(pSteps->GetDifficulty()).c_str() );
						feat.pStringToFill = &hs.sName;
						feat.grade = hs.grade;
						feat.fPercentDP = hs.fPercentDP;
						feat.iScore = hs.iScore;

						if( pSong->HasBanner() )
							feat.Banner = pSong->GetBannerPath();

						asFeatsOut.push_back( feat );
					}
				}
		
				// Find Personal Records
				if( pProf )
				{
					HighScoreList &hsl = pProf->GetStepsHighScoreList(pSong,pSteps);
					for( unsigned j=0; j<hsl.vHighScores.size(); j++ )
					{
						HighScore &hs = hsl.vHighScores[j];

						if( hs.sName != RANKING_TO_FILL_IN_MARKER[pn] )
							continue;

						RankingFeat feat;
						feat.pSong = pSong;
						feat.pSteps = pSteps;
						feat.Type = RankingFeat::SONG;
						feat.Feat = ssprintf("PR #%d in %s %s", j+1, pSong->GetTranslitMainTitle().c_str(), DifficultyToString(pSteps->GetDifficulty()).c_str() );
						feat.pStringToFill = &hs.sName;
						feat.grade = hs.grade;
						feat.fPercentDP = hs.fPercentDP;
						feat.iScore = hs.iScore;

						// XXX: temporary hack
						if( pSong->HasBackground() )
							feat.Banner = pSong->GetBackgroundPath();
		//					if( pSong->HasBanner() )
		//						feat.Banner = pSong->GetBannerPath();

						asFeatsOut.push_back( feat );
					}
				}
			}

			CHECKPOINT;
			StageStats stats;
			GetFinalEvalStats( stats );


			// Find Machine Category Records
			FOREACH_RankingCategory( rc )
			{
				HighScoreList &hsl = PROFILEMAN->GetMachineProfile()->GetCategoryHighScoreList( st, rc );
				for( unsigned j=0; j<hsl.vHighScores.size(); j++ )
				{
					HighScore &hs = hsl.vHighScores[j];
					if( hs.sName != RANKING_TO_FILL_IN_MARKER[pn] )
						continue;

					RankingFeat feat;
					feat.Type = RankingFeat::CATEGORY;
					feat.Feat = ssprintf("MR #%d in Type %c (%d)", j+1, 'A'+rc, stats.GetAverageMeter(pn) );
					feat.pStringToFill = &hs.sName;
					feat.grade = GRADE_NO_DATA;
					feat.iScore = hs.iScore;
					feat.fPercentDP = hs.fPercentDP;
					asFeatsOut.push_back( feat );
				}
			}

			// Find Personal Category Records
			FOREACH_RankingCategory( rc )
			{
				if( pProf )
				{
					HighScoreList &hsl = pProf->GetCategoryHighScoreList( st, rc );
					for( unsigned j=0; j<hsl.vHighScores.size(); j++ )
					{
						HighScore &hs = hsl.vHighScores[j];
						if( hs.sName != RANKING_TO_FILL_IN_MARKER[pn] )
							continue;

						RankingFeat feat;
						feat.Type = RankingFeat::CATEGORY;
						feat.Feat = ssprintf("PR #%d in Type %c (%d)", j+1, 'A'+rc, stats.GetAverageMeter(pn) );
						feat.pStringToFill = &hs.sName;
						feat.grade = GRADE_NO_DATA;
						feat.iScore = hs.iScore;
						feat.fPercentDP = hs.fPercentDP;
						asFeatsOut.push_back( feat );
					}
				}
			}
		}
		break;
	case PLAY_MODE_BATTLE:
	case PLAY_MODE_RAVE:
		break;
	case PLAY_MODE_NONSTOP:
	case PLAY_MODE_ONI:
	case PLAY_MODE_ENDLESS:
		{
			CHECKPOINT;
			Course* pCourse = m_pCurCourse;
			ASSERT( pCourse );
			Trail *pTrail = m_pCurTrail[pn];
			ASSERT( pTrail );
			CourseDifficulty cd = pTrail->m_CourseDifficulty;

			// Find Machine Records
			{
				Profile* pProfile = PROFILEMAN->GetMachineProfile();
				HighScoreList &hsl = pProfile->GetCourseHighScoreList( pCourse, pTrail );
				for( unsigned i=0; i<hsl.vHighScores.size(); i++ )
				{
					HighScore &hs = hsl.vHighScores[i];
					if( hs.sName != RANKING_TO_FILL_IN_MARKER[pn] )
							continue;

					RankingFeat feat;
					feat.Type = RankingFeat::COURSE;
					feat.pCourse = pCourse;
					feat.Feat = ssprintf("MR #%d in %s", i+1, pCourse->GetFullDisplayTitle().c_str() );
					if( cd != DIFFICULTY_MEDIUM )
						feat.Feat += " " + CourseDifficultyToThemedString(cd);
					feat.pStringToFill = &hs.sName;
					feat.grade = GRADE_NO_DATA;
					feat.iScore = hs.iScore;
					feat.fPercentDP = hs.fPercentDP;
					if( pCourse->HasBanner() )
						feat.Banner = pCourse->m_sBannerPath;
					asFeatsOut.push_back( feat );
				}
			}

			// Find Personal Records
			if( PROFILEMAN->IsUsingProfile( pn ) )
			{
				HighScoreList &hsl = pProf->GetCourseHighScoreList( pCourse, pTrail );
				for( unsigned i=0; i<hsl.vHighScores.size(); i++ )
				{
					HighScore& hs = hsl.vHighScores[i];
					if( hs.sName != RANKING_TO_FILL_IN_MARKER[pn] )
							continue;

					RankingFeat feat;
					feat.Type = RankingFeat::COURSE;
					feat.pCourse = pCourse;
					feat.Feat = ssprintf("PR #%d in %s", i+1, pCourse->GetFullDisplayTitle().c_str() );
					feat.pStringToFill = &hs.sName;
					feat.grade = GRADE_NO_DATA;
					feat.iScore = hs.iScore;
					feat.fPercentDP = hs.fPercentDP;
					if( pCourse->HasBanner() )
						feat.Banner = pCourse->m_sBannerPath;
					asFeatsOut.push_back( feat );
				}
			}
		}
		break;
	default:
		ASSERT(0);
	}
}

/*bool GameState::IsNameBlacklisted( CString name )
{

}*/

void GameState::StoreRankingName( PlayerNumber pn, CString name )
{
	//
	// Filter swear words from name
	//
	name.MakeUpper();
	RageFile file(NAMES_BLACKLIST_FILE);
		
	if (file.IsOpen())
	{
		CString line;
		
		while (!file.AtEOF())
		{
			if( file.GetLine( line ) == -1 )
			{
				LOG->Warn( "Error reading \"%s\": %s", NAMES_BLACKLIST_FILE, file.GetError().c_str() );
				break;
			}

			line.MakeUpper();
			if( !line.empty() && name.Find(line) != -1 )	// name contains a bad word
			{
				LOG->Trace( "entered '%s' matches blacklisted item '%s'", name.c_str(), line.c_str() );
				name = "";
				break;
			}
		}
	}

	vector<RankingFeat> aFeats;
	GetRankingFeats( pn, aFeats );

	for( unsigned i=0; i<aFeats.size(); i++ )
	{
		*aFeats[i].pStringToFill = name;

		// save name pointers as we fill them
		m_vpsNamesThatWereFilled.push_back( aFeats[i].pStringToFill );
	}
}

bool GameState::AllAreInDangerOrWorse() const
{
	FOREACH_EnabledPlayer( p )
		if( m_HealthState[p] < DANGER )
			return false;
	return true;
}

bool GameState::AllAreDead() const
{
	FOREACH_EnabledPlayer( p )
		if( m_HealthState[p] < DEAD )
			return false;
	return true;
}

bool GameState::AllHaveComboOf30OrMoreMisses() const
{
	FOREACH_EnabledPlayer( p )
		if( g_CurStageStats.iCurMissCombo[p] < 30 )
			return false;
	return true;
}

bool GameState::OneIsHot() const
{
	FOREACH_EnabledPlayer( p )
		if( m_HealthState[p] == HOT )
			return true;
	return false;
}

bool GameState::IsTimeToPlayAttractSounds()
{
	// if m_iNumTimesThroughAttract is negative, play attract sounds regardless
	// of m_iAttractSoundFrequency.
	if( m_iNumTimesThroughAttract<0 )
		return true;

	// 0 means "never play sound".  Avoid a divide by 0 below.
	if( PREFSMAN->m_iAttractSoundFrequency == 0 )
		return false;

	// play attract sounds once every m_iAttractSoundFrequency times through
	if( (m_iNumTimesThroughAttract % PREFSMAN->m_iAttractSoundFrequency)==0 )
		return true;

	return false;
}

bool GameState::DifficultiesLocked()
{
 	if( GAMESTATE->m_PlayMode == PLAY_MODE_RAVE )
		return true;
	if( IsCourseMode() )
		return PREFSMAN->m_bLockCourseDifficulties;
	return false;
}

bool GameState::ChangePreferredDifficulty( PlayerNumber pn, Difficulty dc )
{
	this->m_PreferredDifficulty[pn] = dc;
	if( DifficultiesLocked() )
		FOREACH_PlayerNumber( p )
			m_PreferredDifficulty[p] = m_PreferredDifficulty[pn];

	return true;
}

void GameState::GetDifficultiesToShow( set<Difficulty> &ret )
{
	static float fExpiration = -999;
	static set<Difficulty> cache;
	if( RageTimer::GetTimeSinceStart() < fExpiration )
	{
		ret = cache;
		return;
	}

	CStringArray asDiff;
	split( DIFFICULTIES_TO_SHOW, ",", asDiff );
	ASSERT( asDiff.size() > 0 );

	cache.clear();
	for( unsigned i = 0; i < asDiff.size(); ++i )
	{
		Difficulty d = StringToDifficulty(asDiff[i]);
		if( d == DIFFICULTY_INVALID )
			RageException::Throw( "Unknown difficulty \"%s\" in CourseDifficultiesToShow", asDiff[i].c_str() );
		cache.insert( d );
	}

	fExpiration = RageTimer::GetTimeSinceStart()+1;
	ret = cache;
}

bool GameState::ChangePreferredDifficulty( PlayerNumber pn, int dir )
{
	set<Difficulty> asDiff;
	GetDifficultiesToShow( asDiff );

	Difficulty d = m_PreferredDifficulty[pn];
	while( 1 )
	{
		d = (Difficulty)(d+dir);
		if( d < 0 || d >= NUM_DIFFICULTIES )
			return false;
		if( asDiff.find(d) == asDiff.end() )
			continue; /* not available */
	}

	return ChangePreferredDifficulty( pn, d );
}

void GameState::GetCourseDifficultiesToShow( set<CourseDifficulty> &ret )
{
	static float fExpiration = -999;
	static set<CourseDifficulty> cache;
	if( RageTimer::GetTimeSinceStart() < fExpiration )
	{
		ret = cache;
		return;
	}

	CStringArray asDiff;
	split( COURSE_DIFFICULTIES_TO_SHOW, ",", asDiff );
	ASSERT( asDiff.size() > 0 );

	cache.clear();
	for( unsigned i = 0; i < asDiff.size(); ++i )
	{
		CourseDifficulty cd = StringToCourseDifficulty(asDiff[i]);
		if( cd == DIFFICULTY_INVALID )
			RageException::Throw( "Unknown difficulty \"%s\" in CourseDifficultiesToShow", asDiff[i].c_str() );
		cache.insert( cd );
	}

	fExpiration = RageTimer::GetTimeSinceStart()+1;
	ret = cache;
}

bool GameState::ChangePreferredCourseDifficulty( PlayerNumber pn, CourseDifficulty cd )
{
	m_PreferredCourseDifficulty[pn] = cd;

	if( PREFSMAN->m_bLockCourseDifficulties )
		FOREACH_PlayerNumber( p )
			m_PreferredCourseDifficulty[p] = m_PreferredCourseDifficulty[pn];

	return true;
}

bool GameState::ChangePreferredCourseDifficulty( PlayerNumber pn, int dir )
{
	/* If we have a course selected, only choose among difficulties available in the course. */
	const Course *pCourse = this->m_pCurCourse;

	set<CourseDifficulty> asDiff;
	GetCourseDifficultiesToShow( asDiff );

	CourseDifficulty cd = m_PreferredCourseDifficulty[pn];
	while( 1 )
	{
		cd = (CourseDifficulty)(cd+dir);
		if( cd < 0 || cd >= NUM_DIFFICULTIES )
			return false;
		if( asDiff.find(cd) == asDiff.end() )
			continue; /* not available */
		if( !pCourse || pCourse->GetTrail( GAMESTATE->GetCurrentStyle()->m_StepsType, cd ) )
			break;
	}

	return ChangePreferredCourseDifficulty( pn, cd );
}

bool GameState::IsCourseDifficultyShown( CourseDifficulty cd )
{
	set<CourseDifficulty> asDiff;
	GetCourseDifficultiesToShow( asDiff );
	return asDiff.find(cd) != asDiff.end();
}

Difficulty GameState::GetEasiestNotesDifficulty() const
{
	Difficulty dc = DIFFICULTY_INVALID;
	FOREACH_HumanPlayer( p )
	{
		if( this->m_pCurSteps[p] == NULL )
		{
			LOG->Warn( "GetEasiestNotesDifficulty called but p%i hasn't chosen notes", p+1 );
			continue;
		}
		dc = min( dc, this->m_pCurSteps[p]->GetDifficulty() );
	}
	return dc;
}

bool PlayerIsUsingModifier( PlayerNumber pn, const CString sModifier )
{
	PlayerOptions po = GAMESTATE->m_PlayerOptions[pn];
	SongOptions so = GAMESTATE->m_SongOptions;
	po.FromString( sModifier );
	so.FromString( sModifier );

	return po == GAMESTATE->m_PlayerOptions[pn] && so == GAMESTATE->m_SongOptions;
}

#include "LuaFunctions.h"
LuaFunction_PlayerNumber( IsPlayerEnabled,	GAMESTATE->IsPlayerEnabled(pn) )
LuaFunction_PlayerNumber( IsHumanPlayer,	GAMESTATE->IsHumanPlayer(pn) )
LuaFunction_PlayerNumber( IsPlayerUsingProfile,	PROFILEMAN->IsUsingProfile(pn) )
LuaFunction_PlayerNumber( IsWinner,			GAMESTATE->GetStageResult(pn)==RESULT_WIN )
LuaFunction_NoArgs( IsCourseMode,			GAMESTATE->IsCourseMode() )
LuaFunction_NoArgs( IsDemonstration,		GAMESTATE->m_bDemonstrationOrJukebox )
LuaFunction_NoArgs( StageIndex,				GAMESTATE->GetStageIndex() )
LuaFunction_NoArgs( NumStagesLeft,			GAMESTATE->GetNumStagesLeft() )
LuaFunction_NoArgs( IsFinalStage,			GAMESTATE->IsFinalStage() )
LuaFunction_NoArgs( IsExtraStage,			GAMESTATE->IsExtraStage() )
LuaFunction_NoArgs( IsExtraStage2,			GAMESTATE->IsExtraStage2() )
LuaFunction_NoArgs( CourseSongIndex,		GAMESTATE->GetCourseSongIndex() )
LuaFunction_NoArgs( PlayModeName,			PlayModeToString(GAMESTATE->m_PlayMode) )
LuaFunction_NoArgs( CurStyleName,			CString( GAMESTATE->m_pCurStyle == NULL ? "none": GAMESTATE->GetCurrentStyle()->m_szName ) )
LuaFunction_NoArgs( GetNumPlayersEnabled,	GAMESTATE->GetNumPlayersEnabled() )
LuaFunction_NoArgs( PlayerUsingBothSides,	GAMESTATE->PlayerUsingBothSides() )
LuaFunction_NoArgs( GetEasiestNotesDifficulty, GAMESTATE->GetEasiestNotesDifficulty() )
LuaFunction_Str(	GetEnv,					GAMESTATE->m_mapEnv[str] )
LuaFunction_StrStr(	SetEnv,					GAMESTATE->m_mapEnv[str1] = str2 )

/* Return an integer into SONGMAN->m_pSongs.  This lets us do input checking, which we
 * can't easily do if we return pointers. */
LuaFunction_NoArgs( CurSong,				GAMESTATE->m_pCurSong )
LuaFunction_PlayerNumber( CurSteps,			GAMESTATE->m_pCurSteps[pn] )

int LuaFunc_UsingModifier( lua_State *L )
{
	REQ_ARGS( "UsingModifier", 2 );
	REQ_ARG_NUMBER_RANGE( "UsingModifier", 1, 1, NUM_PLAYERS );
	REQ_ARG( "UsingModifier", 2, string );

	const PlayerNumber pn = (PlayerNumber) (int(lua_tonumber( L, 1 ))-1);
	const CString modifier = lua_tostring( L, 2 );
	LUA_RETURN( PlayerIsUsingModifier( pn, modifier ) );
}
LuaFunction( UsingModifier );

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
