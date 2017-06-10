// Cross-platform, libusb-based driver for outputting lights
// via a PacDrive (http://www.ultimarc.com/pacdrive.html)

#include "global.h"
#include "RageLog.h"
#include "io/PacDrive.h"
#include "LightsDriver_LinuxPacDrive.h"

REGISTER_LIGHTS_DRIVER_CLASS( LinuxPacDrive );

LightsDriver_LinuxPacDrive::LightsDriver_LinuxPacDrive()
{
	m_bHasDevice = Board.Open();

	if( m_bHasDevice == false )
	{
		LOG->Warn( "Could not establish a connection with PacDrive." );
		return;
	}

	// clear all lights
	Board.Write( 0 );
}

LightsDriver_LinuxPacDrive::~LightsDriver_LinuxPacDrive()
{
	if( !m_bHasDevice )
		return;

	// clear all lights and close the connection
	Board.Write( 0 );
	Board.Close();
}

void LightsDriver_LinuxPacDrive::Set( const LightsState *ls )
{
	if( !m_bHasDevice )
		return;

	uint16_t outb = 0;

	if (ls->m_bCabinetLights[LIGHT_MARQUEE_UP_LEFT]) outb|=BIT(0);
	if (ls->m_bCabinetLights[LIGHT_MARQUEE_UP_RIGHT]) outb|=BIT(1);
	if (ls->m_bCabinetLights[LIGHT_MARQUEE_LR_LEFT]) outb|=BIT(2);
	if (ls->m_bCabinetLights[LIGHT_MARQUEE_LR_RIGHT]) outb|=BIT(3);
	if (ls->m_bCabinetLights[LIGHT_BASS_LEFT] || ls->m_bCabinetLights[LIGHT_BASS_RIGHT]) outb|=BIT(4);
	if (ls->m_bGameButtonLights[GameController_1][DANCE_BUTTON_LEFT]) outb|=BIT(5);
	if (ls->m_bGameButtonLights[GameController_1][DANCE_BUTTON_RIGHT]) outb|=BIT(6);
	if (ls->m_bGameButtonLights[GameController_1][DANCE_BUTTON_UP]) outb|=BIT(7);
	if (ls->m_bGameButtonLights[GameController_1][DANCE_BUTTON_DOWN]) outb|=BIT(8);
	if (ls->m_bGameButtonLights[GameController_1][GAME_BUTTON_START]) outb|=BIT(9);
	if (ls->m_bGameButtonLights[GameController_2][DANCE_BUTTON_LEFT]) outb|=BIT(10);
	if (ls->m_bGameButtonLights[GameController_2][DANCE_BUTTON_RIGHT]) outb|=BIT(11);
	if (ls->m_bGameButtonLights[GameController_2][DANCE_BUTTON_UP]) outb|=BIT(12);
	if (ls->m_bGameButtonLights[GameController_2][DANCE_BUTTON_DOWN]) outb|=BIT(13);
	if (ls->m_bGameButtonLights[GameController_2][GAME_BUTTON_START]) outb|=BIT(14);


	// write the data - if it fails, stop updating
	if( !Board.Write(outb) )
	{
		LOG->Warn( "Lost connection with PacDrive." );
		m_bHasDevice = false;
	}
}

/*
 * Copyright (c) 2008 BoXoRRoXoRs
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
