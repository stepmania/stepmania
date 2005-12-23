#include "global.h"
#include "ArchHooks_Xbox.h"
#include "dsound.h"	// for timeGetTime
#include "archutils/Xbox/custom_launch_params.h" // for XGetCustomLaunchData
#include "archutils/Xbox/VirtualMemory.h"

#include <xtl.h> // for XNetStartup
#include <new.h> // for _set_new_handler and _set_new_mode

typedef struct _UNICODE_STRING {unsigned short Length; unsigned short MaximumLength; PSTR Buffer;} UNICODE_STRING,*PUNICODE_STRING;
extern "C" XBOXAPI DWORD WINAPI IoCreateSymbolicLink(IN PUNICODE_STRING SymbolicLinkName,IN PUNICODE_STRING DeviceName);

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

void MountDriveLetter(char drive, char* szDevice, char* szDir)
{
	char szSourceDevice[256];
	char szDestinationDrive[16];
	sprintf(szDestinationDrive, "\\??\\%c:", drive);
	sprintf(szSourceDevice,"\\Device\\%s",szDevice);
	if (*szDir != 0x00 && *szDir != '\\') 
	{
		strcat(szSourceDevice, "\\");
		strcat(szSourceDevice, szDir);
	}

	UNICODE_STRING LinkName = 
	{
		strlen(szDestinationDrive),
		strlen(szDestinationDrive) + 1,
		szDestinationDrive
	};
	UNICODE_STRING DeviceName = 
	{
		strlen(szSourceDevice),
		strlen(szSourceDevice) + 1,
		szSourceDevice
	};

	IoCreateSymbolicLink(&LinkName, &DeviceName);
}

void MountDrives()
{
	MountDriveLetter('A', "Cdrom0", "\\");
	MountDriveLetter('E', "Harddisk0\\Partition1", "\\");
	MountDriveLetter('C', "Harddisk0\\Partition2", "\\");
	MountDriveLetter('X', "Harddisk0\\Partition3", "\\");
	MountDriveLetter('Y', "Harddisk0\\Partition4", "\\");
	MountDriveLetter('F', "Harddisk0\\Partition6", "\\");
	MountDriveLetter('G', "Harddisk0\\Partition7", "\\");
}

bool SetupNetwork()
{
#if !defined(WITHOUT_NETWORKING)
	XNetStartupParams xnsp;
	memset(&xnsp, 0, sizeof(xnsp));
	xnsp.cfgSizeOfStruct = sizeof(XNetStartupParams);
	xnsp.cfgFlags = XNET_STARTUP_BYPASS_SECURITY;

	INT err = XNetStartup(&xnsp);

	return err == 0;
#else
	return true;
#endif
}

// if Xbox has 128 meg RAM make sure its used
void EnableExtraRAM()
{
	LARGE_INTEGER regVal;

 	// Verify that we have 128 megs available
	MEMORYSTATUS memStatus;
	GlobalMemoryStatus( &memStatus );
	if( memStatus.dwTotalPhys < (100 * 1024 * 1024) )
		return;

	// Grab the existing default type (0x02FF)
	READMSRREG( 0x02FF, &regVal );
  
	// Set the default to WriteBack (0x06)
	regVal.LowPart = (regVal.LowPart & ~0xFF) | 0x06;
	WRITEMSRREG( 0x02FF, regVal );
}

ArchHooks_Xbox::ArchHooks_Xbox()
{
	_set_new_handler(NoMemory);
	_set_new_mode(1);
	SetUnhandledExceptionFilter((LPTOP_LEVEL_EXCEPTION_FILTER) CheckPageFault);

	XGetCustomLaunchData();

	// mount A to DVD, C, E, F, G, X, and Y to the harddisk
	MountDrives();

	SetupNetwork();

	EnableExtraRAM();
}

ArchHooks_Xbox::~ArchHooks_Xbox()
{
	// We only want to reboot the Xbox in a software manner.
	XLaunchNewImage( NULL, NULL );
}

#include "RageFileManager.h"

void ArchHooks_Xbox::MountInitialFilesystems( const CString &sDirOfExecutable )
{
	FILEMAN->Mount( "dir", "D:\\", "/" );
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
