#include "global.h"
#include "NetworkSyncServer.h"
#include "RageLog.h"
#include "PrefsManager.h"
#include <time.h>

#if defined(WITHOUT_NETWORKING)
bool StepManiaLanServer::ServerStart() { return false; }
void StepManiaLanServer::ServerStop() { }
void StepManiaLanServer::ServerUpdate() { }
StepManiaLanServer::StepManiaLanServer() { }
StepManiaLanServer::~StepManiaLanServer() { }
bool StepManiaLanServer::IsBanned(in_addr &ip) {}
#else

LanPlayer::LanPlayer()
{
	score = 0;
	health = 0;
	feet = 0;
	projgrade = 0;
	combo = 0;
	currstep = 0;
	maxCombo = 0;
	Grade = 0;
	offset = 0;
	options = "";
}

StepManiaLanServer::StepManiaLanServer()
{
	stop = true;
	SecondSameSelect = false;
	AssignPlayerIDs();
}

StepManiaLanServer::~StepManiaLanServer()
{
	ServerStop();
}

bool StepManiaLanServer::ServerStart()
{
	server.blocking = 0; /* Turn off blocking */
	if (server.create())
		if (server.bind(8765))
			if (server.listen())
			{
				stop = false;
				statsTime = time(NULL);
				return true;
			}
			else
				lastError = "Failed to make socket listen.";
		else
			lastError = "Failed to bind socket";
	else
		lastError = "Failed to create socket";

	lastErrorCode = server.lastCode;
	//Hopefully we will not get here. If we did, something went wrong above.
	return false;
}

void StepManiaLanServer::ServerStop()
{
	for (int x = 0; x < NUMBERCLIENTS; ++x)
	{
		Client[x].clientSocket.close();
		Client[x].Used = false;
	}
	server.close();
	stop = true;
}

void StepManiaLanServer::ServerUpdate()
{
	if (!stop)
	{
		NewClientCheck(); /* See if there is another client wanting to play */
		UpdateClients();
		if (time(NULL) > statsTime)
		{
			SendStatsToClients();
			statsTime = time(NULL);
		}
	}
}

void StepManiaLanServer::UpdateClients()
{
	//Go through all the clients and check to see if it is being used.
	//If so then try to get a backet and parse the data.
	for (int x = 0; x < NUMBERCLIENTS; ++x)
		if (Client[x].Used == 1)
			if (Client[x].GetData(Packet) >= 0)
				ParseData(Packet, x);
}

GameClient::GameClient()
{
	Used = 0;
	GotStartRequest = 0;
	clientSocket.blocking = 0;
	twoPlayers = false;
	version = 0;
	startPosition = 0;
	InGame = 0;
	hasSong = forceHas = false;
	inNetMusicSelect = false;
	isStarting = false;  //Used for after ScreenNetMusicSelect but before InGame
	wasIngame = false;
	hasCheated = false;
}

void GameClient::Disconnect()
{
	clientSocket.close();
	GameClient();
}

int GameClient::GetData(PacketFunctions& Packet)
{
	int length = -1;
	CheckConnection();
	Packet.ClearPacket();
	length = clientSocket.ReadPack((char*)Packet.Data, NETMAXBUFFERSIZE);
	return length;
}

void StepManiaLanServer::ParseData(PacketFunctions& Packet, int clientNum)
{
	int command = Packet.Read1();
	switch (command)
	{
	case NSCPing:
		// No Operation
		SendValue(NSServerOffset + NSCPingR, clientNum);
		break;
	case NSCPingR:
		// No Operation response
		break;
	case NSCHello:
		// Hello
		Hello(Packet, clientNum);
		break;
	case NSCGSR:
		// Start Request
		Client[clientNum].StartRequest(Packet);
		CheckReady();
		break;
	case NSCGON:
		// GameOver 
		GameOver(Packet, clientNum);
		break;
	case NSCGSU:
		// StatsUpdate
		Client[clientNum].UpdateStats(Packet);
		if (!Client[clientNum].hasCheated)
			Client[clientNum].hasCheated = CheckCheat(clientNum);
		break;
	case NSCSU:
		// Style Update
		Client[clientNum].StyleUpdate(Packet);
		SendUserList();
		break;
	case NSCCM:
		// Chat message
		AnalizeChat(Packet, clientNum);
		break;
	case NSCRSG:
		SelectSong(Packet, clientNum);
		break;
	case NSCSMS:
		ScreenNetMusicSelectStatus(Packet, clientNum);
		break;
	case NSCUPOpts:
		Client[clientNum].Player[0].options = Packet.ReadNT();		
		Client[clientNum].Player[1].options = Packet.ReadNT();		
		break;
	default:
		break;
	}
}	 

