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
	GameButton_Invalid
};

RString GameButtonToString( const InputScheme* pInputs, GameButton i );
RString GameButtonToLocalizedString( const InputScheme* pInputs, GameButton i );
GameButton StringToGameButton( const InputScheme* pInputs, const RString& s );

/** @brief A special way to loop through each game button. */
#define FOREACH_GameButton_Custom( gb ) for( GameButton gb=GAME_BUTTON_CUSTOM_01; gb<NUM_GameButton; enum_add(gb, +1) )

#define GAME_BUTTON_NEXT		GAME_BUTTON_CUSTOM_01

// dance
/** @brief Set up the left arrow for dance mode. */
#define DANCE_BUTTON_LEFT		GAME_BUTTON_CUSTOM_01
/** @brief Set up the right arrow for dance mode. */
#define DANCE_BUTTON_RIGHT		GAME_BUTTON_CUSTOM_02
/** @brief Set up the up arrow for dance mode. */
#define DANCE_BUTTON_UP			GAME_BUTTON_CUSTOM_03
/** @brief Set up the down arrow for dance mode. */
#define DANCE_BUTTON_DOWN		GAME_BUTTON_CUSTOM_04
/** @brief Set up the upleft arrow for dance mode (solo). */
#define DANCE_BUTTON_UPLEFT		GAME_BUTTON_CUSTOM_05
/** @brief Set up the upright arrow for dance mode (solo). */
#define DANCE_BUTTON_UPRIGHT	GAME_BUTTON_CUSTOM_06
#define NUM_DANCE_BUTTONS		GAME_BUTTON_CUSTOM_07
// pump
/** @brief Set up the upleft arrow for pump mode. */
#define PUMP_BUTTON_UPLEFT		GAME_BUTTON_CUSTOM_01
/** @brief Set up the upright arrow for pump mode. */
#define PUMP_BUTTON_UPRIGHT		GAME_BUTTON_CUSTOM_02
/** @brief Set up the center arrow for pump mode. */
#define PUMP_BUTTON_CENTER		GAME_BUTTON_CUSTOM_03
/** @brief Set up the downleft arrow for pump mode. */
#define PUMP_BUTTON_DOWNLEFT	GAME_BUTTON_CUSTOM_04
/** @brief Set up the downright arrow for pump mode. */
#define PUMP_BUTTON_DOWNRIGHT	GAME_BUTTON_CUSTOM_05
#define NUM_PUMP_BUTTONS		GAME_BUTTON_CUSTOM_06
// kb7
#define KB7_BUTTON_KEY1			GAME_BUTTON_CUSTOM_01
#define KB7_BUTTON_KEY2			GAME_BUTTON_CUSTOM_02
#define KB7_BUTTON_KEY3			GAME_BUTTON_CUSTOM_03
#define KB7_BUTTON_KEY4			GAME_BUTTON_CUSTOM_04
#define KB7_BUTTON_KEY5			GAME_BUTTON_CUSTOM_05
#define KB7_BUTTON_KEY6			GAME_BUTTON_CUSTOM_06
#define KB7_BUTTON_KEY7			GAME_BUTTON_CUSTOM_07
#define NUM_KB7_BUTTONS			GAME_BUTTON_CUSTOM_08
// ez2(dancer)
#define EZ2_BUTTON_FOOTUPLEFT	GAME_BUTTON_CUSTOM_01
#define EZ2_BUTTON_FOOTUPRIGHT	GAME_BUTTON_CUSTOM_02
#define EZ2_BUTTON_FOOTDOWN		GAME_BUTTON_CUSTOM_03
#define EZ2_BUTTON_HANDUPLEFT	GAME_BUTTON_CUSTOM_04
#define EZ2_BUTTON_HANDUPRIGHT	GAME_BUTTON_CUSTOM_05
#define EZ2_BUTTON_HANDLRLEFT	GAME_BUTTON_CUSTOM_06
#define EZ2_BUTTON_HANDLRRIGHT	GAME_BUTTON_CUSTOM_07
#define NUM_EZ2_BUTTONS			GAME_BUTTON_CUSTOM_08
// para
#define PARA_BUTTON_LEFT		GAME_BUTTON_CUSTOM_01
#define PARA_BUTTON_UPLEFT		GAME_BUTTON_CUSTOM_02
#define PARA_BUTTON_UP			GAME_BUTTON_CUSTOM_03
#define PARA_BUTTON_UPRIGHT		GAME_BUTTON_CUSTOM_04
#define PARA_BUTTON_RIGHT		GAME_BUTTON_CUSTOM_05
#define NUM_PARA_BUTTONS		GAME_BUTTON_CUSTOM_06
// ds3ddx
#define DS3DDX_BUTTON_HANDLEFT		GAME_BUTTON_CUSTOM_01
#define DS3DDX_BUTTON_FOOTDOWNLEFT	GAME_BUTTON_CUSTOM_02
#define DS3DDX_BUTTON_FOOTUPLEFT	GAME_BUTTON_CUSTOM_03
#define DS3DDX_BUTTON_HANDUP		GAME_BUTTON_CUSTOM_04
#define DS3DDX_BUTTON_HANDDOWN		GAME_BUTTON_CUSTOM_05
#define DS3DDX_BUTTON_FOOTUPRIGHT	GAME_BUTTON_CUSTOM_06
#define DS3DDX_BUTTON_FOOTDOWNRIGHT	GAME_BUTTON_CUSTOM_07
#define DS3DDX_BUTTON_HANDRIGHT		GAME_BUTTON_CUSTOM_08
#define NUM_DS3DDX_BUTTONS		GAME_BUTTON_CUSTOM_09
// beat
#define BEAT_BUTTON_KEY1		GAME_BUTTON_CUSTOM_01
#define BEAT_BUTTON_KEY2		GAME_BUTTON_CUSTOM_02
#define BEAT_BUTTON_KEY3		GAME_BUTTON_CUSTOM_03
#define BEAT_BUTTON_KEY4		GAME_BUTTON_CUSTOM_04
#define BEAT_BUTTON_KEY5		GAME_BUTTON_CUSTOM_05
#define BEAT_BUTTON_KEY6		GAME_BUTTON_CUSTOM_06
#define BEAT_BUTTON_KEY7		GAME_BUTTON_CUSTOM_07
#define BEAT_BUTTON_SCRATCHUP	GAME_BUTTON_CUSTOM_08
#define BEAT_BUTTON_SCRATCHDOWN	GAME_BUTTON_CUSTOM_09
#define NUM_BEAT_BUTTONS		GAME_BUTTON_CUSTOM_10
// maniax
#define MANIAX_BUTTON_HANDUPLEFT	GAME_BUTTON_CUSTOM_01
#define MANIAX_BUTTON_HANDUPRIGHT	GAME_BUTTON_CUSTOM_02
#define MANIAX_BUTTON_HANDLRLEFT	GAME_BUTTON_CUSTOM_03
#define MANIAX_BUTTON_HANDLRRIGHT	GAME_BUTTON_CUSTOM_04
#define NUM_MANIAX_BUTTONS		GAME_BUTTON_CUSTOM_05
// techno
#define TECHNO_BUTTON_LEFT		GAME_BUTTON_CUSTOM_01
#define TECHNO_BUTTON_RIGHT		GAME_BUTTON_CUSTOM_02
#define TECHNO_BUTTON_UP		GAME_BUTTON_CUSTOM_03
#define TECHNO_BUTTON_DOWN		GAME_BUTTON_CUSTOM_04
#define TECHNO_BUTTON_UPLEFT	GAME_BUTTON_CUSTOM_05
#define TECHNO_BUTTON_UPRIGHT	GAME_BUTTON_CUSTOM_06
#define TECHNO_BUTTON_CENTER	GAME_BUTTON_CUSTOM_07
#define TECHNO_BUTTON_DOWNLEFT	GAME_BUTTON_CUSTOM_08
#define TECHNO_BUTTON_DOWNRIGHT	GAME_BUTTON_CUSTOM_09
#define NUM_TECHNO_BUTTONS		GAME_BUTTON_CUSTOM_10
// popn
#define POPN_BUTTON_LEFT_WHITE		GAME_BUTTON_CUSTOM_01
#define POPN_BUTTON_LEFT_YELLOW		GAME_BUTTON_CUSTOM_02
#define POPN_BUTTON_LEFT_GREEN		GAME_BUTTON_CUSTOM_03
#define POPN_BUTTON_LEFT_BLUE		GAME_BUTTON_CUSTOM_04
#define POPN_BUTTON_RED				GAME_BUTTON_CUSTOM_05
#define POPN_BUTTON_RIGHT_BLUE		GAME_BUTTON_CUSTOM_06
#define POPN_BUTTON_RIGHT_GREEN		GAME_BUTTON_CUSTOM_07
#define POPN_BUTTON_RIGHT_YELLOW	GAME_BUTTON_CUSTOM_08
#define POPN_BUTTON_RIGHT_WHITE		GAME_BUTTON_CUSTOM_09
#define NUM_POPN_BUTTONS			GAME_BUTTON_CUSTOM_10

