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
#include "ThemeManager.h"
#include "RageLog.h"
#include "GameManager.h"
#include "string.h"
#include "AnnouncerManager.h"


enum {
	SG_GAME = 0,
	NUM_SELECT_GAME_LINES
};


OptionLineData g_SelectGameLines[NUM_SELECT_GAME_LINES] = 
{
	"Game",	-1, { "" }
};


ScreenSelectGame::ScreenSelectGame() :
	ScreenOptions(
		THEME->GetPathTo(GRAPHIC_PLAYER_OPTIONS_BACKGROUND),
		THEME->GetPathTo(GRAPHIC_PLAYER_OPTIONS_TOP_EDGE)
		)
{
	LOG->WriteLine( "ScreenSelectGame::ScreenSelectGame()" );


	// populate g_SelectGameLines
	CStringArray arrayGameNames;
	GAMEMAN->GetGameNames( arrayGameNames );
	for( int i=0; i<arrayGameNames.GetSize(); i++ )
		strcpy( g_SelectGameLines[0].szOptionsText[i], arrayGameNames[i] );

	g_SelectGameLines[0].iNumOptions = arrayGameNames.GetSize();


	Init( 
		INPUTMODE_P1_ONLY, 
		g_SelectGameLines, 
		NUM_SELECT_GAME_LINES
		);
}

void ScreenSelectGame::ImportOptions()
{
	m_iSelectedOption[0][SG_GAME] = GAMEMAN->m_CurGame;
}

void ScreenSelectGame::ExportOptions()
{
	// Switch the current style to the frist style of the selected game
	Game game = (Game)m_iSelectedOption[0][SG_GAME];
	switch( game )
	{
	case GAME_DANCE:	
		GAMEMAN->m_CurStyle = STYLE_DANCE_SINGLE;
		GAMEMAN->m_CurGame = GAME_DANCE;
		ANNOUNCER->SwitchAnnouncer( "default" );
		break;
	case GAME_PUMP:		
		GAMEMAN->m_CurStyle = STYLE_PUMP_SINGLE;
		GAMEMAN->m_CurGame = GAME_PUMP;
		ANNOUNCER->SwitchAnnouncer( "default" );
		break;
	case GAME_EZ2:		
		GAMEMAN->m_CurStyle = STYLE_EZ2_SINGLE;	
		GAMEMAN->m_CurGame = GAME_EZ2;
		ANNOUNCER->SwitchAnnouncer( "ez2" );
		break;
	default:	ASSERT(0);	// invalid Game
	}

	// Try to switch themes to a theme with the same name as the current game
	CStringArray asGameNames;
	GAMEMAN->GetGameNames( asGameNames );
	CString sGameName = asGameNames[game];
	THEME->SwitchTheme( sGameName );	
}

void ScreenSelectGame::GoToPrevState()
{
	SCREENMAN->SetNewScreen( new ScreenTitleMenu );
}

void ScreenSelectGame::GoToNextState()
{
	SCREENMAN->SetNewScreen( new ScreenTitleMenu );
}


