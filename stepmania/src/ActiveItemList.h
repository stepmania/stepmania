#ifndef _ActiveItemList_H_
#define _ActiveItemList_H_
/*
-----------------------------------------------------------------------------
 File: ActiveItemList.h

 Desc: A graphic displayed in the ActiveItemList during Dancing.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "Sprite.h"
#include "ActorFrame.h"
#include "BitmapText.h"
#include "PlayerNumber.h"
class Inventory;


const int MAX_ACTIVE_ITEMS_LINES = 10;


class ActiveItemList : public ActorFrame
{
public:
	ActiveItemList();

	void Init( PlayerNumber pn, Inventory* pInventory );

	virtual void Update( float fDelta );

protected:
	BitmapText m_text[MAX_ACTIVE_ITEMS_LINES];

	PlayerNumber m_PlayerNumber;
	Inventory* m_pInventory;
};

#endif
