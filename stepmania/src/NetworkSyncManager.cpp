
/*
-----------------------------------------------------------------------------
 Class: Network Sync Manager

 Desc: See Header.

 Copyright (c) 2003-2004 by the person(s) listed below.  All rights reserved.
	Charles Lohr
	Joshua Allen
-----------------------------------------------------------------------------
*/

#include "global.h"
#include <cstring>
#include "NetworkSyncManager.h"
#include "ProfileManager.h"
#include "ezsockets.h"
#include "RageLog.h"
#include "StepMania.h"
#include "RageUtil.h"


NetworkSyncManager::NetworkSyncManager()
{
    NetPlayerClient = new EzSockets;
	m_ServerVersion = 0;
    
	useSMserver = false;

	StartUp();
}

NetworkSyncManager::~NetworkSyncManager ()
{
	//Close Connection to server nicely.
    if (useSMserver)
        NetPlayerClient->close();
	delete NetPlayerClient;
}

void NetworkSyncManager::StartUp()
{
	CString ServerIP;
	if( GetCommandlineArgument( "netip", &ServerIP ) )
	{
		if( !Connect(ServerIP.c_str(),8765) )
		{
			LOG->Warn( "Network Sync Manager failed to connect" );
			return;
		}

	}
	else if( GetCommandlineArgument( "listen" ) )
	{
		if( !Listen(8765) )
		{
			LOG->Warn( "Listen() failed");
			return;
		}

	}

	useSMserver = true;

	int ClientCommand=3;
	NetPlayerClient->send((char*) &ClientCommand, 4);

	NetPlayerClient->receive(m_ServerVersion);
	// If network play is desired and the connection works,
	// halt until we know what server version we're dealing with
	vector <CString> ProfileNames;
	PROFILEMAN->GetLocalProfileNames(ProfileNames);
	
	netName PlayerName;
	if( ProfileNames.size() > 0 )
	{
		PlayerName.m_packID = 30;
		for( unsigned i=0; i < strlen(ProfileNames[0]); i++ )
			PlayerName.m_data[i] = ProfileNames[0][i];
		NetPlayerClient->send((char*) &PlayerName,20);
	}
	if( ProfileNames.size() > 1 )
	{
		PlayerName.m_packID = 31;
		for( unsigned i=0; i < strlen(ProfileNames[1]); i++ )
			PlayerName.m_data[i] = ProfileNames[1][i];
		NetPlayerClient->send( (char*) &PlayerName,20 );
	}

	LOG->Info("Server Version: %d",m_ServerVersion);
}


bool NetworkSyncManager::Connect(const CString& addy, unsigned short port)
{
	LOG->Info("Beginning to connect");
	if (port!=8765) 
		return false;
	//Make sure using port 8765
	//This may change in future versions
	//It is this way now for protocol's purpose.
	//If there is a new protocol developed down the road

	//Since under non-lan conditions, the client
	//will be connecting to localhost, this port does not matter.
    /* What do you mean it doesn't matter if you are connecting to localhost?
     * Also, when does it ever connect to localhost?
     * -- Steve
     */
    NetPlayerClient->create(); // Initilize Socket
    useSMserver = NetPlayerClient->connect(addy, port);
    
	return useSMserver;
}


//Listen (Wait for connection in-bound)
//NOTE: Right now, StepMania cannot connect back to StepMania!

bool NetworkSyncManager::Listen(unsigned short port)
{
	LOG->Info("Beginning to Listen");
	if (port!=8765) 
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
	useSMserver = EZListener->accept( *NetPlayerClient);  //Wait for someone to connect

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
	SendNetPack.m_life=m_playerLife[playerID];

    //Send packet to server
	NetPlayerClient->send((char*)&SendNetPack, sizeof(netHolder)); 

}
	

void NetworkSyncManager::ReportSongOver() 
{
	if (!useSMserver)	//Make sure that we are using the network
		return ;

	netHolder SendNetPack;	//Create packet to send to server

	SendNetPack.m_playerID = 21; // Song over Packet player ID
	SendNetPack.m_combo=0;
	SendNetPack.m_score=0;
	SendNetPack.m_step=0;
	SendNetPack.m_life=0;
	

	NetPlayerClient->send((char*)&SendNetPack, sizeof(netHolder)); 
	return;
}

void NetworkSyncManager::StartRequest() 
{
	if (!useSMserver)
		return ;

	vector <char> tmp;	//Temporary vector used by receive function when waiting

	LOG->Trace("Requesting Start from Server.");

	netHolder SendNetPack;

	SendNetPack.m_playerID = 20; // Song Start Request Packet player ID
	SendNetPack.m_combo=0;
	SendNetPack.m_score=0;
	SendNetPack.m_step=0;
	SendNetPack.m_life=0;

	NetPlayerClient->send((char*)&SendNetPack, sizeof(netHolder)); 
	
	LOG->Trace("Waiting for RECV");

	//Block until go is recieved.
	NetPlayerClient->receive(tmp);	

	LOG->Trace("Starting Game.");
}

//Global and accessable from anywhere
NetworkSyncManager *NSMAN;
