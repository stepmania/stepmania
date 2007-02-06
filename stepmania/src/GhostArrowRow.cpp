#include "global.h"
#include "GhostArrowRow.h"
#include "RageUtil.h"
#include "GameConstantsAndTypes.h"
#include "ArrowEffects.h"
#include "NoteSkinManager.h"
#include "GameState.h"
#include "Game.h"
#include "PlayerState.h"
#include "Style.h"
#include "ActorUtil.h"


GhostArrowRow::GhostArrowRow()
{
}

void GhostArrowRow::Load( const PlayerState* pPlayerState, float fYReverseOffset )
{
	m_pPlayerState = pPlayerState;
	m_fYReverseOffsetPixels = fYReverseOffset;

	const Style* pStyle = GAMESTATE->GetCurrentStyle();

	// init arrows
	for( int c=0; c<pStyle->m_iColsPerPlayer; c++ ) 
	{
		const RString &sButton = GAMESTATE->GetCurrentStyle()->ColToButtonName( c );

		m_bHoldIsActive.push_back( false );
		m_bHoldWasActive.push_back( false );

		m_Ghost.push_back( NOTESKIN->LoadActor(sButton, "Explosion", this) );
		m_Ghost[c]->SetName( "GhostArrow" );
	}
}

GhostArrowRow::~GhostArrowRow()
{
	for( unsigned i = 0; i < m_Ghost.size(); ++i )
		delete m_Ghost[i];
}


void GhostArrowRow::Update( float fDeltaTime )
{
	for( unsigned c=0; c<m_Ghost.size(); c++ )
	{
		m_Ghost[c]->Update( fDeltaTime );

		const float fX = ArrowEffects::GetXPos( m_pPlayerState, c, 0 );
		const float fY = ArrowEffects::GetYPos( m_pPlayerState, c, 0, m_fYReverseOffsetPixels );
		const float fZ = ArrowEffects::GetZPos( m_pPlayerState, c, 0 );

		m_Ghost[c]->SetX( fX );
		m_Ghost[c]->SetY( fY );
		m_Ghost[c]->SetZ( fZ );

		const float fZoom = ArrowEffects::GetZoom( m_pPlayerState );
		m_Ghost[c]->SetZoom( fZoom );
	}

	for( unsigned i = 0; i < m_bHoldIsActive.size(); ++i )
	{
		if( !m_bHoldWasActive[i] && m_bHoldIsActive[i] )
			m_Ghost[i]->PlayCommand( "HoldingOn" );
		else if( m_bHoldWasActive[i] && !m_bHoldIsActive[i] )
			m_Ghost[i]->PlayCommand( "HoldingOff" );
		m_bHoldWasActive[i] = m_bHoldIsActive[i];
		m_bHoldIsActive[i] = false;
	}
}

void GhostArrowRow::DrawPrimitives()
{
	for( unsigned c=0; c<m_Ghost.size(); c++ )
		m_Ghost[c]->Draw();
}


void GhostArrowRow::DidTapNote( int iCol, TapNoteScore tns, bool bBright )
{
	ASSERT( iCol >= 0  &&  iCol < (int) m_Ghost.size() );

	m_Ghost[iCol]->PlayCommand( "Judgment" );
	if( bBright )
		m_Ghost[iCol]->PlayCommand( "Bright" );
	else
		m_Ghost[iCol]->PlayCommand( "Dim" );
	RString sJudge = TapNoteScoreToString( tns );
	m_Ghost[iCol]->PlayCommand( Capitalize(sJudge) );
}

void GhostArrowRow::DidHoldNote( int iCol, HoldNoteScore hns, bool bBright )
{
	ASSERT( iCol >= 0  &&  iCol < (int) m_Ghost.size() );
	m_Ghost[iCol]->PlayCommand( "Judgment" );
	if( bBright )
		m_Ghost[iCol]->PlayCommand( "Bright" );
	else
		m_Ghost[iCol]->PlayCommand( "Dim" );
	RString sJudge = HoldNoteScoreToString( hns );
	m_Ghost[iCol]->PlayCommand( Capitalize(sJudge) );
}

void GhostArrowRow::SetHoldIsActive( int iCol )
{
	ASSERT( iCol >= 0  &&  iCol < (int) m_Ghost.size() );
	m_bHoldIsActive[iCol] = true;
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
