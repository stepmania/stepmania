#include "global.h"
#include "NetworkSyncManager.h"

NetworkSyncManager *NSMAN;

#if defined(WITHOUT_NETWORKING)
NetworkSyncManager::NetworkSyncManager() { }
NetworkSyncManager::~NetworkSyncManager () { }
void NetworkSyncManager::CloseConnection() { }
void NetworkSyncManager::PostStartUp( CString ServerIP ) { }
bool NetworkSyncManager::Connect(const CString& addy, unsigned short port) { return false; }
void NetworkSyncManager::ReportScore(int playerID, int step, int score, int combo) { }
void NetworkSyncManager::ReportSongOver() { }
void NetworkSyncManager::StartRequest() { }
void NetworkSyncManager::SendSongs() { }
void NetworkSyncManager::DisplayStartupStatus() { }
void NetworkSyncManager::Update( float fDeltaTime ) { }

#else
#include "ezsockets.h"
#include "ProfileManager.h"
#include "RageLog.h"
#include "StepMania.h"
#include "RageUtil.h"
#include "ThemeManager.h"
#include "Screen.h"
#include "PlayerNumber.h"
#include "GameConstantsAndTypes.h"
#include "SongOptions.h"
#include "ScreenManager.h"
#include "Course.h"
#include "song.h"
#include "SongManager.h"
#include "GameState.h"
#include "StageStats.h"
#include "Steps.h"

NetworkSyncManager::NetworkSyncManager()
{
    NetPlayerClient = new EzSockets;
	m_ServerVersion = 0;
    
	useSMserver = false;
	m_startupStatus = 0;	//By default, connection not tried.

	StartUp();
}

NetworkSyncManager::~NetworkSyncManager ()
{
	//Close Connection to server nicely.
    if (useSMserver)
        NetPlayerClient->close();
	delete NetPlayerClient;
}

void NetworkSyncManager::CloseConnection()
{
	if (!useSMserver)
		return ;
	m_ServerVersion = 0;
   	useSMserver = false;
	m_startupStatus = 0;
	NetPlayerClient->close();
}

void NetworkSyncManager::PostStartUp(CString ServerIP)
{
	CloseConnection();
	if( ServerIP!="LISTEN" )
	{
		if( !Connect(ServerIP.c_str(), 8765) )
		{
			m_startupStatus = 2;
			LOG->Warn( "Network Sync Manager failed to connect" );
			return;
		}

	} else {
		if( !Listen(8765) )
		{
			m_startupStatus = 2;
			LOG->Warn( "Listen() failed" );
			return;
		}

	}

	useSMserver = true;

	m_startupStatus = 1;	//Connection attepmpt sucessful
	LOG->Info("CNConnected!");
	int ClientCommand=3;
	NetPlayerClient->SendPack((char*) &ClientCommand, 4);
	LOG->Info("CNCHECKPOINT");

	NetPlayerClient->ReadData((char*)&m_ServerVersion, 4);
	LOG->Info("CNCHECKPOINT:%d", int(NetPlayerClient->inBuffer.length()));

	// If network play is desired and the connection works,
	// halt until we know what server version we're dealing with
	vector <CString> ProfileNames;
	PROFILEMAN->GetLocalProfileNames(ProfileNames);
	
	netName PlayerName;
	if( ProfileNames.size() > 0 )
	{
		PlayerName.m_packID = 30;
		unsigned i;
		for( i=0; i < strlen(ProfileNames[0]); i++ )
			PlayerName.m_data[i] = ProfileNames[0][i];
		PlayerName.m_data[i] = 0;
		NetPlayerClient->SendPack((char*)&PlayerName, 20);
	}
	if( ProfileNames.size() > 1 )
	{
		PlayerName.m_packID = 31;
		unsigned i=0;
		for( i=0; i < strlen(ProfileNames[1]); i++ )
			PlayerName.m_data[i] = ProfileNames[1][i];
		PlayerName.m_data[i] = 0;
		NetPlayerClient->SendPack( (char*)&PlayerName, 20 );
	}

	LOG->Info("Server Version: %d", m_ServerVersion);
}


void NetworkSyncManager::StartUp()
{
	CString ServerIP;

	if( GetCommandlineArgument( "netip", &ServerIP ) )
		PostStartUp(ServerIP);
	else if( GetCommandlineArgument( "listen" ) )
		PostStartUp("LISTEN");
}


bool NetworkSyncManager::Connect(const CString& addy, unsigned short port)
{
	LOG->Info("Beginning to connect");
	if (port != 8765) 
		return false;
	//Make sure using port 8765
	//This may change in future versions
	//It is this way now for protocol's purpose.
	//If there is a new protocol developed down the road

    NetPlayerClient->create(); // Initilize Socket
    useSMserver = NetPlayerClient->connect(addy, port);
    
	return useSMserver;
}


//Listen (Wait for connection in-bound)
//NOTE: Right now, StepMania cannot connect back to StepMania!

