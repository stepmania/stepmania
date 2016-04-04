#include "global.h"
#include "RageLog.h"
#include "RageUtil.h"
#include "RageTimer.h"
#include "RageFile.h"
#include "RageThreads.h"
#include "RageUtil.hpp"
#include "RageString.hpp"

#include <ctime>
#if defined(_WINDOWS)
#include <windows.h>
#endif
#include <map>

using std::vector;

RageLog* LOG;		// global and accessible from anywhere in the program

/*
 * We have a couple log types and a couple logs.
 *
 * Traces are for very verbose debug information.  Use them as much as you want.
 *
 * Warnings are for things that shouldn't happen, but can be dealt with.  (If
 * they can't be dealt with, use RageException::Throw, which will also send a
 * warning.)
 *
 * Info is for important bits of information.  These should be used selectively.
 * Try to keep Info text dense; lots of information is fine, but try to keep
 * it to a reasonable total length.
 *
 * log.txt receives all logs.  This file can get rather large.
 *
 * info.txt receives warnings and infos.  This file should be fairly small; small
 * enough to be mailed without having to be edited or zipped, and small enough
 * to be very easily read.
 */

/* Map data names to logged data.
 *
 * This lets us keep individual bits of changing information to be present
 * in crash logs.  For example, we want to know which file was being processed
 * if we crash during a texture load.  However, we don't want to log every
 * load, since there are a huge number, even for log.txt.  We only want to
 * know the current one, if any.
 *
 * So, when a texture begins loading, we do:
 * LOG->MapLog("TextureManager::Load", "Loading foo.png");
 * and when it finishes loading without crashing,
 * LOG->UnmapLog("TextureManager::Load");
 *
 * Each time a mapped log changes, we update a block of static text to be put
 * in info.txt, so we see "Loading foo.png".
 *
 * The identifier is never displayed, so we can use a simple local object to
 * map/unmap, using any mechanism to generate unique IDs. */
static std::map<std::string, std::string> LogMaps;

#define LOG_PATH	"/Logs/log.txt"
#define INFO_PATH	"/Logs/info.txt"
#define TIME_PATH	"/Logs/timelog.txt"
#define USER_PATH	"/Logs/userlog.txt"

static RageFile *g_fileLog, *g_fileInfo, *g_fileUserLog, *g_fileTimeLog;

/* Mutex writes to the files.  Writing to files is not thread-aware, and this is the
 * only place we write to the same file from multiple threads. */
static RageMutex *g_Mutex;

/* staticlog gets info.txt
 * crashlog gets log.txt */

RageLog::RageLog(): m_bLogToDisk(false), m_bInfoToDisk(false),
m_bUserLogToDisk(false), m_bFlush(false), m_bShowLogOutput(false)
{
	g_fileLog = new RageFile;
	g_fileInfo = new RageFile;
	g_fileUserLog = new RageFile;
	g_fileTimeLog = new RageFile;

	if(!g_fileTimeLog->Open(TIME_PATH, RageFile::WRITE|RageFile::STREAMED))
	{ fprintf(stderr, "Couldn't open %s: %s\n", TIME_PATH, g_fileTimeLog->GetError().c_str()); }

	g_Mutex = new RageMutex( "Log" );
}

RageLog::~RageLog()
{
	/* Add the mapped log data to info.txt. */
	const std::string AdditionalLog = GetAdditionalLog();
	auto AdditionalLogLines = Rage::split(AdditionalLog, "\n");
	for (auto &line: AdditionalLogLines)
	{
		this->Info( Rage::trim(line) );
	}

	Flush();
	SetShowLogOutput( false );
	g_fileLog->Close();
	g_fileInfo->Close();
	g_fileUserLog->Close();
	g_fileTimeLog->Close();

	Rage::safe_delete( g_Mutex );
	Rage::safe_delete( g_fileLog );
	Rage::safe_delete( g_fileInfo );
	Rage::safe_delete( g_fileUserLog );
}

