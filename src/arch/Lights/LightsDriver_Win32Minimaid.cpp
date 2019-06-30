// LightsDriver_Win32Minimaid - driver for lights on the Minimaid with libmmmagic
/* To use, StepMania must be run with root permissions, or udev rule for the
 * device must be made something like this (but it's not quite right):
 * echo SUBSYSTEM==\"usb\", ATTR{idVendor}==\"beef\", ATTR{idProduct}==\"5730\", MODE=\"0666\" > /etc/udev/rules.d/50-minimaid
 */

#include "global.h"
#include "LightsDriver_Win32Minimaid.h"
#include "windows.h"

REGISTER_LIGHTS_DRIVER_CLASS( Win32Minimaid );

HINSTANCE hMMMAGICDLL = nullptr;

int minimaid_filter(unsigned int, struct _EXCEPTION_POINTERS *)
{
	return EXCEPTION_EXECUTE_HANDLER;
}

void setup_driver()
{
	__try
	{

		// Get the function pointers
		mm_connect_minimaid = (MM_CONNECT_MINIMAID)GetProcAddress(hMMMAGICDLL, "mm_connect_minimaid");
		mm_setKB = (MM_SETKB)GetProcAddress(hMMMAGICDLL, "mm_setKB");
		mm_setDDRPad1Light = (MM_SETDDRPAD1LIGHT)GetProcAddress(hMMMAGICDLL, "mm_setDDRPad1Light");
		mm_setDDRPad2Light = (MM_SETDDRPAD2LIGHT)GetProcAddress(hMMMAGICDLL, "mm_setDDRPad2Light");
		mm_setDDRCabinetLight = (MM_SETCABINETLIGHT)GetProcAddress(hMMMAGICDLL, "mm_setDDRCabinetLight");
		mm_setDDRBassLight = (MM_SETDDRBASSLIGHT)GetProcAddress(hMMMAGICDLL, "mm_setDDRBassLight");
		mm_setDDRAllOn = (MM_SETDDRALLON)GetProcAddress(hMMMAGICDLL, "mm_setDDRAllOn");
		mm_setDDRAllOff = (MM_SETDDRALLOFF)GetProcAddress(hMMMAGICDLL, "mm_setDDRAllOff");
		mm_setBlueLED = (MM_SETBLUELED)GetProcAddress(hMMMAGICDLL, "mm_setBlueLED");
		mm_setMMOutputReports = (MM_SETMMOUTPUTREPORTS)GetProcAddress(hMMMAGICDLL, "mm_setMMOutputReports");
		mm_sendDDRMiniMaidUpdate = (MM_SENDDDRMINIMAIDUPDATE)GetProcAddress(hMMMAGICDLL, "mm_sendDDRMiniMaidUpdate");
		mm_connect_minimaid();
		mm_setKB(true);

		_mmmagic_loaded = true;
	}
	__except (minimaid_filter(GetExceptionCode(), GetExceptionInformation()))
	{
		MessageBox(nullptr, "Could not connect to the Mimimaid device. Freeing the library now.", "ERROR", MB_OK);
		FreeLibrary(hMMMAGICDLL);
	}

}

LightsDriver_Win32Minimaid::LightsDriver_Win32Minimaid()
{
	_mmmagic_loaded=false;
	hMMMAGICDLL = LoadLibraryW(L"mmmagic.dll");
	if(hMMMAGICDLL == nullptr)
	{
		MessageBox(nullptr, "Could not LoadLibrary( mmmagic.dll ).", "ERROR", MB_OK );
		return;
	}
	setup_driver();
}

LightsDriver_Win32Minimaid::~LightsDriver_Win32Minimaid()
{
	if (_mmmagic_loaded)
	{
		mm_setDDRAllOff();
		FreeLibrary(hMMMAGICDLL);
	}
}

void LightsDriver_Win32Minimaid::Set( const LightsState *ls )
{
	if(_mmmagic_loaded)
	{
		mm_setDDRAllOff();

		// Set the cabinet light values
		if (ls->m_bCabinetLights[LIGHT_MARQUEE_UP_LEFT]) mm_setDDRCabinetLight(DDR_DOUBLE_MARQUEE_UPPER_LEFT, 1);
		if (ls->m_bCabinetLights[LIGHT_MARQUEE_UP_RIGHT]) mm_setDDRCabinetLight(DDR_DOUBLE_MARQUEE_UPPER_RIGHT, 1);
		if (ls->m_bCabinetLights[LIGHT_MARQUEE_LR_LEFT]) mm_setDDRCabinetLight(DDR_DOUBLE_MARQUEE_LOWER_LEFT, 1);
		if (ls->m_bCabinetLights[LIGHT_MARQUEE_LR_RIGHT]) mm_setDDRCabinetLight(DDR_DOUBLE_MARQUEE_LOWER_RIGHT, 1);
		if (ls->m_bCabinetLights[LIGHT_BASS_LEFT] || ls->m_bCabinetLights[LIGHT_BASS_RIGHT]) mm_setDDRBassLight(DDR_DOUBLE_BASS_LIGHTS, 1);

		if (ls->m_bGameButtonLights[GameController_1][DANCE_BUTTON_LEFT]) mm_setDDRPad1Light(DDR_DOUBLE_PAD_LEFT, 1);
		if (ls->m_bGameButtonLights[GameController_1][DANCE_BUTTON_RIGHT]) mm_setDDRPad1Light(DDR_DOUBLE_PAD_RIGHT, 1);
		if (ls->m_bGameButtonLights[GameController_1][DANCE_BUTTON_UP]) mm_setDDRPad1Light(DDR_DOUBLE_PAD_UP, 1);
		if (ls->m_bGameButtonLights[GameController_1][DANCE_BUTTON_DOWN]) mm_setDDRPad1Light(DDR_DOUBLE_PAD_DOWN, 1);
		if (ls->m_bGameButtonLights[GameController_1][GAME_BUTTON_START]) mm_setDDRCabinetLight(DDR_DOUBLE_PLAYER1_PANEL, 1);
		if (ls->m_bGameButtonLights[GameController_2][DANCE_BUTTON_LEFT]) mm_setDDRPad2Light(DDR_DOUBLE_PAD_LEFT, 1);
		if (ls->m_bGameButtonLights[GameController_2][DANCE_BUTTON_RIGHT]) mm_setDDRPad2Light(DDR_DOUBLE_PAD_RIGHT, 1);
		if (ls->m_bGameButtonLights[GameController_2][DANCE_BUTTON_UP]) mm_setDDRPad2Light(DDR_DOUBLE_PAD_UP, 1);
		if (ls->m_bGameButtonLights[GameController_2][DANCE_BUTTON_DOWN]) mm_setDDRPad2Light(DDR_DOUBLE_PAD_DOWN, 1);
		if (ls->m_bGameButtonLights[GameController_2][GAME_BUTTON_START]) mm_setDDRCabinetLight(DDR_DOUBLE_PLAYER2_PANEL, 1);

		// Output the information
		mm_sendDDRMiniMaidUpdate();
	}
}

/*
 * (c) 2013 pkgingo
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
