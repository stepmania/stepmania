#include "global.h"
/*
-----------------------------------------------------------------------------
 Class: NoteFieldPlus

 Desc: See header.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "NoteFieldPlus.h"
#include "GameState.h"
#include "NoteFieldPositioning.h"
#include "NoteSkinManager.h"
#include "ArrowEffects.h"


NoteFieldPlus::NoteFieldPlus()
{
}

void NoteFieldPlus::Load( const NoteData* pNoteData, PlayerNumber pn, int iFirstPixelToDraw, int iLastPixelToDraw, float fYReverseOffset )
{
	NoteField::Load( pNoteData, pn, iFirstPixelToDraw, iLastPixelToDraw, fYReverseOffset );
	
	m_GrayArrowRow.Load( pn, fYReverseOffset );
	m_GhostArrowRow.Load( pn, fYReverseOffset );
}

void NoteFieldPlus::Update( float fDelta )
{
	NoteField::Update( fDelta );

	m_GrayArrowRow.Update( fDelta );
	m_GhostArrowRow.Update( fDelta );
}

void NoteFieldPlus::DrawPrimitives()
{
	m_GrayArrowRow.Draw();

	NoteField::DrawPrimitives();

	m_GhostArrowRow.Draw();
}

void NoteFieldPlus::Step( int iCol )
{
	m_GrayArrowRow.Step( iCol );
}

void NoteFieldPlus::UpdateBars( int iCol )
{
	m_GrayArrowRow.UpdateBars( iCol );
}

void NoteFieldPlus::TapNote( int iCol, TapNoteScore score, bool bBright )
{
	m_GhostArrowRow.TapNote( iCol, score, bBright );
}

void NoteFieldPlus::HoldNote( int iCol )
{
	m_GhostArrowRow.HoldNote( iCol );
}

void NoteFieldPlus::TapMine( int iCol, TapNoteScore score )
{
	m_GhostArrowRow.TapMine( iCol, score );
}
