#include "global.h"
#include "DebugInfoHunt.h"
#include "RageLog.h"
#include "RageUtil.h"
#include "../../archutils/Win32/GotoURL.h"


struct VideoDriverInfo
{
	CString sProvider;
	CString sDescription;
	CString sVersion;
	CString sDate;
	CString sDeviceID;
};


void LogVideoDriverInfo( VideoDriverInfo info )
{
	LOG->Info("Video Driver Information:");
	LOG->Info("%-15s:\t%s", "Provider", info.sProvider.GetString());
	LOG->Info("%-15s:\t%s", "Description", info.sDescription.GetString());
	LOG->Info("%-15s:\t%s", "Version", info.sVersion.GetString());
	LOG->Info("%-15s:\t%s", "Date", info.sDate.GetString());
	LOG->Info("%-15s:\t%s", "DeviceID", info.sDeviceID.GetString());
}

CString GetRegValue( HKEY hKey, CString sName )
{
	char    szBuffer[MAX_PATH];
	DWORD   nSize = sizeof(szBuffer)-1;
	if( RegQueryValueEx(hKey, sName, NULL, NULL, (LPBYTE)szBuffer, &nSize) == ERROR_SUCCESS ) 
		return szBuffer;
	else
		return "";
}

bool GetVideoDriverInfo9x( int iIndex, VideoDriverInfo& infoOut )
{
	HKEY    hkey;
	CString sKey = ssprintf("SYSTEM\\CurrentControlSet\\Services\\Class\\Display\\%04d",iIndex);
	if (RegOpenKey(HKEY_LOCAL_MACHINE, sKey, &hkey) !=  ERROR_SUCCESS)
		return false;

	infoOut.sDate =			GetRegValue( hkey, "DriverDate");
	infoOut.sDescription =	GetRegValue( hkey, "DriverDesc");
	infoOut.sDeviceID =		GetRegValue( hkey, "MatchingDeviceId");
	infoOut.sProvider =		GetRegValue( hkey, "ProviderName");
	infoOut.sVersion =		GetRegValue( hkey, "Ver");

	RegCloseKey(hkey);
	return true;
}


bool GetVideoDriverInfo2k( int iIndex, VideoDriverInfo& infoOut )
{
	HKEY    hkey;
	CString sKey = ssprintf("SYSTEM\\CurrentControlSet\\Control\\Class\\{4D36E968-E325-11CE-BFC1-08002BE10318}\\%04d",iIndex);
	if (RegOpenKey(HKEY_LOCAL_MACHINE, sKey, &hkey) !=  ERROR_SUCCESS)
		return false;

	infoOut.sDate =			GetRegValue( hkey, "DriverDate");
	infoOut.sDescription =	GetRegValue( hkey, "DriverDesc");
	infoOut.sDeviceID =		GetRegValue( hkey, "MatchingDeviceId");
	infoOut.sProvider =		GetRegValue( hkey, "ProviderName");
	infoOut.sVersion =		GetRegValue( hkey, "DriverVersion");

	RegCloseKey(hkey);
	return true;
}


CString GetPrimaryVideoName9xAnd2k()	// this will not work on 95 and NT b/c of EnumDisplayDevices
{
	
    typedef BOOL (WINAPI* pfnEnumDisplayDevices)(PVOID,DWORD,PDISPLAY_DEVICE,DWORD);
	pfnEnumDisplayDevices EnumDisplayDevices;
    HINSTANCE  hInstUser32;
    
    hInstUser32 = LoadLibrary("User32.DLL");
    if( !hInstUser32 ) 
		return "";  

	// VC6 don't have a stub to static link with, so link dynamically.
	EnumDisplayDevices = (pfnEnumDisplayDevices)GetProcAddress(hInstUser32,"EnumDisplayDevicesA");
    if( EnumDisplayDevices == NULL )
	{
        FreeLibrary(hInstUser32);
        return "";
    }
	
	CString sPrimaryDeviceName;
	for( DWORD i=0; true; i++ )
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

    FreeLibrary(hInstUser32);
	return sPrimaryDeviceName;
}




