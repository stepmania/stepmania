#include "stdafx.h"
/*
-----------------------------------------------------------------------------
 Class: RageLog

 Desc: See header.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/
/* */
#include "RageLog.h"
#include "RageUtil.h"
#include <fstream>


#include "dxerr8.h"
#pragma comment(lib, "DxErr8.lib")


RageLog* LOG;		// global and accessable from anywhere in the program


// constants
#define LOG_FILE_NAME "log.txt"


RageLog::RageLog()
{
	// delete old log files
	DeleteFile( LOG_FILE_NAME );

	// Open log file and leave it open.  Let the OS close it when the app exits
	m_fileLog = fopen( LOG_FILE_NAME, "w" );


	SYSTEMTIME st;
    GetLocalTime( &st );

	this->Trace( "Last compiled on %s.", __TIMESTAMP__ );
	this->Trace( "Log starting %.4d-%.2d-%.2d %.2d:%.2d:%.2d", 
					 st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond );
	this->Trace( "\n" );
}

RageLog::~RageLog()
{
	Flush();
	FreeConsole();
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

void RageLog::Trace( LPCTSTR fmt, ...)
{
    va_list	va;
    va_start(va, fmt);

    CString sBuff = vssprintf( fmt, va );
	sBuff += "\n";

	fprintf( m_fileLog, sBuff ); 
	printf( sBuff ); 

#ifdef DEBUG
	this->Flush();	// implicit flush
#endif
}

void RageLog::Trace( HRESULT hr, LPCTSTR fmt, ...)
{
    va_list	va;
    va_start(va, fmt);
    CString s = vssprintf( fmt, va );
	s += ssprintf( "(%s)", DXGetErrorString8(hr) );
	this->Trace( s );
}

void RageLog::Warn( LPCTSTR fmt, ...)
{
    va_list	va;
    va_start(va, fmt);

    CString sBuff = vssprintf( fmt, va );

	Trace(  "/////////////////////////////////////////\n"
			"WARNING:  %s\n"
			"/////////////////////////////////////////",
			sBuff ); 
}

void RageLog::Flush()
{
	fflush( m_fileLog );
}
