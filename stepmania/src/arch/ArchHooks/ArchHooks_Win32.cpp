#include "stdafx.h"
#include "ArchHooks_Win32.h"

#include "archutils/win32/tls.h"
#include "archutils/win32/crash.h"

ArchHooks_Win32::ArchHooks_Win32()
{
	SetUnhandledExceptionFilter(CrashHandler);
	InitThreadData("Main thread");
	VDCHECKPOINT;
}

void ArchHooks_Win32::Log(CString str, bool important)
{
	/* It's OK to send to both of these; it'll let us know when important
	 * events occurred in crash dumps if they're recent. */
	if(important)
		StaticLog(str);

	CrashLog(str);
}

/*
 * Copyright (c) 2002-2003 by the person(s) listed below.  All rights reserved.
 *
 * Glenn Maynard
 */
