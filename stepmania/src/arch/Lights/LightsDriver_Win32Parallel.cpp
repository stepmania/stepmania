#include "global.h"
/*
-----------------------------------------------------------------------------
 Class: LightsDriver_Win32Parallel

 Desc: See header.

 Copyright (c) 2003 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "LightsDriver_Win32Parallel.h"
#include "windows.h"
#include "RageUtil.h"


HINSTANCE hDLL = NULL;

typedef void (WINAPI PORTOUT)(short int Port, char Data);
PORTOUT* PortOut = NULL;
typedef short int (WINAPI ISDRIVERINSTALLED)();
ISDRIVERINSTALLED* IsDriverInstalled = NULL;

const int LIGHTS_PER_PARALLEL_PORT = 8;
const int MAX_PARALLEL_PORTS = 3;
short LPT_ADDRESS[MAX_PARALLEL_PORTS] = 
{
	0x378,	// LPT1
	0x278,	// LPT2
	0x3bc,	// LPT3
};

int CabinetLightToIndex( CabinetLight cl )
{
	return cl;
}

int GameControllerAndGameButtonToIndex( GameController gc, GameButton gb )
{
	CLAMP( (int&)gb, 0, 3 );
	return NUM_CABINET_LIGHTS + gc*4 + gb;
}

void IndexToLptAndPin( int index, int &lpt_out, int &pin_out )
{
	lpt_out = index / LIGHTS_PER_PARALLEL_PORT;
	ASSERT( lpt_out >= 0 && lpt_out < MAX_PARALLEL_PORTS );
	pin_out = index % LIGHTS_PER_PARALLEL_PORT;
}

LightsDriver_Win32Parallel::LightsDriver_Win32Parallel()
{
	// init io.dll
	hDLL = LoadLibrary("parallel_lights_io.dll");
	if(hDLL == NULL)
	{
		MessageBox(NULL, "Could not LoadLibrary( parallel_lights_io.dll ).", "ERROR", MB_OK );
		return;
	}
	
	//Get the function pointers
	PortOut = (PORTOUT*) GetProcAddress(hDLL, "PortOut");
	IsDriverInstalled = (ISDRIVERINSTALLED*) GetProcAddress(hDLL, "IsDriverInstalled");
}

LightsDriver_Win32Parallel::~LightsDriver_Win32Parallel()
{
	FreeLibrary( hDLL );
}

void LightsDriver_Win32Parallel::Set( const LightsState *ls )
{
	BYTE data[MAX_PARALLEL_PORTS] =
	{
		0x00,
		0x00,
		0x00
	};

	{
		FOREACH_CabinetLight( cl )
		{
			bool bOn = ls->m_bCabinetLights[cl];
			int index = CabinetLightToIndex( cl );
			int lpt;
			int pin;
			IndexToLptAndPin( index, lpt, pin );
			BYTE mask = (BYTE) (0x01 << pin);
			if( bOn )
				data[lpt] |= mask;
			else
				data[lpt] &= ~mask;
		}
	}
	
	FOREACH_GameController( gc )
	{
		FOREACH_GameButton( gb )
		{
			bool bOn = ls->m_bGameButtonLights[gc][gb];
			int index = GameControllerAndGameButtonToIndex( gc, gb );
			int lpt;
			int pin;
			IndexToLptAndPin( index, lpt, pin );
			BYTE mask = (BYTE) (0x01 << pin);
			if( bOn )
				data[lpt] |= mask;
			else
				data[lpt] &= ~mask;
		}
	}

	{
		for( int i=0; i<MAX_PARALLEL_PORTS; i++ )
		{
			short address = LPT_ADDRESS[i];
			PortOut( address, data[i] );
		}
	}
}
