#include "global.h"
#include <stdio.h>
#if defined(HAVE_UNISTD_H)
#include <unistd.h>
#endif
#include <sys/types.h>
#include <sys/stat.h>

#if defined(HAVE_FCNTL_H)
#include <fcntl.h>
#endif

#include <errno.h>
#include "LightsDriver_Linux_PIUIO.h"
#include "GameState.h"
#include "Game.h"
#include "RageLog.h"

REGISTER_LIGHTS_DRIVER_CLASS2(PIUIO, Linux_PIUIO);

LightsDriver_Linux_PIUIO::LightsDriver_Linux_PIUIO()
{
	// Open port
	fd = open("/dev/piuio0", O_WRONLY);
	if( fd < 0 )
	{
		LOG->Warn( "Error opening serial port for lights. Error: %d %s", errno, strerror(errno) );
		return;
	}
	LOG->Info("Opened PIUIO device for lights");
}

LightsDriver_Linux_PIUIO::~LightsDriver_Linux_PIUIO()
{
	if( fd >= 0 )
		close(fd);
}

void LightsDriver_Linux_PIUIO::Set( const LightsState *ls )
{
	unsigned char buf[8] = { 0, 0, 0, 0x08, 0x37, 0, 0, 0 };
	static unsigned char oldbuf[8] = { 0, 0, 0, 0x08, 0x37, 0, 0, 0 };

	if (ls->m_bCabinetLights[LIGHT_MARQUEE_UP_LEFT]) buf[2] |= 0x80;
	if (ls->m_bCabinetLights[LIGHT_MARQUEE_UP_RIGHT]) buf[3] |= 0x04;
	if (ls->m_bCabinetLights[LIGHT_MARQUEE_LR_LEFT]) buf[3] |= 0x02;
	if (ls->m_bCabinetLights[LIGHT_MARQUEE_LR_RIGHT]) buf[3] |= 0x01;
	if (ls->m_bCabinetLights[LIGHT_BASS_LEFT] || ls->m_bCabinetLights[LIGHT_BASS_RIGHT]) buf[1] |= 0x04;

	RString sInput = GAMESTATE->GetCurrentGame()->m_InputScheme.m_szName;
	if (sInput.EqualsNoCase("dance")) {
		if (ls->m_bGameButtonLights[GameController_1][DANCE_BUTTON_UP]) buf[2] |= 0x04;
		if (ls->m_bGameButtonLights[GameController_1][DANCE_BUTTON_DOWN]) buf[2] |= 0x08;
		if (ls->m_bGameButtonLights[GameController_1][DANCE_BUTTON_LEFT]) buf[2] |= 0x10;
		if (ls->m_bGameButtonLights[GameController_1][DANCE_BUTTON_RIGHT]) buf[2] |= 0x20;

		if (ls->m_bGameButtonLights[GameController_2][DANCE_BUTTON_UP]) buf[0] |= 0x04;
		if (ls->m_bGameButtonLights[GameController_2][DANCE_BUTTON_DOWN]) buf[0] |= 0x08;
		if (ls->m_bGameButtonLights[GameController_2][DANCE_BUTTON_LEFT]) buf[0] |= 0x10;
		if (ls->m_bGameButtonLights[GameController_2][DANCE_BUTTON_RIGHT]) buf[0] |= 0x20;
	} else if (sInput.EqualsNoCase("pump")) {
		if (ls->m_bGameButtonLights[GameController_1][PUMP_BUTTON_UPLEFT]) buf[0] |= 0x04;
		if (ls->m_bGameButtonLights[GameController_1][PUMP_BUTTON_UPRIGHT]) buf[0] |= 0x08;
		if (ls->m_bGameButtonLights[GameController_1][PUMP_BUTTON_CENTER]) buf[0] |= 0x10;
		if (ls->m_bGameButtonLights[GameController_1][PUMP_BUTTON_DOWNLEFT]) buf[0] |= 0x20;
		if (ls->m_bGameButtonLights[GameController_1][PUMP_BUTTON_DOWNRIGHT]) buf[0] |= 0x40;

		if (ls->m_bGameButtonLights[GameController_2][PUMP_BUTTON_UPLEFT]) buf[2] |= 0x04;
		if (ls->m_bGameButtonLights[GameController_2][PUMP_BUTTON_UPRIGHT]) buf[2] |= 0x08;
		if (ls->m_bGameButtonLights[GameController_2][PUMP_BUTTON_CENTER]) buf[2] |= 0x10;
		if (ls->m_bGameButtonLights[GameController_2][PUMP_BUTTON_DOWNLEFT]) buf[2] |= 0x20;
		if (ls->m_bGameButtonLights[GameController_2][PUMP_BUTTON_DOWNRIGHT]) buf[2] |= 0x40;
	}
	if (!memcmp(buf, oldbuf, 8))
		return;
	memcpy(oldbuf, buf, 8);

	if (write(fd, buf, 8) != 8)
	{
		LOG->Warn( "Error setting lights state. Error: %d %s", errno, strerror(errno) );
		return;
	}
}

/*
 * (c) 2012-2013 StepMania team
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
