#include "global.h"
#include "SnapDisplay.h"
#include "GameState.h"
#include "ThemeManager.h"
#include "Style.h"
#include "ScreenDimensions.h"
#include "EnumHelper.h"

SnapDisplay::SnapDisplay()
{
	for( int i=0; i<2; i++ )
	{
		m_sprIndicators[i].Load( THEME->GetPathG("SnapDisplay","icon 9x1") );
		m_sprIndicators[i].StopAnimating();
		this->AddChild( &m_sprIndicators[i] );
	}

	m_NoteType = NOTE_TYPE_4TH;

	m_iNumCols = 0;
}

void SnapDisplay::Load()
{
	m_iNumCols = GAMESTATE->GetCurrentStyle(GAMESTATE->GetMasterPlayerNumber())->m_iColsPerPlayer;

	m_sprIndicators[0].SetX( -ARROW_SIZE * (m_iNumCols/2 + 0.5f) );
	m_sprIndicators[1].SetX(  ARROW_SIZE * (m_iNumCols/2 + 0.5f) );
}

bool SnapDisplay::PrevSnapMode()
{
	if( m_NoteType == 0 )
		return false;
	enum_add( m_NoteType, -1 );

	SnapModeChanged();
	return true;
}

bool SnapDisplay::NextSnapMode()
{
	if( m_NoteType == NOTE_TYPE_192ND )	// the smallest snap we should allow
		return false;
	enum_add( m_NoteType, 1 );

	SnapModeChanged();
	return true;
}

void SnapDisplay::SnapModeChanged()
{
	for( int i=0; i<2; i++ )
		m_sprIndicators[i].SetState( m_NoteType );
}

/*
 * (c) 2001-2002 Chris Danford
 * All rights reserved.
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, and/or sell copies of the Software, and to permit persons to
 * whom the Software is furnished to do so, provided that the above
 * copyright notice(s) and this permission notice appear in all copies of
 * the Software and that both the above copyright notice(s) and this
 * permission notice appear in supporting documentation.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT OF
 * THIRD PARTY RIGHTS. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR HOLDERS
 * INCLUDED IN THIS NOTICE BE LIABLE FOR ANY CLAIM, OR ANY SPECIAL INDIRECT
 * OR CONSEQUENTIAL DAMAGES, OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS
 * OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
 * OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */
