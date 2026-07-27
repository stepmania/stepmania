#include "global.h"
#include "ReceptorArrowRow.h"
#include "RageUtil.h"
#include "GameConstantsAndTypes.h"
#include "ArrowEffects.h"
#include "GameState.h"
#include "PlayerState.h"
#include "Style.h"

#include <cstddef>
#include <vector>


ReceptorArrowRow::ReceptorArrowRow()
{
	m_pPlayerState = nullptr;
	m_fYReverseOffsetPixels = 0;
	m_fFadeToFailPercent = 0;
	m_renderers= nullptr;
}

void ReceptorArrowRow::Load( const PlayerState* pPlayerState, float fYReverseOffset )
{
	m_pPlayerState = pPlayerState;
	m_fYReverseOffsetPixels = fYReverseOffset;

	const Style* pStyle = GAMESTATE->GetCurrentStyle(pPlayerState->m_PlayerNumber);

	for( int c=0; c<pStyle->m_iColsPerPlayer; c++ )
	{
		m_ReceptorArrow.push_back( new ReceptorArrow );
		m_ReceptorArrow[c]->SetName( "ReceptorArrow" );
		m_ReceptorArrow[c]->Load( m_pPlayerState, c );
		this->AddChild( m_ReceptorArrow[c] );
	}
}

void ReceptorArrowRow::SetColumnRenderers(std::vector<NoteColumnRenderer>& renderers)
{
	ASSERT_M(renderers.size() == m_ReceptorArrow.size(), "Notefield has different number of columns than receptor row.");
	for(std::size_t c= 0; c < m_ReceptorArrow.size(); ++c)
	{
		m_ReceptorArrow[c]->SetFakeParent(&(renderers[c]));
	}
	m_renderers= &renderers;
}

ReceptorArrowRow::~ReceptorArrowRow()
{
	for( unsigned i = 0; i < m_ReceptorArrow.size(); ++i )
		delete m_ReceptorArrow[i];
}

void ReceptorArrowRow::Update( float fDeltaTime )
{
	ActorFrame::Update( fDeltaTime );
	// If we're on gameplay, then the notefield will take care of updating
	// ArrowEffects.  But if we're on ScreenNameEntry, there is no notefield,
	// Checking whether m_renderers is null is a proxy for checking whether
	// there is a notefield. -Kyz
	if(m_renderers == nullptr)
	{
		ArrowEffects::Update();
	}

	for( unsigned c=0; c<m_ReceptorArrow.size(); c++ )
	{
		// m_fDark==1 or m_fFadeToFailPercent==1 should make fBaseAlpha==0
		float fBaseAlpha = (1 - m_pPlayerState->m_PlayerOptions.GetCurrent().m_fDark
			- m_pPlayerState->m_PlayerOptions.GetCurrent().m_fDarks[c]
		);
		if( m_fFadeToFailPercent != -1 )
		{
			fBaseAlpha *= (1 - m_fFadeToFailPercent);
		}
		CLAMP( fBaseAlpha, 0.0f, 1.0f );
		m_ReceptorArrow[c]->SetBaseAlpha( fBaseAlpha );

		if(m_renderers != nullptr)
		{
			// set arrow XYZ
			(*m_renderers)[c].UpdateReceptorGhostStuff(m_ReceptorArrow[c]);
		}
		else
		{
			// ScreenNameEntry uses ReceptorArrowRow but doesn't have or need
			// column renderers.  Just do the lazy thing and offset x. -Kyz
			const Style* style= GAMESTATE->GetCurrentStyle(m_pPlayerState->m_PlayerNumber);
			m_ReceptorArrow[c]->SetX(style->m_ColumnInfo[m_pPlayerState->m_PlayerNumber][c].fXOffset);
		}
	}
}

void ReceptorArrowRow::DrawPrimitives()
{
	const Style* pStyle = GAMESTATE->GetCurrentStyle(m_pPlayerState->m_PlayerNumber);
	for( unsigned i=0; i<m_ReceptorArrow.size(); i++ )
	{
		const int c = pStyle->m_iColumnDrawOrder[i];
		m_ReceptorArrow[c]->Draw();
	}
}

void ReceptorArrowRow::Step( int iCol, TapNoteScore score )
{
	ASSERT( iCol >= 0  &&  iCol < (int) m_ReceptorArrow.size() );
	m_ReceptorArrow[iCol]->Step( score );
}

void ReceptorArrowRow::SetPressed( int iCol )
{
	ASSERT( iCol >= 0  &&  iCol < (int) m_ReceptorArrow.size() );
	m_ReceptorArrow[iCol]->SetPressed();
}

void ReceptorArrowRow::SetNoteUpcoming( int iCol, bool b )
{
	ASSERT( iCol >= 0  &&  iCol < (int) m_ReceptorArrow.size() );
	m_ReceptorArrow[iCol]->SetNoteUpcoming(b);
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
