#include "global.h"
/*
-----------------------------------------------------------------------------
 Class: GrayArrowRow

 Desc: See header.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "GrayArrowRow.h"
#include "RageUtil.h"
#include "GameConstantsAndTypes.h"
#include "PrefsManager.h"
#include "ArrowEffects.h"
#include "GameState.h"
#include "PrefsManager.h"
#include "NoteFieldPositioning.h"


GrayArrowRow::GrayArrowRow()
{
	m_iNumCols = 0;
}

void GrayArrowRow::Load( PlayerNumber pn, CString NoteSkin, float fYReverseOffset )
{
	m_PlayerNumber = pn;
	m_fYReverseOffsetPixels = fYReverseOffset;

	const StyleDef* pStyleDef = GAMESTATE->GetCurrentStyleDef();

	m_iNumCols = pStyleDef->m_iColsPerPlayer;

	for( int c=0; c<m_iNumCols; c++ ) 
		m_GrayArrow[c].Load( NoteSkin, m_PlayerNumber, c );
}

void GrayArrowRow::Update( float fDeltaTime )
{
	for( int c=0; c<m_iNumCols; c++ )
	{
		m_GrayArrow[c].Update( fDeltaTime );
		m_GrayArrow[c].SetDiffuse( RageColor(1,1,1,1 - GAMESTATE->m_CurrentPlayerOptions[m_PlayerNumber].m_fDark) );

		// set arrow XYZ
		const float fX = ArrowGetXPos( m_PlayerNumber, c, 0 );
		const float fY = ArrowGetYPos( m_PlayerNumber, c, 0, m_fYReverseOffsetPixels );
		const float fZ = ArrowGetZPos( m_PlayerNumber, c, 0 );
		m_GrayArrow[c].SetX( fX );
		m_GrayArrow[c].SetY( fY );
		m_GrayArrow[c].SetZ( fZ );
	}
}

void GrayArrowRow::DrawPrimitives()
{
	for( int c=0; c<m_iNumCols; c++ ) 
	{
		g_NoteFieldMode[m_PlayerNumber].BeginDrawTrack(c);
		m_GrayArrow[c].Draw();
		g_NoteFieldMode[m_PlayerNumber].EndDrawTrack(c);
	}
}

void GrayArrowRow::Step( int iCol )
{
	ASSERT( iCol >= 0  &&  iCol < m_iNumCols );
	m_GrayArrow[iCol].Step();
}

void GrayArrowRow::UpdateBars( int iCol )
{
	ASSERT( iCol >= 0  &&  iCol < m_iNumCols );
	m_GrayArrow[iCol].UpdateBars();
}

