/*
-----------------------------------------------------------------------------
 File: StyleDef.h

 Desc: A data structure that holds the definition of a GameMode.

 Copyright (c) 2001 Chris Danford.  All rights reserved.
-----------------------------------------------------------------------------
*/

#ifndef _Style_H_
#define _Style_H_

#include "Pattern.h"


const int MAX_NUM_COLUMNS = 12;



struct StyleDef
{
	int m_iNumPlayers;
	int m_iNumColumns;	// will vary depending on the number panels (4,6,8,etc)
	TapNote m_ColumnToTapNote[MAX_NUM_COLUMNS];
	float m_ColumnToRotation[MAX_NUM_COLUMNS];

	inline int TapNoteToColumnNumber( TapNote tap_step )
	{
		for (int i=0; i<m_iNumColumns; i++)
		{
			if( m_ColumnToTapNote[i] == tap_step )
				return i;
		}

		return -1;	// the TapNote is not used in this StyleDef
	};

};


#endif