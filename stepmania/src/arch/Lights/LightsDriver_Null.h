#ifndef LightsDriver_Null_H
#define LightsDriver_Null_H
/*
-----------------------------------------------------------------------------
 Class: LightsDriver_Null

 Desc: Control lights.

 Copyright (c) 2003 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "LightsDriver.h"

class LightsDriver_Null : public LightsDriver
{
public:
	LightsDriver_Null() {};
	virtual ~LightsDriver_Null() {};
	
	void Set( const LightsState *ls ) {};
};


#endif
