#ifndef MODECHOICE_H
#define MODECHOICE_H
/*
-----------------------------------------------------------------------------
 Class: ModeChoice

 Desc: .

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "Game.h"
#include "Style.h"

struct ModeChoice		// used in SelectMode
{
	Game		game;
	PlayMode	pm;
	Style		style;
	Difficulty	dc;
	char		name[64];
	int			numSidesJoinedToPlay;
};

#endif
