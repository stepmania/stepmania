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

