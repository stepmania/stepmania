#pragma once
/*
-----------------------------------------------------------------------------
 Class: MenuInput

 Desc: An input event specific to a menu navigation.  This is generated based
	on a GameDef.

 Copyright (c) 2001-2002 by the persons listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "GameConstantsAndTypes.h"

enum MenuButton
{
	MENU_BUTTON_LEFT = 0,
	MENU_BUTTON_RIGHT,
	MENU_BUTTON_UP,
	MENU_BUTTON_DOWN,
	MENU_BUTTON_NEXT,
	MENU_BUTTON_BACK,
	NUM_MENU_BUTTONS,		// leave this at the end
	MENU_BUTTON_NONE
};


struct MenuInput
{
	MenuInput() { MakeBlank(); };
	MenuInput( PlayerNumber p, MenuButton b ) { player = p; button = b; };


	PlayerNumber	player;
	MenuButton		button;

	bool operator==( const MenuInput &other ) { return player == other.player && button == other.button; };

	inline bool IsBlank() const { return player == PLAYER_NONE; };
	inline bool IsValid() const { return !IsBlank(); };
	inline void MakeBlank() { player = PLAYER_NONE; button = MENU_BUTTON_NONE; };
	
};


