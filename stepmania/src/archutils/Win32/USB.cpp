#include "global.h"
#include "USB.h"
#include "RageLog.h"
#include "RageUtil.h"

#pragma comment(lib, "ddk/setupapi.lib") 
#pragma comment(lib, "ddk/hid.lib") 

extern "C" {
#include "ddk/setupapi.h"
/* Quiet header warning: */
#pragma warning( push )
#pragma warning (disable : 4201)
#include "ddk/hidsdi.h"
#pragma warning( pop )
}

static char *GetUSBDevicePath (int num)
{
    GUID guid;
    HidD_GetHidGuid(&guid);

    HDEVINFO DeviceInfo = SetupDiGetClassDevs (&guid,
                 NULL, NULL, (DIGCF_PRESENT | DIGCF_DEVICEINTERFACE));

    SP_DEVICE_INTERFACE_DATA DeviceInterface;
    DeviceInterface.cbSize = sizeof(SP_DEVICE_INTERFACE_DATA);

    char *ret = NULL;
    PSP_INTERFACE_DEVICE_DETAIL_DATA DeviceDetail = NULL;

    if (!SetupDiEnumDeviceInterfaces (DeviceInfo,
               NULL, &guid, num, &DeviceInterface))
		goto err;

    unsigned long size;
    SetupDiGetDeviceInterfaceDetail (DeviceInfo, &DeviceInterface, NULL, 0, &size, 0);

    DeviceDetail = (PSP_INTERFACE_DEVICE_DETAIL_DATA) malloc(size);
    DeviceDetail->cbSize = sizeof(SP_INTERFACE_DEVICE_DETAIL_DATA);

    if (SetupDiGetDeviceInterfaceDetail (DeviceInfo, &DeviceInterface,
		DeviceDetail, size, &size, NULL)) 
    {
        ret = strdup(DeviceDetail->DevicePath);
    }

err:
    SetupDiDestroyDeviceInfoList (DeviceInfo);
    free (DeviceDetail);
    return ret;
}

static HANDLE OpenUSB (int VID, int PID, int num)
{
    DWORD index = 0;

    char *path;
	HANDLE h = INVALID_HANDLE_VALUE;

    while ((path = GetUSBDevicePath (index++)) != NULL)
    {
		if(h != INVALID_HANDLE_VALUE)
			CloseHandle (h);

		h = CreateFile (path, GENERIC_READ,
			   FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_FLAG_OVERLAPPED, NULL);

		free(path);

		if(h == INVALID_HANDLE_VALUE)
			continue;

		HIDD_ATTRIBUTES attr;
		if (!HidD_GetAttributes (h, &attr))
			continue;

        if ((VID != -1 && attr.VendorID != VID) &&
            (PID != -1 && attr.ProductID != PID))
			continue; /* This isn't it. */

		/* The VID and PID match. */
		if(num-- == 0)
            return h;
    }
	if(h != INVALID_HANDLE_VALUE)
		CloseHandle (h);

    return INVALID_HANDLE_VALUE;
}

USBDevice::USBDevice()
{
	ZeroMemory( &ov, sizeof(ov) );
	pending=false;
	h = INVALID_HANDLE_VALUE;
}

USBDevice::~USBDevice()
{
	if(h != INVALID_HANDLE_VALUE)
		CloseHandle(h);
}

bool USBDevice::Open(int VID, int PID, int num)
{
	h = OpenUSB (VID, PID, num);
	return h != INVALID_HANDLE_VALUE;
}

int USBDevice::GetPadEvent()
{
	if(h == INVALID_HANDLE_VALUE)
		return -1;

    int ret;

    if(!pending)
    {
		/* Request feedback from the device. */
	    unsigned long r;
    	ret = ReadFile(h, &buf, sizeof(buf), &r, &ov);
    	pending=true;
    }

	/* See if we have a response for our request (which we may
	 * have made on a previous call): */
    if(WaitForSingleObjectEx(h, 0, TRUE) == WAIT_TIMEOUT)
		return -1;
    
	/* We do; get the result.  It'll go into the original &buf
	 * we supplied on the original call; that's why buf is a
	 * member instead of a local. */
    unsigned long cnt;
    ret = GetOverlappedResult(h, &ov, &cnt, FALSE);
    pending=false;

    if(ret == 0 && (GetLastError() == ERROR_IO_PENDING || GetLastError() == ERROR_IO_INCOMPLETE))
		return -1;

    if(ret == 0) {
		// this prints too much info in Win98
		// See if it's fixed--h should be INVALID_HANDLE_VALUE if
		// the pad isn't there. -glenn
		LOG->Warn(werr_ssprintf(GetLastError(), "Error reading Pump pad"));
	    return -1;
    }

    return buf;
}

/*
-----------------------------------------------------------------------------
 Copyright (c) 2002-2003 by the person(s) listed below.  All rights reserved.
	Glenn Maynard
-----------------------------------------------------------------------------
*/
