#include "global.h"
#include "RageUtil.h"
#include "RageLog.h"

#include "io/USBDevice.h"
#include "io/PIUIO.h"
#include "io/ITGIO.h"
#include "io/MiniMaid.h"
#include "io/P3IO.h"

#include <map>
#include <usb.h>

static PSTRING sClassDescriptions[] = {
	"Unknown", // 0
	"Audio", // 1
	"Communications", // 2
	"Input", // 3
	"Unknown", // 4
	"Unknown", // 5
	"Camera", // 6
	"Printer", // 7
	"Storage", // 8
	"Hub", // 9
	"Data" }; // 10?  Shows up for my Motorola Razr

//USBDevice::USBDevice() {}

//USBDevice::~USBDevice() {}

PSTRING USBDevice::GetClassDescription( unsigned iClass )
{
	if ( iClass == 255 ) return "Vendor";
	if ( iClass > 10)
		return "Unknown";
	return sClassDescriptions[iClass];
}

PSTRING USBDevice::GetDescription()
{
	if( IsITGIO() || IsPIUIO() || IsMiniMaid() || IsP3IO() )
		return "Input/lights controller";
	
	vector<PSTRING> sInterfaceDescriptions;

	for (unsigned i = 0; i < m_iInterfaceClasses.size(); i++)
		sInterfaceDescriptions.push_back( GetClassDescription(m_iInterfaceClasses[i]) );

	return join( ", ", sInterfaceDescriptions );
}

bool USBDevice::GetDeviceProperty( const PSTRING &sProperty, PSTRING &out )
{
	PSTRING sTargetFile = "/rootfs/sys/bus/usb/devices/" + m_sDeviceDir + "/" + sProperty;
	return GetFileContents(sTargetFile, out, true);
}

bool USBDevice::GetInterfaceProperty( const PSTRING &sProperty, const unsigned iInterface, PSTRING &out)
{
	if (iInterface > m_sInterfaceDeviceDirs.size() - 1)
	{
		LOG->Warn( "Cannot access interface %i with USBDevice interface count %i", iInterface, m_sInterfaceDeviceDirs.size() );
		return false;
	}
	PSTRING sTargetFile = "/rootfs/sys/bus/usb/devices/" + m_sDeviceDir + ":" + m_sInterfaceDeviceDirs[iInterface] + "/" + sProperty;
	return GetFileContents( sTargetFile, out, true );
}

PSTRING USBDevice::GetDeviceDir()
{
	return m_sDeviceDir;
}

bool USBDevice::IsHub()
{
	for (unsigned i = 0; i < m_iInterfaceClasses.size(); i++)
		if (m_iInterfaceClasses[i] == 9)
			return true;

	return false;
}

bool USBDevice::IsITGIO()
{
	return ITGIO::DeviceMatches( m_iIdVendor, m_iIdProduct );
}

bool USBDevice::IsPIUIO()
{
	return PIUIO::DeviceMatches( m_iIdVendor, m_iIdProduct );
}

bool USBDevice::IsMiniMaid()
{
	return MiniMaid::DeviceMatches( m_iIdVendor, m_iIdProduct );
}

bool USBDevice::IsP3IO()
{
	return P3IO::DeviceMatches( m_iIdVendor, m_iIdProduct );
}

bool USBDevice::Load(const PSTRING &nDeviceDir, const vector<PSTRING> &interfaces)
{
	m_sDeviceDir = nDeviceDir;
	m_sInterfaceDeviceDirs = interfaces;
	PSTRING buf;

	if (GetDeviceProperty("idVendor", buf))
		sscanf(buf, "%x", &m_iIdVendor);
	else
		m_iIdVendor = -1;

	if (GetDeviceProperty("idProduct", buf))
		sscanf(buf, "%x", &m_iIdProduct);
	else
		m_iIdProduct = -1;

	if (GetDeviceProperty("bMaxPower", buf))
		sscanf(buf, "%imA", &m_iMaxPower);
	else
		m_iMaxPower = -1;

	if (m_iIdVendor == -1 || m_iIdProduct == -1 || m_iMaxPower == -1)
	{
		LOG->Warn( "Could not load USBDevice %s", nDeviceDir.c_str() );
		return false;
	}

	for (unsigned i = 0; i < m_sInterfaceDeviceDirs.size(); i++)
	{
		int iClass;
		if ( GetInterfaceProperty( "bInterfaceClass", i, buf ) )
		{
			sscanf( buf, "%x", &iClass );
		}
		else
		{
			//LOG->Warn("Could not read interface %i for %s:%s", i, m_sDeviceDir.c_str(), m_sInterfaceDeviceDirs[i].c_str() );
			iClass = -1;
		}
		m_iInterfaceClasses.push_back(iClass);
	}
	return true;
}

// this is the diary of a mad man
bool GetUSBDeviceList(vector<USBDevice> &pDevList)
{
	FlushDirCache();

	std::map< PSTRING, vector<PSTRING> > sDevInterfaceList;
	vector<PSTRING> sDirList;
	GetDirListing( "/rootfs/sys/bus/usb/devices/", sDirList, true, false );
	for (unsigned i = 0; i < sDirList.size(); i++)
	{
		PSTRING sDirEntry = sDirList[i];
		vector<PSTRING> components;

		if (sDirEntry.substr(0, 3) == "usb") continue;

		split( sDirEntry, ":", components, true );
		if ( components.size() < 2 ) continue;

		if ( ! IsADirectory( "/rootfs/sys/bus/usb/devices/" + components[0] ) ) continue;

		// I win --infamouspat
		sDevInterfaceList[components[0]].push_back(components[1]);

	}

	map< PSTRING, vector<PSTRING> >::iterator iter;
 
	for(iter = sDevInterfaceList.begin(); iter != sDevInterfaceList.end(); iter++)
	{
		USBDevice newDev;
		PSTRING sDevName = iter->first;
		vector<PSTRING> sDevChildren = iter->second;
		
		if ( newDev.Load(sDevName, sDevChildren) )
			pDevList.push_back(newDev);
	}

	return true;
}

/*
 * Copyright (c) 2008 BoXoRRoXoRs
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
