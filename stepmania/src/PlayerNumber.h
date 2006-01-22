/* PlayerNumber - A simple type representing a player. */

#ifndef PlayerNumber_H
#define PlayerNumber_H

#include "EnumHelper.h"


//
// Player number stuff
//
enum PlayerNumber
{
	PLAYER_1 = 0,
	PLAYER_2,
	NUM_PlayerNumber,	// leave this at the end
	PLAYER_INVALID
};
const int NUM_PLAYERS = NUM_PlayerNumber;
const RString& PlayerNumberToString( PlayerNumber pn );
const RString& PlayerNumberToLocalizedString( PlayerNumber pn );

#define FOREACH_PlayerNumber( pn ) FOREACH_ENUM( PlayerNumber, NUM_PLAYERS, pn )
#define FOREACH_HumanPlayer( pn ) for( PlayerNumber pn=GetNextHumanPlayer((PlayerNumber)-1); pn!=PLAYER_INVALID; pn=GetNextHumanPlayer(pn) )
#define FOREACH_EnabledPlayer( pn ) for( PlayerNumber pn=GetNextEnabledPlayer((PlayerNumber)-1); pn!=PLAYER_INVALID; pn=GetNextEnabledPlayer(pn) )
#define FOREACH_CpuPlayer( pn ) for( PlayerNumber pn=GetNextCpuPlayer((PlayerNumber)-1); pn!=PLAYER_INVALID; pn=GetNextCpuPlayer(pn) )
#define FOREACH_PotentialCpuPlayer( pn ) for( PlayerNumber pn=GetNextPotentialCpuPlayer((PlayerNumber)-1); pn!=PLAYER_INVALID; pn=GetNextPotentialCpuPlayer(pn) )

PlayerNumber GetNextHumanPlayer( PlayerNumber pn );
PlayerNumber GetNextEnabledPlayer( PlayerNumber pn );
PlayerNumber GetNextCpuPlayer( PlayerNumber pn );
PlayerNumber GetNextPotentialCpuPlayer( PlayerNumber pn );

const PlayerNumber	OPPOSITE_PLAYER[NUM_PLAYERS] = { PLAYER_2, PLAYER_1 };


enum MultiPlayer
{
	MultiPlayer_1 = 0,
	MultiPlayer_2,
	MultiPlayer_3,
	MultiPlayer_4,
	MultiPlayer_5,
	MultiPlayer_6,
	MultiPlayer_7,
	MultiPlayer_8,
	NUM_MultiPlayer,	// leave this at the end
	MultiPlayer_INVALID
};
const RString& MultiPlayerToString( MultiPlayer mp );
const RString& MultiPlayerToLocalizedString( MultiPlayer mp );

#define FOREACH_MultiPlayer( pn ) FOREACH_ENUM( MultiPlayer, NUM_MultiPlayer, pn )

#define FOREACH_EnabledMultiPlayer( mp ) for( MultiPlayer mp=GetNextEnabledMultiPlayer((MultiPlayer)-1); mp!=MultiPlayer_INVALID; mp=GetNextEnabledMultiPlayer(mp) )

MultiPlayer GetNextEnabledMultiPlayer( MultiPlayer mp );

#endif

/*
 * (c) 2001-2004 Chris Danford, Chris Gomez
 * All rights reserved.
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, and/or sell copies of the Software, and to permit persons to
 * whom the Software is furnished to do so, provided that the above
 * copyright notice(s) and this permission notice appear in all copies of
 * the Software and that both the above copyright notice(s) and this
 * permission notice appear in supporting documentation.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT OF
 * THIRD PARTY RIGHTS. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR HOLDERS
 * INCLUDED IN THIS NOTICE BE LIABLE FOR ANY CLAIM, OR ANY SPECIAL INDIRECT
 * OR CONSEQUENTIAL DAMAGES, OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS
 * OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
 * OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */
