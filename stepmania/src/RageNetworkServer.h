#ifndef RAGENETWORKSERVER_H
#define RAGENETWORKSERVER_H
/*
-----------------------------------------------------------------------------
 Class: RageNetworkServer

 Desc: Network transport layer.  Note that this currently has SM-specific 
	functionality.  All SM stuff should be moved out eventually.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

struct _TCPsocket;
typedef struct _TCPsocket *TCPsocket;

struct _SDLNet_SocketSet;
typedef struct _SDLNet_SocketSet *SDLNet_SocketSet;


#include "NetGameState.h"


class RageNetworkPacket;

class RageNetworkServer
{
public:
	RageNetworkServer();
	~RageNetworkServer();

	void Update( float fDeltaTime );

	void Listen(unsigned short port);
	void StopListening();

	void DisconnectAllClients();

	void TellClientsNetGameState();

	NetGameState	m_NetGameState;

protected:
	void Send( TCPsocket& socket, RageNetworkPacket* pPacket );
	void SendToClients( RageNetworkPacket* pPacket );

	TCPsocket			m_listenSock;
	SDLNet_SocketSet	m_listenSockSet;
	vector<TCPsocket>	m_clientSocks;
	SDLNet_SocketSet	m_clientSocksSet;
};


extern RageNetworkServer*		SERVER;	// global and accessable from anywhere in our program

#endif
