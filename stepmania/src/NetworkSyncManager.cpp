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
void NetworkSyncManager::DisplayStartupStatus() { }
void NetworkSyncManager::Update( float fDeltaTime ) { }

#else
#include "ezsockets.h"
#include "ProfileManager.h"
#include "RageLog.h"
#include "StepMania.h"
#include "ScreenManager.h"
#include "song.h"
#include "GameState.h"
#include "StageStats.h"
#include "Steps.h"

NetworkSyncManager::NetworkSyncManager()
{
    NetPlayerClient = new EzSockets;
	NetPlayerClient->blocking = false;
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

	// If network play is desired and the connection works,
	// halt until we know what server version we're dealing with

	ClearPacket(m_packet);

	Write1(m_packet,2);	//Hello Packet

	Write1(m_packet,NETPROTOCOLVERSION);
	int ctr = 2 * 16 + 0;
	Write1(m_packet,ctr);

	vector <CString> ProfileNames;
	PROFILEMAN->GetLocalProfileNames(ProfileNames);
	
	if( ProfileNames.size() > 0 )
		WriteNT(m_packet,ProfileNames[0]);
	if( ProfileNames.size() > 1 )
		WriteNT(m_packet,ProfileNames[0]);
	
	NetPlayerClient->SendPack((char*)m_packet.Data,m_packet.Position);

	ClearPacket(m_packet);

	//Block until responce is received
	//Move mode to blocking in order to give CPU back to the 
	//system, and not wait.
	
	NetPlayerClient->blocking=true;
	NetPlayerClient->ReadPack((char*)m_packet.Data,NETMAXBUFFERSIZE);
	NetPlayerClient->blocking=false;

	//int command = Read1(m_packet);
	Read1(m_packet);
	m_ServerVersion = Read1(m_packet);
	m_ServerName = ReadNT(m_packet);

	LOG->Info("Server Version: %d %s", m_ServerVersion, m_ServerName.c_str());
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
	
	ClearPacket(m_packet);

	Write1(m_packet,5);
	unsigned char ctr = playerID * 16 + step - 1;
	Write1(m_packet,ctr);

	ctr = g_CurStageStats.GetGrade((PlayerNumber)playerID)*16;

	Write1(m_packet,ctr);

	Write4(m_packet,score);

	Write2(m_packet,combo);

	Write2(m_packet,m_playerLife[playerID]);

	NetPlayerClient->SendPack((char*)m_packet.Data, m_packet.Position); 

}
	

void NetworkSyncManager::ReportSongOver() 
{
	if (!useSMserver)	//Make sure that we are using the network
		return ;

	ClearPacket(m_packet);

	Write1(m_packet,4);

	NetPlayerClient->SendPack((char*)&m_packet.Data, m_packet.Position); 
	return;
}

void NetworkSyncManager::StartRequest() 
{
	if (!useSMserver)
		return ;

	if (GAMESTATE->m_bDemonstrationOrJukebox)
		return ;

	LOG->Trace("Requesting Start from Server.");

	ClearPacket(m_packet);

	Write1(m_packet,3);	

	unsigned char ctr=0;

	Steps * tSteps;
	tSteps = g_CurStageStats.pSteps[PLAYER_1];
	if (tSteps!=NULL)
		ctr = ctr+tSteps->GetMeter()*16;

	tSteps = g_CurStageStats.pSteps[PLAYER_2];
	if (tSteps!=NULL)
		ctr = ctr+tSteps->GetMeter();

	Write1(m_packet,ctr);

	WriteNT(m_packet,GAMESTATE->m_pCurSong->m_sMainTitle);
	WriteNT(m_packet,GAMESTATE->m_pCurSong->m_sSubTitle);
	WriteNT(m_packet,GAMESTATE->m_pCurSong->m_sArtist);

	NetPlayerClient->SendPack((char*)&m_packet.Data, m_packet.Position); 
	
	LOG->Trace("Waiting for RECV");

	ClearPacket(m_packet);
	//Block until go is recieved.
	//Switch to blocking mode (this is the only
	//way I know how to get precievably instantanious results
	NetPlayerClient->blocking=true;
	NetPlayerClient->ReadPack((char *)&m_packet, NETMAXBUFFERSIZE);
	NetPlayerClient->blocking=false;

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
		sMessage = "Connection to " + m_ServerName + " sucessful.";
		break;
	case 2:
		sMessage = "Connection failed.";
		break;
	}
	SCREENMAN->SystemMessage(sMessage);
}

