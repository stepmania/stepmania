#include "global.h"
/*
-----------------------------------------------------------------------------
 Class: ScreenNetworkOptions

 Desc: See header.

 Copyright (c) 2004 by the person(s) listed below.  All rights reserved.
	Charles Lohr
-----------------------------------------------------------------------------
*/
#include "NetworkSyncManager.h"
#include "ScreenNetworkOptions.h"
#include "RageLog.h"
#include "RageSounds.h"
#include "ThemeManager.h"
#include "PrefsManager.h"
#include "ScreenManager.h"
#include "ScreenTextEntry.h"
#include "ScreenPrompt.h"


enum {
	PO_CONNECTION,
	NUM_NETWORK_OPTIONS_LINES
};

enum {
	NO_DISCONNECT=0,
	NO_CONNECT
};

OptionRowData g_NetworkOptionsLines[NUM_NETWORK_OPTIONS_LINES] = {
	OptionRowData( "Connection",	true, "PRESS START" ),
};

const ScreenMessage	SM_DoneConnecting		= ScreenMessage(SM_User+1);

ScreenNetworkOptions::ScreenNetworkOptions( CString sClassName ) : ScreenOptions( sClassName )
{
	LOG->Trace( "ScreenNetworkOptions::ScreenNetworkOptions()" );

	g_NetworkOptionsLines[PO_CONNECTION].choices.clear();
	g_NetworkOptionsLines[PO_CONNECTION].choices.push_back("Disconnect");
	g_NetworkOptionsLines[PO_CONNECTION].choices.push_back("Connect...");
	
	Init( 
		INPUTMODE_SHARE_CURSOR, 
		g_NetworkOptionsLines, 
		NUM_NETWORK_OPTIONS_LINES );

	SOUND->PlayMusic( THEME->GetPathToS("ScreenMachineOptions music") );
}


void ScreenNetworkOptions::GoToPrevState()
{
	SCREENMAN->SetNewScreen( "ScreenOptionsMenu" );
}

void ScreenNetworkOptions::GoToNextState()
{
	GoToPrevState();
}

void ScreenNetworkOptions::HandleScreenMessage( const ScreenMessage SM )
{
	switch( SM )
	{
	case SM_DoneConnecting:
		if( !ScreenTextEntry::s_bCancelledLast )
		{
			CString sNewName = ScreenTextEntry::s_sLastAnswer;
			NSMAN->PostStartUp(sNewName);
			NSMAN->DisplayStartupStatus();
		}
		break;
	}

	ScreenOptions::HandleScreenMessage( SM );
}


void ScreenNetworkOptions::MenuStart( PlayerNumber pn, const InputEventType type )
{
	switch( GetCurrentRow() )
	{
	case PO_CONNECTION:
		switch (m_Rows[GetCurrentRow()]->GetOneSharedSelection())
		{
		case NO_CONNECT:
			SCREENMAN->TextEntry( SM_DoneConnecting, "Enter a Network Address", "", NULL );
			break;
		case NO_DISCONNECT:
			NSMAN->CloseConnection();
			SCREENMAN->SystemMessage("Disconnected from server.");
			break;
		}
		break;
	default:
		ScreenOptions::MenuStart( pn, type );
	}
}

void ScreenNetworkOptions::ImportOptions()
{

}
void ScreenNetworkOptions::ExportOptions()
{

}