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
	m_pInventory = NULL;

	for( int i=0; i<MAX_ACTIVE_ITEMS_LINES; i++ )
	{
		m_text[i].LoadFromFont( THEME->GetPathTo("Fonts","normal") );
		m_text[i].SetZoom( TEXT_ZOOM );
//		m_text[i].SetText( "WWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWW" );
		this->AddChild( &m_text[i] );
	}
}

void ActiveItemList::Init( PlayerNumber pn, Inventory* pInventory )
{
	m_PlayerNumber = pn;
	m_pInventory = pInventory;

	for( int i=0; i<MAX_ACTIVE_ITEMS_LINES; i++ )
	{
		m_text[i].SetHorizAlign( (Actor::HorizAlign)TEXT_HORIZ_ALIGN(pn) );
		bool bReverse = GAMESTATE->m_PlayerOptions[pn].m_bReverseScroll;
		float fHeight = SPACING_Y*MAX_ACTIVE_ITEMS_LINES * (bReverse ? 1 : -1 );
		float fY = SCALE(i,0.f,MAX_ACTIVE_ITEMS_LINES,0.f,fHeight);
		m_text[i].SetY( fY );
	}

}


void ActiveItemList::Update( float fDelta ) 
{ 
	ActorFrame::Update( fDelta ); 


	if( m_pInventory )
	{
		// refresh text only once every second
		float fNowSeconds = RageTimer::GetTimeSinceStart();
		float fLastSeconds = RageTimer::GetTimeSinceStart() - fDelta;

		if( (int)fNowSeconds != (int)fLastSeconds )
		{
			int iNumActiveItems = m_pInventory->m_ActiveItems[m_PlayerNumber].size();
			for( int i=0; i<MAX_ACTIVE_ITEMS_LINES; i++ )
			{
				if( i<iNumActiveItems )
				{
					const Inventory::ActiveItem& active_item = m_pInventory->m_ActiveItems[m_PlayerNumber][i];
					const Inventory::ItemDef& item_def = m_pInventory->m_ItemDefs[ active_item.iItemDefIndex ];

					int iDisplaySecondsLeft = (int)(active_item.fSecondsLeft+1);
					m_text[i].SetText( ssprintf("%s %02d:%02d", item_def.effect.GetString(), iDisplaySecondsLeft/60, iDisplaySecondsLeft%60) );
				}
				else
				{
					m_text[i].SetText( "" );
				}
			}
		}
	}
}
