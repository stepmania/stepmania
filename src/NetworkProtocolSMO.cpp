#include "global.h"
#include "NetworkProtocolSMO.h"
#include "LocalizedString.h"
#include "ezsockets.h"
#include "NetworkPacket.h"

#include "ProfileManager.h"
#include "RageLog.h"
#include "ScreenManager.h"
#include "Song.h"
#include "Course.h"
#include "GameState.h"
#include "StatsManager.h"
#include "Steps.h"
#include "ProductInfo.h"
#include "ScreenMessage.h"
#include "GameManager.h"
#include "MessageManager.h"
#include "arch/LoadingWindow/LoadingWindow.h"
#include "PlayerState.h"
#include "CryptManager.h"

/* NetworkPacket */
uint16_t NetworkPacket::Read2()
{
	if(Position >= MAX_PACKET_BUFFER_SIZE-1)
		return 0;

	uint16_t Temp;
	memcpy(&Temp, Data + Position, 2);
	Position+=2;
	return ntohs(Temp);
}

uint32_t NetworkPacket::Read4()
{
	if(Position >= MAX_PACKET_BUFFER_SIZE-3)
		return 0;

	uint32_t Temp;
	memcpy(&Temp, Data + Position, 4);
	Position+=4;
	return ntohl(Temp);
}

RString NetworkPacket::ReadString()
{
	RString TempStr;
	while((Position < MAX_PACKET_BUFFER_SIZE)&& (((char*)Data)[Position]!=0))
		TempStr = TempStr + (char)Data[Position++];

	++Position;
	return TempStr;
}

void NetworkPacket::Write1(uint8_t data)
{
	if(Position >= MAX_PACKET_BUFFER_SIZE)
		return;
	memcpy(&Data[Position], &data, 1);
	++Position;
}

void NetworkPacket::Write2(uint16_t data)
{
	if(Position >= MAX_PACKET_BUFFER_SIZE-1)
		return;
	data = htons(data);
	memcpy(&Data[Position], &data, 2);
	Position+=2;
}

void NetworkPacket::Write4(uint32_t data)
{
	if(Position >= MAX_PACKET_BUFFER_SIZE-3)
		return;

	data = htonl(data);
	memcpy(&Data[Position], &data, 4);
	Position+=4;
}

void NetworkPacket::WriteString(const RString& data)
{
	size_t index=0;
	while(Position < MAX_PACKET_BUFFER_SIZE && index < data.size())
		Data[Position++] = (unsigned char)(data.c_str()[index++]);
	Data[Position++] = 0;
}

void NetworkPacket::Clear()
{
	memset((void*)(&Data),0, MAX_PACKET_BUFFER_SIZE);
	Position = 0;
}

/* NetworkProtocolSMO */
NetworkProtocolSMO::NetworkProtocolSMO(LoadingWindow *ld)
{
	// initialize shit
	BroadcastReception = NULL;

	ld->SetText(INITIALIZING_CLIENT_NETWORK);
	NetPlayerClient = new EzSockets;
	NetPlayerClient->blocking = false;
	m_ServerVersion = 0;

	m_bUseSMServer = false;
	m_bIsSMOnline = false;
	FOREACH_PlayerNumber(pn)
		m_bIsSMOLoggedIn[pn] = false;
	m_iStartupStatus = 0;
	m_iActivePlayers = 0;

	// start it
	StartUp();
}

NetworkProtocolSMO::~NetworkProtocolSMO()
{
	// kill it
	if(m_bUseSMServer)
		NetPlayerClient->close();
	SAFE_DELETE(NetPlayerClient);

	if(BroadcastReception)
	{
		BroadcastReception->close();
		SAFE_DELETE(BroadcastReception);
	}
}

void NetworkProtocolSMO::StartUp()
{
	RString sServerIP;
	if( GetCommandlineArgument("netip", &ServerIP ))
		PostStartUp(sServerIP);

	BroadcastReception = new EzSockets;
	BroadcastReception->create(IPPROTO_UDP);
	BroadcastReception->bind(8765);
	BroadcastReception->blocking = false;
}

