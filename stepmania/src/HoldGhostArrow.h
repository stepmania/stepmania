#pragma once
/*
-----------------------------------------------------------------------------
 Class: HoldGhostArrow

 Desc: The "electricity around the stationary arrow as it's pressing a HoldNote.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Ben Nordstrom
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "Sprite.h"

class HoldGhostArrow : public Sprite
{
public:
	HoldGhostArrow();

	virtual void Update( float fDeltaTime );
	virtual void DrawPrimitives();

	void  Step();

	bool m_bWasSteppedOnLastFrame;
	float m_fHeatLevel;	// brightness - between 0 and 1
};

