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
#include "RageTimer.h"


#define NUM_ITEM_TYPES			THEME->GetMetricF("Inventory","NumItemTypes")
#define ITEM_DURATION_SECONDS	THEME->GetMetricF("Inventory","ItemDurationSeconds")
#define ITEM_COMBO( i )			THEME->GetMetricI("Inventory",ssprintf("Item%dCombo",i+1))
#define ITEM_EFFECT( i )		THEME->GetMetric ("Inventory",ssprintf("Item%dEffect",i+1))
#define ITEM_LEVEL( i )			THEME->GetMetricI("Inventory",ssprintf("Item%dLevel",i+1))

const PlayerNumber	OPPOSITE_PLAYER[NUM_PLAYERS] = { PLAYER_2, PLAYER_1 };


struct Item
{
	AttackLevel level;
	int iCombo;
	CString sModifier;
};
vector<Item>	g_Items;

void ReloadItems()
{
	g_Items.clear();
	for( int i=0; i<NUM_ITEM_TYPES; i++ )
	{
		Item item;
		item.level = (AttackLevel)(ITEM_LEVEL(i)-1);
		item.iCombo = ITEM_COMBO(i);
		item.sModifier = ITEM_EFFECT(i);
		g_Items.push_back( item );
	}
}


Inventory::Inventory()
{
	ReloadItems();
}

void Inventory::Load( PlayerNumber pn )
{
	m_PlayerNumber = pn;
	m_iLastSeenCombo = 0;

	// don't load battle sounds if they're not going to be used
	if( GAMESTATE->m_PlayMode == PLAY_MODE_BATTLE )
	{
		for( int p=0; p<NUM_PLAYERS; p++ )
		{
			m_soundAcquireItem.Load( THEME->GetPathTo("Sounds","Inventory aquire item") );
			m_soundUseItem.Load( THEME->GetPathTo("Sounds","Inventory use item") );
			m_soundItemEnding.Load( THEME->GetPathTo("Sounds","Inventory item ending") );
		}
	}
}

void Inventory::Update( float fDelta )
{
	if( GAMESTATE->m_PlayMode != PLAY_MODE_BATTLE )
		return;

	if( GAMESTATE->m_bActiveAttackEndedThisUpdate[m_PlayerNumber] )
		m_soundItemEnding.Play();

	PlayerNumber pn = m_PlayerNumber;

	// check to see if they deserve a new item
	if( GAMESTATE->m_CurStageStats.iCurCombo[pn] != m_iLastSeenCombo )
	{
		int iOldCombo = m_iLastSeenCombo;
		m_iLastSeenCombo = GAMESTATE->m_CurStageStats.iCurCombo[pn];
		int iNewCombo = m_iLastSeenCombo;
		
#define CROSSED(i) (iOldCombo<i)&&(iNewCombo>=i)

		for( unsigned i=0; i<g_Items.size(); i++ )
		{
			if( CROSSED(g_Items[i].iCombo) )
			{
				AwardItem( i );
				break;
			}
		}
	}


	// use items if this player is CPU-controlled
	if( GAMESTATE->m_PlayerController[m_PlayerNumber] != HUMAN )
	{
		// every one second, consider using an item
		int iLastSecond = (int)RageTimer::GetTimeSinceStart() - fDelta;
		int iThisSecond = (int)RageTimer::GetTimeSinceStart();
		if( iLastSecond != iThisSecond )
		{
			int iSlotToConsider = rand()%NUM_INVENTORY_SLOTS;
			if( GAMESTATE->m_sInventory[m_PlayerNumber][iSlotToConsider] != ""  &&
				rand()%5 == 0 )
			{
				UseItem( iSlotToConsider );
			}
		}
	}
}

void Inventory::AwardItem( int iItemIndex )
{
	// search for the first open slot
	int iOpenSlot = -1;

	CString* asInventory = GAMESTATE->m_sInventory[m_PlayerNumber]; //[NUM_INVENTORY_SLOTS]

	if( asInventory[NUM_INVENTORY_SLOTS/2] == "" )
		iOpenSlot = NUM_INVENTORY_SLOTS/2;
	else
	{
		for( int s=0; s<NUM_INVENTORY_SLOTS; s++ )
			if( asInventory[s] == "" )
			{
				iOpenSlot = s;
				break;
			}
	}

	if( iOpenSlot != -1 )
	{
		CString sAttackToGive = g_Items[iItemIndex].sModifier;
		asInventory[iOpenSlot] = sAttackToGive;
		m_soundAcquireItem.Play();
	}
	// else not enough room to insert item
}

void Inventory::UseItem( int iSlot )
{
	CString* asInventory = GAMESTATE->m_sInventory[m_PlayerNumber]; //[NUM_INVENTORY_SLOTS]

	if( asInventory[iSlot] == "" )
		return;

    PlayerNumber pnToAttack = OPPOSITE_PLAYER[m_PlayerNumber];
	GameState::ActiveAttack aa;
	aa.sModifier = asInventory[iSlot];
	aa.fSecsRemaining = ITEM_DURATION_SECONDS;
	aa.level = ATTACK_LEVEL_1;

	// UGLY:  Search through g_Items and find out what the attack level for this item is.
	for( unsigned i=0; i<g_Items.size(); i++ )
	{
		if( aa.sModifier == g_Items[i].sModifier )
		{
			aa.level = g_Items[i].level;
			break;
		}
	}

	GAMESTATE->LaunchAttack( pnToAttack, aa );
	GAMESTATE->RebuildPlayerOptionsFromActiveAttacks( pnToAttack );

	// remove the item
	asInventory[iSlot] = "";
	m_soundUseItem.Play();
}
