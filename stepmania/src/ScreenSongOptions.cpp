#include "global.h"
/*
-----------------------------------------------------------------------------
 Class: ScreenSongOptions

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "ScreenSongOptions.h"
#include "ScreenManager.h"
#include "RageLog.h"
#include "GameState.h"
#include "ThemeManager.h"
#include "PrefsManager.h"

#define PREV_SCREEN		THEME->GetMetric ("ScreenSongOptions","PrevScreen")
#define NEXT_SCREEN		THEME->GetMetric ("ScreenSongOptions","NextScreen")

/* Get the next screen we'll go to when finished. */
CString ScreenSongOptions::GetNextScreen()
{
	return NEXT_SCREEN;
}


ScreenSongOptions::ScreenSongOptions( CString sClassName ) :
	ScreenOptionsMaster( sClassName )
{
	/* Hack: If we're coming in from "press start for more options", we need a different
	 * fade in. */
	if(PREFSMAN->m_ShowSongOptions == PrefsManager::ASK)
	{
		m_In.Load( THEME->GetPathToB("ScreenSongOptions option in") );
		m_In.StartTransitioning();
	}
}

void ScreenSongOptions::GoToPrevState()
{
	if( GAMESTATE->m_bEditing )
		SCREENMAN->PopTopScreen( SM_None );
	else
		SCREENMAN->SetNewScreen( PREV_SCREEN );
}

void ScreenSongOptions::GoToNextState()
{
	if( GAMESTATE->m_bEditing )
		SCREENMAN->PopTopScreen();
	else
		SCREENMAN->SetNewScreen( NEXT_SCREEN );
}
