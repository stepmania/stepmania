#include "global.h"
#include "RageUtil.h"
#include "RageLog.h"
#include "io/USBDevice.h"

#include <cstdlib>
#include <map>
#include <usb.h>
#include <stdio.h>
#include "ProductInfo.h" // Used to look for PRODUCT_ID_BARE which means STEPMANIA 5, NOT OITG


#define FMT_BLOCK_SIZE		2048 // # of bytes to increment per try

#ifdef PRODUCT_ID_BARE
	#ifdef STDSTRING_H
		#define PSTRING RString
		#else
		#define PSTRING std::string
	#endif 
#else
	#define PSTRING CString
#endif
#define usbssprintf ssprintf



static PSTRING sClassDescriptions[] =
{
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
	"Data" // 10?  Shows up for my Motorola Razr
};

//USBDevice::USBDevice() {}

//USBDevice::~USBDevice() {}

/* This isn't as easy to do with libusb as it is with Unix/Linux.
* Format, from what I can tell:
*	"x-x" = bus, port
*	"x-x:x.x" bus, port, config, interface (multi-interface)
*	"x-x.x" bus, port, sub-port (child device)
*
* Please note that isn't implemented yet...
*
* XXX: I think "devnum" might be Linux-only. We need to check that out.
*/
PSTRING USBDevice::GetDeviceDir()
{
	LOG->Warn("Bus: %i, Device: %i", atoi(m_Device->bus->dirname), m_Device->devnum);
	PSTRING ret;
	char temp[1024];
	sprintf(temp,"%i-%i", atoi(m_Device->bus->dirname), m_Device->devnum);
	ret=temp;
	return ret;
}

PSTRING USBDevice::GetClassDescription(unsigned iClass)
{
	if (iClass == 255)
		return "Vendor";
	if (iClass > 10)
		return "Unknown";

	return sClassDescriptions[iClass];
}

PSTRING USBDevice::GetDescription()
{
	if( IsP3IO() )
		return "Input/lights controller";

	std::vector<std::string> sInterfaceDescriptions;

	for (unsigned i = 0; i < m_iInterfaceClasses.size(); i++)
		sInterfaceDescriptions.push_back(GetClassDescription(m_iInterfaceClasses[i]));
	return "";
	//return join( ", ", sInterfaceDescriptions );
}

/* Ugly... */
bool USBDevice::GetDeviceProperty(const PSTRING &sProperty, PSTRING &sOut)
{
	char temp[1024];
	//LOG->Trace("USBDevice::GetDeviceProperty(): %s", sProperty.c_str());
	if (sProperty == "idVendor")
		sprintf(temp, "%x", m_Device->descriptor.idVendor);
	else if (sProperty == "idProduct")
		sprintf(temp, "%x", m_Device->descriptor.idProduct);
	else if (sProperty == "bMaxPower")
		/* HACK: for some reason, MaxPower is returning half the actual value... */
		sprintf(temp, "%i", m_Device->config->MaxPower * 2);
	//	else if( sProperty == "bDeviceClass" )
	//		sOut = m_Device->descriptor.bDeviceClass;
	else
		return false;
	sOut=temp;
	return true;
}

/* XXX: doesn't get multiple interfaces like the Linux code does. */
bool USBDevice::GetInterfaceProperty(const PSTRING &sProperty, const unsigned iInterface, PSTRING &sOut)
{
	
	if ((signed)iInterface > m_Device->config->bNumInterfaces)
	{
		LOG->Warn("Cannot access interface %i with USBDevice interface count %i",
			iInterface, m_Device->config->bNumInterfaces);
		return false;
	}
	
	char temp[1024];
	if (sProperty == "bInterfaceClass")
		sprintf(temp,"%i", m_Device->config->interface->altsetting[iInterface].bInterfaceClass);
	else
		return false;
	sOut=temp;
	return true;
}

bool USBDevice::IsHub()
{
	PSTRING sClass;

	if (GetDeviceProperty("bDeviceClass", sClass) && atoi(sClass.c_str()) == 9)
		return true;

	for (unsigned i = 0; i < m_iInterfaceClasses.size(); i++)
		if (m_iInterfaceClasses[i] == 9)
			return true;

	return false;
}

bool USBDevice::IsP3IO()
{
	return P3IO::DeviceMatches( m_iIdVendor, m_iIdProduct );
}

bool USBDevice::Load(struct usb_device *dev)
{
	m_Device = dev;

	if (m_Device == NULL)
	{
		LOG->Warn("Invalid usb_device passed to Load().");
		return false;
	}

	PSTRING buf;

	if (GetDeviceProperty("idVendor", buf))
		sscanf(buf.c_str(), "%x", &m_iIdVendor);
	else
		m_iIdVendor = -1;

	if (GetDeviceProperty("idProduct", buf))

		sscanf(buf.c_str(), "%x", &m_iIdProduct);
	else
		m_iIdProduct = -1;

	if (GetDeviceProperty("bMaxPower", buf))
		sscanf(buf.c_str(), "%imA", &m_iMaxPower);
	else
		m_iMaxPower = -1;

	if (m_iIdVendor == -1 || m_iIdProduct == -1 || m_iMaxPower == -1)
	{
		LOG->Warn("Could not load USBDevice");
		return false;
	}


	for (int i = 0; i < m_Device->config->interface->num_altsetting; i++)
	{
		int iClass;
		if (GetInterfaceProperty("bInterfaceClass", i, buf))
			sscanf(buf.c_str(), "%i", &iClass);
		else
		{
			LOG->Warn("Could not read interface %i.", i);
			iClass = -1;
		}

		m_iInterfaceClasses.push_back(iClass);
	}

	return true;
}

// this is the diary of a mad man
bool GetUSBDeviceList(std::vector<USBDevice> &pDevList)
{
	usb_init();
	usb_find_busses();
	usb_find_devices();

	std::vector<struct usb_device> vDevices;
	std::vector<PSTRING>	vDeviceDirs;

	/* get all devices on the system */
	for (struct usb_bus *bus = usb_get_busses(); bus; bus = bus->next)
	{
		if (bus->root_dev)
			vDevices.push_back(*bus->root_dev);
		else
			for (struct usb_device *dev = bus->devices; dev; dev->next)
				vDevices.push_back(*dev);
	}

	/* get their children - in other cases, this could be an accidental
	* infinite loop, but here, it works out nicely as a recursion process. */
	for (unsigned i = 0; i < vDevices.size(); i++)
		for (unsigned j = 0; j < vDevices[i].num_children; j++)
			if (vDevices[i].children[j])
				vDevices.push_back(*vDevices[i].children[j]);

	for (unsigned i = 0; i < vDevices.size(); i++)
	{
		USBDevice newDev;

		if (vDevices[i].descriptor.idVendor == 0 || vDevices[i].descriptor.idProduct == 0) continue;

		if (newDev.Load(&vDevices[i]))
			pDevList.push_back(newDev);
		else
			if (vDevices[i].descriptor.idVendor != 0 && vDevices[i].descriptor.idProduct != 0)
				LOG->Warn("Loading failed: %x, %x", vDevices[i].descriptor.idVendor, vDevices[i].descriptor.idProduct);
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
