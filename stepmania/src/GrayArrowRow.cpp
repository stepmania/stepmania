#include "stdafx.h"
/*
-----------------------------------------------------------------------------
 Class: GrayArrowRow

 Desc: See header.

 Copyright (c) 2001-2002 by the persons listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "GrayArrowRow.h"
#include "RageUtil.h"
#include "ScreenDimensions.h"
#include "ThemeManager.h"
#include "ColorNote.h"
#include "ArrowEffects.h"
#include "GameManager.h"


GrayArrowRow::GrayArrowRow()
{
	m_iNumCols = 0;
}

void GrayArrowRow::Load( PlayerOptions po )
{
	m_PlayerOptions = po;

	StyleDef* pStyleDef = GAME->GetCurrentStyleDef();
	GameDef* pGameDef = GAME->GetCurrentGameDef();

	m_iNumCols = pStyleDef->m_iColsPerPlayer;

	// init arrow rotations
	for( int c=0; c<m_iNumCols; c++ ) 
	{
		m_GrayArrow[c].Load( GAME->GetPathToGraphic(PLAYER_1, c, GRAPHIC_RECEPTOR) );
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

void GrayArrowRow::RenderPrimitives()
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
