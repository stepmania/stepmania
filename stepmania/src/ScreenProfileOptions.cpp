#include "global.h"
#include "ScreenProfileOptions.h"
#include "RageLog.h"
#include "ProfileManager.h"
#include "GameSoundManager.h"
#include "ThemeManager.h"
#include "PrefsManager.h"
#include "ScreenManager.h"
#include "ScreenTextEntry.h"
#include "ScreenPrompt.h"
#include "GameState.h"


enum {
	PO_PLAYER1,
	PO_PLAYER2,
	PO_CREATE_NEW,
	PO_DELETE_,
	PO_RENAME_,
	PO_OS_MOUNT_1,
	PO_OS_MOUNT_2,
	NUM_PROFILE_OPTIONS_LINES
};

OptionRowDefinition g_ProfileOptionsLines[NUM_PROFILE_OPTIONS_LINES] = {
	OptionRowDefinition( "Player1Profile",	true ),
	OptionRowDefinition( "Player2Profile",	true ),
	OptionRowDefinition( "CreateNew",		true, "PRESS START" ),
	OptionRowDefinition( "Delete",			true ),
	OptionRowDefinition( "Rename",			true ),
	OptionRowDefinition( "OsMountPlayer1",	true, "" ),
	OptionRowDefinition( "OsMountPlayer2",	true, "" ),
};

AutoScreenMessage( SM_DoneCreating )
AutoScreenMessage( SM_DoneRenaming )
AutoScreenMessage( SM_DoneDeleting )

REGISTER_SCREEN_CLASS( ScreenProfileOptions );
ScreenProfileOptions::ScreenProfileOptions( CString sClassName ) : ScreenOptions( sClassName )
{
	LOG->Trace( "ScreenProfileOptions::ScreenProfileOptions()" );
}

void ScreenProfileOptions::Init()
{
	ScreenOptions::Init();

	g_ProfileOptionsLines[PO_PLAYER1].choices.clear();
	g_ProfileOptionsLines[PO_PLAYER1].choices.push_back( "-NONE-" );
	PROFILEMAN->GetLocalProfileNames( g_ProfileOptionsLines[PO_PLAYER1].choices );

	g_ProfileOptionsLines[PO_PLAYER2].choices.clear();
	g_ProfileOptionsLines[PO_PLAYER2].choices.push_back( "-NONE-" );
	PROFILEMAN->GetLocalProfileNames( g_ProfileOptionsLines[PO_PLAYER2].choices );

	g_ProfileOptionsLines[PO_DELETE_].choices.clear();
	g_ProfileOptionsLines[PO_DELETE_].choices.push_back( "-NONE-" );
	PROFILEMAN->GetLocalProfileNames( g_ProfileOptionsLines[PO_DELETE_].choices );

	g_ProfileOptionsLines[PO_RENAME_].choices.clear();
	g_ProfileOptionsLines[PO_RENAME_].choices.push_back( "-NONE-" );
	PROFILEMAN->GetLocalProfileNames( g_ProfileOptionsLines[PO_RENAME_].choices );

	if( PREFSMAN->GetMemoryCardOsMountPoint(PLAYER_1).Get().empty() )
		g_ProfileOptionsLines[PO_OS_MOUNT_1].choices[0] = "-NOT SET IN INI-";
	else
		g_ProfileOptionsLines[PO_OS_MOUNT_1].choices[0] = PREFSMAN->GetMemoryCardOsMountPoint(PLAYER_1).Get();

	if( PREFSMAN->GetMemoryCardOsMountPoint(PLAYER_2).Get().empty() )
		g_ProfileOptionsLines[PO_OS_MOUNT_2].choices[0] = "-NOT SET IN INI-";
	else
		g_ProfileOptionsLines[PO_OS_MOUNT_2].choices[0] = PREFSMAN->GetMemoryCardOsMountPoint(PLAYER_2).Get();

	//Enable all lines for all players
	for ( unsigned int i = 0; i < NUM_PROFILE_OPTIONS_LINES; i++ )
		FOREACH_PlayerNumber( pn )
			g_ProfileOptionsLines[i].m_vEnabledForPlayers.insert( pn );

	vector<OptionRowDefinition> vDefs( &g_ProfileOptionsLines[0], &g_ProfileOptionsLines[ARRAYSIZE(g_ProfileOptionsLines)] );
	vector<OptionRowHandler*> vHands( vDefs.size(), NULL );
	InitMenu( INPUTMODE_SHARE_CURSOR, vDefs, vHands );

	SOUND->PlayMusic( THEME->GetPathS("ScreenMachineOptions","music") );
}

void ScreenProfileOptions::ImportOptions( int row, const vector<PlayerNumber> &vpns )
{
	switch( row )
	{
	case PO_PLAYER1:
	case PO_PLAYER2:
		{
			PlayerNumber pn = (PlayerNumber)(row - PO_PLAYER1);
			vector<CString> vsProfiles;
			PROFILEMAN->GetLocalProfileIDs( vsProfiles );

			CStringArray::iterator iter = find( 
				vsProfiles.begin(),
				vsProfiles.end(),
				PREFSMAN->GetDefaultLocalProfileID(pn).Get() );
			if( iter != vsProfiles.end() )
				m_Rows[row]->SetOneSharedSelection( iter - vsProfiles.begin() + 1 );
		}
		break;
	}
}

