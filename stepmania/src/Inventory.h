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

#include "GameConstantsAndTypes.h"
#include "RageSound.h"

const int MAX_ITEM_TYPES	= 20;


class Inventory
{
public:
	Inventory();
	void Reset();
	void RefreshPossibleItems();

	bool OnComboBroken( PlayerNumber pn, int iCombo );
	void UseItem( PlayerNumber pn, int iSlot );

	struct ItemDef
	{
		int comboLevel;
		CString effect;

		bool operator<( const ItemDef& other ) { return comboLevel < other.comboLevel; }
		void Sort( vector<ItemDef>& items ) { sort( items.begin(), items.end() ); }
	};
	vector<ItemDef>	m_ItemDefs;

	RageSound m_soundAcquireItem;
	RageSound m_soundUseItem;
};

#endif
