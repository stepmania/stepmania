#include "global.h"
#include "VideoDriverInfo.h"
#include "RageUtil.h"
#include "RageLog.h"

static void GetRegSubKeys( HKEY hKey, vector<CString> &lst )
{
	char szBuffer[MAX_PATH];
	DWORD nSize = sizeof(szBuffer)-1;
	FILETIME ft;

	for( int index = 0; ; ++index )
	{
		LONG ret = RegEnumKeyEx( hKey, index, szBuffer, &nSize, NULL, NULL, NULL, &ft);
		if( ret == ERROR_NO_MORE_ITEMS )
			return;

		if( ret != ERROR_SUCCESS && ret != ERROR_MORE_DATA )
		{
			LOG->Warn( werr_ssprintf(ret, "GetRegSubKeys(%p,%i) error", hKey, index) );
			return;
		}

		lst.push_back( szBuffer );
	}
}

static CString GetRegValue( HKEY hKey, CString sName )
{
	char    szBuffer[MAX_PATH];
	DWORD   nSize = sizeof(szBuffer)-1;
	if( RegQueryValueEx(hKey, sName, NULL, NULL, (LPBYTE)szBuffer, &nSize) == ERROR_SUCCESS ) 
		return szBuffer;
	else
		return "";
}


// this will not work on 95 and NT b/c of EnumDisplayDevices
CString GetPrimaryVideoName()
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

CString GetPrimaryVideoDriverName()
{
	const CString sPrimaryDeviceName = GetPrimaryVideoName();
	if( sPrimaryDeviceName != "" )
		return sPrimaryDeviceName;
	
	LOG->Warn("GetPrimaryVideoName failed; renderer selection may be wrong");

	VideoDriverInfo info;
	if( !GetVideoDriverInfo(0, info) )
		return "(ERROR DETECTING VIDEO DRIVER)";

	return info.sDescription;
}

/* Get info for the given card number.  Return false if that card doesn't exist. */
bool GetVideoDriverInfo(int cardno, VideoDriverInfo &info)
{
	OSVERSIONINFO version;
	version.dwOSVersionInfoSize=sizeof(version);
	GetVersionEx(&version);
	const bool bIsWin9x = version.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS;

	static bool Initialized=false;
	static vector<CString> lst;
	if( !Initialized )
	{
		Initialized = true;

		const CString TopKey = bIsWin9x?
			"SYSTEM\\CurrentControlSet\\Services\\Class\\Display":
			"SYSTEM\\CurrentControlSet\\Control\\Class\\{4D36E968-E325-11CE-BFC1-08002BE10318}";

		HKEY hkey;
		int ret = RegOpenKey(HKEY_LOCAL_MACHINE, TopKey, &hkey);
		if ( ret != ERROR_SUCCESS )
		{
			LOG->Warn( werr_ssprintf(ret, "RegOpenKey(%s) error", TopKey.c_str()) );
			return false;
		}

		GetRegSubKeys( hkey, lst );
		RegCloseKey( hkey );

		for( int i = lst.size()-1; i >= 0; --i )
		{
			/* Remove all keys that aren't four characters long ("Properties"). */
			if( lst[i].size() != 4 )
				lst.erase( lst.begin()+i );
			lst[i] = TopKey + "\\" + lst[i];
		}

		if ( lst.size() == 0 )
		{
			LOG->Warn("GetVideoDriverInfo error: no cards found!");
			return false;
		}
	}

	while( cardno < (int)lst.size() )
	{
		const CString sKey = lst[cardno];
		HKEY hkey;
		int ret = RegOpenKey(HKEY_LOCAL_MACHINE, sKey, &hkey);
		if ( ret != ERROR_SUCCESS )
		{
			/* Remove this one from the list and ignore it, */
			LOG->Warn( werr_ssprintf(ret, "RegOpenKey(%s) error", sKey.c_str()) );
			lst.erase( lst.begin()+cardno );
			continue;
		}

		info.sDate =			GetRegValue( hkey, "DriverDate");
		info.sDescription =		GetRegValue( hkey, "DriverDesc");
		info.sDeviceID =		GetRegValue( hkey, "MatchingDeviceId");
		info.sProvider =		GetRegValue( hkey, "ProviderName");
		info.sVersion =			GetRegValue( hkey, bIsWin9x? "Ver":"DriverVersion");

		RegCloseKey(hkey);
		return true;
	}

	return false;
}
