/* NetworkSyncManager - Uses ezsockets for primitive song syncing and score reporting. */

#ifndef NetworkSyncManager_H
#define NetworkSyncManager_H

#include "PlayerNumber.h"
class LoadingWindow;

const int NETPROTOCOLVERSION=1;
const int NETMAXBUFFERSIZE=1020; //1024 - 4 bytes for EzSockets

enum NSScoreBoardColumn
{
	NSSB_NAMES=0,
	NSSB_COMBO,
	NSSB_GRADE,
	NUM_NSSB_CATEGORIES
};
#define FOREACH_NSScoreBoardColumn( sc ) FOREACH_ENUM( NSScoreBoardColumn, NUM_NSSB_CATEGORIES, sc )

class EzSockets;

class NetworkSyncManager 
{
public:
	NetworkSyncManager( LoadingWindow *ld = NULL );
	~NetworkSyncManager();

    //If "useSMserver" then send score to server
	void ReportTiming(float offset, int PlayerNumber);
	void ReportScore(int playerID, int step, int score, int combo);	
	void ReportSongOver();	//Report to server that song is over
	void ReportStyle();		//Report to server the style, players, and names
	void StartRequest(short position);	//Request a start.  Block until granted.
	bool Connect(const CString& addy, unsigned short port); // Connect to SM Server

	void PostStartUp(CString ServerIP);

	void CloseConnection();

	void DisplayStartupStatus();	//Used to note user if connect attempt was sucessful or not.

	int m_playerLife[NUM_PLAYERS];	//Life

	void Update(float fDeltaTime);

	bool useSMserver;


	//Used togeather for 
	bool ChangedScoreboard(int Column);	//If scoreboard changed since this function last called, then true.
	CString m_Scoreboard[NUM_NSSB_CATEGORIES];
private:
#if !defined(WITHOUT_NETWORKING)

	void ProcessInput();

	void StartUp();

	float m_lastOffset[2];	//This is used to determine how much
						//the last step was off.

	int m_playerID;  //these are currently unused, but need to stay
	int m_step;
	int m_score;
	int m_combo;
    
	int m_startupStatus;	//Used to see if attempt was sucessful or not.

	bool m_scoreboardchange[NUM_NSSB_CATEGORIES];

	CString m_ServerName;
 
    EzSockets *NetPlayerClient;

	int m_ServerVersion; //ServerVersion

	bool Listen(unsigned short port);


	//This is it's own type, so that more info can 
	//be put at the end of it; after the data.
	struct NetPacket
	{
		unsigned char Data[NETMAXBUFFERSIZE];	//Data

		int Position;				//Other info
	};

	//We only want to create this once per instance.
	//No need to allocate and deallocate one all the time.
	NetPacket m_packet;

	//Commands used to operate on NetPackets
	uint8_t Read1(NetPacket &Packet);
	uint16_t Read2(NetPacket &Packet);
	uint32_t Read4(NetPacket &Packet);
	CString ReadNT(NetPacket &Packet);

	void Write1(NetPacket &Packet, uint8_t Data);
	void Write2(NetPacket &Packet, uint16_t Data);
	void Write4(NetPacket &Packet, uint32_t Data);
	void WriteNT(NetPacket &Packet, CString Data);

	inline void ClearPacket(NetPacket &Packet)	{ memset((void*)(&Packet),0, sizeof(NetPacket)); }

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
