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
	PO_RENAME_,
	NUM_PROFILE_OPTIONS_LINES
};

OptionRow g_ProfileOptionsLines[NUM_PROFILE_OPTIONS_LINES] = {
	OptionRow( "Player1\nProfile",	true ),
	OptionRow( "Player2\nProfile",	true ),
	OptionRow( "Create\nNew",		true, "PRESS START" ),
	OptionRow( "Delete",			true ),
	OptionRow( "Rename",			true ),
};

ScreenProfileOptions::ScreenProfileOptions( CString sClassName ) : ScreenOptions( sClassName )
{
	LOG->Trace( "ScreenProfileOptions::ScreenProfileOptions()" );

	g_ProfileOptionsLines[PO_PLAYER1].choices.clear();
	g_ProfileOptionsLines[PO_PLAYER1].choices.push_back( "-NONE-" );
	PROFILEMAN->GetProfileDisplayNames( g_ProfileOptionsLines[PO_PLAYER1].choices );

	g_ProfileOptionsLines[PO_PLAYER2].choices.clear();
	g_ProfileOptionsLines[PO_PLAYER2].choices.push_back( "-NONE-" );
	PROFILEMAN->GetProfileDisplayNames( g_ProfileOptionsLines[PO_PLAYER2].choices );

	g_ProfileOptionsLines[PO_DELETE_].choices.clear();
	g_ProfileOptionsLines[PO_DELETE_].choices.push_back( "-NONE-" );
	PROFILEMAN->GetProfileDisplayNames( g_ProfileOptionsLines[PO_DELETE_].choices );

	g_ProfileOptionsLines[PO_RENAME_].choices.clear();
	g_ProfileOptionsLines[PO_RENAME_].choices.push_back( "-NONE-" );
	PROFILEMAN->GetProfileDisplayNames( g_ProfileOptionsLines[PO_RENAME_].choices );

	Init( 
		INPUTMODE_TOGETHER, 
		g_ProfileOptionsLines, 
		NUM_PROFILE_OPTIONS_LINES,
		true );
	m_Menu.m_MenuTimer.Disable();

	SOUND->PlayMusic( THEME->GetPathToS("ScreenMachineOptions music") );
}

void ScreenProfileOptions::ImportOptions()
{
	vector<CString> vsProfiles;
	PROFILEMAN->GetProfiles( vsProfiles );

	CStringArray::iterator iter;

	iter = find( 
		vsProfiles.begin(),
		vsProfiles.end(),
		PREFSMAN->m_sDefaultProfile[PLAYER_1] );
	if( iter != vsProfiles.end() )
		m_iSelectedOption[0][PO_PLAYER1] = iter - vsProfiles.begin() + 1;

	iter = find( 
		vsProfiles.begin(),
		vsProfiles.end(),
		PREFSMAN->m_sDefaultProfile[PO_PLAYER2] );
	if( iter != vsProfiles.end() )
		m_iSelectedOption[0][PO_PLAYER2] = iter - vsProfiles.begin() + 1;
}

void ScreenProfileOptions::ExportOptions()
{
	vector<CString> vsProfiles;
	PROFILEMAN->GetProfiles( vsProfiles );

	if( m_iSelectedOption[0][PO_PLAYER1] > 0 )
		PREFSMAN->m_sDefaultProfile[PLAYER_1] = vsProfiles[m_iSelectedOption[0][PO_PLAYER1]-1];
	else
		PREFSMAN->m_sDefaultProfile[PLAYER_1] = "";

	if( m_iSelectedOption[0][PO_PLAYER2] > 0 )
		PREFSMAN->m_sDefaultProfile[PLAYER_2] = vsProfiles[m_iSelectedOption[0][PO_PLAYER2]-1];
	else
		PREFSMAN->m_sDefaultProfile[PLAYER_2] = "";
}

void ScreenProfileOptions::GoToPrevState()
{
	SCREENMAN->SetNewScreen( "ScreenOptionsMenu" );
}

void CreateProfile( CString sDisplayName )
{
	PROFILEMAN->CreateProfile( sDisplayName );
	SCREENMAN->SetNewScreen( "ScreenProfileOptions" );	
}

void ScreenProfileOptions::GoToNextState()
{
	PREFSMAN->SaveGlobalPrefsToDisk();
	GoToPrevState();
}

void ScreenProfileOptions::MenuStart( PlayerNumber pn )
{
	vector<CString> vsProfiles;
	PROFILEMAN->GetProfiles( vsProfiles );

	switch( GetCurrentRow() )
	{
	case PO_CREATE_NEW:
		SCREENMAN->TextEntry( SM_None, "Enter a profile name", "", CreateProfile );
		return;
	case PO_DELETE_:
		{
			CString sProfile;
			if( m_iSelectedOption[0][PO_DELETE_] > 0 )
				sProfile = vsProfiles[m_iSelectedOption[0][PO_DELETE_]-1];
			else
				sProfile = "";
		}
		break;
	case PO_RENAME_:
		{
			CString sProfile;
			if( m_iSelectedOption[0][PO_RENAME_] > 0 )
				sProfile = vsProfiles[m_iSelectedOption[0][PO_RENAME_]-1];
			else
				sProfile = "";

//			SCREENMAN->TextEntry( SM_None, "Enter a profile name", "NewProfile", SetProfile );
		}
		break;
	default:
		ScreenOptions::MenuStart( pn );
	}
}
