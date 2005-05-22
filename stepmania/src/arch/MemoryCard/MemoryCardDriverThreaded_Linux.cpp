#include "global.h"
#include "MemoryCardDriverThreaded_Linux.h"
#include "RageLog.h"
#include "RageUtil.h"
#include "RageFileManager.h"
#include "ProfileManager.h"
#include "PrefsManager.h"
#include "Foreach.h"

#include <cstdio>
#include <cstring>
#include <cerrno>
#include <unistd.h>
#include <fcntl.h>
#include <fstream>
#include <dirent.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/poll.h>

const CString TEMP_MOUNT_POINT = "/@mctemp/";

void GetNewStorageDevices( vector<UsbStorageDevice>& vDevicesOut );

template<class T>
bool VectorsAreEqual( const T &a, const T &b )
{
	if( a.size() != b.size() )
		return false;
	
	for( unsigned i=0; i<a.size(); i++ )
    {
		if( a[i] != b[i] )
			return false;
    }
	
	return true;
}

static bool TestWrite( CCStringRef sDir )
{
	// Try to write a file.
	// TODO: Can we use RageFile for this?
	CString sFile = sDir + "/temp";
	FILE* fp = fopen( sFile, "w" );
	if( fp == NULL )
		return false;
	fclose( fp );
	remove( sFile );
	return true;
}

static bool ExecuteCommand( CCStringRef sCommand )
{
	LOG->Trace( "executing '%s'", sCommand.c_str() );
	int ret = system(sCommand);
	LOG->Trace( "done executing '%s'", sCommand.c_str() );
	if( ret != 0 )
		LOG->Warn( "failed to execute '%s' with error %d.", sCommand.c_str(), ret );
	return ret == 0;
}

MemoryCardDriverThreaded_Linux::MemoryCardDriverThreaded_Linux()
{
}

MemoryCardDriverThreaded_Linux::~MemoryCardDriverThreaded_Linux()
{
}

void MemoryCardDriverThreaded_Linux::Reset()
{
}

static bool ReadFile( const CString &sPath, CString &sBuf )
{
	sBuf.clear();

	int fd = open( sPath, O_RDONLY );
	if( fd == -1 )
	{
		LOG->Warn( "Error opening \"%s\": %s", sPath.c_str(), strerror(errno) );
		return false;
	}
	
	while(1)
	{
		char buf[1024];
		int iGot = read( fd, buf, sizeof(buf) );
		if( iGot == -1 )
		{
			close(fd);
			LOG->Warn( "Error reading \"%s\": %s", sPath.c_str(), strerror(errno) );
			return false;
		}

		sBuf.append( buf, iGot );
		if( iGot < (int) sizeof(buf) )
			break;
	}
	
	close(fd);
	return true;
}

static void GetFileList( const CString &sPath, vector<CString> &out )
{
	out.clear();

	DIR *dp = opendir( sPath );
	if( dp == NULL )
		return; // false; // XXX warn

	while( const struct dirent *ent = readdir(dp) )
		out.push_back( ent->d_name );

	closedir( dp );
}

static bool BlockDevicesChanged()
{
	static CString sLastDevices = "";
	CString sThisDevices;

	/* If a device is removed and reinserted, the inode of the /sys/block entry
	 * will change. */
	CString sDevicePath = "/sys/block/";
	
	vector<CString> asDevices;
	GetFileList( sDevicePath, asDevices );

	for( unsigned i = 0; i < asDevices.size(); ++i )
	{
		struct stat buf;
		if( stat( sDevicePath + asDevices[i], &buf ) == -1 )
			continue; // XXX warn

		sThisDevices += ssprintf( "%i,", (int) buf.st_ino );
	}
	       
	bool bChanged = sThisDevices != sLastDevices;
	sLastDevices = sThisDevices;
	if( bChanged )
		LOG->Trace( "Change in USB storage devices detected." );
	return bChanged;
}

bool MemoryCardDriverThreaded_Linux::NeedUpdate( bool bMount ) const
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

	/* Nothing needs a write test (or we ca'nt do it right now).  If no devices
	 * have changed, either, we have nothing to do. */
	if( BlockDevicesChanged() )
		return true;

	/* Nothing to do. */
	return false;
}

