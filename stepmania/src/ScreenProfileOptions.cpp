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
#include "ScreenTextEntry.h"
#include "ScreenPrompt.h"


enum {
	PO_PLAYER1,
	PO_PLAYER2,
	PO_CREATE_NEW,
	PO_DELETE_,
	PO_RENAME_,
	PO_CARD_DIR_1,
	PO_CARD_DIR_2,
	NUM_PROFILE_OPTIONS_LINES
};

OptionRow g_ProfileOptionsLines[NUM_PROFILE_OPTIONS_LINES] = {
	OptionRow( "Player1\nProfile",	true ),
	OptionRow( "Player2\nProfile",	true ),
	OptionRow( "Create\nNew",		true, "PRESS START" ),
	OptionRow( "Delete",			true ),
	OptionRow( "Rename",			true ),
	OptionRow( "Card Dir\nPlayer1",	true, "" ),
	OptionRow( "Card Dir\nPlayer2",	true, "" ),
};

const ScreenMessage	SM_DoneCreating		= ScreenMessage(SM_User+1);
const ScreenMessage	SM_DoneRenaming		= ScreenMessage(SM_User+2);
const ScreenMessage	SM_DoneDeleting		= ScreenMessage(SM_User+3);

ScreenProfileOptions::ScreenProfileOptions( CString sClassName ) : ScreenOptions( sClassName )
{
	LOG->Trace( "ScreenProfileOptions::ScreenProfileOptions()" );

	g_ProfileOptionsLines[PO_PLAYER1].choices.clear();
	g_ProfileOptionsLines[PO_PLAYER1].choices.push_back( "-NONE-" );
	PROFILEMAN->GetMachineProfileNames( g_ProfileOptionsLines[PO_PLAYER1].choices );

	g_ProfileOptionsLines[PO_PLAYER2].choices.clear();
	g_ProfileOptionsLines[PO_PLAYER2].choices.push_back( "-NONE-" );
	PROFILEMAN->GetMachineProfileNames( g_ProfileOptionsLines[PO_PLAYER2].choices );

	g_ProfileOptionsLines[PO_DELETE_].choices.clear();
	g_ProfileOptionsLines[PO_DELETE_].choices.push_back( "-NONE-" );
	PROFILEMAN->GetMachineProfileNames( g_ProfileOptionsLines[PO_DELETE_].choices );

	g_ProfileOptionsLines[PO_RENAME_].choices.clear();
	g_ProfileOptionsLines[PO_RENAME_].choices.push_back( "-NONE-" );
	PROFILEMAN->GetMachineProfileNames( g_ProfileOptionsLines[PO_RENAME_].choices );

	if( PREFSMAN->m_sMemoryCardDir[PLAYER_1].empty() )
		g_ProfileOptionsLines[PO_CARD_DIR_1].choices[0] = "-NOT SET IN INI-";
	else
		g_ProfileOptionsLines[PO_CARD_DIR_1].choices[0] = PREFSMAN->m_sMemoryCardDir[PLAYER_1];

	if( PREFSMAN->m_sMemoryCardDir[PLAYER_2].empty() )
		g_ProfileOptionsLines[PO_CARD_DIR_2].choices[0] = "-NOT SET IN INI-";
	else
		g_ProfileOptionsLines[PO_CARD_DIR_2].choices[0] = PREFSMAN->m_sMemoryCardDir[PLAYER_2];

	Init( 
		INPUTMODE_TOGETHER, 
		g_ProfileOptionsLines, 
		NUM_PROFILE_OPTIONS_LINES );

	SOUND->PlayMusic( THEME->GetPathToS("ScreenMachineOptions music") );
}

void ScreenProfileOptions::ImportOptions()
{
	vector<CString> vsProfiles;
	PROFILEMAN->GetMachineProfileIDs( vsProfiles );

	CStringArray::iterator iter;

	iter = find( 
		vsProfiles.begin(),
		vsProfiles.end(),
		PREFSMAN->m_sDefaultMachineProfileID[PLAYER_1] );
	if( iter != vsProfiles.end() )
		m_iSelectedOption[0][PO_PLAYER1] = iter - vsProfiles.begin() + 1;

	iter = find( 
		vsProfiles.begin(),
		vsProfiles.end(),
		PREFSMAN->m_sDefaultMachineProfileID[PLAYER_2] );
	if( iter != vsProfiles.end() )
		m_iSelectedOption[0][PO_PLAYER2] = iter - vsProfiles.begin() + 1;
}

