#include "global.h"
/*
-----------------------------------------------------------------------------
 Class: RageLog

 Desc: See header.

 Copyright (c) 2001-2003 by the person(s) listed below.  All rights reserved.
	Chris Danford
	Glenn Maynard
-----------------------------------------------------------------------------
*/

#include "arch/ArchHooks/ArchHooks.h"
#include "arch/arch.h"
#include "RageLog.h"
#include "RageUtil.h"
#include "RageTimer.h"
#include "PrefsManager.h"
#include "RageFile.h"
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
map<CString, CString> LogMaps;

// constants
#define LOG_PATH BASE_PATH "log.txt"
#define INFO_PATH BASE_PATH "info.txt"

#if defined(_WINDOWS)
extern unsigned long version_num;
extern const char *version_time;
#endif

/* staticlog gets info.txt
 * crashlog gets log.txt */
enum {
	/* If this is set, the message will also be written to info.txt and
	 * will be flagged "important" when sent to HOOKS->Log. (info and warnings) */
	WRITE_TO_INFO = 0x01,
	
	/* Whether this line should be loud when written to log.txt (warnings). */
	WRITE_LOUD = 0x02
};

RageLog::RageLog()
{
	// delete old log files
	remove( LOG_PATH );
	remove( INFO_PATH );

	// Open log file and leave it open.
	m_fileLog = Ragefopen( LOG_PATH, "w" );
	m_fileInfo = Ragefopen( INFO_PATH, "w" );

#if defined(_MSC_VER)
	this->Trace( "Last compiled on %s.", __TIMESTAMP__ );
#endif

	time_t cur_time;
	time(&cur_time);
	const struct tm *now = localtime(&cur_time);

	this->Trace( "Log starting %.4d-%.2d-%.2d %.2d:%.2d:%.2d", 
		1900+now->tm_year, now->tm_mon, now->tm_mday, now->tm_hour, now->tm_min, now->tm_sec );
	this->Trace( "" );
#if defined(_WINDOWS)
	this->Info("Compiled %s (build %i)", version_time, version_num);
#endif
}

RageLog::~RageLog()
{
	/* Add the mapped log data to info.txt. */
	fprintf( m_fileInfo, "%s", GetAdditionalLog() );
	fprintf( m_fileLog, "\nStatics:\n%s", GetAdditionalLog() );

	Flush();
	HideConsole();
	if(m_fileLog) fclose( m_fileLog );
	if(m_fileInfo) fclose( m_fileInfo );
}

void RageLog::ShowConsole()
{
#if defined(WIN32) && !defined(_XBOX)
	// create a new console window and attach standard handles
	AllocConsole();
	freopen("CONOUT$","wb",stdout);
	freopen("CONOUT$","wb",stderr);
#endif
}

void RageLog::HideConsole()
{
#if defined(WIN32) && !defined(_XBOX)
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

	Write(WRITE_TO_INFO, sBuff);
}

void RageLog::Warn( const char *fmt, ...)
{
    va_list	va;
    va_start(va, fmt);
    CString sBuff = vssprintf( fmt, va );
    va_end(va);

	Write(WRITE_TO_INFO | WRITE_LOUD, ssprintf("WARNING: %s", sBuff.c_str()));
}

static const int STATICLOG_SIZE = 1024*32;
static char staticlog[STATICLOG_SIZE]="";
static char *staticlog_ptr=staticlog, *staticlog_end=staticlog+STATICLOG_SIZE;
void RageLog::AddToInfo( CString str )
{
	int len = str.size()+3; /* +\r +\n +null */
	if(staticlog_ptr+len >= staticlog_end)
	{
		const char *txt = "\nStaticlog limit reached\n";
		char *max_ptr = staticlog_end-strlen(txt)-1;
		if(staticlog_ptr > max_ptr)
			staticlog_ptr = max_ptr;

		strcpy(staticlog_ptr, txt);
		staticlog_ptr=NULL; /* stop */
		return;
	}

	strcpy(staticlog_ptr, str);

	staticlog_ptr[len-3] = '\r';
	staticlog_ptr[len-2] = '\n';
	staticlog_ptr[len-1] = 0;
	/* Advance to sit on the NULL, so the terminator will be overwritten
	 * on the next log. */
	staticlog_ptr += len-1;
}