bool MemoryCardDriverThreaded_Linux::DoOneUpdate( bool bMount, vector<UsbStorageDevice>& vStorageDevicesOut )
{
	if( !NeedUpdate(bMount) )
		return false;

	vector<UsbStorageDevice> vOld = m_vDevicesLastSeen; // copy
	GetNewStorageDevices( vStorageDevicesOut );
	vector<UsbStorageDevice> &vNew = vStorageDevicesOut;
	
	// check for connects
	vector<UsbStorageDevice> vConnects;
	FOREACH( UsbStorageDevice, vNew, newd )
	{
		vector<UsbStorageDevice>::iterator iter = find( vOld.begin(), vOld.end(), *newd );
		if( iter == vOld.end() )    // didn't find
		{
			LOG->Trace( "New device connected: %s", newd->sDevice.c_str() );
			vConnects.push_back( *newd );
		}
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
			LOG->Trace("New device entering CHECKING: %s", d.sDevice.c_str() );
			d.m_State = UsbStorageDevice::STATE_CHECKING;
			continue;
		}

		/* Preserve the state of the device, and any data loaded from previous checks. */
		d.m_State = iter->m_State;
		d.bIsNameAvailable = iter->bIsNameAvailable;
		d.sName = iter->sName;

		/* The device was here last time.  If CHECKING, check the device now if
		 * we're allowed to. */
		if( d.m_State == UsbStorageDevice::STATE_CHECKING )
		{
			if( !bMount )
			{
				/* We can't check it now.  Keep the checking state and check it when
				 * we can. */
				d.m_State = UsbStorageDevice::STATE_CHECKING;
				continue;
			}

			if( !ExecuteCommand("mount " + d.sDevice) )
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

			ExecuteCommand( "umount -l \"" + d.sOsMountDir + "\"" );

			LOG->Trace( "WriteTest: %s, Name: %s", d.m_State == UsbStorageDevice::STATE_ERROR? "failed":"succeeded", d.sName.c_str() );
		}
	}
	
	m_vDevicesLastSeen = vNew;
	
	CHECKPOINT;
	return true;
}

void GetNewStorageDevices( vector<UsbStorageDevice>& vDevicesOut )
{
	LOG->Trace( "GetNewStorageDevices" );
	
	vDevicesOut.clear();

	{
		vector<CString> asDevices;
		CString sBlockDevicePath = "/sys/block/";
		GetFileList( sBlockDevicePath, asDevices );

		for( unsigned i = 0; i < asDevices.size(); ++i )
		{
			const CString &sDevice = asDevices[i];
			if( sDevice == "." || sDevice == ".." )
				continue;

			UsbStorageDevice usbd;

			CString sPath = sBlockDevicePath + sDevice + "/";

			/* Ignore non-removable devices. */
			CString sBuf;
			if( !ReadFile( sPath + "removable", sBuf ) )
				continue; // already warned
			if( atoi(sBuf) != 1 )
				continue;


			usbd.sDevice = "/dev/" + sDevice + "1";

			/*
			 * sPath/device should be a symlink to the actual device.  For USB
			 * devices, it looks like this:
			 *
			 * device -> ../../devices/pci0000:00/0000:00:02.1/usb2/2-1/2-1:1.0
			 *
			 * "2-1" is "bus-port".
			 */
			char szLink[256];
			int iRet = readlink( sPath + "device", szLink, sizeof(szLink) );
			if( iRet == -1 )
			{
				LOG->Warn( "readlink(\"%s\"): %s", (sPath + "device").c_str(), strerror(errno) );
			}
			else
			{
				/*
				 * The full path looks like
				 *
				 *   ../../devices/pci0000:00/0000:00:02.1/usb2/2-2/2-2.1/2-2.1:1.0
				 *
				 * Each path element refers to a new hop in the chain.
				 *  "usb2" = second USB host
				 *  2-            second USB host,
				 *   -2           port 1 on the host,
				 *     .1         port 1 on an attached hub
				 *       .2       ... port 2 on the next hub ...
				 * 
				 * We want the bus number and the port of the last hop.  The level is
				 * the number of hops.
				 */
				szLink[iRet] = 0;
				vector<CString> asBits;
				split( szLink, "/", asBits );

				if( strstr( szLink, "usb" ) != NULL )
				{
					CString sHostPort = asBits[asBits.size()-2];
					sHostPort.Replace( "-", "." );
					asBits.clear();
					split( sHostPort, ".", asBits );
					if( asBits.size() > 1 )
					{
						usbd.iBus = atoi( asBits[0] );
						usbd.iPort = atoi( asBits[asBits.size()-1] );
						usbd.iLevel = asBits.size() - 1;
					}
				}
			}

			if( ReadFile( sPath + "device/../idVendor", sBuf ) )
				sscanf( sBuf, "%x", &usbd.idVendor );

			if( ReadFile( sPath + "device/../idProduct", sBuf ) )
				sscanf( sBuf, "%x", &usbd.idProduct );

			if( ReadFile( sPath + "device/../serial", sBuf ) )
			{
				usbd.sSerial = sBuf;
				TrimRight( usbd.sSerial );
			}
			if( ReadFile( sPath + "device/../product", sBuf ) )
			{
				usbd.sProduct = sBuf;
				TrimRight( usbd.sProduct );
			}
			if( ReadFile( sPath + "device/../manufacturer", sBuf ) )
			{
				usbd.sVendor = sBuf;
				TrimRight( usbd.sVendor );
			}

			vDevicesOut.push_back( usbd );
		}
	}

	{
		// Find where each device is mounted. Output looks like:
		
		// /dev/sda1               /mnt/flash1             auto    noauto,owner 0 0
		// /dev/sdb1               /mnt/flash2             auto    noauto,owner 0 0
		// /dev/sdc1               /mnt/flash3             auto    noauto,owner 0 0
		
		CString fn = "/rootfs/etc/fstab";
		RageFile f;
		if( !f.Open(fn) )
		{
			LOG->Warn( "can't open '%s': %s", fn.c_str(), f.GetError().c_str() );
			return;
		}
		
		CString sLine;
		while( !f.AtEOF() )
		{
			switch( f.GetLine(sLine) )
			{
			case 0: continue; /* eof */
			case -1:
				LOG->Warn( "error reading '%s': %s", fn.c_str(), f.GetError().c_str() );
				return;
			}
			
			char szScsiDevice[1024];
			char szMountPoint[1024];
			int iRet = sscanf( sLine, "%s %s", szScsiDevice, szMountPoint );
			if( iRet != 2 )
				continue;	// don't process this line
			
			
			CString sMountPoint = szMountPoint;
			TrimLeft( sMountPoint );
			TrimRight( sMountPoint );
			
			// search for the mountpoint corresponding to the device
			for( unsigned i=0; i<vDevicesOut.size(); i++ )
			{
				UsbStorageDevice& usbd = vDevicesOut[i];
				if( usbd.sDevice == szScsiDevice )	// found our match
				{
					usbd.sOsMountDir = sMountPoint;
					break;	// stop looking for a match
				}
			}
		}
	}

	for( unsigned i=0; i<vDevicesOut.size(); i++ )
	{
		UsbStorageDevice& usbd = vDevicesOut[i];
		LOG->Trace( "    sDevice: %s, iBus: %d, iLevel: %d, iPort: %d, id: %04X:%04X, Vendor: '%s', Product: '%s', sSerial: \"%s\", sOsMountDir: %s",
				usbd.sDevice.c_str(), usbd.iBus, usbd.iLevel, usbd.iPort, usbd.idVendor, usbd.idProduct, usbd.sVendor.c_str(),
				usbd.sProduct.c_str(), usbd.sSerial.c_str(), usbd.sOsMountDir.c_str() );
	}
	
	/* Remove any devices that we couldn't find a mountpoint for. */
	for( unsigned i=0; i<vDevicesOut.size(); i++ )
	{
		UsbStorageDevice& usbd = vDevicesOut[i];
		if( usbd.sOsMountDir.empty() )
		{
			LOG->Trace( "Ignoring %s (couldn't find in /etc/fstab)", usbd.sDevice.c_str() );
			
			vDevicesOut.erase( vDevicesOut.begin()+i );
			--i;
		}
	}
	
	LOG->Trace( "Done with GetNewStorageDevices" );
}


