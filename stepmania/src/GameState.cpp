#include "stdafx.h"
/*
-----------------------------------------------------------------------------
 Class: GameState

 Desc: See Header.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
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
}

GameState::~GameState()
{
}

void GameState::Reset()
{
	int p;

	m_CurStyle = STYLE_NONE;
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
	m_iCurrentStageIndex = 0;

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
}

void GameState::ResetMusicStatistics()
{	
	m_fMusicSeconds = 0;
	m_fSongBeat = 0;
	m_fCurBPS = 10;
	m_bFreeze = false;
	m_bPastHereWeGo = false;
}

int GameState::GetStageIndex()
{
	return m_iCurrentStageIndex;
}

bool GameState::IsFinalStage()
{
	if( PREFSMAN->m_bEventMode )
		return false;
	return m_iCurrentStageIndex == PREFSMAN->m_iNumArcadeStages-1;
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
	ASSERT( m_CurStyle != STYLE_NONE );	// the style must be set before calling this
	return GAMEMAN->GetStyleDefForStyle( m_CurStyle );
}

bool GameState::IsPlayerEnabled( PlayerNumber pn )
{
	if( m_CurStyle == STYLE_NONE )	// if no style set (we're in TitleMenu, ConfigInstruments or something)
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

Grade GameState::GetCurrentGrade( PlayerNumber pn )
{
	ASSERT( GAMESTATE->IsPlayerEnabled(pn) );	// should be calling this is player isn't joined!

	if( m_CurStageStats.bFailed[pn] )
		return GRADE_E;

	/* Based on the percentage of your total "Dance Points" to the maximum
	 * possible number, the following rank is assigned: 
	 *
	 * 100% - AAA
	 *  93% - AA
	 *  80% - A
	 *  65% - B
	 *  45% - C
	 * Less - D
	 * Fail - E
	 */
	float fPercentDancePoints = m_CurStageStats.iActualDancePoints[pn] / (float)m_CurStageStats.iPossibleDancePoints[pn];
	fPercentDancePoints = max( 0, fPercentDancePoints );
	LOG->Trace( "iActualDancePoints: %i", m_CurStageStats.iActualDancePoints[pn] );
	LOG->Trace( "iPossibleDancePoints: %i", m_CurStageStats.iPossibleDancePoints[pn] );
	LOG->Trace( "fPercentDancePoints: %f", fPercentDancePoints  );

	// check for "AAAA"
	if( m_CurStageStats.iTapNoteScores[pn][TNS_MARVELOUS] > 0 &&
		m_CurStageStats.iTapNoteScores[pn][TNS_PERFECT] == 0 &&
		m_CurStageStats.iTapNoteScores[pn][TNS_GREAT] == 0 &&
		m_CurStageStats.iTapNoteScores[pn][TNS_GOOD] == 0 &&
		m_CurStageStats.iTapNoteScores[pn][TNS_BOO] == 0 &&
		m_CurStageStats.iTapNoteScores[pn][TNS_MISS] == 0 )
		return GRADE_AAAA;

	if     ( fPercentDancePoints == 1.00 )		return GRADE_AAA;
	else if( fPercentDancePoints >= 0.93 )		return GRADE_AA;
	else if( fPercentDancePoints >= 0.80 )		return GRADE_A;
	else if( fPercentDancePoints >= 0.65 )		return GRADE_B;
	else if( fPercentDancePoints >= 0.45 )		return GRADE_C;
	else										return GRADE_D;
}

bool GameState::HasEarnedExtraStage()
{
	if( (GAMESTATE->IsFinalStage() || GAMESTATE->IsExtraStage()) )
	{
		for( int p=0; p<NUM_PLAYERS; p++ )
		{
			if( !GAMESTATE->IsPlayerEnabled(p) )
				continue;	// skip

			if( GAMESTATE->m_pCurNotes[p]->GetDifficulty()==DIFFICULTY_HARD && GetCurrentGrade((PlayerNumber)p)==GRADE_AA )
				return true;
		}
	}
	return false;
}