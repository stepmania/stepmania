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
	GAME_CONTROLLER_1 = 0,
	GAME_CONTROLLER_2,
	MAX_GAME_CONTROLLERS,	// leave this at the end
	GAME_CONTROLLER_NONE,
};

enum GameButton
{
	GAME_BUTTON_1 = 0,
	GAME_BUTTON_2,
	GAME_BUTTON_3,
	GAME_BUTTON_4,
	GAME_BUTTON_5,
	GAME_BUTTON_6,
	GAME_BUTTON_7,
	GAME_BUTTON_8,
	GAME_BUTTON_9,
	GAME_BUTTON_10,
	GAME_BUTTON_11,
	GAME_BUTTON_12,
	GAME_BUTTON_13,
	GAME_BUTTON_14,
	GAME_BUTTON_15,
	GAME_BUTTON_16,
	MAX_GAME_BUTTONS,		// leave this at the end
	GAME_BUTTON_NONE
};


struct GameInput
{
	GameInput() { MakeBlank(); };
	GameInput( GameController c, GameButton b ) { controller = c; button = b; };


	GameController	controller;	// 
	GameButton		button;	// instrument button

	bool operator==( const GameInput &other ) { return controller == other.controller && button == other.button; };

	inline bool IsBlank() const { return controller == GAME_CONTROLLER_NONE; };
	inline bool IsValid() const { return !IsBlank(); };
	inline void MakeBlank() { controller = GAME_CONTROLLER_NONE; button = GAME_BUTTON_NONE; };

	CString toString();
	bool fromString( CString s );
};


