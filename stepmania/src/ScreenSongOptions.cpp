#include "global.h"
/*
-----------------------------------------------------------------------------
 Class: ScreenSongOptions

 Desc: See header.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "ScreenSongOptions.h"
#include "RageUtil.h"
#include "ScreenManager.h"
#include "GameConstantsAndTypes.h"
#include "RageLog.h"
#include "GameState.h"
#include "ThemeManager.h"
#include "PrefsManager.h"

#define PREV_SCREEN( play_mode )		THEME->GetMetric ("ScreenSongOptions","PrevScreen"+Capitalize(PlayModeToString(play_mode)))
#define NEXT_SCREEN( play_mode )		THEME->GetMetric ("ScreenSongOptions","NextScreen"+Capitalize(PlayModeToString(play_mode)))

enum {
	SO_LIFE = 0,
	SO_DRAIN,
	SO_BAT_LIVES,
	SO_FAIL,
	SO_ASSIST,
	SO_RATE,
	SO_AUTOSYNC,
	NUM_SONG_OPTIONS_LINES
};

OptionRow g_SongOptionsLines[NUM_SONG_OPTIONS_LINES] = {
	OptionRow( "Life\nType",	"BAR","BATTERY" ),	
	OptionRow( "Bar\nDrain",	"NORMAL","NO RECOVER","SUDDEN DEATH" ),	
	OptionRow( "Bat\nLives",	"1","2","3","4","5","6","7","8","9","10" ),	
	OptionRow( "Fail",			"ARCADE","END OF SONG","OFF" ),	
	OptionRow( "Assist\nTick",	"OFF", "ON" ),
	OptionRow( "Rate",			"0.3x","0.4x","0.5x","0.6x","0.7x","0.8x","0.9x","1.0x","1.1x","1.2x","1.3x","1.4x","1.5x","1.6x","1.7x","1.8x","1.9x","2.0x" ),	
	OptionRow( "Auto\nAdjust",	"OFF", "ON" ),	
};

/* Get the next screen we'll go to when finished. */
CString ScreenSongOptions::GetNextScreen()
{
	return NEXT_SCREEN(GAMESTATE->m_PlayMode);
}


ScreenSongOptions::ScreenSongOptions() :
	ScreenOptions("ScreenSongOptions",true)
{
	LOG->Trace( "ScreenSongOptions::ScreenSongOptions()" );

	Init( INPUTMODE_BOTH, 
		g_SongOptionsLines, 
		NUM_SONG_OPTIONS_LINES,
		false, false );

	/* If we're coming in from "press start for more options", we need a different
	 * fade in. XXX: this is a hack */
	if(PREFSMAN->m_ShowSongOptions == PrefsManager::ASK)
	{
		m_Menu.m_In.Load( THEME->GetPathToB("ScreenSongOptions option in") );
		m_Menu.m_In.StartTransitioning();
	}
}

void ScreenSongOptions::ImportOptions()
{
	SongOptions &so = GAMESTATE->m_SongOptions;

	m_iSelectedOption[0][SO_LIFE] = so.m_LifeType;
	m_iSelectedOption[0][SO_BAT_LIVES] = so.m_iBatteryLives-1;
	m_iSelectedOption[0][SO_FAIL] = so.m_FailType;
	m_iSelectedOption[0][SO_ASSIST] = so.m_bAssistTick;
	m_iSelectedOption[0][SO_AUTOSYNC] = so.m_bAutoSync;

	m_iSelectedOption[0][SO_RATE] = 7;	// in case we don't match below
	for( int i=0; i<g_SongOptionsLines[SO_RATE].choices.size(); i++ )
		if( so.m_fMusicRate == atof(g_SongOptionsLines[SO_RATE].choices[i]) ) 
			m_iSelectedOption[0][SO_RATE] = i;
}

void ScreenSongOptions::ExportOptions()
{
	SongOptions &so = GAMESTATE->m_SongOptions;

	so.m_LifeType = (SongOptions::LifeType)m_iSelectedOption[0][SO_LIFE];
	so.m_DrainType = (SongOptions::DrainType)m_iSelectedOption[0][SO_DRAIN];
	so.m_iBatteryLives = m_iSelectedOption[0][SO_BAT_LIVES]+1;
	if( so.m_FailType !=	(SongOptions::FailType)m_iSelectedOption[0][SO_FAIL] )
	{
		/* The user is changing the fail mode explicitly; stop messing with it. */
		GAMESTATE->m_bChangedFailType = true;
		so.m_FailType =	(SongOptions::FailType)m_iSelectedOption[0][SO_FAIL];
	}
	so.m_bAssistTick = !!m_iSelectedOption[0][SO_ASSIST];
	so.m_bAutoSync = !!m_iSelectedOption[0][SO_AUTOSYNC];

	int iSel = m_iSelectedOption[0][SO_RATE];
	so.m_fMusicRate = atof( g_SongOptionsLines[SO_RATE].choices[iSel] );
}

void ScreenSongOptions::GoToPrevState()
{
	if( GAMESTATE->m_bEditing )
		SCREENMAN->PopTopScreen( SM_None );
	else
		SCREENMAN->SetNewScreen( PREV_SCREEN(GAMESTATE->m_PlayMode) );
}

void ScreenSongOptions::GoToNextState()
{
	if( GAMESTATE->m_bEditing )
		SCREENMAN->PopTopScreen();
	else
		SCREENMAN->SetNewScreen( NEXT_SCREEN(GAMESTATE->m_PlayMode) );
}




