#include "global.h"
#include "USB.h"
#include "RageLog.h"
#include "RageUtil.h"

#pragma comment(lib, "archutils/Win32/ddk/setupapi.lib") 
#pragma comment(lib, "archutils/Win32/ddk/hid.lib") 

extern "C" {
#include "archutils/Win32/ddk/setupapi.h"
/* Quiet header warning: */
#include "archutils/Win32/ddk/hidsdi.h"
}

static CString GetUSBDevicePath (int num)
{
    GUID guid;
    HidD_GetHidGuid(&guid);

    HDEVINFO DeviceInfo = SetupDiGetClassDevs (&guid,
                 NULL, NULL, (DIGCF_PRESENT | DIGCF_DEVICEINTERFACE));

    SP_DEVICE_INTERFACE_DATA DeviceInterface;
    DeviceInterface.cbSize = sizeof(SP_DEVICE_INTERFACE_DATA);

    if (!SetupDiEnumDeviceInterfaces (DeviceInfo,
               NULL, &guid, num, &DeviceInterface))
	{
	    SetupDiDestroyDeviceInfoList (DeviceInfo);
	    return "";
	}

    unsigned long size;
    SetupDiGetDeviceInterfaceDetail (DeviceInfo, &DeviceInterface, NULL, 0, &size, 0);

    PSP_INTERFACE_DEVICE_DETAIL_DATA DeviceDetail = (PSP_INTERFACE_DEVICE_DETAIL_DATA) malloc(size);
    DeviceDetail->cbSize = sizeof(SP_INTERFACE_DEVICE_DETAIL_DATA);

    CString ret;
    if (SetupDiGetDeviceInterfaceDetail (DeviceInfo, &DeviceInterface,
		DeviceDetail, size, &size, NULL)) 
        ret = DeviceDetail->DevicePath;
	free(DeviceDetail);

	SetupDiDestroyDeviceInfoList (DeviceInfo);
    return ret;
}


bool USBDevice::Open(int VID, int PID, int blocksize, int num)
{
    DWORD index = 0;

    CString path;
    while ((path = GetUSBDevicePath (index++)) != "")
    {
		HANDLE h = CreateFile (path, GENERIC_READ,
			FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);

		if(h == INVALID_HANDLE_VALUE)
			continue;

		HIDD_ATTRIBUTES attr;
		if (!HidD_GetAttributes (h, &attr))
		{
			CloseHandle(h);
			continue;
		}
		CloseHandle(h);

        if ((VID != -1 && attr.VendorID != VID) &&
            (PID != -1 && attr.ProductID != PID))
			continue; /* This isn't it. */

		/* The VID and PID match. */
		if(num-- > 0)
			continue;

		io.Open(path, blocksize);
        return true;
    }

    return false;
}

bool USBDevice::IsOpen() const
{
	return io.IsOpen();
}


int USBDevice::GetPadEvent()
{
	if(!IsOpen())
		return -1;

	long buf;
	if( io.read(&buf) <= 0 )
		return -1;

    return buf;
}

WindowsFileIO::WindowsFileIO()
{
	ZeroMemory( &ov, sizeof(ov) );
	h = INVALID_HANDLE_VALUE;
	buf = NULL;
}

WindowsFileIO::~WindowsFileIO()
{
	if(h != INVALID_HANDLE_VALUE)
		CloseHandle(h);
	delete[] buf;
}

bool WindowsFileIO::Open(CString path, int blocksize_)
{
	LOG->Trace( "WindowsFileIO::open(%s)", path.c_str() );
	blocksize = blocksize_;

	if(buf)
		delete[] buf;
	buf = new char[blocksize];

	if(h != INVALID_HANDLE_VALUE)
		CloseHandle (h);

	h = CreateFile (path, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE,
		NULL, OPEN_EXISTING, FILE_FLAG_OVERLAPPED, NULL);

	if(h == INVALID_HANDLE_VALUE)
		return false;

	queue_read();

	return true;
}

void WindowsFileIO::queue_read()
{
	/* Request feedback from the device. */
	unsigned long r;
	ReadFile(h, buf, blocksize, &r, &ov);
}

int WindowsFileIO::finish_read(void *p)
{
	LOG->Trace("this %p, %p", this, p);
	/* We do; get the result.  It'll go into the original buf
	 * we supplied on the original call; that's why buf is a
	 * member instead of a local. */
    unsigned long cnt;
    int ret = GetOverlappedResult(h, &ov, &cnt, FALSE);

    if(ret == 0 && (GetLastError() == ERROR_IO_PENDING || GetLastError() == ERROR_IO_INCOMPLETE))
		return -1;

	queue_read();

	if(ret == 0)
	{
		LOG->Warn(werr_ssprintf(GetLastError(), "Error reading Pump pad"));
	    return -1;
    }

	memcpy( p, buf, cnt );
	return cnt;
}

int WindowsFileIO::read(void *p)
{
	LOG->Trace( "WindowsFileIO::read()" );

	/* See if we have a response for our request (which we may
	 * have made on a previous call): */
    if(WaitForSingleObjectEx(h, 0, TRUE) == WAIT_TIMEOUT)
		return -1;

	return finish_read(p);
}

int WindowsFileIO::read_several(const vector<WindowsFileIO *> &sources, void *p, int &actual, float timeout)
{
	HANDLE *Handles = new HANDLE[sources.size()];
	for( unsigned i = 0; i < sources.size(); ++i )
		Handles[i] = sources[i]->h;

	int ret = WaitForMultipleObjectsEx( sources.size(), Handles, false, int(timeout * 1000), true);
	delete[] Handles;

	if( ret == -1 )
	{
		LOG->Trace( werr_ssprintf(GetLastError(), "WaitForMultipleObjectsEx failed") );
		return -1;
	}

	if( ret >= int(WAIT_OBJECT_0) && ret < int(WAIT_OBJECT_0+sources.size()) )
	{
		actual = ret - WAIT_OBJECT_0;
		return sources[actual]->finish_read(p);
	}

	return 0;
}

bool WindowsFileIO::IsOpen() const
{
	return h != INVALID_HANDLE_VALUE;
}

/*
-----------------------------------------------------------------------------
 Copyright (c) 2002-2003 by the person(s) listed below.  All rights reserved.
	Glenn Maynard
-----------------------------------------------------------------------------
*/
