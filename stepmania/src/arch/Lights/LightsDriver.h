#ifndef LightsDriver_H
#define LightsDriver_H
/*
-----------------------------------------------------------------------------
 Class: LightsDriver

 Desc: Control lights.

 Copyright (c) 2003 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "LightsManager.h"

struct LightsState;

class LightsDriver
{
public:
	LightsDriver() {};
	virtual ~LightsDriver() {};
	
	virtual void Set( const LightsState *ls ) = 0;
};


#endif
