#pragma once
/*
-----------------------------------------------------------------------------
 Class: NetworkInput

 Desc: An input event specific to a menu navigation.  This is generated based
	on a GameDef.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/


struct NetworkInput
{
	NetworkInput() { MakeInvalid(); };
	NetworkInput( PlayerNumber pn, MenuButton b ) { player = pn; button = b; };

	PlayerNumber	player;
	MenuButton		button;

//	bool operator==( const NetworkInput &other ) { return player == other.player && button == other.button; };

	inline bool IsValid() const { return player != PLAYER_INVALID; };
	inline void MakeInvalid() { player = PLAYER_INVALID; button = MENU_BUTTON_INVALID; };
};



