#include "global.h"
/*
-----------------------------------------------------------------------------
 Class: GhostArrowRow

 Desc: A graphic displayed in the GhostArrowRow during Dancing.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "GhostArrowRow.h"
#include "RageUtil.h"
#include "GameConstantsAndTypes.h"
#include "PrefsManager.h"
#include "ArrowEffects.h"
#include "NoteSkinManager.h"
#include "GameState.h"
#include "PrefsManager.h"
#include "NoteFieldPositioning.h"


GhostArrowRow::GhostArrowRow()
{
	m_iNumCols = 0;
}

void GhostArrowRow::Load( PlayerNumber pn )
{
	m_PlayerNumber = pn;

	const StyleDef* pStyleDef = GAMESTATE->GetCurrentStyleDef();

	m_iNumCols = pStyleDef->m_iColsPerPlayer;

	// init arrows
	for( int c=0; c<m_iNumCols; c++ ) 
	{
		CString Button = g_NoteFieldMode[m_PlayerNumber].GhostButtonNames[c];
		if(Button == "")
			Button = NoteSkinManager::ColToButtonName(c);

		m_GhostArrowRow[c].Load( NOTESKIN->GetPathTo(pn, Button, "tap explosion dim") );
		m_GhostArrowRowBright[c].Load( NOTESKIN->GetPathTo(pn, Button, "tap explosion bright") );
		m_HoldGhostArrowRow[c].Load( NOTESKIN->GetPathTo(pn, Button, "hold explosion") );

		m_GhostArrowRow[c].SetX( pStyleDef->m_ColumnInfo[pn][c].fXOffset );
		m_GhostArrowRowBright[c].SetX( pStyleDef->m_ColumnInfo[pn][c].fXOffset );
		m_HoldGhostArrowRow[c].SetX( pStyleDef->m_ColumnInfo[pn][c].fXOffset );
	}
}


void GhostArrowRow::Update( float fDeltaTime )
{
	for( int c=0; c<m_iNumCols; c++ )
	{
		m_GhostArrowRow[c].Update( fDeltaTime );
		m_GhostArrowRowBright[c].Update( fDeltaTime );
		m_HoldGhostArrowRow[c].Update( fDeltaTime );
	}
}

void GhostArrowRow::DrawPrimitives()
{
	for( int c=0; c<m_iNumCols; c++ )
	{
		g_NoteFieldMode[m_PlayerNumber].BeginDrawTrack(c);

		float fX = ArrowGetXPos( m_PlayerNumber, c, 0 );
		m_GhostArrowRow[c].SetX( fX );
		m_GhostArrowRowBright[c].SetX( fX );
		m_HoldGhostArrowRow[c].SetX( fX );
		float fZ = ArrowGetZPos( m_PlayerNumber, c, 0 );
		m_GhostArrowRow[c].SetZ( fZ );
		m_GhostArrowRowBright[c].SetZ( fZ );
		m_HoldGhostArrowRow[c].SetZ( fZ );

		m_GhostArrowRow[c].Draw();
		m_GhostArrowRowBright[c].Draw();
		m_HoldGhostArrowRow[c].Draw();

		g_NoteFieldMode[m_PlayerNumber].EndDrawTrack(c);
	}
}


void GhostArrowRow::TapNote( int iCol, TapNoteScore score, bool bBright )
{
	ASSERT( iCol >= 0  &&  iCol < m_iNumCols );
	if( bBright )
		m_GhostArrowRowBright[iCol].Step( score );
	else
		m_GhostArrowRow[iCol].Step( score );
}

void GhostArrowRow::HoldNote( int iCol )
{
	ASSERT( iCol >= 0  &&  iCol < m_iNumCols );
	m_HoldGhostArrowRow[iCol].Step();
}
