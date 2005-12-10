#include "global.h"
#include "MemoryCardDriverThreaded_Windows.h"
#include "RageUtil.h"
#include "RageFileManager.h"
#include "RageLog.h"
#include "ProfileManager.h"
#include "PrefsManager.h"

const CString TEMP_MOUNT_POINT_INTERNAL = "/@mctemp/";
const CString TEMP_MOUNT_POINT = "/@mctemptimeout/";


MemoryCardDriverThreaded_Windows::MemoryCardDriverThreaded_Windows()
{
	m_dwLastLogicalDrives = 0;
}

MemoryCardDriverThreaded_Windows::~MemoryCardDriverThreaded_Windows()
{
}

static bool TestReady( const CString &sDrive, UsbStorageDevice *pDevice )
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
	pDevice->sVolumeLabel = szVolumeNameBuffer;
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

bool MemoryCardDriverThreaded_Windows::DoOneUpdate( bool bMount, vector<UsbStorageDevice>& vStorageDevicesOut )
{
	DWORD dwNewLogicalDrives = ::GetLogicalDrives();
	if( dwNewLogicalDrives == m_dwLastLogicalDrives )
	{
		// no change from last update
		return false;
	}

	m_dwLastLogicalDrives = dwNewLogicalDrives;

	{
		vector<UsbStorageDevice> vNewStorageDevices;

		const int MAX_DRIVES = 26;
		CString sDrives;
		for( int i=0; i<MAX_DRIVES; ++i )
		{
			DWORD mask = (1 << i);
			if( !(dwNewLogicalDrives & mask) )
				continue; // drive letter is invalid
			if( IsFloppyDrive(i+'a') )
				continue;

			CString sDrive = ssprintf( "%c:\\", 'a'+i%26 );
			if( GetDriveType(sDrive) != DRIVE_REMOVABLE )	// is a removable drive
				continue;

			if( !sDrives.empty() )
				sDrives += ", ";
			sDrives += sDrive;

			UsbStorageDevice usbd;
			if( !TestReady(sDrive, &usbd) )
				continue;
			usbd.SetOsMountDir( sDrive );
			if( TestWrite(sDrive) )
				usbd.m_State = UsbStorageDevice::STATE_READY;
			else
				usbd.SetError( "MountFailed" );

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

			// read name
			this->Mount( &usbd );
			FILEMAN->Mount( "dir", usbd.sOsMountDir, TEMP_MOUNT_POINT_INTERNAL );
			FILEMAN->Mount( "timeout", TEMP_MOUNT_POINT_INTERNAL, TEMP_MOUNT_POINT );

			usbd.bIsNameAvailable = PROFILEMAN->FastLoadProfileNameFromMemoryCard( TEMP_MOUNT_POINT, usbd.sName );

			FILEMAN->Unmount( "timeout", TEMP_MOUNT_POINT_INTERNAL, TEMP_MOUNT_POINT );
			FILEMAN->Unmount( "dir", usbd.sOsMountDir, TEMP_MOUNT_POINT_INTERNAL );

			vNewStorageDevices.push_back( usbd );
		}

		CHECKPOINT;

		vStorageDevicesOut = vNewStorageDevices;

		CHECKPOINT;
		LOG->Trace( "Found drives: %s", sDrives.c_str() );
	}
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
