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
#define STAGE_COLOR_ONI			THEME->GetMetricC("GameState","StageColorOni")
#define STAGE_TEXT_DEMO			THEME->GetMetric("GameState","StageTextDemo")
#define STAGE_TEXT_FINAL		THEME->GetMetric("GameState","StageTextFinal")
#define STAGE_TEXT_EXTRA1		THEME->GetMetric("GameState","StageTextExtra1")
#define STAGE_TEXT_EXTRA2		THEME->GetMetric("GameState","StageTextExtra2")


GameState::GameState()
{
	m_CurGame = GAME_DANCE;
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
	for( int i=0; i<2; i++ )
		m_bSideIsJoined[i] = false;
	m_MasterPlayerNumber = PLAYER_INVALID;
	m_sPreferredGroup	= "";
	for( p=0; p<NUM_PLAYERS; p++ )
		m_PreferredDifficulty[p] = CLASS_INVALID;
	m_SongSortOrder = SORT_GROUP;
	m_PlayMode = PLAY_MODE_INVALID;
	m_bEditing = false;
	m_bDemonstration = false;
	m_iCurrentStageIndex = 0;


	m_pCurSong = NULL;
	for( p=0; p<NUM_PLAYERS; p++ )
		m_pCurNotes[p] = NULL;
	m_pCurCourse = NULL;


	m_fMusicSeconds = 0;
	m_fSongBeat = 0;
	m_fCurBPS = 0;
	m_bFreeze = 0;
	

	m_apSongsPlayed.clear();
	
	m_iSongsIntoCourse = 0;
	for( p=0; p<NUM_PLAYERS; p++ )
	{
		m_fSecondsBeforeFail[p] = -1;
		m_iSongsBeforeFail[p] = 0;
	}


	for( p=0; p<NUM_PLAYERS; p++ )
	{
		int s;

		m_iPossibleDancePoints[p] = 0;
		m_iActualDancePoints[p] = 0;
		for( s=0; s<NUM_TAP_NOTE_SCORES; s++ )
			m_TapNoteScores[p][s] = 0;
		for( s=0; s<NUM_HOLD_NOTE_SCORES; s++ )
			m_HoldNoteScores[p][s] = 0;
		m_iMaxCombo[p] = 0;
		m_fScore[p] = 0;

		for( int r=0; r<NUM_RADAR_VALUES; r++ )
		{
			m_fRadarPossible[p][r] = 0;
			m_fRadarActual[p][r] = 0;
		}
	}

	for( p=0; p<NUM_PLAYERS; p++ )
	{
		int s;

		m_iAccumPossibleDancePoints[p] = 0;
		m_iAccumActualDancePoints[p] = 0;
		for( s=0; s<NUM_TAP_NOTE_SCORES; s++ )
			m_AccumTapNoteScores[p][s] = 0;
		for( s=0; s<NUM_HOLD_NOTE_SCORES; s++ )
			m_AccumHoldNoteScores[p][s] = 0;
		m_iAccumMaxCombo[p] = 0;
		m_fAccumScore[p] = 0;
		for( int r=0; r<NUM_RADAR_VALUES; r++ )
		{
			m_fAccumRadarPossible[p][r] = 0;
			m_fAccumRadarActual[p][r] = 0;
		}
	}


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
}

void GameState::AccumulateStageStatistics()
{
	for( int p=0; p<NUM_PLAYERS; p++ )
	{
		int s;

		m_iAccumPossibleDancePoints[p] += m_iPossibleDancePoints[p];
		m_iAccumActualDancePoints[p] += m_iActualDancePoints[p];
		for( s=0; s<NUM_TAP_NOTE_SCORES; s++ )
			m_AccumTapNoteScores[p][s] += m_TapNoteScores[p][s];
		for( s=0; s<NUM_HOLD_NOTE_SCORES; s++ )
			m_AccumHoldNoteScores[p][s] += m_HoldNoteScores[p][s];
		m_iAccumMaxCombo[p] += m_iMaxCombo[p];
		m_fAccumScore[p] += m_fScore[p];
		for( int r=0; r<NUM_RADAR_VALUES; r++ )
		{
			m_fAccumRadarPossible[p][r] = m_fRadarPossible[p][r];
			m_fAccumRadarActual[p][r] = m_fRadarActual[p][r];
		}
	}
}

