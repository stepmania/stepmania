#include "global.h"
#include "GhostArrowRow.h"
#include "RageUtil.h"
#include "GameConstantsAndTypes.h"
#include "PrefsManager.h"
#include "ArrowEffects.h"
#include "NoteSkinManager.h"
#include "GameState.h"
#include "PrefsManager.h"
#include "NoteFieldPositioning.h"
#include "Game.h"


GhostArrowRow::GhostArrowRow()
{
	m_iNumCols = 0;
}
#include "RageLog.h"
void GhostArrowRow::Load( PlayerNumber pn, CString NoteSkin, float fYReverseOffset )
{
	Unload();

	m_PlayerNumber = pn;
	m_fYReverseOffsetPixels = fYReverseOffset;

	const Style* pStyle = GAMESTATE->GetCurrentStyle();

	m_iNumCols = pStyle->m_iColsPerPlayer;

	// init arrows
	for( int c=0; c<m_iNumCols; c++ ) 
	{
		NoteFieldMode &mode = g_NoteFieldMode[pn];
		CString Button = mode.GhostButtonNames[c];
		if( Button == "" )
			Button = GAMESTATE->GetCurrentGame()->ColToButtonName( c );

		m_GhostDim.push_back( new GhostArrow );
		m_GhostBright.push_back( new GhostArrow );
		m_HoldGhost.push_back( new HoldGhostArrow );

		m_GhostDim[c]->SetName( "GhostArrowDim" );
		m_GhostBright[c]->SetName( "GhostArrowBright" );
		m_HoldGhost[c]->SetName( "HoldGhostArrow" );
		
		m_GhostDim[c]->Init( pn );
		m_GhostBright[c]->Init( pn );
		//m_HoldGhost[c]->Init( pn );

		m_GhostDim[c]->Load( NoteSkin, Button, "tap explosion dim" );
		m_GhostBright[c]->Load( NoteSkin, Button, "tap explosion bright" );
		m_HoldGhost[c]->Load( NoteSkin, Button, "hold explosion" );
	}
}

void GhostArrowRow::Unload()
{
	for( int i = 0; i < m_iNumCols; ++i )
	{
		delete m_GhostDim[i];
		delete m_GhostBright[i];
		delete m_HoldGhost[i];
	}

	m_GhostDim.clear();
	m_GhostBright.clear();
	m_HoldGhost.clear();

	m_iNumCols = 0;
}


void GhostArrowRow::Update( float fDeltaTime )
{
	for( int c=0; c<m_iNumCols; c++ )
	{
		m_GhostDim[c]->Update( fDeltaTime );
		m_GhostBright[c]->Update( fDeltaTime );
		m_HoldGhost[c]->Update( fDeltaTime );

		const float fX = ArrowGetXPos( m_PlayerNumber, c, 0 );
		const float fY = ArrowGetYPos( m_PlayerNumber, c, 0, m_fYReverseOffsetPixels );
		const float fZ = ArrowGetZPos( m_PlayerNumber, c, 0 );

		m_GhostDim[c]->SetX( fX );
		m_GhostBright[c]->SetX( fX );
		m_HoldGhost[c]->SetX( fX );

		m_GhostDim[c]->SetY( fY );
		m_GhostBright[c]->SetY( fY );
		m_HoldGhost[c]->SetY( fY );

		m_GhostDim[c]->SetZ( fZ );
		m_GhostBright[c]->SetZ( fZ );
		m_HoldGhost[c]->SetZ( fZ );

		const float fZoom = ArrowGetZoom( m_PlayerNumber );
		m_GhostDim[c]->SetZoom( fZoom );
		m_GhostBright[c]->SetZoom( fZoom );
		m_HoldGhost[c]->SetZoom( fZoom );
	}
}

void GhostArrowRow::DrawPrimitives()
{
	for( int c=0; c<m_iNumCols; c++ )
	{
		g_NoteFieldMode[m_PlayerNumber].BeginDrawTrack(c);

		m_GhostDim[c]->Draw();
		m_GhostBright[c]->Draw();
		m_HoldGhost[c]->Draw();

		g_NoteFieldMode[m_PlayerNumber].EndDrawTrack(c);
	}
}


void GhostArrowRow::DidTapNote( int iCol, TapNoteScore score, bool bBright )
{
	ASSERT( iCol >= 0  &&  iCol < m_iNumCols );
	if( bBright )
		m_GhostBright[iCol]->Step( score );
	else
		m_GhostDim[iCol]->Step( score );
}

void GhostArrowRow::SetHoldIsActive( int iCol )
{
	ASSERT( iCol >= 0  &&  iCol < m_iNumCols );
	m_HoldGhost[iCol]->SetHoldIsActive( true );
}

void GhostArrowRow::CopyTweening( const GhostArrowRow &from )
{
	for( int c=0; c<m_iNumCols; c++ ) 
	{
		m_GhostDim[c]->CopyTweening( *from.m_GhostDim[c] );
		m_GhostBright[c]->CopyTweening( *from.m_GhostBright[c] );
		m_HoldGhost[c]->CopyTweening( *from.m_HoldGhost[c] );
	}
	ActorFrame::CopyTweening( from );
}

/*
 * (c) 2001-2004 Chris Danford
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
