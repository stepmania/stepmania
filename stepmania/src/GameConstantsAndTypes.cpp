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
#define COLOR_EASY		THEME->GetMetricC("Common","ColorEasy")
#define COLOR_MEDIUM	THEME->GetMetricC("Common","ColorMedium")
#define COLOR_HARD		THEME->GetMetricC("Common","ColorHard")


D3DXCOLOR DifficultyClassToColor( DifficultyClass dc )
{
	switch( dc )
	{
	case CLASS_EASY:	return COLOR_EASY;
	case CLASS_MEDIUM:	return COLOR_MEDIUM;
	case CLASS_HARD:	return COLOR_HARD;
	default:	ASSERT(0);	return D3DXCOLOR();	// invalid DifficultyClass
	}
}

CString DifficultyClassToString( DifficultyClass dc )
{
	switch( dc )
	{
	case CLASS_EASY:	return "easy";
	case CLASS_MEDIUM:	return "medium";
	case CLASS_HARD:	return "hard";
	default:	ASSERT(0);	return "";	// invalid DifficultyClass
	}
}

DifficultyClass StringToDifficultyClass( CString sDC )
{
	for( int i=0; i<NUM_DIFFICULTY_CLASSES; i++ )
	{
		DifficultyClass dc = (DifficultyClass)i;
		if( sDC == DifficultyClassToString(dc) )
			return dc;
	}
	return CLASS_INVALID;
}

D3DXCOLOR PlayerToColor( PlayerNumber p ) 
{
	switch( p )
	{
		case PLAYER_1:	return COLOR_P1;
		case PLAYER_2:	return COLOR_P2;
		default: ASSERT(0); return D3DXCOLOR(0.5f,0.5f,0.5f,1);
	}
};

D3DXCOLOR PlayerToColor( int p ) 
{ 
	return PlayerToColor( (PlayerNumber)p ); 
}



/*  This was a dumb idea.  I'm change Style so that it knows what game it belongs to.

Game StyleToGame( Style s )
{
	switch( s )
	{
	case STYLE_DANCE_SINGLE:
	case STYLE_DANCE_VERSUS:
	case STYLE_DANCE_DOUBLE:
	case STYLE_DANCE_COUPLE:
	case STYLE_DANCE_SOLO:
		return GAME_DANCE;
	case STYLE_PUMP_SINGLE:
	case STYLE_PUMP_VERSUS:
	case STYLE_PUMP_DOUBLE:
		return GAME_PUMP;
	case STYLE_EZ2_SINGLE:
	case STYLE_EZ2_SINGLE_HARD:
	case STYLE_EZ2_DOUBLE:
	case STYLE_EZ2_REAL:
	case STYLE_EZ2_SINGLE_VERSUS:
	case STYLE_EZ2_SINGLE_HARD_VERSUS:
	case STYLE_EZ2_REAL_VERSUS:
		return GAME_EZ2;
	default:
		ASSERT(0);	// invalid Style
		return GAME_DANCE;
	}
}
*/