#include "global.h"
#include "RageLog.h"
#include "RageThreads.h"
#include "ArchHooks_Unix.h"
#include "StepMania.h"
#include "archutils/Unix/SignalHandler.h"
#include "archutils/Unix/GetSysInfo.h"
#include "archutils/Unix/LinuxThreadHelpers.h"
#include "archutils/Unix/EmergencyShutdown.h"
#include "archutils/Unix/AssertionHandler.h"
#include <unistd.h>
#include "RageUtil.h"
#include <sys/time.h>

#if defined(CRASH_HANDLER)
#include "archutils/Unix/CrashHandler.h"
#endif


static bool IsFatalSignal( int signal )
{
	switch( signal )
	{
	case SIGINT:
	case SIGTERM:
	case SIGHUP:
		return false;
	default:
		return true;
	}
}

static void DoCleanShutdown( int signal, siginfo_t *si, const ucontext_t *uc )
{
	if( IsFatalSignal(signal) )
		return;

	/* ^C. */
	ExitGame();
}

#if defined(CRASH_HANDLER)
static void DoCrashSignalHandler( int signal, siginfo_t *si, const ucontext_t *uc )
{
        /* Don't dump a debug file if the user just hit ^C. */
	if( !IsFatalSignal(signal) )
		return;

	CrashSignalHandler( signal, si, uc );
}
#endif

static void EmergencyShutdown( int signal, siginfo_t *si, const ucontext_t *uc )
{
	if( !IsFatalSignal(signal) )
		return;

	DoEmergencyShutdown();

#if defined(CRASH_HANDLER)
	/* If we ran the crash handler, then die. */
	kill( getpid(), SIGKILL );
#else
	/* We didn't run the crash handler.  Run the default handler, so we can dump core. */
	SignalHandler::ResetSignalHandlers();
	raise( signal );
#endif
}
	
#if defined(HAVE_TLS)
static thread_local int g_iTestTLS = 0;

static int TestTLSThread( void *p )
{
	g_iTestTLS = 2;
	return 0;
}

static void TestTLS()
{
	/* TLS won't work on older threads libraries, and may crash. */
	if( !UsingNPTL() )
		return;

	/* TLS won't work on older Linux kernels.  Do a simple check. */
	g_iTestTLS = 1;

	RageThread TestThread;
	TestThread.SetName( "TestTLS" );
	TestThread.Create( TestTLSThread, NULL );
	TestThread.Wait();

	if( g_iTestTLS == 1 )
		RageThread::SetSupportsTLS( true );
}
#endif

static int64_t GetMicrosecondsSinceEpoch()
{
	struct timeval tv;
	gettimeofday( &tv, NULL );

	return int64_t(tv.tv_sec) * 1000000 + int64_t(tv.tv_usec);
}

int64_t ArchHooks::GetMicrosecondsSinceStart( bool bAccurate )
{
	int64_t ret = GetMicrosecondsSinceEpoch();
	if( bAccurate )
		ret = FixupTimeIfBackwards( ret );
	return ret;
}

ArchHooks_Unix::ArchHooks_Unix()
{
	/* First, handle non-fatal termination signals. */
	SignalHandler::OnClose( DoCleanShutdown );

#if defined(CRASH_HANDLER)
	CrashHandlerHandleArgs( g_argc, g_argv );
	InitializeCrashHandler();
	SignalHandler::OnClose( DoCrashSignalHandler );
#endif

	/* Set up EmergencyShutdown, to try to shut down the window if we crash.
	 * This might blow up, so be sure to do it after the crash handler. */
	SignalHandler::OnClose( EmergencyShutdown );

	InstallExceptionHandler();
	
#if defined(HAVE_TLS)
	TestTLS();
#endif
}

#ifndef _CS_GNU_LIBC_VERSION
#define _CS_GNU_LIBC_VERSION 2
#endif

static CString LibcVersion()
{	
	char buf[1024] = "(error)";
	int ret = confstr( _CS_GNU_LIBC_VERSION, buf, sizeof(buf) );
	if( ret == -1 )
		return "(unknown)";

	return buf;
}

void ArchHooks_Unix::DumpDebugInfo()
{
	CString sys;
	int vers;
	GetKernel( sys, vers );
	LOG->Info( "OS: %s ver %06i", sys.c_str(), vers );

#if defined(CRASH_HANDLER)
	LOG->Info( "Crash backtrace component: %s", BACKTRACE_METHOD_TEXT );
	LOG->Info( "Crash lookup component: %s", BACKTRACE_LOOKUP_METHOD_TEXT );
#if defined(BACKTRACE_DEMANGLE_METHOD_TEXT)
	LOG->Info( "Crash demangle component: %s", BACKTRACE_DEMANGLE_METHOD_TEXT );
#endif
#endif

	LOG->Info( "Runtime library: %s", LibcVersion().c_str() );
	LOG->Info( "Threads library: %s", ThreadsVersion().c_str() );
}

void ArchHooks_Unix::SetTime( tm newtime )
{
	CString sCommand = ssprintf( "date %02d%02d%02d%02d%04d.%02d",
		newtime.tm_mon+1,
		newtime.tm_mday,
		newtime.tm_hour,
		newtime.tm_min,
		newtime.tm_year+1900,
		newtime.tm_sec );

	LOG->Trace( "executing '%s'", sCommand.c_str() ); 
	system( sCommand );

	system( "hwclock --systohc" );
}

/*
 * (c) 2003-2004 Glenn Maynard
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
