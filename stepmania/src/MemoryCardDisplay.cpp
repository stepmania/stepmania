#include "global.h"
#include "MemoryCardDisplay.h"
#include "ThemeManager.h"
#include "MemoryCardManager.h"


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
   Copyright (c) 2003 by the person(s) listed below. All rights reserved.
     Chris Danford
*/
