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


const int NUM_LAST_DAYS = 7;
const int NUM_LAST_WEEKS = 52;
const int DAYS_IN_YEAR = 365;
const int HOURS_IN_DAY = 24;
const int DAYS_IN_WEEK = 7;

CString DayInYearToString( int iDayInYearIndex );
CString LastDayToString( int iLastDayIndex );
CString DayOfWeekToString( int iDayOfWeekIndex );
CString HourInDayToString( int iHourIndex );


#endif
