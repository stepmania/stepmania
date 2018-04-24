#ifndef IO_USBDEVICE_H
#define IO_USBDEVICE_H

#include "global.h"
#include "ProductInfo.h" // Used to look for PRODUCT_ID_BARE which means STEPMANIA 5, NOT OITG

/* For "DeviceMatches" */
#include "P3IO.h"

#include <usb.h>
#ifdef PRODUCT_ID_BARE
#ifdef STDSTRING_H
#define PSTRING RString
#else
#define PSTRING std::string
#endif 
#else
#define PSTRING CString
#endif

// XXX: we probably need to move this to arch or archutils in preparation for windows portability
//           --infamouspat

// I concur. I'll work on this for alpha 4. -- Vyhd

/* A class we can use to characterize all USB devices on the system */
class USBDevice
{
public:
	//	USBDevice();
	//	~USBDevice();

	bool Load(const PSTRING &nDeviceDir, const std::vector<PSTRING> &interfaces);
	bool Load(struct usb_device *dev);

	bool GetDeviceProperty(const PSTRING &sProperty, PSTRING &out);
	bool GetInterfaceProperty(const PSTRING &sProperty, const unsigned iInterface, PSTRING &out);
	PSTRING GetClassDescription(unsigned iClass);
	PSTRING GetDescription();

	int GetIdVendor() { return m_iIdVendor; }
	int GetIdProduct() { return m_iIdProduct; }
	int GetMaxPower() { return m_iMaxPower; }

	/* This can't be inlined - Libusb has a more complex implementation */
	PSTRING GetDeviceDir();

	bool IsHub();
	bool IsP3IO();

private:
	int m_iIdVendor;
	int m_iIdProduct;
	int m_iMaxPower;
	PSTRING m_sDeviceDir;

	/* USBDevice.cpp doesn't use this, but USBDevice_Libusb.cpp does. */
	struct usb_device *m_Device;

	std::vector<PSTRING> m_sInterfaceDeviceDirs;
	std::vector<unsigned> m_iInterfaceClasses;


};

bool GetUSBDeviceList(std::vector<USBDevice> &pDevList);

#endif /* IO_USBDEVICE_H */

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
