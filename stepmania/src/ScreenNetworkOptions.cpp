#include "global.h"

#if !defined(WITHOUT_NETWORKING)
#include "NetworkSyncManager.h"
#include "ScreenNetworkOptions.h"
#include "RageLog.h"
#include "GameSoundManager.h"
#include "ThemeManager.h"
#include "PrefsManager.h"
#include "ScreenManager.h"
#include "ScreenTextEntry.h"
#include "ScreenPrompt.h"
#include "NetworkSyncServer.h"
#include "VirtualKeyboard.h"

enum {
	PO_CONNECTION,
	PO_SERVER,
	NUM_NETWORK_OPTIONS_LINES
};

enum {
	NO_STOP_SERVER=0,
	NO_START_SERVER
};

OptionRowDefinition g_NetworkOptionsLines[NUM_NETWORK_OPTIONS_LINES] = {
	OptionRowDefinition( "Connection",	true, "PRESS START" ),
	OptionRowDefinition( "Server",		true, "PRESS START" )
};

const ScreenMessage	SM_DoneConnecting		= ScreenMessage(SM_User+1);
const ScreenMessage	SM_ServerNameEnter		= ScreenMessage(SM_User+2);

static Preference<CString> g_sLastServer( Options, "LastConnectedServer",	"" );

REGISTER_SCREEN_CLASS( ScreenNetworkOptions );
ScreenNetworkOptions::ScreenNetworkOptions( CString sClassName ) : ScreenOptions( sClassName )
{
	LOG->Trace( "ScreenNetworkOptions::ScreenNetworkOptions()" );

	m_sClassName = sClassName;
}

void ScreenNetworkOptions::Init()
{
	ScreenOptions::Init();

	g_NetworkOptionsLines[PO_CONNECTION].choices.clear();
	if ( NSMAN->useSMserver )
		g_NetworkOptionsLines[PO_CONNECTION].choices.push_back("Disconnect from "+NSMAN->GetServerName());
	else
		g_NetworkOptionsLines[PO_CONNECTION].choices.push_back("Connect...");

	g_NetworkOptionsLines[PO_SERVER].choices.clear();
	g_NetworkOptionsLines[PO_SERVER].choices.push_back("Stop");
	g_NetworkOptionsLines[PO_SERVER].choices.push_back("Start...");
	
	InitMenu( 
		INPUTMODE_SHARE_CURSOR, 
		g_NetworkOptionsLines, 
		NUM_NETWORK_OPTIONS_LINES );

	SOUND->PlayMusic( THEME->GetPathS("ScreenMachineOptions","music") );
}


void ScreenNetworkOptions::GoToPrevScreen()
{
	SCREENMAN->SetNewScreen( "ScreenOptionsMenu" );
}

void ScreenNetworkOptions::GoToNextScreen()
{
	GoToPrevScreen();
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
			UpdateConnectStatus( );
			g_sLastServer = ScreenTextEntry::s_sLastAnswer;
		}
		break;
	case SM_ServerNameEnter:
		if( !ScreenTextEntry::s_bCancelledLast )
		{
			if ( NSMAN->LANserver == NULL)
				NSMAN->LANserver = new StepManiaLanServer;
			NSMAN->LANserver->servername = ScreenTextEntry::s_sLastAnswer;
			if (NSMAN->LANserver->ServerStart())
			{
				NSMAN->isLanServer = true;
				SCREENMAN->SystemMessage( "Server Started." );
			}
			else
				SCREENMAN->SystemMessage( "Server failed: " + NSMAN->LANserver->lastError + ssprintf(" Code:%d",NSMAN->LANserver->lastErrorCode) );
		}
	}

	ScreenOptions::HandleScreenMessage( SM );
}


void ScreenNetworkOptions::MenuStart( PlayerNumber pn, const InputEventType type )
{
#if defined( WITHOUT_NETWORKING )
#else
	switch( GetCurrentRow() )
	{
	case PO_CONNECTION:
		if ( !NSMAN->useSMserver )
		{
			VIRTUALKB.Reset(VKMODE_IP);
			SCREENMAN->TextEntry( SM_DoneConnecting, "Enter a Network Address\n127.0.0.1 to connect to yourself", g_sLastServer );
		}
		else {
			NSMAN->CloseConnection();
			SCREENMAN->SystemMessage("Disconnected from server.");
			UpdateConnectStatus( );
		}
		break;
	case PO_SERVER:
		switch (m_Rows[GetCurrentRow()]->GetOneSharedSelection())
		{
		case NO_START_SERVER:
			if (!NSMAN->isLanServer)
			{
				VIRTUALKB.Reset(VKMODE_PROFILE);
				SCREENMAN->TextEntry( SM_ServerNameEnter, "Enter a server name...", "", NULL );
			}
			break;
		case NO_STOP_SERVER:
			if ( NSMAN->LANserver != NULL )
				NSMAN->LANserver->ServerStop();
			SCREENMAN->SystemMessage( "Server Stopped." );
			NSMAN->isLanServer = false;
			break;
		}
		break;
	default:
		ScreenOptions::MenuStart( pn, type );
	}
#endif
}

void ScreenNetworkOptions::ImportOptions( int row ) { }
void ScreenNetworkOptions::ExportOptions( int row ) { }

void ScreenNetworkOptions::UpdateConnectStatus( )
{
	SCREENMAN->SetNewScreen( m_sClassName );
}
#endif

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
