#pragma once
/*
-----------------------------------------------------------------------------
 Class: GameInput

 Desc: An input event specific to a Game definied by an instrument and a button space.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/
enum GameController
{
	GAME_CONTROLLER_1 = 0,	// left controller
	GAME_CONTROLLER_2,		// right controller
	MAX_GAME_CONTROLLERS,	// leave this at the end
	GAME_CONTROLLER_INVALID,
};

typedef int GameButton;

enum	// DanceButtons
{
	DANCE_BUTTON_LEFT,
	DANCE_BUTTON_RIGHT,
	DANCE_BUTTON_UP,
	DANCE_BUTTON_DOWN,
	DANCE_BUTTON_UPLEFT,
	DANCE_BUTTON_UPRIGHT,
	DANCE_BUTTON_START,
	DANCE_BUTTON_BACK,
	DANCE_BUTTON_MENULEFT,
	DANCE_BUTTON_MENURIGHT,
	DANCE_BUTTON_MENUUP,
	DANCE_BUTTON_MENUDOWN,
	NUM_DANCE_BUTTONS,		// leave this at the end
};

enum	// PumpButtons
{
	PUMP_BUTTON_UPLEFT,
	PUMP_BUTTON_UPRIGHT,
	PUMP_BUTTON_CENTER,
	PUMP_BUTTON_DOWNLEFT,
	PUMP_BUTTON_DOWNRIGHT,
	PUMP_BUTTON_START,
	PUMP_BUTTON_BACK,
	PUMP_BUTTON_MENULEFT,
	PUMP_BUTTON_MENURIGHT,
	PUMP_BUTTON_MENUUP,
	PUMP_BUTTON_MENUDOWN,
	NUM_PUMP_BUTTONS,		// leave this at the end
};

enum	// EZ2Buttons
{
	EZ2_BUTTON_FOOTUPLEFT,
	EZ2_BUTTON_FOOTUPRIGHT,
	EZ2_BUTTON_FOOTDOWN,
	EZ2_BUTTON_HANDUPLEFT,
	EZ2_BUTTON_HANDUPRIGHT,
	EZ2_BUTTON_HANDLRLEFT,
	EZ2_BUTTON_HANDLRRIGHT,
	EZ2_BUTTON_START,
	EZ2_BUTTON_BACK,
	EZ2_BUTTON_MENULEFT,
	EZ2_BUTTON_MENURIGHT,
	EZ2_BUTTON_MENUUP,
	EZ2_BUTTON_MENUDOWN,
	NUM_EZ2_BUTTONS,		// leave this at the end
};


const GameButton MAX_GAME_BUTTONS = 14;
const GameButton GAME_BUTTON_INVALID = MAX_GAME_BUTTONS+1;



struct GameInput
{
	GameInput(): controller(GAME_CONTROLLER_INVALID), button(GAME_BUTTON_INVALID) { }

	GameInput( GameController c, GameButton b ): controller(c), button(b) { }

	GameController	controller;
	GameButton		button;

	bool operator==( const GameInput &other ) { return controller == other.controller && button == other.button; };

	inline bool IsValid() const { return controller != GAME_CONTROLLER_INVALID; };
	inline void MakeInvalid() { controller = GAME_CONTROLLER_INVALID; button = GAME_BUTTON_INVALID; };

	CString toString();
	bool fromString( CString s );
};


