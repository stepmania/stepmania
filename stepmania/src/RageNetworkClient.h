#ifndef RAGENETWORKCLIENT_H
#define RAGENETWORKCLIENT_H
/*
-----------------------------------------------------------------------------
 Class: RageNetworkClient

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


class RageNetworkClient
{
public:
	RageNetworkClient();
	~RageNetworkClient();

	void Update( float fDeltaTime );

	void Connect(CString ip, unsigned short port);
	void Disconnect();

	void SendMyPlayerState( NetPlayerState ps );	// calling this implies bReady = true

	NetGameState	m_NetGameState;

protected:
	void SendToServer( RageNetworkPacket* pPacket );
	void Send( TCPsocket& socket, RageNetworkPacket* pPacket );


	TCPsocket			m_priSock;			// client: active socket.  server: listen socket
	SDLNet_SocketSet	m_priSockSet;
};


extern RageNetworkClient*		CLIENT;	// global and accessable from anywhere in our program

#endif
