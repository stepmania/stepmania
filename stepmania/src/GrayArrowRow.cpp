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
#include "ThemeManager.h"
#include "ColorNote.h"
#include "ArrowEffects.h"
#include "GameManager.h"


GrayArrowRow::GrayArrowRow()
{
	m_iNumCols = 0;
}

void GrayArrowRow::Load( PlayerNumber pn, StyleDef* pStyleDef, PlayerOptions po )
{
	m_PlayerOptions = po;

	GameDef* pGameDef = GAMEMAN->GetCurrentGameDef();

	m_iNumCols = pStyleDef->m_iColsPerPlayer;

	// init arrow rotations
	for( int c=0; c<m_iNumCols; c++ ) 
	{
		m_GrayArrow[c].Load( GAMEMAN->GetPathToGraphic(pn, c, GRAPHIC_RECEPTOR) );
		m_GrayArrow[c].SetX( pStyleDef->m_ColumnInfo[pn][c].fXOffset );
	}
	
}

void GrayArrowRow::Update( float fDeltaTime, float fSongBeat )
{
	m_fSongBeat = fSongBeat;

	for( int c=0; c<m_iNumCols; c++ )
	{
		m_GrayArrow[c].Update( fDeltaTime );
		m_GrayArrow[c].SetBeat( fSongBeat );
	}
}

void GrayArrowRow::DrawPrimitives()
{
	for( int c=0; c<m_iNumCols; c++ ) 
	{
		// set arrow X
		float fX = ArrowGetXPos( m_PlayerOptions, c, 0, m_fSongBeat );
		m_GrayArrow[c].SetX( fX );


		m_GrayArrow[c].Draw();
	}

}

void GrayArrowRow::Step( int iCol )
{
	ASSERT( iCol >= 0  &&  iCol < m_iNumCols );
	m_GrayArrow[iCol].Step();
}
