#include "global.h"
#include "DebugInfoHunt.h"
#include "RageLog.h"
#include "RageUtil.h"

#include <d3d8.h>
#pragma comment(lib, "d3d8.lib")


/*
 * The only way I've found to get the display driver version is through
 * D3D.  (There's an interface in DirectDraw, but it always returned 0.)
 * Make sure this doesn't break if D3D isn't available!
 */
static void GetDisplayDriverDebugInfo()
{
	if (FAILED(CoInitialize(NULL)))
		return;

	IDirect3D8 *m_pd3d = NULL;
	try
	{
		// Construct a Direct3D object
		m_pd3d = Direct3DCreate8( D3D_SDK_VERSION );
	}
	catch (...) 
	{
	}

    if( m_pd3d == NULL )
	{
		LOG->Info("Couldn't get video driver info (no d3d)");
		return;
	}

	HRESULT  hr;

	D3DADAPTER_IDENTIFIER8 id;
	hr = m_pd3d->GetAdapterIdentifier(D3DADAPTER_DEFAULT, D3DENUM_NO_WHQL_LEVEL, &id);
	if(FAILED(hr))
	{
		LOG->Info(hr_ssprintf(hr, "Couldn't get video driver info"));
		return;
	}

	LOG->Info("Video description: %s", id.Description);
	LOG->Info("Chipset rev: %i  Vendor ID: %i  Device ID: %i", id.Revision, id.VendorId, id.DeviceId);
	LOG->Info("Video driver: %s %i.%i.%i", id.Driver,
		LOWORD(id.DriverVersion.HighPart),
		HIWORD(id.DriverVersion.LowPart), 
		LOWORD(id.DriverVersion.LowPart));
	LOG->Info("Video ID: {%8.8x-%4.4x-%4.4x-%6.6x}",
		id.DeviceIdentifier.Data1,
		id.DeviceIdentifier.Data2,
		id.DeviceIdentifier.Data3,
		id.DeviceIdentifier.Data4);

	CoUninitialize();
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
