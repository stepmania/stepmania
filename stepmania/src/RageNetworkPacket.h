#ifndef RAGENETWORKPACKET
#define RAGENETWORKPACKET


#include "NetGameState.h"


const int MAX_DATA_SIZE = sizeof(NetGameState);

class RageNetworkPacket
{
public:
	RageNetworkPacket() { type=invalid; }
	~RageNetworkPacket() {}

	enum { 
		invalid,
		update_player,		// sent by clients
		update_game,		// sent by server
		go_to_next_screen	// sent by server
	} type;
	union
	{
//		char data[MAX_DATA_SIZE];
		NetGameState game;
		NetPlayerState player;
	};
};

#endif