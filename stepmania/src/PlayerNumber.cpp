#include "global.h"
/*
-----------------------------------------------------------------------------
 File: PlayerNumber.cpp

 Desc: See header.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "PlayerNumber.h"
#include "ThemeManager.h"


#define COLOR_P1		THEME->GetMetricC("Common","ColorP1")
#define COLOR_P2		THEME->GetMetricC("Common","ColorP2")


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

