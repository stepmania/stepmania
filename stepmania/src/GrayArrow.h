/*
-----------------------------------------------------------------------------
 File: GrayArrow.h

 Desc: Class used to represent a gray arrow on the screen.

 Copyright (c) 2001 Ben Norstrom.  All rights reserved.
-----------------------------------------------------------------------------
*/


#ifndef _GRAYARROW_H_
#define _GRAYARROW_H_


#include "Arrow.h"

class GrayArrow : public Arrow  
{
public:
	GrayArrow();

	// animates a step on the arrow
	virtual void Step();

	virtual void CalculateColor( const float fBeatsTilStep );
};

#endif 
