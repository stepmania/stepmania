#include "stdafx.h"
/*
-----------------------------------------------------------------------------
 Class: RageNetworkClient

 Desc: See header.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Brendan Walker
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "RageNetworkClient.h"
#include "RageUtil.h"
#include "RageException.h"
#include "RageLog.h"
#include "RageTimer.h"
#include "RageException.h"
#include "GameState.h"			// remove this dependency later
#include "RageNetworkPacket.h"

#include "SDL_net-1.2.4/include/SDL_net.h"
//#ifdef _DEBUG
//#pragma comment(lib, "SDL_net-1.2.4/lib/SDL_netd.lib")
//#else
#pragma comment(lib, "SDL_net-1.2.4/lib/SDL_net.lib")
//#endif


RageNetworkClient*		CLIENT	= NULL;

const int MAX_CLIENTS = 16;

/*
///////////////////////////////
//
//  THIS IS UNTESTED SINCE THE CONVERSION TO SDL_net!!!!!
//
///////////////////////////////
*/

RageNetworkClient::RageNetworkClient()
{
	m_priSock = 0;
	m_priSockSet = NULL;

	/* Don't do this; it'll fire up things we might not want.  Let StepMania.cpp
	 * do the initial SDL config. */
	// SDL_Init(0);	// this may have already been init'd somewhere else

	if( SDLNet_Init() < 0 )
		throw RageException("SDLNet_Init: %s\n", SDLNet_GetError());
	
	// allocate socket sets
	m_priSockSet = SDLNet_AllocSocketSet(1);
	if(!m_priSockSet)
		throw RageException("SDLNet_AllocSocketSet: %s\n", SDLNet_GetError());
}

RageNetworkClient::~RageNetworkClient()
{
	Disconnect();
	SDLNet_FreeSocketSet( m_priSockSet );
	SDLNet_Quit();
}


void RageNetworkClient::Disconnect()
{
	if( m_priSock == NULL )
		return;

	SDLNet_TCP_DelSocket( m_priSockSet, m_priSock );
	SDLNet_TCP_Close( m_priSock );
	m_priSock = NULL;
}

void RageNetworkClient::Connect(CString host, unsigned short port) 
{
	Disconnect();

	char szHost[1024];
	strcpy( szHost, host );

	IPaddress ip;
	if( SDLNet_ResolveHost(&ip,szHost,port)==-1 )
	{
		LOG->Warn("SDLNet_ResolveHost: %s\n", SDLNet_GetError());
		return;
	}

	m_priSock=SDLNet_TCP_Open(&ip);
	if( !m_priSock ) 
	{
		LOG->Warn("SDLNet_TCP_Open: %s\n", SDLNet_GetError());
		return;
	}

	SDLNet_TCP_AddSocket( m_priSockSet, m_priSock );

	return;
}

void RageNetworkClient::Update( float fDeltaTime )
{
	// receive
	int numready = SDLNet_CheckSockets(m_priSockSet, 0);
	if(numready==-1) 
	{
		LOG->Warn("SDLNet_CheckSockets: %s\n", SDLNet_GetError());
	}
	else if( numready )
	{
		if( SDLNet_SocketReady(m_priSock) ) 
		{
			RageNetworkPacket packet;
			SDLNet_TCP_Recv( m_priSock, &packet, sizeof(RageNetworkPacket) ); 

			switch( packet.type )
			{
			case RageNetworkPacket::update_game:
				this->m_NetGameState = packet.game;
				break;
			default:
				LOG->Warn( "Invalid packet received with type '%d'", packet.type );
				break;
			}
		}
	}
}

void RageNetworkClient::SendMyPlayerState( NetPlayerState ps )
{
	RageNetworkPacket packet;
	packet.type = RageNetworkPacket::update_player;
	packet.player = ps;
	SendToServer( &packet );
}

void RageNetworkClient::Send( TCPsocket& socket, RageNetworkPacket* pPacket ) 
{
	int result = SDLNet_TCP_Send(socket,pPacket,sizeof(RageNetworkPacket));
	if( result<int(sizeof(RageNetworkPacket)) )
	{
		LOG->Warn("SDLNet_TCP_Send: %s\n", SDLNet_GetError());
		// It may be good to disconnect sock because it is likely invalid now.
	}
}

void RageNetworkClient::SendToServer( RageNetworkPacket* pPacket ) 
{
	Send( m_priSock, pPacket );
}
