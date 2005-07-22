/*
 * This can be used in two ways: as a timestamp or as a timer.
 *
 * As a timer,
 * RageTimer Timer;
 * while(1) {
 *   printf( "Will be approximately: %f", Timer.PeekDeltaTime()) ;
 *   float fDeltaTime = Timer.GetDeltaTime();
 * }
 *
 * or as a timestamp:
 * void foo( RageTimer &timestamp ) {
 *     if( timestamp.IsZero() )
 *         printf( "The timestamp isn't set." );
 *     else
 *         printf( "The timestamp happened %f ago", timestamp.Ago() );
 *     timestamp.Touch();
 *     printf( "Near zero: %f", timestamp.Age() );
 * }
 */

#include "global.h"

#include "RageTimer.h"
#include "RageLog.h"

#include "arch/ArchHooks/ArchHooks.h"
 
#define TIMESTAMP_RESOLUTION 1000000

const RageTimer RageZeroTimer(0,0);
static uint64_t g_iStartTime = ArchHooks::GetMicrosecondsSinceStart( true );

static uint64_t GetTime( bool bAccurate )
{
	// if !bAccurate, then don't call ArchHooks to find the current time.  Just return the 
	// last calculated time.  GetMicrosecondsSinceStart is slow on some archs.
	static uint64_t usecs = 0;
	if( bAccurate )
		usecs = ArchHooks::GetMicrosecondsSinceStart( true );
	return usecs;
}

float RageTimer::GetTimeSinceStart( bool bAccurate )
{
	uint64_t usecs = GetTime( bAccurate );
	usecs -= g_iStartTime;
	uint32_t ret = usecs / 1000000;
	return (float)ret;
}

void RageTimer::Touch()
{
	uint64_t usecs = GetTime( true );

	this->m_secs = unsigned(usecs / 1000000);
	this->m_us = unsigned(usecs % 1000000);
}

float RageTimer::Ago() const
{
	const RageTimer Now;
	return Now - *this;
}

float RageTimer::GetDeltaTime()
{
	const RageTimer Now;
	const float diff = Difference( Now, *this );
	*this = Now;
	return diff;
}

/* Get a timer representing half of the time ago as this one.  This is
 * useful for averaging time.  For example,
 * 
 * RageTimer tm;
 * ... do stuff ...
 * RageTimer AverageTime = tm.Half();
 * printf("Something happened between now and tm; the average time is %f.\n", tm.Ago());
 * tm.Touch();
 */
RageTimer RageTimer::Half() const
{
	const RageTimer now;
	const float ProbableDelay = -(now - *this) / 2;
	return *this + ProbableDelay;
}


RageTimer RageTimer::operator+(float tm) const
{
	return Sum(*this, tm);
}

float RageTimer::operator-(const RageTimer &rhs) const
{
	return Difference(*this, rhs);
}

RageTimer RageTimer::Sum(const RageTimer &lhs, float tm)
{
	/* tm == 5.25  -> secs =  5, us = 5.25  - ( 5) = .25
	 * tm == -1.25 -> secs = -2, us = -1.25 - (-2) = .75 */
	int seconds = (int) floorf(tm);
	int us = int( (tm - seconds) * TIMESTAMP_RESOLUTION );

	RageTimer ret(0,0); // Prevent unnecessarily checking the time
	ret.m_secs = seconds + lhs.m_secs;
	ret.m_us = us + lhs.m_us;

	if( ret.m_us >= TIMESTAMP_RESOLUTION )
	{
		ret.m_us -= TIMESTAMP_RESOLUTION;
		++ret.m_secs;
	}

	return ret;
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
 * Copyright (c) 2001-2003 Chris Danford, Glenn Maynard
 * All rights reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, and/or sell copies of the Software, and to permit persons to
 * whom the Software is furnished to do so, provided that the above
 * copyright notice(s) and this permission notice appear in all copies of
 * the Software and that both the above copyright notice(s) and this
 * permission notice appear in supporting documentation.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT OF
 * THIRD PARTY RIGHTS. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR HOLDERS
 * INCLUDED IN THIS NOTICE BE LIABLE FOR ANY CLAIM, OR ANY SPECIAL INDIRECT
 * OR CONSEQUENTIAL DAMAGES, OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS
 * OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
 * OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */

