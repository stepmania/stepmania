#include "stdafx.h"
/*
-----------------------------------------------------------------------------
 Class: ScreenInputOptions

 Desc: See header.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "ScreenInputOptions.h"
#include "RageTextureManager.h"
#include "RageUtil.h"
#include "RageSoundManager.h"
#include "ScreenManager.h"
#include "PrefsManager.h"
#include "GameConstantsAndTypes.h"
#include "StepMania.h"
#include "PrefsManager.h"
#include "RageLog.h"
#include "ThemeManager.h"


enum {
	IO_IGNORE_AXES = 0,
	IO_DEDICATED_MENU_BUTTONS,
	IO_AUTOPLAY,
	IO_DELAYED_ESCAPE,
	IO_OPTIONS_NAVIGATION,
	IO_WHEEL_SPEED,
	NUM_INPUT_OPTIONS_LINES
};

/* Hmm.  Ignore JoyAxes and Back Delayed probably belong in "key/joy config",
 * preferably alongside button configuration. */
OptionRowData g_InputOptionsLines[NUM_INPUT_OPTIONS_LINES] = {
	{ "Ignore\nJoy Axes",	2, {"OFF","ON (for NTPad or DirectPad)"} },
	{ "Menu\nButtons",		2, {"USE GAMEPLAY BUTTONS","ONLY DEDICATED BUTTONS"} },
	{ "AutoPlay",			2, {"OFF","ON"} },
	{ "Back\nDelayed",		2, {"INSTANT","HOLD"} },
	{ "Options\nNavigation",2, {"SM STYLE","ARCADE STYLE"} },
	{ "Wheel\nSpeed",		4, {"SLOW","NORMAL","FAST","REALLY FAST"} },
};

ScreenInputOptions::ScreenInputOptions() :
	ScreenOptions(
		THEME->GetPathTo("BGAnimations","input options"),
		THEME->GetPathTo("Graphics","input options page"),
		THEME->GetPathTo("Graphics","input options top edge")
		)
{
	LOG->Trace( "ScreenInputOptions::ScreenInputOptions()" );

	// fill g_InputOptionsLines with explanation text
	for( int i=0; i<NUM_INPUT_OPTIONS_LINES; i++ )
	{
		CString sLineName = g_InputOptionsLines[i].szTitle;
		sLineName.Replace("\n","");
		sLineName.Replace(" ","");
		strcpy( g_InputOptionsLines[i].szExplanation, THEME->GetMetric("ScreenInputOptions",sLineName) );
	}

	Init( 
		INPUTMODE_BOTH, 
		g_InputOptionsLines, 
		NUM_INPUT_OPTIONS_LINES,
		false );
	m_Menu.StopTimer();

	SOUNDMAN->PlayMusic( THEME->GetPathTo("Sounds","input options music") );
}

void ScreenInputOptions::ImportOptions()
{
	m_iSelectedOption[0][IO_IGNORE_AXES]			= PREFSMAN->m_bIgnoreJoyAxes ? 1:0;
	m_iSelectedOption[0][IO_DEDICATED_MENU_BUTTONS]	= PREFSMAN->m_bOnlyDedicatedMenuButtons ? 1:0;
	m_iSelectedOption[0][IO_AUTOPLAY]				= PREFSMAN->m_bAutoPlay;
	m_iSelectedOption[0][IO_DELAYED_ESCAPE]			= PREFSMAN->m_bDelayedEscape ? 1:0;
	m_iSelectedOption[0][IO_OPTIONS_NAVIGATION]		= PREFSMAN->m_bArcadeOptionsNavigation ? 1:0;
	m_iSelectedOption[0][IO_WHEEL_SPEED] =
		(PREFSMAN->m_iMusicWheelSwitchSpeed <= 5)? 0:
		(PREFSMAN->m_iMusicWheelSwitchSpeed <= 10)? 1:
		(PREFSMAN->m_iMusicWheelSwitchSpeed <= 15)? 2:3;
}

void ScreenInputOptions::ExportOptions()
{
	PREFSMAN->m_bIgnoreJoyAxes			= m_iSelectedOption[0][IO_IGNORE_AXES] == 1;
	PREFSMAN->m_bOnlyDedicatedMenuButtons= m_iSelectedOption[0][IO_DEDICATED_MENU_BUTTONS] == 1;
	PREFSMAN->m_bDelayedEscape			= m_iSelectedOption[0][IO_DELAYED_ESCAPE] == 1;
	PREFSMAN->m_bAutoPlay				= m_iSelectedOption[0][IO_AUTOPLAY] == 1;
	PREFSMAN->m_bArcadeOptionsNavigation= m_iSelectedOption[0][IO_OPTIONS_NAVIGATION] == 1;
	switch(m_iSelectedOption[0][IO_WHEEL_SPEED])
	{
	case 0: PREFSMAN->m_iMusicWheelSwitchSpeed = 5; break;
	case 1: PREFSMAN->m_iMusicWheelSwitchSpeed = 10; break;
	case 2: PREFSMAN->m_iMusicWheelSwitchSpeed = 15; break;
	case 3: PREFSMAN->m_iMusicWheelSwitchSpeed = 25; break;
	default: ASSERT(0);
	}
}

void ScreenInputOptions::GoToPrevState()
{
	SCREENMAN->SetNewScreen( "ScreenOptionsMenu" );
}

void ScreenInputOptions::GoToNextState()
{
	PREFSMAN->SaveGlobalPrefsToDisk();
	GoToPrevState();
}

