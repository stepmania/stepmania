#ifndef PERCENTAGE_DISPLAY_H
#define PERCENTAGE_DISPLAY_H

#include "ActorFrame.h"
#include "PlayerNumber.h"
#include "BitmapText.h"

class PercentageDisplay: public ActorFrame
{
public:
	PercentageDisplay();
	void Load( PlayerNumber pn );
	void Update( float fDeltaTime );

private:
	PlayerNumber m_PlayerNumber;
	int m_Last;
	BitmapText	m_textPercent;
};

#endif
/*
 * Copyright (c) 2001-2003 by the person(s) listed below.  All rights reserved.
 *  Chris Danford
 */
