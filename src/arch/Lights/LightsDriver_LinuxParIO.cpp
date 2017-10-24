/* LightsDriver_LinuxParIO - Parallel Port Based Lights Driver for Linux using ParIO
 * This does not require root permissions to work ;)
 * This code was written using LightsDriver_LinuxParallel as a template */

#include "global.h"
#include "LightsDriver_LinuxParIO.h"
#include "ScreenManager.h"
#include "InputMapper.h"
#include "Game.h"
#include <iostream>

// /dev/parport0 - cab lights
// /dev/parport1 - pad lights
static const int CAB_PARPORT = 0;
static const int PAD_PARPORT = 1;
static const bool SCREEN_DEBUG = false;

REGISTER_LIGHTS_DRIVER_CLASS(LinuxParIO);

LightsDriver_LinuxParIO::LightsDriver_LinuxParIO()
: m_cabPort( CAB_PARPORT )
, m_padPort( PAD_PARPORT )
{
	if( m_cabPort.is_open() == false )
	{
		fprintf( stderr, "LightsDriver_LinuxParIO: Failed to open parallel port for cabinet lights\n" );
	}
	if( m_padPort.is_open() == false )
	{
		fprintf( stderr, "LightsDriver_LinuxParIO: Failed to open parallel port for pad lights\n" );
	}
}

LightsDriver_LinuxParIO::~LightsDriver_LinuxParIO()
{
}

void LightsDriver_LinuxParIO::Set( const LightsState *ls )
{

	// Set LightState to port
	RString s;

	// Prepare screen output too for debugging
	s += "LinuxParIO Lights Driver Debug\n";
	s += "Lights Mode: " + LightsModeToString(LIGHTSMAN->GetLightsMode()) + "\n";

	// Tell someone if the port isn't open
	if( m_cabPort.is_open() == false ||
	    m_padPort.is_open() == false )
	{
		s += "ERROR: Failed to open parallel port(s), no lights will be available\n";
        	if( SCREEN_DEBUG )
		{
	                SCREENMAN->SystemMessageNoAnimate( s );
		}
		return;
	}

	// Cabinet Lights
	int i = 0;
	unsigned char output0 = 0;
	unsigned char output1 = 0;
	s += "Cabinet Bits: ";
	FOREACH_CabinetLight( cl )
	{
		s += ls->m_bCabinetLights[cl] ? '1' : '0';
		if ( ls->m_bCabinetLights[cl] )
			output0 += (unsigned char)pow((double)2,i);
		i++;
	}
	s += "\n";

	s += ssprintf("Controller Bits: ");
	if(ls->m_bGameButtonLights[GameController_1][DANCE_BUTTON_LEFT])
	{
		output1 |= 0b00000001;
	}
	if(ls->m_bGameButtonLights[GameController_1][DANCE_BUTTON_RIGHT])
	{
		output1 |= 0b00000010;
	}
	if(ls->m_bGameButtonLights[GameController_1][DANCE_BUTTON_UP])
	{
		output1 |= 0b00000100;
	}
	if(ls->m_bGameButtonLights[GameController_1][DANCE_BUTTON_DOWN])
	{
		output1 |= 0b00001000;
	}	
	if(ls->m_bGameButtonLights[GameController_2][DANCE_BUTTON_LEFT])
	{
		output1 |= 0b00010000;
	}
	if(ls->m_bGameButtonLights[GameController_2][DANCE_BUTTON_RIGHT])
	{
		output1 |= 0b00100000;
	}
	if(ls->m_bGameButtonLights[GameController_2][DANCE_BUTTON_UP])
	{
		output1 |= 0b01000000;
	}
	if(ls->m_bGameButtonLights[GameController_2][DANCE_BUTTON_DOWN])
	{
		output1 |= 0b10000000;
	}
	s += ssprintf("0x%x",output1);
	s += "\n";
	
	s += ssprintf("Output Port (cab): 0x%s\n", m_cabPort.path().c_str());
	s += ssprintf("Output Byte (cab): 0x%x\n", output0);
	s += ssprintf("Output Port (pad): 0x%s\n", m_cabPort.path().c_str());
	s += ssprintf("Output Byte (pad): 0x%x\n", output1);

	if( SCREEN_DEBUG )
		SCREENMAN->SystemMessageNoAnimate( s );

	// Send byte to port
	m_cabPort.write( (const unsigned char)output0 );
	m_padPort.write( (const unsigned char)output1 );
}

/*
 * LightsDriver_LinuxParIO
 * Based on LightsDriver_LinuxParallel 
 * (c) 2004 Hugo Hromic M. <hhromic@udec.cl>
 *
 * (c) 2017 Gareth Francis. <gfrancis.dev@gmail.com>
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
