#include "global.h"
#include "DateTime.h"
#include "RageUtil.h"


DateTime::DateTime()
{
	Init();
}

void DateTime::Init()
{
	ZERO( *this );
}

DateTime DateTime::GetNowDateTime()
{
	time_t now = time(NULL);
    tm tNow;
	localtime_r( &now, &tNow );
	DateTime dtNow;
#define COPY_M( v ) dtNow.v = tNow.v;
	COPY_M( tm_year );
	COPY_M( tm_mon );
	COPY_M( tm_mday );
	COPY_M( tm_hour );
	COPY_M( tm_min );
	COPY_M( tm_sec );
#undef COPY_M
	return dtNow;
}

DateTime DateTime::GetNowDate()
{
    DateTime tNow = GetNowDateTime();
	tNow.StripTime();
	return tNow;
}

void DateTime::StripTime()
{
	tm_hour = 0;
	tm_min = 0;
	tm_sec = 0;
}

//
// Common SQL/XML format: "YYYY-MM-DD HH:MM:SS"
//
CString DateTime::GetString() const
{
	CString s = ssprintf( "%d-%02d-%02d",
		tm_year+1900,
		tm_mon+1,
		tm_mday );
	
	if( tm_hour != 0 || 
		tm_min != 0 ||
		tm_sec != 0 )
	{
		s += ssprintf( " %02d:%02d:%02d",
			tm_hour,
			tm_min,
			tm_sec );
	}

	return s;
}

bool DateTime::FromString( const CString sDateTime )
{
	Init();

	int ret;

	ret = sscanf( sDateTime, "%d-%d-%d %d:%d:%d", 
		&tm_year,
		&tm_mon,
		&tm_mday,
		&tm_hour,
		&tm_min,
		&tm_sec );
	if( ret == 6 )
		goto success;

	ret = sscanf( sDateTime, "%d-%d-%d", 
		&tm_year,
		&tm_mon,
		&tm_mday );
	if( ret == 3 )
		goto success;

	return false;

success:
	tm_year -= 1900;
	tm_mon -= 1;
	return true;
}



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
	"Today",
	"Yesterday",
	"Day2Ago",
	"Day3Ago",
	"Day4Ago",
	"Day5Ago",
	"Day6Ago",
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
	return ssprintf("Hour%02d", iHourInDayIndex);
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
	case 0:		return "ThisWeek";	break;
	case 1:		return "LastWeek";	break;
	default:	return ssprintf("Week%02dAgo",iLastWeekIndex);	break;
	}
}

CString LastDayToDisplayString( int iLastDayIndex )
{
	CString s = LastDayToString( iLastDayIndex );
	s.Replace( "Day", "" );
	s.Replace( "Ago", " Ago" );
	return s;
}

CString LastWeekToDisplayString( int iLastWeekIndex )
{
	CString s = LastWeekToString( iLastWeekIndex );
	s.Replace( "Week", "" );
	s.Replace( "Ago", " Ago" );
	return s;
}

CString HourInDayToDisplayString( int iHourIndex )
{
	int iBeginHour = iHourIndex;
	iBeginHour--;
	wrap( iBeginHour, 24 );
	iBeginHour++;

	return ssprintf("%02d:00+", iBeginHour );
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
	/* If iDayInYearIndex is 200, set the date to Jan 200th, and let mktime
	 * round it.  This shouldn't suffer from the OSX mktime() issue described
	 * above, since we're not giving it negative values. */
	tm when;
	ZERO( when );
	when.tm_mon = 0;
	when.tm_mday = iDayInYearIndex+1;
	when.tm_year = iYear - 1900;
	time_t then = mktime( &when );

	localtime_r( &then, &when );
	return when;
}


/*
 * (c) 2001-2004 Chris Danford
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
