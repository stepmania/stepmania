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

enum StyleElement { 
	GRAPHIC_NOTE_COLOR_PART,
	GRAPHIC_NOTE_GRAY_PART,
	GRAPHIC_RECEPTOR,
	GRAPHIC_TAP_EXPLOSION_BRIGHT,
	GRAPHIC_TAP_EXPLOSION_DIM,
	GRAPHIC_HOLD_EXPLOSION,

	NUM_STYLE_ELEMENTS	// leave this at the end
};

struct StyleDef
{
public:
	int m_iNumPlayers;
	int m_iNumColumns;	// will vary depending on the number panels (4,6,8,etc)

	CString m_sColumnToNoteName[MAX_NUM_COLUMNS];	// e.g. "left", "right", "middle C", "snare"
	TapNote m_ColumnToTapNote[MAX_NUM_COLUMNS];		// which TapNote does this column correspond to?
	CString GetPathToGraphic( const int iColumnNumber, const StyleElement se );

	inline int TapNoteToColumnNumber( TapNote tap_step )
	{
		for (int i=0; i<m_iNumColumns; i++)
		{
			if( m_ColumnToTapNote[i] == tap_step )
				return i;
		}

		return -1;	// the TapNote is not used in this StyleDef
	};

protected:
	CString ElementToGraphicSuffix( const StyleElement se );

};


#endif