#define LIGHTS_BUTTON_MARQUEE_UP_LEFT	GAME_BUTTON_CUSTOM_01
#define LIGHTS_BUTTON_MARQUEE_UP_RIGHT	GAME_BUTTON_CUSTOM_02
#define LIGHTS_BUTTON_MARQUEE_LR_LEFT	GAME_BUTTON_CUSTOM_03
#define LIGHTS_BUTTON_MARQUEE_LR_RIGHT	GAME_BUTTON_CUSTOM_04
#define LIGHTS_BUTTON_BUTTONS_LEFT	GAME_BUTTON_CUSTOM_05
#define LIGHTS_BUTTON_BUTTONS_RIGHT	GAME_BUTTON_CUSTOM_06
#define LIGHTS_BUTTON_BASS_LEFT		GAME_BUTTON_CUSTOM_07
#define LIGHTS_BUTTON_BASS_RIGHT	GAME_BUTTON_CUSTOM_08
#define NUM_LIGHTS_BUTTONS		GAME_BUTTON_CUSTOM_09

#define KICKBOX_BUTTON_DOWN_LEFT_FOOT GAME_BUTTON_CUSTOM_01
#define KICKBOX_BUTTON_UP_LEFT_FOOT GAME_BUTTON_CUSTOM_02
#define KICKBOX_BUTTON_UP_LEFT_FIST GAME_BUTTON_CUSTOM_03
#define KICKBOX_BUTTON_DOWN_LEFT_FIST GAME_BUTTON_CUSTOM_04
#define KICKBOX_BUTTON_DOWN_RIGHT_FIST GAME_BUTTON_CUSTOM_05
#define KICKBOX_BUTTON_UP_RIGHT_FIST GAME_BUTTON_CUSTOM_06
#define KICKBOX_BUTTON_UP_RIGHT_FOOT GAME_BUTTON_CUSTOM_07
#define KICKBOX_BUTTON_DOWN_RIGHT_FOOT GAME_BUTTON_CUSTOM_08
#define NUM_KICKBOX_BUTTONS GAME_BUTTON_CUSTOM_09

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
