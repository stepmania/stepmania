#include "global.h"
#include "NetworkSyncManager.h"

NetworkSyncManager *NSMAN;

#if defined(WITHOUT_NETWORKING)
NetworkSyncManager::NetworkSyncManager() { useSMServer=false }
NetworkSyncManager::~NetworkSyncManager () { }
void NetworkSyncManager::CloseConnection() { }
void NetworkSyncManager::PostStartUp( CString ServerIP ) { }
bool NetworkSyncManager::Connect(const CString& addy, unsigned short port) { return false; }
void NetworkSyncManager::ReportTiming(float offset, int PlayerNumber) { }
void NetworkSyncManager::ReportScore(int playerID, int step, int score, int combo) { }
void NetworkSyncManager::ReportSongOver() { }
void NetworkSyncManager::StartRequest(short position) { }
void NetworkSyncManager::DisplayStartupStatus() { }
void NetworkSyncManager::Update( float fDeltaTime ) { }
bool NetworkSyncManager::ChangedScoreboard() {}
#else
#include "ezsockets.h"
#include "ProfileManager.h"
#include "RageLog.h"
#include "StepMania.h"
#include "ScreenManager.h"
#include "song.h"
#include "Course.h"
#include "GameState.h"
#include "StageStats.h"
#include "Steps.h"
#include "PrefsManager.h"

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
	Write1(m_packet, (uint8_t) ctr);

	vector <CString> profileNames;
	PROFILEMAN->GetLocalProfileNames(profileNames);

	FOREACH_PlayerNumber(pn)
	{
		int localID=atoi(PREFSMAN->m_sDefaultLocalProfileID[pn])-1;
		if ((localID>=0)&&(localID<profileNames.size()))
			WriteNT(m_packet,profileNames[localID]);
		else
			WriteNT(m_packet,"[No Profile Set]");
	}

	NetPlayerClient->SendPack((char*)m_packet.Data,m_packet.Position);

	ClearPacket(m_packet);

	//Block until responce is received
	//Move mode to blocking in order to give CPU back to the 
	//system, and not wait.
	
	bool dontExit=true;
	NetPlayerClient->blocking=true;
	while (dontExit)
	{
		ClearPacket(m_packet);
		if (NetPlayerClient->ReadPack((char *)&m_packet, NETMAXBUFFERSIZE)<1)
			dontExit=false; // Also allow exit if there is a problem on the socket
		if (Read1(m_packet) == (128+2))
			dontExit=false;
		//Only allow passing on handshake. 
		//Otherwise scoreboard updates and such will confuse us.
	}
	NetPlayerClient->blocking=false;
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


void NetworkSyncManager::ReportTiming(float offset, int PlayerNumber)
{
	m_lastOffset[PlayerNumber] = offset;
}

void NetworkSyncManager::ReportScore(int playerID, int step, int score, int combo)
{
	if (!useSMserver) //Make sure that we are using the network
		return;
	
	ClearPacket(m_packet);

	Write1(m_packet,5);
	uint8_t ctr = (uint8_t) (playerID * 16 + step - 1);
	Write1(m_packet,ctr);

	ctr = uint8_t( g_CurStageStats.GetGrade((PlayerNumber)playerID)*16 );

	Write1(m_packet,ctr);

	Write4(m_packet,score);

	Write2(m_packet, (uint16_t) combo);

	Write2(m_packet, (uint16_t) m_playerLife[playerID]);

	//Offset Info
	//Note: if a 0 is sent, then disregard data.
	//
	//ASSUMED: No step will be more than 16 seconds off center
	//If assumption false: read 16 seconds either direction
	int iOffset = int((m_lastOffset[playerID]+16.384)*2000.0);

	if (iOffset>65535)
		iOffset=65535;
	if (iOffset<1)
		iOffset=1;

	//Report 0 if hold, or miss (don't forget mines should report)
	if (((step<TNS_BOO)||(step>TNS_MARVELOUS))&&(step!=TNS_HIT_MINE))
		iOffset = 0;

	Write2(m_packet, (uint16_t) iOffset);

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

void NetworkSyncManager::StartRequest(short position) 
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
		ctr = uint8_t(ctr+tSteps->GetMeter()*16);

	tSteps = g_CurStageStats.pSteps[PLAYER_2];
	if (tSteps!=NULL)
		ctr = uint8_t(ctr+tSteps->GetMeter());

	Write1(m_packet,ctr);
	
	//Notify server if this is for sync or not.
	ctr = char(position*16);
	Write1(m_packet,ctr);

	if (GAMESTATE->m_pCurSong !=NULL) {
		WriteNT(m_packet,GAMESTATE->m_pCurSong->m_sMainTitle);
		WriteNT(m_packet,GAMESTATE->m_pCurSong->m_sSubTitle);
		WriteNT(m_packet,GAMESTATE->m_pCurSong->m_sArtist);
	} else {
		WriteNT(m_packet,"");
		WriteNT(m_packet,"");
		WriteNT(m_packet,"");
	}

	if (GAMESTATE->m_pCurCourse != NULL)
		WriteNT(m_packet,GAMESTATE->m_pCurCourse->GetFullDisplayTitle());
	else
		WriteNT(m_packet,CString(""));

	NetPlayerClient->SendPack((char*)&m_packet.Data, m_packet.Position); 
	
	LOG->Trace("Waiting for RECV");

	ClearPacket(m_packet);
	//Block until go is recieved.
	//Switch to blocking mode (this is the only
	//way I know how to get precievably instantanious results

	bool dontExit=true;
	NetPlayerClient->blocking=true;
	while (dontExit)
	{
		ClearPacket(m_packet);
		if (NetPlayerClient->ReadPack((char *)&m_packet, NETMAXBUFFERSIZE)<1)
			dontExit=false; // Also allow exit if there is a problem on the socket
		if (Read1(m_packet) == (128+3))
			dontExit=false;
		//Only allow passing on Start request. 
		//Otherwise scoreboard updates and such will confuse us.
	}
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
	if (useSMserver)
		ProcessInput();
}

