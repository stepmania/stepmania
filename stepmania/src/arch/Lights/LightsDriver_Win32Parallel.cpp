#include "global.h"
/*
-----------------------------------------------------------------------------
 Class: LightsDriver_Win32Parallel

 Desc: See header.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
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

void LightsDriver_Win32Parallel::SetLight( Light light, bool bOn )
{
	static BYTE data = 0x00;
	BYTE mask = 0x01 << light;
	if( bOn )
		data |= mask;
	else
		data &= ~mask;
	PortOut( 0x378, data );
	// ports of interest:  0x278, 0x3BC, 0x378
}
