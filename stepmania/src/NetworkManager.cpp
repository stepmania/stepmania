#include "global.h"
/*
-----------------------------------------------------------------------------
 Class: NetworkManager

 Desc: See Header.

 Copyright (c) 2001-2003 by the person(s) listed below.  All rights reserved.
	Kevin Slaughter
-----------------------------------------------------------------------------
*/

#include "GameState.h"
#include "NetworkManager.h"
#include "RageLog.h"
#include "RageException.h"
#include "SongManager.h"
NetworkManager*	NETWORKMAN = NULL;


NetworkManager::NetworkManager()
{
	LOG->Trace("NetworkManager::NetworkManager");
	if( SDLNet_Init()==-1 )
		throw;
}

NetworkManager::~NetworkManager()
{
	LOG->Trace("NetworkManager::~NetworkManager");
	SDLNet_Quit();
}

bool NetworkManager::Connect()
{
	LOG->Trace("NetworkManager::Connect()");
	RageException*	Bitcher;

	IPaddress ip;
	TCPsocket tcpsock;
	SDLNet_SocketSet socketset;

	/* Allocate the socket set for polling the network */
	socketset = SDLNet_AllocSocketSet(2);
	if ( socketset == NULL )
		Bitcher->Throw("Couldn't create socket set: %s\n",SDLNet_GetError());
	SDLNet_TCP_AddSocket(socketset, tcpsock);



	if(SDLNet_ResolveHost(&ip,"127.0.0.1",6288))	// Establish a connection.
	{
		LOG->Trace("SDLNet_ResolveHost: %s\n", SDLNet_GetError());
		Bitcher->Throw("SDLNet_ResolveHost() FAILED.  --> %s", SDLNet_GetError());
		throw;
	}

	tcpsock=SDLNet_UDP_Open(&ip);
	if(!tcpsock)	// Make sure we opened a valid connection.
		Bitcher->Throw("NetworkManager::SDLNet_TCP_Open: %s\n", SDLNet_GetError());



	// Test send & recv data //
	Uint8 data[512];
	char*	test[1+1+256];
//	int pos
	int len;
//	int used;

	/* Has the connection been lost with the server? */
		test[3] = "$";
		SDLNet_TCP_Send(tcpsock, test, 4);
	LOG->Trace("\n\n\nDATA BEING SENT..");
		len = SDLNet_TCP_Recv(tcpsock, (char *)data, 512);
	LOG->Trace("\n\n\nDATA RECEIVED!!!");
/*	if ( len <= 0 ) {
		//SDLNet_TCP_DelSocket(socketset, tcpsock);
		SDLNet_TCP_Close(tcpsock);
		tcpsock = NULL;
		LOG->Trace("NetworkManager:: Connection lost!\n");
	}
	else
	{
		pos = 0;
		while ( len > 0 )
		{
			used = 1;
			pos += used;
			len -= used;
			if ( used == 0 )
			{
				// We might lose data here.. oh well,
				// we got a corrupt packet from server
				
				len = 0;
			}
		}
	}
*/
	return true;
}
