#include "global.h"
#include "MemoryCardDriver_Linux.h"
#include "RageLog.h"
#include "RageUtil.h"

#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <linux/types.h>

#include <fstream>

static const char *USB_DEVICE_LIST_FILE = "/proc/bus/usb/devices";

MemoryCardDriver_Linux::MemoryCardDriver_Linux()
{
	m_lastModTime = 0;
	m_fds = open(USB_DEVICE_LIST_FILE, O_RDONLY);
	if( m_fds == -1 )
		LOG->Trace( "Failed to open \"%s\": %s", USB_DEVICE_LIST_FILE, strerror(errno) );
}

MemoryCardDriver_Linux::~MemoryCardDriver_Linux()
{
	if( m_fds != -1 ) 
	{
		close( m_fds );
		m_fds = -1;
	}
}

bool MemoryCardDriver_Linux::StorageDevicesChanged()
{
	// has USB_DEVICE_LIST_FILE changed?
	if( m_fds == -1 )	// file not opened
		return false;	// we'll never know...

	struct stat st;
	if( fstat(m_fds, &st) == -1 )
	{
		LOG->Warn( "stat of '%s' failed.", USB_DEVICE_LIST_FILE );
		return false;
	}

	bool bChanged = st.st_mtime != m_lastModTime;
	m_lastModTime = st.st_mtime;
	    
	return bChanged;
}

void MemoryCardDriver_Linux::GetStorageDevices( vector<UsbStorageDevice>& vDevicesOut )
{
	/* If we couldn't open it before, we probably can't open it now; don't
	 * output more errors. */
	if( m_fds == -1 )
		return;

	vDevicesOut.clear();

	{
		// Find all attached USB devices.  Output looks like:

		// T:  Bus=02 Lev=00 Prnt=00 Port=00 Cnt=00 Dev#=  1 Spd=12  MxCh= 2
		// B:  Alloc=  0/900 us ( 0%), #Int=  0, #Iso=  0
		// D:  Ver= 1.00 Cls=09(hub  ) Sub=00 Prot=00 MxPS= 8 #Cfgs=  1
		// P:  Vendor=0000 ProdID=0000 Rev= 0.00
		// S:  Product=USB UHCI Root Hub
		// S:  SerialNumber=ff80
		// C:* #Ifs= 1 Cfg#= 1 Atr=40 MxPwr=  0mA
		// I:  If#= 0 Alt= 0 #EPs= 1 Cls=09(hub  ) Sub=00 Prot=00 Driver=hub
		// E:  Ad=81(I) Atr=03(Int.) MxPS=   8 Ivl=255ms
		// T:  Bus=02 Lev=01 Prnt=01 Port=00 Cnt=01 Dev#=  2 Spd=12  MxCh= 0
		// D:  Ver= 1.10 Cls=00(>ifc ) Sub=00 Prot=00 MxPS= 8 #Cfgs=  1
		// P:  Vendor=04e8 ProdID=0100 Rev= 0.01
		// S:  Manufacturer=KINGSTON     
		// S:  Product=USB DRIVE    
		// S:  SerialNumber=1125198948886
		// C:* #Ifs= 1 Cfg#= 1 Atr=80 MxPwr= 90mA
		// I:  If#= 0 Alt= 0 #EPs= 2 Cls=08(stor.) Sub=06 Prot=50 Driver=usb-storage
		// E:  Ad=82(I) Atr=02(Bulk) MxPS=  64 Ivl=0ms
		// E:  Ad=03(O) Atr=02(Bulk) MxPS=  64 Ivl=0ms

		ifstream f;
		CString fn = "/proc/bus/usb/devices";
		f.open(fn);
		if( !f.is_open() )
		{
			LOG->Warn( "can't open '%s'", fn.c_str() );
			return;
		}

		UsbStorageDevice usbd;
		CString sLine;
		while( getline(f, sLine) )
		{
			int iRet, iThrowAway;

			// T:  Bus=02 Lev=00 Prnt=00 Port=00 Cnt=00 Dev#=  1 Spd=12  MxCh= 2
			int iBus, iPort, iDevice;
			iRet = sscanf( sLine.c_str(), "T:  Bus=%d Lev=%d Prnt=%d Port=%d Cnt=%d Dev#=%d Spd=%d  MxCh=%d", &iBus, &iThrowAway, &iThrowAway, &iPort, &iThrowAway, &iDevice, &iThrowAway, &iThrowAway );
			if( iRet == 8 )
			{
				usbd.iBus = iBus;
				usbd.iDeviceOnBus = iDevice;
				usbd.iPortOnHub = iPort;
				continue;	// stop processing this line
			}

			// S:  SerialNumber=ff80
			char szSerial[1024];
			iRet = sscanf( sLine.c_str(), "S:  SerialNumber=%[^\n]", szSerial );
			if( iRet == 1 )
			{
				usbd.sSerial = szSerial;
				continue;	// stop processing this line
			}
			
			// I:  If#= 0 Alt= 0 #EPs= 2 Cls=08(stor.) Sub=06 Prot=50 Driver=usb-storage
			int iClass;
			iRet = sscanf( sLine.c_str(), "I:  If#=%d Alt=%d #EPs=%d Cls=%d", &iThrowAway, &iThrowAway, &iThrowAway, &iClass );
			if( iRet == 4 )
			{
				if( iClass == 8 )	// storage class
					vDevicesOut.push_back( usbd );
				continue;	// stop processing this line
			}
		}
	}

	{
		// Find the usb-storage device index for all storage class devices.
		for( unsigned i=0; i<vDevicesOut.size(); i++ )
		{
			UsbStorageDevice& usbd = vDevicesOut[i];
			// Output looks like:

			//    Host scsi0: usb-storage
			//        Vendor: KINGSTON     
			//       Product: USB DRIVE    
			// Serial Number: 1125198948886
			//      Protocol: Transparent SCSI
			//     Transport: Bulk
			//          GUID: 04e801000001125198948886
			//      Attached: Yes
	
			CString fn = ssprintf( "/proc/scsi/usb-storage-%d/%d", i, i );
			ifstream f;
			f.open(fn);
			if( !f.is_open() )
			{
				LOG->Warn( "can't open '%s'", fn.c_str() );
				return;
			}

			CString sLine;
			while( getline(f, sLine) )
			{
				// Serial Number: 1125198948886
				char szSerial[1024];
				int iRet = sscanf( sLine.c_str(), "Serial Number: %[^\n]", szSerial );
				if( iRet == 1 )	// we found our line
				{
					usbd.iUsbStorageIndex = i;

					LOG->Trace( "iUsbStorageIndex: %d, iBus: %d, iDeviceOnBus: %d, iPortOnHub: %d",
						usbd.iUsbStorageIndex, usbd.iBus, usbd.iDeviceOnBus, usbd.iPortOnHub );

					break;	// stop looking
				}
			}
		}
	}

	{
		// Find where each device is mounted. Output looks like:

		// /dev/sda1               /mnt/flash1             auto    noauto,owner 0 0
		// /dev/sdb1               /mnt/flash2             auto    noauto,owner 0 0
		// /dev/sdc1               /mnt/flash3             auto    noauto,owner 0 0

		CString fn = "/etc/fstab";
		ifstream f;
		f.open(fn);
		if( !f.is_open() )
		{
			LOG->Warn( "can't open '%s'", fn.c_str() );
			return;
		}

		CString sLine;
		while( getline(f, sLine) )
		{
			// /dev/sda1               /mnt/flash1             auto    noauto,owner 0 0
			char cScsiDev;
			char szMountPoint[1024];
			int iRet = sscanf( sLine.c_str(), "/dev/sd%c1 %s", &cScsiDev, szMountPoint );
			if( iRet != 2 )
				continue;	// don't process this line

			int iUsbStorageIndex = cScsiDev - 'a';
			CString sMountPoint = szMountPoint;
			TrimLeft( sMountPoint );
			TrimRight( sMountPoint );

			// search for the usb-storage device corresponding to the SCSI device
			for( unsigned i=0; i<vDevicesOut.size(); i++ )
			{
				UsbStorageDevice& usbd = vDevicesOut[i];
				if( usbd.iUsbStorageIndex == iUsbStorageIndex )	// found our match
				{
					usbd.sOsMountDir = sMountPoint;
					break;	// stop looking for a match
				}
			}
		}
	}
}