bool NetworkSyncManager::Listen(unsigned short port)
{
	LOG->Info("Beginning to Listen");
	if (port != 8765) 
		return false;
	//Make sure using port 8765
	//This may change in future versions
	//It is this way now for protocol's purpose.
	//If there is a new protocol developed down the road


	EzSockets * EZListener = new EzSockets;

	EZListener->create();
	NetPlayerClient->create(); // Initilize Socket

	EZListener->bind(8765);
    
	useSMserver = EZListener->listen();
	useSMserver = EZListener->accept( *NetPlayerClient );  //Wait for someone to connect

	EZListener->close();	//Kill Listener
	delete EZListener;
    
	//LOG->Info("Accept Responce: ",useSMserver);
	useSMserver=true;
	return useSMserver;
}



void NetworkSyncManager::ReportScore(int playerID, int step, int score, int combo)
{
	if (!useSMserver) //Make sure that we are using the network
		return;

	netHolder SendNetPack;	//Create packet to send to server

	SendNetPack.m_playerID = playerID;
	SendNetPack.m_combo=combo;
	SendNetPack.m_score=score;			//Load packet with appropriate info
	SendNetPack.m_step=step-1;
	
	int CurGrade = g_CurStageStats.GetGrade((PlayerNumber)playerID);

	//Should be cleaned up
 	SendNetPack.m_life=m_playerLife[playerID] + CurGrade*65536;

    //Send packet to server
	NetPlayerClient->SendPack((char*)&SendNetPack, sizeof(netHolder)); 

}
	

void NetworkSyncManager::ReportSongOver() 
{
	if (!useSMserver)	//Make sure that we are using the network
		return ;

	netHolder SendNetPack;	//Create packet to send to server

	SendNetPack.m_playerID = 21; // Song over Packet player ID


	SendNetPack.m_step=0;	
	SendNetPack.m_score=0;
	SendNetPack.m_combo=0;
	SendNetPack.m_life=0;
	

	NetPlayerClient->SendPack((char*)&SendNetPack, sizeof(netHolder)); 
	return;
}

void NetworkSyncManager::StartRequest() 
{
	if (!useSMserver)
		return ;

	//If it's going into demonstration (or jukebox) mode, do not 
	//bother to sync.
	if (GAMESTATE->m_bDemonstrationOrJukebox)
		return ;


	char tmp[4];	//Temporary vector used by receive function when waiting

	LOG->Trace("Requesting Start from Server.");

	netHolder SendNetPack;

	SendNetPack.m_playerID = 20; // Song Start Request Packet player ID

	//Report Step difficulties 

	Steps * tSteps;
	SendNetPack.m_step = 0;
	tSteps = g_CurStageStats.pSteps[PLAYER_1];
	if (tSteps!=NULL)
	{
		SendNetPack.m_step = tSteps->GetMeter();
	}

	tSteps = g_CurStageStats.pSteps[PLAYER_2];
	if (tSteps!=NULL)
	{
		SendNetPack.m_step += tSteps->GetMeter()*256;
	}
	SendNetPack.m_score = 0;
	SendNetPack.m_life = 0;
	SendNetPack.m_combo = 0;

	NetPlayerClient->SendPack((char*)&SendNetPack, sizeof(netHolder)); 
	
	LOG->Trace("Waiting for RECV");

	//Block until go is recieved.
	NetPlayerClient->ReadData((char *)tmp, 4);	

	LOG->Trace("Starting Game.");
}

//This must be executed after NSMAN was started.

//this is for use with SMOnline.
//In order to gaurentee song title and group names
//line up, SM will be started and load, and then 
//send all info to the local client. 
//This information will be then relayed to the 
//SMOnline server.
void NetworkSyncManager::SendSongs()
{	
	if (!useSMserver)
		return ;

	vector <Song *> LogSongs;
	LogSongs = SONGMAN->GetAllSongs();
	unsigned i,j;
	CString SongInfo;
	char toSend[1020];	//Standard buffer is 1024 and 
						//we have to include 4 size bytes
	for (i=0; i<LogSongs.size(); i++)
	{
		Song * CSong = LogSongs[i];
		SongInfo = char(1) + CSong->m_sGroupName;
		SongInfo += char(2) + CSong->GetTranslitMainTitle();
		SongInfo += char(3) + CSong->GetTranslitSubTitle();
		SongInfo += char(4) + CSong->GetTranslitArtist(); 
		
		for (j=4;j<SongInfo.length()+4;j++)
			toSend[j] = SongInfo.c_str()[j-4];
		toSend[0] = char(35);
		toSend[1] = char(0);
		toSend[2] = char(0);
		toSend[3] = char(0);

		NetPlayerClient->SendPack(toSend, SongInfo.length() + 4);
	}
	toSend[0] = char(36);
	NetPlayerClient->SendPack(toSend, 4);

	//Exit because this is ONLY for use with SMOnline
	//If admins feel strongly, this can be exported
	//to another command line, like "--exitfirst"
	exit(0);

	//NOTE TO ADMINS: I do not actually know the 
	//safe way to exit stepmania, if you can, either tell me
	//or do it, please?
}

	
void NetworkSyncManager::DisplayStartupStatus()
{
	CString sMessage("");

	switch (m_startupStatus)
	{
	case 0:
		//Networking wasn't attepmpted
		return;
	case 1:
		sMessage = "Connect to server successful.";
		break;
	case 2:
		sMessage = "Connection failed.";
		break;
	}
	SCREENMAN->SystemMessage(sMessage);
}

void NetworkSyncManager::Update(float fDeltaTime)
{
}
#endif

/*
 * (c) 2003-2004 Charles Lohr, Joshua Allen
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
