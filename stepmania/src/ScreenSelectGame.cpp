#include "stdafx.h"
/*
-----------------------------------------------------------------------------
 File: ScreenSelectGame.cpp

 Desc: Select a song.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
-----------------------------------------------------------------------------
*/

#include "ScreenSelectGame.h"
#include <assert.h>
#include "RageTextureManager.h"
#include "RageUtil.h"
#include "RageMusic.h"
#include "ScreenManager.h"
#include "PrefsManager.h"
#include "ScreenOptions.h"
#include "ScreenTitleMenu.h"
#include "GameConstantsAndTypes.h"
#include "StepMania.h"
#include "PrefsManager.h"
#include "RageLog.h"
#include "GameManager.h"
#include <string.h>
#include "GameState.h"
#include "InputMapper.h"


enum {
	SG_GAME = 0,
	NUM_SELECT_GAME_LINES
};


OptionLineData g_SelectGameLines[NUM_SELECT_GAME_LINES] = 
{
	"Game",	0, { "" }
};


ScreenSelectGame::ScreenSelectGame() :
	ScreenOptions(
		THEME->GetPathTo(GRAPHIC_PLAYER_OPTIONS_BACKGROUND),
		THEME->GetPathTo(GRAPHIC_PLAYER_OPTIONS_TOP_EDGE)
		)
{
	LOG->Trace( "ScreenSelectGame::ScreenSelectGame()" );


	// populate g_SelectGameLines
	CStringArray arrayGameNames;
	GAMEMAN->GetGameNames( arrayGameNames );
	for( int i=0; i<arrayGameNames.GetSize(); i++ )
		strcpy( g_SelectGameLines[0].szOptionsText[i], arrayGameNames[i] );

	g_SelectGameLines[0].iNumOptions = arrayGameNames.GetSize();


	// HACK:  Chris: Temporarily disable EZ2Dancer.  The EZ2 menus aren't working correctly, 
	// and I have to leave for California in 4 hours - not enough time to fix this and do a 
	// release.
	g_SelectGameLines[0].iNumOptions = 2;


	Init( 
		INPUTMODE_P1_ONLY, 
		g_SelectGameLines, 
		NUM_SELECT_GAME_LINES
		);
}

void ScreenSelectGame::ImportOptions()
{
	m_iSelectedOption[0][SG_GAME] = GAMESTATE->m_CurGame;
}

void ScreenSelectGame::ExportOptions()
{
	LOG->Trace("ScreenSelectGame::ExportOptions()");

	INPUTMAPPER->SaveMappingsToDisk();	// save mappings before switching the game
	PREFSMAN->SaveGamePrefsToDisk();

	// Switch the current style to the frist style of the selected game
	Game game = (Game)m_iSelectedOption[0][SG_GAME];
	GAMESTATE->m_CurGame = game;
	PREFSMAN->ReadGamePrefsFromDisk();
	INPUTMAPPER->ReadMappingsFromDisk();
}

void ScreenSelectGame::GoToPrevState()
{
	SCREENMAN->SetNewScreen( new ScreenTitleMenu );
}

void ScreenSelectGame::GoToNextState()
{
	SCREENMAN->SetNewScreen( new ScreenTitleMenu );
}


