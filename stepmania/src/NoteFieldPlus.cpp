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


NoteFieldPlus::NoteFieldPlus()
{
}

void NoteFieldPlus::Load( NoteData* pNoteData, PlayerNumber pn, int iFirstPixelToDraw, int iLastPixelToDraw, float fYReverseOffset )
{
	NoteField::Load( pNoteData, pn, iFirstPixelToDraw, iLastPixelToDraw, fYReverseOffset );
	
	const StyleDef* pStyleDef = GAMESTATE->GetCurrentStyleDef();
	m_iNumCols = pStyleDef->m_iColsPerPlayer;

	
	int c;


	//
	// GrayArrowRow
	//
	for( c=0; c<m_iNumCols; c++ ) 
	{
		CString Button = g_NoteFieldMode[m_PlayerNumber].GrayButtonNames[c];
		if(Button == "")
			Button = NoteSkinManager::ColToButtonName(c);

		CString sPath = NOTESKIN->GetPathTo(pn, Button, "receptor");
		m_GrayArrow[c].Load( sPath );
		// XXX
		if( m_GrayArrow[c].GetNumStates() != 2 &&
			m_GrayArrow[c].GetNumStates() != 3 )
			RageException::Throw( "'%s' must have 2 or 3 frames", sPath.c_str() );
	}	


	//
	// GhostArrowRow
	//
	for( c=0; c<m_iNumCols; c++ ) 
	{
		CString Button = g_NoteFieldMode[m_PlayerNumber].GhostButtonNames[c];
		if(Button == "")
			Button = NoteSkinManager::ColToButtonName(c);

		m_GhostDim[c].SetName( "GhostArrowDim" );
		m_GhostBright[c].SetName( "GhostArrowBright" );
		m_GhostMine[c].SetName( "GhostArrowMine" );
		m_HoldGhost[c].SetName( "HoldGhostArrow" );
		
		m_GhostDim[c].Init( pn );
		m_GhostBright[c].Init( pn );
		m_GhostMine[c].Init( pn );
		//m_HoldGhost[c].Init( pn );

		m_GhostDim[c].Load( NOTESKIN->GetPathTo(pn, Button, "tap explosion dim") );
		m_GhostBright[c].Load( NOTESKIN->GetPathTo(pn, Button, "tap explosion bright") );
		m_GhostMine[c].Load( NOTESKIN->GetPathTo(pn, Button, "tap explosion mine") );
		m_HoldGhost[c].Load( NOTESKIN->GetPathTo(pn, Button, "hold explosion") );

	}
}

void NoteFieldPlus::Update( float fDelta )
{
	NoteField::Update( fDelta );

	for( int c=0; c<m_iNumCols; c++ )
	{
		float fX = ArrowGetXPos( m_PlayerNumber, c, 0 );
		float fY = ArrowGetYPos( m_PlayerNumber, c, 0, m_fYReverseOffsetPixels );
		float fZ = ArrowGetZPos( m_PlayerNumber, c, 0 );

		m_GrayArrow[c].Update( fDelta );
		m_GrayArrow[c].SetDiffuse( RageColor(1,1,1,1 - GAMESTATE->m_CurrentPlayerOptions[m_PlayerNumber].m_fDark) );
		m_GhostDim[c].Update( fDelta );
		m_GhostBright[c].Update( fDelta );
		m_GhostMine[c].Update( fDelta );
		m_HoldGhost[c].Update( fDelta );

		m_GrayArrow[c].SetX( fX );
		m_GhostDim[c].SetX( fX );
		m_GhostBright[c].SetX( fX );
		m_GhostMine[c].SetX( fX );
		m_HoldGhost[c].SetX( fX );

		m_GrayArrow[c].SetY( fY );
		m_GhostDim[c].SetY( fY );
		m_GhostBright[c].SetY( fY );
		m_GhostMine[c].SetY( fY );
		m_HoldGhost[c].SetY( fY );

		m_GrayArrow[c].SetZ( fY );
		m_GhostDim[c].SetZ( fZ );
		m_GhostBright[c].SetZ( fZ );
		m_GhostMine[c].SetZ( fZ );
		m_HoldGhost[c].SetZ( fZ );
	}

}

void NoteFieldPlus::DrawPrimitives()
{
	int c;

	//
	// GrayArrowRow
	//
	for( c=0; c<m_iNumCols; c++ ) 
	{
		g_NoteFieldMode[m_PlayerNumber].BeginDrawTrack(c);

		m_GrayArrow[c].Draw();

		g_NoteFieldMode[m_PlayerNumber].EndDrawTrack(c);
	}


	NoteField::DrawPrimitives();


	//
	// GhostArrowRow
	//
	for( c=0; c<m_iNumCols; c++ )
	{
		g_NoteFieldMode[m_PlayerNumber].BeginDrawTrack(c);

		m_GhostDim[c].Draw();
		m_GhostBright[c].Draw();
		m_GhostMine[c].Draw();
		m_HoldGhost[c].Draw();

		g_NoteFieldMode[m_PlayerNumber].EndDrawTrack(c);
	}
}

void NoteFieldPlus::Step( int iCol )
{
	ASSERT( iCol >= 0  &&  iCol < m_iNumCols );
	m_GrayArrow[iCol].Step();
}

void NoteFieldPlus::TapNote( int iCol, TapNoteScore score, bool bBright )
{
	ASSERT( iCol >= 0  &&  iCol < m_iNumCols );
	if( bBright )
		m_GhostBright[iCol].Step( score );
	else
		m_GhostDim[iCol].Step( score );
}

void NoteFieldPlus::HoldNote( int iCol )
{
	ASSERT( iCol >= 0  &&  iCol < m_iNumCols );
	m_HoldGhost[iCol].Step();
}

void NoteFieldPlus::TapMine( int iCol, TapNoteScore score )
{
	m_GhostMine[iCol].Step( score );
}
