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
#include "EnumHelper.h"


//
// Player number stuff
//
enum PlayerNumber {
	PLAYER_1 = 0,
	PLAYER_2,
	NUM_PLAYERS,	// leave this at the end
	PLAYER_INVALID
};
#define FOREACH_PlayerNumber( pn ) FOREACH_ENUM( PlayerNumber, NUM_PLAYERS, pn )
#define FOREACH_HumanPlayer( pn ) for( PlayerNumber pn=GetNextHumanPlayer((PlayerNumber)-1); pn!=PLAYER_INVALID; pn=GetNextHumanPlayer(pn) )
#define FOREACH_EnabledPlayer( pn ) for( PlayerNumber pn=GetNextEnabledPlayer((PlayerNumber)-1); pn!=PLAYER_INVALID; pn=GetNextEnabledPlayer(pn) )
#define FOREACH_CpuPlayer( pn ) for( PlayerNumber pn=GetNextCpuPlayer((PlayerNumber)-1); pn!=PLAYER_INVALID; pn=GetNextCpuPlayer(pn) )

PlayerNumber GetNextHumanPlayer( PlayerNumber pn );
PlayerNumber GetNextEnabledPlayer( PlayerNumber pn );
PlayerNumber GetNextCpuPlayer( PlayerNumber pn );

const PlayerNumber	OPPOSITE_PLAYER[NUM_PLAYERS] = { PLAYER_2, PLAYER_1 };

RageColor PlayerToColor( PlayerNumber pn );
RageColor PlayerToColor( int p );



#endif
