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

CString GetRegValue( HKEY hKey, CString sName )
{
	char    szBuffer[MAX_PATH];
	DWORD   nSize = sizeof(szBuffer)-1;
	if( RegQueryValueEx(hKey, sName, NULL, NULL, (LPBYTE)szBuffer, &nSize) == ERROR_SUCCESS ) 
		return szBuffer;
	else
		return "";
}

bool GetDebugInfoWin9x( VideoDriverInfo& infoOut )
{
	HKEY    hkey;
	if (RegOpenKey(HKEY_LOCAL_MACHINE, "SYSTEM\\CurrentControlSet\\Services\\Class\\Display\\0000", &hkey) !=  ERROR_SUCCESS)
		return false;

	infoOut.sDate =			GetRegValue( hkey, "DriverDate");
	infoOut.sDescription =	GetRegValue( hkey, "DriverDesc");
	infoOut.sDeviceID =		GetRegValue( hkey, "MatchingDeviceId");
	infoOut.sProvider =		GetRegValue( hkey, "ProviderName");
	infoOut.sVersion =		GetRegValue( hkey, "Ver");

	RegCloseKey(hkey);
	return true;
}


bool GetDebugInfoWinNT( VideoDriverInfo& infoOut )
{
	HKEY    hkey;
	if (RegOpenKey(HKEY_LOCAL_MACHINE, "SYSTEM\\CurrentControlSet\\Control\\Class\\{4D36E968-E325-11CE-BFC1-08002BE10318}\\0000", &hkey) !=  ERROR_SUCCESS)
		return false;

	infoOut.sDate =			GetRegValue( hkey, "DriverDate");
	infoOut.sDescription =	GetRegValue( hkey, "DriverDesc");
	infoOut.sDeviceID =		GetRegValue( hkey, "MatchingDeviceId");
	infoOut.sProvider =		GetRegValue( hkey, "ProviderName");
	infoOut.sVersion =		GetRegValue( hkey, "DriverVersion");

	RegCloseKey(hkey);
	return true;
}

void HandleKnownTerribleDriver( VideoDriverInfo info )
{
	struct ProblemAndBookmark
	{
		char szProblemProvider[1024];
		char szProblemDescription[1024];
		char szReadMeBookmark[64];
	};
	ProblemAndBookmark ENTRIES[] =
	{
		// Hacked around the bug in the V3 driver.   -Chris
//		{"3dfx Interactive, Inc. (Optimized by Amigamerlin)", "Voodoo3 PCI", "Voodoo3"},
		{"blah", "blah", "Voodoo3"},
	};
	const int NUM_ENTRIES = sizeof(ENTRIES) / sizeof(ENTRIES[0]);

	for( int i=0; i<NUM_ENTRIES; i++ )
	{
		if( info.sProvider == ENTRIES[i].szProblemProvider  &&
			info.sDescription == ENTRIES[i].szProblemDescription )
		{
			CString sQuestion = ssprintf(
				"Video Driver Information:\n\n"
				"Provider: %s\n"
				"Description: %s\n"
				"Version: %s\n"
				"Date: %s\n"
				"DeviceID: %s\n"
				"\n"
				"This video driver is known to have bugs that make StepMania unplayable.\n"
				"Click OK to see information on where to find a newer driver.\n"
				"Click Cancel to dismiss this warning and continue playing.",
				info.sProvider.GetString(),
				info.sDescription.GetString(),
				info.sVersion.GetString(),
				info.sDate.GetString(),
				info.sDeviceID.GetString() );
			if( IDOK == MessageBox(NULL, sQuestion, "Known problem driver", MB_ICONHAND|MB_OKCANCEL) )
			{
				char szBuffer[MAX_PATH];
				GetCurrentDirectory( MAX_PATH, szBuffer );
				GotoURL( ssprintf("%s/README-FIRST.html#%s", szBuffer, ENTRIES[i].szReadMeBookmark) );
				exit(1);	// Is there a better way to clean up? -Chris
			}
			else
				return;
		}
	}
}


static void GetDisplayDriverDebugInfo()
{
	VideoDriverInfo info;

	if( GetDebugInfoWin9x(info) )
		goto got_debug_info;
	if( GetDebugInfoWinNT(info) )
		goto got_debug_info;
	
	LOG->Warn( "Failed to get video card driver info." );
	return;

got_debug_info:

	LOG->Info("Video Driver Information:");
	LOG->Info("%-15s:\t%s", "Provider", info.sProvider.GetString());
	LOG->Info("%-15s:\t%s", "Description", info.sDescription.GetString());
	LOG->Info("%-15s:\t%s", "Version", info.sVersion.GetString());
	LOG->Info("%-15s:\t%s", "Date", info.sDate.GetString());
	LOG->Info("%-15s:\t%s", "DeviceID", info.sDeviceID.GetString());

	HandleKnownTerribleDriver( info );
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
