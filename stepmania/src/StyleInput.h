#pragma once
/*
-----------------------------------------------------------------------------
 Class: StyleInput

 Desc: An input event specific to a style that is defined by a player number and the player's note column.

 Copyright (c) 2001-2002 by the persons listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/
#include "NoteData.h"

struct StyleInput
{
	StyleInput() { player = PLAYER_NONE; col = -1; };
	StyleInput( PlayerNumber p, int c ) { player = p; col = c; };
	bool operator==( const StyleInput &other ) { return player == other.player && col == other.col; };

	PlayerNumber player;
	ColumnNumber col;

	inline bool IsBlank() const { return player == PLAYER_NONE; };
	inline bool IsValid() const { return !IsBlank(); };
	inline void MakeBlank() { player = PLAYER_NONE; col = -1; };

};