#include "global.h"
#include "MemoryCardDriverThreaded_Windows.h"
#include "RageUtil.h"
#include "RageFileManager.h"
#include "RageLog.h"
#include "ProfileManager.h"
#include "PrefsManager.h"
#include "Foreach.h"

const CString TEMP_MOUNT_POINT_INTERNAL = "/@mctemp/";
const CString TEMP_MOUNT_POINT = "/@mctemptimeout/";


MemoryCardDriverThreaded_Windows::MemoryCardDriverThreaded_Windows()
{
	m_dwLastLogicalDrives = 0;
}

MemoryCardDriverThreaded_Windows::~MemoryCardDriverThreaded_Windows()
{
}

static bool TestReady( const CString &sDrive, CString &sVolumeLabelOut )
{
	TCHAR szVolumeNameBuffer[MAX_PATH];
	DWORD dwVolumeSerialNumber;
	DWORD dwMaximumComponentLength;
	DWORD lpFileSystemFlags;
	TCHAR szFileSystemNameBuffer[MAX_PATH];

	bool bRet = !!GetVolumeInformation( 
		sDrive,
		szVolumeNameBuffer,
		sizeof(szVolumeNameBuffer),
		&dwVolumeSerialNumber,
		&dwMaximumComponentLength,
		&lpFileSystemFlags,
		szFileSystemNameBuffer,
		sizeof(szFileSystemNameBuffer) );
	sVolumeLabelOut = szVolumeNameBuffer;
	return bRet;
}

static bool TestWrite( const CString &sDrive )
{
	// Try to write a file.
	CString sFile = sDrive + "temp";
	FILE* fp = fopen( sFile, "w" );
	if( fp == NULL )
		return false;
	fclose( fp );
	remove( sFile );

	return true;
}

void MemoryCardDriverThreaded_Windows::Reset()
{
	m_dwLastLogicalDrives = 0;
}

static bool IsFloppyDrive( char c )
{
	char szBuf[1024];

	QueryDosDevice( ssprintf("%c:", c), szBuf, 1024 );

	char *p = szBuf;
	while( *p )
	{
		if( BeginsWith(p, "\\Device\\Floppy") )
			return true;

		p += strlen(p)+1;
	}
	return false;
}

void MemoryCardDriverThreaded_Windows::GetUSBStorageDevices( vector<UsbStorageDevice>& vDevicesOut )
{
	const int MAX_DRIVES = 26;
	for( int i=0; i<MAX_DRIVES; ++i )
	{
		DWORD mask = (1 << i);
		if( !(m_dwLastLogicalDrives & mask) )
			continue; // drive letter is invalid
		if( IsFloppyDrive(i+'a') )
			continue;

		CString sDrive = ssprintf( "%c:\\", 'a'+i%26 );
		if( GetDriveType(sDrive) != DRIVE_REMOVABLE )	// is a removable drive
			continue;

		CString sVolumeLabel;
		if( !TestReady(sDrive, sVolumeLabel) )
			continue;

		vDevicesOut.push_back( UsbStorageDevice() );
		UsbStorageDevice &usbd = vDevicesOut.back();
		usbd.SetOsMountDir( sDrive );
		usbd.sDevice = sDrive;
		usbd.sVolumeLabel = sVolumeLabel;

		// find volume size
		DWORD dwSectorsPerCluster;
		DWORD dwBytesPerSector;
		DWORD dwNumberOfFreeClusters;
		DWORD dwTotalNumberOfClusters;
		if( GetDiskFreeSpace(
				sDrive,
				&dwSectorsPerCluster,
				&dwBytesPerSector,
				&dwNumberOfFreeClusters,
				&dwTotalNumberOfClusters ) )
		{
			usbd.iVolumeSizeMB = (int)roundf( dwTotalNumberOfClusters * (float)dwSectorsPerCluster * dwBytesPerSector / (1024*1024) );
		}
	}
}

