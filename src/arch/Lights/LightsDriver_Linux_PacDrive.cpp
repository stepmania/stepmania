#include "global.h"
#include "RageLog.h"

#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>

#include "LightsDriver_Linux_PacDrive.h"

#define PAC_DEV_NODE "/dev/pacdrive"
#define PAC_UDEV_RULE "ACTION==\"add\", SUBSYSTEM==\"hidraw\" ATTRS{idVendor}==\"d209\", ATTRS{idProduct}==\"1500\", MODE:=\"666\" SYMLINK+=\"pacdrive\""

#define HIDIOCSOUTPUT(len) _IOC(_IOC_WRITE | _IOC_READ, 'H', 0x0B, len)
#define BIT(i) (1 << (i))

REGISTER_LIGHTS_DRIVER_CLASS(PacDrive);

static Preference<RString> g_sPacDriveLightOrdering("PacDriveLightOrdering", "minimaid");

LightsDriver_PacDrive::LightsDriver_PacDrive()
{
	OpenPacDrive();
	WritePacDrive(0);

	RString lightOrder = g_sPacDriveLightOrdering.Get();
	if (lightOrder.CompareNoCase("lumenar") == 0 || lightOrder.CompareNoCase("openitg") == 0)
	{
		iLightingOrder = 1;
	}
}

LightsDriver_PacDrive::~LightsDriver_PacDrive()
{
	ClosePacDrive();
}

void LightsDriver_PacDrive::Set(const LightsState *ls)
{
	uint16_t state = MapLights(ls);

	if (state != lastState)
	{
		WritePacDrive(state);
	}

	lastState = state;
}

void LightsDriver_PacDrive::OpenPacDrive()
{
	int result = open(PAC_DEV_NODE, O_WRONLY | O_NONBLOCK);

	if (result < 0)
	{
		if (errno == ENOENT)
		{
			LOG->Warn("Pacdrive device not found. Ensure the device is available on %s via a udev rule such as:\n" PAC_UDEV_RULE, PAC_DEV_NODE);
		}
		else if (errno == EACCES)
		{
			LOG->Warn("Unable to open Pacdrive device. Ensure the device is writable on %s via a udev rule such as:\n" PAC_UDEV_RULE, PAC_DEV_NODE);
		}
		else
		{
			LOG->Warn("Error opening Pacdrive: %s", strerror(errno));
		}

		return;
	}

	fd = result;
}

void LightsDriver_PacDrive::ClosePacDrive()
{
	if (fd < 0)
	{
		return;
	}

	if (close(fd) < 0)
	{
		LOG->Warn("Error closing Pacdrive: %s", strerror(errno));
	}

	fd = -1;
}

void LightsDriver_PacDrive::WritePacDrive(const uint16_t val)
{
	if (fd < 0)
	{
		return;
	}

	memset(buffer, 0, sizeof(buffer));

	// buffer[0] = 0; First byte is the report number.

	buffer[3] = (val & 0xff00) >> 8;
	buffer[4] = (val & 0x00ff);

	if (ioctl(fd, HIDIOCSOUTPUT(5), buffer) < 0)
	{
		LOG->Warn("Error writing to Pacdrive: %s", strerror(errno));
		ClosePacDrive();
	}
}

const uint16_t LightsDriver_PacDrive::MapLights(const LightsState *ls)
{
	uint16_t result = 0;

	switch (iLightingOrder)
	{
		case 1:
			// Sets the cabinet light values to follow LumenAR/OpenITG wiring standards
			if (ls->m_bGameButtonLights[GameController_1][DANCE_BUTTON_LEFT])						result |= BIT(0);
			if (ls->m_bGameButtonLights[GameController_1][DANCE_BUTTON_RIGHT])  					result |= BIT(1);
			if (ls->m_bGameButtonLights[GameController_1][DANCE_BUTTON_UP])     					result |= BIT(2);
			if (ls->m_bGameButtonLights[GameController_1][DANCE_BUTTON_DOWN])   					result |= BIT(3);
			if (ls->m_bGameButtonLights[GameController_2][DANCE_BUTTON_LEFT]) 						result |= BIT(4);
			if (ls->m_bGameButtonLights[GameController_2][DANCE_BUTTON_RIGHT]) 						result |= BIT(5);
			if (ls->m_bGameButtonLights[GameController_2][DANCE_BUTTON_UP]) 						result |= BIT(6);
			if (ls->m_bGameButtonLights[GameController_2][DANCE_BUTTON_DOWN]) 						result |= BIT(7);
			if (ls->m_bCabinetLights[LIGHT_MARQUEE_UP_LEFT]) 										result |= BIT(8);
			if (ls->m_bCabinetLights[LIGHT_MARQUEE_UP_RIGHT]) 										result |= BIT(9);
			if (ls->m_bCabinetLights[LIGHT_MARQUEE_LR_LEFT]) 										result |= BIT(10);
			if (ls->m_bCabinetLights[LIGHT_MARQUEE_LR_RIGHT]) 										result |= BIT(11);
			if (ls->m_bGameButtonLights[GameController_1][GAME_BUTTON_START]) 						result |= BIT(12);
			if (ls->m_bGameButtonLights[GameController_2][GAME_BUTTON_START]) 						result |= BIT(13);
			if (ls->m_bCabinetLights[LIGHT_BASS_LEFT] || ls->m_bCabinetLights[LIGHT_BASS_RIGHT])	result |= BIT(14);
			return result;

		case 0:
		default:
			// Minimaid order
			if (ls->m_bCabinetLights[LIGHT_MARQUEE_UP_LEFT]) 										result |= BIT(0);
			if (ls->m_bCabinetLights[LIGHT_MARQUEE_UP_RIGHT]) 										result |= BIT(1);
			if (ls->m_bCabinetLights[LIGHT_MARQUEE_LR_LEFT]) 										result |= BIT(2);
			if (ls->m_bCabinetLights[LIGHT_MARQUEE_LR_RIGHT]) 										result |= BIT(3);
			if (ls->m_bCabinetLights[LIGHT_BASS_LEFT] || ls->m_bCabinetLights[LIGHT_BASS_RIGHT]) 	result |= BIT(4);
			if (ls->m_bGameButtonLights[GameController_1][DANCE_BUTTON_LEFT]) 						result |= BIT(5);
			if (ls->m_bGameButtonLights[GameController_1][DANCE_BUTTON_RIGHT]) 						result |= BIT(6);
			if (ls->m_bGameButtonLights[GameController_1][DANCE_BUTTON_UP]) 						result |= BIT(7);
			if (ls->m_bGameButtonLights[GameController_1][DANCE_BUTTON_DOWN]) 						result |= BIT(8);
			if (ls->m_bGameButtonLights[GameController_1][GAME_BUTTON_START]) 						result |= BIT(9);
			if (ls->m_bGameButtonLights[GameController_2][DANCE_BUTTON_LEFT]) 						result |= BIT(10);
			if (ls->m_bGameButtonLights[GameController_2][DANCE_BUTTON_RIGHT]) 						result |= BIT(11);
			if (ls->m_bGameButtonLights[GameController_2][DANCE_BUTTON_UP]) 						result |= BIT(12);
			if (ls->m_bGameButtonLights[GameController_2][DANCE_BUTTON_DOWN]) 						result |= BIT(13);
			if (ls->m_bGameButtonLights[GameController_2][GAME_BUTTON_START]) 						result |= BIT(14);
			return result;
	}
}

/*
 * (c) 2021 farthom
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
 * 
 * Rainbow Dash is best pony.
 */
