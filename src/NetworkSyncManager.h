#ifndef NetworkSyncManager_H
#define NetworkSyncManager_H

#include "PlayerNumber.h"
#include "Difficulty.h"
#include <queue>

class LoadingWindow;

const int NETPROTOCOLVERSION=4;
const int NETMAXBUFFERSIZE=1020; //1024 - 4 bytes for EzSockets
const int NETNUMTAPSCORES=8;

// [SMLClientCommands name]
enum NSCommand
{
	NSCPing = 0,
	NSCPingR,		//  1 [SMLC_PingR]
	NSCHello,		//  2 [SMLC_Hello]
	NSCGSR,			//  3 [SMLC_GameStart]
	NSCGON,			//  4 [SMLC_GameOver]
	NSCGSU,			//  5 [SMLC_GameStatusUpdate]
	NSCSU,			//  6 [SMLC_StyleUpdate]
	NSCCM,			//  7 [SMLC_Chat]
	NSCRSG,			//  8 [SMLC_RequestStart]
	NSCUUL,			//  9 [SMLC_Reserved1]
	NSCSMS,			// 10 [SMLC_MusicSelect]
	NSCUPOpts,		// 11 [SMLC_PlayerOpts]
	NSCSMOnline,	// 12 [SMLC_SMO]
	NSCFormatted,	// 13 [SMLC_RESERVED1]
	NSCAttack,		// 14 [SMLC_RESERVED2]
	XML,		// 15 [SMLC_RESERVED3]
	FLU,		// 15 [SMLC_FriendListUpdate]
	NUM_NS_COMMANDS
};

enum SMOStepType
{
	SMOST_UNUSED = 0,
	SMOST_HITMINE,
	SMOST_AVOIDMINE,
	SMOST_MISS,
	SMOST_W5,
	SMOST_W4,
	SMOST_W3,
	SMOST_W2,
	SMOST_W1,
	SMOST_LETGO,
	SMOST_HELD
	/*
	,SMOST_CHECKPOINTMISS,
	SMOST_CHECKPOINTHIT
	 */
};

const NSCommand NSServerOffset = (NSCommand)128;

// TODO: Provide a Lua binding that gives access to this data. -aj
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
/** @brief A special foreach loop going through each NSScoreBoardColumn. */
#define FOREACH_NSScoreBoardColumn( sc ) FOREACH_ENUM( NSScoreBoardColumn, sc )

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

	void Write1( uint8_t Data );
	void Write2( uint16_t Data );
	void Write4( uint32_t Data );
	void WriteNT( const RString& Data );

	void ClearPacket();
};
/** @brief Uses ezsockets for primitive song syncing and score reporting. */
class NetworkSyncManager 
{
public:
	NetworkSyncManager( LoadingWindow *ld = nullptr );
	~NetworkSyncManager();

    // If "useSMserver" then send score to server
	void ReportScore( int playerID, int step, int score, int combo, float offset );	
	void ReportScore(int playerID, int step, int score, int combo, float offset, int numNotes);
	void ReportSongOver();
	void ReportStyle(); // Report style, players, and names
	void ReportNSSOnOff( int i );	// Report song selection screen on/off
	void StartRequest( short position );	// Request a start; Block until granted.
	RString GetServerName();

	// SMOnline stuff
	// FIXME: This should NOT be public. PERIOD.
	void SendSMOnline( );

	bool Connect( const RString& addy, unsigned short port );

	void PostStartUp( const RString& ServerIP );

	void CloseConnection();

	void DisplayStartupStatus();	// Notify user if connect attempt was successful or not.

	int m_playerLife[NUM_PLAYERS];	// Life (used for sending to server)

	void Update( float fDeltaTime );

	bool useSMserver;
	bool isSMOnline;
	bool isSMOLoggedIn[NUM_PLAYERS];

	vector<int> m_PlayerStatus;
	int m_ActivePlayers;
	vector<int> m_ActivePlayer;
	vector<RString> m_PlayerNames;

	//friendlist
	std::vector<RString> fl_PlayerNames;
	std::vector<int> fl_PlayerStates;
	
	// Used for ScreenNetEvaluation
	vector<EndOfGame_PlayerData> m_EvalPlayerData;

	// Used together: 
	bool ChangedScoreboard(int Column);	// Returns true if scoreboard changed since function was last called.
	RString m_Scoreboard[NUM_NSScoreBoardColumn];

	// Used for chatting
	void SendChat(const RString& message);
	RString m_WaitingChat;

	// Used for options
	void ReportPlayerOptions();

	// Used for song checking/changing
	RString m_sMainTitle;
	RString m_sArtist;
	RString m_sSubTitle;
	RString m_sFileHash;
	int m_iSelectMode;
	void SelectUserSong();

	int GetServerVersion();

	RString m_sChatText;

	// FIXME: This should NOT be public. PERIOD. It probably shouldn't be a member at all.
	PacketFunctions	m_SMOnlinePacket;

	StepManiaLanServer *LANserver;

	int GetSMOnlineSalt();

	RString MD5Hex( const RString &sInput );

	void GetListOfLANServers( vector<NetServerInfo>& AllServers );

	// Aldo: Please move this method to a new class, I didn't want to create new files because I don't know how to properly update the files for each platform.
	// I preferred to misplace code rather than cause unneeded headaches to non-windows users, although it would be nice to have in the wiki which files to
	// update when adding new files.
	static unsigned long GetCurrentSMBuild( LoadingWindow* ld );
private:
#if !defined(WITHOUT_NETWORKING)

	void ProcessInput();
	SMOStepType TranslateStepType(int score);
	void StartUp();

	int m_playerID;  // Currently unused, but need to stay
	int m_step;
	int m_score;
	int m_combo;
    
	int m_startupStatus;	// Used to see if attempt was successful or not.
	int m_iSalt;

	bool m_scoreboardchange[NUM_NSScoreBoardColumn];

	RString m_ServerName;
 
	EzSockets *NetPlayerClient;
	EzSockets *BroadcastReception;

	vector<NetServerInfo> m_vAllLANServers;

	int m_ServerVersion; // ServerVersion

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
