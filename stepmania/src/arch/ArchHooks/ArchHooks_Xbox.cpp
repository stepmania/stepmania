#include "global.h"
#include "ArchHooks_Xbox.h"
#include "dsound.h"	// for timeGetTime
#include "archutils/Xbox/custom_launch_params.h" // for XGetCustomLaunchData

static bool g_bTimerInitialized;
static DWORD g_iStartTime;

static void InitTimer()
{
	if( g_bTimerInitialized )
		return;
	g_bTimerInitialized = true;

	g_iStartTime = timeGetTime();
}

int64_t ArchHooks::GetMicrosecondsSinceStart( bool bAccurate )
{
	if( !g_bTimerInitialized )
		InitTimer();

	int64_t ret = (timeGetTime() - g_iStartTime) * int64_t(1000);
	if( bAccurate )
	{
		ret = FixupTimeIfLooped( ret );
		ret = FixupTimeIfBackwards( ret );
	}
	
	return ret;
}

ArchHooks_Xbox::ArchHooks_Xbox()
{
	XGetCustomLaunchData();
}

ArchHooks_Xbox::~ArchHooks_Xbox()
{
	// We only want to reboot the Xbox in a software manner.
	XLaunchNewImage( NULL, NULL );
}

/*
 * (c) 2003-2004 Glenn Maynard, Chris Danford
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
