#include "global.h"
/*
-----------------------------------------------------------------------------
 Class: Inventory

 Desc: See header.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "Inventory.h"
#include "ThemeManager.h"
#include "RageUtil.h"
#include "GameState.h"


#define NUM_ITEM_TYPES				THEME->GetMetricF("Inventory","NumItemTypes")
#define ITEM_DURATION_SECONDS		THEME->GetMetricF("Inventory","ItemDuration")
#define ITEM_COMBO( i )				THEME->GetMetricI("Inventory",ssprintf("Item%dCombo",i+1))
#define ITEM_EFFECT( i )			THEME->GetMetric ("Inventory",ssprintf("Item%dEffect",i+1))


bool ItemDef::operator<( const ItemDef& other )
{
	return comboLevel < other.comboLevel;
}

void ItemDef::Sort( vector<ItemDef>& items )
{
	sort( items.begin(), items.end() );
}


Inventory::Inventory()
{
	Reset();
}

void Inventory::Reset()
{
	for( int i=0; i<NUM_ITEM_SLOTS; i++ )
		m_iItems[i] = ITEM_NONE;
}

void Inventory::RefreshPossibleItems()
{
	m_ItemDefs.clear();

	for( int i=0; i<NUM_ITEM_TYPES; i++ )
	{
		ItemDef def;
		def.comboLevel = ITEM_COMBO(i);
		def.effect = ITEM_EFFECT(i);
		m_ItemDefs.push_back( def );
	}
}

void Inventory::OnComboBroken( int iCombo )
{
	// search for the item acquired
	for( int item=0; item<(int)m_ItemDefs.size(); item++ )
		if( m_ItemDefs[item].comboLevel > iCombo )
			break;

	item--;	// back up because we went one too far

	if( item == -1 )	// no item acquired
		return;
	else
	{
		// give them an item

		// search for the first open slot
		int iOpenSlot = -1;

		if( m_iItems[NUM_ITEM_SLOTS/2] == ITEM_NONE )
			iOpenSlot = NUM_ITEM_SLOTS/2;
		else
		{
			for( int s=0; s<NUM_ITEM_SLOTS; s++ )
				if( m_iItems[s] == ITEM_NONE )
				{
					iOpenSlot = s;
					break;
				}
		}

		if( iOpenSlot != -1 )
			m_iItems[iOpenSlot] = item;
		else
			; // not enough room to insert item
	}
}

void Inventory::UseItem( PlayerNumber pn, int iSlot )
{
	if( m_iItems[iSlot] == ITEM_NONE )
		return;

	const ItemDef& def = m_ItemDefs[iSlot];

    PlayerNumber pnToAttack;
	switch( pn )
	{
	case PLAYER_1:	pnToAttack = PLAYER_2;	break;
	case PLAYER_2:	pnToAttack = PLAYER_1;	break;
	default:	ASSERT(0);	pnToAttack = PLAYER_INVALID;	break;
	}

	GAMESTATE->m_PlayerOptions[pnToAttack].FromString( def.effect );

	// remove the item
	m_iItems[iSlot] = ITEM_NONE;
}

