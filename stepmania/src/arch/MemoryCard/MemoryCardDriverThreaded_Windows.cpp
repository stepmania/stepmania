#include "global.h"
#include "MemoryCardDriverThreaded_Windows.h"
#include "RageUtil.h"
#include <io.h>
#include <fcntl.h>
#include "RageFileManager.h"
#include "RageLog.h"
#include "Profile.h"
#include "PrefsManager.h"
#include <windows.h>

const CString TEMP_MOUNT_POINT = "@mctemp/";


MemoryCardDriverThreaded_Windows::MemoryCardDriverThreaded_Windows()
{
	StartThread();
}

typedef const CString& CCStringRef;

bool TestWrite( CCStringRef sDrive )
{
	// TODO: Use RageFileDirect here to detect ready state?
	TCHAR szVolumeNameBuffer[MAX_PATH];
	DWORD dwVolumeSerialNumber;
	DWORD dwMaximumComponentLength;
	DWORD lpFileSystemFlags;
	TCHAR szFileSystemNameBuffer[MAX_PATH];
	BOOL bReady = GetVolumeInformation( 
		sDrive,
		szVolumeNameBuffer,
		sizeof(szVolumeNameBuffer),
		&dwVolumeSerialNumber,
		&dwMaximumComponentLength,
		&lpFileSystemFlags,
		szFileSystemNameBuffer,
		sizeof(szFileSystemNameBuffer) );
	if( !bReady )
		return false;

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

void MemoryCardDriverThreaded_Windows::MountThreadMain()
{
	DWORD dwLastLogicalDrives = 0;
	
	while( !m_bShutdown )
	{
		DWORD dwNewLogicalDrives = ::GetLogicalDrives();
		if( dwNewLogicalDrives != dwLastLogicalDrives )
		{
			vector<UsbStorageDeviceEx> vNewStorageDevices;

			const int MAX_DRIVES = 26;
			for( int i=2; i<MAX_DRIVES; i++ )	// skip 'a:" and "b:"
			{
				DWORD mask = (1 << i);
				if( dwNewLogicalDrives & mask ) // drive letter is valid
				{
					CString sDrive = ssprintf( "%c:\\", 'a'+i%26 );

					LOG->Trace( "Found drive %s", sDrive.c_str() );

					if( GetDriveType(sDrive) == DRIVE_REMOVABLE )	// is a removable drive
					{
						UsbStorageDeviceEx usbd;
						usbd.sOsMountDir = sDrive;
						usbd.bWriteTestSucceeded = TestWrite( sDrive );

						// read name
						this->Mount( &usbd, TEMP_MOUNT_POINT );
						FILEMAN->FlushDirCache( TEMP_MOUNT_POINT );
						Profile profile;
						CString sProfileDir = TEMP_MOUNT_POINT + PREFSMAN->m_sMemoryCardProfileSubdir + '/'; 
						profile.LoadEditableDataFromDir( sProfileDir );
						usbd.sName = profile.GetDisplayName();

						vNewStorageDevices.push_back( usbd );
					}
				}
			}

			{
				LockMut( m_mutexStorageDevices );
				m_bStorageDevicesChanged = true;
				m_vStorageDevices = vNewStorageDevices;
			}
	    }
		dwLastLogicalDrives = dwNewLogicalDrives;

		::Sleep( 100 );
	}
}

void MemoryCardDriverThreaded_Windows::Mount( UsbStorageDevice* pDevice, CString sMountPoint )
{
	ASSERT( !pDevice->sOsMountDir.empty() );

	/* Unmount any previous mounts for this mountpoint. */
	vector<RageFileManager::DriverLocation> Mounts;
	FILEMAN->GetLoadedDrivers( Mounts );
	for( unsigned i = 0; i < Mounts.size(); ++i )
	{
		if( Mounts[i].Type.CompareNoCase( "dir" ) )
				continue; // wrong type
		if( Mounts[i].MountPoint.CompareNoCase( sMountPoint ) )
				continue; // wrong mount point
		FILEMAN->Unmount( Mounts[i].Type, Mounts[i].Root, Mounts[i].MountPoint );
	}

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
 * Copyright (c) 2003 by the person(s) listed below.  All rights reserved.
 *	Chris Danford
 */
