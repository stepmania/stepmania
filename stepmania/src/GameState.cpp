#include "global.h"
/*
-----------------------------------------------------------------------------
 Class: GameState

 Desc: See Header.

 Copyright (c) 2001-2003 by the person(s) listed below.  All rights reserved.
	Chris Danford
	Chris Gomez
-----------------------------------------------------------------------------
*/

#include "GameState.h"
#include "IniFile.h"
#include "GameManager.h"
#include "PrefsManager.h"
#include "InputMapper.h"
#include "song.h"
#include "RageLog.h"
#include "RageUtil.h"
#include "SongManager.h"
#include "Notes.h"
#include "NoteSkinManager.h"
#include "ModeChoice.h"
#include "NoteFieldPositioning.h"
#include "Character.h"
#include "UnlockSystem.h"
#include "AnnouncerManager.h"


GameState*	GAMESTATE = NULL;	// global and accessable from anywhere in our program


GameState::GameState()
{
	m_CurGame = GAME_DANCE;
	m_iCoins = 0;
	/* Don't reset yet; let the first screen do it, so we can
	 * use PREFSMAN. */
//	Reset();
	m_pPosition = NULL;

	m_pUnlockingSys = new UnlockSystem;
	ResetLastRanking();
	ReloadCharacters();
}

GameState::~GameState()
{
	delete m_pUnlockingSys;
	delete m_pPosition;
}

void GameState::Reset()
{
	int p;

	m_CurStyle = STYLE_INVALID;
	m_bPlayersCanJoin = false;
	for( p=0; p<NUM_PLAYERS; p++ )
		m_bSideIsJoined[p] = false;
//	m_iCoins = 0;	// don't reset coin count!
	m_MasterPlayerNumber = PLAYER_INVALID;
	m_sPreferredGroup	= GROUP_ALL_MUSIC;
	m_bChangedFailMode = false;
	for( p=0; p<NUM_PLAYERS; p++ )
		m_PreferredDifficulty[p] = DIFFICULTY_INVALID;
	m_SongSortOrder = SORT_INVALID;
	m_PlayMode = PLAY_MODE_INVALID;
	m_bEditing = false;
	m_bDemonstrationOrJukebox = false;
	m_bJukeboxUsesModifiers = false;
	m_iCurrentStageIndex = 0;
	m_bAllow2ndExtraStage = true;
	m_bDifficultCourses = false;

	m_pCurSong = NULL;
	for( p=0; p<NUM_PLAYERS; p++ )
		m_pCurNotes[p] = NULL;
	m_pCurCourse = NULL;

	SAFE_DELETE( m_pPosition );
	m_pPosition = new NoteFieldPositioning("Positioning.ini");

	ResetMusicStatistics();
	ResetStageStatistics();

	m_vPassedStageStats.clear();

	for( p=0; p<NUM_PLAYERS; p++ )
	{	
		m_CurrentPlayerOptions[p].Init();
		m_PlayerOptions[p].Init();
		m_StoredPlayerOptions[p].Init();
	}
	m_SongOptions.Init();
	
	for( p=0; p<NUM_PLAYERS; p++ )
		ApplyModifiers( (PlayerNumber)p, PREFSMAN->m_sDefaultModifiers );

	for( p=0; p<NUM_PLAYERS; p++ )
	{
		if( PREFSMAN->m_bShowDancingCharacters && m_pCharacters.size() )
			m_pCurCharacters[p] = m_pCharacters[rand()%m_pCharacters.size()];
		else
			m_pCurCharacters[p] = NULL;
	}

	for( p=0; p<NUM_PLAYERS; p++ )
	{
		m_fSuperMeterGrowthScale[p] = 1;
		m_iCpuSkill[p] = 5;
	}
}

void GameState::Update( float fDelta )
{
	for( int p=0; p<NUM_PLAYERS; p++ )
	{
		m_CurrentPlayerOptions[p].Approach( m_PlayerOptions[p], fDelta );

		m_bActiveAttackEndedThisUpdate[p] = false;

		for( unsigned s=0; s<NUM_INVENTORY_SLOTS; s++ )
		{
			if( m_ActiveAttacks[p][s].fSecsRemaining > 0 )
			{
				m_ActiveAttacks[p][s].fSecsRemaining -= fDelta;
				if( m_ActiveAttacks[p][s].fSecsRemaining <= 0 )
				{
					m_ActiveAttacks[p][s].fSecsRemaining = 0;
					m_ActiveAttacks[p][s].sModifier = "";
					m_bActiveAttackEndedThisUpdate[p] = true;
				}
			}
		}

		if( m_bActiveAttackEndedThisUpdate[p] )
			RebuildPlayerOptionsFromActiveAttacks( (PlayerNumber)p );
	}
}

