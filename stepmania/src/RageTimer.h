/* RageTimer - Timer services. */

#ifndef RAGE_TIMER_H
#define RAGE_TIMER_H

class RageTimer
{
public:
	RageTimer() { Touch(); }
	RageTimer( int secs, int us ): m_secs(secs), m_us(us) { }

	/* Time ago this RageTimer represents. */
	float Ago() const;
	void Touch();
	inline bool IsZero() const { return m_secs == 0 && m_us == 0; }
	inline void SetZero() { m_secs = m_us = 0; }

	/* Time between last call to GetDeltaTime() (Ago() + Touch()): */
	float GetDeltaTime();
	/* (alias) */
	float PeekDeltaTime() const { return Ago(); }

	/* deprecated: */
	static float GetTimeSinceStart( bool bAccurate = true );	// seconds since the program was started
	static float GetTimeSinceStartFast() { return GetTimeSinceStart(false); }

	/* Get a timer representing half of the time ago as this one. */
	RageTimer Half() const;

	/* Add (or subtract) a duration from a timestamp.  The result is another timestamp. */
	RageTimer operator+( float tm ) const;
	void operator+=( float tm ) { *this = *this + tm; }

	/* Find the amount of time between two timestamps.  The result is a duration. */
	float operator-( const RageTimer &rhs ) const;

	/* "float" is bad for a "time since start" RageTimer.  If the game is running for
	 * several days, we'll lose a lot of resolution.  I don't want to use double
	 * everywhere, since it's slow.  I'd rather not use double just for RageTimers, since
	 * it's too easy to get a type wrong and end up with obscure resolution problems. */
	unsigned m_secs, m_us;

private:
	static RageTimer Sum( const RageTimer &lhs, float tm );
	static float Difference( const RageTimer &lhs, const RageTimer &rhs );
};

extern const RageTimer RageZeroTimer;

#endif

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

