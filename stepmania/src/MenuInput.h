#ifndef MENUINPUT_H
#define MENUINPUT_H
/*
-----------------------------------------------------------------------------
 Class: MenuInput

 Desc: An input event specific to a menu navigation.  This is generated based
	on a GameDef.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
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
	MENU_BUTTON_START,
	MENU_BUTTON_BACK,
	MENU_BUTTON_INSERTCOIN,
	NUM_MENU_BUTTONS,		// leave this at the end
	MENU_BUTTON_INVALID
};


struct MenuInput
{
	MenuInput() { MakeInvalid(); };
	MenuInput( PlayerNumber pn, MenuButton b ) { player = pn; button = b; };

	PlayerNumber	player;
	MenuButton		button;

//	bool operator==( const MenuInput &other ) { return player == other.player && button == other.button; };

	inline bool IsValid() const { return player != PLAYER_INVALID; };
	inline void MakeInvalid() { player = PLAYER_INVALID; button = MENU_BUTTON_INVALID; };
};




#endif
