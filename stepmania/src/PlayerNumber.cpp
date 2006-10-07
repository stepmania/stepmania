#include "global.h"
#include "PlayerNumber.h"
#include "GameState.h"
#include "LuaManager.h"
#include "LocalizedString.h"

static const char *PlayerNumberNames[] = {
	"P1",
	"P2",
};
XToString( PlayerNumber, NUM_PlayerNumber );
XToLocalizedString( PlayerNumber );
LuaFunction( PlayerNumberToString, PlayerNumberToString(Enum::Check<PlayerNumber>(L, 1)) );
LuaXType( PlayerNumber );

static const char *MultiPlayerNames[] = {
	"P1",
	"P2",
	"P3",
	"P4",
	"P5",
	"P6",
	"P7",
	"P8",
	"P9",
	"P10",
	"P11",
	"P12",
	"P13",
	"P14",
	"P15",
	"P16",
	"P17",
	"P18",
	"P19",
	"P20",
	"P21",
	"P22",
	"P23",
	"P24",
	"P25",
	"P26",
	"P27",
	"P28",
	"P29",
	"P30",
	"P31",
	"P32",
};
XToString( MultiPlayer, NUM_MultiPlayer );
XToLocalizedString( MultiPlayer );
LuaFunction( MultiPlayerToString, MultiPlayerToString(Enum::Check<MultiPlayer>(L, 1)) );
LuaFunction( MultiPlayerToLocalizedString, MultiPlayerToLocalizedString(Enum::Check<MultiPlayer>(L, 1)) );
LuaXType( MultiPlayer );


PlayerNumber GetNextHumanPlayer( PlayerNumber pn )
{
	for( enum_add(pn, 1); pn < NUM_PLAYERS; enum_add(pn, 1) )
		if( GAMESTATE->IsHumanPlayer(pn) )
			return pn;
	return PLAYER_INVALID;
}

PlayerNumber GetNextEnabledPlayer( PlayerNumber pn )
{
	for( enum_add(pn, 1); pn < NUM_PLAYERS; enum_add(pn, 1) )
		if( GAMESTATE->IsPlayerEnabled(pn) )
			return pn;
	return PLAYER_INVALID;
}

PlayerNumber GetNextCpuPlayer( PlayerNumber pn )
{
	for( enum_add(pn, 1); pn < NUM_PLAYERS; enum_add(pn, 1) )
		if( GAMESTATE->IsCpuPlayer(pn) )
			return pn;
	return PLAYER_INVALID;
}

PlayerNumber GetNextPotentialCpuPlayer( PlayerNumber pn )
{
	for( enum_add(pn, 1); pn < NUM_PLAYERS; enum_add(pn, 1) )
		if( !GAMESTATE->IsHumanPlayer(pn) )
			return pn;
	return PLAYER_INVALID;
}

MultiPlayer GetNextEnabledMultiPlayer( MultiPlayer mp )
{
	for( enum_add(mp, 1); mp < NUM_MultiPlayer; enum_add(mp, 1) )
		if( GAMESTATE->IsMultiPlayerEnabled(mp) )
			return mp;
	return MultiPlayer_Invalid;
}

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
