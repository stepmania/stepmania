#include "global.h"
#include "MemoryCardDriverThreaded_Xbox.h"
#include "RageUtil.h"
#include "RageLog.h"

MemoryCardDriverThreaded_Xbox::MemoryCardDriverThreaded_Xbox()
{
}

MemoryCardDriverThreaded_Xbox::~MemoryCardDriverThreaded_Xbox()
{
}

static bool TestReady( const RString &sDrive, RString &sVolumeLabelOut )
{
	TCHAR szVolumeNameBuffer[MAX_PATH];
	DWORD dwVolumeSerialNumber;
	DWORD dwMaximumComponentLength;
	DWORD lpFileSystemFlags;
	TCHAR szFileSystemNameBuffer[MAX_PATH];

	if( !GetVolumeInformation( 
		sDrive,
		szVolumeNameBuffer,
		sizeof(szVolumeNameBuffer),
		&dwVolumeSerialNumber,
		&dwMaximumComponentLength,
		&lpFileSystemFlags,
		szFileSystemNameBuffer,
		sizeof(szFileSystemNameBuffer)) ){
			LOG->Trace("GetVolumeInformation failed %u", GetLastError());
			return false;
		}

	sVolumeLabelOut = szVolumeNameBuffer;
	return true;
}

bool MemoryCardDriverThreaded_Xbox::TestWrite( UsbStorageDevice* pDevice )
{
	/* Try to write a file, to check if the device is writable and that we have write permission.*/
	for( int i = 0; i < 10; ++i )
	{
		HANDLE hFile = CreateFile( 
			ssprintf( "%s\\tmp%i", pDevice->sOsMountDir.c_str(), RandomInt(100000)),
			GENERIC_WRITE, 
			FILE_SHARE_READ | FILE_SHARE_WRITE,
			NULL, 
			CREATE_NEW, 
			FILE_FLAG_DELETE_ON_CLOSE, 
			NULL );

		if( hFile == INVALID_HANDLE_VALUE )
		{
			DWORD iError = GetLastError();
			LOG->Warn( "Couldn't write to %s (%u)", pDevice->sOsMountDir.c_str(), iError);

			if( iError == ERROR_FILE_EXISTS )
				continue;
			break;
		}

		CloseHandle( hFile );
		return true;
	}

	pDevice->SetError( "TestFailed" );
	return false;
}

void MemoryCardDriverThreaded_Xbox::GetUSBStorageDevices( vector<UsbStorageDevice>& vDevicesOut )
{
	DWORD devices=XGetDevices(XDEVICE_TYPE_MEMORY_UNIT);

	for(int port=0;port<4;port++){
		//top slot
		if(devices&(1<<port)){
			vDevicesOut.push_back( UsbStorageDevice() );
			UsbStorageDevice &usbd = vDevicesOut.back();
			usbd.iPort=port;
			usbd.iLevel=0;
			usbd.sDevice=ssprintf("Memory card on port %u on slot 0", port);
		}
		//bottom slot
		if(devices&(1<<(port+16))){
			vDevicesOut.push_back( UsbStorageDevice() );
			UsbStorageDevice &usbd = vDevicesOut.back();
			usbd.iPort=port;
			usbd.iLevel=1;
			usbd.sDevice=ssprintf("Memory card on port %u on slot 1", port);
		}
	}
}

bool MemoryCardDriverThreaded_Xbox::USBStorageDevicesChanged()
{
	DWORD ins, rem;
	return XGetDeviceChanges(XDEVICE_TYPE_MEMORY_UNIT, &ins, &rem)==TRUE;
}

bool MemoryCardDriverThreaded_Xbox::Mount( UsbStorageDevice* pDevice )
{
	LOG->Trace( "%s", __FUNCTION__);
	CHAR drive;
	DWORD MountRetval=XMountMU(pDevice->iPort, pDevice->iLevel, &drive);
	if(MountRetval==ERROR_SUCCESS){
		LOG->Trace("Mounted memory card from port %u slot %u to %c:", pDevice->iPort, pDevice->iLevel, drive);
		pDevice->SetOsMountDir(ssprintf("%c:", drive));
		RString sVolumeLabel;
		if( !TestReady(pDevice->sOsMountDir + "\\", sVolumeLabel) )
		{
			LOG->Trace( "not TestReady" );
		}
		pDevice->sVolumeLabel = sVolumeLabel;
		return true;
	}else{
		LOG->Trace("Could not mount memory card %u", MountRetval);
		return false;
	}
}

void MemoryCardDriverThreaded_Xbox::Unmount( UsbStorageDevice* pDevice )
{
	XUnmountMU(pDevice->iPort, pDevice->iLevel);
}

/*
 * (c) 2003-2004 Chris Danford
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
