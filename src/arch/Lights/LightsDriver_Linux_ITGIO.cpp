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
#include "LightsDriver_Linux_ITGIO.h"
#include "GameState.h"
#include "Game.h"
#include "RageLog.h"

REGISTER_LIGHTS_DRIVER_CLASS2(ITGIO, Linux_ITGIO);

//NOTE: light index 14 is not mapped, this is the coin drop "light"

namespace {
	const char *cabinet_leds[NUM_CabinetLight] = {
		//UL, UR, LL, LR, Bass L, Bass R
		"/sys/class/leds/itgio::output8/brightness",
		"/sys/class/leds/itgio::output10/brightness",
		"/sys/class/leds/itgio::output9/brightness",
		"/sys/class/leds/itgio::output11/brightness",
		"/sys/class/leds/itgio::output15/brightness",
		"/sys/class/leds/itgio::output15/brightness",
	};

	const char *dance_leds[NUM_GameController][NUM_GameButton] = {
		{
			nullptr, nullptr, nullptr, nullptr,
			//player start
			"/sys/class/leds/itgio::output13/brightness",
			nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
			//left right up down
			"/sys/class/leds/itgio::output1/brightness",
			"/sys/class/leds/itgio::output0/brightness",
			"/sys/class/leds/itgio::output3/brightness",
			"/sys/class/leds/itgio::output2/brightness",
		},
		{
			nullptr, nullptr, nullptr, nullptr,
			//player start
			"/sys/class/leds/itgio::output12/brightness",
			nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
			//left right up down
			"/sys/class/leds/itgio::output5/brightness",
			"/sys/class/leds/itgio::output4/brightness",
			"/sys/class/leds/itgio::output7/brightness",
			"/sys/class/leds/itgio::output6/brightness",
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

LightsDriver_Linux_ITGIO::LightsDriver_Linux_ITGIO()
{
}

LightsDriver_Linux_ITGIO::~LightsDriver_Linux_ITGIO()
{
}

void LightsDriver_Linux_ITGIO::SetGameControllerLight(const LightsState *ls, GameController c, GameButton gb)
{
	//only push a change in the light iff it's an actual change.
	if (ls->m_bGameButtonLights[c][gb] == previousLS.m_bGameButtonLights[c][gb])
	{
		return;
	}

	if (!SetLight(dance_leds[c][gb], ls->m_bGameButtonLights[c][gb]))
	{
		//grab the game info for a proper debug log.
		const InputScheme *pInput = &GAMESTATE->GetCurrentGame()->m_InputScheme;
		RString sInputName = pInput->m_szName;

		LOG->Warn("Error setting button light %s", GameButtonToString(pInput, gb).c_str());
	}
}

void LightsDriver_Linux_ITGIO::Set( const LightsState *ls )
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

	FOREACH_ENUM(GameController, c)
	{
		//takes care of each of the player buttons (UDLR)
		FOREACH_GameButton_Custom(gb)
		{
			SetGameControllerLight(ls, c, gb);
		}

		//Takes care of the cabinet based player lights.
		SetGameControllerLight(ls, c, GAME_BUTTON_START);
	}

	previousLS = *ls;
}

/*
 * (c) 2020 StepMania team
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