void StepManiaLanServer::Hello(PacketFunctions& Packet, int clientNum)
{
	int ClientVersion = Packet.Read1();
	CString build = Packet.ReadNT();

	Client[clientNum].SetClientVersion(ClientVersion, build);

	Reply.ClearPacket();
	Reply.Write1( NSCHello + NSServerOffset );
	Reply.Write1(1);
	Reply.WriteNT(servername);

	SendNetPacket(clientNum, Reply);

	if (ClientHost == -1)
		ClientHost = clientNum;

}

void GameClient::StyleUpdate(PacketFunctions& Packet)
{
	int playernumber = 0;
	Player[0].name = Player[1].name = "";
	twoPlayers = Packet.Read1()-1;
	for (int x = 0; x < twoPlayers+1; ++x)
	{
		playernumber = Packet.Read1();
		Player[playernumber].name = Packet.ReadNT();
	}
}

void GameClient::SetClientVersion(int ver, const CString& b)
{
	version = ver;
	build = b;
}

void GameClient::StartRequest(PacketFunctions& Packet)
{
	int firstbyte = Packet.Read1();
	int secondbyte = Packet.Read1();
	int thirdbyte = Packet.Read1();
	Player[0].feet = firstbyte/16;
	Player[1].feet = firstbyte%16;

	if ((Player[0].feet > 0)&&(Player[1].feet > 0))
		twoPlayers = true;

	Player[0].diff = secondbyte/16;
	Player[1].diff = secondbyte%16;

	startPosition = thirdbyte/16;
	gameInfo.title = Packet.ReadNT();
	gameInfo.subtitle = Packet.ReadNT();
	gameInfo.artist = Packet.ReadNT();
	gameInfo.course = Packet.ReadNT();

	for (int x = 0; x < 2; ++x)
	 {
		Player[x].score = 0;
		Player[x].combo = 0;
		Player[x].projgrade = 0;
		Player[x].maxCombo = 0;

		memset(Player[x].steps, 0, sizeof(int)*9);
	}

	GotStartRequest = true;
}

void StepManiaLanServer::CheckReady()
{
	bool canStart = true;
	int x;

	//Only check clients that are starting (after ScreenNetMusicSelect before InGame).
	for (x = 0; (x < NUMBERCLIENTS)&& canStart; ++x)
			if (Client[x].Used)
				if (Client[x].isStarting)
					if (!Client[x].GotStartRequest)
						canStart = false;
			
	if (canStart)
	{
		//(Test this) 
		//For whatever reason we need to pause in a way
		//that will not use a lot of CPU.
		//When you try playing the music as soon as it's loaded
		//it will not always play ... immediately
		usleep ( 2000000 );

		//Only start clients waiting for start between ScreenNetMusicSelect and game.
		for (x = 0; x < NUMBERCLIENTS; ++x)
		{
			if (Client[x].Used)
				if (Client[x].isStarting)
				{
					Client[x].hasCheated = false;
					Client[x].clientSocket.blocking = true;
					SendValue(NSCGSR + NSServerOffset, x);
					Client[x].GotStartRequest = false;
					if (Client[x].startPosition == 1)
					{
						Client[x].isStarting = false;
						Client[x].InGame = true;
						//After we start the clients, clear each client's hasSong.
						Client[x].hasSong = false;
					}
					Client[x].clientSocket.blocking = false;
				}
		}
	}
}


