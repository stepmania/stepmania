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

void GhostArrowRow::Load( PlayerNumber pn, CString NoteSkin, float fYReverseOffset )
{
	m_PlayerNumber = pn;
	m_fYReverseOffsetPixels = fYReverseOffset;

	const StyleDef* pStyleDef = GAMESTATE->GetCurrentStyleDef();

	m_iNumCols = pStyleDef->m_iColsPerPlayer;

	// init arrows
	for( int c=0; c<m_iNumCols; c++ ) 
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

		m_GhostDim[c].Load( NoteSkin, Button, "tap explosion dim", false);
		m_GhostBright[c].Load( NoteSkin, Button, "tap explosion bright", false );
		m_GhostMine[c].Load( NoteSkin, Button, "tap explosion mine", true );
		m_HoldGhost[c].Load( NOTESKIN->GetPathToFromNoteSkinAndButton(NoteSkin, Button, "hold explosion") );
	}
}


void GhostArrowRow::Update( float fDeltaTime )
{
	for( int c=0; c<m_iNumCols; c++ )
	{
		m_GhostDim[c].Update( fDeltaTime );
		m_GhostBright[c].Update( fDeltaTime );
		m_GhostMine[c].Update( fDeltaTime );
		m_HoldGhost[c].Update( fDeltaTime );

		const float fX = ArrowGetXPos( m_PlayerNumber, c, 0 );
		const float fY = ArrowGetYPos( m_PlayerNumber, c, 0, m_fYReverseOffsetPixels );
		const float fZ = ArrowGetZPos( m_PlayerNumber, c, 0 );

		m_GhostDim[c].SetX( fX );
		m_GhostBright[c].SetX( fX );
		m_GhostMine[c].SetX( fX );
		m_HoldGhost[c].SetX( fX );

		m_GhostDim[c].SetY( fY );
		m_GhostBright[c].SetY( fY );
		m_GhostMine[c].SetY( fY );
		m_HoldGhost[c].SetY( fY );

		m_GhostDim[c].SetZ( fZ );
		m_GhostBright[c].SetZ( fZ );
		m_GhostMine[c].SetZ( fZ );
		m_HoldGhost[c].SetZ( fZ );
	}
}

void GhostArrowRow::DrawPrimitives()
{
	for( int c=0; c<m_iNumCols; c++ )
	{
		g_NoteFieldMode[m_PlayerNumber].BeginDrawTrack(c);

		m_GhostDim[c].Draw();
		m_GhostBright[c].Draw();
		m_GhostMine[c].Draw();
		m_HoldGhost[c].Draw();

		g_NoteFieldMode[m_PlayerNumber].EndDrawTrack(c);
	}
}


void GhostArrowRow::DidTapNote( int iCol, TapNoteScore score, bool bBright )
{
	ASSERT( iCol >= 0  &&  iCol < m_iNumCols );
	if( bBright )
		m_GhostBright[iCol].Step( score );
	else
		m_GhostDim[iCol].Step( score );
}

void GhostArrowRow::DidHoldNote( int iCol )
{
	ASSERT( iCol >= 0  &&  iCol < m_iNumCols );
	m_HoldGhost[iCol].Step();
}

void GhostArrowRow::DidTapMine( int iCol, TapNoteScore score )
{
	m_GhostMine[iCol].Step( score );
}


void GhostArrowRow::CopyTweening( const GhostArrowRow &from )
{
	for( int c=0; c<m_iNumCols; c++ ) 
	{
		m_GhostDim[c].CopyTweening( from.m_GhostDim[c] );
		m_GhostBright[c].CopyTweening( from.m_GhostBright[c] );
		m_GhostMine[c].CopyTweening( from.m_GhostMine[c] );
		m_HoldGhost[c].CopyTweening( from.m_HoldGhost[c] );
	}
	ActorFrame::CopyTweening( from );
}
