#include "stdafx.h"
/*
-----------------------------------------------------------------------------
 File: StyleDef.cpp

 Desc: A data structure that holds the definition of a GameMode.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
-----------------------------------------------------------------------------
*/

#include "StyleDef.h"
#include "RageLog.h"
#include "RageUtil.h"

#include "GameDef.h"
#include "IniFile.h"


void StyleDef::GetTransformedNoteDataForStyle( PlayerNumber p, NoteData* pOriginal, NoteData &newNoteData )
{
	int iNewToOriginalTrack[MAX_COLS_PER_PLAYER];
	for( int col=0; col<m_iColsPerPlayer; col++ )
	{
		ColumnInfo colInfo = m_ColumnInfo[p][col];
		int originalTrack = colInfo.track;
		
		iNewToOriginalTrack[col] = originalTrack;
	}
	
	newNoteData.LoadTransformed( pOriginal, m_iColsPerPlayer, iNewToOriginalTrack );
}