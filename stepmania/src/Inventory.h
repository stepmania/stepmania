#ifndef Inventory_H
#define Inventory_H
/*
-----------------------------------------------------------------------------
 Class: Inventory

 Desc: This a mark the player receives after clearing a song.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "Actor.h"
#include "PlayerNumber.h"
#include "RageSound.h"


class Inventory : public Actor
{
public:
	Inventory();
	void Reset();
	void RefreshPossibleItems();

	virtual void Update( float fDelta );
	virtual void DrawPrimitives() {};

	bool OnComboBroken( PlayerNumber pn, int iCombo );
	void UseItem( PlayerNumber pn, int iSlot );
	void RemoveAllActiveItems();	// called on end of song

	struct ItemDef
	{
		int comboLevel;
		CString effect;

		bool operator<( const ItemDef& other ) const { return comboLevel < other.comboLevel; }
		void Sort( vector<ItemDef>& items ) { sort( items.begin(), items.end() ); }
	};
	vector<ItemDef>	m_ItemDefs;

	struct ActiveItem
	{
		float fSecondsLeft;
		int iItemDefIndex;
	};
	vector<ActiveItem>	m_ActiveItems[NUM_PLAYERS];

protected:
	void RebuildPlayerOptions( PlayerNumber pn );

	RageSound m_soundAcquireItem;
	RageSound m_soundUseItem;
	RageSound m_soundItemEnding;
};

#endif