void GameState::ResetStageStatistics()
{
	m_iSongsIntoCourse = 0;

	switch( m_PlayMode )
	{
	case PLAY_MODE_ONI:
	case PLAY_MODE_ENDLESS:
		m_apSongsPlayed.clear();
		break;
	}

	for( int p=0; p<NUM_PLAYERS; p++ )
	{
		int s;

		m_iSongsBeforeFail[p] = 0;
		m_fSecondsBeforeFail[p] = -1;

		m_iPossibleDancePoints[p] = 0;
		m_iActualDancePoints[p] = 0;
		for( s=0; s<NUM_TAP_NOTE_SCORES; s++ )
			m_TapNoteScores[p][s] = 0;
		for( s=0; s<NUM_HOLD_NOTE_SCORES; s++ )
			m_HoldNoteScores[p][s] = 0;
		m_iMaxCombo[p] = 0;
		m_fScore[p] = 0;
		for( int r=0; r<NUM_RADAR_VALUES; r++ )
		{
			m_fRadarPossible[p][r] = 0;
			m_fRadarActual[p][r] = 0;
		}
	}
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
	if( m_bDemonstration )		return STAGE_COLOR_DEMO;
	else if( m_PlayMode==PLAY_MODE_ONI || m_PlayMode==PLAY_MODE_ENDLESS )	return STAGE_COLOR_ONI;
	else if( IsFinalStage() )	return STAGE_COLOR_FINAL;
	else if( IsExtraStage() )	return STAGE_COLOR_EXTRA1;
	else if( IsExtraStage2() )	return STAGE_COLOR_EXTRA2;
	else						return STAGE_COLOR( min(m_iCurrentStageIndex,4) );
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

float GameState::GetElapsedSeconds()
{
	switch( m_PlayMode )
	{
	case PLAY_MODE_ARCADE:
		return max( 0, m_fMusicSeconds );
	case PLAY_MODE_ONI:
	case PLAY_MODE_ENDLESS:
		{
			float fSecondsTotal = 0;
			for( unsigned i=0; i<m_apSongsPlayed.size(); i++ )
				fSecondsTotal += m_apSongsPlayed[i]->m_fMusicLengthSeconds;
			fSecondsTotal += max( 0, m_fMusicSeconds );
			return fSecondsTotal;
		}
	default:
		ASSERT(0);
		return 0;
	}
}

float GameState::GetPlayerSurviveTime( PlayerNumber pn )
{
	if( m_fSecondsBeforeFail[pn] == -1 )	// haven't failed yet
		return GetElapsedSeconds();
	else
		return m_fSecondsBeforeFail[pn];
}

Grade GameState::GetCurrentGrade( PlayerNumber pn )
{
	if( !GAMESTATE->IsPlayerEnabled(pn)  ||  GAMESTATE->m_fSecondsBeforeFail[pn] != -1 )
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
	float fPercentDancePoints = m_iActualDancePoints[pn] / (float)m_iPossibleDancePoints[pn];
	fPercentDancePoints = max( fPercentDancePoints, 0 );
	LOG->Trace( "iActualDancePoints: %i", m_iActualDancePoints[pn] );
	LOG->Trace( "iPossibleDancePoints: %i", m_iPossibleDancePoints[pn] );
	LOG->Trace( "fPercentDancePoints: %f", fPercentDancePoints  );

	if     ( fPercentDancePoints >= 1.00 )	return GRADE_AAA;
	else if( fPercentDancePoints >= 0.93 )	return GRADE_AA;
	else if( fPercentDancePoints >= 0.80 )	return GRADE_A;
	else if( fPercentDancePoints >= 0.65 )	return GRADE_B;
	else if( fPercentDancePoints >= 0.45 )	return GRADE_C;
	else									return GRADE_D;
}
