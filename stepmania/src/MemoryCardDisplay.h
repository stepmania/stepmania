#ifndef MEMORY_CARD_DISPLAY_H
#define MEMORY_CARD_DISPLAY_H

/*
-----------------------------------------------------------------------
File: MemoryCardDisplay.h

Desc: A graphic displayed on the screen during dance

Copyright (c) 2003 by the person(s) listed below. All rights reserved.
  Chris Danford
-----------------------------------------------------------------------
*/

#include "GameConstantsAndTypes.h"
#include "PlayerNumber.h"
#include "Sprite.h"
#include "ActorFrame.h"


class MemoryCardDisplay : public ActorFrame
{
public:
	MemoryCardDisplay();
	void Load( PlayerNumber pn );
	void Update( float fDelta );

protected:
	PlayerNumber m_PlayerNumber;
	MemoryCardState	m_LastSeenState;
	Sprite m_spr[NUM_MEMORY_CARD_STATES];
};

#endif
