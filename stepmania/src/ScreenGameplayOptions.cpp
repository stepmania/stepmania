#include "stdafx.h"
/*
-----------------------------------------------------------------------------
 Class: ScreenGameplayOptions

 Desc: See header.

 Copyright (c) 2001-2003 by the person(s) listed below.  All rights reserved.
	Chris Danford
	Chris Gomez
-----------------------------------------------------------------------------
*/

#include "ScreenGameplayOptions.h"
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
	GO_BGMODE,
	GO_BGBRIGHTNESS,
	GO_BGIFNOBANNER,
	GO_SHOW_DANGER,
	GO_HIDDEN_SONGS,
	GO_EASTER_EGGS,
	GO_MARVELOUS,
	GO_PICK_EXTRA_STAGE,
	NUM_GAMEPLAY_OPTIONS_LINES
};

OptionRowData g_GameplayOptionsLines[NUM_GAMEPLAY_OPTIONS_LINES] = {
	{ "Background\nMode",		4,  {"OFF","ANIMATIONS","VISUALIZATIONS","RANDOM MOVIES"} },
	{ "Background\nBrightness",	11, {"0%","10%","20%","30%","40%","50%","60%","70%","80%","90%","100%"} },
	{ "BG For\nBanner",			2,  {"NO", "YES (slow)"} },
	{ "Show\nDanger",			2, {"OFF","ON"} },
	{ "Hidden\nSongs",			2, {"OFF","ON"} },
	{ "Easter\nEggs",			2, {"OFF","ON"} },
	{ "Marvelous\nTiming",		2, {"OFF","ON"} },
	{ "Pick Extra\nStage",		2, {"OFF","ON"} }
};

ScreenGameplayOptions::ScreenGameplayOptions() :
	ScreenOptions(
		THEME->GetPathTo("BGAnimations","gameplay options"),
		THEME->GetPathTo("Graphics","gameplay options page"),
		THEME->GetPathTo("Graphics","gameplay options top edge")
		)
{
	LOG->Trace( "ScreenGameplayOptions::ScreenGameplayOptions()" );

	// fill g_InputOptionsLines with explanation text
	for( int i=0; i<NUM_GAMEPLAY_OPTIONS_LINES; i++ )
	{
		CString sLineName = g_GameplayOptionsLines[i].szTitle;
		sLineName.Replace("\n","");
		sLineName.Replace(" ","");
		strcpy( g_GameplayOptionsLines[i].szExplanation, THEME->GetMetric("ScreenGameplayOptions",sLineName) );
	}

	Init( 
		INPUTMODE_BOTH, 
		g_GameplayOptionsLines, 
		NUM_GAMEPLAY_OPTIONS_LINES,
		false );
	m_Menu.StopTimer();

	SOUNDMAN->PlayMusic( THEME->GetPathTo("Sounds","machine options music") );
}

void ScreenGameplayOptions::ImportOptions()
{
	m_iSelectedOption[0][GO_BGMODE]					= PREFSMAN->m_BackgroundMode;
	m_iSelectedOption[0][GO_BGBRIGHTNESS]			= (int)( PREFSMAN->m_fBGBrightness*10+0.5f ); 
	m_iSelectedOption[0][GO_BGIFNOBANNER]			= PREFSMAN->m_bUseBGIfNoBanner ? 1:0;
	m_iSelectedOption[0][GO_SHOW_DANGER]			= PREFSMAN->m_bShowDanger ? 1:0;
	m_iSelectedOption[0][GO_HIDDEN_SONGS]			= PREFSMAN->m_bHiddenSongs ? 1:0;
	m_iSelectedOption[0][GO_EASTER_EGGS]			= PREFSMAN->m_bEasterEggs ? 1:0;
	m_iSelectedOption[0][GO_MARVELOUS]				= PREFSMAN->m_bMarvelousTiming ? 1:0;
	m_iSelectedOption[0][GO_PICK_EXTRA_STAGE]		= PREFSMAN->m_bPickExtraStage? 1:0;
}

void ScreenGameplayOptions::ExportOptions()
{
	(int&)PREFSMAN->m_BackgroundMode	= m_iSelectedOption[0][GO_BGMODE];
	PREFSMAN->m_fBGBrightness			= m_iSelectedOption[0][GO_BGBRIGHTNESS] / 10.0f;
	PREFSMAN->m_bUseBGIfNoBanner		= m_iSelectedOption[0][GO_BGIFNOBANNER] == 1;
	PREFSMAN->m_bShowDanger				= m_iSelectedOption[0][GO_SHOW_DANGER] == 1;
	PREFSMAN->m_bHiddenSongs			= m_iSelectedOption[0][GO_HIDDEN_SONGS]	== 1;
	PREFSMAN->m_bEasterEggs			= m_iSelectedOption[0][GO_EASTER_EGGS] == 1;
	PREFSMAN->m_bMarvelousTiming	= m_iSelectedOption[0][GO_MARVELOUS] == 1;
	PREFSMAN->m_bPickExtraStage		= m_iSelectedOption[0][GO_PICK_EXTRA_STAGE] == 1;
}

void ScreenGameplayOptions::GoToPrevState()
{
	SCREENMAN->SetNewScreen( "ScreenOptionsMenu" );
}

void ScreenGameplayOptions::GoToNextState()
{
	PREFSMAN->SaveGlobalPrefsToDisk();
	GoToPrevState();
}