void ScreenProfileOptions::ExportOptions()
{
	vector<CString> vsProfiles;
	PROFILEMAN->GetMachineProfileIDs( vsProfiles );

	if( m_iSelectedOption[0][PO_PLAYER1] > 0 )
		PREFSMAN->m_sDefaultMachineProfileID[PLAYER_1] = vsProfiles[m_iSelectedOption[0][PO_PLAYER1]-1];
	else
		PREFSMAN->m_sDefaultMachineProfileID[PLAYER_1] = "";

	if( m_iSelectedOption[0][PO_PLAYER2] > 0 )
		PREFSMAN->m_sDefaultMachineProfileID[PLAYER_2] = vsProfiles[m_iSelectedOption[0][PO_PLAYER2]-1];
	else
		PREFSMAN->m_sDefaultMachineProfileID[PLAYER_2] = "";
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

void ScreenProfileOptions::HandleScreenMessage( const ScreenMessage SM )
{
	CString sProfileID = GetSelectedProfileID();
	CString sName = GetSelectedProfileName();

	switch( SM )
	{
	case SM_DoneCreating:
		if( !ScreenTextEntry::s_bCancelledLast )
		{
			CString sNewName = ScreenTextEntry::s_sLastAnswer;
			bool bResult = PROFILEMAN->CreateMachineProfile( sNewName );
			if( bResult )
				SCREENMAN->SetNewScreen( "ScreenProfileOptions" );	// reload
			else
				SCREENMAN->Prompt( SM_None, ssprintf("Error creating profile '%s'.", sNewName.c_str()) );
		}
		break;
	case SM_DoneRenaming:
		if( !ScreenTextEntry::s_bCancelledLast )
		{
			CString sNewName = ScreenTextEntry::s_sLastAnswer;
			bool bResult = PROFILEMAN->RenameMachineProfile( sProfileID, sNewName );
			if( bResult )
				SCREENMAN->SetNewScreen( "ScreenProfileOptions" );	// reload
			else
				SCREENMAN->Prompt( SM_None, ssprintf("Error renaming profile %s '%s'\nto '%s'.", sProfileID.c_str(), sName.c_str(), sNewName.c_str()) );
		}
		break;
	case SM_DoneDeleting:
		if( ScreenPrompt::s_bLastAnswer )
		{
			bool bResult = PROFILEMAN->DeleteMachineProfile( sProfileID );
			if( bResult )
				SCREENMAN->SetNewScreen( "ScreenProfileOptions" );	// reload
			else
				SCREENMAN->Prompt( SM_None, ssprintf("Error deleting profile %s '%s'.", sName.c_str(), sProfileID.c_str()) );
		}
		break;
	}

	ScreenOptions::HandleScreenMessage( SM );
}


void ScreenProfileOptions::MenuStart( PlayerNumber pn, const InputEventType type )
{
	CString sProfileID = GetSelectedProfileID();
	CString sName = GetSelectedProfileName();

	switch( GetCurrentRow() )
	{
	case PO_CREATE_NEW:
		SCREENMAN->TextEntry( SM_DoneCreating, "Enter a profile name", "", NULL );
		break;
	case PO_DELETE_:
		if( sProfileID=="" )
			SOUND->PlayOnce( THEME->GetPathToS("common invalid") );
		else
			SCREENMAN->Prompt( SM_DoneDeleting, ssprintf("Delete profile %s '%s'?",sProfileID.c_str(),sName.c_str()), true );
		break;
	case PO_RENAME_:
		if( sProfileID=="" )
			SOUND->PlayOnce( THEME->GetPathToS("common invalid") );
		else
			SCREENMAN->TextEntry( SM_DoneRenaming, ssprintf("Rename profile %s '%s'",sProfileID.c_str(),sName.c_str()), sName, NULL );
		break;
	default:
		ScreenOptions::MenuStart( pn, type );
	}
}

CString ScreenProfileOptions::GetSelectedProfileID()
{
	vector<CString> vsProfiles;
	PROFILEMAN->GetMachineProfileIDs( vsProfiles );

	if( m_iSelectedOption[0][m_iCurrentRow[0]]==0 )
		return "";
	else
		return vsProfiles[m_iSelectedOption[0][m_iCurrentRow[0]]-1];
}

CString ScreenProfileOptions::GetSelectedProfileName()
{
	if( m_iSelectedOption[0][m_iCurrentRow[0]]==0 )
		return "";
	else
		return g_ProfileOptionsLines[PO_PLAYER1].choices[ m_iSelectedOption[0][m_iCurrentRow[0]] ];
}