void NetworkSyncManager::ProcessInput()
{
	//If we're disconnected, just exit
	if ((NetPlayerClient->state!=NetPlayerClient->skCONNECTED) || 
			NetPlayerClient->IsError())
	{
		SCREENMAN->SystemMessageNoAnimate("Connection to server dropped.");
		useSMserver=false;
		return;
	}

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
		case 5: //Scoreboard Update
			{
				int ColumnNumber=Read1(m_packet);
				m_Scoreboard[ColumnNumber] = ReadNT(m_packet);
				m_scoreboardchange[ColumnNumber]=true;
			}
			break;
		case 6:	//System message from server
			CString SysMSG = ReadNT(m_packet);
			SCREENMAN->SystemMessage(SysMSG);
			break;
		}
	}
}

bool NetworkSyncManager::ChangedScoreboard(int Column) 
{
	if (!m_scoreboardchange[Column])
		return false;
	m_scoreboardchange[Column]=false;
	return true;
}


uint8_t NetworkSyncManager::Read1(NetPacket &Packet)
{
	if (Packet.Position>=NETMAXBUFFERSIZE)
		return 0;

	return Packet.Data[Packet.Position++];
}

uint16_t NetworkSyncManager::Read2(NetPacket &Packet)
{
	if (Packet.Position>=NETMAXBUFFERSIZE-1)
		return 0;

	uint16_t Temp;
	memcpy( &Temp, *(&Packet.Data + Packet.Position),2 );
	Packet.Position+=2;
	return Swap16BE(Temp);
}

uint32_t NetworkSyncManager::Read4(NetPacket &Packet)
{
	if (Packet.Position>=NETMAXBUFFERSIZE-3)
		return 0;

	uint32_t Temp;
	memcpy( &Temp, (&Packet.Data + Packet.Position),4 );
	Packet.Position+=2;
	return Swap32BE(Temp);
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


void NetworkSyncManager::Write1(NetPacket &Packet, uint8_t Data)
{
	if (Packet.Position>=NETMAXBUFFERSIZE)
		return;
	memcpy( &Packet.Data[Packet.Position], &Data,1 );
	Packet.Position++;
}

void NetworkSyncManager::Write2(NetPacket &Packet, uint16_t Data)
{
	if (Packet.Position>=NETMAXBUFFERSIZE-1)
		return;
	Data = Swap16BE(Data);
	memcpy( &Packet.Data[Packet.Position], &Data,2 );
	Packet.Position+=2;
}

void NetworkSyncManager::Write4(NetPacket &Packet, uint32_t Data)
{
	if (Packet.Position>=NETMAXBUFFERSIZE-3)
		return ;

	Data = Swap32BE(Data);
	memcpy( &Packet.Data[Packet.Position], &Data,4 );
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
