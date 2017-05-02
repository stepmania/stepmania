// LightsDriver_PacDrive for use with a PacDrive hooked up with LEDs
// You need PacDrive32.dll in the StepMania directory to use this.

#include "global.h"
#include "LightsDriver_PacDrive.h"
#include "windows.h"
#include "RageUtil.h"
#include "Preference.h"

REGISTER_LIGHTS_DRIVER_CLASS(PacDrive);

HINSTANCE PachDLL = NULL;

bool PacDriveConnected = false;
typedef int (WINAPI PacInitialize)(void);
PacInitialize* m_pacinit = NULL;
typedef void (WINAPI PacShutdown)(void);
PacShutdown* m_pacdone = NULL;
typedef bool (WINAPI PacSetLEDStates)(int, short int);
PacSetLEDStates* m_pacset = NULL;

//Adds new preference to allow for different light wiring setups
static Preference<RString> g_sPacDriveLightOrdering("PacDriveLightOrdering", "");


LightsDriver_PacDrive::LightsDriver_PacDrive()
{
	// init io.dll
	PachDLL = LoadLibrary("pacdrive32.dll");
	if (PachDLL == NULL)
	{
		MessageBox(NULL, "Could not LoadLibrary( pacdrive32.dll ).", "ERROR", MB_OK);
		return;
	}

	//Get the function pointers
	m_pacinit = (PacInitialize*)GetProcAddress(PachDLL, "PacInitialize");
	m_pacset = (PacSetLEDStates*)GetProcAddress(PachDLL, "PacSetLEDStates");
	m_pacdone = (PacShutdown*)GetProcAddress(PachDLL, "PacShutdown");

	int NumPacDrives = m_pacinit(); //initialize the pac drive

	if (NumPacDrives == 0)
	{
		PacDriveConnected = false; // set not connected 
		MessageBox(NULL, "Could not find connected PacDrive.", "ERROR", MB_OK);
		return;
	}
	else
	{
		PacDriveConnected = true; // set connected 
		m_pacset(0, 0x0);  // clear all lights for device i
	}
}

LightsDriver_PacDrive::~LightsDriver_PacDrive()
{
	if (PacDriveConnected)
		m_pacset(0, 0x0);  // clear all lights for device i
	m_pacdone();
	FreeLibrary(PachDLL);
}

