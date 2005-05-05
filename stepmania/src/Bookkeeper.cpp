#include "global.h"
#include "Bookkeeper.h"
#include "RageUtil.h"
#include "PrefsManager.h"
#include "RageLog.h"
#include "IniFile.h"
#include "GameConstantsAndTypes.h"
#include "SongManager.h"
#include "RageFile.h"
#include "XmlFile.h"
#include <ctime>


Bookkeeper*	BOOKKEEPER = NULL;	// global and accessable from anywhere in our program

static const CString COINS_DAT = "Data/Coins.xml";

Bookkeeper::Bookkeeper()
{
	ClearAll();

	ReadFromDisk();
}

Bookkeeper::~Bookkeeper()
{
	WriteToDisk();
}

#define WARN_AND_RETURN { LOG->Warn("Error parsing at %s:%d",__FILE__,__LINE__); return; }

void Bookkeeper::ClearAll()
{
	m_mapCoinsForHour.clear();
}

bool Bookkeeper::Date::operator<( const Date &rhs ) const
{
	if( m_iYear != rhs.m_iYear )
		return m_iYear < rhs.m_iYear;
	if( m_iDayOfYear != rhs.m_iDayOfYear )
		return m_iDayOfYear < rhs.m_iDayOfYear;
	return m_iHour < rhs.m_iHour;
}

void Bookkeeper::Date::Set( time_t t )
{
	tm ltime;
	localtime_r( &t, &ltime );

	Set( ltime );
}

void Bookkeeper::Date::Set( tm pTime )
{
	m_iHour = pTime.tm_hour;
	m_iDayOfYear = pTime.tm_yday;
	m_iYear = pTime.tm_year + 1900;
}

void Bookkeeper::LoadFromNode( const XNode *pNode )
{
	if( pNode->m_sName != "Bookkeeping" )
	{
		LOG->Warn( "Error loading bookkeeping: unexpected \"%s\"", pNode->m_sName.c_str() );
		return;
	}

	const XNode *pData = pNode->GetChild( "Data" );
	if( pData == NULL )
	{
		LOG->Warn( "Error loading bookkeeping: Data node missing", pNode->m_sName.c_str() );
		return;
	}

	FOREACH_CONST_Child( pData, day )
	{
		Date d;
		if( !day->GetAttrValue( "Hour", d.m_iHour ) ||
			!day->GetAttrValue( "Day", d.m_iDayOfYear ) ||
			!day->GetAttrValue( "Year", d.m_iYear ) )
		{
			LOG->Warn( "Incomplete date field" );
			continue;
		}

		int iCoins;
		day->GetValue( iCoins );

		m_mapCoinsForHour[d] = iCoins;
	}
}

XNode* Bookkeeper::CreateNode() const
{
	XNode *xml = new XNode;
	xml->m_sName = "Bookkeeping";

	{
		XNode* pData = xml->AppendChild("Data");

		for( map<Date,int>::const_iterator it = m_mapCoinsForHour.begin(); it != m_mapCoinsForHour.end(); ++it )
		{
			XNode *pDay = pData->AppendChild( "Coins" );

			const Date &d = it->first;
			pDay->AppendAttr( "Hour", d.m_iHour );
			pDay->AppendAttr( "Day", d.m_iDayOfYear );
			pDay->AppendAttr( "Year", d.m_iYear );
			
			int iCoins = it->second;
			pDay->SetValue( iCoins );
		}
	}

	return xml;
}

void Bookkeeper::ReadFromDisk()
{
	XNode xml;
	PARSEINFO pi;
	if( !xml.LoadFromFile(COINS_DAT, &pi) )
	{
		LOG->Warn( "Error parsing file \"%s\": %s", COINS_DAT.c_str(), pi.error_string.c_str() );
		return;
	}

	LoadFromNode( &xml );
}

void Bookkeeper::WriteToDisk()
{
	// Write data.  Use SLOW_FLUSH, to help ensure that we don't lose coin data.
    RageFile f;
	if( !f.Open(COINS_DAT, RageFile::WRITE|RageFile::SLOW_FLUSH) )
	{
		LOG->Warn( "Couldn't open file \"%s\" for writing: %s", COINS_DAT.c_str(), f.GetError().c_str() );
		return;
	}

	DISP_OPT opt;
	XNode *xml = CreateNode();
	xml->SaveToFile( f, &opt );
	delete xml;
}

void Bookkeeper::CoinInserted()
{
 	Date d;
	d.Set( time(NULL) );

	++m_mapCoinsForHour[d];
}

/* Return the number of coins between [beginning,ending). */
int Bookkeeper::GetNumCoinsInRange( map<Date,int>::const_iterator begin, map<Date,int>::const_iterator end ) const
{
	int iCoins = 0;

	while( begin != end )
	{
		iCoins += begin->second;
		++begin;
	}

	return iCoins;
}

int Bookkeeper::GetNumCoins( Date beginning, Date ending ) const
{
	return GetNumCoinsInRange( m_mapCoinsForHour.lower_bound( beginning ),
		m_mapCoinsForHour.lower_bound( ending ) );
}

int Bookkeeper::GetCoinsTotal() const
{
	return GetNumCoinsInRange( m_mapCoinsForHour.begin(), m_mapCoinsForHour.end() );
}


void Bookkeeper::GetCoinsLastDays( int coins[NUM_LAST_DAYS] ) const
{
	time_t lOldTime = time(NULL);
    tm time;
	localtime_r( &lOldTime, &time );

	time.tm_hour = 0;

	for( int i=0; i<NUM_LAST_DAYS; i++ )
	{
		tm EndTime = AddDays( time, +1 );
		coins[i] = GetNumCoins( time, EndTime );
		time = GetYesterday( time );
	}
}

void Bookkeeper::GetCoinsLastWeeks( int coins[NUM_LAST_WEEKS] ) const
{
	time_t lOldTime = time(NULL);
    tm time;
	localtime_r( &lOldTime, &time );

	time = GetNextSunday( time );
	time = GetYesterday( time );

	for( int w=0; w<NUM_LAST_WEEKS; w++ )
	{
		tm StartTime = AddDays( time, -DAYS_IN_WEEK );
		coins[w] = GetNumCoins( StartTime, time );
		time = StartTime;
	}
}

/* iDay is days since Jan 1.  iYear is eg. 2005.  Return the day of the week, where
 * 0 is Sunday. */
void Bookkeeper::GetCoinsByDayOfWeek( int coins[DAYS_IN_WEEK] ) const
{
	for( int i=0; i<DAYS_IN_WEEK; i++ )
		coins[i] = 0;

	for( map<Date,int>::const_iterator it = m_mapCoinsForHour.begin(); it != m_mapCoinsForHour.end(); ++it )
	{
		const Date &d = it->first;
		int iDayOfWeek = GetDayInYearAndYear( d.m_iDayOfYear, d.m_iYear ).tm_wday;
		coins[iDayOfWeek] += it->second;
	}
}

void Bookkeeper::GetCoinsByHour( int coins[HOURS_IN_DAY] ) const
{
	memset( coins, 0, sizeof(int) * HOURS_IN_DAY );
	for( map<Date,int>::const_iterator it = m_mapCoinsForHour.begin(); it != m_mapCoinsForHour.end(); ++it )
	{
		const Date &d = it->first;

		if( d.m_iHour >= HOURS_IN_DAY )
		{
			LOG->Warn( "Hour %i >= %i", d.m_iHour, HOURS_IN_DAY );
			continue;
		}

		coins[d.m_iHour] += it->second;
	}
}

/*
 * (c) 2003-2005 Chris Danford, Glenn Maynard
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