void NetworkProtocolSMO::PostStartUp(const RString& sServerIP)
{
	RString sAddress;
	unsigned short iPort;
	size_t cLoc = sServerIP.find( ':' );
	if( sServerIP.find(':') != RString::npos )
	{
		iPort = (unsigned short)atoi(sServerIP.substr(cLoc + 1).c_str());
		sAddress = ServerIP.substr(0, cLoc);
	}
	else
	{
		iPort = 8765;			// SMO default port
		sAddress = sServerIP;
	}
	LOG->Info("Attempting to connect to: %s, Port: %d", sAddress.c_str(), iPort);

	Disconnect();
	if(!Connect(sAddress.c_str(), iPort))
	{
		m_iStartupStatus = 2;
		LOG->Warn("Failed to connect to %s:%d",sAddress.c_str(), iPort);
		return;
	}

	FOREACH_PlayerNumber(pn)
		m_bIsSMOLoggedIn[pn] = false;
	m_bUseSMServer = true;
	m_iStartupStatus = 1;		// success!

	// okay this part gets specific.
	/* "If network play is desired and the connection works, halt until we know
	 * what server version we're dealing with."*/

	// send Hello packet
	SMOHello();

	/* "Block until response is received. Move mode to blocking in order to
	 * give CPU back to the system, and not wait." */
	bool bWaitingForResponse = true;
	NetPlayerClient->blocking = true;

	/* "The following packet must get through, so we block for it. If we are
	 * serving, we do not block for this. */
	NetPlayerClient->SendPack((char*)m_packet.Data, m_packet.Position);

	while(bWaitingForResponse)
	{
		m_packet.Clear();
		// Allow an exit if there is a problem on the socket:
		if(NetPlayerClient->ReadPack((char *)&m_packet,MAX_PACKET_BUFFER_SIZE) < 1)
			bWaitingForResponse = false;

		if(m_packet.Read1() == ServerOffset+SMOCMD_Hello)
			bWaitingForResponse = false;
		/* "Only allow passing on handshake; otherwise scoreboard updates and
		 * such will confuse us." */
	}
	NetPlayerClient->blocking = false;

	GetServerInfo();
}

void NetworkProtocolSMO::GetServerInfo()
{
	m_iServerVersion = m_packet.Read1();
	if(m_iServerVersion >= 128)
		m_bIsSMOnline = true;
	m_sServerName = m_packet.ReadString();
	m_iSalt = m_packet.Read4();
	LOG->Info("Server Version: %d %s", m_ServerVersion, m_ServerName.c_str());
}

bool NetworkProtocolSMO::Connect(const RString& address, unsigned short port)
{
	LOG->Info("Beginning connection to %s:%d",address,port);
	NetPlayerClient->create();
	m_bUseSMServer = NetPlayerClient->connect(address, port);
	return m_bUseSMServer;
}

void NetworkProtocolSMO::Disconnect()
{
	if(!m_bUseSMServer)
		return;
	m_iServerVersion = 0;
	m_bUseSMServer = false;
	m_bIsSMOnline = false;
	FOREACH_PlayerNumber(pn)
		m_bIsSMOLoggedIn[pn] = false;
	m_iStartupStatus = 0;
	NetPlayerClient->close();
}

// keep these metric section names for now... -freem
static LocalizedString CONNECTION_SUCCESSFUL("NetworkSyncManager", "Connection to '%s' successful.");
static LocalizedString CONNECTION_FAILED("NetworkSyncManager", "Connection failed.");
void NetworkProtocolSMO::DisplayStartupStatus()
{
	RString sMessage("");
	switch(m_iStartupStatus)
	{
		case 0:
			// Networking not attempted; ignore.
			return;
		case 1:
			sMessage = ssprintf(CONNECTION_SUCCESSFUL.GetValue(), m_ServerName.c_str());
			break;
		case 2:
			sMessage = CONNECTION_FAILED.GetValue();
			break;
	}
	SCREENMAN->SystemMessage(sMessage);
}

void NetworkProtocolSMO::Update(float fDeltaTime)
{
	if(m_bUseSMServer)
		ProcessInput();
}

static LocalizedString CONNECTION_DROPPED("NetworkSyncManager", "Connection to server dropped.");
void NetworkProtocolSMO::ConnectionDropped()
{
	SCREENMAN->SystemMessageNoAnimate(CONNECTION_DROPPED);
	m_bUseSMServer = false;
	m_bIsSMOnline = false;
	FOREACH_PlayerNumber(pn)
		m_bIsSMOLoggedIn[pn] = false;
	NetPlayerClient->close();
}

