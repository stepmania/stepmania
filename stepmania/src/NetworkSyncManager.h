/* NetworkSyncManager - Uses ezsockets for primitive song syncing and score reporting. */

#ifndef NetworkSyncManager_H
#define NetworkSyncManager_H

#include "PlayerNumber.h"

class EzSockets;

class NetworkSyncManager 
{
public:
	NetworkSyncManager();
	~NetworkSyncManager();

    //If "useSMserver" then send score to server
	void ReportScore(int playerID, int step, int score, int combo);	
	void ReportSongOver();	//Report to server that song is over
	void StartRequest();	//Request a start.  Block until granted.
	bool Connect(const CString& addy, unsigned short port); // Connect to SM Server
	void SendSongs();  //Send song list to server (And exit) 

	void PostStartUp(CString ServerIP);

	void CloseConnection();

	void DisplayStartupStatus();	//Used to note user if connect attempt was sucessful or not.

	int m_playerLife[NUM_PLAYERS];	//Life


	void Update(float fDeltaTime);

private:
#if !defined(WITHOUT_NETWORKING)
	void StartUp();

	int m_playerID;  //these are currently unused, but need to stay
	int m_step;
	int m_score;
	int m_combo;
    
	int m_startupStatus;	//Used to see if attempt was sucessful or not.

	bool useSMserver;
 
    EzSockets *NetPlayerClient;

	int m_ServerVersion; //ServerVersion

	bool Listen(unsigned short port);

    
    struct netHolder		//Data structure used for sending data to server
    {

		//NOTICE: Order of variables in this struct matters
		//if order is changed, server would need to be re-written
        int m_playerID;	//PID (also used for Commands)
        int m_step;		//SID (StepID, 0-6 for Miss to Marv, 7,8 for boo and ok)
        int m_score;	//Player's Score
        int m_combo;	//Player's Current Combo
		int m_life;		//Player's Life (AND GRADE) 
			//m_life = [16-bit int life] <- LSB [16-bit int grade] <-MSB
    };

	struct netName			//Data structure used for 16-character strings.
	{
		int m_packID;
		char m_data[16];
	};

	struct netHolderFlash
	{
		char data[73];
	};
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
