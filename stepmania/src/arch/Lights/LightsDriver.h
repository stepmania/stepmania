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

class LightsDriver
{
public:
	LightsDriver() {};
	virtual ~LightsDriver() {};
	
	virtual void SetLight( Light light, bool bOn ) = 0;
	virtual void Flush() = 0;
};


#endif
