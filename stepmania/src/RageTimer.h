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
	float GetTimeSinceStart() const;	// seconds since the program was started

private:
	int m_iLastDeltaTime;
};



extern RageTimer*		TIMER;	// global and accessable from anywhere in our program

#endif
