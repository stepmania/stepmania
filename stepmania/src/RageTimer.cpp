#include "global.h"

#include "RageTimer.h"
#include "RageLog.h"

#include "SDL.h"
#include "SDL_timer.h"
 
/* We only actually get 1000 using SDL. */
#define TIMESTAMP_RESOLUTION 1000000

const RageTimer RageZeroTimer(0,0);

void mySDL_GetTicks( unsigned &secs, unsigned &us )
{
	static bool bInitialized = false;
	if( !bInitialized )
	{
		bInitialized = true;
		/* We don't want the timer system; it starts a thread, and we never use it. */
		// if( !SDL_WasInit(SDL_INIT_TIMER) )
		//	SDL_InitSubSystem( SDL_INIT_TIMER );

		/* Calling this will still initialize the hidden "ticks" system, so we can
		 * use SDL_GetTicks. */
		SDL_InitSubSystem( 0 );
	}

	/* Ticks may be less than last for at least two reasons: the time may have wrapped (after
	 * about 49 days), or the system clock may have moved backwards.  If the system clock moves
	 * backwards, we can't just clamp the time; if it moved back an hour, we'll sit around for
	 * an hour until it catches up.
	 *
	 * Keep track of an offset: the amount of time to add to the result of SDL_GetTicks.
	 * If we move back by 100ms, the offset will be increased by 100ms.  If we loop, the
	 * offset will be increased by the duration 2^32 ticks.  This is stored in the same
	 * notation as RageTimer: one 32-bit int for seconds and another for microseconds, so
	 * wrapping isn't a problem.
	 */

	static uint32_t last = 0;
	static uint32_t offset_secs = 0, offset_us = 0;

	const uint32_t millisecs = SDL_GetTicks();

	/* The time has wrapped if the last time was very high and the current time is very low. */
	const uint32_t one_day = 24*60*60*1000;
	if( last > (0-one_day) && millisecs < one_day )
	{
		const uint32_t wraparound_secs = 4294967; /* (2^32 / 1000) */
		const uint32_t wraparound_us = 296000;    /* (2^32 % 1000) * 1000 */
		offset_secs += wraparound_secs;
		offset_us += wraparound_us;
	}
	else if( millisecs < last )
	{
		/* The time has moved backwards.  Increase the offset by the amount we moved. */
		const uint32_t offset_ms = last - millisecs;
		offset_secs +=  offset_ms/1000;
		offset_us   += (offset_ms%1000) * 1000;
	}

	if( offset_us >= TIMESTAMP_RESOLUTION )
	{
		offset_us -= TIMESTAMP_RESOLUTION;
		++offset_secs;
	}

	last = millisecs;

	secs = millisecs / 1000;
	us =  (millisecs % 1000)*1000;

	secs += offset_secs;
	us += offset_us;

	if( us >= TIMESTAMP_RESOLUTION )
	{
		us -= TIMESTAMP_RESOLUTION;
		++secs;
	}
}

float RageTimer::GetTimeSinceStart()
{
	unsigned secs, us;
	mySDL_GetTicks( secs, us );
	return secs + us / 1000000.0f;
}

void RageTimer::Touch()
{
	mySDL_GetTicks( this->m_secs, this->m_us );
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

	RageTimer ret;
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

