/* Time helpers. */

#ifndef TimeConstants_H
#define TimeConstants_H

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

/*
 * (c) 2004 Chris Danford
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
