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


HINSTANCE hDLL = NULL;

typedef void (WINAPI PORTOUT)(short int Port, char Data);
PORTOUT* PortOut = NULL;
typedef short int (WINAPI ISDRIVERINSTALLED)();
ISDRIVERINSTALLED* IsDriverInstalled = NULL;

const int LIGHTS_PER_PARALLEL_PORT = 8;
const int MAX_PARALLEL_PORTS = 3;
DWORD LPT_ADDRESS[MAX_PARALLEL_PORTS] = 
{
	0x378,	// LPT1
	0x278,	// LPT2
	0x3bc,	// LPT3
};
void LightToLptAndPin( Light light, int &lpt_out, int &pin_out )
{
	lpt_out = light / LIGHTS_PER_PARALLEL_PORT;
	ASSERT( lpt_out >= 0 && lpt_out < MAX_PARALLEL_PORTS );
	pin_out = light % LIGHTS_PER_PARALLEL_PORT;
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

BYTE g_data[MAX_PARALLEL_PORTS] =
{
	0x00,
	0x00,
	0x00
};

void LightsDriver_Win32Parallel::SetLight( Light light, bool bOn )
{
	int lpt;
	int pin;
	LightToLptAndPin( light, lpt, pin );

	BYTE &data = g_data[lpt];
	BYTE mask = (BYTE) (0x01 << pin);
	if( bOn )
		data |= mask;
	else
		data &= ~mask;
}

void LightsDriver_Win32Parallel::Flush()
{
	for( int i=0; i<MAX_PARALLEL_PORTS; i++ )
	{
		BYTE &data = g_data[i];
		DWORD address = LPT_ADDRESS[i];
		PortOut( (short) address, data );
	}
}