void NetworkSyncManager::Update(float fDeltaTime)
{
	ProcessInput();
}

void NetworkSyncManager::ProcessInput()
{
	//If we're disconnected, just exit
	if (NetPlayerClient->state!=NetPlayerClient->skCONNECTED)
		return;

	//load new data into buffer

	NetPlayerClient->update();

	ClearPacket(m_packet);

	while (NetPlayerClient->ReadPack((char *)&m_packet, NETMAXBUFFERSIZE)>0)
	{
		int command = Read1(m_packet);
		LOG->Trace("Received command from server:%d",command-128);

		//Check to make sure command is valid from server
		if (command<128)
			break;

		command+=-128;

		switch (command) {
		case 0: //Ping packet responce
			ClearPacket(m_packet);
			Write1(m_packet,0);
			NetPlayerClient->SendPack((char*)m_packet.Data,m_packet.Position);
			break;
		case 1:	//These are in responce to when/if we send packet 0's
		case 2: //This is already taken care of by the blocking code earlier on
		case 3: //This is taken care of by the blocking start code
		case 4: //Undefined
		case 5: //Undefined
			break;
		case 6:	//System message from server
			CString SysMSG = ReadNT(m_packet);
			SCREENMAN->SystemMessage(SysMSG);
			break;
		}
	}
}



unsigned char NetworkSyncManager::Read1(NetPacket &Packet)
{
	if (Packet.Position>=NETMAXBUFFERSIZE)
		return 0;

	return Packet.Data[Packet.Position++];
}

unsigned short int NetworkSyncManager::Read2(NetPacket &Packet)
{
	if (Packet.Position>=NETMAXBUFFERSIZE-1)
		return 0;

	unsigned short int Temp;
	memcpy((void *)&Temp,(void *)*(&Packet.Data + Packet.Position),2);
	Packet.Position+=2;
	return ntohs(Temp);
}

unsigned int NetworkSyncManager::Read4(NetPacket &Packet)
{
	if (Packet.Position>=NETMAXBUFFERSIZE-3)
		return 0;

	unsigned int Temp;
	memcpy((void *)&Temp,(void *)(&Packet.Data + Packet.Position),4);
	Packet.Position+=2;
	return ntohl(Temp);
}

CString NetworkSyncManager::ReadNT(NetPacket &Packet)
{
	//int Orig=Packet.Position;
	CString TempStr;
	while ((Packet.Position<NETMAXBUFFERSIZE)&& (((char*)Packet.Data)[Packet.Position]!=0))
		TempStr= TempStr + (char)Packet.Data[Packet.Position++];

	Packet.Position++;
	return TempStr;
}


void NetworkSyncManager::Write1(NetPacket &Packet, unsigned char Data)
{
	if (Packet.Position>=NETMAXBUFFERSIZE)
		return;
	memcpy((void*)(&Packet.Data[Packet.Position]),(void *)&Data,1);	
	Packet.Position++;
}

void NetworkSyncManager::Write2(NetPacket &Packet, unsigned short int Data)
{
	if (Packet.Position>=NETMAXBUFFERSIZE-1)
		return;
	Data = htons(Data);
	memcpy((void*)(&Packet.Data[Packet.Position]),(void *)&Data,2);	
	Packet.Position+=2;
}

void NetworkSyncManager::Write4(NetPacket &Packet, unsigned long Data)
{
	if (Packet.Position>=NETMAXBUFFERSIZE-3)
		return ;

	Data = htonl(Data);
	memcpy((void*)(&Packet.Data[Packet.Position]),(void *)&Data,4);	
	Packet.Position+=4;
}

void NetworkSyncManager::WriteNT(NetPacket &Packet, CString Data)
{
	int index=0;
	while ((Packet.Position<NETMAXBUFFERSIZE)&&(index<Data.GetLength()))
		Packet.Data[Packet.Position++] = (unsigned char)(Data.c_str()[index++]);
			//Is it proper to do "(unsigned char)(Data.c_str()[index++])"?
	Packet.Data[Packet.Position++] = 0;
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
