#include "stdafx.h"
/*
-----------------------------------------------------------------------------
 File: GhostArrowRow.h

 Desc: A graphic displayed in the GhostArrowRow during Dancing.

 Copyright (c) 2001-2002 by the persons listed below.  All rights reserved.
-----------------------------------------------------------------------------
*/

#include "GhostArrowRow.h"
#include "RageUtil.h"
#include "ScreenDimensions.h"
#include "ThemeManager.h"
#include "ColorNote.h"
#include "ArrowEffects.h"
#include "GameManager.h"



GhostArrowRow::GhostArrowRow()
{
	m_iNumCols = 0;
}

void GhostArrowRow::Load( PlayerOptions po )
{
	m_PlayerOptions = po;

	StyleDef* pStyleDef = GAME->GetCurrentStyleDef();
	GameDef* pGameDef = GAME->GetCurrentGameDef();

	m_iNumCols = pStyleDef->m_iColsPerPlayer;

	// init arrows
	for( int c=0; c<m_iNumCols; c++ ) 
	{
		m_GhostArrowRow[c].Load( GAME->GetPathToGraphic(PLAYER_1, c, GRAPHIC_TAP_EXPLOSION_DIM) );
		m_GhostArrowRowBright[c].Load( GAME->GetPathToGraphic(PLAYER_1, c, GRAPHIC_TAP_EXPLOSION_BRIGHT) );
		m_HoldGhostArrowRow[c].Load( GAME->GetPathToGraphic(PLAYER_1, c, GRAPHIC_HOLD_EXPLOSION) );
	}
}


void GhostArrowRow::Update( float fDeltaTime, float fSongBeat )
{
	m_fSongBeat = fSongBeat;

	for( int c=0; c<m_iNumCols; c++ )
	{
		m_GhostArrowRow[c].Update( fDeltaTime );
		m_GhostArrowRowBright[c].Update( fDeltaTime );
		m_HoldGhostArrowRow[c].Update( fDeltaTime );

		m_GhostArrowRow[c].SetBeat( fSongBeat );
		m_GhostArrowRowBright[c].SetBeat( fSongBeat );
		m_HoldGhostArrowRow[c].SetBeat( fSongBeat );
	}
}

void GhostArrowRow::RenderPrimitives()
{
	for( int c=0; c<m_iNumCols; c++ )
	{
		float fX = ArrowGetXPos( m_PlayerOptions, c, 0, m_fSongBeat );
		m_GhostArrowRow[c].SetX( fX );
		m_GhostArrowRowBright[c].SetX( fX );
		m_HoldGhostArrowRow[c].SetX( fX );

		m_GhostArrowRow[c].Draw();
		m_GhostArrowRowBright[c].Draw();
		m_HoldGhostArrowRow[c].Draw();
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
