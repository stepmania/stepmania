#include "global.h"
#include "ArchHooks_Unix.h"
#include "archutils/Unix/SignalHandler.h"

#if defined(CRASH_HANDLER)
#include "archutils/Unix/CrashHandler.h"
#endif

#include "SDL_utils.h"

static void EmergencyShutdown( int signal )
{
	/* If we don't actually use SDL for video, this should be a no-op. */
	if( SDL_WasInit(SDL_INIT_VIDEO) )
		SDL_QuitSubSystem(SDL_INIT_VIDEO);
}
	
ArchHooks_Unix::ArchHooks_Unix()
{
#if defined(CRASH_HANDLER)
	CrashHandlerHandleArgs();

	SignalHandler::OnClose( CrashSignalHandler );
#endif

	/* Set up EmergencyShutdown, to try to shut down the window if we crash.
	 * This might blow up, so be sure to do it after the crash handler. */
	SignalHandler::OnClose( EmergencyShutdown );
}

/*
 * Copyright (c) 2003 by the person(s) listed below.  All rights reserved.
 *
 * Glenn Maynard
 */
