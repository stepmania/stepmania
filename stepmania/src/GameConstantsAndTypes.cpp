#include "stdafx.h"
/*
-----------------------------------------------------------------------------
 File: GameConstantsAndTypes.cpp

 Desc: See header.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "GameConstantsAndTypes.h"
#include "ThemeManager.h"

#define COLOR_P1		THEME->GetMetricC("Common","ColorP1")
#define COLOR_P2		THEME->GetMetricC("Common","ColorP2")


CString DifficultyToString( Difficulty dc )
{
	switch( dc )
	{
	case DIFFICULTY_EASY:	return "easy";
	case DIFFICULTY_MEDIUM:	return "medium";
	case DIFFICULTY_HARD:	return "hard";
	default:	ASSERT(0);	return "";	// invalid Difficulty
	}
}

Difficulty StringToDifficulty( CString sDC )
{
	for( int i=0; i<NUM_DIFFICULTIES; i++ )
	{
		Difficulty dc = (Difficulty)i;
		if( sDC == DifficultyToString(dc) )
			return dc;
	}
	return CLASS_INVALID;
}

RageColor PlayerToColor( PlayerNumber pn ) 
{
	switch( pn )
	{
		case PLAYER_1:	return COLOR_P1;
		case PLAYER_2:	return COLOR_P2;
		default: ASSERT(0); return RageColor(0.5f,0.5f,0.5f,1);
	}
};

RageColor PlayerToColor( int p ) 
{ 
	return PlayerToColor( (PlayerNumber)p ); 
}