void RageLog::SetLogToDisk( bool b )
{
	if( m_bLogToDisk == b )
		return;

	m_bLogToDisk = b;

	if( !m_bLogToDisk )
	{
		if( g_fileLog->IsOpen() )
			g_fileLog->Close();
		return;
	}

	if( !g_fileLog->Open( LOG_PATH, RageFile::WRITE|RageFile::STREAMED ) )
		fprintf( stderr, "Couldn't open %s: %s\n", LOG_PATH, g_fileLog->GetError().c_str() );
}

void RageLog::SetInfoToDisk( bool b )
{
	if( m_bInfoToDisk == b )
		return;

	m_bInfoToDisk = b;

	if( !m_bInfoToDisk )
	{
		if( g_fileInfo->IsOpen() )
			g_fileInfo->Close();
		return;
	}

	if( !g_fileInfo->Open( INFO_PATH, RageFile::WRITE|RageFile::STREAMED ) )
		fprintf( stderr, "Couldn't open %s: %s\n", INFO_PATH, g_fileInfo->GetError().c_str() );
}

void RageLog::SetUserLogToDisk( bool b )
{
	if( m_bUserLogToDisk == b )
		return;

	m_bUserLogToDisk = b;

	if( !m_bUserLogToDisk )
	{
		if( g_fileUserLog->IsOpen() )
			g_fileUserLog->Close();
		return;
	}
	if( !g_fileUserLog->Open(USER_PATH, RageFile::WRITE|RageFile::STREAMED) )
		fprintf( stderr, "Couldn't open %s: %s\n", USER_PATH, g_fileUserLog->GetError().c_str() );
}

void RageLog::SetFlushing( bool b )
{
	m_bFlush = b;
}

/* Enable or disable display of output to stdout, or a console window in Windows. */
void RageLog::SetShowLogOutput( bool show )
{
	m_bShowLogOutput = show;

#if defined(WIN32)
	if( m_bShowLogOutput )
	{
		// create a new console window and attach standard handles
		AllocConsole();
		freopen( "CONOUT$","wb", stdout );
		freopen( "CONOUT$","wb", stderr );
	}
	else
	{
		FreeConsole();
	}
#endif
}

void RageLog::Write( WriteDest where, std::string const &line )
{
	LockMut( *g_Mutex );

	const char *const sWarningSeparator = "/////////////////////////////////////////";
	auto lines = Rage::split(line, "\n", Rage::EmptyEntries::include);
	bool containsLoud = flags(where & WriteDest::Loud);
	bool containsInfo = flags(where & WriteDest::Info);
	if( containsLoud )
	{
		if( m_bLogToDisk && g_fileLog->IsOpen() )
			g_fileLog->PutLine( sWarningSeparator );
		puts( sWarningSeparator );
	}

	std::string sTimestamp = SecondsToMMSSMsMsMs( RageTimer::GetTimeSinceStart() ) + ": ";
	std::string sWarning;
	if( containsLoud )
		sWarning = "WARNING: ";

	for (auto &str : lines)
	{
		if( sWarning.size() )
		{
			str = sWarning + str;
		}
		if( m_bShowLogOutput || containsInfo )
		{
			puts(str.c_str());
		}
		if( containsInfo )
		{
			AddToInfo( str );
		}
		if( m_bLogToDisk && containsInfo && g_fileInfo->IsOpen() )
		{
			g_fileInfo->PutLine( str );
		}
		if( m_bUserLogToDisk && (flags(where & WriteDest::UserLog)) && g_fileUserLog->IsOpen() )
		{
			g_fileUserLog->PutLine( str );
		}
		/* Add a timestamp to log.txt and RecentLogs, but not the rest of info.txt
		 * and stdout. */
		str = sTimestamp + str;

		if( flags(where & WriteDest::Time) )
		{
			g_fileTimeLog->PutLine(str);
		}
		AddToRecentLogs( str );

		if( m_bLogToDisk && g_fileLog->IsOpen() )
		{
			g_fileLog->PutLine( str );
		}
	}
	if( containsLoud )
	{
		if( m_bLogToDisk && g_fileLog->IsOpen() )
		{
			g_fileLog->PutLine( sWarningSeparator );
		}
		puts( sWarningSeparator );
	}
	if( m_bFlush || containsInfo )
	{
		Flush();
	}
}


void RageLog::Flush()
{
	g_fileLog->Flush();
	g_fileInfo->Flush();
	g_fileTimeLog->Flush();
	g_fileUserLog->Flush();
}


#define NEWLINE "\n"

static char staticlog[1024*32]="";
static unsigned staticlog_size = 0;
void RageLog::AddToInfo( const std::string &str )
{
	using std::min;
	static bool limit_reached = false;
	if( limit_reached )
		return;

	unsigned len = str.size() + strlen( NEWLINE );
	if( staticlog_size + len > sizeof(staticlog) )
	{
		const std::string txt( NEWLINE "Staticlog limit reached" NEWLINE );
		unsigned txtSize = static_cast<unsigned>(sizeof(staticlog) - txt.size());
		unsigned const pos = min( staticlog_size, txtSize );
		memcpy( staticlog + pos, txt.data(), txt.size() );
		limit_reached = true;
		return;
	}

	memcpy( staticlog+staticlog_size, str.data(), str.size() );
	staticlog_size += str.size();
	memcpy( staticlog+staticlog_size, NEWLINE, strlen(NEWLINE) );
	staticlog_size += strlen( NEWLINE );
}

const char *RageLog::GetInfo()
{
	staticlog[ sizeof(staticlog)-1 ] = 0;
	return staticlog;
}

static const int BACKLOG_LINES = 10;
static char backlog[BACKLOG_LINES][1024];
static int backlog_start=0, backlog_cnt=0;
void RageLog::AddToRecentLogs( const std::string &str )
{
	unsigned len = str.size();
	if( len > sizeof(backlog[backlog_start])-1 )
		len = sizeof(backlog[backlog_start])-1;

	strncpy( backlog[backlog_start], str.c_str(), len );
	backlog[backlog_start] [ len ] = 0;

	backlog_start++;
	if( backlog_start > backlog_cnt )
		backlog_cnt=backlog_start;
	backlog_start %= BACKLOG_LINES;
}

const char *RageLog::GetRecentLog( int n )
{
	if( n >= BACKLOG_LINES || n >= backlog_cnt )
		return nullptr;

	if( backlog_cnt == BACKLOG_LINES )
	{
		n += backlog_start;
		n %= BACKLOG_LINES;
	}
	/* Make sure it's terminated: */
	backlog[n][ sizeof(backlog[n])-1 ] = 0;

	return backlog[n];
}


static char g_AdditionalLogStr[10240] = "";
static int g_AdditionalLogSize = 0;

void RageLog::UpdateMappedLog()
{
	using std::min;
	std::string str;
	for (auto const &i: LogMaps)
	{
		str += fmt::sprintf( "%s" NEWLINE, i.second.c_str() );
	}
	g_AdditionalLogSize = min( sizeof(g_AdditionalLogStr), str.size()+1 );
	memcpy( g_AdditionalLogStr, str.c_str(), g_AdditionalLogSize );
	g_AdditionalLogStr[ sizeof(g_AdditionalLogStr)-1 ] = 0;
}

const char *RageLog::GetAdditionalLog()
{
	using std::min;
	int size = min( g_AdditionalLogSize, (int) sizeof(g_AdditionalLogStr)-1 );
	g_AdditionalLogStr[size] = 0;
	return g_AdditionalLogStr;
}

void RageLog::StoreMapLog(std::string const &key, std::string const &result)
{
	LogMaps[key] = result;
	UpdateMappedLog();
}

void RageLog::UnmapLog( const std::string &key )
{
	LogMaps.erase( key );
	UpdateMappedLog();
}

void ShowWarningOrTrace( const char *file, int line, std::string const &message, bool bWarning )
{
	/* Ignore everything up to and including the first "src/". */
	const char *temp = strstr( file, "src/" );
	if( temp )
		file = temp + 4;

	if (LOG)
	{
		if (bWarning)
		{
			LOG->Warn("%s:%i: %s", file, line, message);
		}
		else
		{
			LOG->Trace("%s:%i: %s", file, line, message);
		}
	}
	else
	{
		fprintf(stderr, "%s:%i: %s\n", file, line, message.c_str());
	}
}


/*
 * Copyright (c) 2001-2004 Chris Danford, Glenn Maynard
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