const char *RageLog::GetInfo()
{
	if( staticlog == NULL )
		staticlog[ sizeof(staticlog)-1 ] = 0;
	else
	{
		int len = min( staticlog_ptr-staticlog, (int) sizeof(staticlog)-1 );
		staticlog[ len ] = 0;
	}
	return staticlog;
}

/* Since this is a static, preallocated region, even if it gets trampled by
 * a crash we'll just print garbage, not crash due to an invalid pointer. */
static const int BACKLOG_LINES = 10;
static char backlog[BACKLOG_LINES][1024];
static int backlog_start=0, backlog_cnt=0;
void RageLog::AddToRecentLogs( CString str )
{
	int len = str.size();
	if(len > sizeof(backlog[backlog_start])-1)
		len = sizeof(backlog[backlog_start])-1;

	strncpy(backlog[backlog_start], str, len);
	backlog[backlog_start] [ len ] = 0;

	backlog_start++;
	if(backlog_start > backlog_cnt)
		backlog_cnt=backlog_start;
	backlog_start %= BACKLOG_LINES;
}

const char *RageLog::GetRecentLog( int n )
{
	if( n >= BACKLOG_LINES || n >= backlog_cnt )
		return false;
	n += backlog_start;
	n %= BACKLOG_LINES;
	/* Make sure it's terminated: */
	backlog[n][ sizeof(backlog[n])-1 ] = 0;

	return backlog[n];
}

void RageLog::Write( int where, CString str)
{
	if( PREFSMAN && PREFSMAN->m_bTimestamping )
		str = SecondsToTime(RageTimer::GetTimeSinceStart()) + ": " + str;

	if( where&WRITE_TO_INFO && m_fileInfo )
		fprintf(m_fileInfo, "%s\n", str.c_str() );

	HOOKS->Log(str, where & WRITE_TO_INFO);

	if( where & WRITE_TO_INFO )
		AddToInfo( str );
	AddToRecentLogs( str );
	
	/* Only do this for log.txt and stdout.  The other outputs are
	 * fairly quiet anyway, so the prepended "WARNING" makes them stand
	 * out well enough and this is just clutter. */
	if( where & WRITE_LOUD )
	{
		str = ssprintf(
			  "/////////////////////////////////////////\n"
			  "%s\n"
			  "/////////////////////////////////////////",
			  str.c_str());
	}

	if( m_fileLog )
		fprintf(m_fileLog, "%s\n", str.c_str() );

	printf("%s\n", str.c_str() );

	if( (PREFSMAN && PREFSMAN->m_bDebugMode) || (where & WRITE_TO_INFO) )
		Flush();
}


void RageLog::Flush()
{
	fflush( m_fileLog );
	fflush( m_fileInfo );
}


static char g_AdditionalLogStr[10240] = "";
static int g_AdditionalLogSize = 0;

void RageLog::UpdateMappedLog()
{
	CString str;
	for(map<CString, CString>::const_iterator i = LogMaps.begin(); i != LogMaps.end(); ++i)
		str += ssprintf("%s\n", i->second.c_str());

	g_AdditionalLogSize = min( sizeof(g_AdditionalLogStr), str.size()+1 );
	memcpy( g_AdditionalLogStr, str.c_str(), g_AdditionalLogSize );
	g_AdditionalLogStr[ sizeof(g_AdditionalLogStr)-1 ] = 0;

	/* XXX: deprecated */
	HOOKS->AdditionalLog(str);
}

const char *RageLog::GetAdditionalLog()
{
	int size = min( g_AdditionalLogSize, (int) sizeof(g_AdditionalLogStr)-1 );
	g_AdditionalLogStr[size] = 0;
	return g_AdditionalLogStr;
}

void RageLog::MapLog(const CString &key, const char *fmt, ...)
{
    va_list	va;
    va_start(va, fmt);
	LogMaps[key] = vssprintf( fmt, va );
    va_end(va);

	UpdateMappedLog();
}

void RageLog::UnmapLog(const CString &key)
{
	LogMaps.erase(key);
	UpdateMappedLog();
}

Checkpoint_::Checkpoint_(CString key_, int n, const char *fmt, ...)
{
	key = ssprintf("%s:%i", key_.c_str(), n);

    va_list	va;
    va_start(va, fmt);
	LOG->MapLog( key, vssprintf(fmt, va) );
    va_end(va);
}

Checkpoint_::~Checkpoint_()
{
	LOG->UnmapLog( key );
}

