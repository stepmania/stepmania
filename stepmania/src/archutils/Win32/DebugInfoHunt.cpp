#include "global.h"
#include "DebugInfoHunt.h"
#include "RageLog.h"
#include "RageUtil.h"


void PrintDebugInfoWin9x()
{
	HKEY    hkey;
	if (RegOpenKey(HKEY_LOCAL_MACHINE, "SYSTEM\\CurrentControlSet\\Services\\Class\\Display\\0000", &hkey) !=  ERROR_SUCCESS)
		return;

	char    szBuffer[MAX_PATH];
	DWORD   nSize;

#define GETSZ2(a) 	nSize = sizeof(szBuffer)-1;	if( RegQueryValueEx(hkey, a, NULL, NULL, (LPBYTE)szBuffer, &nSize) == ERROR_SUCCESS ) LOG->Info("%-15s:\t%s", a, szBuffer);

	LOG->Info("Video Driver Information (win9x):");
    GETSZ2("DriverDate");
    GETSZ2("DriverDesc");
    GETSZ2("MatchingDeviceId");
    GETSZ2("ProviderName");
    GETSZ2("Ver");

	RegCloseKey(hkey);
}


void PrintDebugInfoWinNT()
{
	HKEY    hkey;
	if (RegOpenKey(HKEY_LOCAL_MACHINE, "SYSTEM\\CurrentControlSet\\Control\\Class\\{4D36E968-E325-11CE-BFC1-08002BE10318}\\0000", &hkey) !=  ERROR_SUCCESS)
		return;

	char    szBuffer[MAX_PATH];
	DWORD   nSize;

#define GETSZ2(a) 	nSize = sizeof(szBuffer)-1;	if( RegQueryValueEx(hkey, a, NULL, NULL, (LPBYTE)szBuffer, &nSize) == ERROR_SUCCESS ) LOG->Info("%-15s:\t%s", a, szBuffer);

	LOG->Info("Video Driver Information (winnt):");
    GETSZ2("DriverDate");
    GETSZ2("DriverDesc");
    GETSZ2("DriverVersion");
    GETSZ2("InfSection");
    GETSZ2("ProviderName");
    GETSZ2("MatchingDeviceId");

	RegCloseKey(hkey);
}

static void GetDisplayDriverDebugInfo()
{
	PrintDebugInfoWin9x();	// will print nothing if not 9x
	PrintDebugInfoWinNT();  // will print nothing if not NT
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
