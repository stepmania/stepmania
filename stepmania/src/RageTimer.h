#pragma once
/*
-----------------------------------------------------------------------------
 File: RageTimer.h

 Desc: Timer services.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

class RageTimer
{
public:
	RageTimer();
	~RageTimer();
	float GetDeltaTime();	// time between last call to GetDeltaTime()
	float PeekDeltaTime();
	float GetTimeSinceStart();	// seconds since the program was started

private:
	float m_fLastDeltaTime;
	float m_fTimeSinceStart;	// seconds since the program was started
};



extern RageTimer*		TIMER;	// global and accessable from anywhere in our program