void StepManiaLanServer::GameOver(PacketFunctions& Packet, int clientNum)
{
	bool allOver = true;
	int x;

	Client[clientNum].hasSong = Client[clientNum].forceHas = 0;
	Client[clientNum].GotStartRequest = false;
	Client[clientNum].InGame = false;
	Client[clientNum].wasIngame = true;

	for (x = 0; (x < NUMBERCLIENTS)&&allOver ; ++x)
		if (Client[x].Used)
			if (Client[x].InGame)
				allOver = false;

	//Wait untill everyone is done before sending
	if (allOver)
	{
		numPlayers = SortStats(playersPtr);
		Reply.ClearPacket();
		Reply.Write1( NSCGON + NSServerOffset );
		Reply.Write1( (uint8_t) numPlayers );
		for (x = 0; x < numPlayers; ++x) 
			Reply.Write1((uint8_t)playersPtr[x]->PlayerID);
		for (x = 0; x < numPlayers; ++x) 
			Reply.Write4(playersPtr[x]->score);
		for (x = 0; x < numPlayers; ++x) 
			Reply.Write1( (uint8_t) playersPtr[x]->projgrade );
		for (x = 0; x < numPlayers; ++x) 
			Reply.Write1( (uint8_t) playersPtr[x]->diff );
		for (int y = 6; y >= 1; --y)
			for (x = 0; x < numPlayers; ++x)
				Reply.Write2( (uint16_t) playersPtr[x]->steps[y] );
		for (x = 0; x < numPlayers; ++x) 
			Reply.Write2( (uint16_t) playersPtr[x]->steps[8] );  //Tack on OK
		for (x = 0; x < numPlayers; ++x) 
			Reply.Write2( (uint16_t) playersPtr[x]->maxCombo );
		for (x = 0; x < numPlayers; ++x)
			Reply.WriteNT( playersPtr[x]->options );

		for (x = 0; x < NUMBERCLIENTS; ++x)
			if(Client[x].wasIngame)
			{
				SendNetPacket(x, Reply);
				Client[x].wasIngame = false;
			}
	}
}

void StepManiaLanServer::AssignPlayerIDs()
{
	int counter = 0;
	//Future: Figure out how to do dynamic numbering.
	for (int x = 0; x < NUMBERCLIENTS; ++x)
		for(int y = 0; y < 2; ++y)
			Client[x].Player[y].PlayerID = counter++;
}

int StepManiaLanServer::SortStats(LanPlayer *playersPtr[])
{
	int counter = 0;
	LanPlayer *tmp;
	StatsNameChange = false;	//Set it to no name so stats will not send if there is not a change
	bool allZero = true;

	//Populate with in game players only
	for (int x = 0; x < NUMBERCLIENTS; ++x)
		if (Client[x].Used)
			if (Client[x].InGame||Client[x].wasIngame)
				for (int y = 0; y < 2; ++y)
					if (Client[x].IsPlaying(y))
						playersPtr[counter++] = &Client[x].Player[y];


	for (int x = 0; x < counter; ++x)
	{
		 //See if all the players have zero. Used to send names if everyone is 0
		if ((playersPtr[x]->score > 0) && allZero)
			allZero = false;

		if ((x+1) < counter)
			if ((playersPtr[x]->score) < (playersPtr[x+1]->score))
			{
				tmp = playersPtr[x];
				playersPtr[x] = playersPtr[x+1];
				playersPtr[x+1] = tmp;
				x = 0;
				StatsNameChange = true;
			}

	}
	//If there are players all at 0, the name lsit won't be updated
	//this make sure it is sent to the server eventually
	if (!StatsNameChange && allZero)
		StatsNameChange = true;

	return counter;
}

void StepManiaLanServer::SendStatsToClients()
{
	int x;

	numPlayers = SortStats(playersPtr); //Return number of players

	//If there was a change in the name data, send it to the clients.
	//Used to save bandwidth and some processing time.
	//Disabled this. FUTURE: Figure out why it dosn't work correctly and fix.
//	if (StatsNameChange)
//	{
		/* Write and Send name packet */
		Reply.ClearPacket();
		Reply.Write1(NSCGSU + NSServerOffset);
		Reply.Write1(0);
		Reply.Write1( (uint8_t) numPlayers );
		StatsNameColumn(Reply, playersPtr, numPlayers);

		//Send to in game clients only.
		for (x = 0; x < NUMBERCLIENTS; ++x)
			if (Client[x].InGame)
				SendNetPacket(x, Reply);

//	}

	/* Write and send Combo packet */
	Reply.ClearPacket();

	Reply.Write1(NSCGSU + NSServerOffset);
	Reply.Write1(1);
	Reply.Write1( (uint8_t) numPlayers );
	StatsComboColumn(Reply, playersPtr, numPlayers);

	//Send to in game clients only.
	for (x = 0; x < NUMBERCLIENTS; ++x)
		if (Client[x].InGame)
			SendNetPacket(x, Reply);
	

	/* Write and send projgrade packet*/
	//Is it worth the programing troube to save a small amount of bandwidth here?
	//Probably not. Sends all everytime unless developer feelings change.
	Reply.ClearPacket();

	Reply.Write1(NSCGSU + NSServerOffset);
	Reply.Write1(2);
	Reply.Write1( (uint8_t) numPlayers );
	StatsProjgradeColumn(Reply, playersPtr, numPlayers);

	//Send to in game clients only.
	for (x = 0; x < NUMBERCLIENTS; ++x)
		if (Client[x].InGame)
			SendNetPacket(x, Reply);

}

void StepManiaLanServer::SendNetPacket(int client, PacketFunctions& Packet)
{
	/* If there is a connected client where we want to send then do so */
	if (Client[client].Used)
		Client[client].clientSocket.SendPack((char*)Packet.Data, Packet.Position);

}

void StepManiaLanServer::StatsNameColumn(PacketFunctions &data, LanPlayer *playersPtr[], int numPlayers)
{
	CString numname;
	for (int x = 0; x < numPlayers; ++x)
		data.Write1( (uint8_t) playersPtr[x]->PlayerID );
}

void StepManiaLanServer::StatsComboColumn(PacketFunctions &data, LanPlayer *playersPtr[], int numPlayers)
{
	for( int x = 0; x < numPlayers; ++x )
		data.Write2( (uint16_t) playersPtr[x]->combo);
}

void StepManiaLanServer::StatsProjgradeColumn(PacketFunctions& data, LanPlayer *playersPtr[], int numPlayers)
{
	for( int x = 0; x < numPlayers; ++x )
		data.Write1( (uint8_t) playersPtr[x]->projgrade );
}

bool GameClient::IsPlaying(int x)
{
	/* If the feet setting is above 0, there must be a player. */
	if (Player[x].feet > 0)
		return true;

	return false;
}

void GameClient::UpdateStats(PacketFunctions& Packet)
{
	/* Get the Stats from a packet */
	char firstbyte = Packet.Read1();
	char secondbyte = Packet.Read1();
	int pID = int(firstbyte/16); /* MSN */

	if (!hasCheated) {

		Player[pID].currstep = int(firstbyte%16); /* LSN */
		Player[pID].projgrade = int(secondbyte/16);
		Player[pID].score = Packet.Read4();
		Player[pID].combo = Packet.Read2();

		if (Player[pID].combo > Player[pID].maxCombo)
			Player[pID].maxCombo = Player[pID].combo;

		Player[pID].health = Packet.Read2();
		Player[pID].offset = ((double)abs(int(Packet.Read2())-32767)/2000);
		Player[pID].steps[Player[pID].currstep]++;
	}
	else
	{
		Player[pID].currstep = 0;
		Player[pID].projgrade = 'E';
		Player[pID].score = 0;
		Player[pID].combo = 0;
		Player[pID].maxCombo = 0;
		Player[pID].health = 0;
		Player[pID].offset = 99999.99;
	}
}

void StepManiaLanServer::NewClientCheck()
{
	/* Find the first available empty client. Then stick a new connection
		 in that client socket.*/
	int CurrentEmptyClient = FindEmptyClient();

	if (CurrentEmptyClient > -1)
		if (server.accept(Client[CurrentEmptyClient].clientSocket) == 1)
			Client[CurrentEmptyClient].Used = 1;

	if (IsBanned(Client[CurrentEmptyClient].clientSocket.GetIn_addr()))
		Client[CurrentEmptyClient].Disconnect();

}

void StepManiaLanServer::SendValue(uint8_t value, int clientNum)
{
	Client[clientNum].clientSocket.SendPack((char*)&value, sizeof(uint8_t));
}

