#ifndef NOTEFIELDPLUS_H
#define NOTEFIELDPLUS_H
/*
-----------------------------------------------------------------------------
 Class: NoteFieldPlus

 Desc: NoteField + gray and ghost arrows.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/


#include "NoteField.h"
#include "GrayArrowRow.h"
#include "GhostArrowRow.h"


class NoteFieldPlus : public NoteField
{
public:
	NoteFieldPlus();

	virtual void Load( NoteData* pNoteData, PlayerNumber pn, int iFirstPixelToDraw, int iLastPixelToDraw, float fYReverseOffset );

	virtual void Update( float fDelta );
	virtual void DrawPrimitives();

	void Step( int iCol );
	void TapNote( int iCol, TapNoteScore score, bool bBright );
	void TapMine( int iCol, TapNoteScore score );
	void HoldNote( int iCol );

protected:
	int m_iNumCols;

	GrayArrow		m_GrayArrow[MAX_NOTE_TRACKS];

	GhostArrow		m_GhostDim[MAX_NOTE_TRACKS];
	GhostArrow		m_GhostBright[MAX_NOTE_TRACKS];
	GhostArrow		m_GhostMine[MAX_NOTE_TRACKS];
	HoldGhostArrow	m_HoldGhost[MAX_NOTE_TRACKS];

};

#endif
