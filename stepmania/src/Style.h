/*
-----------------------------------------------------------------------------
 File: Style.h

 Desc: A data structure that holds the definition of a GameMode.

 Copyright (c) 2001 Chris Danford.  All rights reserved.
-----------------------------------------------------------------------------
*/

#ifndef _Style_H_
#define _Style_H_

#include "Steps.h"


const int MAX_NUM_COLUMNS = 12;


struct Style
{
	int m_iNumPlayers;
	int m_iNumColumns;	// will vary depending on the number panels (4,6,8,etc)
	TapStep m_ColumnToTapStep[MAX_NUM_COLUMNS];
	float m_ColumnToRotation[MAX_NUM_COLUMNS];

	int TapStepToColumnNumber( TapStep tap_step )
	{
		//for (int i=0; i<m_iNumColumns; i++)
		for( int i=0; i<MAX_NUM_COLUMNS; i++ ) 
		// Switched to MAX_NUM_COLUMNS so up-left and up-right don't cause ASSERT(false)
		// Player.HandelPlayerStep will ignore up-left and up-right if it needs.
		{
			if( m_ColumnToTapStep[i] == tap_step )
				return i;
		}
		ASSERT( false );
		return -1;
	};

};


Style GetStyle( GameMode mode );

#endif