void GameState::ResetLastRanking()
{
	for( int p=0; p<NUM_PLAYERS; p++ )
	{
		m_RankingCategory[p] = (RankingCategory)-1;
		m_iRankingIndex[p] = -1;
	}
}

void GameState::ReloadCharacters()
{
	unsigned i;

	for( i=0; i<m_pCharacters.size(); i++ )
		delete m_pCharacters[i];
	m_pCharacters.clear();

	for( int p=0; p<NUM_PLAYERS; p++ )
		m_pCurCharacters[p] = NULL;

	CStringArray as;
	GetDirListing( "Characters/*", as, true, true );
	for( i=0; i<as.size(); i++ )
	{
		Character* pChar = new Character;
		if( pChar->Load( as[i] ) )
			m_pCharacters.push_back( pChar );
		else
			delete pChar;
	}
}

const float GameState::MUSIC_SECONDS_INVALID = -5000.0f;

void GameState::ResetMusicStatistics()
{	
	m_fMusicSeconds = MUSIC_SECONDS_INVALID;
	m_fSongBeat = 0;
	m_fCurBPS = 10;
	m_bFreeze = false;
	m_bPastHereWeGo = false;
}

void GameState::ResetStageStatistics()
{
	m_CurStageStats = StageStats();
	RemoveAllActiveAttacks();
	RemoveAllInventory();
	m_fOpponentHealthPercent = 1;
	m_fTugLifePercentP1 = 0.5f;
	for( int p=0; p<NUM_PLAYERS; p++ )
		m_fSuperMeter[p] = 0;
}

void GameState::UpdateSongPosition(float fPositionSeconds)
{
	ASSERT(m_pCurSong);

	m_fMusicSeconds = fPositionSeconds;
	m_pCurSong->GetBeatAndBPSFromElapsedTime( m_fMusicSeconds, m_fSongBeat, m_fCurBPS, m_bFreeze );
//	LOG->Trace( "m_fMusicSeconds = %f, m_fSongBeat = %f, m_fCurBPS = %f, m_bFreeze = %f", m_fMusicSeconds, m_fSongBeat, m_fCurBPS, m_bFreeze );
}

int GameState::GetStageIndex()
{
	return m_iCurrentStageIndex;
}

int GameState::GetNumStagesLeft()
{
	if(GAMESTATE->IsExtraStage() || GAMESTATE->IsExtraStage2())
		return 1;
	if( PREFSMAN->m_bEventMode )
		return 999;
	return PREFSMAN->m_iNumArcadeStages - m_iCurrentStageIndex;
}

bool GameState::IsFinalStage()
{
	if( PREFSMAN->m_bEventMode )
		return false;
	int iPredictedStageForCurSong = 1;
	if( m_pCurSong != NULL )
		iPredictedStageForCurSong = SongManager::GetNumStagesForSong( m_pCurSong );
	
	return m_iCurrentStageIndex + iPredictedStageForCurSong == PREFSMAN->m_iNumArcadeStages;
}

bool GameState::IsExtraStage()
{
	if( PREFSMAN->m_bEventMode )
		return false;
	return m_iCurrentStageIndex == PREFSMAN->m_iNumArcadeStages;
}

bool GameState::IsExtraStage2()
{
	if( PREFSMAN->m_bEventMode )
		return false;
	return m_iCurrentStageIndex == PREFSMAN->m_iNumArcadeStages+1;
}

CString GameState::GetStageText()
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

int GameState::GetCourseSongIndex()
{
	int iSongIndex = 0;
	/* iSongsPlayed includes the current song, so it's 1-based; subtract one. */
	for( int p=0; p<NUM_PLAYERS; p++ )
		if( IsPlayerEnabled(p) )
			iSongIndex = max( iSongIndex, m_CurStageStats.iSongsPlayed[p]-1 );
	return iSongIndex;
}

GameDef* GameState::GetCurrentGameDef()
{
	ASSERT( m_CurGame != GAME_INVALID );	// the game must be set before calling this
	return GAMEMAN->GetGameDefForGame( m_CurGame );
}

const StyleDef* GameState::GetCurrentStyleDef()
{

	ASSERT( m_CurStyle != STYLE_INVALID );	// the style must be set before calling this
	return GAMEMAN->GetStyleDefForStyle( m_CurStyle );
}

bool GameState::IsPlayable( const ModeChoice& mc )
{
	return mc.numSidesJoinedToPlay == GAMESTATE->GetNumSidesJoined(); 
}

bool GameState::IsPlayerEnabled( PlayerNumber pn )
{
	// In rave, all players are present.  Non-human players are CPU controlled.
	if( m_PlayMode == PLAY_MODE_RAVE )
		return true;
	
	// In cpu battle, show cpu player only if there's room on the screen.
	if( m_PlayMode == PLAY_MODE_CPU_BATTLE )
		if( GetCurrentStyleDef()->m_StyleType == StyleDef::ONE_PLAYER_ONE_CREDIT )
			return true;
	
	return IsHumanPlayer( pn );
}

bool GameState::IsHumanPlayer( PlayerNumber pn )
{
	if( m_CurStyle == STYLE_INVALID )	// no style chosen
		if( this->m_bPlayersCanJoin )	
			return m_bSideIsJoined[pn];	// only allow input from sides that have already joined
		else
			return true;	// if we can't join, then we're on a screen like MusicScroll or GameOver

	switch( GetCurrentStyleDef()->m_StyleType )
	{
	case StyleDef::TWO_PLAYERS_TWO_CREDITS:
		return true;
	case StyleDef::ONE_PLAYER_ONE_CREDIT:
	case StyleDef::ONE_PLAYER_TWO_CREDITS:
		return pn == m_MasterPlayerNumber;
	default:
		ASSERT(0);		// invalid style type
		return false;
	}
}

PlayerNumber GameState::GetFirstHumanPlayer()
{
	for( int p=0; p<NUM_PLAYERS; p++ )
		if( IsHumanPlayer(p) )
			return (PlayerNumber)p;
	ASSERT(0);	// there must be at least 1 human player
	return PLAYER_INVALID;
}

bool GameState::IsCpuPlayer( PlayerNumber pn )
{
	return IsPlayerEnabled(pn) && !IsHumanPlayer(pn);
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
	switch( GAMESTATE->m_PlayMode )
	{
	case PLAY_MODE_HUMAN_BATTLE:
	case PLAY_MODE_CPU_BATTLE:
		return true;
	default:
		return false;
	}
}

bool GameState::HasEarnedExtraStage()
{
	if( PREFSMAN->m_bEventMode )
		return false;

	if( GAMESTATE->m_PlayMode != PLAY_MODE_ARCADE )
		return false;

	if( (GAMESTATE->IsFinalStage() || GAMESTATE->IsExtraStage()) )
	{
		for( int p=0; p<NUM_PLAYERS; p++ )
		{
			if( !GAMESTATE->IsPlayerEnabled(p) )
				continue;	// skip

			if( GAMESTATE->m_pCurNotes[p]->GetDifficulty() != DIFFICULTY_HARD && 
				GAMESTATE->m_pCurNotes[p]->GetDifficulty() != DIFFICULTY_CHALLENGE )
				continue; /* not hard enough! */

			/* If "choose EX" is enabled, then we should only grant EX2 if the chosen
			 * stage was the EX we would have chosen (m_bAllow2ndExtraStage is true). */
			if( PREFSMAN->m_bPickExtraStage && GAMESTATE->IsExtraStage() && !GAMESTATE->m_bAllow2ndExtraStage )
				continue;

			if( m_CurStageStats.GetGrade((PlayerNumber)p) >= GRADE_AA )
				return true;
		}
	}
	return false;
}

PlayerNumber GameState::GetBestPlayer()
{
	PlayerNumber winner = PLAYER_1;
	for( int p=PLAYER_1+1; p<NUM_PLAYERS; p++ )
	{
		if( GAMESTATE->m_CurStageStats.iActualDancePoints[p] == GAMESTATE->m_CurStageStats.iActualDancePoints[winner] )
			return PLAYER_INVALID;	// draw
		else if( GAMESTATE->m_CurStageStats.iActualDancePoints[p] > GAMESTATE->m_CurStageStats.iActualDancePoints[winner] )
			winner = (PlayerNumber)p;
	}
	return winner;
}

StageResult GameState::GetStageResult( PlayerNumber pn )
{
	switch( GAMESTATE->m_PlayMode )
	{
	case PLAY_MODE_CPU_BATTLE:
		if( IsHumanPlayer(pn) )
			return (m_fOpponentHealthPercent==0)?RESULT_WIN:RESULT_LOSE;
		else
			return (m_fOpponentHealthPercent==0)?RESULT_LOSE:RESULT_WIN;
	case PLAY_MODE_RAVE:
		switch( pn )
		{
		case PLAYER_1:	return (m_fTugLifePercentP1>=0.5f)?RESULT_WIN:RESULT_LOSE;
		case PLAYER_2:	return (m_fTugLifePercentP1<0.5f)?RESULT_WIN:RESULT_LOSE;
		default:	ASSERT(0); return RESULT_LOSE;
		}
	case PLAY_MODE_HUMAN_BATTLE:
	default:
		return (GetBestPlayer()==pn)?RESULT_WIN:RESULT_LOSE;
	}
}

