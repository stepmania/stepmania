#include "global.h"
/*
-----------------------------------------------------------------------------
 Class: TimeConstants

 Desc: 

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "TimeConstants.h"
#include "RageUtil.h"

CString DayInYearToString( int iDayInYear )
{
	return ssprintf("DayInYear%03d",iDayInYear);
}

int StringToDayInYear( CString sDayInYear )
{
	int iDayInYear;
	if( sscanf( sDayInYear, "DayInYear%d", &iDayInYear ) != 1 )
		return -1;
	return iDayInYear;
}

static const CString LAST_DAYS_NAME[NUM_LAST_DAYS] =
{
	"Yesterday",
	"2 Days Ago",
	"3 Days Ago",
	"4 Days Ago",
	"5 Days Ago",
	"6 Days Ago",
	"7 Days Ago",
};

CString LastDayToString( int iLastDayIndex )
{
	return LAST_DAYS_NAME[iLastDayIndex];
}

static const CString DAY_OF_WEEK_TO_NAME[DAYS_IN_WEEK] =
{
	"Sunday",
	"Monday",
	"Tuesday",
	"Wednesday",
	"Thursday",
	"Friday",
	"Saturday",
};

CString DayOfWeekToString( int iDayOfWeekIndex )
{
	return DAY_OF_WEEK_TO_NAME[iDayOfWeekIndex];
}

CString HourInDayToString( int iHourInDayIndex )
{
	return ssprintf("%02d:00", iHourInDayIndex);
}

static const CString MONTH_TO_NAME[MONTHS_IN_YEAR] =
{
	"January",
	"February",
	"March",
	"April",
	"May",
	"June",
	"July",
	"August",
	"September",
	"October",
	"November",
	"December",
};

CString MonthToString( int iMonthIndex )
{
	return MONTH_TO_NAME[iMonthIndex];
}

CString LastWeekToString( int iLastWeekIndex )
{
	switch( iLastWeekIndex )
	{
	case 0:		return "This week";	break;
	case 1:		return "Last week";	break;
	default:	return ssprintf("%d weeks ago",iLastWeekIndex);	break;
	}
}

tm AddDays( tm start, int iDaysToMove )
{
	/*
	 * This causes problems on OS X, which doesn't correctly handle range that are below
	 * their normal values (eg. mday = 0).  According to the manpage, it should adjust them:
	 *
	 * "If structure members are outside their legal interval, they will be normalized (so
	 * that, e.g., 40 October is changed into 9 November)."
	 *
	 * Instead, it appears to simply fail.
	 *
	 * Refs:
	 *  http://bugs.php.net/bug.php?id=10686
	 *  http://sourceforge.net/tracker/download.php?group_id=37892&atid=421366&file_id=79179&aid=91133
	 *
	 * Note "Log starting 2004-03-07 03:50:42"; mday is 7, and PrintCaloriesBurned calls us
	 * with iDaysToMove = -7, resulting in an out-of-range value 0.  This seems legal, but
	 * OS X chokes on it.
	 */
/*	start.tm_mday += iDaysToMove;
	time_t seconds = mktime( &start );
	ASSERT( seconds != (time_t)-1 );
	*/

	/* This handles DST differently: it returns the time that was exactly n*60*60*24 seconds
	 * ago, where the above code always returns the same time of day.  I prefer the above
	 * behavior, but I'm not sure that it mattersmatters. */
	time_t seconds = mktime( &start );
	seconds += iDaysToMove*60*60*24;

	tm time;
	localtime_r( &seconds, &time );
	return time;
}

tm GetYesterday( tm start )
{
	return AddDays( start, -1 );
}

int GetDayOfWeek( tm time )
{
	int iDayOfWeek = time.tm_wday;
	ASSERT( iDayOfWeek < DAYS_IN_WEEK );
	return iDayOfWeek;
}

tm GetNextSunday( tm start )
{
	return AddDays( start, DAYS_IN_WEEK-GetDayOfWeek(start) );
}


tm GetDayInYearAndYear( int iDayInYearIndex, int iYear )
{
	tm when = GetLocalTime();

	when.tm_mday = iDayInYearIndex;
	when.tm_year = iYear - 1900;
	time_t then = mktime( &when );

	when = *localtime( &then );
	return when;
}
