#include "global.h"
#include "ArchHooks.h"
#include "RageUtil.h"
#include "archutils/Win32/SpecialDirs.h"
#include "ProductInfo.h"
#include "RageFileManager.h"

// for timeGetTime
#include <windows.h>
#include <mmsystem.h>
#if defined(_MSC_VER)
#pragma comment(lib, "winmm.lib")
#endif


static bool g_bTimerInitialized;

static void InitTimer()
{
	if( g_bTimerInitialized )
		return;
	g_bTimerInitialized = true;

	timeBeginPeriod( 1 );
}

int64_t ArchHooks::GetMicrosecondsSinceStart( bool bAccurate )
{
	if( !g_bTimerInitialized )
		InitTimer();

	int64_t ret = timeGetTime() * int64_t(1000);
	if( bAccurate )
	{
		ret = FixupTimeIfLooped( ret );
		ret = FixupTimeIfBackwards( ret );
	}
	
	return ret;
}

void ArchHooks::MountInitialFilesystems( const CString &sDirOfExecutable )
{
	/* All Windows data goes in the directory one level above the executable. */
	CHECKPOINT_M( ssprintf( "DOE \"%s\"", sDirOfExecutable.c_str()) );
	vector<CString> parts;
	split( sDirOfExecutable, "/", parts );
	CHECKPOINT_M( ssprintf( "... %i parts", parts.size()) );
	ASSERT_M( parts.size() > 1, ssprintf("Strange sDirOfExecutable: %s", sDirOfExecutable.c_str()) );
	CString Dir = join( "/", parts.begin(), parts.end()-1 );
	FILEMAN->Mount( "dir", Dir, "/" );

	CString sMyDocumentsDir = GetMyDocumentsDir();
	CString sApplicationDataDir = GetApplicationDataDir();

	// Mount everything game-writable (not counting the editor) to the user's directory.
	FILEMAN->Mount( "dir", sApplicationDataDir + PRODUCT_ID + "/Cache", "/Cache" );
	FILEMAN->Mount( "dir", sMyDocumentsDir + PRODUCT_ID + "/Save", "/Save" );
	FILEMAN->Mount( "dir", sMyDocumentsDir + PRODUCT_ID + "/Screenshots", "/Screenshots" );
}

/*
 * (c) 2003-2004 Chris Danford
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
