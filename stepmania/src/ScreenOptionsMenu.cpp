#include "global.h"
/*
-----------------------------------------------------------------------------
 Class: ScreenOptionsMenu

 Desc: See header.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "ScreenOptionsMenu.h"
#include "RageTextureManager.h"
#include "RageUtil.h"
#include "RageSoundManager.h"
#include "ScreenManager.h"
#include "PrefsManager.h"
#include "GameConstantsAndTypes.h"
#include "StepMania.h"
#include "PrefsManager.h"
#include "RageLog.h"
#include "AnnouncerManager.h"
#include "GameManager.h"
#include "GameState.h"
#include "ThemeManager.h"


enum {
	OM_APPEARANCE = 0,
	OM_INPUT,
	OM_CONFIG_KEY_JOY,
	OM_GAMEPLAY,
	OM_GRAPHIC,
	OM_MACHINE,
	OM_SOUND,
	NUM_OPTIONS_MENU_LINES
};

OptionRowData g_OptionsMenuLines[NUM_OPTIONS_MENU_LINES] = {
	{ "",	1, {"Appearance Options"} },
	{ "",	1, {"Config Key/Joy"} },
	{ "",	1, {"Input Options"} },
	{ "",	1, {"Gameplay Options"} },
	{ "",	1, {"Graphic Options"} },
	{ "",	1, {"Machine Options"} },
	{ "",	1, {"Sound Options"} },
};

ScreenOptionsMenu::ScreenOptionsMenu() :
	ScreenOptions(
		THEME->GetPathTo("BGAnimations","options menu"),
		THEME->GetPathTo("Graphics","options menu page"),
		THEME->GetPathTo("Graphics","options menu top edge")
		)
{
	LOG->Trace( "ScreenOptionsMenu::ScreenOptionsMenu()" );

	// fill g_InputOptionsLines with explanation text
	for( int i=0; i<NUM_OPTIONS_MENU_LINES; i++ )
	{
		CString sLineName = g_OptionsMenuLines[i].szOptionsText[0];
		sLineName.Replace("\n","");
		sLineName.Replace(" ","");
		strcpy( g_OptionsMenuLines[i].szExplanation, THEME->GetMetric("ScreenOptionsMenu",sLineName) );
	}

	Init( 
		INPUTMODE_BOTH, 
		g_OptionsMenuLines, 
		NUM_OPTIONS_MENU_LINES,
		false );
	m_Menu.SetTimer( 99 );
	m_Menu.StopTimer();

	SOUNDMAN->PlayMusic( THEME->GetPathTo("Sounds","options menu music") );
}

/* We depend on the SM options navigation for this screen, not arcade. */
void ScreenOptionsMenu::MenuStart( PlayerNumber pn )
{
	StartGoToNextState();
}

void ScreenOptionsMenu::ImportOptions()
{
}

void ScreenOptionsMenu::ExportOptions()
{

}

void ScreenOptionsMenu::GoToPrevState()
{
	SCREENMAN->SetNewScreen( "ScreenTitleMenu" );
}

void ScreenOptionsMenu::GoToNextState()
{
	switch( this->m_iCurrentRow[0] )
	{
		case OM_APPEARANCE:		SCREENMAN->SetNewScreen("ScreenAppearanceOptions");	break;
		case OM_CONFIG_KEY_JOY:	SCREENMAN->SetNewScreen("ScreenMapControllers");	break;
		case OM_GAMEPLAY:		SCREENMAN->SetNewScreen("ScreenGameplayOptions");	break;
		case OM_GRAPHIC:		SCREENMAN->SetNewScreen("ScreenGraphicOptions");	break;
		case OM_INPUT:			SCREENMAN->SetNewScreen("ScreenInputOptions");		break;
		case OM_MACHINE:		SCREENMAN->SetNewScreen("ScreenMachineOptions");	break;
		case OM_SOUND:			SCREENMAN->SetNewScreen("ScreenSoundOptions");		break;
		default:	// Exit
			SCREENMAN->SetNewScreen("ScreenTitleMenu");
	}
}