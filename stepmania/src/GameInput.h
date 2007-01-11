/* GameInput - An input event specific to an InputScheme defined by a logical controller and button. */

#ifndef GAME_INPUT_H
#define GAME_INPUT_H

#include "EnumHelper.h"

class InputScheme;

enum GameController
{
	GAME_CONTROLLER_1 = 0,	// left controller
	GAME_CONTROLLER_2,	// right controller
	NUM_GameController,	// leave this at the end
	GameController_Invalid,
};
#define FOREACH_GameController( gc ) FOREACH_ENUM( GameController, gc )

typedef int GameButton;
RString GameButtonToString( const InputScheme* pInputs, GameButton i );
RString GameButtonToLocalizedString( const InputScheme* pInputs, GameButton i );
GameButton StringToGameButton( const InputScheme* pInputs, const RString& s );

enum GameButtonType
{
	GameButtonType_Step,
	GameButtonType_Fret,
	GameButtonType_Strum,
	GameButtonType_INVALID
};

const GameButton NUM_GameButton = 20;
const GameButton GameButton_Invalid = NUM_GameButton+1;
#define FOREACH_GameButton( gb ) FOREACH_ENUM( GameButton, gb )

enum	// common GameButtons
{
	GAME_BUTTON_MENULEFT,
	GAME_BUTTON_MENURIGHT,
	GAME_BUTTON_MENUUP,
	GAME_BUTTON_MENUDOWN,
	GAME_BUTTON_START,
	GAME_BUTTON_SELECT,
	GAME_BUTTON_BACK,
	GAME_BUTTON_COIN,
	GAME_BUTTON_OPERATOR,
	GAME_BUTTON_NEXT
};

enum	// DanceButtons
{
	DANCE_BUTTON_LEFT = GAME_BUTTON_NEXT,
	DANCE_BUTTON_RIGHT,
	DANCE_BUTTON_UP,
	DANCE_BUTTON_DOWN,
	DANCE_BUTTON_UPLEFT,
	DANCE_BUTTON_UPRIGHT,
	NUM_DANCE_BUTTONS,		// leave this at the end
};

enum	// PumpButtons
{
	PUMP_BUTTON_UPLEFT = GAME_BUTTON_NEXT,
	PUMP_BUTTON_UPRIGHT,
	PUMP_BUTTON_CENTER,
	PUMP_BUTTON_DOWNLEFT,
	PUMP_BUTTON_DOWNRIGHT,
	NUM_PUMP_BUTTONS,		// leave this at the end
};

enum	// EZ2Buttons
{
	EZ2_BUTTON_FOOTUPLEFT = GAME_BUTTON_NEXT,
	EZ2_BUTTON_FOOTUPRIGHT,
	EZ2_BUTTON_FOOTDOWN,
	EZ2_BUTTON_HANDUPLEFT,
	EZ2_BUTTON_HANDUPRIGHT,
	EZ2_BUTTON_HANDLRLEFT,
	EZ2_BUTTON_HANDLRRIGHT,
	NUM_EZ2_BUTTONS,		// leave this at the end
};

enum	// ParaButtons
{
	PARA_BUTTON_LEFT = GAME_BUTTON_NEXT,
	PARA_BUTTON_UPLEFT,
	PARA_BUTTON_UP,
	PARA_BUTTON_UPRIGHT,
	PARA_BUTTON_RIGHT,
	NUM_PARA_BUTTONS,		// leave this at the end
};

enum // 3DDX Buttons
{
	DS3DDX_BUTTON_HANDLEFT = GAME_BUTTON_NEXT,
	DS3DDX_BUTTON_FOOTDOWNLEFT,
	DS3DDX_BUTTON_FOOTUPLEFT,
	DS3DDX_BUTTON_HANDUP,
	DS3DDX_BUTTON_HANDDOWN,
	DS3DDX_BUTTON_FOOTUPRIGHT,
	DS3DDX_BUTTON_FOOTDOWNRIGHT,
	DS3DDX_BUTTON_HANDRIGHT,
	NUM_DS3DDX_BUTTONS, // leave this at the end.
};

enum // BEAT Buttons
{
	BEAT_BUTTON_KEY1 = GAME_BUTTON_NEXT,
	BEAT_BUTTON_KEY2,
	BEAT_BUTTON_KEY3,
	BEAT_BUTTON_KEY4,
	BEAT_BUTTON_KEY5,
	BEAT_BUTTON_KEY6,
	BEAT_BUTTON_KEY7,
	BEAT_BUTTON_SCRATCHUP,
	/* XXX special case: this button is an alias of BEAT_BUTTON_SCRATCHUP for track
	 * matching. */
	BEAT_BUTTON_SCRATCHDOWN,
	NUM_BEAT_BUTTONS, // leave this at the end.
};

enum	// ManiaxButtons
{
	MANIAX_BUTTON_HANDUPLEFT = GAME_BUTTON_NEXT,
	MANIAX_BUTTON_HANDUPRIGHT,
	MANIAX_BUTTON_HANDLRLEFT,
	MANIAX_BUTTON_HANDLRRIGHT,
	NUM_MANIAX_BUTTONS,		// leave this at the end
};

enum	// TechnoButtons
{
	TECHNO_BUTTON_LEFT = GAME_BUTTON_NEXT,
	TECHNO_BUTTON_RIGHT,
	TECHNO_BUTTON_UP,
	TECHNO_BUTTON_DOWN,
	TECHNO_BUTTON_UPLEFT,
	TECHNO_BUTTON_UPRIGHT,
	TECHNO_BUTTON_CENTER,
	TECHNO_BUTTON_DOWNLEFT,
	TECHNO_BUTTON_DOWNRIGHT,
	NUM_TECHNO_BUTTONS,		// leave this at the end
};

enum	// PnM Buttons
{
	POPN_BUTTON_LEFT_WHITE = GAME_BUTTON_NEXT,
	POPN_BUTTON_LEFT_YELLOW,
	POPN_BUTTON_LEFT_GREEN,
	POPN_BUTTON_LEFT_BLUE,
	POPN_BUTTON_RED,
	POPN_BUTTON_RIGHT_BLUE,
	POPN_BUTTON_RIGHT_GREEN,
	POPN_BUTTON_RIGHT_YELLOW,
	POPN_BUTTON_RIGHT_WHITE,
	NUM_POPN_BUTTONS,		// leave this at the end
};

enum	// LightsButtons
{
	LIGHTS_BUTTON_MARQUEE_UP_LEFT = GAME_BUTTON_NEXT,
	LIGHTS_BUTTON_MARQUEE_UP_RIGHT,
	LIGHTS_BUTTON_MARQUEE_LR_LEFT,
	LIGHTS_BUTTON_MARQUEE_LR_RIGHT,
	LIGHTS_BUTTON_BUTTONS_LEFT,
	LIGHTS_BUTTON_BUTTONS_RIGHT,
	LIGHTS_BUTTON_BASS_LEFT,
	LIGHTS_BUTTON_BASS_RIGHT,
	NUM_LIGHTS_BUTTONS,		// leave this at the end
};


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

/*
 * (c) 2001-2004 Chris Danford
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
