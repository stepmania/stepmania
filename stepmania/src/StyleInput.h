#pragma once
/*
-----------------------------------------------------------------------------
 Class: StyleInput

 Desc: An input event specific to a style that is defined by a player number and the player's note column.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/
#include "NoteData.h"

struct StyleInput
{
	PlayerNumber	player;
	int				col;

	StyleInput() { MakeInvalid(); };
	StyleInput( PlayerNumber p, int c ) { player = p; col = c; };
	bool operator==( const StyleInput &other ) { return player == other.player && col == other.col; };

	inline bool IsValid() const { return player != PLAYER_INVALID; };
	inline void MakeInvalid() { player = PLAYER_INVALID; col = -1; };
};