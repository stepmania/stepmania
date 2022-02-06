#include "global.h"
#include "MemoryCardDriverThreaded_Linux.h"
#include "RageLog.h"
#include "RageUtil.h"
#include "RageTimer.h"

#include <cerrno>
#if defined(HAVE_FCNTL_H)
#include <fcntl.h>
#endif

#if defined(HAVE_DIRENT_H)
#include <dirent.h>
#endif

#include <cstring>
#include <fstream>
#include <limits.h>
#include <stdlib.h>
#include <string>
#include <sys/types.h>
#include <sys/stat.h>

bool MemoryCardDriverThreaded_Linux::TestWrite( UsbStorageDevice* pDevice )
{
	if( access(pDevice->sOsMountDir, W_OK) == -1 )
	{
		pDevice->SetError( "TestFailed" );
		return false;
	}

	return true;
}

static bool ExecuteCommand( const RString &sCommand )
{
	LOG->Trace( "executing '%s'", sCommand.c_str() );
	int ret = system(sCommand);
	LOG->Trace( "done executing '%s'", sCommand.c_str() );
	if( ret != 0 )
	{
		RString sError = ssprintf("failed to execute '%s' with error %d", sCommand.c_str(), ret);
		if( ret == -1 )
			sError += ssprintf(": %s", sCommand.c_str());
		LOG->Warn( "%s", sError.c_str() );
	}
	return ret == 0;
}

