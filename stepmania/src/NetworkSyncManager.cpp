
/*
-----------------------------------------------------------------------------
 Class: Network Sync Manager

 Desc: See Header.

 Copyright (c) 2003-2004 by the person(s) listed below.  All rights reserved.
	Charles Lohr
	Joshua Allen
-----------------------------------------------------------------------------
*/



/*
-----------------------
NOTE TO DEVS:

This file is still under heavy editing.
Don't bother to edit it.  I will be editing it as I go.
-----------------------
*/



#include "global.h"
#include <string.h>
#include "NetworkSyncManager.h"

#include "BPMDisplay.h"
//for ssprintf
#include "ezsockets.h"
#include "RageLog.h"

NetworkSyncManager::NetworkSyncManager(int argc, char **argv)
{
    NetPlayerClient = new EzSockets;
	m_ServerVersion = 0;

	CString tempargv(argv[1]);

    if (argc > 2) {
		LOG->Info("Multiple arguements were entered.");
		
		if (!tempargv.CompareNoCase("LISTEN"))
			if (Listen(8765)) {
				useSMserver = true;
			} else
				useSMserver = false;
		else
			if (Connect(argv[1],8765) == true)
			{
				useSMserver = true;
			} else
				useSMserver = false;

	}
    else
        useSMserver = false;

	if (useSMserver) {
		int ClientCommand=3;
		NetPlayerClient->send((char*) &ClientCommand, 4);
		NetPlayerClient->send(0);//Send 0 for flash client

		NetPlayerClient->receive(m_ServerVersion);
			//If network play is desired
			//AND the connection works
			//Halt until we know what server 
			//version we're dealing with
		if (((m_ServerVersion / 512) % 2) == 1)
			FlashXMLStyle = true;
		else
			FlashXMLStyle = false;

		LOG->Info("Server Version: %d",m_ServerVersion);
	}

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
	if (FlashXMLStyle)
	{
		netHolderFlash FlashSendPack;	
		FlashSendPack.data[72]='\0';
		int i=0;
		for (i=0;i<72;i++)
			FlashSendPack.data[i] = ' ';
		CString TID;
		FlashSendPack.data[0]='D';
		TID = ssprintf("%d",playerID);
		for (i=0;i<TID.GetLength();i++)
			FlashSendPack.data[i+1] = (TID.c_str())[i];
		
		TID = ssprintf("%d",combo);
		for (i=0;i<TID.GetLength();i++)
			FlashSendPack.data[i+17] = (TID.c_str())[i];

		TID =  ssprintf("%d",score);
		for (i=0;i<TID.GetLength();i++)
			FlashSendPack.data[i+33] = (TID.c_str())[i];

		TID = ssprintf("%d",step-1);
		for (i=0;i<TID.GetLength();i++)
			FlashSendPack.data[i+65] = (TID.c_str())[i];

		TID = ssprintf("%d",m_playerLife[playerID]);
		for (i=0;i<TID.GetLength();i++)
			FlashSendPack.data[i+81] = (TID.c_str())[i];

		NetPlayerClient->send((char*)&FlashSendPack,sizeof(netHolderFlash));

	} else
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
	

	if (FlashXMLStyle)
	{
		netHolderFlash FlashSendPack;	
		FlashSendPack.data[72]='\0';
		for ( int i=0;i<72;i++)
			FlashSendPack.data[i] = ' ';
		FlashSendPack.data[0] = 'E';
		NetPlayerClient->send((char*)&FlashSendPack,sizeof(netHolderFlash));
	} else
		NetPlayerClient->send((char*)&SendNetPack, sizeof(netHolder)); 
	return;
}

void NetworkSyncManager::StartRequest() 
{
	if (!useSMserver)
		return ;

	vector <char> tmp;	//Temporary vector used by receive function when waiting

	LOG->Info("Requesting Start from Server.");

	netHolder SendNetPack;

	SendNetPack.m_playerID = 20; // Song Start Request Packet player ID
	SendNetPack.m_combo=0;
	SendNetPack.m_score=0;
	SendNetPack.m_step=0;
	SendNetPack.m_life=0;

	if (FlashXMLStyle)
	{
		netHolderFlash FlashSendPack;	
		FlashSendPack.data[72]='\0';
		for (int i=0;i<72;i++)
			FlashSendPack.data[i] = ' ';
		FlashSendPack.data[0] = 'S';
		NetPlayerClient->send((char*)&FlashSendPack,sizeof(netHolderFlash));
	} else
		NetPlayerClient->send((char*)&SendNetPack, sizeof(netHolder)); 
	
	LOG->Info("Waiting for RECV");

	//Block until go is recieved.
	NetPlayerClient->receive(tmp);	

	LOG->Info("Starting Game.");
}

//Global and accessable from anywhere
NetworkSyncManager *NSMAN;
