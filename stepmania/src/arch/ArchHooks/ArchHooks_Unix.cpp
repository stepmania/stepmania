#include "global.h"
#include "RageLog.h"
#include "RageThreads.h"
#include "ArchHooks_Unix.h"
#include "StepMania.h"
#include "archutils/Unix/SignalHandler.h"
#include "archutils/Unix/GetSysInfo.h"

#if defined(CRASH_HANDLER)
#include "archutils/Unix/CrashHandler.h"
#endif

#include "SDL_utils.h"

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

static void DoCrashSignalHandler( int signal, siginfo_t *si, const ucontext_t *uc )
{
        /* Don't dump a debug file if the user just hit ^C. */
	if( !IsFatalSignal(signal) )
		return;

	CrashSignalHandler( signal, si, uc );
}

static void EmergencyShutdown( int signal, siginfo_t *si, const ucontext_t *uc )
{
	if( !IsFatalSignal(signal) )
		return;

	/* If we don't actually use SDL for video, this should be a no-op.  Only
	 * do this if the main thread crashes; trying to shut down from
	 * another thread causes crashes (eg. GL may be using TLS). */
	if( !strcmp(RageThread::GetCurThreadName(), "Main thread") && SDL_WasInit(SDL_INIT_VIDEO) )
		SDL_QuitSubSystem(SDL_INIT_VIDEO);

	kill( 0, SIGKILL );
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
}

/*
 * Copyright (c) 2003 by the person(s) listed below.  All rights reserved.
 *
 * Glenn Maynard
 */
