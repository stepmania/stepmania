#include "global.h"
/*
-----------------------------------------------------------------------------
 Class: ScreenBackgroundOptions

 Desc: See header.

 Copyright (c) 2001-2003 by the person(s) listed below.  All rights reserved.
	Chris Danford
	Chris Gomez
-----------------------------------------------------------------------------
*/

#include "ScreenBackgroundOptions.h"
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
	BO_MODE,
	BO_BRIGHTNESS,
	BO_DANGER,
//	BO_DANCING_CHARACTERS,
	NUM_BACKGROUND_OPTIONS_LINES
};

OptionRow g_BackgroundOptionsLines[NUM_BACKGROUND_OPTIONS_LINES] = {
	OptionRow( "Mode",					"OFF","ANIMATIONS","VISUALIZATIONS","RANDOM MOVIES" ),
	OptionRow( "Brightness",			"0%","10%","20%","30%","40%","50%","60%","70%","80%","90%","100%" ),
	OptionRow( "Danger",				"HIDE","SHOW" ),
//	OptionRow( "Dancing\nCharacters",	"HIDE","SHOW" )
};

ScreenBackgroundOptions::ScreenBackgroundOptions() :
	ScreenOptions("ScreenBackgroundOptions",false)
{
	LOG->Trace( "ScreenBackgroundOptions::ScreenBackgroundOptions()" );

	Init( 
		INPUTMODE_BOTH, 
		g_BackgroundOptionsLines, 
		NUM_BACKGROUND_OPTIONS_LINES,
		false, true );
	m_Menu.m_MenuTimer.Disable();

	SOUNDMAN->PlayMusic( THEME->GetPathToS("ScreenMachineOptions music") );
}

void ScreenBackgroundOptions::ImportOptions()
{
	m_iSelectedOption[0][BO_MODE]				= PREFSMAN->m_BackgroundMode;
	m_iSelectedOption[0][BO_BRIGHTNESS]			= (int)( PREFSMAN->m_fBGBrightness*10+0.5f ); 
	m_iSelectedOption[0][BO_DANGER]				= PREFSMAN->m_bShowDanger ? 1:0;
//	m_iSelectedOption[0][BO_DANCING_CHARACTERS]	= PREFSMAN->m_bShowDancingCharacters? 1:0;
}

void ScreenBackgroundOptions::ExportOptions()
{
	(int&)PREFSMAN->m_BackgroundMode	= m_iSelectedOption[0][BO_MODE];
	PREFSMAN->m_fBGBrightness			= m_iSelectedOption[0][BO_BRIGHTNESS] / 10.0f;
	PREFSMAN->m_bShowDanger				= m_iSelectedOption[0][BO_DANGER] == 1;
//	PREFSMAN->m_bShowDancingCharacters	= m_iSelectedOption[0][BO_DANCING_CHARACTERS] == 1;
}

void ScreenBackgroundOptions::GoToPrevState()
{
	SCREENMAN->SetNewScreen( "ScreenOptionsMenu" );
}

void ScreenBackgroundOptions::GoToNextState()
{
	PREFSMAN->SaveGlobalPrefsToDisk();
	GoToPrevState();
}

