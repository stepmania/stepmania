#include "global.h"
#include "ReceptorArrowRow.h"
#include "RageUtil.h"
#include "GameConstantsAndTypes.h"
#include "PrefsManager.h"
#include "ArrowEffects.h"
#include "GameState.h"
#include "PrefsManager.h"
#include "NoteFieldPositioning.h"
#include "PlayerState.h"
#include "Style.h"


ReceptorArrowRow::ReceptorArrowRow()
{
	m_iNumCols = 0;
	m_pPlayerState = NULL;
	m_fYReverseOffsetPixels = 0;
	m_fFadeToFailPercent = 0;
}

void ReceptorArrowRow::Load( const PlayerState* pPlayerState, float fYReverseOffset )
{
	m_pPlayerState = pPlayerState;
	m_fYReverseOffsetPixels = fYReverseOffset;

	const Style* pStyle = GAMESTATE->GetCurrentStyle();

	m_iNumCols = pStyle->m_iColsPerPlayer;

	for( int c=0; c<m_iNumCols; c++ ) 
	{
		m_ReceptorArrow[c].SetName( "ReceptorArrow" );
		m_ReceptorArrow[c].Load( m_pPlayerState, c );
	}
}

void ReceptorArrowRow::Update( float fDeltaTime )
{
	for( int c=0; c<m_iNumCols; c++ )
	{
		m_ReceptorArrow[c].Update( fDeltaTime );
		
		// m_fDark==1 or m_fFadeToFailPercent==1 should make fBaseAlpha==0
		float fBaseAlpha = (1 - m_pPlayerState->m_CurrentPlayerOptions.m_fDark) * (1 - m_fFadeToFailPercent);
		m_ReceptorArrow[c].SetBaseAlpha( fBaseAlpha );

		// set arrow XYZ
		const float fX = ArrowEffects::GetXPos( m_pPlayerState, c, 0 );
		const float fY = ArrowEffects::GetYPos( m_pPlayerState, c, 0, m_fYReverseOffsetPixels );
		const float fZ = ArrowEffects::GetZPos( m_pPlayerState, c, 0 );
		m_ReceptorArrow[c].SetX( fX );
		m_ReceptorArrow[c].SetY( fY );
		m_ReceptorArrow[c].SetZ( fZ );

		const float fZoom = ArrowEffects::GetZoom( m_pPlayerState );
		m_ReceptorArrow[c].SetZoom( fZoom );
	}
}

void ReceptorArrowRow::DrawPrimitives()
{
	// TODO: Remove use of PlayerNumber.
	PlayerNumber pn = m_pPlayerState->m_PlayerNumber;

	for( int c=0; c<m_iNumCols; c++ ) 
	{
		NoteFieldMode::BeginDrawTrack( pn, c );
		m_ReceptorArrow[c].Draw();
		NoteFieldMode::EndDrawTrack(c);
	}
}

void ReceptorArrowRow::Step( int iCol, TapNoteScore score )
{
	ASSERT( iCol >= 0  &&  iCol < m_iNumCols );
	m_ReceptorArrow[iCol].Step( score );
}

void ReceptorArrowRow::SetPressed( int iCol )
{
	ASSERT( iCol >= 0  &&  iCol < m_iNumCols );
	m_ReceptorArrow[iCol].SetPressed();
}

void ReceptorArrowRow::CopyTweening( const ReceptorArrowRow &from )
{
	for( int c=0; c<m_iNumCols; c++ ) 
		m_ReceptorArrow[c].CopyTweening( from.m_ReceptorArrow[c] );
	ActorFrame::CopyTweening( from );
}

/*
 * (c) 2001-2003 Chris Danford
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