void StepManiaLanServer::AnalizeChat(PacketFunctions &Packet, int clientNum)
{
	CString message = Packet.ReadNT();
	char firstc = message.at(0);
	if (message.at(0) == '/')
	{
		CString command = message.substr(1, message.find(" ")-1);
		if ((command.compare("list") == 0)||(command.compare("have") == 0))
		{
			if (command.compare("list") == 0)
			{
				Reply.ClearPacket();
				Reply.Write1(NSCCM + NSServerOffset);
				Reply.WriteNT(ListPlayers());
				SendNetPacket(clientNum, Reply);
			}
			else
			{
				message = "";
				message += Client[clientNum].Player[0].name;
				if (Client[clientNum].twoPlayers)
					message += "&";
				message += Client[clientNum].Player[1].name;
				message += " forces has song.";
				Client[clientNum].forceHas = true;
				ServerChat(message);
			}
		}
		else
		{
			if (clientNum == 0)
			{
				if (command.compare("force_start") == 0)
					ForceStart();
				if (command.compare("kick") == 0)
				{
					CString name = message.substr(message.find(" ")+1);
					Kick(name);
				}
				if (command.compare("ban") == 0)
				{
					CString name = message.substr(message.find(" ")+1);
					Ban(name);
				}
			}
			else
			{
				Reply.ClearPacket();
				Reply.Write1(NSCCM + NSServerOffset);
				Reply.WriteNT("You do not have permission to use server commands.");
				SendNetPacket(clientNum, Reply);
			}
		}
	}
	else
	{
		RelayChat((CString&)message, clientNum);
	}
}

void StepManiaLanServer::RelayChat(CString &passedmessage, int clientNum)
{
	Reply.ClearPacket();
	CString message = "";

	message += Client[clientNum].Player[0].name;

	if (Client[clientNum].twoPlayers)
			message += "&";

	message += Client[clientNum].Player[1].name;

	message += ": ";
	message += passedmessage;
	Reply.Write1(NSCCM + NSServerOffset);
	Reply.WriteNT(message);

	//Send to all clients
	SendToAllClients(Reply);
}

void StepManiaLanServer::SelectSong(PacketFunctions& Packet, int clientNum)
{
	int use = Packet.Read1();
	CString message;

	if (use == 2)
	{
		if (clientNum == 0)
		{ 
			SecondSameSelect = false;

			CurrentSongInfo.title = Packet.ReadNT();
			CurrentSongInfo.artist = Packet.ReadNT();
			CurrentSongInfo.subtitle = Packet.ReadNT();

			Reply.ClearPacket();
			Reply.Write1(NSCRSG + NSServerOffset);
			Reply.Write1(1);
			Reply.WriteNT(CurrentSongInfo.title);
			Reply.WriteNT(CurrentSongInfo.artist);
			Reply.WriteNT(CurrentSongInfo.subtitle);		

			//Only send data to clients currently in ScreenNetMusicSelect
			for (int x = 0; x < NUMBERCLIENTS; ++x)
				if (Client[x].inNetMusicSelect)
					SendNetPacket(x, Reply);
//			SendToAllClients(Reply);

			//The following code forces the host to select the same song twice in order to play it.
			if (strcmp(CurrentSongInfo.title, LastSongInfo.title) == 0)
				if (strcmp(CurrentSongInfo.artist, LastSongInfo.artist) == 0)
					if (strcmp(CurrentSongInfo.artist, LastSongInfo.artist) == 0)
						SecondSameSelect = true;

			if (!SecondSameSelect)
			{
				LastSongInfo.title = CurrentSongInfo.title;
				LastSongInfo.artist = CurrentSongInfo.artist;
				LastSongInfo.subtitle = CurrentSongInfo.subtitle;
				message = "Play \"";
				message += CurrentSongInfo.title + " " + CurrentSongInfo.subtitle;
				message += "\"?";
				ServerChat(message);
			}

		}
		else
		{
			message = servername;
			message += ": You do not have permission to pick a song.";
			Reply.ClearPacket();
			Reply.Write1(NSCCM + NSServerOffset);
			Reply.WriteNT(message);
			SendNetPacket(clientNum, Reply);
		}
	}

	if (use == 1)
	{
		//If user dosn't have song
		Client[clientNum].hasSong = false;
		message = Client[clientNum].Player[0].name;

		if (Client[clientNum].twoPlayers)
		{
			message += "&";
			message += Client[clientNum].Player[1].name;
		}

		message += " lacks song \"";
		message += CurrentSongInfo.title;
		message += "\"";
		ServerChat(message);
	}

	//If client has song
	if (use == 0)
		Client[clientNum].hasSong = true;

	//Only play if everyone has the same song and the host has select the same song twice.
	if ( CheckHasSongState() && SecondSameSelect && (clientNum == 0) )
	{
		Reply.ClearPacket();
		Reply.Write1(NSCRSG + NSServerOffset);
		Reply.Write1(2);
		Reply.WriteNT(CurrentSongInfo.title);
		Reply.WriteNT(CurrentSongInfo.artist);
		Reply.WriteNT(CurrentSongInfo.subtitle);

		//Reset last song in case host picks same song again (otherwise dual select is bypassed)
		LastSongInfo.title = "";
		LastSongInfo.artist = "";
		LastSongInfo.subtitle = "";

		//Only send data to clients currently in ScreenNetMusicSelect
		for (int x = 0; x < NUMBERCLIENTS; ++x)
			if (Client[x].inNetMusicSelect)
			{
				SendNetPacket(x, Reply);
				//Designate the client is starting,
				//after ScreenNetMusicSelect but before game play (InGame).
				Client[x].isStarting = true;
			}

//		SendToAllClients(Reply);
	}
}

