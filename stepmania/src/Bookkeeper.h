/* Bookkeeper - coin tracking. */

#ifndef Bookkeeper_H
#define Bookkeeper_H

#include "DateTime.h"


class Bookkeeper
{
public:
	Bookkeeper();
	~Bookkeeper();

	void ClearAll();

	void CoinInserted();
	void UpdateLastSeenTime();

	void GetCoinsLastDays( int coins[NUM_LAST_DAYS] );
	void GetCoinsLastWeeks( int coins[NUM_LAST_WEEKS] );
	void GetCoinsByDayOfWeek( int coins[DAYS_IN_WEEK] );
	void GetCoinsByHour( int coins[HOURS_IN_DAY] );

	void ReadFromDisk();
	void WriteToDisk();

private:

	int GetCoinsForDay( int iDayOfYear );

	int m_iLastSeenTime;
	int m_iCoinsByHourForYear[DAYS_IN_YEAR][HOURS_IN_DAY];
};


extern Bookkeeper*	BOOKKEEPER;	// global and accessable from anywhere in our program

#endif

/*
 * (c) 2003-2004 Chris Danford
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
