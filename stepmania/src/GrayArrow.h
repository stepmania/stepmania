/*
-----------------------------------------------------------------------------
 File: GrayArrow.h

 Desc: Class used to represent a color arrow on the screen.

 Copyright (c) 2001 Ben Norstrom.  All rights reserved.
-----------------------------------------------------------------------------
*/


#ifndef _GrayArrow_H_
#define _GrayArrow_H_


#include "Sprite.h"
#include "Steps.h"

class GrayArrow : public Sprite
{
public:
	GrayArrow();

	virtual void  SetBeat( const float fSongBeat );
	void Step();
};

#endif 
