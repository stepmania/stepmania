#include "global.h"
#include "ArchHooks_Unix.h"
#include "archutils/Unix/SignalHandler.h"

#if defined(HAVE_BACKTRACE)
#include "archutils/Unix/CrashHandler.h"
#endif

ArchHooks_Unix::ArchHooks_Unix()
{
#if defined(HAVE_BACKTRACE)
	CrashHandlerHandleArgs();

	SignalHandler::OnClose( CrashSignalHandler );
#endif
}

/*
 * Copyright (c) 2003 by the person(s) listed below.  All rights reserved.
 *
 * Glenn Maynard
 */
