#pragma once
/*
-----------------------------------------------------------------------------
 Class: TransitionFade

 Desc: Fades whole screen to color.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/


#include "Transition.h"
#include "RageDisplay.h"
#include "RageSound.h"
#include "Quad.h"


class TransitionFade : public Transition
{
public:
	TransitionFade();
	virtual ~TransitionFade();

	virtual void DrawPrimitives();

protected:
	Quad m_rect;
};