void NetworkProtocolSMO::ProcessInput()
{
	// If we're disconnected, don't bother.
	if ((NetPlayerClient->state!=NetPlayerClient->skCONNECTED) || NetPlayerClient->IsError())
	{
		ConnectionDropped();
		return;
	}

	// "load new data into buffer"
	NetPlayerClient->update();
	m_packet.Clear();

	int packetSize;
	while ((packetSize = NetPlayerClient->ReadPack((char *)&m_packet, MAX_PACKET_BUFFER_SIZE) ) > 0)
	{
		m_packet.size = packetSize;
		int command = m_packet.Read1();
		// Make sure command is a valid server command
		if(command < ServerOffset)
		{
			LOG->Trace("Invalid server command %d (< %d)",command,ServerOffset);
			break;
		}
		command -= ServerOffset;
		switch(command)
		{
			case SMOCMD_Ping: // Ping reply
				SMOPing();
				break;
			case SMOCMD_PingReply:			// response to server packet 0
			case SMOCMD_Hello:				// Taken care of earlier
			case SMOCMD_GameStartRequest:	// Taken care of elsewhere
				break;
			case SMOCMD_GameOverNotice:
				SMOGameOverNotice();
				break;
			case SMOCMD_GameStatusUpdate:	// Scoreboard Update
				SMOScoreboardUpdate();
				break;
			case SMOCMD_StyleUpdate:		// System message
				SMOSystemMessage();
				break;
			case SMOCMD_ChatMessage:
				SMOChatMessage();
				break;
			case SMOCMD_RequestStartGame:	// Select Song/Play song
				SMOChangeSong();
				break;
			case SMOCMD_UpdateUserList:
				SMOUpdateUserList();
				break;
			case SMOCMD_ScreenChange:
				SMOSelectMusic();
				break;
			case SMOCMD_SMOnline:
				SMOnlinePacket();
				break;
			case SMOCMD_Attack:
				SMOAttack();
				break;
		}
		m_packet.Clear();
	}
}

RString NetworkProtocolSMO::MD5Hex(const RString &sInput)
{
	return BinaryToHex(CryptManager::GetMD5ForString(sInput)).MakeUpper();
}

// Server commands (for ProcessInput)
void NetworkProtocolSMO::SMOPing()
{
	m_packet.Clear();
	m_packet.Write1( SMOCMD_PingReply );
	NetPlayerClient->SendPack((char*)m_packet.Data, m_packet.Position);
}

// case NSCGON:
void NetworkProtocolSMO::SMOGameOverNotice()
{
	int PlayersInPack = m_packet.Read1();
	m_EvalPlayerData.resize(PlayersInPack);

	for(int i = 0; i < PlayersInPack; ++i)
		m_EvalPlayerData[i].iNameIndex = m_packet.Read1();

	for(int i = 0; i < PlayersInPack; ++i)
		m_EvalPlayerData[i].iScore = m_packet.Read4();

	for(int i = 0; i < PlayersInPack; ++i)
		m_EvalPlayerData[i].iGrade = m_packet.Read1();

	for(int i = 0; i < PlayersInPack; ++i)
		m_EvalPlayerData[i].difficulty = (Difficulty)m_packet.Read1();

	for(int j = 0; j < NUM_SMO_TAP_SCORES; ++j)
		for(int i = 0; i < PlayersInPack; ++i)
			m_EvalPlayerData[i].iTapScores[j] = m_packet.Read2();

	for(int i = 0; i < PlayersInPack; ++i)
		m_EvalPlayerData[i].sPlayerOptions = m_packet.ReadString();

	SCREENMAN->SendMessageToTopScreen( SM_GotEval );
}

// case NSCGSU: "Scoreboard Update"
void NetworkProtocolSMO::SMOScoreboardUpdate()
{
	// lots of shit goes here
}

// case NSCSU: "System message from server"
void NetworkProtocolSMO::SMOSystemMessage()
{
	RString sSysMsg = m_packet.ReadString();
	SCREENMAN->SystemMessage( sSysMsg );
}

// case NSCCM: "Chat message from server"
void NetworkProtocolSMO::SMOChatMessage()
{
	m_sChatText += m_packet.ReadString() + " \n ";
	// "10000 chars backlog should be more than enough"
	// sure.... -freem
	m_sChatText = m_sChatText.Right(10000);
	SCREENMAN->SendMessageToTopScreen( SM_AddToChat );
}

