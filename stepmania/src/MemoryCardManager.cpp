#include "global.h"
/*
-----------------------------------------------------------------------------
 Class: MemoryCardManager

 Desc: See Header.

 Copyright (c) 2001-2003 by the person(s) listed below.  All rights reserved.
	Chris Danford
	Chris Gomez
-----------------------------------------------------------------------------
*/

#include "MemoryCardManager.h"

MemoryCardManager*	MEMCARDMAN = NULL;	// global and accessable from anywhere in our program

//
// TODO: Change this to use arch/driver format
//
#ifdef LINUX

	#include <fstream>
	#include "RageLog.h"
	#include "RageUtil.h"
	#include "ScreenManager.h"
	#include <stdio.h>
	#include <unistd.h>
	#include <fcntl.h>
	#include <linux/types.h>

	static const char *USB_DEVICE_LIST_FILE = "/proc/bus/usb/devices";
	static int g_fds = -1;
	static time_t g_last_mtime = 0;

	struct UsbStorageDevice
	{
		UsbStorageDevice()
		{
			iBus = -1;
			iDeviceOnBus = -1;
			iPortOnHub = -1;
			iUsbStorageIndex = -1;
		};
		int iBus;
		int iDeviceOnBus;
		int iPortOnHub;
		CString sSerial;
		int iUsbStorageIndex;
		CString	sOsMountDir;	// WITHOUT trailing slash

		bool operator==(const UsbStorageDevice& other)
		{
			return 
				iBus==other.iBus &&
				iDeviceOnBus==other.iDeviceOnBus &&
				iPortOnHub==other.iPortOnHub &&
				sSerial==other.sSerial &&
				iUsbStorageIndex==other.iUsbStorageIndex &&
				sOsMountDir==other.sOsMountDir;
		}
	};

	vector<UsbStorageDevice> g_StorageDevices;

	void UpdateAttachedUsbStorageDevices()
	{
		vector<UsbStorageDevice> &vDevicesOut = g_StorageDevices;

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
		return;
	}


	MemoryCardManager::MemoryCardManager()
	{
		g_fds = open(USB_DEVICE_LIST_FILE, O_RDONLY);
		ASSERT( g_fds != -1 );
	}

	MemoryCardManager::~MemoryCardManager()
	{
		if( g_fds != -1 ) 
		{
			close( g_fds );
			g_fds = -1;
		}
	}


	bool UsbChanged()
	{
		// has USB_DEVICE_LIST_FILE changed?
		if( g_fds == -1 )
			return false;

		struct stat st;
		if( fstat(g_fds, &st) == -1 )
		{
			LOG->Warn( "stat failed." );
			return false;
		}

		bool bChanged = st.st_mtime != g_last_mtime;
		g_last_mtime = st.st_mtime;
	       
		return bChanged;
	}

	void MemoryCardManager::Update( float fDelta )
	{
		if( UsbChanged() )
		{
		  SCREENMAN->SystemMessage( "USB changed" );
		  return;
			vector<UsbStorageDevice> vOld = g_StorageDevices;	// make a copy
			UpdateAttachedUsbStorageDevices();
			vector<UsbStorageDevice> &vNew = g_StorageDevices;

			unsigned i;

			// check for disconnects
			for( i=0; i<vOld.size(); i++ )
			{
				const UsbStorageDevice old = vOld[i];
				if( find(vNew.begin(),vNew.end(),old) == vNew.end() )
					SCREENMAN->SystemMessage( ssprintf("Disconnected bus %d port %d device %d", old.iBus, old.iPortOnHub, old.iDeviceOnBus) );
			}

			// check for connects
			for( i=0; i<vNew.size(); i++ )
			{
				const UsbStorageDevice newd = vNew[i];
				if( find(vOld.begin(),vOld.end(),newd) == vOld.end() )
					SCREENMAN->SystemMessage( ssprintf("Connected bus %d port %d device %d", newd.iBus, newd.iPortOnHub, newd.iDeviceOnBus) );
			}
		}
	}

#else	// !LINUX

	MemoryCardManager::MemoryCardManager() {}
	MemoryCardManager::~MemoryCardManager() {}
	void MemoryCardManager::Update( float fDelta ) {}

#endif
