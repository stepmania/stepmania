#include "global.h"

#include "RageTimer.h"
#include "RageLog.h"

#include "SDL-1.2.5/include/SDL.h"
#include "SDL-1.2.5/include/SDL_timer.h"
 
/* We only actually get 1000 using SDL. */
#define TIMESTAMP_RESOLUTION 1000000

static RageTimer g_Start;

float RageTimer::GetTimeSinceStart()
{
	return g_Start.Ago();
}

void RageTimer::Touch()
{
	unsigned ms = SDL_GetTicks();
	this->m_secs = ms / 1000;
	ms %= 1000;

	unsigned mult = TIMESTAMP_RESOLUTION / 1000;
	this->m_us = ms*mult;
}

float RageTimer::Ago() const
{
	const RageTimer Now;
	return Difference(*this, Now);
}

float RageTimer::GetDeltaTime()
{
	const RageTimer Now;
	const float diff = Difference( Now, *this );
	*this = Now;
	return diff;
}

float RageTimer::Difference(const RageTimer &lhs, const RageTimer &rhs)
{
	int secs = lhs.m_secs - rhs.m_secs;
	int us = lhs.m_us - rhs.m_us;

	if( us < 0 )
	{
		us += TIMESTAMP_RESOLUTION;
		--secs;
	}

	return float(secs) + float(us) / TIMESTAMP_RESOLUTION;
}

/*
 * Copyright (c) 2001-2003 by the person(s) listed below.  All rights reserved.
 *	Chris Danford
 *	Glenn Maynard
 */