bool MemoryCardDriverThreaded_Windows::NeedUpdate( bool bMount )
{
	if( bMount )
	{
		/* Check if any devices need a write test. */
		for( unsigned i=0; i<m_vDevicesLastSeen.size(); i++ )
		{
			const UsbStorageDevice &d = m_vDevicesLastSeen[i];
			if( d.m_State == UsbStorageDevice::STATE_CHECKING )
				return true;
		}
	}

	/* Nothing needs a write test (or we can't do it right now).  If no devices
	 * have changed, either, we have nothing to do. */
	DWORD dwNewLogicalDrives = ::GetLogicalDrives();
	if( dwNewLogicalDrives != m_dwLastLogicalDrives )
	{
		m_dwLastLogicalDrives = dwNewLogicalDrives;
		return true;
	}

	/* Nothing to do. */
	return false;
}

bool MemoryCardDriverThreaded_Windows::DoOneUpdate( bool bMount, vector<UsbStorageDevice>& vStorageDevicesOut )
{
	if( !NeedUpdate(bMount) )
		return false;

	vector<UsbStorageDevice> vOld = m_vDevicesLastSeen; // copy
	GetUSBStorageDevices( vStorageDevicesOut );

	// log connects
	FOREACH( UsbStorageDevice, vStorageDevicesOut, newd )
	{
		vector<UsbStorageDevice>::iterator iter = find( vOld.begin(), vOld.end(), *newd );
		if( iter == vOld.end() )    // didn't find
			LOG->Trace( "New device connected: %s", newd->sDevice.c_str() );
	}

	/* When we first see a device, regardless of bMount, just return it as CHECKING,
	 * so the main thread knows about the device.  On the next call where bMount is
	 * true, check it. */
	for( unsigned i=0; i<vStorageDevicesOut.size(); i++ )
	{
		UsbStorageDevice &d = vStorageDevicesOut[i];

		/* If this device was just connected (it wasn't here last time), set it to
		 * CHECKING and return it, to let the main thread know about the device before
		 * we start checking. */
		vector<UsbStorageDevice>::iterator iter = find( vOld.begin(), vOld.end(), d );
		if( iter == vOld.end() )    // didn't find
		{
			LOG->Trace( "New device entering CHECKING: %s", d.sDevice.c_str() );
			d.m_State = UsbStorageDevice::STATE_CHECKING;
			continue;
		}

		/* Preserve the state of the device, and any data loaded from previous checks. */
		d.m_State = iter->m_State;
		d.bIsNameAvailable = iter->bIsNameAvailable;
		d.sName = iter->sName;

		/* The device was here last time.  If CHECKING, check the device now, if
		 * we're allowed to. */
		if( d.m_State == UsbStorageDevice::STATE_CHECKING )
		{
			if( !bMount )
			{
				/* We can't check it now.  Keep STATE_CHECKING, and check it when we can. */
				d.m_State = UsbStorageDevice::STATE_CHECKING;
				continue;
			}

			if( !this->Mount(&d) )
			{
				d.SetError( "MountFailed" );
				continue;
			}

			if( !TestWrite(d.sOsMountDir) )
			{
				d.SetError( "TestFailed" );
			}
			else
			{
				/* We've successfully mounted and tested the device.  Read the
				 * profile name (by mounting a temporary, private mountpoint),
				 * and then unmount it until Mount() is called. */
				d.m_State = UsbStorageDevice::STATE_READY;
			
				FILEMAN->Mount( "dir", d.sOsMountDir, TEMP_MOUNT_POINT );
				d.bIsNameAvailable = PROFILEMAN->FastLoadProfileNameFromMemoryCard( TEMP_MOUNT_POINT, d.sName );
				FILEMAN->Unmount( "dir", d.sOsMountDir, TEMP_MOUNT_POINT );
			}

			this->Unmount( &d );

			LOG->Trace( "WriteTest: %s, Name: %s", d.m_State == UsbStorageDevice::STATE_ERROR? "failed":"succeeded", d.sName.c_str() );
		}
	}

	m_vDevicesLastSeen = vStorageDevicesOut;

	return true;
}

bool MemoryCardDriverThreaded_Windows::Mount( UsbStorageDevice* pDevice )
{
	// nothing to do here...
	return true;
}

void MemoryCardDriverThreaded_Windows::Unmount( UsbStorageDevice* pDevice )
{
	// nothing to do here...
}

void MemoryCardDriverThreaded_Windows::Flush( UsbStorageDevice* pDevice )
{
	// Do we need anything here?  I don't lose data if ejecting 
	// soon after a write.  From the activity LED, it looks like 
	// Windows flushes automatically every ~2 seconds. -Chris
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