void ScreenProfileOptions::ExportOptions( int row, const vector<PlayerNumber> &vpns )
{
	switch( row )
	{
	case PO_PLAYER1:
	case PO_PLAYER2:
		{
			PlayerNumber pn = (PlayerNumber)(row - PO_PLAYER1);
			vector<CString> vsProfiles;
			PROFILEMAN->GetLocalProfileIDs( vsProfiles );

			if( m_Rows[row]->GetOneSharedSelection() > 0 )
				PREFSMAN->GetDefaultLocalProfileID(pn).Set( vsProfiles[m_Rows[row]->GetOneSharedSelection()-1] );
			else
				PREFSMAN->GetDefaultLocalProfileID(pn).Set( "" );
		}
		break;
	}
}

void ScreenProfileOptions::GoToPrevScreen()
{
	SCREENMAN->SetNewScreen( "ScreenOptionsMenu" );
}

void ScreenProfileOptions::GoToNextScreen()
{
	PREFSMAN->SaveGlobalPrefsToDisk();
	GoToPrevScreen();
}

void ScreenProfileOptions::HandleScreenMessage( const ScreenMessage SM )
{
	if( SM == SM_DoneCreating)
	{
		if( !ScreenTextEntry::s_bCancelledLast && ScreenTextEntry::s_sLastAnswer != "" )
		{
			CString sNewName = ScreenTextEntry::s_sLastAnswer;
			bool bResult = PROFILEMAN->CreateLocalProfile( sNewName );
			if( bResult )
				SCREENMAN->SetNewScreen( "ScreenProfileOptions" );	// reload
			else
				SCREENMAN->Prompt( SM_None, ssprintf("Error creating profile '%s'.", sNewName.c_str()) );
		}
	}
	else if( SM == SM_DoneRenaming )
	{
		if( !ScreenTextEntry::s_bCancelledLast )
		{
			CString sProfileID = GetSelectedProfileID();
			CString sName = GetSelectedProfileName();
			CString sNewName = ScreenTextEntry::s_sLastAnswer;
			bool bResult = PROFILEMAN->RenameLocalProfile( sProfileID, sNewName );
			if( bResult )
				SCREENMAN->SetNewScreen( "ScreenProfileOptions" );	// reload
			else
				SCREENMAN->Prompt( SM_None, ssprintf("Error renaming profile %s '%s'\nto '%s'.", sProfileID.c_str(), sName.c_str(), sNewName.c_str()) );
		}
	}
	else if ( SM == SM_DoneDeleting )
	{
		if( ScreenPrompt::s_LastAnswer == ANSWER_YES )
		{
			CString sProfileID = GetSelectedProfileID();
			CString sName = GetSelectedProfileName();
			bool bResult = PROFILEMAN->DeleteLocalProfile( sProfileID );
			if( bResult )
				SCREENMAN->SetNewScreen( "ScreenProfileOptions" );	// reload
			else
				SCREENMAN->Prompt( SM_None, ssprintf("Error deleting profile %s '%s'.", sName.c_str(), sProfileID.c_str()) );
		}
	}

	ScreenOptions::HandleScreenMessage( SM );
}


void ScreenProfileOptions::MenuStart( PlayerNumber pn, const InputEventType type )
{
	switch( GetCurrentRow() )
	{
	case PO_CREATE_NEW:
		SCREENMAN->TextEntry( SM_DoneCreating, "Enter a profile name", "", 12 );
		break;
	case PO_DELETE_:
	{
		const CString sProfileID = GetSelectedProfileID();
		const CString sName = GetSelectedProfileName();

		if( sProfileID=="" )
			SCREENMAN->PlayInvalidSound();
		else
			SCREENMAN->Prompt( SM_DoneDeleting, ssprintf("Delete profile %s '%s'?",sProfileID.c_str(),sName.c_str()), PROMPT_YES_NO, ANSWER_NO );
		break;
	}
	case PO_RENAME_:
	{
		const CString sProfileID = GetSelectedProfileID();
		const CString sName = GetSelectedProfileName();

		if( sProfileID=="" )
			SCREENMAN->PlayInvalidSound();
		else
			SCREENMAN->TextEntry( SM_DoneRenaming, ssprintf("Rename profile %s '%s'",sProfileID.c_str(),sName.c_str()), sName, 12 );
		break;
	}
	default:
		ScreenOptions::MenuStart( pn, type );
	}
}

CString ScreenProfileOptions::GetSelectedProfileID()
{
	vector<CString> vsProfiles;
	PROFILEMAN->GetLocalProfileIDs( vsProfiles );

	const OptionRow &row = *m_Rows[GetCurrentRow()];
	const int Selection = row.GetOneSharedSelection();
	if( !Selection )
		return "";
	return vsProfiles[ Selection-1 ];
}

CString ScreenProfileOptions::GetSelectedProfileName()
{
	const OptionRow &row = *m_Rows[GetCurrentRow()];
	const int Selection = row.GetOneSharedSelection();
	if( !Selection )
		return "";
	return g_ProfileOptionsLines[PO_PLAYER1].choices[ Selection ];
}

/*
 * (c) 2001-2004 Chris Danford
 * All rights reserved.
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, and/or sell copies of the Software, and to permit persons to
 * whom the Software is furnished to do so, provided that the above
 * copyright notice(s) and this permission notice appear in all copies of
 * the Software and that both the above copyright notice(s) and this
 * permission notice appear in supporting documentation.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT OF
 * THIRD PARTY RIGHTS. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR HOLDERS
 * INCLUDED IN THIS NOTICE BE LIABLE FOR ANY CLAIM, OR ANY SPECIAL INDIRECT
 * OR CONSEQUENTIAL DAMAGES, OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS
 * OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
 * OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */
