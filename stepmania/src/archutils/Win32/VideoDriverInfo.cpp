#include "global.h"
#include "VideoDriverInfo.h"
#include "RageUtil.h"
#include "RageLog.h"

static CString GetRegValue( HKEY hKey, CString sName )
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


// this will not work on 95 and NT b/c of EnumDisplayDevices
CString GetPrimaryVideoName9xAnd2k()
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

CString GetPrimaryVideoDriverInfo()
{
	const CString sPrimaryDeviceName = GetPrimaryVideoName9xAnd2k();
	if( sPrimaryDeviceName != "" )
		return sPrimaryDeviceName;
	
	LOG->Warn("GetPrimaryVideoName9xAnd2k failed; renderer selection may be wrong");

	VideoDriverInfo info;
	if( !GetVideoDriverInfo(0, info) )
		RageException::Throw( "GetVideoDriverInfo(0) failed");

	return info.sDescription;
}

/* Get info for the given card number.  Return false if that card doesn't exist. */
bool GetVideoDriverInfo(int cardno, VideoDriverInfo &info)
{
	OSVERSIONINFO version;
	version.dwOSVersionInfoSize=sizeof(version);
	GetVersionEx(&version);
	const bool bIsWin9x = version.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS;

	return (bIsWin9x ? GetVideoDriverInfo9x : GetVideoDriverInfo2k)(cardno,info);
}
