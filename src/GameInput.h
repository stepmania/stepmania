#ifndef GAME_INPUT_H
#define GAME_INPUT_H

#include "EnumHelper.h"

class InputScheme;

/** @brief the list of controllers in use. */
enum GameController
{
	GameController_1 = 0,	/**< The left controller */
	GameController_2,	/**< The right controller */
	NUM_GameController,	/**< The number of controllers allowed. */
	GameController_Invalid,
};
const RString& GameControllerToString( GameController mp );
LuaDeclareType( GameController );

/** @brief the list of buttons StepMania recognizes. */
enum GameButton
{
	GAME_BUTTON_MENULEFT, /**< Navigate the menus to the left. */
	GAME_BUTTON_MENURIGHT, /**< Navigate the menus to the right. */
	GAME_BUTTON_MENUUP, /**< Navigate the menus to the top. */
	GAME_BUTTON_MENUDOWN, /**< Navigate the menus to the bottom. */
	GAME_BUTTON_START,
	GAME_BUTTON_SELECT,
	GAME_BUTTON_BACK,
	GAME_BUTTON_COIN, /**< Insert a coin to play. */
	GAME_BUTTON_OPERATOR, /**< Access the operator menu. */
	GAME_BUTTON_EFFECT_UP,
	GAME_BUTTON_EFFECT_DOWN,
	GAME_BUTTON_CUSTOM_01,
	GAME_BUTTON_CUSTOM_02,
	GAME_BUTTON_CUSTOM_03,
	GAME_BUTTON_CUSTOM_04,
	GAME_BUTTON_CUSTOM_05,
	GAME_BUTTON_CUSTOM_06,
	GAME_BUTTON_CUSTOM_07,
	GAME_BUTTON_CUSTOM_08,
	GAME_BUTTON_CUSTOM_09,
	GAME_BUTTON_CUSTOM_10,
	GAME_BUTTON_CUSTOM_11,
	GAME_BUTTON_CUSTOM_12,
	GAME_BUTTON_CUSTOM_13,
	GAME_BUTTON_CUSTOM_14,
	GAME_BUTTON_CUSTOM_15,
	GAME_BUTTON_CUSTOM_16,
	GAME_BUTTON_CUSTOM_17,
	GAME_BUTTON_CUSTOM_18,
	GAME_BUTTON_CUSTOM_19,

	NUM_GameButton,
	GameButton_Invalid,

	/* game-specific aliases for custom GameButtons */

	// dance
	DANCE_BUTTON_LEFT = GAME_BUTTON_CUSTOM_01,
	DANCE_BUTTON_RIGHT,
	DANCE_BUTTON_UP,
	DANCE_BUTTON_DOWN,
	DANCE_BUTTON_UPLEFT,
	DANCE_BUTTON_UPRIGHT,
	NUM_DANCE_BUTTONS,

	// pump
	PUMP_BUTTON_UPLEFT = GAME_BUTTON_CUSTOM_01,
	PUMP_BUTTON_UPRIGHT,
	PUMP_BUTTON_CENTER,
	PUMP_BUTTON_DOWNLEFT,
	PUMP_BUTTON_DOWNRIGHT,
	NUM_PUMP_BUTTONS,

	// kb7
	KB7_BUTTON_KEY1 = GAME_BUTTON_CUSTOM_01,
	KB7_BUTTON_KEY2,
	KB7_BUTTON_KEY3,
	KB7_BUTTON_KEY4,
	KB7_BUTTON_KEY5,
	KB7_BUTTON_KEY6,
	KB7_BUTTON_KEY7,
	NUM_KB7_BUTTONS,

	// ez2(dancer)
	EZ2_BUTTON_FOOTUPLEFT = GAME_BUTTON_CUSTOM_01,
	EZ2_BUTTON_FOOTUPRIGHT,
	EZ2_BUTTON_FOOTDOWN,
	EZ2_BUTTON_HANDUPLEFT,
	EZ2_BUTTON_HANDUPRIGHT,
	EZ2_BUTTON_HANDLRLEFT,
	EZ2_BUTTON_HANDLRRIGHT,
	NUM_EZ2_BUTTONS,

	// para
	PARA_BUTTON_LEFT = GAME_BUTTON_CUSTOM_01,
	PARA_BUTTON_UPLEFT,
	PARA_BUTTON_UP,
	PARA_BUTTON_UPRIGHT,
	PARA_BUTTON_RIGHT,
	NUM_PARA_BUTTONS,

	// ds3ddx
	DS3DDX_BUTTON_HANDLEFT = GAME_BUTTON_CUSTOM_01,
	DS3DDX_BUTTON_FOOTDOWNLEFT,
	DS3DDX_BUTTON_FOOTUPLEFT,
	DS3DDX_BUTTON_HANDUP,
	DS3DDX_BUTTON_HANDDOWN,
	DS3DDX_BUTTON_FOOTUPRIGHT,
	DS3DDX_BUTTON_FOOTDOWNRIGHT,
	DS3DDX_BUTTON_HANDRIGHT,
	NUM_DS3DDX_BUTTONS,

