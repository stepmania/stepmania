#ifndef QUAD_H
#define QUAD_H
/*
-----------------------------------------------------------------------------
 Class: Quad

 Desc: A rectangle shaped actor with color.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "Sprite.h"


class Quad : public Sprite
{
public:
	Quad()
	{
		m_bDrawIfTextureNull = true; 
	}	
};

#endif
