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
	GAME->GetGameNames( arrayGameNames );
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
	for( int i=0; i<m_OptionLineData[0].iNumOptions; i++ )
	{
		if( 0 == strcmp(GAME->m_sCurrentGame, m_OptionLineData[0].szOptionsText[i]) )
		{
			CString sGameName = m_OptionLineData[0].szOptionsText[i];
			if( sGameName == GAME->m_sCurrentGame )
				m_iSelectedOption[0][SG_GAME] = i;
		}
	}
}

void ScreenSelectGame::ExportOptions()
{
	int iOption = m_iSelectedOption[0][SG_GAME];
	CString sGameName = m_OptionLineData[0].szOptionsText[iOption];
	GAME->SwitchGame( sGameName );
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


