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
#include "LightsDriver_Linux_PIUIO_Leds.h"
#include "GameState.h"
#include "Game.h"
#include "RageLog.h"

REGISTER_LIGHTS_DRIVER_CLASS2(PIUIO_Leds, Linux_PIUIO_Leds);

namespace {
	const char *cabinet_leds[NUM_CabinetLight] = {
		"/sys/class/leds/piuio::output23/brightness",
		"/sys/class/leds/piuio::output26/brightness",
		"/sys/class/leds/piuio::output25/brightness",
		"/sys/class/leds/piuio::output24/brightness",
		"/sys/class/leds/piuio::output10/brightness",
		"/sys/class/leds/piuio::output10/brightness",
	};

	const char *dance_leds[NUM_GameController][NUM_GameButton] = {
		{
			nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
			"/sys/class/leds/piuio::output20/brightness",
			"/sys/class/leds/piuio::output21/brightness",
			"/sys/class/leds/piuio::output18/brightness",
			"/sys/class/leds/piuio::output19/brightness",
		},
		{
			nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
			"/sys/class/leds/piuio::output4/brightness",
			"/sys/class/leds/piuio::output5/brightness",
			"/sys/class/leds/piuio::output2/brightness",
			"/sys/class/leds/piuio::output3/brightness",
		},
	};

	const char *pump_leds[NUM_GameController][NUM_GameButton] = {
		{
			nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
			"/sys/class/leds/piuio::output2/brightness",
			"/sys/class/leds/piuio::output3/brightness",
			"/sys/class/leds/piuio::output4/brightness",
			"/sys/class/leds/piuio::output5/brightness",
			"/sys/class/leds/piuio::output6/brightness",
		},
		{
			nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
			"/sys/class/leds/piuio::output18/brightness",
			"/sys/class/leds/piuio::output19/brightness",
			"/sys/class/leds/piuio::output20/brightness",
			"/sys/class/leds/piuio::output21/brightness",
			"/sys/class/leds/piuio::output22/brightness",
		},
	};

	bool SetLight(const char *filename, bool on)
	{
		if (filename == nullptr)
			return true;
		FILE *f = fopen(filename, "w");
		if (f == nullptr)
		{
			return false;
		}
		fprintf(f, "%d", on ? 255 : 0);
		fclose(f);
		return true;
	}
}

LightsDriver_Linux_PIUIO_Leds::LightsDriver_Linux_PIUIO_Leds()
{
}

LightsDriver_Linux_PIUIO_Leds::~LightsDriver_Linux_PIUIO_Leds()
{
}

void LightsDriver_Linux_PIUIO_Leds::Set( const LightsState *ls )
{
	FOREACH_CabinetLight(light)
	{
		// Only SetLight if LightsState has changed since previous iteration.
		// This reduces unncessary strain on udev.  -dguzek
		if (ls->m_bCabinetLights[light] == previousLS.m_bCabinetLights[light] )
		{
			continue;
		}

		if (!SetLight(cabinet_leds[light], ls->m_bCabinetLights[light]))
		{
			LOG->Warn("Error setting cabinet light %s",
					CabinetLightToString(light).c_str());
			return;
		}
	}

	const InputScheme *pInput = &GAMESTATE->GetCurrentGame()->m_InputScheme;
	RString sInputName = pInput->m_szName;
	if (sInputName.EqualsNoCase("dance"))
	{
		FOREACH_ENUM(GameController, c)
		{
			FOREACH_GameButton_Custom(gb)
			{
				if (ls->m_bGameButtonLights[c][gb] == previousLS.m_bGameButtonLights[c][gb])
				{
					continue;
				}

				if (!SetLight(dance_leds[c][gb], ls->m_bGameButtonLights[c][gb]))
				{
					LOG->Warn("Error setting button light %s",
							GameButtonToString(pInput, gb).c_str());
					return;
				}
			}
		}
	}
	else if (sInputName.EqualsNoCase("pump"))
	{
		FOREACH_ENUM(GameController, c)
		{
			FOREACH_GameButton_Custom(gb)
			{
				if (ls->m_bGameButtonLights[c][gb] == previousLS.m_bGameButtonLights[c][gb])
				{
					continue;
				}

				if (!SetLight(pump_leds[c][gb], ls->m_bGameButtonLights[c][gb]))
				{
					LOG->Warn("Error setting button light %s",
							GameButtonToString(pInput, gb).c_str());
					return;
				}
			}
		}
	}
	else
	{
		return;
	}

	previousLS = *ls;
}

/*
 * (c) 2014 StepMania team
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
