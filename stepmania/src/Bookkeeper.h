#ifndef Bookkeeper_H
#define Bookkeeper_H
/*
-----------------------------------------------------------------------------
 Class: Bookkeeper

 Desc: Interface to profiles that exist on the machine or a memory card
	plugged into the machine.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "Style.h"
#include "TimeConstants.h"


class Bookkeeper
{
public:
	Bookkeeper();
	~Bookkeeper();

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
