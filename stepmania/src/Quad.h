#pragma once
/*
-----------------------------------------------------------------------------
 Class: Quad

 Desc: A rectangle shaped actor with color.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "Actor.h"


class Quad : public Actor
{
public:
	Quad();

	virtual void DrawPrimitives();
	
};
