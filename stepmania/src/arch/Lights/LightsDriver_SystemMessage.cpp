#include "global.h"
/*
-----------------------------------------------------------------------------
 Class: LightsDriver_SystemMessage

 Desc: See header.

 Copyright (c) 2003 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "LightsDriver_SystemMessage.h"
#include "ScreenManager.h"


bool g_bLightOn[NUM_LIGHTS];

LightsDriver_SystemMessage::LightsDriver_SystemMessage()
{
	memset( g_bLightOn, 0, sizeof(g_bLightOn) );
}

LightsDriver_SystemMessage::~LightsDriver_SystemMessage()
{
}

void LightsDriver_SystemMessage::SetLight( Light light, bool bOn )
{
	g_bLightOn[light] = bOn;
}

void LightsDriver_SystemMessage::Flush()
{
	CString s = "Lights: ";
	for( int i=0; i<NUM_LIGHTS; i++ )
	{
		s += g_bLightOn[i] ? '1' : '0';
	}
	SCREENMAN->SystemMessageNoAnimate( s );
}
