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

//NOTE: front usb ports are off by default, index 27 enables them.
//It is currently unmapped.

namespace
{
	const int cabinet_lights[NUM_CabinetLight] = {
		23, //LIGHT_MARQUEE_UP_LEFT
		26, //LIGHT_MARQUEE_UP_RIGHT
		25, //LIGHT_MARQUEE_LR_LEFT
		24, //LIGHT_MARQUEE_LR_RIGHT
		10, //LIGHT_BASS_LEFT
		10, //LIGHT_BASS_RIGHT
	};

	const int player1_dance_lights[NUM_GameButton] = {
		-1, //GAME_BUTTON_MENULEFT
		-1, //GAME_BUTTON_MENURIGHT
		-1, //GAME_BUTTON_MENUUP
		-1, //GAME_BUTTON_MENUDOWN
		-1, //GAME_BUTTON_START
		-1, //GAME_BUTTON_SELECT
		-1, //GAME_BUTTON_BACK
		-1, //GAME_BUTTON_COIN
		-1, //GAME_BUTTON_OPERATOR
		-1, //GAME_BUTTON_EFFECT_UP
		-1, //GAME_BUTTON_EFFECT_DOWN
		20, //GAME_BUTTON_CUSTOM_01
		21, //GAME_BUTTON_CUSTOM_02
		18, //GAME_BUTTON_CUSTOM_03
		19, //GAME_BUTTON_CUSTOM_04
		-1, //GAME_BUTTON_CUSTOM_05
		-1, //GAME_BUTTON_CUSTOM_06
		-1, //GAME_BUTTON_CUSTOM_07
		-1, //GAME_BUTTON_CUSTOM_08
		-1, //GAME_BUTTON_CUSTOM_09
		-1, //GAME_BUTTON_CUSTOM_10
		-1, //GAME_BUTTON_CUSTOM_11
		-1, //GAME_BUTTON_CUSTOM_12
		-1, //GAME_BUTTON_CUSTOM_13
		-1, //GAME_BUTTON_CUSTOM_14
		-1, //GAME_BUTTON_CUSTOM_15
		-1, //GAME_BUTTON_CUSTOM_16
		-1, //GAME_BUTTON_CUSTOM_17
		-1, //GAME_BUTTON_CUSTOM_18
		-1, //GAME_BUTTON_CUSTOM_19
	};

	const int player2_dance_lights[NUM_GameButton] = {
		-1, //GAME_BUTTON_MENULEFT
		-1, //GAME_BUTTON_MENURIGHT
		-1, //GAME_BUTTON_MENUUP
		-1, //GAME_BUTTON_MENUDOWN
		-1, //GAME_BUTTON_START
		-1, //GAME_BUTTON_SELECT
		-1, //GAME_BUTTON_BACK
		-1, //GAME_BUTTON_COIN
		-1, //GAME_BUTTON_OPERATOR
		-1, //GAME_BUTTON_EFFECT_UP
		-1, //GAME_BUTTON_EFFECT_DOWN
		4,	//GAME_BUTTON_CUSTOM_01
		5,	//GAME_BUTTON_CUSTOM_02
		2,	//GAME_BUTTON_CUSTOM_03
		3,	//GAME_BUTTON_CUSTOM_04
		-1, //GAME_BUTTON_CUSTOM_05
		-1, //GAME_BUTTON_CUSTOM_06
		-1, //GAME_BUTTON_CUSTOM_07
		-1, //GAME_BUTTON_CUSTOM_08
		-1, //GAME_BUTTON_CUSTOM_09
		-1, //GAME_BUTTON_CUSTOM_10
		-1, //GAME_BUTTON_CUSTOM_11
		-1, //GAME_BUTTON_CUSTOM_12
		-1, //GAME_BUTTON_CUSTOM_13
		-1, //GAME_BUTTON_CUSTOM_14
		-1, //GAME_BUTTON_CUSTOM_15
		-1, //GAME_BUTTON_CUSTOM_16
		-1, //GAME_BUTTON_CUSTOM_17
		-1, //GAME_BUTTON_CUSTOM_18
		-1, //GAME_BUTTON_CUSTOM_19
	};

	const int player1_pump_lights[NUM_GameButton] = {
		-1, //GAME_BUTTON_MENULEFT
		-1, //GAME_BUTTON_MENURIGHT
		-1, //GAME_BUTTON_MENUUP
		-1, //GAME_BUTTON_MENUDOWN
		-1, //GAME_BUTTON_START
		-1, //GAME_BUTTON_SELECT
		-1, //GAME_BUTTON_BACK
		-1, //GAME_BUTTON_COIN
		-1, //GAME_BUTTON_OPERATOR
		-1, //GAME_BUTTON_EFFECT_UP
		-1, //GAME_BUTTON_EFFECT_DOWN
		2,	//GAME_BUTTON_CUSTOM_01
		3,	//GAME_BUTTON_CUSTOM_02
		4,	//GAME_BUTTON_CUSTOM_03
		5,	//GAME_BUTTON_CUSTOM_04
		6,	//GAME_BUTTON_CUSTOM_05
		-1, //GAME_BUTTON_CUSTOM_06
		-1, //GAME_BUTTON_CUSTOM_07
		-1, //GAME_BUTTON_CUSTOM_08
		-1, //GAME_BUTTON_CUSTOM_09
		-1, //GAME_BUTTON_CUSTOM_10
		-1, //GAME_BUTTON_CUSTOM_11
		-1, //GAME_BUTTON_CUSTOM_12
		-1, //GAME_BUTTON_CUSTOM_13
		-1, //GAME_BUTTON_CUSTOM_14
		-1, //GAME_BUTTON_CUSTOM_15
		-1, //GAME_BUTTON_CUSTOM_16
		-1, //GAME_BUTTON_CUSTOM_17
		-1, //GAME_BUTTON_CUSTOM_18
		-1, //GAME_BUTTON_CUSTOM_19
	};

	const int player2_pump_lights[NUM_GameButton] = {
		-1, //GAME_BUTTON_MENULEFT
		-1, //GAME_BUTTON_MENURIGHT
		-1, //GAME_BUTTON_MENUUP
		-1, //GAME_BUTTON_MENUDOWN
		-1, //GAME_BUTTON_START
		-1, //GAME_BUTTON_SELECT
		-1, //GAME_BUTTON_BACK
		-1, //GAME_BUTTON_COIN
		-1, //GAME_BUTTON_OPERATOR
		-1, //GAME_BUTTON_EFFECT_UP
		-1, //GAME_BUTTON_EFFECT_DOWN
		18, //GAME_BUTTON_CUSTOM_01
		19, //GAME_BUTTON_CUSTOM_02
		20, //GAME_BUTTON_CUSTOM_03
		21, //GAME_BUTTON_CUSTOM_04
		22, //GAME_BUTTON_CUSTOM_05
		-1, //GAME_BUTTON_CUSTOM_06
		-1, //GAME_BUTTON_CUSTOM_07
		-1, //GAME_BUTTON_CUSTOM_08
		-1, //GAME_BUTTON_CUSTOM_09
		-1, //GAME_BUTTON_CUSTOM_10
		-1, //GAME_BUTTON_CUSTOM_11
		-1, //GAME_BUTTON_CUSTOM_12
		-1, //GAME_BUTTON_CUSTOM_13
		-1, //GAME_BUTTON_CUSTOM_14
		-1, //GAME_BUTTON_CUSTOM_15
		-1, //GAME_BUTTON_CUSTOM_16
		-1, //GAME_BUTTON_CUSTOM_17
		-1, //GAME_BUTTON_CUSTOM_18
		-1, //GAME_BUTTON_CUSTOM_19
	};

} // namespace

void LightsDriver_Linux_PIUIO_Leds::Set(const LightsState *ls)
{
	SetCabinetLights(cabinet_lights, ls);

	//gamemode is checked here as gamemode can change during runtime
	//...although I'd be impressed if someone wanted to swap from pump to dance
	//on a cabinet without shutting down first...
	//...including swapping pads from your pump cabinet to your itg cabinet...
	if (IsDance())
	{
		SetGameControllerLights(GameController_1, player1_dance_lights, ls);
		SetGameControllerLights(GameController_2, player2_dance_lights, ls);
	}
	else if (IsPump())
	{
		SetGameControllerLights(GameController_1, player1_pump_lights, ls);
		SetGameControllerLights(GameController_2, player2_pump_lights, ls);
	}

	previousLS = *ls;
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