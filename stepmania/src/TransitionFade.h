#ifndef TRANSITIONFADE_H
#define TRANSITIONFADE_H
/*
-----------------------------------------------------------------------------
 Class: TransitionFade

 Desc: Fades whole screen to color.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/


#include "Transition.h"
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


#endif
