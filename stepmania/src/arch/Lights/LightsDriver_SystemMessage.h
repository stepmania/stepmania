#ifndef LightsDriver_SystemMessage_H
#define LightsDriver_SystemMessage_H
/*
-----------------------------------------------------------------------------
 Class: LightsDriver_SystemMessage

 Desc: Control lights.

 Copyright (c) 2003 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "arch/Lights/LightsDriver.h"

class LightsDriver_SystemMessage : public LightsDriver
{
public:
	LightsDriver_SystemMessage();
	virtual ~LightsDriver_SystemMessage();
	
	virtual void SetLight( Light light, bool bOn );
	virtual void Flush();
};



#endif
