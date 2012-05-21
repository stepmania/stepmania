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

void GhostArrowRow::Load( const PlayerState* pPlayerState, float fYReverseOffset )
{
	m_pPlayerState = pPlayerState;
	m_fYReverseOffsetPixels = fYReverseOffset;

	const Style* pStyle = GAMESTATE->GetCurrentStyle();
	const PlayerNumber pn = m_pPlayerState->m_PlayerNumber;
	NOTESKIN->SetPlayerNumber( pn );

	// init arrows
	for( int c=0; c<pStyle->m_iColsPerPlayer; c++ ) 
	{
		const RString &sButton = GAMESTATE->GetCurrentStyle()->ColToButtonName( c );

		const GameInput GameI = GAMESTATE->GetCurrentStyle()->StyleInputToGameInput( c, pn );
		NOTESKIN->SetGameController( GameI.controller );

		m_bHoldShowing.push_back( TapNote::SubType_Invalid );
		m_bLastHoldShowing.push_back( TapNote::SubType_Invalid );

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

		float fX = ArrowEffects::GetXPos( m_pPlayerState, c, 0 );
		float fY = ArrowEffects::GetYPos( m_pPlayerState, c, 0, m_fYReverseOffsetPixels );
		float fZ = ArrowEffects::GetZPos( m_pPlayerState, c, 0 );

		m_Ghost[c]->SetX( fX );
		m_Ghost[c]->SetY( fY );
		m_Ghost[c]->SetZ( fZ );

		const float fRotation = ArrowEffects::ReceptorGetRotationZ( m_pPlayerState );
		m_Ghost[c]->SetRotationZ( fRotation );

		const float fZoom = ArrowEffects::GetZoom( m_pPlayerState );
		m_Ghost[c]->SetZoom( fZoom );
	}

	for( unsigned i = 0; i < m_bHoldShowing.size(); ++i )
	{
		if( m_bLastHoldShowing[i] != m_bHoldShowing[i] )
		{
			if( m_bLastHoldShowing[i] == TapNote::hold_head_hold )
				m_Ghost[i]->PlayCommand( "HoldingOff" );
			else if( m_bLastHoldShowing[i] == TapNote::hold_head_roll )
				m_Ghost[i]->PlayCommand( "RollOff" );
			/*
			else if( m_bLastHoldShowing[i] == TapNote::hold_head_mine )
				m_Ghost[i]->PlayCommand( "MinefieldOff" );
			*/

			if( m_bHoldShowing[i] == TapNote::hold_head_hold )
				m_Ghost[i]->PlayCommand( "HoldingOn" );
			else if( m_bHoldShowing[i] == TapNote::hold_head_roll )
				m_Ghost[i]->PlayCommand( "RollOn" );
			/*
			else if( m_bHoldShowing[i] == TapNote::hold_head_mine )
				m_Ghost[i]->PlayCommand( "MinefieldOn" );
			*/
			m_bLastHoldShowing[i] = m_bHoldShowing[i];
		}
		m_bHoldShowing[i] = TapNote::SubType_Invalid;
	}
}

void GhostArrowRow::DrawPrimitives()
{
	const Style* pStyle = GAMESTATE->GetCurrentStyle();
	for( unsigned i=0; i<m_Ghost.size(); i++ )
	{
		const int c = pStyle->m_iColumnDrawOrder[i];
		m_Ghost[c]->Draw();
	}
}

void GhostArrowRow::DidTapNote( int iCol, TapNoteScore tns, bool bBright )
{
	ASSERT_M( iCol >= 0  &&  iCol < (int) m_Ghost.size(), ssprintf("assert(iCol %i >= 0  && iCol %i < (int)m_Ghost.size() %i) failed",iCol,iCol,(int)m_Ghost.size()) );

	Message msg("ColumnJudgment");
	msg.SetParam( "TapNoteScore", tns );
	// This may be useful for popn styled judgment :) -DaisuMaster
	msg.SetParam( "Column", iCol );
	if( bBright )
		msg.SetParam( "Bright", true );
	m_Ghost[iCol]->HandleMessage( msg );

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
	Message msg("ColumnJudgment");
	msg.SetParam( "HoldNoteScore", hns );
	msg.SetParam( "Column", iCol );
	if( bBright )
		msg.SetParam( "Bright", true );
	m_Ghost[iCol]->HandleMessage( msg );

	m_Ghost[iCol]->PlayCommand( "Judgment" );
	if( bBright )
		m_Ghost[iCol]->PlayCommand( "Bright" );
	else
		m_Ghost[iCol]->PlayCommand( "Dim" );
	RString sJudge = HoldNoteScoreToString( hns );
	m_Ghost[iCol]->PlayCommand( Capitalize(sJudge) );
}

void GhostArrowRow::SetHoldShowing( int iCol, const TapNote &tn )
{
	ASSERT( iCol >= 0  &&  iCol < (int) m_Ghost.size() );
	m_bHoldShowing[iCol] = tn.subType;
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
