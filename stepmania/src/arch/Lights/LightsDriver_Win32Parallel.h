#ifndef LightsDriver_Win32Parallel_H
#define LightsDriver_Win32Parallel_H
/*
-----------------------------------------------------------------------------
 Class: LightsDriver_Win32Parallel

 Desc: Control lights.

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
	
	virtual void SetLight( Light light, bool bOn );
	virtual void Flush();
};



#endif
