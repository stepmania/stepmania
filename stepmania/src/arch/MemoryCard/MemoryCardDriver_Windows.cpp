#include "global.h"
#include "MemoryCardDriver_Windows.h"
#include "RageUtil.h"
#include <io.h>
#include <fcntl.h>
#include "RageFileManager.h"
#include "RageLog.h"

MemoryCardDriver_Windows::MemoryCardDriver_Windows()
{
	m_dwLastLogicalDrives = GetLogicalDrives();
}

MemoryCardDriver_Windows::~MemoryCardDriver_Windows()
{
}

bool MemoryCardDriver_Windows::StorageDevicesChanged()
{
	DWORD dwNewLogicalDrives = GetLogicalDrives();
	bool bChanged = (dwNewLogicalDrives != m_dwLastLogicalDrives);
	m_dwLastLogicalDrives = dwNewLogicalDrives;
	return bChanged;
}

void MemoryCardDriver_Windows::GetStorageDevices( vector<UsbStorageDevice>& vDevicesOut )
{
	vDevicesOut.clear();

	int i = 2;	// skip 'a:" and "b:"
	const int MAX_DRIVES = sizeof(m_dwLastLogicalDrives)*8;
	for( ; i<MAX_DRIVES; i++ )
	{
		DWORD mask = (1 << i);
		if( m_dwLastLogicalDrives & mask )	// drive is valid
		{
			CString sDrive;
			if( i >= 26 )
				sDrive = ssprintf( "%c%c:\\", 'a'+(i/26)-1, i%26 );
			else
				sDrive = ssprintf( "%c:\\", 'a'+i%26 );

			if( GetDriveType(sDrive) != DRIVE_REMOVABLE )
				continue;	// skip

			UsbStorageDevice usbd;
			usbd.sOsMountDir = sDrive;
			
			vDevicesOut.push_back( usbd );
		}
	}
}

bool MemoryCardDriver_Windows::MountAndTestWrite( UsbStorageDevice* pDevice, CString sMountPoint )
{
	if( pDevice->sOsMountDir.empty() )
		return false;

	// TODO: Use RageFileDirect here to detect ready state?
	TCHAR szVolumeNameBuffer[MAX_PATH];
	DWORD dwVolumeSerialNumber;
	DWORD dwMaximumComponentLength;
	DWORD lpFileSystemFlags;
	TCHAR szFileSystemNameBuffer[MAX_PATH];
	BOOL bReady = GetVolumeInformation( 
		pDevice->sOsMountDir,
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
	CString sFile = pDevice->sOsMountDir + "temp";
	FILE* fp = fopen( sFile, "w" );
	if( fp == NULL )
		return false;
	fclose( fp );
	remove( sFile );

	/* Unmount any previous mounts for this mountpoint. */
	vector<RageFileManager::DriverLocation> Mounts;
	FILEMAN->GetLoadedDrivers( Mounts );
	for( unsigned i = 0; i < Mounts.size(); ++i )
		FILEMAN->Unmount( Mounts[i].Type, Mounts[i].Root, Mounts[i].MountPoint );

	FILEMAN->Mount( "dir", pDevice->sOsMountDir, sMountPoint.c_str() );
	LOG->Trace( "FILEMAN->Mount %s %s", pDevice->sOsMountDir.c_str(), sMountPoint.c_str() );

	return true;
}

void MemoryCardDriver_Windows::Flush( UsbStorageDevice* pDevice )
{
	// Do we need anything here?  I don't lose data if ejecting 
	// soon after a write.  From the activity LED, it looks like 
	// Windows flushes automatically every ~2 seconds. -Chris
}

/*
 * Copyright (c) 2003 by the person(s) listed below.  All rights reserved.
 *	Chris Danford
 */
