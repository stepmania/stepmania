#include "stdafx.h"
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
#include "GameManager.h"
#include "GameState.h"
#include "PrefsManager.h"


GrayArrowRow::GrayArrowRow()
{
	m_iNumCols = 0;
}

void GrayArrowRow::Load( PlayerNumber pn )
{
	m_PlayerNumber = pn;

	const StyleDef* pStyleDef = GAMESTATE->GetCurrentStyleDef();

	m_iNumCols = pStyleDef->m_iColsPerPlayer;

	for( int c=0; c<m_iNumCols; c++ ) 
	{
		m_GrayArrow[c].Load( GAMEMAN->GetPathTo(c, GRAPHIC_RECEPTOR) );
		m_GrayArrow[c].SetX( pStyleDef->m_ColumnInfo[pn][c].fXOffset );
	}
	
}

void GrayArrowRow::Update( float fDeltaTime )
{
	for( int c=0; c<m_iNumCols; c++ )
	{
		m_GrayArrow[c].Update( fDeltaTime );
	}
}

void GrayArrowRow::DrawPrimitives()
{
	if( GAMESTATE->m_PlayerOptions[m_PlayerNumber].m_bDark )
		return;

	for( int c=0; c<m_iNumCols; c++ ) 
	{
		// set arrow X
		float fX = ArrowGetXPos( m_PlayerNumber, c, 0 );
		m_GrayArrow[c].SetX( fX );


		m_GrayArrow[c].Draw();
	}

}

void GrayArrowRow::Step( int iCol )
{
	ASSERT( iCol >= 0  &&  iCol < m_iNumCols );
	m_GrayArrow[iCol].Step();
}
