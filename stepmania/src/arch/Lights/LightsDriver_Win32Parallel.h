#ifndef LightsDriver_Win32Parallel_H
#define LightsDriver_Win32Parallel_H
/*
-----------------------------------------------------------------------------
 Class: LightsDriver_Win32Parallel

 Desc: Control lights with Kit 74:
	http://www.google.com/search?hl=en&lr=&ie=UTF-8&oe=UTF-8&q=kit+74+relay

 Copyright (c) 2003 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "arch/Lights/LightsDriver.h"

class LightsDriver_Win32Parallel : public LightsDriver
{
public:
	LightsDriver_Win32Parallel();
	virtual ~LightsDriver_Win32Parallel();
	
	virtual void Set( const LightsState *ls );
};



#endif
