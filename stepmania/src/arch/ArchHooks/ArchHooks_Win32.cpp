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

/*
 * Copyright (c) 2002-2003 by the person(s) listed below.  All rights reserved.
 *
 * Glenn Maynard
 */
