// LightsDriver_LinuxMinimaid - driver for lights on the Minimaid with libmmmagic
/* To use, StepMania must be run with root permissions, or udev rule for the
 * device must be made something like this (but it's not quite right):
 * echo SUBSYSTEM==\"usb\", ATTR{idVendor}==\"beef\", ATTR{idProduct}==\"5730\", MODE=\"0666\" > /etc/udev/rules.d/50-minimaid
 */

#include "global.h"
#include "LightsDriver_LinuxMinimaid.h"
#include "../../../extern/libmmmagic/mmmagic.h"

REGISTER_LIGHTS_DRIVER_CLASS( LinuxMinimaid );

LightsDriver_LinuxMinimaid::LightsDriver_LinuxMinimaid()
{
	// libmmmagic is actually statically linked in, so there's no wrapper
	// initializing to do. -Kyz
	_mmmagic_loaded= true;
	mm_connect_minimaid();
	mm_setKB(true);
}

LightsDriver_LinuxMinimaid::~LightsDriver_LinuxMinimaid()
{
	if(_mmmagic_loaded)
	{
		mm_setDDRAllOff();
	}
	// libmmmagic is actually statically linked in, so there's no wrapper
	// destruction to do. -Kyz
}

void LightsDriver_LinuxMinimaid::Set(LightsState const* ls)
{
	if(_mmmagic_loaded)
	{
		mm_setDDRAllOff();

		// Set the cabinet light values
		if(ls->m_bCabinetLights[LIGHT_MARQUEE_UP_LEFT])
		{
			mm_setDDRCabinetLight(DDR_DOUBLE_MARQUEE_UPPER_LEFT, 1);
		}
		if(ls->m_bCabinetLights[LIGHT_MARQUEE_UP_RIGHT])
		{
			mm_setDDRCabinetLight(DDR_DOUBLE_MARQUEE_UPPER_RIGHT, 1);
		}
		if(ls->m_bCabinetLights[LIGHT_MARQUEE_LR_LEFT])
		{
			mm_setDDRCabinetLight(DDR_DOUBLE_MARQUEE_LOWER_LEFT, 1);
		}
		if(ls->m_bCabinetLights[LIGHT_MARQUEE_LR_RIGHT])
		{
			mm_setDDRCabinetLight(DDR_DOUBLE_MARQUEE_LOWER_RIGHT, 1);
		}
		if(ls->m_bCabinetLights[LIGHT_BASS_LEFT] || ls->m_bCabinetLights[LIGHT_BASS_RIGHT])
		{
			mm_setDDRBassLight(DDR_DOUBLE_BASS_LIGHTS, 1);
		}

		if(ls->m_bGameButtonLights[GameController_1][DANCE_BUTTON_LEFT])
		{
			mm_setDDRPad1Light(DDR_DOUBLE_PAD_LEFT, 1);
		}
		if(ls->m_bGameButtonLights[GameController_1][DANCE_BUTTON_RIGHT])
		{
			mm_setDDRPad1Light(DDR_DOUBLE_PAD_RIGHT, 1);
		}
		if(ls->m_bGameButtonLights[GameController_1][DANCE_BUTTON_UP])
		{
			mm_setDDRPad1Light(DDR_DOUBLE_PAD_UP, 1);
		}
		if(ls->m_bGameButtonLights[GameController_1][DANCE_BUTTON_DOWN])
		{
			mm_setDDRPad1Light(DDR_DOUBLE_PAD_DOWN, 1);
		}
		if(ls->m_bGameButtonLights[GameController_1][GAME_BUTTON_START])
		{
			mm_setDDRCabinetLight(DDR_DOUBLE_PLAYER1_PANEL, 1);
		}
		if(ls->m_bGameButtonLights[GameController_2][DANCE_BUTTON_LEFT])
		{
			mm_setDDRPad2Light(DDR_DOUBLE_PAD_LEFT, 1);
		}
		if(ls->m_bGameButtonLights[GameController_2][DANCE_BUTTON_RIGHT])
		{
			mm_setDDRPad2Light(DDR_DOUBLE_PAD_RIGHT, 1);
		}
		if(ls->m_bGameButtonLights[GameController_2][DANCE_BUTTON_UP])
		{
			mm_setDDRPad2Light(DDR_DOUBLE_PAD_UP, 1);
		}
		if(ls->m_bGameButtonLights[GameController_2][DANCE_BUTTON_DOWN])
		{
			mm_setDDRPad2Light(DDR_DOUBLE_PAD_DOWN, 1);
		}
		if(ls->m_bGameButtonLights[GameController_2][GAME_BUTTON_START])
		{
			mm_setDDRCabinetLight(DDR_DOUBLE_PLAYER2_PANEL, 1);
		}

		// Output the information
		mm_sendDDRMiniMaidUpdate();
	}
}
