#ifndef RAGE_TIMER_H
#define RAGE_TIMER_H

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
	unsigned int GetTicks();
	float GetDeltaTime();	// time between last call to GetDeltaTime()
	float PeekDeltaTime() const;
	static float GetTimeSinceStart();	// seconds since the program was started

private:
	static void Init();
	int m_iLastDeltaTime;
};

#endif
