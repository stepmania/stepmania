#include "global.h"
#include "MemoryCardDriverThreaded_Windows.h"
#include "RageUtil.h"
#include <io.h>
#include <fcntl.h>
#include "RageFileManager.h"
#include "RageLog.h"
#include "Profile.h"
#include "PrefsManager.h"

const CString TEMP_MOUNT_POINT = "@mctemp/";


MemoryCardDriverThreaded_Windows::MemoryCardDriverThreaded_Windows()
{
	m_dwLastLogicalDrives = 0;

	StartThread();
}

MemoryCardDriverThreaded_Windows::~MemoryCardDriverThreaded_Windows()
{
	StopThread();
}

typedef const CString& CCStringRef;

static bool TestReady( CCStringRef sDrive )
{
	// TODO: Use RageFileDirect here to detect ready state?
	TCHAR szVolumeNameBuffer[MAX_PATH];
	DWORD dwVolumeSerialNumber;
	DWORD dwMaximumComponentLength;
	DWORD lpFileSystemFlags;
	TCHAR szFileSystemNameBuffer[MAX_PATH];

	return !!GetVolumeInformation( 
		sDrive,
		szVolumeNameBuffer,
		sizeof(szVolumeNameBuffer),
		&dwVolumeSerialNumber,
		&dwMaximumComponentLength,
		&lpFileSystemFlags,
		szFileSystemNameBuffer,
		sizeof(szFileSystemNameBuffer) );
}

static bool TestWrite( CCStringRef sDrive )
{
	// Try to write a file.
	// TODO: Can we use RageFile for this?
	CString sFile = sDrive + "temp";
	FILE* fp = fopen( sFile, "w" );
	if( fp == NULL )
		return false;
	fclose( fp );
	remove( sFile );

	return true;
}

void MemoryCardDriverThreaded_Windows::ResetUsbStorage()
{
	m_dwLastLogicalDrives = 0;
}

void MemoryCardDriverThreaded_Windows::MountThreadDoOneUpdate()
{
	DWORD dwNewLogicalDrives = ::GetLogicalDrives();

	if( dwNewLogicalDrives == m_dwLastLogicalDrives )
	{
		// no change from last update
		usleep( 50000 );
		return;
	}

	{
		vector<UsbStorageDevice> vNewStorageDevices;

		const int MAX_DRIVES = 26;
		for( int i=2; i<MAX_DRIVES; i++ )	// skip 'a:" and "b:"
		{
			DWORD mask = (1 << i);
			if( dwNewLogicalDrives & mask ) // drive letter is valid
			{
				CString sDrive = ssprintf( "%c:\\", 'a'+i%26 );

				LOG->Trace( "Found drive %s", sDrive.c_str() );

				if( GetDriveType(sDrive) != DRIVE_REMOVABLE )	// is a removable drive
					continue;

				if( !TestReady(sDrive) )
					continue;

				UsbStorageDevice usbd;
				usbd.SetOsMountDir( sDrive );
				usbd.bWriteTestSucceeded = TestWrite( sDrive );
				// read name
				this->Mount( &usbd, TEMP_MOUNT_POINT );
				FILEMAN->FlushDirCache( TEMP_MOUNT_POINT );
				Profile profile;
				CString sProfileDir = TEMP_MOUNT_POINT + PREFSMAN->m_sMemoryCardProfileSubdir + '/'; 
				profile.LoadEditableDataFromDir( sProfileDir );
				usbd.sName = profile.GetDisplayName();
				UnmountMountPoint( TEMP_MOUNT_POINT );

				vNewStorageDevices.push_back( usbd );
			}
		}

		CHECKPOINT;

		{
			LockMut( m_mutexStorageDevices );
			m_bStorageDevicesChanged = true;
			m_vStorageDevices = vNewStorageDevices;
		}

		CHECKPOINT;

		m_dwLastLogicalDrives = dwNewLogicalDrives;
	}
}

void MemoryCardDriverThreaded_Windows::Mount( UsbStorageDevice* pDevice, CString sMountPoint )
{
	ASSERT( !pDevice->sOsMountDir.empty() );

	UnmountMountPoint( sMountPoint );

	FILEMAN->Mount( "dir", pDevice->sOsMountDir, sMountPoint.c_str() );
	LOG->Trace( "FILEMAN->Mount %s %s", pDevice->sOsMountDir.c_str(), sMountPoint.c_str() );
}

void MemoryCardDriverThreaded_Windows::Unmount( UsbStorageDevice* pDevice, CString sMountPoint )
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
