#include "global.h"
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
	GO_SOLO_SINGLE,
	GO_HIDDEN_SONGS,
	GO_EASTER_EGGS,
	GO_MARVELOUS,
	GO_PICK_EXTRA_STAGE,
	NUM_GAMEPLAY_OPTIONS_LINES
};

OptionRow g_GameplayOptionsLines[NUM_GAMEPLAY_OPTIONS_LINES] = {
	OptionRow( "Background\nMode",		"OFF","ANIMATIONS","VISUALIZATIONS","RANDOM MOVIES" ),
	OptionRow( "Background\nBrightness","0%","10%","20%","30%","40%","50%","60%","70%","80%","90%","100%" ),
	OptionRow( "BG For\nBanner",		"NO", "YES (slow)" ),
	OptionRow( "Show\nDanger",			"OFF","ON" ),
	OptionRow( "Solo\nSingles",			"OFF","ON" ),
	OptionRow( "Hidden\nSongs",			"OFF","ON" ),
	OptionRow( "Easter\nEggs",			"OFF","ON" ),
	OptionRow( "Marvelous\nTiming",		"OFF","ON" ),
	OptionRow( "Pick Extra\nStage",		"OFF","ON" )
};

ScreenGameplayOptions::ScreenGameplayOptions() :
	ScreenOptions("ScreenGameplayOptions",false)
{
	LOG->Trace( "ScreenGameplayOptions::ScreenGameplayOptions()" );

	Init( 
		INPUTMODE_BOTH, 
		g_GameplayOptionsLines, 
		NUM_GAMEPLAY_OPTIONS_LINES,
		false, true );
	m_Menu.m_MenuTimer.Disable();

	SOUNDMAN->PlayMusic( THEME->GetPathTo("Sounds","ScreenMachineOptions music") );
}

void ScreenGameplayOptions::ImportOptions()
{
	m_iSelectedOption[0][GO_BGMODE]					= PREFSMAN->m_BackgroundMode;
	m_iSelectedOption[0][GO_BGBRIGHTNESS]			= (int)( PREFSMAN->m_fBGBrightness*10+0.5f ); 
	m_iSelectedOption[0][GO_BGIFNOBANNER]			= PREFSMAN->m_bUseBGIfNoBanner ? 1:0;
	m_iSelectedOption[0][GO_SHOW_DANGER]			= PREFSMAN->m_bShowDanger ? 1:0;
	m_iSelectedOption[0][GO_SOLO_SINGLE]			= PREFSMAN->m_bSoloSingle ? 1:0;
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
	PREFSMAN->m_bSoloSingle				= m_iSelectedOption[0][GO_SOLO_SINGLE] == 1;
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

