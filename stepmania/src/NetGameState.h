#ifndef NETGAMESTATE_H
#define NETGAMESTATE_H


#define MAX_PLAYERS 8
#define MAX_NAME_LENGTH 32


struct NetPlayerState
{
	char name[MAX_NAME_LENGTH];
	float score;
	int combo;
	bool bReady;
};

struct NetGameState
{
	int num_players;
	NetPlayerState player[MAX_PLAYERS];
};


#endif