#ifndef NetworkSyncManager_H
#define NetworkSyncManager_H

#include "PlayerNumber.h"
#include "Difficulty.h"
#include <queue>
#include "NetworkPacket.h"
#include "NetworkProtocol.h"

class LoadingWindow;

const int NETPROTOCOLVERSION=3;
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

/** @brief Uses ezsockets for primitive song syncing and score reporting. */
class NetworkSyncManager 
{
public:
	NetworkSyncManager( LoadingWindow *ld = NULL );
	~NetworkSyncManager();

	NetworkProtocol *m_Protocol;

	void CloseConnection();
	void PostStartUp( const RString& ServerIP );
	bool Connect( const RString& addy, unsigned short port );
	void DisplayStartupStatus();	// Notify user if connect attempt was successful or not.
	RString GetServerName();
	int GetSMOnlineSalt();
	void Update( float fDeltaTime );

	RString MD5Hex( const RString &sInput );

	bool useSMserver;
	bool isSMOnline;	// based on server version number
	bool isSMOLoggedIn[NUM_PLAYERS];

	// legacy-specific:
	void ReportNSSOnOff( int i );	// Report song selection screen on/off
	// If "useSMserver" then send score to server
	void ReportScore( int playerID, int step, int score, int combo, float offset );
	void ReportSongOver();
	void ReportStyle(); // Report style, players, and names
	void StartRequest( short position );	// Request a start; Block until granted.

	// (Legacy) SMOnline stuff;
	void SendSMOnline();
	NetworkPacket	m_SMOnlinePacket;

	int m_playerLife[NUM_PLAYERS];	// Life (used for sending to server)

	// (legacy?) user list:
	vector<int> m_PlayerStatus;
	int m_ActivePlayers;
	vector<int> m_ActivePlayer;
	vector<RString> m_PlayerNames;

	// (Legacy) Used for ScreenNetEvaluation
	vector<EndOfGame_PlayerData> m_EvalPlayerData;

	// (legacy) Used together: 
	bool ChangedScoreboard(int Column);	// Returns true if scoreboard changed since function was last called.
	RString m_Scoreboard[NUM_NSScoreBoardColumn];

	// (Legacy but likely common) Used for chatting
	void SendChat(const RString& message);
	RString m_WaitingChat;
	RString m_sChatText;	// chatroom text buffer

	// (Legacy) Used for options
	void ReportPlayerOptions();

	// (Legacy) Used for song checking/changing
	RString m_sMainTitle;
	RString m_sArtist;
	RString m_sSubTitle;
	int m_iSelectMode;
	void SelectUserSong();

	// (common) LAN-related
	StepManiaLanServer *LANserver;
	void GetListOfLANServers( vector<NetServerInfo>& AllServers );

	// new code
	void SendPacket(NetworkPacket *p);

private:
#if !defined(WITHOUT_NETWORKING)

	void StartUp();

	// core of the networking experience
	EzSockets *NetPlayerClient;
	NetworkPacket m_packet;

	RString m_ServerName;
	int m_ServerVersion;
	int m_startupStatus;	// Used to see if attempt was successful or not.
	int m_iSalt;	// Legacy, but might be useful in SMO-SSC

	// common stuff + Legacy packet parsing
	void ProcessInput();
	// (Legacy) for ReportScore()
	SMOStepType TranslateStepType(int score);

	// Currently unused, but need to stay. (who? -aj)
	// why? -aj
	/*
	int m_playerID;
	int m_step;
	int m_score;
	int m_combo;
	*/

	// (legacy) scoreboard
	bool m_scoreboardchange[NUM_NSScoreBoardColumn];

	// (common) LAN
	EzSockets *BroadcastReception;
	vector<NetServerInfo> m_vAllLANServers;
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
