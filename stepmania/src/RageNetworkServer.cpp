#include "stdafx.h"
/*
-----------------------------------------------------------------------------
 Class: RageNetworkServer

 Desc: See header.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Brendan Walker
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "RageNetworkServer.h"
#include "RageUtil.h"
#include "RageException.h"
#include "RageLog.h"
#include "RageException.h"
#include "RageNetworkPacket.h"

#include "SDL_net-1.2.4/include/SDL_net.h"
//#ifdef _DEBUG
//#pragma comment(lib, "SDL_net-1.2.4/lib/SDL_netd.lib")
//#else
#pragma comment(lib, "SDL_net-1.2.4/lib/SDL_net.lib")
//#endif

#ifdef _DEBUG
#pragma comment(lib, "SDL-1.2.5/lib/SDLmaind.lib")
#else
#pragma comment(lib, "SDL-1.2.5/lib/SDLmain.lib")
#endif

/* Pull in all of our SDL libraries here. */
#ifdef _DEBUG
#pragma comment(lib, "SDL-1.2.5/lib/SDLd.lib")
#pragma comment(lib, "SDL_image-1.2/SDL_imaged.lib")
#else
#pragma comment(lib, "SDL-1.2.5/lib/SDL.lib")
#pragma comment(lib, "SDL_image-1.2/SDL_image.lib")
#endif


RageNetworkServer*		SERVER	= NULL;

const int MAX_CLIENTS = 16;

/*
///////////////////////////////
//
//  THIS IS UNTESTED SINCE THE CONVERSION TO SDL_net!!!!!
//
///////////////////////////////
*/

RageNetworkServer::RageNetworkServer()
{
	m_listenSock = 0;
	m_listenSockSet = NULL;
	m_clientSocksSet = NULL;

	/* Don't do this; it'll fire up things we might not want.  Let StepMania.cpp
	 * do the initial SDL config. */
	// SDL_Init(0);	// this may have already been init'd somewhere else

	if( SDLNet_Init() < 0 )
		RageException::Throw("SDLNet_Init: %s\n", SDLNet_GetError());
	
	// allocate socket sets
	m_listenSockSet = SDLNet_AllocSocketSet(1);
	if(!m_listenSockSet)
		RageException::Throw("SDLNet_AllocSocketSet: %s\n", SDLNet_GetError());

	m_clientSocksSet = SDLNet_AllocSocketSet(MAX_CLIENTS);
	if(!m_clientSocksSet)
		RageException::Throw("SDLNet_AllocSocketSet: %s\n", SDLNet_GetError());
}

RageNetworkServer::~RageNetworkServer()
{
	StopListening();
	DisconnectAllClients();
	SDLNet_FreeSocketSet( m_listenSockSet );
	SDLNet_FreeSocketSet( m_clientSocksSet );
	SDLNet_Quit();
}


void RageNetworkServer::StopListening()
{
	if( m_listenSock == NULL )
		return;

	SDLNet_TCP_DelSocket( m_listenSockSet, m_listenSock );
	SDLNet_TCP_Close( m_listenSock );
	m_listenSock = NULL;
}

void RageNetworkServer::DisconnectAllClients()
{
	// I hope passing NULL to these is OK...
	for( unsigned int i=0; i<m_clientSocks.size(); i++ )
	{
		SDLNet_TCP_DelSocket( m_clientSocksSet, m_clientSocks[i] );
		SDLNet_TCP_Close( m_clientSocks[i] );
	}
	m_clientSocks.erase( m_clientSocks.begin(), m_clientSocks.end() );
}


void RageNetworkServer::Listen(unsigned short port)
{
	StopListening();

	IPaddress ip;
	if( SDLNet_ResolveHost(&ip,NULL,port)==-1 )
		RageException::Throw("SDLNet_ResolveHost: %s\n", SDLNet_GetError());

	m_listenSock=SDLNet_TCP_Open(&ip);
	if( !m_listenSock ) 
		RageException::Throw("SDLNet_TCP_Open: %s\n", SDLNet_GetError());

	SDLNet_TCP_AddSocket( m_listenSockSet, m_listenSock );
}

void RageNetworkServer::Update( float fDeltaTime )
{
	// accept
	if( m_listenSock )
	{
		TCPsocket new_sock = SDLNet_TCP_Accept( m_listenSock ) ;
		if( new_sock != 0 )
		{ 
			m_clientSocks.push_back( new_sock );
			SDLNet_TCP_AddSocket( m_clientSocksSet, new_sock );

			LOG->Trace( "Accepting new client." );

			m_NetGameState.num_players = m_clientSocks.size();
		}
	}

	// receive
	if( m_clientSocks.size() > 0 )	// there are some clients connected
	{
		int numready = SDLNet_CheckSockets(m_clientSocksSet, 0);
		if(numready==-1) 
		{
			// XXX sorry, this was really noisy ... - glenn
//			LOG->Warn("SDLNet_CheckSockets: %s\n", SDLNet_GetError());
		}
		else if( numready > 0 )
		{
			for( unsigned int i=0; i<m_clientSocks.size(); i++ )
			{
				if( SDLNet_SocketReady(m_clientSocks[i]) ) 
				{
					RageNetworkPacket packet;
					SDLNet_TCP_Recv( m_clientSocks[i], &packet, sizeof(RageNetworkPacket) ); 
					
					switch( packet.type )
					{
					case RageNetworkPacket::update_player:
						m_NetGameState.player[i] = packet.player;
						break;
					default:
						LOG->Warn( "Invalid packet received with type '%d'", packet.type );
						break;
					}
				}
			}
		}
	}
}

void RageNetworkServer::TellClientsNetGameState()
{
	// tell everyone about the new names
	RageNetworkPacket packet;
	packet.type = RageNetworkPacket::update_game;
	packet.game = m_NetGameState;
	this->SendToClients( &packet );
}

void RageNetworkServer::Send( TCPsocket& socket, RageNetworkPacket* pPacket ) 
{
}

void RageNetworkServer::SendToClients( RageNetworkPacket* pPacket ) 
{
	for( int i=m_clientSocks.size()-1; i>=0; i-- )		// iterate backwards in case we need to delete
	{
		int result = SDLNet_TCP_Send(m_clientSocks[i],pPacket,sizeof(RageNetworkPacket));
		if( result<int(sizeof(RageNetworkPacket)) )
		{
			LOG->Warn("SDLNet_TCP_Send: %s\n", SDLNet_GetError());

			LOG->Trace( "disconnecting %d", i );
			SDLNet_TCP_DelSocket( m_clientSocksSet, m_clientSocks[i] );
			SDLNet_TCP_Close( m_clientSocks[i] );
			m_clientSocks.erase( m_clientSocks.begin() + i );
		}
	}
}

