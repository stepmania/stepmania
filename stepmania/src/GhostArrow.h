/*
-----------------------------------------------------------------------------
 File: GhostArrow.h

 Desc: Class used to represent a ghost arrow on the screen.

 Copyright (c) 2001 Ben Norstrom.  All rights reserved.
-----------------------------------------------------------------------------
*/


#ifndef _GHOST_ARROW_H_
#define _GHOST_ARROW_H_


#include "Arrow.h"

class GhostArrow : public Arrow  
{
public:
	GhostArrow();

	virtual void Step();
};

#endif
