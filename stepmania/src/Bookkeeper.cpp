#include "global.h"
#include "Bookkeeper.h"
#include "RageUtil.h"
#include "arch/arch.h"
#include "PrefsManager.h"
#include "RageLog.h"
#include "IniFile.h"
#include "GameConstantsAndTypes.h"
#include "SongManager.h"
#include "RageFile.h"
#include <ctime>


Bookkeeper*	BOOKKEEPER = NULL;	// global and accessable from anywhere in our program

static const CString COINS_DAT = "Data/Coins.dat";

const int COINS_DAT_VERSION = 1;

Bookkeeper::Bookkeeper()
{
	ClearAll();

	ReadFromDisk();

	UpdateLastSeenTime();
}

Bookkeeper::~Bookkeeper()
{
	WriteToDisk();
}

#define WARN_AND_RETURN { LOG->Warn("Error parsing at %s:%d",__FILE__,__LINE__); return; }

void Bookkeeper::ClearAll()
{
	m_iLastSeenTime = time(NULL);
	for( int i=0; i<DAYS_IN_YEAR; i++ )
		for( int j=0; j<HOURS_IN_DAY; j++ )
			m_iCoinsByHourForYear[i][j] = 0;
}

void Bookkeeper::ReadFromDisk()
{
    RageFile f;
	if( !f.Open(COINS_DAT, RageFile::READ) )
	{
		LOG->Trace( "Couldn't open file \"%s\": %s", COINS_DAT.c_str(), f.GetError().c_str() );
		return;
	}

	int iVer;
	if( !FileRead(f, iVer) )
		WARN_AND_RETURN;
	if( iVer != COINS_DAT_VERSION )
		WARN_AND_RETURN;

	if( !FileRead(f, m_iLastSeenTime) )
		WARN_AND_RETURN;

    for (int i=0; i<DAYS_IN_YEAR; ++i)
	{
        for (int j=0; j<HOURS_IN_DAY; ++j)
		{
			int iCoins;
			if( !FileRead(f, iCoins) )
				WARN_AND_RETURN;

            m_iCoinsByHourForYear[i][j] = iCoins;
		}
	}
}

void Bookkeeper::WriteToDisk()
{
	// write dat
    RageFile f;
	if( !f.Open(COINS_DAT, RageFile::WRITE) )
	{
		LOG->Warn( "Couldn't open file \"%s\" for writing: %s", COINS_DAT.c_str(), f.GetError().c_str() );
		return;
	}
    
	FileWrite(f, COINS_DAT_VERSION );
	
	FileWrite(f, m_iLastSeenTime);

	for (int i=0; i<DAYS_IN_YEAR; ++i)
        for (int j=0; j<HOURS_IN_DAY; ++j)
            FileWrite( f, m_iCoinsByHourForYear[i][j] );
}

void Bookkeeper::UpdateLastSeenTime()
{
	// clear all coin counts from (lOldTime,lNewTime]

	time_t lOldTime = m_iLastSeenTime;
	time_t lNewTime = time(NULL);

	if( lNewTime < lOldTime )
	{
		LOG->Warn( "The new time is older than the last seen time.  Is someone fiddling with the system clock?" );
		m_iLastSeenTime = lNewTime;
		return;
	}

    tm tOld, tNew;
	localtime_r( &lOldTime, &tOld );
    localtime_r( &lNewTime, &tNew );

	CLAMP( tOld.tm_year, tNew.tm_year-1, tNew.tm_year );

	while( 
		tOld.tm_year != tNew.tm_year ||
		tOld.tm_yday != tNew.tm_yday ||
		tOld.tm_hour != tNew.tm_hour )
	{
		tOld.tm_hour++;
		if( tOld.tm_hour == HOURS_IN_DAY )
		{
			tOld.tm_hour = 0;
			tOld.tm_yday++;
		}
		if( tOld.tm_yday == DAYS_IN_YEAR )
		{
			tOld.tm_yday = 0;
			tOld.tm_year++;
		}

		m_iCoinsByHourForYear[tOld.tm_yday][tOld.tm_hour] = 0;
	}

	m_iLastSeenTime = lNewTime;
}

void Bookkeeper::CoinInserted()
{
	UpdateLastSeenTime();

	time_t lTime = m_iLastSeenTime;
    tm pTime;
	localtime_r( &lTime, &pTime );

	m_iCoinsByHourForYear[pTime.tm_yday][pTime.tm_hour]++;
}

int Bookkeeper::GetCoinsForDay( int iDayOfYear )
{
	int iCoins = 0;
	for( int i=0; i<HOURS_IN_DAY; i++ )
		iCoins += m_iCoinsByHourForYear[iDayOfYear][i];
	return iCoins;
}


void Bookkeeper::GetCoinsLastDays( int coins[NUM_LAST_DAYS] )
{
	UpdateLastSeenTime();

	time_t lOldTime = m_iLastSeenTime;
    tm time;
	localtime_r( &lOldTime, &time );

	for( int i=0; i<NUM_LAST_DAYS; i++ )
	{
		coins[i] = GetCoinsForDay( time.tm_yday );
		time = GetYesterday( time );
	}
}


void Bookkeeper::GetCoinsLastWeeks( int coins[NUM_LAST_WEEKS] )
{
	UpdateLastSeenTime();

	time_t lOldTime = m_iLastSeenTime;
    tm time;
	localtime_r( &lOldTime, &time );

	time = GetNextSunday( time );
	time = GetYesterday( time );

	for( int w=0; w<NUM_LAST_WEEKS; w++ )
	{
		coins[w] = 0;

		for( int d=0; d<DAYS_IN_WEEK; d++ )
		{
			coins[w] += GetCoinsForDay( time.tm_yday );
			time = GetYesterday( time );
		}
	}
}

void Bookkeeper::GetCoinsByDayOfWeek( int coins[DAYS_IN_WEEK] )
{
	UpdateLastSeenTime();

	for( int i=0; i<DAYS_IN_WEEK; i++ )
		coins[i] = 0;

	time_t lOldTime = m_iLastSeenTime;
    tm time;
	localtime_r( &lOldTime, &time );

	for( int d=0; d<DAYS_IN_YEAR; d++ )
	{
		coins[GetDayOfWeek(time)] += GetCoinsForDay( time.tm_yday );
		time = GetYesterday( time );
	}
}

void Bookkeeper::GetCoinsByHour( int coins[HOURS_IN_DAY] )
{
	UpdateLastSeenTime();

	for( int h=0; h<HOURS_IN_DAY; h++ )
	{
		coins[h] = 0;

		for( int d=0; d<DAYS_IN_YEAR; d++ )
			coins[h] += m_iCoinsByHourForYear[d][h];
	}
}

/*
 * (c) 2003-2004 Chris Danford
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
