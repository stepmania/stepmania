
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
#include "NetworkSyncManager.h"
#include "ezsockets.h"
#include "RageLog.h"

NetworkSyncManager::NetworkSyncManager(int argc, char **argv)
{
    NetPlayerClient = new EzSockets;
	m_ServerVersion = 0;

    if (argc > 2)
        if (Connect(argv[1],8765) == true)
		{
			useSMserver = true;
			int ClientCommand=3;
			NetPlayerClient->send((char*) &ClientCommand, 4);
			NetPlayerClient->send(0);//Send 0 for flash client

			NetPlayerClient->receive(m_ServerVersion);
				//If network play is desired
				//AND the connection works
				//Halt until we know what server 
				//version we're dealing with
			if (((m_ServerVersion / 512) % 2) == 1)
				appendWithZero = true;
			else
				appendWithZero = false;
		} else
			useSMserver = false;
    else
        useSMserver = false;
}

NetworkSyncManager::~NetworkSyncManager ()
{
	//Close Connection to server nicely.
    if (useSMserver)
        NetPlayerClient->close();
	delete NetPlayerClient;
}

bool NetworkSyncManager::Connect(const CString& addy, unsigned short port)
{
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

void NetworkSyncManager::ReportScore(int playerID, int step, int score, int combo)
{
	if (!useSMserver) //Make sure that we are using the network
		return;

	netHolder SendNetPack;	//Create packet to send to server

	SendNetPack.m_playerID = playerID;
	SendNetPack.m_combo=combo;
	SendNetPack.m_score=score;			//Load packet with aproperate info
	SendNetPack.m_step=step-1;
	SendNetPack.m_life=m_playerLife[playerID];

    //Send packet to server
	NetPlayerClient->send((char*)&SendNetPack, sizeof(netHolder)); 

	if (appendWithZero)
		NetPlayerClient->send(0);
}

void NetworkSyncManager::ReportSongOver() 
{
	if (!useSMserver)	//Make sure that we are using the network
		return ;

	netHolder SendNetPack;	//Create packet to send to server

	SendNetPack.m_playerID = 21;	
	SendNetPack.m_combo=0;
	SendNetPack.m_score=0;		//Use PID 21 (Song over Packet)
	SendNetPack.m_step=0;
	SendNetPack.m_life=0;
	
	NetPlayerClient->send((char*)&SendNetPack, sizeof(netHolder)); 

	if (appendWithZero)
		NetPlayerClient->send(0);
	
	return;
}

void NetworkSyncManager::StartRequest() 
{
	if (!useSMserver)
		return ;

	vector <char> tmp;	//Temporary vector used by receive function when waiting

	LOG->Trace("Requesting Start from Server.");

	netHolder SendNetPack;

	SendNetPack.m_playerID = 20;
	SendNetPack.m_combo=0;
	SendNetPack.m_score=0;	//PID 20 (Song Start Request Packet)
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
