#include "stdafx.h"
/*
-----------------------------------------------------------------------------
 Class: RageLog

 Desc: See header.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "RageLog.h"
#include "RageUtil.h"
#include <fstream>

#include "crash.h"

RageLog* LOG;		// global and accessable from anywhere in the program

// constants
#define LOG_FILE_NAME "log.txt"

RageLog::RageLog()
{
	// delete old log files
	remove( LOG_FILE_NAME );

	// Open log file and leave it open.
	m_fileLog = fopen( LOG_FILE_NAME, "w" );

	SYSTEMTIME st;
    GetLocalTime( &st );

#if defined(_MSC_VER)
	this->Trace( "Last compiled on %s.", __TIMESTAMP__ );
#endif

	this->Trace( "Log starting %.4d-%.2d-%.2d %.2d:%.2d:%.2d", 
					 st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond );
	this->Trace( "\n" );
}

RageLog::~RageLog()
{
	Flush();
	FreeConsole();
	if( m_fileLog )
		fclose( m_fileLog );
}

void RageLog::ShowConsole()
{
	// create a new console window and attach standard handles
	AllocConsole();
	freopen("CONOUT$","wb",stdout);
	freopen("CONOUT$","wb",stderr);
}

void RageLog::HideConsole()
{
	FreeConsole();
}

void RageLog::Trace( const char *fmt, ...)
{
    va_list	va;
    va_start(va, fmt);
    CString sBuff = vssprintf( fmt, va );
    va_end(va);

	if( m_fileLog )
		fprintf( m_fileLog, "%s\n", sBuff.GetString() ); 
	printf( "%s\n", sBuff.GetString() ); 
	CrashLog(sBuff);

#ifdef DEBUG
	this->Flush();	// implicit flush
#endif
}

/* Use this for more important information; it'll always be included
 * in crash dumps. */
void RageLog::Info( const char *fmt, ...)
{
    va_list	va;
    va_start(va, fmt);
    CString sBuff = vssprintf( fmt, va );
    va_end(va);
	Trace("%s", sBuff);
	StaticLog(sBuff);
}

void RageLog::Warn( const char *fmt, ...)
{
    va_list	va;
    va_start(va, fmt);
    CString sBuff = vssprintf( fmt, va );
    va_end(va);

	Trace(  "/////////////////////////////////////////\n"
			"WARNING:  %s\n"
			"/////////////////////////////////////////",
			sBuff.GetString() ); 
}

void RageLog::Flush()
{
	fflush( m_fileLog );
}
