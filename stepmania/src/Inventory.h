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
#include "GameConstantsAndTypes.h"


class Inventory : public Actor
{
public:
	Inventory();
	void Load( PlayerNumber pn );

	virtual void Update( float fDelta );
	virtual void DrawPrimitives() {};

	void UseAttack( int iSlot );

protected:
	void AwardItem( AttackLevel al );

	PlayerNumber m_PlayerNumber;
	int m_iLastSeenCombo;

	RageSound m_soundAcquireItem;
	RageSound m_soundUseItem;
	RageSound m_soundItemEnding;
};

#endif
