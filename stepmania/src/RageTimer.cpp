#include "global.h"
/*
-----------------------------------------------------------------------------
 File: RageTimer.cpp

 Desc: See header.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/


#include "RageTimer.h"
#include "RageLog.h"

#include "SDL-1.2.5/include/SDL.h"
#include "SDL-1.2.5/include/SDL_timer.h"
 
const float SECS_IN_DAY	=	60*60*24;

/* XXX: SDL_GetTicks() wraps every month or so.  Handle it. */
RageTimer::RageTimer()
{
	Init();
	GetDeltaTime(); /* so the next call to GetDeltaTime is from the construction of this object */
}

void RageTimer::Init()
{
/*
 This is needed for the "timer" system, not the "ticks" system; it often starts
 a thread, so let's not do it--we don't need it.
	static bool SDL_Initialized = false;
	if(!SDL_Initialized) {
		SDL_InitSubSystem(SDL_INIT_TIMER);
		SDL_Initialized = true;
	}
*/
}

float RageTimer::GetDeltaTime()
{
	/* Store the LDT in integral milliseconds, to avoid rounding error. */
	int now = SDL_GetTicks();
	int ret = now - m_iLastDeltaTime;
	m_iLastDeltaTime = now;
	return ret / 1000.f;
}

float RageTimer::PeekDeltaTime() const
{
	return (SDL_GetTicks() - m_iLastDeltaTime) / 1000.f;
}

float RageTimer::GetTimeSinceStart()
{
	return SDL_GetTicks() / 1000.f;
}

/* XXX: SDL_GetTicks() wraps every month or so.  Handle it. */
unsigned int GetTicks()
{
	return SDL_GetTicks();
}
