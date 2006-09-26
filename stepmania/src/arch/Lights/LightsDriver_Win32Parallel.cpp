#include "global.h"
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
	CLAMP( (int&)gb, 0, 4 );
	return NUM_CabinetLight + gc*4 + gb;
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

/*
 * (c) 2003-2004 Chris Danford
 * All rights reserved.
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, and/or sell copies of the Software, and to permit persons to
 * whom the Software is furnished to do so, provided that the above
 * copyright notice(s) and this permission notice appear in all copies of
 * the Software and that both the above copyright notice(s) and this
 * permission notice appear in supporting documentation.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT OF
 * THIRD PARTY RIGHTS. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR HOLDERS
 * INCLUDED IN THIS NOTICE BE LIABLE FOR ANY CLAIM, OR ANY SPECIAL INDIRECT
 * OR CONSEQUENTIAL DAMAGES, OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS
 * OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
 * OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */
