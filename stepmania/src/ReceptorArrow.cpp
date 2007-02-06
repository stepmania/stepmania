#include "global.h"
#include "ReceptorArrow.h"
#include "GameState.h"
#include "NoteSkinManager.h"
#include "RageLog.h"
#include "RageUtil.h"
#include "Style.h"
#include "PlayerState.h"

ReceptorArrow::ReceptorArrow()
{
	m_bIsPressed = false;
	m_bWasPressed = false;
	m_bWasReverse = false;
}

void ReceptorArrow::Load( const PlayerState* pPlayerState, int iColNo )
{
	m_pPlayerState = pPlayerState;
	m_iColNo = iColNo;

	RString sButton = GAMESTATE->GetCurrentStyle()->ColToButtonName( iColNo );
	m_pReceptor.Load( NOTESKIN->LoadActor(sButton, "Receptor") );
	this->AddChild( m_pReceptor );

	bool bReverse = m_pPlayerState->m_PlayerOptions.GetCurrent().GetReversePercentForColumn(m_iColNo) > 0.5f;
	m_pReceptor->PlayCommand( bReverse? "ReverseOn":"ReverseOff" );
	m_bWasReverse = bReverse;
}

void ReceptorArrow::Update( float fDeltaTime )
{
	ActorFrame::Update( fDeltaTime );

	bool bReverse = m_pPlayerState->m_PlayerOptions.GetCurrent().GetReversePercentForColumn(m_iColNo) > 0.5f;
	if( bReverse != m_bWasReverse )
	{
		m_pReceptor->PlayCommand( bReverse? "ReverseOn":"ReverseOff" );
		m_bWasReverse = bReverse;
	}
}

void ReceptorArrow::DrawPrimitives()
{
	if( m_bWasPressed  &&  !m_bIsPressed )
		m_pReceptor->PlayCommand( "Lift" );
	else if( !m_bWasPressed  &&  m_bIsPressed )
		m_pReceptor->PlayCommand( "Press" );

	m_bWasPressed = m_bIsPressed;
	m_bIsPressed = false;	// it may get turned back on next update

	ActorFrame::DrawPrimitives();
}

void ReceptorArrow::Step( TapNoteScore score )
{
	m_bIsPressed = true;

	RString sJudge = TapNoteScoreToString( score );
	m_pReceptor->PlayCommand( Capitalize(sJudge) );
}

/*
 * (c) 2001-2004 Ben Nordstrom, Chris Danford
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
