#include "global.h"
#include "MemoryCardDisplay.h"
#include "ThemeManager.h"
#include "MemoryCardManager.h"
#include "RageUtil.h"


MemoryCardDisplay::MemoryCardDisplay()
{
	m_PlayerNumber = PLAYER_INVALID;
	m_LastSeenState = MEMORY_CARD_STATE_INVALID;
}

void MemoryCardDisplay::Load( PlayerNumber pn )
{
	m_PlayerNumber = pn;

	for( int i=0; i<NUM_MEMORY_CARD_STATES; i++ )
	{
		MemoryCardState mcs = (MemoryCardState)i;
		CString sState = MemoryCardStateToString(mcs);
		m_spr[i].Load( THEME->GetPathToG(ssprintf("MemoryCardDisplay %s p%d",sState.c_str(),pn+1)) );
		m_spr[i].SetHidden( true );
		this->AddChild( &m_spr[i] );
	}
}

void MemoryCardDisplay::Update( float fDelta )
{
	MemoryCardState newMcs = MEMCARDMAN->GetCardState(m_PlayerNumber);
	if( m_LastSeenState != newMcs )
	{
		if( m_LastSeenState != MEMORY_CARD_STATE_INVALID )
			m_spr[m_LastSeenState].SetHidden( true );
		m_LastSeenState = newMcs;
		m_spr[m_LastSeenState].SetHidden( false );
	}

	ActorFrame::Update( fDelta );
}

/*
 * (c) 2003 Chris Danford
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
