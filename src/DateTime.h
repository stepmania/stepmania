#ifndef DATE_TIME_H
#define DATE_TIME_H

#include "EnumHelper.h"
#include <ctime>

int StringToDayInYear( RString sDayInYear );

/** @brief The number of days we check for previously. */
const int NUM_LAST_DAYS = 7;
/** @brief The number of weeks we check for previously. */
const int NUM_LAST_WEEKS = 52;
/**
 * @brief The number of days that are in a year.
 *
 * This is set up to be a maximum for leap years. */
const int DAYS_IN_YEAR = 366;
/**
 * @brief The number of hours in a day. */
const int HOURS_IN_DAY = 24;
/**
 * @brief The number of days that are in a week. */
const int DAYS_IN_WEEK = 7;
/** @brief Which month are we focusing on?
 *
 * Is there any reason why the actual months aren't defined 
 * in here? -Wolfman2000 */
enum Month 
{ 
	NUM_Month = 12, /**< The number of months in the year. */
	Month_Invalid /**< There should be no month at this point. */
};

RString DayInYearToString( int iDayInYearIndex );
RString LastDayToString( int iLastDayIndex );
RString LastDayToLocalizedString( int iLastDayIndex );
RString DayOfWeekToString( int iDayOfWeekIndex );
RString DayOfWeekToLocalizedString( int iDayOfWeekIndex );
RString HourInDayToString( int iHourIndex );
RString HourInDayToLocalizedString( int iHourIndex );
const RString &MonthToString( Month month );
const RString &MonthToLocalizedString( Month month );
RString LastWeekToString( int iLastWeekIndex );
RString LastWeekToLocalizedString( int iLastWeekIndex );
LuaDeclareType( Month );

tm AddDays( tm start, int iDaysToMove );
tm GetYesterday( tm start );
int GetDayOfWeek( tm time );
tm GetNextSunday( tm start );

tm GetDayInYearAndYear( int iDayInYearIndex, int iYear );

/** @brief A standard way of determining the date and the time. */
struct DateTime 
{
	/**
	 * @brief The number of seconds after the minute.
	 *
	 * Valid values are [0, 59]. */
	int tm_sec;
	/**
	 * @brief The number of minutes after the hour.
	 *
	 * Valid values are [0, 59]. */
	int tm_min;
	/**
	 * @brief The number of hours since midnight (or 0000 hours).
	 *
	 * Valid values are [0, 23]. */
	int tm_hour;
	/**
	 * @brief The specified day of the current month.
	 *
	 * Valid values are [1, 31].
	 *
	 * XXX: Is it possible to set an illegal date through here,
	 * such as day 30 of February? -Wolfman2000 */
	int tm_mday;
	/**
	 * @brief The number of months since January.
	 *
	 * Valid values are [0, 11]. */
	int tm_mon;
	/** @brief The number of years since the year 1900. */
	int tm_year;

	/** @brief Set up a default date and time. */
	DateTime();
	/** @brief Initialize the date and time. */
	void Init();

	/**
	 * @brief Determine if this DateTime is less than some other time.
	 * @param other the other DateTime to check.
	 * @return true if this is less than the other time, or false otherwise. */
	bool operator<( const DateTime& other ) const;
	/**
	 * @brief Determine if this DateTime is greater than some other time.
	 * @param other the other DateTime to check.
	 * @return true if this is greater than the other time, or false otherwise. */
	bool operator>( const DateTime& other ) const;
	/**
	 * @brief Determine if this DateTime is equal to some other time.
	 * @param other the other DateTime to check.
	 * @return true if this is equal to the other time, or false otherwise. */
	bool operator==( const DateTime& other ) const;
	/**
	 * @brief Determine if this DateTime is not equal to some other time.
	 * @param other the other DateTime to check.
	 * @return true if this is not equal to the other time, or false otherwise. */
	bool operator!=( const DateTime& other ) const { return !operator==(other); }
	/**
	 * @brief Determine if this DateTime is less than or equal to some other time.
	 * @param other the other DateTime to check.
	 * @return true if this is less than or equal to the other time, or false otherwise. */
	bool operator<=( const DateTime& other ) const { return !operator>(other); }
	
	/**
	 * @brief Determine if this DateTime is greater than or equal to some other time.
	 * @param other the other DateTime to check.
	 * @return true if this is greater than or equal to the other time, or false otherwise. */
	bool operator>=( const DateTime& other ) const { return !operator<(other); }

	/** 
	 * @brief Retrieve the current date and time.
	 * @return the current date and time. */
	static DateTime GetNowDateTime();
	/**
	 * @brief Retrieve the current date.
	 * @return the current date. */
	static DateTime GetNowDate();

	/** @brief Remove the time portion from the date. */
	void StripTime();

	/**
	 * @brief Retrieve a string representation of the current date and time.
	 *
	 * This returns a common SQL/XML format: "YYYY-MM-DD HH:MM:SS".
	 * @return the string representation of the date and time. */
	RString GetString() const;
	/**
	 * @brief Attempt to turn a string into a DateTime.
	 *
	 * @param sDateTime the string to attempt to convert.
	 * @return true if the conversion worked, or false otherwise. */
	bool FromString( const RString sDateTime );
};

#endif

/**
 * @file
 * @author Chris Danford (c) 2001-2004
 * @section LICENSE
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
