#ifndef RAGE_TIMER_H
#define RAGE_TIMER_H

/* Timer services. */

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
class RageTimer
{
public:
	RageTimer() { Touch(); }

	/* Time ago this RageTimer represents. */
	float Ago() const;
	inline void Touch();
	inline bool IsZero() const { return m_secs == 0 && m_us == 0; }
	inline void SetZero() { m_secs = m_us = 0; }

	/* Time between last call to GetDeltaTime() (Ago() + Touch()): */
	float GetDeltaTime();
	/* (alias) */
	float PeekDeltaTime() const { return Ago(); }

	/* deprecated: */
	static float GetTimeSinceStart();	// seconds since the program was started

	/* Add (or subtract) a duration from a timestamp.  The result is another timestamp. */
	RageTimer operator+(float tm) const;

	/* Find the amount of time between two timestamps.  The result is a duration. */
	float operator-(const RageTimer &rhs) const;

private:
	static RageTimer Sum(const RageTimer &lhs, float tm);
	static float Difference(const RageTimer &lhs, const RageTimer &rhs);

	/* "float" is bad for a "time since start" RageTimer.  If the game is running for
	 * several days, we'll lose a lot of resolution.  I don't want to use double
	 * everywhere, since it's slow.  I'd rather not use double just for RageTimers, since
	 * it's too easy to get a type wrong and end up with obscure resolution problems. */
	unsigned m_secs, m_us;
};

#endif

/*
 * Copyright (c) 2001-2003 by the person(s) listed below.  All rights reserved.
 *	Chris Danford
 *	Glenn Maynard
 */