	// beat
	BEAT_BUTTON_KEY1 = GAME_BUTTON_CUSTOM_01,
	BEAT_BUTTON_KEY2,
	BEAT_BUTTON_KEY3,
	BEAT_BUTTON_KEY4,
	BEAT_BUTTON_KEY5,
	BEAT_BUTTON_KEY6,
	BEAT_BUTTON_KEY7,
	BEAT_BUTTON_SCRATCHUP,
	BEAT_BUTTON_SCRATCHDOWN,
	NUM_BEAT_BUTTONS,

	// maniax
	MANIAX_BUTTON_HANDUPLEFT = GAME_BUTTON_CUSTOM_01,
	MANIAX_BUTTON_HANDUPRIGHT,
	MANIAX_BUTTON_HANDLRLEFT,
	MANIAX_BUTTON_HANDLRRIGHT,
	NUM_MANIAX_BUTTONS,

	// techno
	TECHNO_BUTTON_LEFT = GAME_BUTTON_CUSTOM_01,
	TECHNO_BUTTON_RIGHT,
	TECHNO_BUTTON_UP,
	TECHNO_BUTTON_DOWN,
	TECHNO_BUTTON_UPLEFT,
	TECHNO_BUTTON_UPRIGHT,
	TECHNO_BUTTON_CENTER,
	TECHNO_BUTTON_DOWNLEFT,
	TECHNO_BUTTON_DOWNRIGHT,
	NUM_TECHNO_BUTTONS,

	// popn
	POPN_BUTTON_LEFT_WHITE = GAME_BUTTON_CUSTOM_01,
	POPN_BUTTON_LEFT_YELLOW,
	POPN_BUTTON_LEFT_GREEN,
	POPN_BUTTON_LEFT_BLUE,
	POPN_BUTTON_RED,
	POPN_BUTTON_RIGHT_BLUE,
	POPN_BUTTON_RIGHT_GREEN,
	POPN_BUTTON_RIGHT_YELLOW,
	POPN_BUTTON_RIGHT_WHITE,
	NUM_POPN_BUTTONS,

	// lights
	LIGHTS_BUTTON_MARQUEE_UP_LEFT = GAME_BUTTON_CUSTOM_01,
	LIGHTS_BUTTON_MARQUEE_UP_RIGHT,
	LIGHTS_BUTTON_MARQUEE_LR_LEFT,
	LIGHTS_BUTTON_MARQUEE_LR_RIGHT,
	LIGHTS_BUTTON_BUTTONS_LEFT,
	LIGHTS_BUTTON_BUTTONS_RIGHT,
	LIGHTS_BUTTON_BASS_LEFT,
	LIGHTS_BUTTON_BASS_RIGHT,
	NUM_LIGHTS_BUTTONS,
};

RString GameButtonToString( const InputScheme* pInputs, GameButton i );
RString GameButtonToLocalizedString( const InputScheme* pInputs, GameButton i );
GameButton StringToGameButton( const InputScheme* pInputs, const RString& s );

#define GAME_BUTTON_NEXT		GAME_BUTTON_CUSTOM_01

/** @brief A special way to loop through each game button. */
#define FOREACH_GameButton_Custom( gb ) for( GameButton gb=GAME_BUTTON_NEXT; gb<NUM_GameButton; enum_add(gb, +1) )

/* XXX: compatibility with older declarations. we should remove this... */
#define GAME_BUTTON_LEFT GAME_BUTTON_MENULEFT
#define GAME_BUTTON_RIGHT GAME_BUTTON_MENURIGHT
#define GAME_BUTTON_UP GAME_BUTTON_MENUUP
#define GAME_BUTTON_DOWN GAME_BUTTON_MENUDOWN
#define GAME_BUTTON_START GAME_BUTTON_START
#define GAME_BUTTON_BACK GAME_BUTTON_BACK

/** @brief An input event specific to an InputScheme defined by a logical controller and button. */
struct GameInput
{
	GameInput(): controller(GameController_Invalid), button(GameButton_Invalid) { }

	GameInput( GameController c, GameButton b ): controller(c), button(b) { }

	GameController	controller;
	GameButton	button;

	bool operator==( const GameInput &other ) const { return controller == other.controller && button == other.button; };
	bool operator<( const GameInput &other ) const
	{
		if( controller < other.controller )
			return true;
		else if( controller > other.controller )
			return false;
		return button < other.button;
	}

	inline bool IsValid() const { return controller != GameController_Invalid && button != GameButton_Invalid; };
	inline void MakeInvalid() { controller = GameController_Invalid; button = GameButton_Invalid; };

	RString ToString( const InputScheme* pInputs ) const;
	bool FromString( const InputScheme* pInputs, const RString &s );
};

#endif

/**
 * @file
 * @author Chris Danford (c) 2001-2004
 * @section LICENSE
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