bool MemoryCardDriverThreaded_Linux::Mount( UsbStorageDevice* pDevice )
{
	ASSERT( !pDevice->sDevice.empty() );
	
        CString sCommand = "mount " + pDevice->sDevice;
        bool bMountedSuccessfully = ExecuteCommand( sCommand );

	return bMountedSuccessfully;
}

void MemoryCardDriverThreaded_Linux::Unmount( UsbStorageDevice* pDevice )
{
	if( pDevice->sDevice.empty() )
		return;
	
	/* Use umount -l, so we unmount the device even if it's in use.  Open
	 * files remain usable, and the device (eg. /dev/sda) won't be reused
	 * by new devices until those are closed.  Without this, if something
	 * causes the device to not unmount here, we'll never unmount it; that
	 * causes a device name leak, eventually running us out of mountpoints. */
	CString sCommand = "umount -l \"" + pDevice->sDevice + "\"";
	ExecuteCommand( sCommand );
}

void MemoryCardDriverThreaded_Linux::Flush( UsbStorageDevice* pDevice )
{
	if( pDevice->sDevice.empty() )
		return;
	
	// "sync" will only flush all file systems at the same time.  -Chris
	// I don't think so.  Also, sync() merely queues a flush; it doesn't guarantee
	// that the flush is completed on return.  However, we can mount the filesystem
	// with the flag "-o sync", which forces synchronous access (but that's probably
	// very slow.) -glenn
	ExecuteCommand( "mount -o remount " + pDevice->sDevice );
}

/*
 * (c) 2003-2005 Chris Danford, Glenn Maynard
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
