#pragma once
/*
-----------------------------------------------------------------------------
 Class: TransitionInvisible

 Desc: Doesn't draw anything.  Only used for timing.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/


#include "Transition.h"
#include "RageDisplay.h"
#include "RageSound.h"
#include "Quad.h"


class TransitionInvisible : public Transition
{
public:
	TransitionInvisible();

	virtual void DrawPrimitives();

protected:
};