// case NSCRSG: "Select Song/Play Song"
void NetworkProtocolSMO::SMOChangeSong()
{
	m_iSelectMode = m_packet.Read1();
	m_sMainTitle = m_packet.ReadString();
	m_sArtist = m_packet.ReadString();
	m_sSubTitle = m_packet.ReadString();
	SCREENMAN->SendMessageToTopScreen(SM_ChangeSong);
}

void NetworkProtocolSMO::SMOUpdateUserList()
{
	/*int ServerMaxPlayers=*/m_packet.Read1();
	int PlayersInThisPacket=m_packet.Read1();
	m_iActivePlayer.clear();
	m_iPlayerStatus.clear();
	m_sPlayerNames.clear();
	m_iActivePlayers = 0;
	for(int i=0; i<PlayersInThisPacket; ++i)
	{
		int PStatus = m_packet.Read1();
		/* Status list:
		 * 0	Inative (no info)
		 * 1	Active (known user)
		 * 2	In Selection Screen
		 * 3	In Options
		 * 4 	In Evaluation
		 */
		if (PStatus > 0)
		{
			m_iActivePlayers++;
			m_iActivePlayer.push_back(i);
		}
		m_iPlayerStatus.push_back(PStatus);
		m_sPlayerNames.push_back(m_packet.ReadString());
	}
	SCREENMAN->SendMessageToTopScreen( SM_UsersUpdate );
}

void NetworkProtocolSMO::SMOSelectMusic()
{
	RString sStyleName, sGameName;
	sGameName = m_packet.ReadString();
	sStyleName = m_packet.ReadString();

	GAMESTATE->SetCurGame(GAMEMAN->StringToGame(sGameName));
	GAMESTATE->SetCurrentStyle(GAMEMAN->GameAndStringToStyle(GAMESTATE->m_pCurGame,sStyleName));

	// xxx: hardcoded screen names make AJ sad.
	SCREENMAN->SetNewScreen("ScreenNetSelectMusic");
}

void NetworkProtocolSMO::SMOnlinePacket()
{
	m_SMOnlinePacket.size = packetSize - 1;
	m_SMOnlinePacket.Position = 0;
	memcpy(m_SMOnlinePacket.Data, (m_packet.Data + 1), packetSize-1);
	LOG->Trace("Received SMOnline Command: %d, size:%d", command, packetSize - 1);
	SCREENMAN->SendMessageToTopScreen(SM_SMOnlinePack);
}

void NetworkProtocolSMO::SMOAttack()
{
	PlayerNumber iPlayerNumber = (PlayerNumber)m_packet.Read1();

	if(GAMESTATE->IsPlayerEnabled(iPlayerNumber))
	{
		Attack a;
		a.fSecsRemaining = float(m_packet.Read4()) / 1000.0f;
		a.bGlobal = false;
		a.sModifiers = m_packet.ReadString();
		GAMESTATE->m_pPlayerState[iPlayerNumber]->LaunchAttack(a);
	}
	m_packet.Clear();
}

// Client commands
void NetworkProtocolSMO::SMOHello()
{
	m_packet.Clear();
	m_packet.Write1(SMOCMD_Hello);
	m_packet.Write1(ProtocolVersion);
	m_packet.WriteString(RString(PRODUCT_ID_VER));
}

void NetworkProtocolSMO::ReportStyle()
{
	if(!m_bUseSMServer)
		return;

	LOG->Trace("Sending Style to server");
	m_packet.Clear();
	m_packet.Write1(SMOCMD_StyleUpdate);
	m_packet.Write1((int8_t)GAMESTATE->GetNumPlayersEnabled());

	FOREACH_EnabledPlayer(pn)
	{
		m_packet.Write1((uint8_t)pn);
		m_packet.WriteString(GAMESTATE->GetPlayerDisplayName(pn));
	}

	NetPlayerClient->SendPack((char*)&m_packet.Data, m_packet.Position);
}

void NetworkProtocolSMO::SendSMOnline()
{
	m_packet.Position = m_SMOnlinePacket.Position + 1;
	memcpy((m_packet.Data + 1), m_SMOnlinePacket.Data, m_SMOnlinePacket.Position);
	m_packet.Data[0] = SMOCMD_SMOnline;
	NetPlayerClient->SendPack((char*)&m_packet.Data, m_packet.Position);
}

