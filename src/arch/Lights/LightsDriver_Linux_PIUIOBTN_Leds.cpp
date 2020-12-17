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

#include "LightsDriver_Linux_PIUIOBTN_Leds.h"
#include "GameState.h"
#include "Game.h"
#include "RageLog.h"

REGISTER_LIGHTS_DRIVER_CLASS2(PIUIOBTN_Leds, Linux_PIUIOBTN_Leds);

namespace {
	const char *game_btn_leds[NUM_GameController][NUM_GameButton] = {
		{
			"/sys/class/leds/piuio::bboutput6/brightness", //P1 GAME_BUTTON_MENULEFT
			"/sys/class/leds/piuio::bboutput5/brightness", //P1 GAME_BUTTON_MENURIGHT
			nullptr, nullptr,
			"/sys/class/leds/piuio::bboutput4/brightness", //P1 GAME_BUTTON_START
			"/sys/class/leds/piuio::bboutput7/brightness", //P1 GAME_BUTTON_SELECT
		},
		{
			"/sys/class/leds/piuio::bboutput2/brightness", //P2 GAME_BUTTON_MENULEFT
			"/sys/class/leds/piuio::bboutput1/brightness", //P2 GAME_BUTTON_MENURIGHT
			nullptr, nullptr,
			"/sys/class/leds/piuio::bboutput0/brightness", //P2 GAME_BUTTON_START
			"/sys/class/leds/piuio::bboutput3/brightness", //P2 GAME_BUTTON_SELECT
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

LightsDriver_Linux_PIUIOBTN_Leds::LightsDriver_Linux_PIUIOBTN_Leds()
{
}

LightsDriver_Linux_PIUIOBTN_Leds::~LightsDriver_Linux_PIUIOBTN_Leds()
{
}

void LightsDriver_Linux_PIUIOBTN_Leds::Set( const LightsState *ls )
{
	const InputScheme *pInput = &GAMESTATE->GetCurrentGame()->m_InputScheme;
	RString sInputName = pInput->m_szName;

	FOREACH_ENUM(GameController, c)
	{
		FOREACH_ENUM( GameButton, gb )
		{
			if (ls->m_bGameButtonLights[c][gb] == previousLS.m_bGameButtonLights[c][gb])
			{
				continue;
			}

			if (!SetLight(game_btn_leds[c][gb], ls->m_bGameButtonLights[c][gb]))
			{
				LOG->Warn("Error setting button light %s",
						GameButtonToString(pInput, gb).c_str());
				return;
			}
		}
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
