#include "stdafx.h"
#include "SDL.h"
#include "RageNetworkServer.h"
#include "RageLog.h"

const int SM_PORT = 573;	// "Ko" + "na" + "mitsu"

int main(int argc, char* argv[])
{
	SDL_Init(0);	/* Fire up the SDL, but don't actually start any subsystems. */

	LOG = new RageLog;
	LOG->ShowConsole();

	LOG->Trace( "smserver starting up..." );


	SERVER = new RageNetworkServer;
	SERVER->Listen( SM_PORT );


	SDL_Event	event;
	while( 1 )
	{
		// process all queued events
		while(SDL_PollEvent(&event))
		{
			switch(event.type)
			{
			case SDL_KEYDOWN:
				SERVER->DisconnectAllClients();
				exit(1);
				break;
			}
		}
		SERVER->Update( 0.1f );
		SERVER->TellClientsNetGameState();
		::Sleep( 100 );
	}
}