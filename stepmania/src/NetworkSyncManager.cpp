
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
#include "ThemeManager.h"
#include "ScreenSelectMusic.h"
#include "PlayerNumber.h"
#include "GameConstantsAndTypes.h"
#include "SongOptions.h"
#include "ScreenManager.h"
#include "Course.h"
#include "Song.h"
#include "SongManager.h"
#include "GameState.h"


#define NEXT_SCREEN( play_mode )			THEME->GetMetric ("ScreenSelectMusic","NextScreen"+Capitalize(PlayModeToString(play_mode)))
	//Used for advancing to gameplay screen

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
    else
        return;

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
		PlayerName.m_data[i] = 0;
		NetPlayerClient->send((char*) &PlayerName,20);
	}
	if( ProfileNames.size() > 1 )
	{
		PlayerName.m_packID = 31;
		for( unsigned i=0; i < strlen(ProfileNames[1]); i++ )
			PlayerName.m_data[i] = ProfileNames[1][i];
		PlayerName.m_data[i] = 0;
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

//This must be executed after NSMAN was started.

//this is for use with SMOnline.
//In order to gaurentee song title and group names
//line up, SM will be started and load, and then 
//send all info to the local client. 
//This information will be then relayed to the 
//SMOnline server.
void NetworkSyncManager::SendSongs()
{	
	vector <Song *> LogSongs;
	LogSongs = SONGMAN->GetAllSongs();
	int i,j;
	CString SongInfo;
	char toSend[1020];	//Standard buffer is 1024 and 
						//we have to include 4 size bytes
	for (i=0;i<LogSongs.size();i++)
	{
		Song * CSong = LogSongs[i];
		SongInfo=char(1) + CSong->m_sGroupName;
		SongInfo+=char(2) + CSong->GetTranslitMainTitle();
		SongInfo+=char(3) + CSong->GetTranslitSubTitle();
		SongInfo+=char(4) + CSong->GetTranslitArtist(); 
		
		for (j=4;j<SongInfo.length()+4;j++)
			toSend[j] = SongInfo.c_str()[j-4];
		toSend[0]=char(35);
		toSend[1]=char(0);
		toSend[2]=char(0);
		toSend[3]=char(0);

		LOG->Info("SLen: %d",SongInfo.length());
		NetPlayerClient->send (toSend,SongInfo.length()+4);
	}
	toSend[0]=char(36);
	NetPlayerClient->send (toSend,4);

	//Exit because this is ONLY for use with SMOnline
	//If admins feel strongly, this can be exported
	//to another command line, like "--exitfirst"
	exit (0);

	//NOTE TO ADMINS: I do not actually know the 
	//safe way to exit stepmania, if you can, either tell me
	//or do it, please?
}


//I am adding this for Network support
//Feel free to change the way --course is used
//I am not building any part of SMOnline specific to this yet.

void ArgStartCourse(CString CourseName)
{
	Course * desCourse = SONGMAN->GetCourseFromName(CourseName);
	LOG->Info ("Course Desied: %s/%d",CourseName.c_str(), desCourse);
	if (desCourse == 0)
	{
		LOG->Info ("Desired Course not found!");
		return ;
	}

	LOG->Info("desCourse->GetPlayMode(); = %d",desCourse->GetPlayMode());

	GAMESTATE->m_MasterPlayerNumber = PLAYER_1;
		//Need to add more functionality eventually
	
	GAMESTATE->m_pCurCourse = desCourse;
	GAMESTATE->m_CurStyle = STYLE_DANCE_SINGLE;

	GAMESTATE->m_PlayMode = desCourse->GetPlayMode();
	if (desCourse->IsOni())
		GAMESTATE->m_SongOptions.m_LifeType = SongOptions::LIFE_BATTERY;
	else
		GAMESTATE->m_SongOptions.m_LifeType = SongOptions::LIFE_BAR;

	GAMESTATE->m_SongOptions.m_iBatteryLives = desCourse->m_iLives;

	GAMESTATE->PlayersFinalized();

	//Go to Gameplay Screen
	SCREENMAN->SetNewScreen( NEXT_SCREEN(GAMESTATE->m_PlayMode) );

	//Not sure if this is correct, it may make more sense
	//to have a metric to allow/disallow user options at this point.

	GAMESTATE->BeginGame();
	GAMESTATE->BeginStage();
}




//Global and accessable from anywhere
NetworkSyncManager *NSMAN;

