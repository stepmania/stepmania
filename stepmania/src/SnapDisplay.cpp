#include "stdafx.h"
/*
-----------------------------------------------------------------------------
 Class: SnapDisplay

 Desc: A graphic displayed in the SnapDisplay during edit.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "SnapDisplay.h"
#include "RageUtil.h"
#include "GameConstantsAndTypes.h"
#include "PrefsManager.h"
#include "ArrowEffects.h"
#include "GameManager.h"
#include "GameState.h"


SnapDisplay::SnapDisplay()
{
	for( int i=0; i<2; i++ )
	{
		m_sprIndicators[i].Load( THEME->GetPathTo("Graphics","edit snap indicator") );
		this->AddChild( &m_sprIndicators[i] );
	}

	m_NoteType = NOTE_TYPE_4TH;
	D3DXCOLOR color = NoteTypeToColor( m_NoteType );
	
	for( i=0; i<2; i++ )
		m_sprIndicators[i].SetDiffuse( color );

	m_iNumCols = 0;
}

void SnapDisplay::Load( PlayerNumber pn )
{
	m_iNumCols = GAMESTATE->GetCurrentStyleDef()->m_iColsPerPlayer;

	m_sprIndicators[0].StopTweening();
	m_sprIndicators[1].StopTweening();
	m_sprIndicators[0].SetX( -ARROW_SIZE * (m_iNumCols/2 + 0.5f) );
	m_sprIndicators[1].SetX(  ARROW_SIZE * (m_iNumCols/2 + 0.5f) );
}

bool SnapDisplay::PrevSnapMode()
{
	if( m_NoteType == 0 )
		return false;
	m_NoteType = NoteType(m_NoteType-1);

	SnapModeChanged();
	return true;
}

bool SnapDisplay::NextSnapMode()
{
	if( m_NoteType == NUM_NOTE_TYPES-1 )
		return false;
	m_NoteType = NoteType(m_NoteType+1);

	SnapModeChanged();
	return true;
}


void SnapDisplay::SnapModeChanged()
{
	D3DXCOLOR color = NoteTypeToColor( m_NoteType );
	
	for( int i=0; i<2; i++ )
	{
		m_sprIndicators[i].BeginTweening( 0.3f );
		m_sprIndicators[i].SetTweenDiffuse( color );
	}
}
