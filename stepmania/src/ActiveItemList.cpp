#include "global.h"
/*
-----------------------------------------------------------------------------
 File: ActiveItemList.h

 Desc: A graphic displayed in the ActiveItemList during Dancing.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "ActiveItemList.h"
#include "RageUtil.h"
#include "GameState.h"
#include "ThemeManager.h"
#include "Inventory.h"
#include "RageTimer.h"


#define TEXT_HORIZ_ALIGN( p )	THEME->GetMetricI("ActiveItemList",ssprintf("TextHorizAlignP%d",p+1))
#define TEXT_ZOOM				THEME->GetMetricF("ActiveItemList","TextZoom")
#define SPACING_Y				THEME->GetMetricF("ActiveItemList","SpacingY")


ActiveItemList::ActiveItemList()
{
	for( int i=0; i<MAX_ACTIVE_ITEMS_LINES; i++ )
	{
		m_text[i].LoadFromFont( THEME->GetPathTo("Fonts","Common normal") );
		m_text[i].SetZoom( TEXT_ZOOM );
//		m_text[i].SetText( "WWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWW" );
		this->AddChild( &m_text[i] );
	}
}

void ActiveItemList::Init( PlayerNumber pn )
{
	m_PlayerNumber = pn;

	for( int i=0; i<MAX_ACTIVE_ITEMS_LINES; i++ )
	{
		m_text[i].SetHorizAlign( (Actor::HorizAlign)TEXT_HORIZ_ALIGN(pn) );
		bool bReverse = GAMESTATE->m_PlayerOptions[pn].m_fReverseScroll == 1;
		float fHeight = SPACING_Y*MAX_ACTIVE_ITEMS_LINES * (bReverse ? 1 : -1 );
		float fY = SCALE(i,0.f,MAX_ACTIVE_ITEMS_LINES,0.f,fHeight);
		m_text[i].SetY( fY );
	}

}


void ActiveItemList::Update( float fDelta ) 
{ 
	ActorFrame::Update( fDelta ); 


	// refresh text only once every second
	float fNowSeconds = RageTimer::GetTimeSinceStart();
	float fLastSeconds = RageTimer::GetTimeSinceStart() - fDelta;

	if( (int)fNowSeconds != (int)fLastSeconds )
	{
		GameState::ActiveAttack* sActiveAttacks = GAMESTATE->m_sActiveAttacks[m_PlayerNumber];	// NUM_INVENTORY_SLOTS
		for( int s=0; s<NUM_INVENTORY_SLOTS; s++ )
		{
			GameState::ActiveAttack& aa = sActiveAttacks[s];

			CString sDisplayText = aa.sModifier;
			if( sDisplayText == "" )
				m_text[s].SetText( "" );
			else
			{
				int iDisplaySecondsLeft = (int)(aa.fSecsRemaining+1);
				m_text[s].SetText( ssprintf("%s %02d:%02d", sDisplayText.GetString(), iDisplaySecondsLeft/60, iDisplaySecondsLeft%60) );
			}
		}
	}
}
