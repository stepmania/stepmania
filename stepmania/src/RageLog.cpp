#include "stdafx.h"
/*
-----------------------------------------------------------------------------
 Class: RageLog

 Desc: See header.

 Copyright (c) 2001-2003 by the person(s) listed below.  All rights reserved.
	Chris Danford
	Glenn Maynard
-----------------------------------------------------------------------------
*/

#include "StepMania.h"
#include "RageLog.h"
#include "RageUtil.h"
#include <fstream>
#include <time.h>

RageLog* LOG;		// global and accessable from anywhere in the program

/* We have a couple log types and a couple logs.
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
 *
 */

// constants
#define LOG_FILE_NAME "log.txt"
#define INFO_FILE_NAME "info.txt"


/* staticlog gets info.txt
 * crashlog gets log.txt */
enum {
	TO_INFO = 0x01,
};

RageLog::RageLog()
{
	// delete old log files
	remove( LOG_FILE_NAME );
	remove( INFO_FILE_NAME );

	// Open log file and leave it open.
	m_fileLog = fopen( LOG_FILE_NAME, "w" );
	m_fileInfo = fopen( INFO_FILE_NAME, "w" );

#if defined(_MSC_VER)
	this->Trace( "Last compiled on %s.", __TIMESTAMP__ );
#endif

	time_t cur_time;
	time(&cur_time);
	const struct tm *now = localtime(&cur_time);

	this->Trace( "Log starting %.4d-%.2d-%.2d %.2d:%.2d:%.2d", 
		1900+now->tm_year, now->tm_mon, now->tm_mday, now->tm_hour, now->tm_min, now->tm_sec );
	this->Trace( "" );
}

RageLog::~RageLog()
{
	Flush();
	HideConsole();
	if(m_fileLog) fclose( m_fileLog );
	if(m_fileInfo) fclose( m_fileInfo );
}

void RageLog::ShowConsole()
{
#if defined(WIN32)
	// create a new console window and attach standard handles
	AllocConsole();
	freopen("CONOUT$","wb",stdout);
	freopen("CONOUT$","wb",stderr);
#endif
}

void RageLog::HideConsole()
{
#if defined(WIN32)
	FreeConsole();
#endif
}

void RageLog::Trace( const char *fmt, ...)
{
    va_list	va;
    va_start(va, fmt);
    CString sBuff = vssprintf( fmt, va );
    va_end(va);

	Write(0, sBuff);
}

/* Use this for more important information; it'll always be included
 * in crash dumps. */
void RageLog::Info( const char *fmt, ...)
{
    va_list	va;
    va_start(va, fmt);
    CString sBuff = vssprintf( fmt, va );
    va_end(va);

	Write(TO_INFO, sBuff);
}

void RageLog::Warn( const char *fmt, ...)
{
    va_list	va;
    va_start(va, fmt);
    CString sBuff = vssprintf( fmt, va );
    va_end(va);

	Write(TO_INFO, ssprintf(
		"/////////////////////////////////////////\n"
		"WARNING:  %s\n"
		"/////////////////////////////////////////", sBuff.GetString()));
}

void RageLog::Write( int where, CString str)
{
	if( m_fileLog )
		fprintf(m_fileLog, "%s\n", str.GetString() );

	if( where & TO_INFO && m_fileInfo )
		fprintf(m_fileInfo, "%s\n", str.GetString() );

	HOOKS->Log(str, where & TO_INFO);

	printf("%s\n", str.GetString() );

#ifdef DEBUG
	Flush();
#else
	if(where & TO_INFO)
		Flush();
#endif
}

void RageLog::Flush()
{
	fflush( m_fileLog );
	fflush( m_fileInfo );
}