void NetworkProtocolSMO::SendChat(const RString& sMessage)
{
	m_packet.Clear();
	m_packet.Write1(SMOCMD_ChatMessage);
	m_packet.WriteString(sMessage);
	NetPlayerClient->SendPack((char*)&m_packet.Data, m_packet.Position); 
}

// formerly RoomInfoDisplay::RequestRoomInfo
void NetworkProtocolSMO::RequestRoomInfo(const RString& sName)
{
	m_SMOnlinePacket.Clear();
	m_SMOnlinePacket.Write1((uint8_t)3) // Request room info
	m_SMOnlinePacket.WriteString(sName);
	SendSMOnline();
}

void NetworkProtocolSMO::ChangeScreen(int i)
{
	m_packet.Clear();
	m_packet.Write1(SMOCMD_ScreenChange);
	m_packet.Write1((uint8_t)i);
	NetPlayerClient->SendPack((char*)m_packet.Data, m_packet.Position);
}

void NetworkProtocolSMO::ReportPlayerOptions()
{
	m_packet.Clear();
	m_packet.Write1(SMOCMD_ChangePlayerOptions);
	FOREACH_PlayerNumber(pn)
		m_packet.WriteString(GAMESTATE->m_pPlayerState[pn]->m_PlayerOptions.GetCurrent().GetString());
	NetPlayerClient->SendPack((char*)&m_packet.Data, m_packet.Position); 
}

void NetworkProtocolSMO::StartRequest(short iPosition)
{
	if(!m_bUseSMServer)
		return;

	if(GAMESTATE->m_bDemonstrationOrJukebox)
		return;

	LOG->Trace( "Requesting Start from Server." );
	m_packet.Clear();
	m_packet.Write1(SMOCMD_GameStartRequest);
	unsigned char ctr=0;

	Steps* tSteps;
	tSteps = GAMESTATE->m_pCurSteps[PLAYER_1];
	if(tSteps!=NULL && GAMESTATE->IsPlayerEnabled(PLAYER_1))
		ctr = uint8_t(ctr+tSteps->GetMeter()*16);

	tSteps = GAMESTATE->m_pCurSteps[PLAYER_2];
	if(tSteps!=NULL && GAMESTATE->IsPlayerEnabled(PLAYER_2))
		ctr = uint8_t(ctr+tSteps->GetMeter());

	m_packet.Write1(ctr);
	ctr=0;

	tSteps = GAMESTATE->m_pCurSteps[PLAYER_1];
	if(tSteps!=NULL && GAMESTATE->IsPlayerEnabled(PLAYER_1))
		ctr = uint8_t(ctr + (int)tSteps->GetDifficulty()*16);

	tSteps = GAMESTATE->m_pCurSteps[PLAYER_2];
	if(tSteps!=NULL && GAMESTATE->IsPlayerEnabled(PLAYER_2))
		ctr = uint8_t(ctr + (int)tSteps->GetDifficulty());

	m_packet.Write1(ctr);

	// "Notify server if this is for sync or not."
	ctr = char(position*16);
	m_packet.Write1(ctr);

	if(GAMESTATE->m_pCurSong != NULL)
	{
		m_packet.WriteString(GAMESTATE->m_pCurSong->m_sMainTitle);
		m_packet.WriteString(GAMESTATE->m_pCurSong->m_sSubTitle);
		m_packet.WriteString(GAMESTATE->m_pCurSong->m_sArtist);
	}
	else
	{
		m_packet.WriteString("");
		m_packet.WriteString("");
		m_packet.WriteString("");
	}

	if(GAMESTATE->m_pCurCourse != NULL)
		m_packet.WriteString(GAMESTATE->m_pCurCourse->GetDisplayFullTitle());
	else
		m_packet.WriteString(RString());

	// "Send Player (and song) Options"
	m_packet.WriteString(GAMESTATE->m_SongOptions.GetCurrent().GetString());

	int players = 0;
	FOREACH_PlayerNumber(p)
	{
		++players;
		m_packet.WriteString(GAMESTATE->m_pPlayerState[p]->m_PlayerOptions.GetCurrent().GetString());
	}
	for (int i = 0; i < 2-players; ++i)
		m_packet.WriteString("");	//Write a NULL if no player

	// "This needs to be reset before ScreenEvaluation could possibly be called"
	m_EvalPlayerData.clear();

	// Block until go is recieved.
	bool bWaitingForReply = true;
	NetPlayerClient->blocking = true;

	// "The following packet HAS to get through, so we turn blocking on for it as well"
	NetPlayerClient->SendPack((char*)&m_packet.Data, m_packet.Position);
	LOG->Trace("Waiting for RECV");
	m_packet.Clear();
	while(bWaitingForReply)
	{
		m_packet.Clear();
		// "Also allow exit if there is a problem on the socket"
		if(NetPlayerClient->ReadPack((char *)&m_packet, NETMAXBUFFERSIZE) < 1)
			bWaitingForReply = false;

		if (m_packet.Read1() == (ServerOffset + SMOCMD_GameStartRequest))
			bWaitingForReply = false;

		// Only allow passing on Start request; otherwise scoreboard updates
		// and such will confuse us.
	}
	NetPlayerClient->blocking = false;
}

