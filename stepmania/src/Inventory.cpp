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
#include "PrefsManager.h"
#include "song.h"


#define NUM_ITEM_TYPES			THEME->GetMetricF("Inventory","NumItemTypes")
#define ITEM_DURATION_SECONDS	THEME->GetMetricF("Inventory","ItemDurationSeconds")
#define ITEM_COMBO( i )			THEME->GetMetricI("Inventory",ssprintf("Item%dCombo",i+1))
#define ITEM_EFFECT( i )		THEME->GetMetric ("Inventory",ssprintf("Item%dEffect",i+1))
#define ITEM_LEVEL( i )			THEME->GetMetricI("Inventory",ssprintf("Item%dLevel",i+1))
CachedThemeMetricF ITEM_USE_RATE_SECONDS("Inventory","ItemUseRateSeconds");

const PlayerNumber	OPPOSITE_PLAYER[NUM_PLAYERS] = { PLAYER_2, PLAYER_1 };

#define ITEM_USE_PROBABILITY (1.f/ITEM_USE_RATE_SECONDS)

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
}

void Inventory::Load( PlayerNumber pn )
{
	ITEM_USE_RATE_SECONDS.Refresh();

	ReloadItems();

	m_PlayerNumber = pn;
	m_iLastSeenCombo = 0;

	// don't load battle sounds if they're not going to be used
	if( GAMESTATE->m_PlayMode == PLAY_MODE_BATTLE )
	{
		for( int p=0; p<NUM_PLAYERS; p++ )
		{
			m_soundAcquireItem.Load( THEME->GetPathToS("Inventory aquire item") );
			m_soundUseItem.Load( THEME->GetPathToS("Inventory use item") );
			m_soundItemEnding.Load( THEME->GetPathToS("Inventory item ending") );
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
#define BROKE_ABOVE(i) (iNewCombo<iOldCombo)&&(iOldCombo>=i)

		for( unsigned i=0; i<g_Items.size(); i++ )
		{
			bool bEarnedThisItem = false;
			if( PREFSMAN->m_bBreakComboToGetItem )
				bEarnedThisItem = BROKE_ABOVE(g_Items[i].iCombo);
			else
				bEarnedThisItem = CROSSED(g_Items[i].iCombo);
			
			if( bEarnedThisItem )
			{
				AwardItem( i );
				break;
			}
		}
	}


	// use items if this player is CPU-controlled
	if( GAMESTATE->m_PlayerController[m_PlayerNumber] != PC_HUMAN &&
		GAMESTATE->m_fSongBeat < GAMESTATE->m_pCurSong->m_fLastBeat )
	{
		// every 1 seconds, try to use an item
		int iLastSecond = (int)(RageTimer::GetTimeSinceStart() - fDelta);
		int iThisSecond = (int)RageTimer::GetTimeSinceStart();
		if( iLastSecond != iThisSecond )
		{
			for( int s=0; s<NUM_INVENTORY_SLOTS; s++ )
				if( !GAMESTATE->m_Inventory[m_PlayerNumber][s].IsBlank() )
					if( randomf(0,1) < ITEM_USE_PROBABILITY )
						UseItem( s );
		}
	}
}

void Inventory::AwardItem( int iItemIndex )
{
	// search for the first open slot
	int iOpenSlot = -1;

	GameState::Attack* asInventory = GAMESTATE->m_Inventory[m_PlayerNumber]; //[NUM_INVENTORY_SLOTS]

	if( asInventory[NUM_INVENTORY_SLOTS/2].IsBlank() )
		iOpenSlot = NUM_INVENTORY_SLOTS/2;
	else
	{
		for( int s=0; s<NUM_INVENTORY_SLOTS; s++ )
			if( asInventory[s].IsBlank() )
			{
				iOpenSlot = s;
				break;
			}
	}

	if( iOpenSlot != -1 )
	{
		GameState::Attack a;
		a.sModifier = g_Items[iItemIndex].sModifier;
		a.fSecsRemaining = ITEM_DURATION_SECONDS;
		a.level = g_Items[iItemIndex].level;
		asInventory[iOpenSlot] = a;
		m_soundAcquireItem.Play();
	}
	// else not enough room to insert item
}

void Inventory::UseItem( int iSlot )
{
	GameState::Attack* asInventory = GAMESTATE->m_Inventory[m_PlayerNumber]; //[NUM_INVENTORY_SLOTS]

	if( asInventory[iSlot].IsBlank() )
		return;

    PlayerNumber pnToAttack = OPPOSITE_PLAYER[m_PlayerNumber];
	GameState::Attack a = asInventory[iSlot];

	GAMESTATE->LaunchAttack( pnToAttack, a );
	GAMESTATE->RebuildPlayerOptionsFromActiveAttacks( pnToAttack );

	// remove the item
	asInventory[iSlot].MakeBlank();
	m_soundUseItem.Play();
}