bool StepManiaLanServer::CheckHasSongState()
{
	for (int x = 0; x < NUMBERCLIENTS; ++x)
		if (Client[x].Used)
			if (Client[x].inNetMusicSelect)
				if (!Client[x].hasSong)
					return false;

	return true;
}

void StepManiaLanServer::ClearHasSong()
{
	for (int x = 0; x < NUMBERCLIENTS; ++x)
		if (Client[x].Used)
			Client[x].hasSong = false;

}

void StepManiaLanServer::SendToAllClients(PacketFunctions& Packet)
{
	for (int x = 0; x < NUMBERCLIENTS; ++x)
		SendNetPacket(x, Packet);

}
void StepManiaLanServer::ServerChat(const CString& message)
{
	CString x = servername + ": " + message;
	Reply.ClearPacket();
	Reply.Write1(NSCCM + NSServerOffset);
	Reply.WriteNT(x);
	SendToAllClients(Reply);
}

int StepManiaLanServer::FindEmptyClient()
{
	//Look at Used flag, if zero then it's an empty client (return client index, -1 if none).
	for (int x = 0; x < NUMBERCLIENTS; ++x)
		if (Client[x].Used == 0)
			return x;
	
	return -1;
}

void GameClient::CheckConnection()
{
	//If there is an error close the socket.
	if (clientSocket.IsError())
	{
		clientSocket.close();
		Used = GotStartRequest = hasSong = InGame = inNetMusicSelect = false;
		Player[0].name = Player[1].name = "";
	}
}

void StepManiaLanServer::MoveClientToHost()
{
	if (!Client[ClientHost].Used)
		for (int x = 1; x < NUMBERCLIENTS; ++x)
			if (Client[x].Used)
			{
				ClientHost = x;
				x = NUMBERCLIENTS+1;
			}
}

void StepManiaLanServer::SendUserList()
{
	Reply.ClearPacket();
	Reply.Write1(NSCUUL + NSServerOffset);
	Reply.Write1(NUMBERCLIENTS*2);
	Reply.Write1(NUMBERCLIENTS*2);

	for (int x = 0; x < NUMBERCLIENTS; ++x)
	{
		for (int y = 0; y < 2; ++y)
		{
			if (Client[x].Player[y].name.length() == 0)
				Reply.Write1(0);
			else
				Reply.Write1(1);
			Reply.WriteNT(Client[x].Player[y].name);
		}
	}

	SendToAllClients(Reply);
}

void StepManiaLanServer::ScreenNetMusicSelectStatus(PacketFunctions& Packet, int clientNum)
{
	CString message = "";
	int EntExitCode = Packet.Read1();

	message += Client[clientNum].Player[0].name;
	if (Client[clientNum].twoPlayers)
		message += "&";
	message += Client[clientNum].Player[1].name;

	if (EntExitCode % 2 == 1)
		Client[clientNum].inNetMusicSelect = true;
	else
		Client[clientNum].inNetMusicSelect = false;

	switch (EntExitCode)
	{
	case 0:
		message += " left the song selection.";
		break;
	case 1:
		message += " entered the song selection.";
		break;
	case 2:
		message += " went into options.";
		break;
	case 3:
		message += " came back from options.";
		break;
	}
	ServerChat(message);
}

