#ifndef GHOSTARROWROW_H
#define GHOSTARROWROW_H
/*
-----------------------------------------------------------------------------
 File: GhostArrowRow.h

 Desc: A row of GhostArrow Actors

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "ActorFrame.h"
#include "GhostArrow.h"
#include "GhostArrowBright.h"
#include "HoldGhostArrow.h"
#include "GameConstantsAndTypes.h"
#include "StyleDef.h"



class GhostArrowRow : public ActorFrame
{
public:
	GhostArrowRow();
	virtual void Update( float fDeltaTime );
	virtual void DrawPrimitives();

	void Load( PlayerNumber pn );
	
	void TapNote( int iCol, TapNoteScore score, bool bBright );
	void HoldNote( int iCol );
	
protected:
	int m_iNumCols;
	PlayerNumber m_PlayerNumber;

	GhostArrow			m_GhostArrowRow[MAX_NOTE_TRACKS];
	GhostArrowBright	m_GhostArrowRowBright[MAX_NOTE_TRACKS];
	HoldGhostArrow		m_HoldGhostArrowRow[MAX_NOTE_TRACKS];
};


#endif
