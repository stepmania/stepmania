#include "global.h"
/*
-----------------------------------------------------------------------------
 File: ScreenSelectGame.cpp

 Desc: Select a song.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
-----------------------------------------------------------------------------
*/

#include "ScreenSelectGame.h"
#include "RageSoundManager.h"
#include "ScreenManager.h"
#include "PrefsManager.h"
#include "RageLog.h"
#include "GameManager.h"
#include "GameState.h"
#include "InputMapper.h"
#include "ThemeManager.h"
#include "StepMania.h"


enum {
	SG_GAME = 0,
	NUM_SELECT_GAME_LINES
};


OptionRow g_SelectGameLines[NUM_SELECT_GAME_LINES] = 
{
	OptionRow(	"Game"	),	
};


ScreenSelectGame::ScreenSelectGame() :
	ScreenOptions("ScreenSelectGame",false)
{
	LOG->Trace( "ScreenSelectGame::ScreenSelectGame()" );

	/* populate g_SelectGameLines */
	vector<Game> aGames;
	GAMEMAN->GetEnabledGames( aGames );
	
	m_OptionRow[SG_GAME].choices.clear();
	for( unsigned i=0; i<aGames.size(); i++ )
	{
		Game game = aGames[i];
		CString sGameName = GAMEMAN->GetGameDefForGame(game)->m_szName;
		sGameName.MakeUpper();
		m_OptionRow[SG_GAME].choices.push_back( sGameName );
	}


	Init( 
		INPUTMODE_BOTH, 
		g_SelectGameLines, 
		NUM_SELECT_GAME_LINES,
		false, true );
	m_Menu.m_MenuTimer.Disable();

	SOUNDMAN->PlayMusic( THEME->GetPathTo("Sounds","ScreenSelectGame music") );
}

void ScreenSelectGame::ImportOptions()
{
	/* Search the list of games for the currently active game.  If it's
	 * not there, we might have set a game and then the user removed its
	 * note skins; reset it to the first available. */
	vector<Game> aGames;
	GAMEMAN->GetEnabledGames( aGames );
	ASSERT(!aGames.empty());

	m_iSelectedOption[0][SG_GAME] = 0;
	for(unsigned sel = 0; sel < aGames.size(); ++sel)
		if(aGames[sel] == GAMESTATE->m_CurGame) m_iSelectedOption[0][SG_GAME] = sel;
}

void ScreenSelectGame::ExportOptions()
{
	LOG->Trace("ScreenSelectGame::ExportOptions()");

	INPUTMAPPER->SaveMappingsToDisk();	// save mappings before switching the game
	PREFSMAN->SaveGamePrefsToDisk();

	// Switch the current style to the frist style of the selected game
	int iSelection = m_iSelectedOption[0][SG_GAME];

	vector<Game> aGames;
	GAMEMAN->GetEnabledGames( aGames );
	Game game = aGames[iSelection];

	GAMESTATE->m_CurGame = game;
	PREFSMAN->ReadGamePrefsFromDisk();
	INPUTMAPPER->ReadMappingsFromDisk();
}

void ScreenSelectGame::GoToPrevState()
{
	ResetGame();
}

void ScreenSelectGame::GoToNextState()
{
	ResetGame();
}