CString StepManiaLanServer::ListPlayers()
{
	CString list= "Player List:\n";
	for (int x = 0; x < NUMBERCLIENTS; ++x)
		if ((Client[x].Used)&&(Client[x].inNetMusicSelect))
			for (int y = 0; y < 2; ++y)
				if (Client[x].Player[y].name.length() > 0)
					list += Client[x].Player[y].name + "\n";
	return list;
}

void StepManiaLanServer::Kick(CString &name)
{
	for (int x = 0; x < NUMBERCLIENTS; ++x)
		if (Client[x].Used)
			for (int y = 0; y < 2; ++y)
				if (name == Client[x].Player[y].name)
				{
					ServerChat("Kicked " + name + ".");
					Client[x].Disconnect();
				}
}

void StepManiaLanServer::Ban(CString &name)
{
	for (int x = 0; x < NUMBERCLIENTS; ++x)
		if (Client[x].Used)
			for (int y = 0; y < 2; y++)
				if (name == Client[x].Player[y].name)
				{
					ServerChat("Banned " + name + ".");
					bannedIPs.push_back(Client[x].clientSocket.GetIn_addr());
					Client[x].Disconnect();
				}
}

bool StepManiaLanServer::IsBanned(in_addr &ip)
{
	for (unsigned int x = 0; x < bannedIPs.size(); ++x)
		if (ip.S_un.S_addr == bannedIPs[x].S_un.S_addr)
			return true;
	return false;
}

void StepManiaLanServer::ForceStart()
{
		Reply.ClearPacket();
		Reply.Write1(NSCRSG + NSServerOffset);
		Reply.Write1(2);
		Reply.WriteNT(CurrentSongInfo.title);
		Reply.WriteNT(CurrentSongInfo.artist);
		Reply.WriteNT(CurrentSongInfo.subtitle);

		//Reset last song in case host picks same song again (otherwise dual select is bypassed)
		LastSongInfo.title = "";
		LastSongInfo.artist = "";
		LastSongInfo.subtitle = "";

		//Only send data to clients currently in ScreenNetMusicSelect
		for (int x = 0; x < NUMBERCLIENTS; ++x)
			if (Client[x].inNetMusicSelect)
				if((Client[x].hasSong)||(Client[x].forceHas))
				{
					Client[x].hasCheated = false;
					SendNetPacket(x, Reply);
					//Designate the client is starting,
					//after ScreenNetMusicSelect but before game play (InGame).
					Client[x].isStarting = true;
				}
}

bool StepManiaLanServer::CheckCheat(int clientNum)
{
	for (int x = 0; x < 2; ++x)
		if (Client[clientNum].IsPlaying(x))
		{
			if ((Client[clientNum].Player[x].currstep == 2)&&
				(PREFSMAN->m_fJudgeWindowSecondsBoo < Client[clientNum].Player[x].offset))
				return true;
			if ((Client[clientNum].Player[x].currstep == 3)&&
				(PREFSMAN->m_fJudgeWindowSecondsGood < Client[clientNum].Player[x].offset))
				return true;
			if ((Client[clientNum].Player[x].currstep == 4)&&
				(PREFSMAN->m_fJudgeWindowSecondsGreat < Client[clientNum].Player[x].offset))
				return true;
			if ((Client[clientNum].Player[x].currstep == 5)&&
				(PREFSMAN->m_fJudgeWindowSecondsPerfect < Client[clientNum].Player[x].offset))
				return true;
			if ((Client[clientNum].Player[x].currstep == 6)&&
				(PREFSMAN->m_fJudgeWindowSecondsMarvelous < Client[clientNum].Player[x].offset))
				return true;
			if ((Client[clientNum].Player[x].currstep == 7)&&
				(PREFSMAN->m_fJudgeWindowSecondsOK < Client[clientNum].Player[x].offset))
				return true;
		}
	return false;
}
#endif
/*
 * (c) 2003-2004 Joshua Allen
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

