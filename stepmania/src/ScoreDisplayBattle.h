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
#include "Sprite.h"
#include "Inventory.h"



class ScoreDisplayBattle : public ScoreDisplay
{
public:
	ScoreDisplayBattle();

	virtual void Update( float fDelta );

protected:
	Sprite m_sprFrames[NUM_ITEM_SLOTS];
	Sprite m_sprItems[NUM_ITEM_SLOTS];

	int m_iLastSeenItems[NUM_ITEM_SLOTS];
};

#endif
