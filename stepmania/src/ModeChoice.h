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
#include "GameConstantsAndTypes.h"
#include "PlayerNumber.h"

struct ModeChoice		// used in SelectMode
{
	ModeChoice() { Init(); }
	void Init();

	bool FromString( CString str );
	void ApplyToAllPlayers();
	void Apply( PlayerNumber pn );

	Game		game;
	Style		style;
	PlayMode	pm;
	Difficulty	dc;
	CString		sAnnouncer;

	char		name[64];
	int			numSidesJoinedToPlay;
};

#endif
