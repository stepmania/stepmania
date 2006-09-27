/* NetworkSyncManager - Uses ezsockets for primitive song syncing and score reporting. */

#ifndef NetworkSyncManager_H
#define NetworkSyncManager_H

#include "PlayerNumber.h"
#include "Difficulty.h"
#include <queue>

class LoadingWindow;

const int NETPROTOCOLVERSION=3;
const int NETMAXBUFFERSIZE=1020; //1024 - 4 bytes for EzSockets
const int NETNUMTAPSCORES=8;

enum NSCommand
{
	NSCPing = 0,
	NSCPingR,		//1
	NSCHello,		//2
	NSCGSR,			//3
	NSCGON,			//4
	NSCGSU,			//5
	NSCSU,			//6
	NSCCM,			//7
	NSCRSG,			//8
	NSCUUL,			//9
	NSCSMS,			//10
	NSCUPOpts,		//11
	NSCSMOnline,	//12
	NSCFormatted,	//13
	NSCAttack,		//14
	NUM_NS_COMMANDS
};

const NSCommand NSServerOffset = (NSCommand)128;

struct EndOfGame_PlayerData
{
	int name;
	int score;
	int grade;
	Difficulty difficulty;
	int tapScores[NETNUMTAPSCORES];	//This will be a const soon enough
	RString playerOptions;
};

enum NSScoreBoardColumn
{
	NSSB_NAMES=0,
	NSSB_COMBO,
	NSSB_GRADE,
	NUM_NSScoreBoardColumn,
	NSScoreBoardColumn_Invalid
};
#define FOREACH_NSScoreBoardColumn( sc ) FOREACH_ENUM( NSScoreBoardColumn, NUM_NSScoreBoardColumn, sc )

struct NetServerInfo
{
	RString Name;
	RString Address;
};

class EzSockets;
class StepManiaLanServer;

class PacketFunctions
{
public:
	unsigned char Data[NETMAXBUFFERSIZE];	//Data
	int Position;				//Other info (Used for following functions)
	int size;					//When sending these pacs, Position should
								//be used; NOT size.

	//Commands used to operate on NetPackets
	uint8_t Read1();
	uint16_t Read2();
	uint32_t Read4();
	RString ReadNT();

	void Write1(uint8_t Data);
	void Write2(uint16_t Data);
	void Write4(uint32_t Data);
	void WriteNT(const RString& Data);

	void ClearPacket();
};

class NetworkSyncManager 
{
public:
	NetworkSyncManager( LoadingWindow *ld = NULL );
	~NetworkSyncManager();

    //If "useSMserver" then send score to server
	void ReportScore(int playerID, int step, int score, int combo, float offset);	
	void ReportSongOver();	//Report to server that song is over
	void ReportStyle();		//Report to server the style, players, and names
	void ReportNSSOnOff(int i);	//Report song selection screen on/off
	void StartRequest(short position);	//Request a start.  Block until granted.
	RString GetServerName();
	
	//SMOnline stuff
	void SendSMOnline( );

	bool Connect(const RString& addy, unsigned short port); // Connect to SM Server

	void PostStartUp(const RString& ServerIP);

	void CloseConnection();

	void DisplayStartupStatus();	//Used to note user if connect attempt was successful or not.

	int m_playerLife[NUM_PLAYERS];	//Life (used for sending to server)

	void Update(float fDeltaTime);

	bool useSMserver;
	bool isSMOnline;
	bool isSMOLoggedIn[NUM_PLAYERS];

	vector <int> m_PlayerStatus;
	int m_ActivePlayers;
	vector <int> m_ActivePlayer;
	vector <RString> m_PlayerNames;

	//Used for ScreenNetEvaluation
	vector<EndOfGame_PlayerData> m_EvalPlayerData;

	//Used togeather for 
	bool ChangedScoreboard(int Column);	//If scoreboard changed since this function last called, then true.
	RString m_Scoreboard[NUM_NSScoreBoardColumn];

	//Used for chatting
	void SendChat(const RString& message);
	RString m_WaitingChat;

	//Used for options
	void ReportPlayerOptions();

	//Used for song checking/changing
	RString m_sMainTitle;
	RString m_sArtist;
	RString m_sSubTitle;
	int m_iSelectMode;
	void SelectUserSong();

	RString			m_sChatText;

	PacketFunctions	m_SMOnlinePacket;

	bool isLanServer;	//Must be public for ScreenNetworkOptions
	StepManiaLanServer *LANserver;

	int GetSMOnlineSalt();

	RString MD5Hex( const RString &sInput );

	void GetListOfLANServers( vector< NetServerInfo > & AllServers );
private:
#if !defined(WITHOUT_NETWORKING)

	void ProcessInput();

	void StartUp();

	int m_playerID;  //these are currently unused, but need to stay
	int m_step;
	int m_score;
	int m_combo;
    
	int m_startupStatus;	//Used to see if attempt was successful or not.
	int	m_iSalt;

	bool m_scoreboardchange[NUM_NSScoreBoardColumn];

	RString m_ServerName;
 
    EzSockets *NetPlayerClient;
	EzSockets *BroadcastReception;

	vector< NetServerInfo > m_vAllLANServers;

	int m_ServerVersion; //ServerVersion

	bool Listen(unsigned short port);

	PacketFunctions m_packet;
#endif
};

extern NetworkSyncManager *NSMAN;
 
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
