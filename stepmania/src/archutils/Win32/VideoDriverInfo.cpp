#include "global.h"
#include "VideoDriverInfo.h"
#include "RageUtil.h"
#include "RageLog.h"
#include "RegistryAccess.h"
#include <windows.h>

// this will not work on 95 and NT because of EnumDisplayDevices
RString GetPrimaryVideoName()
{
    typedef BOOL (WINAPI* pfnEnumDisplayDevices)(PVOID,DWORD,PDISPLAY_DEVICE,DWORD);
	pfnEnumDisplayDevices EnumDisplayDevices;
    HINSTANCE hInstUser32;
    
    hInstUser32 = LoadLibrary( "User32.DLL" );
    if( !hInstUser32 ) 
		return RString();  

	// VC6 don't have a stub to static link with, so link dynamically.
	EnumDisplayDevices = (pfnEnumDisplayDevices)GetProcAddress(hInstUser32,"EnumDisplayDevicesA");
    if( EnumDisplayDevices == NULL )
	{
        FreeLibrary(hInstUser32);
        return RString();
    }
	
	RString sPrimaryDeviceName;
	for( int i=0; true; ++i )
	{
		DISPLAY_DEVICE dd;
		ZERO( dd );
		dd.cb = sizeof(dd);
		if( !EnumDisplayDevices(NULL, i, &dd, 0) )
			break;
		if( dd.StateFlags & DISPLAY_DEVICE_PRIMARY_DEVICE )
		{
			sPrimaryDeviceName = (char*)dd.DeviceString;
			break;
		}
	}

    FreeLibrary( hInstUser32 );
	return sPrimaryDeviceName;
}

RString GetPrimaryVideoDriverName()
{
	const RString sPrimaryDeviceName = GetPrimaryVideoName();
	if( sPrimaryDeviceName != "" )
		return sPrimaryDeviceName;
	
	LOG->Warn("GetPrimaryVideoName failed; renderer selection may be wrong");

	VideoDriverInfo info;
	if( !GetVideoDriverInfo(0, info) )
		return "(ERROR DETECTING VIDEO DRIVER)";

	return info.sDescription;
}

/* Get info for the given card number.  Return false if that card doesn't exist. */
bool GetVideoDriverInfo( int iCardno, VideoDriverInfo &info )
{
	OSVERSIONINFO version;
	version.dwOSVersionInfoSize = sizeof(version);
	GetVersionEx(&version);
	const bool bIsWin9x = version.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS;

	static bool bInitialized=false;
	static vector<RString> lst;
	if( !bInitialized )
	{
		bInitialized = true;

		const RString sTopKey = bIsWin9x?
			"HKEY_LOCAL_MACHINE\\SYSTEM\\CurrentControlSet\\Services\\Class\\Display":
			"HKEY_LOCAL_MACHINE\\SYSTEM\\CurrentControlSet\\Control\\Class\\{4D36E968-E325-11CE-BFC1-08002BE10318}";

		RegistryAccess::GetRegSubKeys( sTopKey, lst, ".*", false );

		for( int i=lst.size()-1; i >= 0; --i )
		{
			/* Remove all keys that aren't four characters long ("Properties"). */
			if( lst[i].size() != 4 )
			{
				lst.erase( lst.begin()+i );
				continue;
			}

			lst[i] = sTopKey + "\\" + lst[i];
		}

		if( lst.size() == 0 )
		{
			LOG->Warn("GetVideoDriverInfo error: no cards found!");
			return false;
		}
	}

	while( iCardno < (int)lst.size() )
	{
		const RString sKey = lst[iCardno];

		if( !RegistryAccess::GetRegValue( sKey, "DriverDesc", info.sDescription ) )
		{
			/* Remove this one from the list and ignore it, */
			lst.erase( lst.begin()+iCardno );
			continue;
		}

		RegistryAccess::GetRegValue( sKey, "DriverDate", info.sDate );
		RegistryAccess::GetRegValue( sKey, "MatchingDeviceId", info.sDeviceID );
		RegistryAccess::GetRegValue( sKey, "ProviderName", info.sProvider );
		RegistryAccess::GetRegValue( sKey, bIsWin9x? "Ver":"DriverVersion", info.sVersion );

		return true;
	}

	return false;
}

/*
 * (c) 2002-2004 Chris Danford, Glenn Maynard
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
