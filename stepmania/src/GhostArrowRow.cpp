#include "global.h"
#include "GhostArrowRow.h"
#include "RageUtil.h"
#include "GameConstantsAndTypes.h"
#include "ArrowEffects.h"
#include "NoteSkinManager.h"
#include "GameState.h"
#include "NoteFieldPositioning.h"
#include "Game.h"
#include "PlayerState.h"
#include "Style.h"


GhostArrowRow::GhostArrowRow()
{
	m_iNumCols = 0;
}

void GhostArrowRow::Load( const PlayerState* pPlayerState, float fYReverseOffset )
{
	Unload();

	m_pPlayerState = pPlayerState;
	m_fYReverseOffsetPixels = fYReverseOffset;

	const Style* pStyle = GAMESTATE->GetCurrentStyle();

	m_iNumCols = pStyle->m_iColsPerPlayer;

	// init arrows
	for( int c=0; c<m_iNumCols; c++ ) 
	{
		const CString &sButton = GAMESTATE->GetCurrentGame()->ColToButtonName( c );

		m_GhostDim.push_back( new GhostArrow );
		m_GhostBright.push_back( new GhostArrow );
		m_HoldGhost.push_back( new HoldGhostArrow );

		m_GhostDim[c]->SetName( "GhostArrowDim" );
		m_GhostBright[c]->SetName( "GhostArrowBright" );
		m_HoldGhost[c]->SetName( "HoldGhostArrow" );
		
		m_GhostDim[c]->Load( sButton, "tap explosion dim" );
		m_GhostBright[c]->Load( sButton, "tap explosion bright" );
		m_HoldGhost[c]->Load( sButton, "hold explosion" );
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

		const float fX = ArrowEffects::GetXPos( m_pPlayerState, c, 0 );
		const float fY = ArrowEffects::GetYPos( m_pPlayerState, c, 0, m_fYReverseOffsetPixels );
		const float fZ = ArrowEffects::GetZPos( m_pPlayerState, c, 0 );

		m_GhostDim[c]->SetX( fX );
		m_GhostBright[c]->SetX( fX );
		m_HoldGhost[c]->SetX( fX );

		m_GhostDim[c]->SetY( fY );
		m_GhostBright[c]->SetY( fY );
		m_HoldGhost[c]->SetY( fY );

		m_GhostDim[c]->SetZ( fZ );
		m_GhostBright[c]->SetZ( fZ );
		m_HoldGhost[c]->SetZ( fZ );

		const float fZoom = ArrowEffects::GetZoom( m_pPlayerState );
		m_GhostDim[c]->SetZoom( fZoom );
		m_GhostBright[c]->SetZoom( fZoom );
		m_HoldGhost[c]->SetZoom( fZoom );
	}
}

void GhostArrowRow::DrawPrimitives()
{
	for( int c=0; c<m_iNumCols; c++ )
	{
		// TODO: Remove indexing by PlayerNumber.
		PlayerNumber pn = m_pPlayerState->m_PlayerNumber;

		NoteFieldMode::BeginDrawTrack( pn, c );

		m_GhostDim[c]->Draw();
		m_GhostBright[c]->Draw();
		m_HoldGhost[c]->Draw();

		NoteFieldMode::EndDrawTrack( c );
	}
}


void GhostArrowRow::DidTapNote( int iCol, TapNoteScore tns, bool bBright )
{
	ASSERT( iCol >= 0  &&  iCol < m_iNumCols );
	if( bBright )
		m_GhostBright[iCol]->StepTap( tns );
	else
		m_GhostDim[iCol]->StepTap( tns );
}

void GhostArrowRow::DidHoldNote( int iCol, HoldNoteScore hns, bool bBright )
{
	ASSERT( iCol >= 0  &&  iCol < m_iNumCols );
	if( bBright )
		m_GhostBright[iCol]->StepHold( hns );
	else
		m_GhostDim[iCol]->StepHold( hns );
}

void GhostArrowRow::SetHoldIsActive( int iCol )
{
	ASSERT( iCol >= 0  &&  iCol < m_iNumCols );
	m_HoldGhost[iCol]->SetHoldIsActive( true );
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