void GameState::GetFinalEvalStatsAndSongs( StageStats& statsOut, vector<Song*>& vSongsOut )
{
	statsOut = StageStats();

	// Show stats only for the latest 3 normal songs + passed extra stages
	int iNumSongsToThrowAway = max( 0, PREFSMAN->m_iNumArcadeStages-3 );
	for( unsigned i=iNumSongsToThrowAway; i<GAMESTATE->m_vPassedStageStats.size(); i++ )
	{
		statsOut += GAMESTATE->m_vPassedStageStats[i];
		vSongsOut.push_back( GAMESTATE->m_vPassedStageStats[i].pSong );
	}

	if(!vSongsOut.size()) return;

	/* XXX: I have no idea if this is correct--but it's better than overflowing,
	 * anyway. -glenn */
	for( int p=0; p<NUM_PLAYERS; p++ )
	{
		if( !IsPlayerEnabled(p) )
			continue;

		for( int r = 0; r < NUM_RADAR_CATEGORIES; r++)
		{
			statsOut.fRadarPossible[p][r] /= vSongsOut.size();
			statsOut.fRadarActual[p][r] /= vSongsOut.size();
		}
	}
}


void GameState::ApplyModifiers( PlayerNumber pn, CString sModifiers )
{
	m_PlayerOptions[pn].FromString( sModifiers );
	m_SongOptions.FromString( sModifiers );
}

/* Store the player's preferred options.  This is called at the very beginning
 * of gameplay. */
void GameState::StoreSelectedOptions()
{
	for( int p=0; p<NUM_PLAYERS; p++ )
		GAMESTATE->m_StoredPlayerOptions[p] = GAMESTATE->m_PlayerOptions[p];
	m_StoredSongOptions = m_SongOptions;
}

/* Restore the preferred options.  This is called after a song ends, before
 * setting new course options, so options from one song don't carry into the
 * next and we default back to the preferred options.  This is also called
 * at the end of gameplay to restore options. */
void GameState::RestoreSelectedOptions()
{
	for( int p=0; p<NUM_PLAYERS; p++ )
		GAMESTATE->m_PlayerOptions[p] = GAMESTATE->m_StoredPlayerOptions[p];
	m_SongOptions = m_StoredSongOptions;
}


void GameState::LaunchAttack( PlayerNumber target, Attack a )
{
	LOG->Trace( "Launch attack '%s' against P%d", a.sModifier.c_str(), target+1 );

	// search for an open slot
	for( unsigned s=0; s<NUM_INVENTORY_SLOTS; s++ )
		if( m_ActiveAttacks[target][s].fSecsRemaining <= 0 )
		{
			m_ActiveAttacks[target][s] = a;
			GAMESTATE->RebuildPlayerOptionsFromActiveAttacks( target );
			return;
		}
}

void GameState::RemoveAllActiveAttacks()
{
	for( int p=0; p<NUM_PLAYERS; p++ )
	{
		for( unsigned s=0; s<NUM_INVENTORY_SLOTS; s++ )
		{
			m_ActiveAttacks[p][s].fSecsRemaining = 0;
			m_ActiveAttacks[p][s].sModifier = "";
		}
		RebuildPlayerOptionsFromActiveAttacks( (PlayerNumber)p );
	}
}

void GameState::RemoveAllInventory()
{
	for( int p=0; p<NUM_PLAYERS; p++ )
		for( unsigned s=0; s<NUM_INVENTORY_SLOTS; s++ )
		{
			m_Inventory[p][s].fSecsRemaining = 0;
			m_Inventory[p][s].sModifier = "";
		}
}

void GameState::RebuildPlayerOptionsFromActiveAttacks( PlayerNumber pn )
{
	// rebuild player options
	PlayerOptions po = m_StoredPlayerOptions[pn];
	for( int s=0; s<NUM_INVENTORY_SLOTS; s++ )
		po.FromString( m_ActiveAttacks[pn][s].sModifier );
	m_PlayerOptions[pn] = po;
}

int GameState::GetSumOfActiveAttackLevels( PlayerNumber pn )
{
	int iSum = 0;

	for( unsigned s=0; s<NUM_INVENTORY_SLOTS; s++ )
		if( m_ActiveAttacks[pn][s].fSecsRemaining > 0 )
			iSum += m_ActiveAttacks[pn][s].level;

	return iSum;
}

