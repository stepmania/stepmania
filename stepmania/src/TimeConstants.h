#ifndef TimeConstants_H
#define TimeConstants_H
/*
-----------------------------------------------------------------------------
 Class: TimeConstants

 Desc: 

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include <ctime>


const int NUM_LAST_DAYS = 7;
const int NUM_LAST_WEEKS = 52;
const int DAYS_IN_YEAR = 365;
const int HOURS_IN_DAY = 24;
const int DAYS_IN_WEEK = 7;
const int MONTHS_IN_YEAR = 12;

CString DayInYearToString( int iDayInYearIndex );
CString LastDayToString( int iLastDayIndex );
CString DayOfWeekToString( int iDayOfWeekIndex );
CString HourInDayToString( int iHourIndex );
CString MonthToString( int iMonthIndex );
CString LastWeekToString( int iLastWeekIndex );

tm AddDays( tm start, int iDaysToMove );
tm GetYesterday( tm start );
int GetDayOfWeek( tm time );
tm GetNextSunday( tm start );

tm GetDayInYearAndYear( int iDayInYearIndex, int iYear );


#endif
