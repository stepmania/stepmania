#ifndef Bookkeeper_H
#define Bookkeeper_H

#include "DateTime.h"
#include <map>
class XNode;

/** @brief Track when coins were put into the machine. */
class Bookkeeper
{
public:
	Bookkeeper();
	~Bookkeeper();

	void ClearAll();

	void CoinInserted();

	int GetCoinsTotal() const;
	void GetCoinsLastDays( int coins[NUM_LAST_DAYS] ) const;
	void GetCoinsLastWeeks( int coins[NUM_LAST_WEEKS] ) const;
	void GetCoinsByDayOfWeek( int coins[DAYS_IN_WEEK] ) const;
	void GetCoinsByHour( int coins[HOURS_IN_DAY] ) const;
	void WriteCoinsFile( int coins ) ;
	void ReadCoinsFile( int &coins ) ;


	void LoadFromNode( const XNode *pNode );
	XNode* CreateNode() const;

	void ReadFromDisk();
	void WriteToDisk();

private:
	/** @brief A simple way of handling the date. */
	struct Date
	{
		/**
		 * @brief The hour of the date.
		 *
		 * The value 0 is defined to be midnight. */
		int m_iHour;
		/**
		 * @brief The day of the year of the date.
		 *
		 * The value 0 is defined to be January 1st. */
		int m_iDayOfYear;
		/** @brief The year of the date (e.g., 2005). */
		int m_iYear;
		/** @brief Set up a date with initial values. */
		Date() { m_iHour = m_iDayOfYear = m_iYear = 0; }
		/**
		 * @brief Set up a date based on the given time.
		 * @param time the time to turn into a Date. */
		Date( tm time ) { Set(time); }
		void Set( time_t t );
		void Set( tm pTime );
		bool operator<( const Date &rhs ) const;
	};
	int GetNumCoins( Date beginning, Date ending ) const;
	int GetNumCoinsInRange( map<Date,int>::const_iterator begin, map<Date,int>::const_iterator end ) const;

	int m_iLastSeenTime;
	map<Date,int> m_mapCoinsForHour;
};

extern Bookkeeper*	BOOKKEEPER;	// global and accessible from anywhere in our program

#endif

/**
 * @file
 * @author Chris Danford (c) 2003-2004
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
