/* NetworkSyncServer.cpp
 */

#include "NetworkSyncServer.h"
#include "RageLog.h"

LanPlayer::LanPlayer() {
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

StepManiaLanServer::StepManiaLanServer() {
	stop = true;
	SecondSameSelect = false;
	AssignPlayerIDs();
}

StepManiaLanServer::~StepManiaLanServer() {
	ServerStop();
}

bool StepManiaLanServer::ServerStart() {
	server.blocking = 0; /* Turn off blocking */
	if (server.create())
		if (server.bind(8765))
			if (server.listen()) {
				stop = false;
				statsTime = time(NULL);
				return true;
			}

	//Hopefully we will not get here. If we did, something went wrong above.
	return false;
}

void StepManiaLanServer::ServerStop() {
	int x;
	for (x = 0; x < NUMBERCLIENTS; x++) {
		Client[x].clientSocket.close();
		Client[x].Used = false;
	}
	server.close();
	stop = true;
}

void StepManiaLanServer::ServerUpdate() {
	if (!stop) {
		NewClientCheck(); /* See if there is another client wanting to play */
		UpdateClients();
//		CheckReady();
		if (time(NULL) > statsTime) {
			SendStatsToClients();
			statsTime = time(NULL);
		}
	// MoveClientToHost();
	}
}

void StepManiaLanServer::UpdateClients() {
	/* Go through all the clients and check to see if it is being used.
		 If so then try to get a backet and parse the data. */
	for (int x = 0; x < NUMBERCLIENTS; x++)
		if (Client[x].Used == 1)
			if (Client[x].GetData(Packet) >= 0)
				ParseData(Packet, x);
}

GameClient::GameClient() {
	Used = 0;
	GotStartRequest = 0;
	clientSocket.blocking = 0;
	twoPlayers = false;
	version = 0;
	startPosition = 0;
	InGame = 0;
	hasSong = false;
	inNetMusicSelect = false;
	isStarting = false;  //Used for after ScreenNetMusicSelect but before InGame
	wasIngame = false;
}

int GameClient::GetData(PacketFunctions &Packet) {
	int length = -1;
	CheckConnection();
	Packet.ClearPacket();
	length = clientSocket.ReadPack((char*)Packet.Data, NETMAXBUFFERSIZE);
	return length;
}

void StepManiaLanServer::ParseData(PacketFunctions &Packet, int clientNum) {
	int command = Packet.Read1();
	switch (command) {
	case 0:
		// No Operation
		SendValue(129, clientNum);
		break;
	case 1:
		// No Operation response
		break;
	case 2:
		// Hello
		Hello(Packet, clientNum);
		break;
	case 3:
		// Start Request
		Client[clientNum].StartRequest(Packet);
		CheckReady();
		break;
	case 4:
		// GameOver 
		GameOver(Packet, clientNum);
		break;
	case 5:
		// StatsUpdate
		Client[clientNum].UpdateStats(Packet);
		break;
	case 6:
		// Style Update
		Client[clientNum].StyleUpdate(Packet);
		SendUserList();
		break;
	case 7:
		// Chat message
		RelayChat(Packet, clientNum);
		break;
	case 8:
		SelectSong(Packet, clientNum);
		break;
	case 10:
		ScreenNetMusicSelectStatus(Packet, clientNum);
		break;
	case 11:
		Client[clientNum].Player[0].options = Packet.ReadNT();		
		Client[clientNum].Player[1].options = Packet.ReadNT();		
		break;
	default:
		break;
	}
}	 

void StepManiaLanServer::Hello(PacketFunctions &Packet, int clientNum) {
	int ClientVersion = Packet.Read1();
	CString build = Packet.ReadNT();

	Client[clientNum].SetClientVersion(ClientVersion, build);

	Reply.ClearPacket();
	Reply.Write1(130);
	Reply.Write1(1);
	Reply.WriteNT(servername);

	SendNetPacket(clientNum, Reply);

	if (ClientHost == -1)
		ClientHost = clientNum;

}

void GameClient::StyleUpdate(PacketFunctions &Packet) {
	int playernumber = 0;
	Player[0].name = Player[1].name = "";
	twoPlayers = Packet.Read1()-1;
	for (int x = 0; x < twoPlayers+1; x++) {
		playernumber = Packet.Read1();
		Player[playernumber].name = Packet.ReadNT();
	}
}

void GameClient::SetClientVersion(int ver, CString b) {
	version = ver;
	build = b;
}

void GameClient::StartRequest(PacketFunctions &Packet) {
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

	for (int x = 0; x < 2; x++) {
		Player[x].score = 0;
		Player[x].combo = 0;
		Player[x].projgrade = 0;
		Player[x].maxCombo = 0;

		memset((void*)Player[x].steps, 0, sizeof(int)*9);
	}

	GotStartRequest = true;
//	InGame = true;
}

void StepManiaLanServer::CheckReady() {
	bool canStart = true;
	int x;

	//Only check clients that are starting (after ScreenNetMusicSelect before InGame).
	for (x = 0; (x < NUMBERCLIENTS)&&(canStart == true); x++)
			if (Client[x].Used == true)
				if (Client[x].isStarting)
					if (Client[x].GotStartRequest == false)
						canStart = false;
			
	if (canStart == true) {
		//(Test this) 
		//For whatever reason we need to pause in a way
		//that will not use a lot of CPU.
		//When you try playing the music as soon as it's loaded
		//it will not always play ... immediately
		usleep ( 2000000 );

		//Only start clients waiting for start between ScreenNetMusicSelect and game.
		for (x = 0; x < NUMBERCLIENTS; x++) {
			if (Client[x].Used == true)
				if (Client[x].isStarting) {
					Client[x].clientSocket.blocking = true;
					SendValue(131, x);
					Client[x].GotStartRequest = false;
					if (Client[x].startPosition == 1) {
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


void StepManiaLanServer::GameOver(PacketFunctions &Packet, int clientNum) {
	bool allOver = true;
	int x;

	Client[clientNum].GotStartRequest = false;
	Client[clientNum].InGame = false;
	Client[clientNum].wasIngame = true;

	for (x = 0; (x < NUMBERCLIENTS)&&(allOver == true); x++)
		if (Client[x].Used == true)
			if ((Client[x].InGame == true))
				allOver = false;

	//Wait untill everyone is done before sending
	if (allOver) {
		numPlayers = SortStats(playersPtr);
		Reply.ClearPacket();
		Reply.Write1( 4+128 );
		Reply.Write1( (uint8_t) numPlayers );
		for (x = 0; x < numPlayers; x++) 
			Reply.Write1((uint8_t)playersPtr[x]->PlayerID);
		for (x = 0; x < numPlayers; x++) 
			Reply.Write4(playersPtr[x]->score);
		for (x = 0; x < numPlayers; x++) 
			Reply.Write1( (uint8_t) playersPtr[x]->projgrade );
		for (x = 0; x < numPlayers; x++) 
			Reply.Write1( (uint8_t) playersPtr[x]->diff );
		for (int y = 6; y >= 1; y--)
			for (x = 0; x < numPlayers; x++)
				Reply.Write2( (uint16_t) playersPtr[x]->steps[y] );
		for (x = 0; x < numPlayers; x++) 
			Reply.Write2( (uint16_t) playersPtr[x]->steps[8] );  //Tack on OK
		for (x = 0; x < numPlayers; x++) 
			Reply.Write2( (uint16_t) playersPtr[x]->maxCombo );
		for (x = 0; x < numPlayers; x++)
			Reply.WriteNT( playersPtr[x]->options );

		for (x = 0; x < NUMBERCLIENTS; x++)
			if(Client[x].wasIngame)
				SendNetPacket(x, Reply);

//		SendToAllClients(Reply);
	}
}

void StepManiaLanServer::AssignPlayerIDs() {
	int counter = 0;
	//Future: Figure out how to do dynamic numbering.
	for (int x = 0; x < NUMBERCLIENTS; x++)
		for(int y = 0; y < 2; y++)
			Client[x].Player[y].PlayerID = counter++;
}

int StepManiaLanServer::SortStats(LanPlayer *playersPtr[]) {
	int counter = 0;
	LanPlayer *tmp;
	StatsNameChange = false;	//Set it to no name so stats will not send if there is not a change
	bool allZero = true;

	//Populate with in game players only
	for (int x = 0; x < NUMBERCLIENTS; x++)
		if (Client[x].Used == true)
			if (Client[x].InGame||Client[x].wasIngame)
				for (int y = 0; y < 2; y++)
					if (Client[x].IsPlaying(y))
						playersPtr[counter++] = &Client[x].Player[y];


	for (int x = 0; x < counter; x++) {
		 //See if all the players have zero. Used to send names if everyone is 0
		if ((playersPtr[x]->score > 0)&&(allZero == true))
			allZero = false;

		if ((x+1) < counter)
			if ((playersPtr[x]->score) < (playersPtr[x+1]->score)) {
				tmp = playersPtr[x];
				playersPtr[x] = playersPtr[x+1];
				playersPtr[x+1] = tmp;
				x = 0;
				StatsNameChange = true;
			}

	}
	//If there are players all at 0, the name lsit won't be updated
	//this make sure it is sent to the server eventually
	if ((StatsNameChange == false)&&(allZero == true))
		StatsNameChange = true;

	return counter;
}

void StepManiaLanServer::SendStatsToClients() {
	int x;

	numPlayers = SortStats(playersPtr); //Return number of players

	//If there was a chagne in the name data, send it to the clients.
	//Used to save bandwidth and some processing time.
	if (StatsNameChange) {

		/* Write and Send name packet */
		Reply.ClearPacket();
		Reply.Write1(133);
		Reply.Write1(0);
		Reply.Write1( (uint8_t) numPlayers );
		StatsNameColumn(Reply, playersPtr, numPlayers);

		//Send to in game clients only.
		for (x = 0; x < NUMBERCLIENTS; x++)
			if (Client[x].InGame == true)
				SendNetPacket(x, Reply);

	}

	/* Write and send Combo packet */
	Reply.ClearPacket();

	Reply.Write1(133);
	Reply.Write1(1);
	Reply.Write1( (uint8_t) numPlayers );
	StatsComboColumn(Reply, playersPtr, numPlayers);

	//Send to in game clients only.
	for (x = 0; x < NUMBERCLIENTS; x++)
		if (Client[x].InGame == true)
			SendNetPacket(x, Reply);
	

	/* Write and send projgrade packet*/
	//Is it worth the programing troube to save a small amount of bandwidth here?
	//Probably not. Sends all everytime unless developer feelings change.
	Reply.ClearPacket();

	Reply.Write1(133);
	Reply.Write1(2);
	Reply.Write1( (uint8_t) numPlayers );
	StatsProjgradeColumn(Reply, playersPtr, numPlayers);

	//Send to in game clients only.
	for (x = 0; x < NUMBERCLIENTS; x++)
		if (Client[x].InGame == true)
			SendNetPacket(x, Reply);

}

void StepManiaLanServer::SendNetPacket(int client, PacketFunctions &Packet) {
	/* If there is a connected client where we want to send then do so */
	if (Client[client].Used == true)
		Client[client].clientSocket.SendPack((char*)Packet.Data, Packet.Position);

}

void StepManiaLanServer::StatsNameColumn(PacketFunctions &data, LanPlayer *playersPtr[], int numPlayers) {
	CString numname;
	for (int x = 0; x < numPlayers; x++)
		data.Write1( (uint8_t) playersPtr[x]->PlayerID );
}

void StepManiaLanServer::StatsComboColumn(PacketFunctions &data, LanPlayer *playersPtr[], int numPlayers) {
	for( int x = 0; x < numPlayers; x++ )
		data.Write2( (uint16_t) playersPtr[x]->combo);
}

void StepManiaLanServer::StatsProjgradeColumn(PacketFunctions &data, LanPlayer *playersPtr[], int numPlayers) {
	for( int x = 0; x < numPlayers; x++ )
		data.Write1( (uint8_t) playersPtr[x]->projgrade );
}

bool GameClient::IsPlaying(int x) {
	/* If the feet setting is above 0, there must be a player. */
	if (Player[x].feet > 0)
		return true;

	return false;
}

void GameClient::UpdateStats(PacketFunctions &Packet) {
	/* Get the Stats form a packet */
	char firstbyte = Packet.Read1();
	char secondbyte = Packet.Read1();
	int pID = int(firstbyte/16); /* MSN */

	Player[pID].currstep = int(firstbyte%16); /* LSN */
	Player[pID].projgrade = int(secondbyte/16);
	Player[pID].score = Packet.Read4();
	Player[pID].combo = Packet.Read2();

	if (Player[pID].combo > Player[pID].maxCombo)
		Player[pID].maxCombo = Player[pID].combo;

	Player[pID].health = Packet.Read2();
	Player[pID].offset = Packet.Read2();
	Player[pID].steps[Player[pID].currstep]++;
}

void StepManiaLanServer::NewClientCheck() {
	/* Find the first available empty client. Then stick a new connection
		 in that client socket.*/
	int CurrentEmptyClient = FindEmptyClient();

	if (CurrentEmptyClient > -1)
		if (server.accept(Client[CurrentEmptyClient].clientSocket) == 1)
			Client[CurrentEmptyClient].Used = 1;

}

void StepManiaLanServer::SendValue(Uint8 value, int clientNum) {
	Client[clientNum].clientSocket.SendPack((char*)&value, sizeof(Uint8));
}

void StepManiaLanServer::RelayChat(PacketFunctions &Packet, int clientNum) {
	Reply.ClearPacket();
	CString message = "";

	message += Client[clientNum].Player[0].name;

	if (Client[clientNum].twoPlayers)
			message += "&";

	message += Client[clientNum].Player[1].name;

	message += ": ";
	message += Packet.ReadNT();
	Reply.Write1(135);
	Reply.WriteNT(message);

	//Send to all clients
	SendToAllClients(Reply);
}

void StepManiaLanServer::SelectSong(PacketFunctions &Packet, int clientNum) {
	int use = Packet.Read1();
	CString message;

	if (use == 2) {
		if (clientNum == 0) { 
			SecondSameSelect = false;

			CurrentSongInfo.title = Packet.ReadNT();
			CurrentSongInfo.artist = Packet.ReadNT();
			CurrentSongInfo.subtitle = Packet.ReadNT();

			Reply.ClearPacket();
			Reply.Write1(136);
			Reply.Write1(1);
			Reply.WriteNT(CurrentSongInfo.title);
			Reply.WriteNT(CurrentSongInfo.artist);
			Reply.WriteNT(CurrentSongInfo.subtitle);		

			//Only send data to clients currently in ScreenNetMusicSelect
			for (int x = 0; x < NUMBERCLIENTS; x++)
				if (Client[x].inNetMusicSelect)
					SendNetPacket(x, Reply);
//			SendToAllClients(Reply);

			//The following code forces the host to select the same song twice in order to play it.
			if (strcmp(CurrentSongInfo.title, LastSongInfo.title) == 0)
				if (strcmp(CurrentSongInfo.artist, LastSongInfo.artist) == 0)
					if (strcmp(CurrentSongInfo.artist, LastSongInfo.artist) == 0)
						SecondSameSelect = true;

			if (SecondSameSelect == false) {
				LastSongInfo.title = CurrentSongInfo.title;
				LastSongInfo.artist = CurrentSongInfo.artist;
				LastSongInfo.subtitle = CurrentSongInfo.subtitle;
				message = servername;
				message += ":Play \"";
				message += CurrentSongInfo.title;
				message += "\"?";
				ServerChat(message);
			}

		} else {
			message = servername;
			message += ": You do not have permission to pick a song.";
			Reply.ClearPacket();
			Reply.Write1(135);
			Reply.WriteNT(message);
			SendNetPacket(clientNum, Reply);
		}
	}

	if (use == 1) {
		//If user dosn't have song
		Client[clientNum].hasSong = false;
		message = servername;
		message += ": ";
		message += Client[clientNum].Player[0].name;

		if (Client[clientNum].twoPlayers) {
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
	if ( CheckHasSongState() && SecondSameSelect && (clientNum == 0) ) {
		Reply.ClearPacket();
		Reply.Write1(136);
		Reply.Write1(2);
		Reply.WriteNT(CurrentSongInfo.title);
		Reply.WriteNT(CurrentSongInfo.artist);
		Reply.WriteNT(CurrentSongInfo.subtitle);

		//Reset last song in case host picks same song again (otherwise dual select is bypassed)
		LastSongInfo.title = "";
		LastSongInfo.artist = "";
		LastSongInfo.subtitle = "";

		//Only send data to clients currently in ScreenNetMusicSelect
		for (int x = 0; x < NUMBERCLIENTS; x++)
			if (Client[x].inNetMusicSelect) {
				SendNetPacket(x, Reply);
				//Designate the client is starting,
				//after ScreenNetMusicSelect but before game play (InGame).
				Client[x].isStarting = true;
			}

//		SendToAllClients(Reply);
	}
}

bool StepManiaLanServer::CheckHasSongState() {
	for (int x = 0; x < NUMBERCLIENTS; x++)
		if (Client[x].Used == true)
			if (Client[x].inNetMusicSelect)
				if (Client[x].hasSong == false)
					return false;

	return true;
}

void StepManiaLanServer::ClearHasSong() {
	for (int x = 0; x < NUMBERCLIENTS; x++)
		if (Client[x].Used == true)
			Client[x].hasSong = false;

}

void StepManiaLanServer::SendToAllClients(PacketFunctions &Packet) {
	for (int x = 0; x < NUMBERCLIENTS; x++)
		SendNetPacket(x, Packet);

}
void StepManiaLanServer::ServerChat(CString message) {
	Reply.ClearPacket();
	Reply.Write1(135);
	Reply.WriteNT(message);
	SendToAllClients(Reply);
}

int StepManiaLanServer::FindEmptyClient() {
	//Look at Used flag, if zero then it's an empty client (return client index, -1 if none).
	for (int x = 0; x < NUMBERCLIENTS; x++)
		if (Client[x].Used == 0)
			return x;
	
	return -1;
}

void GameClient::CheckConnection() {
	//If there is an error close the socket.
	if (clientSocket.IsError()) {
		clientSocket.close();
		Used = GotStartRequest = hasSong = InGame = inNetMusicSelect = false;
		Player[0].name = Player[1].name = "";
	}
}

void StepManiaLanServer::MoveClientToHost() {
	if (Client[ClientHost].Used == false)
		for (int x = 1; x < NUMBERCLIENTS; x++)
			if (Client[x].Used == true) {
				ClientHost = x;
				x = NUMBERCLIENTS+1;
			}
}

void StepManiaLanServer::SendUserList() {
	Reply.ClearPacket();
	Reply.Write1(137);
	Reply.Write1(NUMBERCLIENTS*2);
	Reply.Write1(NUMBERCLIENTS*2);

	for (int x = 0; x < NUMBERCLIENTS; x++) {
		for (int y = 0; y < 2; y++) {
			if (Client[x].Player[y].name.length() == 0) {
				Reply.Write1(0);
			} else {
				Reply.Write1(1);
			}

			Reply.WriteNT(Client[x].Player[y].name);
		}
	}

	SendToAllClients(Reply);
}

void StepManiaLanServer::ScreenNetMusicSelectStatus(PacketFunctions &Packet, int clientNum) {
	LOG->Info("SNMS Change");
	CString message = servername;
	message += ": ";
	if (Packet.Read1() == 1) {
		Client[clientNum].inNetMusicSelect = true;
		message += Client[clientNum].Player[0].name;
		if (Client[clientNum].twoPlayers)
			message += "&";
		message += Client[clientNum].Player[1].name;
		message += " entered the game.";
	} else {
		Client[clientNum].inNetMusicSelect = false;
		message += Client[clientNum].Player[0].name;
		if (Client[clientNum].twoPlayers)
			message += "&";
		message += Client[clientNum].Player[1].name;
		message += " left the game.";
	}
	ServerChat(message);
}

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

