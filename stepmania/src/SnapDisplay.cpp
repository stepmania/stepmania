#include "global.h"
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
#include "GameManager.h"
#include "GameState.h"
#include "ThemeManager.h"


SnapDisplay::SnapDisplay()
{
	int i;
	for( i=0; i<2; i++ )
	{
		m_sprIndicators[i].Load( THEME->GetPathToG("SnapDisplay icon 8x1") );
		// MD 11/12/03 - this assert doesn't allow us to set the editor's maximum
		// resolution to less than the maximum one we have available.  Meh.
		// ASSERT( m_sprIndicators[i].GetNumStates() == NUM_NOTE_TYPES );
		m_sprIndicators[i].StopAnimating();
		this->AddChild( &m_sprIndicators[i] );
	}

	m_NoteType = NOTE_TYPE_4TH;
	
	m_iNumCols = 0;
}

void SnapDisplay::Load( PlayerNumber pn )
{
	m_iNumCols = GAMESTATE->GetCurrentStyleDef()->m_iColsPerPlayer;

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
	if( m_NoteType == NOTE_TYPE_64TH )	// this is the smallest snap we should allow
		return false;
	m_NoteType = NoteType(m_NoteType+1);

	SnapModeChanged();
	return true;
}


void SnapDisplay::SnapModeChanged()
{
	for( int i=0; i<2; i++ )
		m_sprIndicators[i].SetState( m_NoteType );
}
