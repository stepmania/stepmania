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

const int NUM_LAST_DAYS = 7;
const int NUM_LAST_WEEKS = 52;
const int DAYS_PER_YEAR = 365;
const int HOURS_PER_DAY = 24;
const int DAYS_IN_WEEK = 7;

const CString LAST_DAYS_NAME[NUM_LAST_DAYS] =
{
	"Yesterday",
	"2 Days Ago",
	"3 Days Ago",
	"4 Days Ago",
	"5 Days Ago",
	"6 Days Ago",
	"7 Days Ago",
};

const CString DAY_OF_WEEK_TO_NAME[DAYS_IN_WEEK] =
{
	"Sunday",
	"Monday",
	"Tuesday",
	"Wednesday",
	"Thursday",
	"Friday",
	"Saturday",
};

CString HourToString( int iHourIndex );


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
	void GetCoinsByHour( int coins[HOURS_PER_DAY] );

	void ReadFromDisk();
	void WriteToDisk();

private:

	int GetCoinsForDay( int iDayOfYear );

	int m_iLastSeenTime;
	int m_iCoinsByHourForYear[DAYS_PER_YEAR][HOURS_PER_DAY];
};


extern Bookkeeper*	BOOKKEEPER;	// global and accessable from anywhere in our program

#endif
