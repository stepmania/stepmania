#ifndef NetworkSyncServer_H
#define NetworkSyncServer_H

#include "global.h"
#include "ezsockets.h"
#include "NetworkSyncManager.h"

#include <time.h>

#include "StdString.h"

using namespace std;

typedef unsigned char Uint8;

#define NETMAXBUFFERSIZE 1020
#define NUMBERCLIENTS 16

class LanPlayer {
	public:
		string name;
		long score;
		int health;
		int feet;
		int projgrade;
		int combo;
		int currstep;
		int steps[8];
		int maxCombo;
		int Grade;
		int offset;
		int PlayerID;
		char diff;
		LanPlayer();
	private:
};

class GameInfo {
	public:
		CString title;
		CString subtitle;
		CString artist;
		CString course;
	private:
};

class GameClient {
	public:
		bool Ready;
		EzSockets clientSocket;
		bool Used;
		void UpdateStats(PacketFunctions &Packet);
		void SetClientVersion(int ver, CString b);
		void StartRequest(PacketFunctions &Packet);
		int GetData(PacketFunctions &Packet);
		GameClient();
		LanPlayer Player[2];
		void CheckConnection();
		bool IsPlaying(int Player);
		void StyleUpdate(PacketFunctions &Packet);
		bool InGame;
		int twoPlayers;
		bool hasSong;
	private:
		string build;
		GameInfo gameInfo;
		int version;
		int startPosition;
};

class StepManiaLanServer {
	public:
		bool ServerStart();
		void ServerStop();
		void ServerUpdate();
		StepManiaLanServer();
		~StepManiaLanServer();
		CString servername;
	private:
		bool stop;
		PacketFunctions Packet;
		PacketFunctions Reply;
		GameClient Client[NUMBERCLIENTS];
		int CurrentEmptyClient; 
		EzSockets server;
		int ClientHost;
		LanPlayer *playersPtr[NUMBERCLIENTS*2];
		time_t statsTime;
		GameInfo CurrentSongInfo;
		GameInfo LastSongInfo;
		bool StatsNameChange;
		bool SecondSameSelect;
		int numPlayers;

		void Hello(PacketFunctions&Packet, int clientNum);
		void UpdateClients();
		int FindEmptyClient();
		void NewClientCheck();
		void ParseData(PacketFunctions &Packet, int clientNum);
		void SendValue(Uint8 value, int clientNum);
		void CheckReady();
		void MoveClientToHost();
		void StatsComboColumn(PacketFunctions &data, LanPlayer *playersPtr[],
		int numPlayers);
		void SendStatsToClients();
		void StatsProjgradeColumn(PacketFunctions &data, LanPlayer *playersPtr[], int numPlayers);
		void StatsNameColumn(PacketFunctions &data, LanPlayer *playersPtr[], int numPlayers);
		void SendNetPacket(int client, char *data, int size);
		int SortStats(LanPlayer *playersPtr[]);
		void RelayChat(PacketFunctions &Packet, int clientNum);
		void SelectSong(PacketFunctions &Packet, int clientNum);
		void ServerChat(CString message);
		void SendToAllClients(PacketFunctions &Packet);
		bool CheckHasSongState();
		void ClearHasSong();
		void AssignPlayerIDs();
		void SendUserList();
		void GameOver(PacketFunctions &Packet, int clientNum);
};

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
