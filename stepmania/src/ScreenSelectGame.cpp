#include "stdafx.h"
/*
-----------------------------------------------------------------------------
 File: ScreenSelectGame.cpp

 Desc: Select a song.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
-----------------------------------------------------------------------------
*/

#include "ScreenSelectGame.h"
#include "RageMusic.h"
#include "ScreenManager.h"
#include "PrefsManager.h"
#include "RageLog.h"
#include "GameManager.h"
#include "GameState.h"
#include "InputMapper.h"


enum {
	SG_GAME = 0,
	NUM_SELECT_GAME_LINES
};


OptionRowData g_SelectGameLines[NUM_SELECT_GAME_LINES] = 
{
	"Game",	0, { "" }
};


ScreenSelectGame::ScreenSelectGame() :
	ScreenOptions(
		THEME->GetPathTo("BGAnimations","select game"),
		THEME->GetPathTo("Graphics","select game page"),
		THEME->GetPathTo("Graphics","select game top edge")
		)
{
	LOG->Trace( "ScreenSelectGame::ScreenSelectGame()" );

	/* populate g_SelectGameLines */
	CArray<Game,Game> aGames;
	GAMEMAN->GetEnabledGames( aGames );
	for( int i=0; i<aGames.GetSize(); i++ )
	{
		Game game = aGames[i];
		CString sGameName = GAMEMAN->GetGameDefForGame(game)->m_szName;
		strcpy( g_SelectGameLines[0].szOptionsText[i], sGameName );
	}
	g_SelectGameLines[0].iNumOptions = i;


	Init( 
		INPUTMODE_BOTH, 
		g_SelectGameLines, 
		NUM_SELECT_GAME_LINES,
		false );
	m_Menu.SetTimer( 99 );
	m_Menu.StopTimer();

	MUSIC->LoadAndPlayIfNotAlready( THEME->GetPathTo("Sounds","select game music") );
}

void ScreenSelectGame::ImportOptions()
{
	/* Search the list of games for the currently active game.  If it's
	 * not there, we might have set a game and then the user removed its
	 * note skins; reset it to the first available. */
	CArray<Game,Game> aGames;
	GAMEMAN->GetEnabledGames( aGames );
	ASSERT(aGames.GetSize());

	m_iSelectedOption[0][SG_GAME] = 0;
	for(int sel = 0; sel < aGames.GetSize(); ++sel)
		if(aGames[sel] == GAMESTATE->m_CurGame) m_iSelectedOption[0][SG_GAME] = sel;
}

void ScreenSelectGame::ExportOptions()
{
	LOG->Trace("ScreenSelectGame::ExportOptions()");

	INPUTMAPPER->SaveMappingsToDisk();	// save mappings before switching the game
	PREFSMAN->SaveGamePrefsToDisk();

	// Switch the current style to the frist style of the selected game
	int iSelection = m_iSelectedOption[0][SG_GAME];

	CArray<Game,Game> aGames;
	GAMEMAN->GetEnabledGames( aGames );
	Game game = aGames[iSelection];

	GAMESTATE->m_CurGame = game;
	PREFSMAN->ReadGamePrefsFromDisk();
	INPUTMAPPER->ReadMappingsFromDisk();
}

void ScreenSelectGame::GoToPrevState()
{
	SCREENMAN->SetNewScreen( "ScreenTitleMenu" );
}

void ScreenSelectGame::GoToNextState()
{
	SCREENMAN->SetNewScreen( "ScreenTitleMenu" );
}


