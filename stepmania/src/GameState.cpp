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


GameState*	GAMESTATE = NULL;	// global and accessable from anywhere in our program


GameState::GameState()
{
	m_sLoadingMessage = "Initializing hardware...";
	m_CurGame = GAME_DANCE;
	Reset();
}

GameState::~GameState()
{
}

void GameState::Reset()
{
	int p;

	m_pCurSong = NULL;
	for( p=0; p<NUM_PLAYERS; p++ )
		m_pCurNotes[p] = NULL;
	m_pCurCourse = NULL;
	m_sPreferredGroup = "";

	m_aGameplayStatistics.RemoveAll();

	// We can simply reset it cause it would override the selection the user just did
	// Moved to the constructor for now
	//m_CurGame = GAME_DANCE;

	m_CurStyle = STYLE_NONE;
	m_MasterPlayerNumber = PLAYER_INVALID;


	for( p=0; p<NUM_PLAYERS; p++ )
		m_PreferredDifficultyClass[p] = CLASS_EASY;
	m_SongSortOrder = SORT_GROUP;
	m_PlayMode = PLAY_MODE_INVALID;
	m_iCurrentStageIndex = 0;
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
	if( IsFinalStage() )
		return "Final";
	else if( IsExtraStage() )
		return "Extra";
	else if( IsExtraStage2() )
		return "Extra 2";


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
	return ssprintf( "%d%s", iStageNo, sNumberSuffix );
}

D3DXCOLOR GameState::GetStageColor()
{
	if( IsFinalStage() )
		return D3DXCOLOR(1,0.1f,0.1f,1);	// red
	else if( IsExtraStage() || IsExtraStage2() )
		return D3DXCOLOR(1,1,0.3f,1);		// yellow
	else
		return D3DXCOLOR(0.3f,1,0.3f,1);	// green
}

GameplayStatistics& GameState::GetLatestGameplayStatistics()
{
	ASSERT( m_aGameplayStatistics.GetSize() > 0 );
	return m_aGameplayStatistics[ m_aGameplayStatistics.GetSize()-1 ];
}

GameDef* GameState::GetCurrentGameDef()
{
	return GAMEMAN->GetGameDefForGame( m_CurGame );
}

StyleDef* GameState::GetCurrentStyleDef()
{
	return GAMEMAN->GetStyleDefForStyle( m_CurStyle );
}

bool GameState::IsPlayerEnabled( PlayerNumber pn )
{
	if( m_CurStyle == STYLE_NONE )	// if no style set (we're in TitleMenu, ConfigInstruments or something)
		return true;				// allow input from both sides

	return ( pn == m_MasterPlayerNumber ) ||  
		( GetCurrentStyleDef()->m_StyleType == StyleDef::TWO_PLAYERS_USE_TWO_SIDES );
};
