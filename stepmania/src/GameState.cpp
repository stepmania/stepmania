#include "stdafx.h"
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
#include "Song.h"
#include "RageLog.h"
#include "ThemeManager.h"
#include "RageUtil.h"
#include "SongManager.h"


GameState*	GAMESTATE = NULL;	// global and accessable from anywhere in our program


#define STAGE_COLOR_DEMO		THEME->GetMetricC("GameState","StageColorDemo")
#define STAGE_COLOR( i )		THEME->GetMetricC("GameState",ssprintf("StageColor%d",i+1))
#define STAGE_COLOR_FINAL		THEME->GetMetricC("GameState","StageColorFinal")
#define STAGE_COLOR_EXTRA1		THEME->GetMetricC("GameState","StageColorExtra1")
#define STAGE_COLOR_EXTRA2		THEME->GetMetricC("GameState","StageColorExtra2")
#define STAGE_COLOR_NONSTOP		THEME->GetMetricC("GameState","StageColorNonstop")
#define STAGE_COLOR_ONI			THEME->GetMetricC("GameState","StageColorOni")
#define STAGE_COLOR_ENDLESS		THEME->GetMetricC("GameState","StageColorEndless")
#define STAGE_TEXT_DEMO			THEME->GetMetric("GameState","StageTextDemo")
#define STAGE_TEXT_FINAL		THEME->GetMetric("GameState","StageTextFinal")
#define STAGE_TEXT_EXTRA1		THEME->GetMetric("GameState","StageTextExtra1")
#define STAGE_TEXT_EXTRA2		THEME->GetMetric("GameState","StageTextExtra2")


GameState::GameState()
{
	m_CurGame = GAME_DANCE;
	m_iCoins = 0;
	Reset();

	ResetLastRanking();
}

GameState::~GameState()
{
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
	m_sPreferredGroup	= "";
	for( p=0; p<NUM_PLAYERS; p++ )
		m_PreferredDifficulty[p] = DIFFICULTY_INVALID;
	m_SongSortOrder = SORT_GROUP;
	m_PlayMode = PLAY_MODE_INVALID;
	m_bEditing = false;
	m_bDemonstration = false;
	m_bJukeboxUsesModifiers = false;
	m_iCurrentStageIndex = 0;
	m_bAllow2ndExtraStage = true;

	m_pCurSong = NULL;
	for( p=0; p<NUM_PLAYERS; p++ )
		m_pCurNotes[p] = NULL;
	m_pCurCourse = NULL;

	ResetMusicStatistics();

	m_CurStageStats = StageStats();
	m_vPassedStageStats.clear();

	for( p=0; p<NUM_PLAYERS; p++ )
		m_PlayerOptions[p] = PlayerOptions();
	m_SongOptions = SongOptions();
	if( PREFSMAN )
		m_SongOptions.m_FailType = PREFSMAN->m_DefaultFailType;
}

void GameState::ResetLastRanking()
{
	for( int p=0; p<NUM_PLAYERS; p++ )
	{
		m_LastRankingCategory[p] = (RankingCategory)-1;
		m_iLastRankingIndex[p] = -1;
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

void GameState::UpdateSongPosition(float fPositionSeconds)
{
	ASSERT(m_pCurSong);

	m_fMusicSeconds = fPositionSeconds;
	m_pCurSong->GetBeatAndBPSFromElapsedTime( m_fMusicSeconds, m_fSongBeat, m_fCurBPS, m_bFreeze );
}

int GameState::GetStageIndex()
{
	return m_iCurrentStageIndex;
}

int GameState::GetNumStagesLeft()
{
	if(GAMESTATE->IsExtraStage() || GAMESTATE->IsExtraStage2())
		return 1;
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
	if( m_bDemonstration )			return STAGE_TEXT_DEMO;
	else if( IsFinalStage() )		return STAGE_TEXT_FINAL;
	else if( IsExtraStage() )		return STAGE_TEXT_EXTRA1;
	else if( IsExtraStage2() )		return STAGE_TEXT_EXTRA2;


	int iStageNo = m_iCurrentStageIndex+1;

	CString sNumberSuffix;
	if( ( (iStageNo/10) % 10 ) == 1 )	// in the teens (e.g. 19, 213)
	{
		sNumberSuffix = "th";
	}
	else	// not in the teens
	{
		const int iLastDigit = iStageNo%10;
		switch( iLastDigit )
		{
		case 1:	sNumberSuffix = "st";	break;
		case 2:	sNumberSuffix = "nd";	break;
		case 3:	sNumberSuffix = "rd";	break;
		default:sNumberSuffix = "th";	break;
		}
	}
	return ssprintf( "%d%s", iStageNo, sNumberSuffix.GetString() );
}

RageColor GameState::GetStageColor()
{
	if( m_bDemonstration )						return STAGE_COLOR_DEMO;
	else if( m_PlayMode==PLAY_MODE_NONSTOP )	return STAGE_COLOR_NONSTOP;
	else if( m_PlayMode==PLAY_MODE_ONI )		return STAGE_COLOR_ONI;
	else if( m_PlayMode==PLAY_MODE_ENDLESS )	return STAGE_COLOR_ENDLESS;
	else if( IsFinalStage() )					return STAGE_COLOR_FINAL;
	else if( IsExtraStage() )					return STAGE_COLOR_EXTRA1;
	else if( IsExtraStage2() )					return STAGE_COLOR_EXTRA2;
	else										return STAGE_COLOR( min(m_iCurrentStageIndex,4) );
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

bool GameState::IsPlayerEnabled( PlayerNumber pn )
{
	if( m_CurStyle == STYLE_INVALID )	// if no style set (we're in TitleMenu, ConfigInstruments or something)
		return true;				// allow input from both sides

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

			/* XXX: if "choose EX" is enabled, then we should only grant EX2
			 * if the chosen stage was the EX we would have chosen (the hardest
			 * song or extra1.crs).  Also, that song should be highlighted in the
			 * music wheel. */
			if( PREFSMAN->m_bPickExtraStage && GAMESTATE->IsExtraStage() && !GAMESTATE->m_bAllow2ndExtraStage )
				continue;

			if( m_CurStageStats.GetGrade((PlayerNumber)p) >= GRADE_AA )
				return true;
		}
	}
	return false;
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
}
