#include "global.h"
/*
-----------------------------------------------------------------------------
 Class: ScreenProfileOptions

 Desc: See header.

 Copyright (c) 2001-2003 by the person(s) listed below.  All rights reserved.
	Chris Danford
	Chris POmez
-----------------------------------------------------------------------------
*/

#include "ScreenProfileOptions.h"
#include "RageLog.h"
#include "ProfileManager.h"
#include "RageSounds.h"
#include "ThemeManager.h"
#include "PrefsManager.h"
#include "ScreenManager.h"


enum {
	PO_PLAYER1,
	PO_PLAYER2,
	PO_CREATE_NEW,
	PO_DELETE_,
	NUM_GAMEPLAY_OPTIONS_LINES
};

OptionRow g_ProfileOptionsLines[NUM_GAMEPLAY_OPTIONS_LINES] = {
	OptionRow( "Player1\nProfile",	true ),
	OptionRow( "Player2\nProfile",	true ),
	OptionRow( "Create\nNew",		true, "PRESS START" ),
	OptionRow( "Delete",			true ),
};

ScreenProfileOptions::ScreenProfileOptions() :
	ScreenOptions("ScreenProfileOptions",false)
{
	LOG->Trace( "ScreenProfileOptions::ScreenProfileOptions()" );

	g_ProfileOptionsLines[PO_PLAYER1].choices.clear();
	PROFILEMAN->GetProfileNames( g_ProfileOptionsLines[PO_PLAYER1].choices );
	if( g_ProfileOptionsLines[PO_PLAYER1].choices.empty() )
		g_ProfileOptionsLines[PO_PLAYER1].choices.push_back( "-NONE-" );

	g_ProfileOptionsLines[PO_PLAYER2].choices.clear();
	PROFILEMAN->GetProfileNames( g_ProfileOptionsLines[PO_PLAYER2].choices );
	if( g_ProfileOptionsLines[PO_PLAYER2].choices.empty() )
		g_ProfileOptionsLines[PO_PLAYER2].choices.push_back( "-NONE-" );

	g_ProfileOptionsLines[PO_DELETE_].choices.clear();
	PROFILEMAN->GetProfileNames( g_ProfileOptionsLines[PO_DELETE_].choices );
	if( g_ProfileOptionsLines[PO_DELETE_].choices.empty() )
		g_ProfileOptionsLines[PO_DELETE_].choices.push_back( "-NONE-" );

	Init( 
		INPUTMODE_TOGETHER, 
		g_ProfileOptionsLines, 
		NUM_GAMEPLAY_OPTIONS_LINES,
		true );
	m_Menu.m_MenuTimer.Disable();

	SOUND->PlayMusic( THEME->GetPathToS("ScreenMachineOptions music") );
}

void ScreenProfileOptions::ImportOptions()
{
	CStringArray::iterator iter;

	iter = find( 
		g_ProfileOptionsLines[PO_PLAYER1].choices.begin(),
		g_ProfileOptionsLines[PO_PLAYER1].choices.end(),
		PREFSMAN->m_sDefaultProfile[PLAYER_1] );
	if( iter != g_ProfileOptionsLines[PO_PLAYER1].choices.end() )
		m_iSelectedOption[0][PO_PLAYER1]			= iter - g_ProfileOptionsLines[PO_PLAYER1].choices.begin();

	iter = find( 
		g_ProfileOptionsLines[PO_PLAYER2].choices.begin(),
		g_ProfileOptionsLines[PO_PLAYER2].choices.end(),
		PREFSMAN->m_sDefaultProfile[PLAYER_2] );
	if( iter != g_ProfileOptionsLines[PO_PLAYER2].choices.end() )
		m_iSelectedOption[0][PO_PLAYER2]			= iter - g_ProfileOptionsLines[PO_PLAYER2].choices.begin();
}

void ScreenProfileOptions::ExportOptions()
{
	PREFSMAN->m_sDefaultProfile[PLAYER_1] = g_ProfileOptionsLines[PO_PLAYER1].choices[m_iSelectedOption[0][PO_PLAYER1]];
	PREFSMAN->m_sDefaultProfile[PLAYER_2] = g_ProfileOptionsLines[PO_PLAYER2].choices[m_iSelectedOption[0][PO_PLAYER2]];
}

void ScreenProfileOptions::GoToPrevState()
{
	SCREENMAN->SetNewScreen( "ScreenOptionsMenu" );
}

void ScreenProfileOptions::GoToNextState()
{
	PREFSMAN->SaveGlobalPrefsToDisk();
	GoToPrevState();
}
