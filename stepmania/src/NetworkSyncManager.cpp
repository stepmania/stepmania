
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
    if (argc > 1)
        Connect(argv[1],8765);
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

int NetworkSyncManager::Connect(char * addy, unsigned short port)
{
	if (port!=8765) 
		return -1;
	//Make sure using port 8765
	//This may change in future versions
	//It is this way now for protocol's purpose.
	//If there is a new protocol developed down the road

	//Since under non-lan conditions, the client
	//will be connecting to localhost, this port does not matter.
	

	NetPlayerClient->create();	//Initilize Socket

	if(!NetPlayerClient->connect(addy,port)) {
		useSMserver = false;	//If connection to socket fails, tell
							//other network functions to not do anything
	} else {
		useSMserver = 1;	//Utilize other network funtions
	}
	return 1;
}

void NetworkSyncManager::ReportScore(int playerID, int step, int score, int combo)
{
	if (useSMserver!=1) //Make sure that we are using the network
		return;

	netHolder SendNetPack;	//Create packet to send to server

	SendNetPack.m_playerID = playerID;
	SendNetPack.m_combo=combo;
	SendNetPack.m_score=score;			//Load packet with aproperate info
	SendNetPack.m_step=step-1;

    //Send packet to server
	NetPlayerClient->send((char*)&SendNetPack, sizeof(netHolder)); 
}

void NetworkSyncManager::ReportSongOver() 
{
	if (useSMserver!=1)	//Make sure that we are using the network
		return ;

	netHolder SendNetPack;	//Create packet to send to server

	SendNetPack.m_playerID = 21;	
	SendNetPack.m_combo=0;
	SendNetPack.m_score=0;		//Use PID 21 (Song over Packet)
	SendNetPack.m_step=0;
	
	NetPlayerClient->send((char*)&SendNetPack, sizeof(netHolder)); 
	return;
}

void NetworkSyncManager::StartRequest() 
{
	if (useSMserver!=1)
		return ;

	vector <char> tmp;	//Temporary vector used by receive function when waiting

	LOG->Trace("Requesting Start from Server.");

	netHolder SendNetPack;

	SendNetPack.m_playerID = 20;
	SendNetPack.m_combo=0;
	SendNetPack.m_score=0;	//PID 20 (Song Start Request Packet)
	SendNetPack.m_step=0;

	NetPlayerClient->send((char*)&SendNetPack, sizeof(netHolder));
	
	LOG->Trace("Waiting for RECV");

	//Block until go is recieved.
	NetPlayerClient->receive(tmp);	

	LOG->Trace("Starting Game.");
}

//Global and accessable from anywhere
NetworkSyncManager *NSMAN;
