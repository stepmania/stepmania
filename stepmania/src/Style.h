#pragma once
/*
-----------------------------------------------------------------------------
 File: Style

 Desc: .

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

enum Style
{
	STYLE_DANCE_SINGLE,
	STYLE_DANCE_VERSUS,
	STYLE_DANCE_DOUBLE,
	STYLE_DANCE_COUPLE,
	STYLE_DANCE_SOLO,
	STYLE_DANCE_EDIT_COUPLE_1,
	STYLE_DANCE_EDIT_COUPLE_2,
	STYLE_PUMP_SINGLE,
	STYLE_PUMP_VERSUS,
	STYLE_PUMP_DOUBLE,
	STYLE_PUMP_COUPLE,
	STYLE_PUMP_EDIT_COUPLE_1,
	STYLE_PUMP_EDIT_COUPLE_2,
	STYLE_EZ2_SINGLE,
	STYLE_EZ2_SINGLE_HARD,
	STYLE_EZ2_DOUBLE,
	STYLE_EZ2_REAL,
	STYLE_EZ2_SINGLE_VERSUS,
	STYLE_EZ2_SINGLE_HARD_VERSUS,
	STYLE_EZ2_REAL_VERSUS,
	STYLE_PARA_SINGLE,
	NUM_STYLES,	// leave this at the end
	STYLE_NONE,
};

const int NUM_DANCE_STYLES = 7;
const int NUM_PUMP_STYLES = 6;
const int NUM_EZ2_STYLES = 7;
const int NUM_PARA_STYLES = 1;

// Ugh.  This is needed for the style icon.
// TODO:  Find a more elegant way to handle this
inline int GetStyleIndexRelativeToGame( int iGameIndex, Style style )
{
	int iStyleIndex = style;
	switch( iGameIndex )
	{
	case 0:														break;	// dance
	case 1:	iStyleIndex -= NUM_DANCE_STYLES;					break;	// pump
	case 2:	iStyleIndex -= NUM_DANCE_STYLES+NUM_PUMP_STYLES;	break;	// ez2
	case 3:	iStyleIndex -= NUM_DANCE_STYLES+NUM_PUMP_STYLES+NUM_EZ2_STYLES;	break;	// ez2
	default:	ASSERT(0);	// invalid game index
	}
	return iStyleIndex;
}