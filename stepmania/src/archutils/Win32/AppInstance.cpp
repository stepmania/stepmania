#include "global.h"
#include "AppInstance.h"

AppInstance::AppInstance()
{
	/* Little trick to get an HINSTANCE of ourself without having access to the hwnd ... */
	TCHAR szFullAppPath[MAX_PATH];
	GetModuleFileName(NULL, szFullAppPath, MAX_PATH);
	h = LoadLibrary(szFullAppPath);
	/* h will be NULL if this fails.  Most operations that take an HINSTANCE
	 * will still work without one (but may be missing graphics); that's OK. */
}

AppInstance::~AppInstance()
{
	if(h)
		FreeLibrary(h);
}

/*
 * Copyright (c) 2002 by the person(s) listed below.  All rights reserved.
 *
 * Glenn Maynard
 */