void LightsDriver_PacDrive::Set(const LightsState *ls)
{
	std::string lightOrder = g_sPacDriveLightOrdering.Get();
	short int outb = 0;
	if (lightOrder.empty() || lightOrder.compare("minimaid") == 0) {
		// Set the cabinet light values according to Minimaid order if no preference is set

		if (ls->m_bCabinetLights[LIGHT_MARQUEE_UP_LEFT]) outb |= BIT(0);
		if (ls->m_bCabinetLights[LIGHT_MARQUEE_UP_RIGHT]) outb |= BIT(1);
		if (ls->m_bCabinetLights[LIGHT_MARQUEE_LR_LEFT]) outb |= BIT(2);
		if (ls->m_bCabinetLights[LIGHT_MARQUEE_LR_RIGHT]) outb |= BIT(3);
		if (ls->m_bCabinetLights[LIGHT_BASS_LEFT] || ls->m_bCabinetLights[LIGHT_BASS_RIGHT]) outb |= BIT(4);
		if (ls->m_bGameButtonLights[GameController_1][DANCE_BUTTON_LEFT]) outb |= BIT(5);
		if (ls->m_bGameButtonLights[GameController_1][DANCE_BUTTON_RIGHT]) outb |= BIT(6);
		if (ls->m_bGameButtonLights[GameController_1][DANCE_BUTTON_UP]) outb |= BIT(7);
		if (ls->m_bGameButtonLights[GameController_1][DANCE_BUTTON_DOWN]) outb |= BIT(8);
		if (ls->m_bGameButtonLights[GameController_1][GAME_BUTTON_START]) outb |= BIT(9);
		if (ls->m_bGameButtonLights[GameController_2][DANCE_BUTTON_LEFT]) outb |= BIT(10);
		if (ls->m_bGameButtonLights[GameController_2][DANCE_BUTTON_RIGHT]) outb |= BIT(11);
		if (ls->m_bGameButtonLights[GameController_2][DANCE_BUTTON_UP]) outb |= BIT(12);
		if (ls->m_bGameButtonLights[GameController_2][DANCE_BUTTON_DOWN]) outb |= BIT(13);
		if (ls->m_bGameButtonLights[GameController_2][GAME_BUTTON_START]) outb |= BIT(14);
	}
	else if (lightOrder.compare("lumenar") == 0 || lightOrder.compare("openitg") == 0) {
		//Sets the cabinet light values to follow LumenAR/OpenITG wiring standards

		if (ls->m_bGameButtonLights[GameController_1][DANCE_BUTTON_LEFT]) outb |= BIT(0);
		if (ls->m_bGameButtonLights[GameController_1][DANCE_BUTTON_RIGHT]) outb |= BIT(1);
		if (ls->m_bGameButtonLights[GameController_1][DANCE_BUTTON_UP]) outb |= BIT(2);
		if (ls->m_bGameButtonLights[GameController_1][DANCE_BUTTON_DOWN]) outb |= BIT(3);
		if (ls->m_bGameButtonLights[GameController_2][DANCE_BUTTON_LEFT]) outb |= BIT(4);
		if (ls->m_bGameButtonLights[GameController_2][DANCE_BUTTON_RIGHT]) outb |= BIT(5);
		if (ls->m_bGameButtonLights[GameController_2][DANCE_BUTTON_UP]) outb |= BIT(6);
		if (ls->m_bGameButtonLights[GameController_2][DANCE_BUTTON_DOWN]) outb |= BIT(7);
		if (ls->m_bCabinetLights[LIGHT_MARQUEE_UP_LEFT]) outb |= BIT(8);
		if (ls->m_bCabinetLights[LIGHT_MARQUEE_UP_RIGHT]) outb |= BIT(9);
		if (ls->m_bCabinetLights[LIGHT_MARQUEE_LR_LEFT]) outb |= BIT(10);
		if (ls->m_bCabinetLights[LIGHT_MARQUEE_LR_RIGHT]) outb |= BIT(11);
		if (ls->m_bGameButtonLights[GameController_1][GAME_BUTTON_START]) outb |= BIT(12);
		if (ls->m_bGameButtonLights[GameController_2][GAME_BUTTON_START]) outb |= BIT(13);
		if (ls->m_bCabinetLights[LIGHT_BASS_LEFT] || ls->m_bCabinetLights[LIGHT_BASS_RIGHT]) outb |= BIT(14);
	}
	else {
		//If all else fails, falls back to Minimaid order

		if (ls->m_bCabinetLights[LIGHT_MARQUEE_UP_LEFT]) outb |= BIT(0);
		if (ls->m_bCabinetLights[LIGHT_MARQUEE_UP_RIGHT]) outb |= BIT(1);
		if (ls->m_bCabinetLights[LIGHT_MARQUEE_LR_LEFT]) outb |= BIT(2);
		if (ls->m_bCabinetLights[LIGHT_MARQUEE_LR_RIGHT]) outb |= BIT(3);
		if (ls->m_bCabinetLights[LIGHT_BASS_LEFT] || ls->m_bCabinetLights[LIGHT_BASS_RIGHT]) outb |= BIT(4);
		if (ls->m_bGameButtonLights[GameController_1][DANCE_BUTTON_LEFT]) outb |= BIT(5);
		if (ls->m_bGameButtonLights[GameController_1][DANCE_BUTTON_RIGHT]) outb |= BIT(6);
		if (ls->m_bGameButtonLights[GameController_1][DANCE_BUTTON_UP]) outb |= BIT(7);
		if (ls->m_bGameButtonLights[GameController_1][DANCE_BUTTON_DOWN]) outb |= BIT(8);
		if (ls->m_bGameButtonLights[GameController_1][GAME_BUTTON_START]) outb |= BIT(9);
		if (ls->m_bGameButtonLights[GameController_2][DANCE_BUTTON_LEFT]) outb |= BIT(10);
		if (ls->m_bGameButtonLights[GameController_2][DANCE_BUTTON_RIGHT]) outb |= BIT(11);
		if (ls->m_bGameButtonLights[GameController_2][DANCE_BUTTON_UP]) outb |= BIT(12);
		if (ls->m_bGameButtonLights[GameController_2][DANCE_BUTTON_DOWN]) outb |= BIT(13);
		if (ls->m_bGameButtonLights[GameController_2][GAME_BUTTON_START]) outb |= BIT(14);
	}
	m_pacset(0, outb);
}

/* Modified 2015 Dave Barribeau for StepMania 5.09
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