static void GetDisplayDriverDebugInfo()
{
	CString sPrimaryDeviceName = GetPrimaryVideoName9xAnd2k();
	
	if( sPrimaryDeviceName == "" )
		LOG->Info( "Primary display driver could not be determined." );
	else
		LOG->Info( "Primary display driver: %s", sPrimaryDeviceName.GetString() );

	OSVERSIONINFO version;
	version.dwOSVersionInfoSize=sizeof(version);
	GetVersionEx(&version);
	bool bIsWin9x = version.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS;

	for( int i=0; true; i++ )
	{
		VideoDriverInfo info;
		if( ! (bIsWin9x ? GetVideoDriverInfo9x : GetVideoDriverInfo2k)(i,info) )
			break;
		
		if( sPrimaryDeviceName == "" )	// failed to get primary disaply name (NT4)
		{
			LogVideoDriverInfo( info );
		}
		else if( info.sDescription == sPrimaryDeviceName )
		{
			LogVideoDriverInfo( info );
			break;
		}
	}
}

static CString wo_ssprintf( MMRESULT err, const char *fmt, ...)
{
	char buf[MAXERRORLENGTH];
	waveOutGetErrorText(err, buf, MAXERRORLENGTH);

    va_list	va;
    va_start(va, fmt);
    CString s = vssprintf( fmt, va );
    va_end(va);

	return s += ssprintf( "(%s)", buf );
}

static void GetSoundDriverDebugInfo()
{
	int cnt = waveOutGetNumDevs();

	for(int i = 0; i < cnt; ++i)
	{
		WAVEOUTCAPS caps;
	
		MMRESULT ret = waveOutGetDevCaps(i, &caps, sizeof(caps));
		if(ret != MMSYSERR_NOERROR)
		{
			LOG->Info(wo_ssprintf(ret, "waveOutGetDevCaps(%i) failed", i));
			continue;
		}
		LOG->Info("Sound device %i: %s, %i.%i, MID %i, PID %i %s", i, caps.szPname,
			HIBYTE(caps.vDriverVersion),
			LOBYTE(caps.vDriverVersion),
			caps.wMid, caps.wPid,
			caps.dwSupport & WAVECAPS_SAMPLEACCURATE? "":"(INACCURATE)");
	}
}

void SearchForDebugInfo()
{
	GetDisplayDriverDebugInfo();
	GetSoundDriverDebugInfo();

	/* Detect operating system. */
	OSVERSIONINFO ovi;
	ovi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
	if (GetVersionEx(&ovi)) {
		CString Ver = ssprintf("Windows %i.%i (", ovi.dwMajorVersion, ovi.dwMinorVersion);
		if(ovi.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS)
		{
			if(ovi.dwMinorVersion == 0)
				Ver += "Win95";
			else if(ovi.dwMinorVersion == 10)
				Ver += "Win98";
			else if(ovi.dwMinorVersion == 90)
				Ver += "WinME";
			else 
				Ver += "unknown 9x-based";
		}
		else if(ovi.dwPlatformId == VER_PLATFORM_WIN32_NT)
		{
			if(ovi.dwMajorVersion == 4 && ovi.dwMinorVersion == 0)
				Ver += "WinNT 4.0";
			else if(ovi.dwMajorVersion == 5 && ovi.dwMinorVersion == 0)
				Ver += "Win2000";
			else if(ovi.dwMajorVersion == 5 && ovi.dwMinorVersion == 1)
				Ver += "WinXP";
			else
				Ver += "unknown NT-based";
		} else Ver += "???";

		Ver += ssprintf(") build %i [%s]", ovi.dwBuildNumber & 0xffff, ovi.szCSDVersion);
		LOG->Info("%s", Ver.GetString());
	}
}

/*
 * Copyright (c) 2003 by the person(s) listed below.  All rights reserved.
 *
 * Glenn Maynard
 */
