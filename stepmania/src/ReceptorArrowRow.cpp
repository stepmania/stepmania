#include "global.h"
/*
-----------------------------------------------------------------------------
 Class: ReceptorArrowRow

 Desc: See header.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "ReceptorArrowRow.h"
#include "RageUtil.h"
#include "GameConstantsAndTypes.h"
#include "PrefsManager.h"
#include "ArrowEffects.h"
#include "GameState.h"
#include "PrefsManager.h"
#include "NoteFieldPositioning.h"


ReceptorArrowRow::ReceptorArrowRow()
{
	m_iNumCols = 0;
}

void ReceptorArrowRow::Load( PlayerNumber pn, CString NoteSkin, float fYReverseOffset )
{
	m_PlayerNumber = pn;
	m_fYReverseOffsetPixels = fYReverseOffset;

	const StyleDef* pStyleDef = GAMESTATE->GetCurrentStyleDef();

	m_iNumCols = pStyleDef->m_iColsPerPlayer;

	for( int c=0; c<m_iNumCols; c++ ) 
		m_ReceptorArrow[c].Load( NoteSkin, m_PlayerNumber, c );
}

void ReceptorArrowRow::Update( float fDeltaTime )
{
	for( int c=0; c<m_iNumCols; c++ )
	{
		m_ReceptorArrow[c].Update( fDeltaTime );
		m_ReceptorArrow[c].SetDiffuse( RageColor(1,1,1,1 - GAMESTATE->m_CurrentPlayerOptions[m_PlayerNumber].m_fDark) );

		// set arrow XYZ
		const float fX = ArrowGetXPos( m_PlayerNumber, c, 0 );
		const float fY = ArrowGetYPos( m_PlayerNumber, c, 0, m_fYReverseOffsetPixels );
		const float fZ = ArrowGetZPos( m_PlayerNumber, c, 0 );
		m_ReceptorArrow[c].SetX( fX );
		m_ReceptorArrow[c].SetY( fY );
		m_ReceptorArrow[c].SetZ( fZ );
	}
}

void ReceptorArrowRow::DrawPrimitives()
{
	for( int c=0; c<m_iNumCols; c++ ) 
	{
		g_NoteFieldMode[m_PlayerNumber].BeginDrawTrack(c);
		m_ReceptorArrow[c].Draw();
		g_NoteFieldMode[m_PlayerNumber].EndDrawTrack(c);
	}
}

void ReceptorArrowRow::Step( int iCol )
{
	ASSERT( iCol >= 0  &&  iCol < m_iNumCols );
	m_ReceptorArrow[iCol].Step();
}

void ReceptorArrowRow::SetPressed( int iCol )
{
	ASSERT( iCol >= 0  &&  iCol < m_iNumCols );
	m_ReceptorArrow[iCol].SetPressed();
}

void ReceptorArrowRow::CopyTweening( const ReceptorArrowRow &from )
{
	for( int c=0; c<m_iNumCols; c++ ) 
		m_ReceptorArrow[c].CopyTweening( from.m_ReceptorArrow[c] );
	ActorFrame::CopyTweening( from );
}
