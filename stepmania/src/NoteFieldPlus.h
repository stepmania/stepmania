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

	virtual void Load( const NoteData* pNoteData, PlayerNumber pn, int iFirstPixelToDraw, int iLastPixelToDraw, float fYReverseOffset );

	virtual void Update( float fDelta );
	virtual void DrawPrimitives();

	void Step( int iCol );
	void DidTapNote( int iCol, TapNoteScore score, bool bBright );
	void DidTapMine( int iCol, TapNoteScore score );
	void DidHoldNote( int iCol );
	void UpdateBars( int iCol );
protected:
	GrayArrowRow	m_GrayArrowRow;
	GhostArrowRow	m_GhostArrowRow;
};

#endif