void NetworkProtocolSMO::SelectUserSong()
{
	m_packet.Clear();
	m_packet.Write1(SMOCMD_RequestStartGame);
	m_packet.Write1((uint8_t)m_iSelectMode);
	m_packet.WriteString(m_sMainTitle);
	m_packet.WriteString(m_sArtist);
	m_packet.WriteString(m_sSubTitle);
	NetPlayerClient->SendPack((char*)&m_packet.Data, m_packet.Position);
}

void NetworkProtocolSMO::ReportScore(int iPlayerID, int iStep, int iCombo, float fOffset)
{
	if(!m_bUseSMServer)
		return;

	LOG->Trace(ssprintf("Player ID %i combo = %i", iPlayerID, iCombo));
	m_packet.Clear();

	m_packet.Write1(SMOCMD_GameStatusUpdate);
	iStep = TranslateStepType(iStep);
	uint8_t ctr = (uint8_t)(iPlayerID * 16 + iStep - (SMOST_HitMine - 1));

	if (STATSMAN->m_CurStageStats.m_player[iPlayerID].m_bFailed)
		ctr = uint8_t(112);	// "Code for failed (failed constant seems not to work)"

	m_packet.Write1(ctr );
	m_packet.Write4(iScore);
	m_packet.Write2((uint16_t)iCombo);
	m_packet.Write2((uint16_t)m_iPlayerLife[iPlayerID]);

	// Offset Info (if a 0 is sent, disregard data)

	// ASSUMED: No step will be more than 16 seconds off center.
	// If this assumption is false, read 16 seconds in either direction.
	int iOffset = int((fOffset+16.384)*2000.0f);

	if(iOffset > 65535)
		iOffset = 65535;
	if(iOffset < 1)
		iOffset = 1;

	// Report 0 if a hold or miss (don't forget mines either)
	if( step == SMOST_HitMine || step > SMOST_W1 )
		iOffset = 0;

	m_packet.Write2((uint16_t)iOffset);
	NetPlayerClient->SendPack( (char*)m_packet.Data, m_packet.Position ); 
}

SMOStepType NetworkProtocolSMO::TranslateStepType(int score)
{
	// SMO decides to have its own TapNoteScores... weird.
	switch(score)
	{
		case TNS_HitMine:
			return SMOST_HitMine;
		case TNS_AvoidMine:
			return SMOST_AvoidMine;
		case TNS_Miss:
			return SMOST_Miss;
		case TNS_W5:
			return SMOST_W5;
		case TNS_W4:
			return SMOST_W4;
		case TNS_W3:
			return SMOST_W3;
		case TNS_W2:
			return SMOST_W2;
		case TNS_W1:
			return SMOST_W1;
		case HNS_LetGo+TapNoteScore_Invalid:
			return SMOST_LetGo;
		case HNS_Held+TapNoteScore_Invalid:
			return SMOST_Held;
		default:
			return SMOST_Unused;
	}
}

void NetworkProtocolSMO::ReportSongOver()
{
	if(!m_bUseSMServer)
		return;

	m_packet.Clear();
	m_packet.Write1(SMOCMD_GameOverNotice);
	NetPlayerClient->SendPack((char*)&m_packet.Data, m_packet.Position);
	return;
}

/*
 * (c) 2010 AJ Kelly
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
