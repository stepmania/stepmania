/*
-----------------------------------------------------------------------------
 Class: GrayArrow

 Desc: A gray arrow that "receives" ColorNotes.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Ben Nordstrom
	Chris Danford
-----------------------------------------------------------------------------
*/


#ifndef _GrayArrow_H_
#define _GrayArrow_H_


#include "Sprite.h"
#include "Notes.h"

class GrayArrow : public Sprite
{
public:
	GrayArrow();

	virtual void  Update( float fDeltaTime );
	void Step();
};

#endif 
