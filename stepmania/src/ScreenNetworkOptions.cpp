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

AutoScreenMessage( SM_DoneConnecting )
AutoScreenMessage( SM_ServerNameEnter )

static Preference<CString> g_sLastServer( "LastConnectedServer",	"" );

REGISTER_SCREEN_CLASS( ScreenNetworkOptions );
ScreenNetworkOptions::ScreenNetworkOptions( CString sClassName ) : ScreenOptions( sClassName )
{
	LOG->Trace( "ScreenNetworkOptions::ScreenNetworkOptions()" );

	m_sClassName = sClassName;
}

void ScreenNetworkOptions::Init()
{
	ScreenOptions::Init();

	g_NetworkOptionsLines[PO_CONNECTION].m_vsChoices.clear();
	if ( NSMAN->useSMserver )
		g_NetworkOptionsLines[PO_CONNECTION].m_vsChoices.push_back("Disconnect from "+NSMAN->GetServerName());
	else
		g_NetworkOptionsLines[PO_CONNECTION].m_vsChoices.push_back("Connect...");

	g_NetworkOptionsLines[PO_SERVER].m_vsChoices.clear();
	g_NetworkOptionsLines[PO_SERVER].m_vsChoices.push_back("Stop");
	g_NetworkOptionsLines[PO_SERVER].m_vsChoices.push_back("Start...");
	
	//Enable all lines for all players
	for ( unsigned int i = 0; i < NUM_NETWORK_OPTIONS_LINES; i++ )
		FOREACH_PlayerNumber( pn )
			g_NetworkOptionsLines[i].m_vEnabledForPlayers.insert( pn );

	vector<OptionRowDefinition> vDefs( &g_NetworkOptionsLines[0], &g_NetworkOptionsLines[ARRAYSIZE(g_NetworkOptionsLines)] );
	vector<OptionRowHandler*> vHands( vDefs.size(), NULL );

	InitMenu( INPUTMODE_SHARE_CURSOR, vDefs, vHands );

	SOUND->PlayMusic( THEME->GetPathS("ScreenMachineOptions","music") );
}


void ScreenNetworkOptions::HandleScreenMessage( const ScreenMessage SM )
{
	if( SM == SM_DoneConnecting )
	{
		if( !ScreenTextEntry::s_bCancelledLast )
		{
			CString sNewName = ScreenTextEntry::s_sLastAnswer;
			NSMAN->PostStartUp(sNewName);
			NSMAN->DisplayStartupStatus();
			UpdateConnectStatus( );
			g_sLastServer.Set( ScreenTextEntry::s_sLastAnswer );
		}
	}
	else if( SM == SM_ServerNameEnter )
	{
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
	else if( SM == SM_GoToNextScreen )
	{
		SCREENMAN->SetNewScreen( "ScreenOptionsMenu" );
		return;
	}
	else if( SM == SM_GoToPrevScreen )
	{
		SCREENMAN->SetNewScreen( "ScreenOptionsMenu" );
		return;
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
			ScreenTextEntry::TextEntry( SM_DoneConnecting, "Enter a Network Address\n127.0.0.1 to connect to yourself", g_sLastServer, 128 );
		}
		else
		{
			NSMAN->CloseConnection();
			SCREENMAN->SystemMessage("Disconnected from server.");
			UpdateConnectStatus( );
		}
		break;
	case PO_SERVER:
		switch( m_pRows[GetCurrentRow()]->GetOneSharedSelection() )
		{
		case NO_START_SERVER:
			if (!NSMAN->isLanServer)
			{
				ScreenTextEntry::TextEntry( SM_ServerNameEnter, "Enter a server name...", "", 0);
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

void ScreenNetworkOptions::ImportOptions( int row, const vector<PlayerNumber> &vpns ) { }
void ScreenNetworkOptions::ExportOptions( int row, const vector<PlayerNumber> &vpns ) { }

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
