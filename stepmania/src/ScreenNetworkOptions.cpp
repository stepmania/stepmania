#include "global.h"
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

/*
 * (c) 2004 Charles Lohr
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
