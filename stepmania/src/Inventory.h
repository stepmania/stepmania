#ifndef Inventory_H
#define Inventory_H
/*
-----------------------------------------------------------------------------
 Class: Inventory

 Desc: Inventory management for PLAY_MODE_BATTLE.

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
	~Inventory();
	void Load( PlayerNumber pn );

	virtual void Update( float fDelta );
	virtual void DrawPrimitives() {};

	void UseItem( int iSlot );

protected:
	void AwardItem( int iItemIndex );

	PlayerNumber m_PlayerNumber;
	int m_iLastSeenCombo;

	RageSound m_soundAcquireItem;
	vector<RageSound*> m_vpSoundUseItem;
	RageSound m_soundItemEnding;
};

#endif
