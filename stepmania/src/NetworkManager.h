#ifndef NETWORKMANAGER_H
#define NETWORKMANAGER_H

#include "SDL_net.h"

class NetworkManager
{
public:
	NetworkManager();
	~NetworkManager();

	bool Connect();

private:
	int	i;
};

extern NetworkManager* NETWORKMAN;
#endif
