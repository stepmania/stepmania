#ifndef STYLEINPUT_H
#define STYLEINPUT_H
/*
-----------------------------------------------------------------------------
 Class: StyleInput

 Desc: An input event specific to a style that is defined by a player number and the player's note column.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "PlayerNumber.h"


struct StyleInput
{
	PlayerNumber	player;
	int				col;

	StyleInput() { MakeInvalid(); };
	StyleInput( PlayerNumber pn, int c ) { player = pn; col = c; };
	bool operator==( const StyleInput &other ) { return player == other.player && col == other.col; };

	inline bool IsValid() const { return player != PLAYER_INVALID; };
	inline void MakeInvalid() { player = PLAYER_INVALID; col = -1; };
};
#endif