static bool ReadFile( const RString &sPath, RString &sBuf )
{
	sBuf.clear();

	int fd = open( sPath, O_RDONLY );
	if( fd == -1 )
	{
		// "No such file or directory" is understandable
		if (errno != ENOENT)
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

static void GetFileList( const RString &sPath, vector<RString> &out )
{
	out.clear();

	DIR *dp = opendir( sPath );
	if( dp == nullptr )
		return; // false; // XXX warn

	while( const struct dirent *ent = readdir(dp) )
		out.push_back( ent->d_name );

	closedir( dp );
}

bool MemoryCardDriverThreaded_Linux::USBStorageDevicesChanged()
{
	RString sThisDevices;

	/* If a device is removed and reinserted, the inode of the /sys/block entry
	 * will change. */
	RString sDevicePath = "/sys/block/";
	
	vector<RString> asDevices;
	GetFileList( sDevicePath, asDevices );

	for( unsigned i = 0; i < asDevices.size(); ++i )
	{
		struct stat buf;
		if( stat( sDevicePath + asDevices[i], &buf ) == -1 )
			continue; // XXX warn

		sThisDevices += ssprintf( "%i,", (int) buf.st_ino );
	}
	       
	bool bChanged = sThisDevices != m_sLastDevices;
	m_sLastDevices = sThisDevices;
	if( bChanged )
		LOG->Trace( "Change in USB storage devices detected." );
	return bChanged;
}

void MemoryCardDriverThreaded_Linux::GetUSBStorageDevices( vector<UsbStorageDevice>& vDevicesOut )
{
	LOG->Trace( "GetUSBStorageDevices" );
	
	vDevicesOut.clear();

	{
		vector<RString> asDevices;
		RString sBlockDevicePath = "/sys/block/";
		GetFileList( sBlockDevicePath, asDevices );

		for( unsigned i = 0; i < asDevices.size(); ++i )
		{
			const RString &sDevice = asDevices[i];
			if( sDevice == "." || sDevice == ".." )
				continue;

			UsbStorageDevice usbd;

			RString sPath = sBlockDevicePath + sDevice + "/";
			usbd.sSysPath = sPath;

			/* Ignore non-removable devices. */
			RString sBuf;
			if( !ReadFile( sPath + "removable", sBuf ) )
				continue; // already warned
			if( atoi(sBuf) != 1 )
				continue;

			/*
			 * The kernel isn't exposing all of /sys atomically, so we end up missing
			 * the partition due to it not being shown yet.  It won't show up until the
			 * kernel has scanned the partition table, which can take a variable amount
			 * of time, sometimes over a second.  Watch for the "queue" sysfs directory,
			 * which is created after this, to tell when partition directories are created.
			 */
			RageTimer WaitUntil;
			WaitUntil += 5;
			RString sQueueFilePath = usbd.sSysPath + "queue";
			while(1)
			{
				if( WaitUntil.Ago() >= 0 )
				{
					LOG->Warn( "Timed out waiting for %s", sQueueFilePath.c_str() );
					break;
				}

				if( access(usbd.sSysPath, F_OK) == -1 )
				{
					LOG->Warn( "Block directory %s went away while we were waiting for %s",
							usbd.sSysPath.c_str(), sQueueFilePath.c_str() );
					break;
				}

				if( access(sQueueFilePath, F_OK) != -1 )
					break;

				usleep(10000);
			}

			/* Wait for udev to finish handling device node creation */
			ExecuteCommand( "udevadm settle" );

			/* If the first partition device exists, eg. /sys/block/uba/uba1, use it. */
			if( access(usbd.sSysPath + sDevice + "1", F_OK) != -1 )
			{
				LOG->Trace("OK");
				usbd.sDevice = "/dev/" + sDevice + "1";
			}
			else
			{
				LOG->Trace("error %s", strerror(errno));
				usbd.sDevice = "/dev/" + sDevice;
			}

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
				 * In newer kernels, it looks like:
				 *
				 * ../../../3-2.1:1.0
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
				vector<RString> asBits;
				split( szLink, "/", asBits );

				RString sHostPort = asBits[asBits.size()-1];
				if( !sHostPort.empty() )
				{
					/* Strip off the endpoint information after the colon. */
					size_t pos = sHostPort.find(':');
					if( pos != string::npos )
						sHostPort.erase( pos );
					
					/* sHostPort is eg. 2-2.1. */
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

		std::ifstream f("/etc/fstab");
		if (f.fail())
		{
			LOG->Warn( "can't open '/etc/fstab': %s", strerror(errno) );
			return;
		}

		std::string line;
		while( !f.eof() )
		{
			std::getline(f, line);
			if (f.eof())
			{
				continue;
			}
			else if (f.fail())
			{
				LOG->Warn( "error reading '/etc/fstab': %s", strerror(errno) );
				return;
			}

			char szScsiDevice[1024];
			char szMountPoint[1024];
			int iRet = sscanf( line.c_str(), "%s %s", szScsiDevice, szMountPoint );
			if( iRet != 2 || szScsiDevice[0] == '#')
				continue;	// don't process this line

			/* Get the real kernel device name, which should match
			 * the name from /sys/block, by following symlinks in
			 * /dev.  This allows us to specify persistent names in
			 * /etc/fstab using things like /dev/device/by-path. */
			char szUnderlyingDevice[PATH_MAX];
			if( realpath(szScsiDevice, szUnderlyingDevice) == nullptr )
			{
				// "No such file or directory" is understandable
				if (errno != ENOENT)
					LOG->Warn( "realpath(\"%s\"): %s", szScsiDevice, strerror(errno) );
				continue;
			}

			RString sMountPoint = szMountPoint;
			TrimLeft( sMountPoint );
			TrimRight( sMountPoint );

			// search for the mountpoint corresponding to the device
			for( unsigned i=0; i<vDevicesOut.size(); i++ )
			{
				UsbStorageDevice& usbd = vDevicesOut[i];
				if( usbd.sDevice == szUnderlyingDevice )	// found our match
				{
					// Use the device entry from fstab so the mount command works
					usbd.sDevice = szScsiDevice;
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
	
	LOG->Trace( "Done with GetUSBStorageDevices" );
}


bool MemoryCardDriverThreaded_Linux::Mount( UsbStorageDevice* pDevice )
{
	ASSERT( !pDevice->sDevice.empty() );
	
        RString sCommand = "mount " + pDevice->sDevice;
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
	RString sCommand = "sync; umount -l \"" + pDevice->sDevice + "\"";
	ExecuteCommand( sCommand );
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
