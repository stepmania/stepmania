#include "global.h"
#include "Inventory.h"
#include "ThemeManager.h"
#include "RageUtil.h"
#include "GameState.h"
#include "RageTimer.h"
#include "PrefsManager.h"
#include "song.h"
#include "ScreenManager.h"
#include "StatsManager.h"
#include "ThemeMetric.h"
#include "PlayerState.h"


#define NUM_ITEM_TYPES			THEME->GetMetricF("Inventory","NumItemTypes")
#define ITEM_DURATION_SECONDS	THEME->GetMetricF("Inventory","ItemDurationSeconds")
#define ITEM_COMBO( i )			THEME->GetMetricI("Inventory",ssprintf("Item%dCombo",i+1))
#define ITEM_EFFECT( i )		THEME->GetMetric ("Inventory",ssprintf("Item%dEffect",i+1))
#define ITEM_LEVEL( i )			THEME->GetMetricI("Inventory",ssprintf("Item%dLevel",i+1))
ThemeMetric<float> ITEM_USE_RATE_SECONDS("Inventory","ItemUseRateSeconds");

#define ITEM_USE_PROBABILITY (1.f/ITEM_USE_RATE_SECONDS)

struct Item
{
	AttackLevel level;
	int iCombo;
	RString sModifier;
};
static vector<Item>	g_Items;

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
	switch( GAMESTATE->m_PlayMode )
	{
	case PLAY_MODE_BATTLE:
		break;
	default:
		ASSERT(0);
	}
}

Inventory::~Inventory()
{
	for( unsigned i=0; i<m_vpSoundUseItem.size(); i++ )
		delete m_vpSoundUseItem[i];
	m_vpSoundUseItem.clear();
}

void Inventory::Load( PlayerState* pPlayerState )
{
	ReloadItems();

	m_pPlayerState = pPlayerState;
	m_iLastSeenCombo = 0;

	// don't load battle sounds if they're not going to be used
	switch( GAMESTATE->m_PlayMode )
	{
	case PLAY_MODE_BATTLE:
		m_soundAcquireItem.Load( THEME->GetPathS("Inventory","aquire item") );
		for( unsigned i=0; i<g_Items.size(); i++ )
		{
			RageSound* pSound = new RageSound;
			pSound->Load( THEME->GetPathS("Inventory",ssprintf("use item %u",i+1)) );
			m_vpSoundUseItem.push_back( pSound );
		}
		m_soundItemEnding.Load( THEME->GetPathS("Inventory","item ending") );
		break;
	}
}

void Inventory::Update( float fDelta )
{
	if( m_pPlayerState->m_bAttackEndedThisUpdate )
		m_soundItemEnding.Play();

	// TODO: remove use of PlayerNumber
	PlayerNumber pn = m_pPlayerState->m_PlayerNumber;

	// check to see if they deserve a new item
	if( STATSMAN->m_CurStageStats.m_player[pn].iCurCombo != m_iLastSeenCombo )
	{
		int iOldCombo = m_iLastSeenCombo;
		m_iLastSeenCombo = STATSMAN->m_CurStageStats.m_player[pn].iCurCombo;
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
	if( m_pPlayerState->m_PlayerController != PC_HUMAN &&
		GAMESTATE->m_fSongBeat < GAMESTATE->m_pCurSong->m_fLastBeat )
	{
		// every 1 seconds, try to use an item
		int iLastSecond = (int)(RageTimer::GetTimeSinceStartFast() - fDelta);
		int iThisSecond = (int)RageTimer::GetTimeSinceStartFast();
		if( iLastSecond != iThisSecond )
		{
			for( int s=0; s<NUM_INVENTORY_SLOTS; s++ )
				if( !m_pPlayerState->m_Inventory[s].IsBlank() )
					if( randomf(0,1) < ITEM_USE_PROBABILITY )
						UseItem( s );
		}
	}
}

void Inventory::AwardItem( int iItemIndex )
{
	// CPU player is vanity only.  It should have no effect on gameplay
	// and should not aquire/launch attacks.
	if( m_pPlayerState->m_PlayerController == PC_CPU )
		return;


	// search for the first open slot
	int iOpenSlot = -1;

	Attack* pInventory = m_pPlayerState->m_Inventory; //[NUM_INVENTORY_SLOTS]

	if( pInventory[NUM_INVENTORY_SLOTS/2].IsBlank() )
	{
		iOpenSlot = NUM_INVENTORY_SLOTS/2;
	}
	else
	{
		for( int s=0; s<NUM_INVENTORY_SLOTS; s++ )
		{
			if( pInventory[s].IsBlank() )
			{
				iOpenSlot = s;
				break;
			}
		}
	}

	if( iOpenSlot != -1 )
	{
		Attack a;
		a.sModifiers = g_Items[iItemIndex].sModifier;
		a.fSecsRemaining = ITEM_DURATION_SECONDS;
		a.level = g_Items[iItemIndex].level;
		pInventory[iOpenSlot] = a;
		m_soundAcquireItem.Play();
	}
	// else not enough room to insert item
}

void Inventory::UseItem( int iSlot )
{
	Attack* pInventory = m_pPlayerState->m_Inventory; //[NUM_INVENTORY_SLOTS]

	if( pInventory[iSlot].IsBlank() )
		return;

	PlayerNumber pn = m_pPlayerState->m_PlayerNumber;
	Attack a = pInventory[iSlot];

	// remove the item
	pInventory[iSlot].MakeBlank();
	m_vpSoundUseItem[a.level]->Play();

	PlayerNumber pnToAttack = OPPOSITE_PLAYER[pn];
	PlayerState *pPlayerStateToAttack = GAMESTATE->m_pPlayerState[pnToAttack];
	pPlayerStateToAttack->LaunchAttack( a );

	float fPercentHealthToDrain = (a.level+1) / 10.f;
	ASSERT( fPercentHealthToDrain > 0 );
	GAMESTATE->m_fOpponentHealthPercent -= fPercentHealthToDrain;
	CLAMP( GAMESTATE->m_fOpponentHealthPercent, 0.f, 1.f );

	// play announcer sound
	SCREENMAN->SendMessageToTopScreen( (ScreenMessage)(SM_BattleDamageLevel1+a.level) );
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
