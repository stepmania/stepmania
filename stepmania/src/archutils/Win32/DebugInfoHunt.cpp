#include "stdafx.h"
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

void SearchForDebugInfo()
{
	GetDisplayDriverDebugInfo();
}

/*
 * Copyright (c) 2003 by the person(s) listed below.  All rights reserved.
 *
 * Glenn Maynard
 */
