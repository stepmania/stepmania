#include "global.h"
#include "USB.h"
#include "RageLog.h"
#include "RageUtil.h"

#if defined(_MSC_VER)
#pragma comment(lib, "archutils/Win32/ddk/setupapi.lib") 
#pragma comment(lib, "archutils/Win32/ddk/hid.lib") 
#endif

extern "C" {
#include "archutils/Win32/ddk/setupapi.h"
/* Quiet header warning: */
#include "archutils/Win32/ddk/hidsdi.h"
}

static CString GetUSBDevicePath( int iNum )
{
    GUID guid;
    HidD_GetHidGuid( &guid );

    HDEVINFO DeviceInfo = SetupDiGetClassDevs( &guid, NULL, NULL, (DIGCF_PRESENT | DIGCF_DEVICEINTERFACE) );

    SP_DEVICE_INTERFACE_DATA DeviceInterface;
    DeviceInterface.cbSize = sizeof(SP_DEVICE_INTERFACE_DATA);

    if( !SetupDiEnumDeviceInterfaces (DeviceInfo,
               NULL, &guid, iNum, &DeviceInterface) )
	{
	    SetupDiDestroyDeviceInfoList( DeviceInfo );
	    return CString();
	}

    unsigned long iSize;
    SetupDiGetDeviceInterfaceDetail( DeviceInfo, &DeviceInterface, NULL, 0, &iSize, 0 );

    PSP_INTERFACE_DEVICE_DETAIL_DATA DeviceDetail = (PSP_INTERFACE_DEVICE_DETAIL_DATA) malloc( iSize );
    DeviceDetail->cbSize = sizeof(SP_INTERFACE_DEVICE_DETAIL_DATA);

    CString sRet;
    if( SetupDiGetDeviceInterfaceDetail(DeviceInfo, &DeviceInterface,
		DeviceDetail, iSize, &iSize, NULL) ) 
        sRet = DeviceDetail->DevicePath;
	free( DeviceDetail );

	SetupDiDestroyDeviceInfoList( DeviceInfo );
    return sRet;
}


bool USBDevice::Open( int iVID, int iPID, int iBlockSize, int iNum )
{
    DWORD iIndex = 0;

    CString path;
    while( (path = GetUSBDevicePath(iIndex++)) != "" )
    {
		HANDLE h = CreateFile( path, GENERIC_READ,
			FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL );

		if( h == INVALID_HANDLE_VALUE )
			continue;

		HIDD_ATTRIBUTES attr;
		if( !HidD_GetAttributes(h, &attr) )
		{
			CloseHandle( h );
			continue;
		}
		CloseHandle( h );

        if( (iVID != -1 && attr.VendorID != iVID) ||
            (iPID != -1 && attr.ProductID != iPID) )
			continue; /* This isn't it. */

		/* The VID and PID match. */
		if( iNum-- > 0 )
			continue;

		m_IO.Open( path, iBlockSize );
        return true;
    }

    return false;
}

bool USBDevice::IsOpen() const
{
	return m_IO.IsOpen();
}


int USBDevice::GetPadEvent()
{
	if( !IsOpen() )
		return -1;

	long iBuf;
	if( m_IO.read(&iBuf) <= 0 )
		return -1;

    return iBuf;
}

WindowsFileIO::WindowsFileIO()
{
	ZeroMemory( &m_Overlapped, sizeof(m_Overlapped) );
	m_Handle = INVALID_HANDLE_VALUE;
	m_pBuffer = NULL;
}

WindowsFileIO::~WindowsFileIO()
{
	if( m_Handle != INVALID_HANDLE_VALUE )
		CloseHandle( m_Handle );
	delete[] m_pBuffer;
}

bool WindowsFileIO::Open( CString path, int iBlockSize )
{
	LOG->Trace( "WindowsFileIO::open(%s)", path.c_str() );
	m_iBlockSize = iBlockSize;

	if( m_pBuffer )
		delete[] m_pBuffer;
	m_pBuffer = new char[m_iBlockSize];

	if( m_Handle != INVALID_HANDLE_VALUE )
		CloseHandle( m_Handle );

	m_Handle = CreateFile( path, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE,
		NULL, OPEN_EXISTING, FILE_FLAG_OVERLAPPED, NULL );

	if( m_Handle == INVALID_HANDLE_VALUE )
		return false;

	queue_read();

	return true;
}

void WindowsFileIO::queue_read()
{
	/* Request feedback from the device. */
	unsigned long iRead;
	ReadFile( m_Handle, m_pBuffer, m_iBlockSize, &iRead, &m_Overlapped );
}

int WindowsFileIO::finish_read( void *p )
{
	LOG->Trace( "this %p, %p", this, p );
	/* We do; get the result.  It'll go into the original m_pBuffer
	 * we supplied on the original call; that's why m_pBuffer is a
	 * member instead of a local. */
    unsigned long iCnt;
    int iRet = GetOverlappedResult( m_Handle, &m_Overlapped, &iCnt, FALSE );

    if( iRet == 0 && (GetLastError() == ERROR_IO_PENDING || GetLastError() == ERROR_IO_INCOMPLETE) )
		return -1;

	queue_read();

	if( iRet == 0 )
	{
		LOG->Warn( werr_ssprintf(GetLastError(), "Error reading USB device") );
	    return -1;
    }

	memcpy( p, m_pBuffer, iCnt );
	return cnt;
}

int WindowsFileIO::read( void *p )
{
	LOG->Trace( "WindowsFileIO::read()" );

	/* See if we have a response for our request (which we may
	 * have made on a previous call): */
    if( WaitForSingleObjectEx(m_Handle, 0, TRUE) == WAIT_TIMEOUT )
		return -1;

	return finish_read(p);
}

int WindowsFileIO::read_several(const vector<WindowsFileIO *> &sources, void *p, int &actual, float timeout)
{
	HANDLE *Handles = new HANDLE[sources.size()];
	for( unsigned i = 0; i < sources.size(); ++i )
		Handles[i] = sources[i]->m_Handle;

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
	return m_Handle != INVALID_HANDLE_VALUE;
}

/*
 * (c) 2002-2005 Glenn Maynard
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