bool MemoryCardDriver_Linux::MountAndTestWrite( UsbStorageDevice* pDevice )
{
	if( pDevice->sOsMountDir.empty() )
		return false;

	CString sCommand = "umount " + pDevice->sOsMountDir;
	LOG->Trace( "executing '%s'", sCommand.c_str() );
	system( sCommand );
	sCommand = "mount " + pDevice->sOsMountDir;
	LOG->Trace( "executing '%s'", sCommand.c_str() );
	system( sCommand );

	// Try to write a file.
	// TODO: Can we use RageFile for this?
	CString sFile = pDevice->sOsMountDir;
	if( sFile[sFile.length()-1] != '/' )
		sFile += '/';
	sFile += "temp";
	FILE* fp = fopen( sFile, "w" );
	if( fp == NULL )
		return false;
	fclose( fp );
	remove( sFile );

	return true;
}

void MemoryCardDriver_Linux::Flush( UsbStorageDevice* pDevice )
{
	if( !pDevice->sOsMountDir.empty() )
		return;

	// unmount and mount again.  Is there a better way to flush?
	// "sync" will only flush all file systems at the same time.  -Chris
	CString sCommand = "umount " + pDevice->sOsMountDir;
	LOG->Trace( "executing '%s'", sCommand.c_str() );
	system( sCommand );
	sCommand = "mount " + pDevice->sOsMountDir;
	system( sCommand );
}

void MemoryCardDriver_Linux::ResetUsbStorage()
{
	system( "rmmod usb-storage" );
	system( "modprobe usb-storage" );
}


/*
 * Copyright (c) 2003 by the person(s) listed below.  All rights reserved.
 *	Chris Danford
 */
