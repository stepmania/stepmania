#include "stdafx.h"
/*
-----------------------------------------------------------------------------
 Class: OptionIconRow

 Desc: A graphic displayed in the OptionIconRow during Dancing.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "OptionIconRow.h"
#include "ThemeManager.h"
#include "PlayerOptions.h"
#include "GameState.h"
#include "RageLog.h"


#define SPACING_X	THEME->GetMetricF("OptionIconRow","SpacingX")
#define SPACING_Y	THEME->GetMetricF("OptionIconRow","SpacingY")


OptionIconRow::OptionIconRow()
{
	for( int i=0; i<NUM_OPTION_COLS; i++ )
	{
		m_OptionIcon[i].SetXY( i*SPACING_X, i*SPACING_Y );
		this->AddChild( &m_OptionIcon[i] );
	}
}


struct OptionColumnEntry
{
	char szString[30];
	int iSlotIndex;
};

const OptionColumnEntry g_OptionColumnEntries[] =
{
	{"Boost",	0},
	{"Wave",	1},
	{"Drunk",	1},
	{"Dizzy",	1},
	{"Space",	1},
	{"Mini",	1},
	{"Flip",	1},
	{"Tornado",	1},
	{"Hidden",	2},
	{"Sudden",	2},
	{"Stealth",	2},
	{"Blink",	2},
	{"Mirror",	3},
	{"Left",	3},
	{"Right",	3},
	{"Shuffle",	3},
	{"SuperShuffle",	3},
	{"Little",	4},
	{"NoHolds",	4},
	{"Dark",	4},
	{"Reverse",	5},
	{"Note",	6},
	{"Flat",	6},
	{"Plain",	6},
};
const int NUM_OPTION_COL_ENTRIES = sizeof(g_OptionColumnEntries)/sizeof(OptionColumnEntry);

int OptionToPreferredColumn( CString sOptionText )
{
	/* Speedups always go in column 0. digit ... x */
	if(sOptionText.GetLength() > 1 &&
		isdigit(sOptionText[0])    &&
		tolower(sOptionText[sOptionText.GetLength()-1]) == 'x') {
			return 0;
	}

	for( int i=0; i<NUM_OPTION_COL_ENTRIES; i++ )
		if( g_OptionColumnEntries[i].szString == sOptionText )
			return g_OptionColumnEntries[i].iSlotIndex;

	LOG->Warn("Unknown option: '%s'", sOptionText.GetString() );
	return -1;
}

void OptionIconRow::Refresh( PlayerNumber pn )
{
	// init
	unsigned i;
	for( i=0; i<NUM_OPTION_COLS; i++ )
		m_OptionIcon[i].Load( pn, "", i==0 );		

	CString sOptions = GAMESTATE->m_PlayerOptions[pn].GetString();
	CStringArray asOptions;
	split( sOptions, ", ", asOptions, true );


	CString asTabs[NUM_OPTION_COLS-1];	// fill these with what will be displayed on the tabs
	
	// for each option, look for the best column to place it in
	for( i=0; i<asOptions.size(); i++ )
	{
		CString sOption = asOptions[i];
		int iPerferredCol = OptionToPreferredColumn( sOption );

		if( iPerferredCol == -1 )
			continue;	// skip

		// search for a vacant spot
		for( int i=iPerferredCol; i<NUM_OPTION_COLS-1; i++ )
		{
			if( asTabs[i] != "" )
				continue;
			else
			{
				asTabs[i] = sOption;
				break;
			}
		}
	}

	for( i=0; i<NUM_OPTION_COLS-1; i++ )
		m_OptionIcon[i+1].Load( pn, asTabs[i], false );		
}

void OptionIconRow::DrawPrimitives()
{
	ActorFrame::DrawPrimitives();
}