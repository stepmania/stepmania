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

const int DAYS_PER_YEAR = 365;
const int HOURS_PER_DAY = 24;
const int DAYS_IN_WEEK = 7;

class Bookkeeper
{
public:
	Bookkeeper();
	~Bookkeeper();

	void CoinInserted();
	void UpdateLastSeenTime();

	void GetCoinsLast7Days( int coins[7] );
	void GetCoinsLast52Weeks( int coins[52] );
	void GetCoinsByDayOfWeek( int coins[DAYS_IN_WEEK] );
	void GetCoinsByHour( int coins[HOURS_PER_DAY] );

	void ReadFromDisk();
	void WriteToDisk();

private:

	int GetCoinsForDay( int iDayOfYear );

	int m_iLastSeenTime;
	int m_iTotalCoins;
	int m_iTotalUptimeSeconds;
	int m_iTotalPlaySeconds;
	int m_iTotalPlays;
	int m_iCoinsByHourForYear[DAYS_PER_YEAR][HOURS_PER_DAY];
};


extern Bookkeeper*	BOOKKEEPER;	// global and accessable from anywhere in our program

#endif
