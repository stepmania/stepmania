#ifndef LightsDriver_LinuxSerial_H
#define LightsDriver_LinuxSerial_H
/*
-----------------------------------------------------------------------------
 Class: LightsDriver_LinuxSerial

 Desc: Control lights.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "arch/Lights/LightsDriver.h"

class LightsDriver_LinuxSerial : public LightsDriver
{
public:
	LightsDriver_LinuxSerial();
	virtual ~LightsDriver_LinuxSerial();
	
	virtual void SetLight( Light light, bool bOn );
	virtual void Flush();
};



#endif
