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

const int ITEM_NONE			= -1;
const int NUM_ITEM_SLOTS	= 3;
const int MAX_ITEM_TYPES	= 20;

struct ItemDef
{
	int comboLevel;
	CString effect;

	bool operator<( const ItemDef& other );
	void Sort( vector<ItemDef>& items );
};

class Inventory 
{
public:
	Inventory();
	void Reset();
	void RefreshPossibleItems();

	void OnComboBroken( int iCombo );
	void UseItem( PlayerNumber pn, int iSlot );

	vector<ItemDef>	m_ItemDefs;
	int m_iItems[NUM_ITEM_SLOTS];
};

#endif
