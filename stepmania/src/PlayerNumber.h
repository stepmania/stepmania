#ifndef PlayerNumber_H
#define PlayerNumber_H

/*
-----------------------------------------------------------------------------
 File: PlayerNumber.h

 Desc: Things that are used in many places and don't change often.

 Copyright (c) 2001-2003 by the person(s) listed below.  All rights reserved.
	Chris Danford
	Chris Gomez
-----------------------------------------------------------------------------
*/

#include "RageTypes.h"	// for RageColor



//
// Player number stuff
//
enum PlayerNumber {
	PLAYER_1 = 0,
	PLAYER_2,
	NUM_PLAYERS,	// leave this at the end
	PLAYER_INVALID
};

RageColor PlayerToColor( PlayerNumber pn );
RageColor PlayerToColor( int p );



#endif
