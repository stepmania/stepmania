//
//    LightsDriver_LinuxParallel - Parallel Port Based Lights Driver for Linux
//
//    This requires root permissions to work! (run as root or suid)
//    This code was written using SystemMessage Driver as template.
//

#include "global.h"
#include <sys/io.h>
#include "LightsDriver_LinuxParallel.h"
#include "ScreenManager.h"
#include "InputMapper.h"
#include "Game.h"

static const int PORT_ADDRESS = 0x378;
static const bool SCREEN_DEBUG = false;

LightsDriver_LinuxParallel::LightsDriver_LinuxParallel()
{
	// Give port's permissions and reset all bits to zero
	ioperm( PORT_ADDRESS, 1, 1 );
	outb( 0, PORT_ADDRESS );
}

LightsDriver_LinuxParallel::~LightsDriver_LinuxParallel()
{
	// Reset all bits to zero and free the port's permissions
	outb( 0, PORT_ADDRESS );
	ioperm( PORT_ADDRESS, 1, 0 );
}

void LightsDriver_LinuxParallel::Set( const LightsState *ls )
{
	// Set LightState to port
	RString s;

	// Prepare Screen Output too for debugging
	s += "LinuxParallel Lights Driver Debug\n";
	s += "Lights Mode: " + LightsModeToString(LIGHTSMAN->GetLightsMode()) + "\n";

	// Cabinet Lights
	int i = 0;
	unsigned char output = 0;
	s += "Cabinet Bits: ";
	FOREACH_CabinetLight( cl )
	{
		s += ls->m_bCabinetLights[cl] ? '1' : '0';
		if ( ls->m_bCabinetLights[cl] )
			output += (unsigned char)pow((double)2,i);
		i++;
	}
	s += "\n";

	int iNumGameButtonsToShow = INPUTMAPPER->GetInputScheme()->ButtonNameToIndex( "Start" );
	if( iNumGameButtonsToShow == GameButton_Invalid )
		iNumGameButtonsToShow = INPUTMAPPER->GetInputScheme()->m_iButtonsPerController;
	FOREACH_GameController( gc )
	{
		s += ssprintf("Controller%d Bits: ",gc+1);
		for( int gb=0; gb<iNumGameButtonsToShow; gb++ )
			s += ls->m_bGameButtonLights[gc][gb] ? '1' : '0';
		s += "\n";
	}
	s += ssprintf("Output Port: 0x%x\n", PORT_ADDRESS);
	s += ssprintf("Output Byte: %i\n", output);

	if( SCREEN_DEBUG )
		SCREENMAN->SystemMessageNoAnimate( s );

	// Send byte to port
	outb( output, PORT_ADDRESS );
}

/*
 * (c) 2004 Hugo Hromic M. <hhromic@udec.cl>
 *
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
