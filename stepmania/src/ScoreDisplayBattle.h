#ifndef ScoreDisplayBattle_H
#define ScoreDisplayBattle_H
/*
-----------------------------------------------------------------------------
 Class: ScoreDisplayBattle

 Desc: Shows point score during gameplay and used in some menus.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "ScoreDisplay.h"
#include "GameConstantsAndTypes.h"
#include "OptionIcon.h"


class ScoreDisplayBattle : public ScoreDisplay
{
public:
	ScoreDisplayBattle();
	virtual void Init( PlayerNumber pn );

	virtual void Update( float fDelta );

protected:
	OptionIcon m_Inventory[NUM_INVENTORY_SLOTS];

	CString m_iLastSeenInventory[NUM_INVENTORY_SLOTS];
};

#endif
