#include "global.h"
/*
-----------------------------------------------------------------------------
 Class: Bookkeeper

 Desc: See header.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "Bookkeeper.h"
#include "RageUtil.h"
#include "arch/arch.h"
#include "PrefsManager.h"
#include "RageLog.h"
#include "IniFile.h"
#include "GameConstantsAndTypes.h"
#include "SongManager.h"
#include "RageFile.h"
#include <time.h>


Bookkeeper*	BOOKKEEPER = NULL;	// global and accessable from anywhere in our program

static const CString BOOKKEEPING_INI = BASE_PATH "Data" SLASH "Bookkeeping.ini";
static const CString COINS_DAT = BASE_PATH "Data" SLASH "Coins.dat";



tm GetDaysAgo( tm start, int iDaysAgo )
{
	start.tm_mday -= iDaysAgo;
	time_t seconds = mktime( &start );
	ASSERT( seconds != (time_t)-1 );
	tm time = *localtime( &seconds );
	return time;
}

tm GetYesterday( tm start )
{
	return GetDaysAgo( start, -1 );
}

int GetDayOfWeek( tm time )
{
	return time.tm_wday;
}

tm GetLastSunday( tm start )
{
	return GetDaysAgo( start, -GetDayOfWeek(start) );
}



Bookkeeper::Bookkeeper()
{
	int i, j;

	m_iLastSeenTime = time(NULL);
	m_iTotalCoins = 0;
	m_iTotalUptimeSeconds = 0;
	m_iTotalPlaySeconds = 0;
	m_iTotalPlays = 0;
	for( i=0; i<DAYS_PER_YEAR; i++ )
		for( j=0; j<HOURS_PER_DAY; j++ )
			m_iCoinsByHourForYear[i][j] = 0;

	ReadFromDisk();

	UpdateLastSeenTime();
}

Bookkeeper::~Bookkeeper()
{
	WriteToDisk();
}

void Bookkeeper::ReadFromDisk()
{
	// read ini
	IniFile ini;
	ini.SetPath( BOOKKEEPING_INI );
	ini.ReadFile();

	ini.GetValue( "MachineStatistics", "LastSeenTime",			m_iLastSeenTime ); 
	ini.GetValue( "MachineStatistics", "TotalCoins",			m_iTotalCoins ); 
	ini.GetValue( "MachineStatistics", "TotalUptimeSeconds",	m_iTotalUptimeSeconds ); 
	ini.GetValue( "MachineStatistics", "TotalPlaySeconds",		m_iTotalPlaySeconds ); 
	ini.GetValue( "MachineStatistics", "TotalPlays",			m_iTotalPlays ); 

	// read dat
    RageFile file(COINS_DAT, "r");
    if (file.IsOpen())
    {
		const CString line = file.GetLine();

		vector<CString> parts;
		split( line, " ", parts, true );

		unsigned p = 0;
        for (int i=0; i<DAYS_PER_YEAR; ++i)
            for (int j=0; j<HOURS_PER_DAY; ++j)
			{
				if( p >= parts.size() )
				{
					LOG->Warn( "Parse error in %s", COINS_DAT.c_str() );
					return;
				}

                m_iCoinsByHourForYear[i][j] = atoi( parts[p++] );
			}
    }
}

void Bookkeeper::WriteToDisk()
{
	// write ini
	IniFile ini;
	ini.SetPath( BOOKKEEPING_INI );
	
	ini.SetValue( "MachineStatistics", "LastSeenTime",			m_iLastSeenTime ); 
	ini.SetValue( "MachineStatistics", "TotalCoins",			m_iTotalCoins ); 
	ini.SetValue( "MachineStatistics", "TotalUptimeSeconds",	m_iTotalUptimeSeconds ); 
	ini.SetValue( "MachineStatistics", "TotalPlaySeconds",		m_iTotalPlaySeconds ); 
	ini.SetValue( "MachineStatistics", "TotalPlays",			m_iTotalPlays ); 

	ini.WriteFile();

	// write dat
    RageFile file(COINS_DAT, "w");
    
    if (file.IsOpen())
    {
		CString line;
        for (int i=0; i<DAYS_PER_YEAR; ++i)
            for (int j=0; j<HOURS_PER_DAY; ++j)
                line += ssprintf( "%d ", m_iCoinsByHourForYear[i][j] );
		file.PutString( line );
    }
}

void Bookkeeper::UpdateLastSeenTime()
{
	// clear all coin counts from (lOldTime,lNewTime]

	long lOldTime = m_iLastSeenTime;
	long lNewTime = time(NULL);

	if( lNewTime < lOldTime )
	{
		LOG->Warn( "The new time is older than the last seen time.  Is someone fiddling with the system clock?" );
		m_iLastSeenTime = lNewTime;
		return;
	}

    tm tOld = *localtime( &lOldTime );
    tm tNew = *localtime( &lNewTime );

	CLAMP( tOld.tm_year, tNew.tm_year-1, tNew.tm_year );

	while( 
		tOld.tm_year != tNew.tm_year ||
		tOld.tm_yday != tNew.tm_yday ||
		tOld.tm_hour != tNew.tm_hour )
	{
		tOld.tm_hour++;
		if( tOld.tm_hour == HOURS_PER_DAY )
		{
			tOld.tm_hour = 0;
			tOld.tm_yday++;
		}
		if( tOld.tm_yday == DAYS_PER_YEAR )
		{
			tOld.tm_yday = 0;
			tOld.tm_year++;
		}

		m_iCoinsByHourForYear[tOld.tm_yday][tOld.tm_hour] = 0;
	}
}

void Bookkeeper::CoinInserted()
{
	UpdateLastSeenTime();

	long lOldTime = m_iLastSeenTime;
    tm *pNewTime = localtime( &lOldTime );

	m_iCoinsByHourForYear[pNewTime->tm_yday][pNewTime->tm_hour]++;
}

int Bookkeeper::GetCoinsForDay( int iDayOfYear )
{
	int iCoins = 0;
	for( int i=0; i<HOURS_PER_DAY; i++ )
		iCoins += m_iCoinsByHourForYear[iDayOfYear][i];
	return iCoins;
}


void Bookkeeper::GetCoinsLast7Days( int coins[7] )
{
	UpdateLastSeenTime();

	long lOldTime = m_iLastSeenTime;
    tm time = *localtime( &lOldTime );

	for( int i=0; i<7; i++ )
	{
		time = GetYesterday( time );
		coins[i] = GetCoinsForDay( time.tm_yday );
	}
}


void Bookkeeper::GetCoinsLast52Weeks( int coins[52] )
{
	UpdateLastSeenTime();

	long lOldTime = m_iLastSeenTime;
    tm time = *localtime( &lOldTime );

	time = GetLastSunday( time );

	for( int w=0; w<52; w++ )
	{
		coins[w] = 0;

		for( int d=0; d<DAYS_IN_WEEK; d++ )
		{
			time = GetYesterday( time );
			coins[w] += GetCoinsForDay( time.tm_yday );
		}
	}
}

void Bookkeeper::GetCoinsByDayOfWeek( int coins[DAYS_IN_WEEK] )
{
	UpdateLastSeenTime();

	for( int i=0; i<DAYS_IN_WEEK; i++ )
		coins[i] = 0;

	long lOldTime = m_iLastSeenTime;
    tm time = *localtime( &lOldTime );

	for( int d=0; d<DAYS_PER_YEAR; d++ )
	{
		time = GetYesterday( time );
		coins[GetDayOfWeek(time)] += GetCoinsForDay( time.tm_yday );
	}
}

void Bookkeeper::GetCoinsByHour( int coins[HOURS_PER_DAY] )
{
	UpdateLastSeenTime();

	for( int h=0; h<HOURS_PER_DAY; h++ )
	{
		coins[h] = 0;

		for( int d=0; d<DAYS_PER_YEAR; d++ )
			coins[h] += m_iCoinsByHourForYear[d][h];
	}
}
