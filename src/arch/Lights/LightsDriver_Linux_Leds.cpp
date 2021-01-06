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
#include "LightsDriver_Linux_Leds.h"
#include "GameState.h"
#include "Game.h"
#include "RageLog.h"

bool LightsDriver_Linux_Leds::WriteLight(const char *filename, bool state)
{
	//if we are setting a light that doesn't exist, let the caller know the function was a "success"
	//nullptr is used to define that the stepmania light is not mapped by the caller.
	if (filename == nullptr)
	{
		return true;
	}

	//LOG->Trace("LED: %s -> %d", filename, state);

	FILE *f = fopen(filename, "w");

	//if the filename should exist, but doesn't, let the caller know of the failure (if desired) and log it.
	if (f == nullptr)
	{
		LOG->Warn("Failed to set Linux_Led at %s. Check device permissions or udev rules. errno: %d: %s",
				  filename, errno, strerror(errno));
		return false;
	}

	//write and close the file.
	fprintf(f, "%d", state ? LINUX_LED_STATE_ON : LINUX_LED_STATE_OFF);
	fclose(f);

	return true;
}

bool LightsDriver_Linux_Leds::IsDance()
{
	pInput = &GAMESTATE->GetCurrentGame()->m_InputScheme;
	sInputName = pInput->m_szName;

	return sInputName.EqualsNoCase("dance");
}

bool LightsDriver_Linux_Leds::IsPump()
{
	pInput = &GAMESTATE->GetCurrentGame()->m_InputScheme;
	sInputName = pInput->m_szName;

	return sInputName.EqualsNoCase("pump");
}

void LightsDriver_Linux_Leds::SetLight(const char *filename, bool previous, bool desired)
{
	//don't overload the linux system if the light has not been changed.
	if (previous != desired)
	{
		WriteLight(filename, desired);
	}
}

void LightsDriver_Linux_Leds::SetCabinetLights(const char *stringArray[], const LightsState *ls)
{
	FOREACH_CabinetLight(light)
	{
		SetLight(stringArray[light], previousLS.m_bCabinetLights[light], ls->m_bCabinetLights[light]);
	}
}

void LightsDriver_Linux_Leds::SetCabinetLights(const int intArray[], const LightsState *ls)
{
	const char *baseFileLocation = GetGameControllerLightFile();

	if (baseFileLocation != nullptr)
	{
		char fileName[LINUX_LED_MAX_DIRECTORY_LENGTH];

		FOREACH_CabinetLight(light)
		{
			//valid lights are zero and positive.
			if (intArray[light] >= 0)
			{
				//don't waste sprintf time if we don't need to change the light
				if (previousLS.m_bCabinetLights[light] != ls->m_bCabinetLights[light])
				{
					sprintf(fileName, baseFileLocation, intArray[light]);
					SetLight(fileName, previousLS.m_bCabinetLights[light], ls->m_bCabinetLights[light]);
				}
			}
		}
	}
}

void LightsDriver_Linux_Leds::SetGameControllerLights(GameController gc, const char *stringArray[], const LightsState *ls)
{
	//iterate over all gamebuttons, including the menu/start/etc buttons.
	FOREACH_ENUM(GameButton, gb)
	{
		SetLight(stringArray[gb], previousLS.m_bGameButtonLights[gc][gb], ls->m_bGameButtonLights[gc][gb]);
	}
}

void LightsDriver_Linux_Leds::SetGameControllerLights(GameController gc, const int intArray[], const LightsState *ls)
{
	const char *baseFileLocation = GetGameControllerLightFile();

	if (baseFileLocation != nullptr)
	{
		char fileName[LINUX_LED_MAX_DIRECTORY_LENGTH];

		//iterate over all gamebuttons, including the menu/start/etc buttons.
		FOREACH_ENUM(GameButton, gb)
		{
			//valid lights are zero and positive.
			if (intArray[gb] >= 0)
			{
				//don't waste sprintf time if we don't need to change the light
				if (previousLS.m_bGameButtonLights[gc][gb] != ls->m_bGameButtonLights[gc][gb])
				{
					sprintf(fileName, baseFileLocation, intArray[gb]);
					SetLight(fileName, previousLS.m_bGameButtonLights[gc][gb], ls->m_bGameButtonLights[gc][gb]);
				}
			}
		}
	}
}

/*
 * (c) 2020 din
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
 * i love lamp
 */
