#include "global.h"
#include "MemoryCardDriverThreaded_Linux.h"
#include "RageLog.h"
#include "RageUtil.h"
#include "RageFileManager.h"
#include "Profile.h"
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

const CString TEMP_MOUNT_POINT = "@mctemp/";

static const char *USB_DEVICE_LIST_FILE = "/proc/bus/usb/devices";
static const char *ETC_MTAB = "/etc/mtab";

void GetNewStorageDevices( vector<UsbStorageDevice>& vDevicesOut );

static int RunProgram( const char *path, char *const args[], CString &out )
{
	LOG->Trace( "Running \"%s\"", path );
	int fds[2];
	if( pipe(fds) == -1 )
		return -1;
	
	int pid = fork();
	if( pid == -1 )
    {
		close( fds[0] );
		close( fds[1] );
		return -1;
    }
	
	if( pid == 0 )
    {
		close( fds[0] );
		close( fileno(stdout) );
		
		dup2( fds[1], fileno(stdout) );
		
		for( int i = 0; i < 1024; ++i )
			if( i != fileno(stdout) )
				close( i );
			execvp( path, args );
			_exit(1);
    }
	
	close( fds[1] );
	
	while( 1 )
    {
		char buf[1024];
		int got = read( fds[0], buf, sizeof(buf) );
		if( got == -1 )
		{
			if( errno == EINTR )
				continue;
			fprintf( stderr, "err %s\n", strerror(errno) );
			exit(0);
		}
		
		if( got == 0 )
			break;
		
		out.append( buf, got );
    }
	
	close( fds[0] );
	int status;
	int ret = waitpid( pid, &status, 0 );
	
	if( ret == -1 )
		return -1;
	
	if( !WIFEXITED(status) )
		return -1; /* signal */
	
	return WEXITSTATUS(status);
}


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
	m_fd = open(USB_DEVICE_LIST_FILE, O_RDONLY);
	if( m_fd == -1 )
		LOG->Warn( "Failed to open \"%s\": %s", USB_DEVICE_LIST_FILE, strerror(errno) );
	
	this->StartThread();
}

MemoryCardDriverThreaded_Linux::~MemoryCardDriverThreaded_Linux()
{
	this->StopThread();
	if( m_fd != -1 )
		close( m_fd );
}

void MemoryCardDriverThreaded_Linux::ResetUsbStorage()
{
	//
	// if usb-storage gets in a bad state, resetting usb-storage will sometimes fix it.
	//
	
	// unmount all devices before trying to remove the module
	FOREACH( UsbStorageDevice, m_vDevicesLastSeen, d )
    {
		CString sCommand = "umount " + d->sOsMountDir;
		LOG->Trace( "reset unmount (%s)", sCommand.c_str() );
		ExecuteCommand( sCommand );
		LOG->Trace( "reset unmount done" );
		
		// force a remount the next time cards aren't locked
		d->bNeedsWriteTest = true;
		d->bWriteTestSucceeded = false;
    }
	
	ExecuteCommand( "rmmod usb-storage" );
	ExecuteCommand( "modprobe usb-storage" );
	
	MountThreadDoOneUpdate();
}

void MemoryCardDriverThreaded_Linux::MountThreadDoOneUpdate()
{
	if( m_fd == -1 )
	{
		usleep( 50000 );
		return;
	}
	
	bool bNeedToDoAnyMounts = false;
	for( unsigned i=0; i<m_vDevicesLastSeen.size(); i++ )
	{
		UsbStorageDevice &d = m_vDevicesLastSeen[i];
		if( d.bNeedsWriteTest )
		{
			bNeedToDoAnyMounts = true;
			break;
		}
	}
	
	if( bNeedToDoAnyMounts )
	{
		// fall through
	}
	else
	{
		pollfd pfd = { m_fd, POLLIN, 0 };
		int ret = poll( &pfd, 1, 100 );
		switch( ret )
		{
		case 1:
			// file changed.  Fall through.
			break;
		case 0: // no change.  Poll again.
			return;
		case -1:
			if( errno != EINTR )
				LOG->Warn( "Error polling: %s", strerror(errno) );
			return;
		}
	}
	
	// TRICKY: We're waiting for a change in the USB device list, but 
	// the usb-storage descriptors take a bit longer to update.  It's more convenient to wait
	// on the USB device list because the usb-storage descriptors are separate files per 
	// device.  So, sleep for a little bit of time after we detect a new USB device and give
	// usb-storage a chance to initialize.  
	usleep(1000*300);
	
	vector<UsbStorageDevice> vNew;
	GetNewStorageDevices( vNew );
	vector<UsbStorageDevice> vOld = m_vDevicesLastSeen; // copy
	
	// check for disconnects
	vector<UsbStorageDevice> vDisconnects;
	FOREACH( UsbStorageDevice, vOld, old )
	{
		vector<UsbStorageDevice>::iterator iter = find( vNew.begin(), vNew.end(), *old );
		if( iter == vNew.end() )    // didn't find
		{
			LOG->Trace( "Disconnected bus %d port %d level %d path %s", old->iBus, old->iPort, old->iLevel, old->sOsMountDir.c_str() );
			vDisconnects.push_back( *old );
			
			vector<UsbStorageDevice>::iterator iter = find( m_vDevicesLastSeen.begin(), m_vDevicesLastSeen.end(), *old );
			ASSERT( iter != m_vDevicesLastSeen.end() );
			m_vDevicesLastSeen.erase( iter );
		}
	}
	
	
	// check for connects
	vector<UsbStorageDevice> vConnects;
	FOREACH( UsbStorageDevice, vNew, newd )
	{
		vector<UsbStorageDevice>::iterator iter = find( vOld.begin(), vOld.end(), *newd );
		if( iter == vOld.end() )    // didn't find
		{
			LOG->Trace( "Connected bus %d port %d level %d path %s", newd->iBus, newd->iPort, newd->iLevel, newd->sOsMountDir.c_str() );
			vConnects.push_back( *newd );
			
			m_vDevicesLastSeen.push_back( *newd );
		}
	}
	
	bool bDidAnyMounts = false;	
	
	// unmount all disconnects
	if( ShouldDoOsMount() )
	{
		for( unsigned i=0; i<vDisconnects.size(); i++ )
		{
			UsbStorageDevice &d = vDisconnects[i];
			CString sCommand = "umount " + d.sOsMountDir;
			LOG->Trace( "unmount disconnects %i/%i (%s)", i, vDisconnects.size(), sCommand.c_str() );
			ExecuteCommand( sCommand );
			LOG->Trace( "unmount disconnects %i/%i done", i, vDisconnects.size() );
		}
		
		// mount all devices that need a write test
		for( unsigned i=0; i<m_vDevicesLastSeen.size(); i++ )
		{	  
			UsbStorageDevice &d = m_vDevicesLastSeen[i];
			if( !d.bNeedsWriteTest )
				continue;  // skip
			
			bDidAnyMounts = true;
			
			d.bNeedsWriteTest = false;
			
			CString sCommand;
			
			// unmount this device before trying to mount it.  If this device
			// wasn't unmounted before, then our mount call will fail and the 
			// mount may contain an out-of-date view of the files on the device.
			sCommand = "umount " + d.sOsMountDir;
			LOG->Trace( "unmount old connect %i/%i (%s)", i, vConnects.size(), sCommand.c_str() );
			ExecuteCommand( sCommand );   // don't care if this fails
			LOG->Trace( "unmount old connect %i/%i done", i, vConnects.size() );
			
			sCommand = "mount " + d.sOsMountDir;
			LOG->Trace( "mount new connect %i/%i (%s)", i, vConnects.size(), sCommand.c_str() );
			bool bMountedSuccessfully = ExecuteCommand( sCommand );
			LOG->Trace( "mount new connect %i/%i done", i, vConnects.size() );
			
			d.bWriteTestSucceeded = bMountedSuccessfully && TestWrite( d.sOsMountDir );
			
			// read name
			this->Mount( &d, TEMP_MOUNT_POINT );
			FILEMAN->FlushDirCache( TEMP_MOUNT_POINT );
			Profile profile;
			CString sProfileDir = TEMP_MOUNT_POINT + PREFSMAN->m_sMemoryCardProfileSubdir + '/'; 
			profile.LoadEditableDataFromDir( sProfileDir );
			d.sName = profile.GetDisplayName();
			UnmountMountPoint( TEMP_MOUNT_POINT );
			
			LOG->Trace( "WriteTest: %s, Name: %s", d.bWriteTestSucceeded ? "succeeded" : "failed", d.sName.c_str() );
		}
	}
	
	if( bDidAnyMounts || !vDisconnects.empty() || !vConnects.empty() )	  
	{
		LockMut( m_mutexStorageDevices );
		m_bStorageDevicesChanged = true;
		m_vStorageDevices = m_vDevicesLastSeen;
	}
	
	CHECKPOINT;
}

bool ReadUsbStorageDescriptor( CString fn, int iScsiIndex, vector<UsbStorageDevice>& vDevicesOut )
{
	LOG->Trace( "ReadUsbStorageDescriptor %s", fn.c_str() );
	
	// Read the usb-storage descriptor.  It looks like:
	
	//    Host scsi0: usb-storage
	//        Vendor: KINGSTON
	//       Product: USB DRIVE
	// Serial Number: 1125198948886
	//      Protocol: Transparent SCSI
	//     Transport: Bulk
	//          GUID: 04e801000001125198948886
	//      Attached: Yes
	
	ifstream f;
	f.open(fn);
	if( !f.is_open() )
		return false;
	
	CString sLine;
	while( getline(f, sLine) )
    {
		// Serial Number: 1125198948886
		char szSerial[1024];
		int iRet = sscanf( sLine.c_str(), "Serial Number: %[^\n]", szSerial );
		if( iRet == 1 ) // we found our line
		{
			// Search for the device corresponding to this serial number.
			for( unsigned j=0; j<vDevicesOut.size(); j++ )
			{
				UsbStorageDevice& usbd = vDevicesOut[j];
				
				if( usbd.sSerial == szSerial )
				{
					usbd.iScsiIndex = iScsiIndex;
					LOG->Trace( "iScsiIndex: %d, iBus: %d, iLevel: %d, iPort: %d, sSerial: %s",
						usbd.iScsiIndex, usbd.iBus, usbd.iLevel, usbd.iPort, usbd.sSerial.c_str() );
					break;  // done looking for the corresponding device.
				}
			}
			break;  // we already found the line we care about
		}
    }
	return true;
}

#if 0
/* usbd must have the serial number filled in; fill in iPort and iLevel. */
static bool GetPortAndLevelFromSerial( CString sSerial, UsbStorageDevice &usbd )
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
	
	RageFile f;
	CString fn = "/proc/bus/usb/devices";
	LOG->Trace( fn );
	if( !f.Open(fn) )
	{
		LOG->Warn( "can't open \"%s\": %s", fn.c_str(), f.GetError().c_str() );
		return false;
	}
	
	CString sLine;
	int iPort, iLevel;
	while( f.GetLine(sLine) > 0 )
	{
		int iRet, iThrowAway;
		
		// T:  Bus=02 Lev=00 Prnt=00 Port=00 Cnt=00 Dev#=  1 Spd=12  MxCh= 2
		iRet = sscanf( sLine.c_str(), "T:  Bus=%d Lev=%d Prnt=%d Port=%d Cnt=%d Dev#=%d Spd=%d  MxCh=%d", &iThrowAway, &iLevel, &iThrowAway, &iPort, &iThrowAway, &iThrowAway, &iThrowAway, &iThrowAway );
		if( iRet == 8 )
			continue;	// stop processing this line
		
		// S:  SerialNumber=ff80
		char szSerial[1024];
		iRet = sscanf( sLine.c_str(), "S:  SerialNumber=%[^\n]", szSerial );
		if( iRet == 1 )
		{
			if( sSerial == szSerial )
			{
				usbd.iPort = iPort;
				usbd.iLevel = iLevel;
				/* got what we came for */
				return true;
			}
			
			continue;	// stop processing this line
		}
	}
}
#endif

void GetNewStorageDevices( vector<UsbStorageDevice>& vDevicesOut )
{
	LOG->Trace( "GetNewStorageDevices" );
	
	vDevicesOut.clear();
	
	{
		// Bus 002 Device 001: ID 0000:0000
		// Port 4
		//   iSerial                 3 1125198948886
		//       bInterfaceClass         8 Mass Storage
		
		// Don't include a path.  The usbutils installer script installs 
		// to /usr/local/sbin and the Debian package installs to /usr/sbin/.
		CString sCommand = "lsusb";
		char *szParams[] = { "lsusb", "-v", NULL }; 
		CString sOutput;
		RunProgram( sCommand, szParams, sOutput );
		
		CStringArray vsLines;
		split( sOutput, "\n", vsLines );
		
		UsbStorageDevice usbd;
		for( unsigned i=0; i<vsLines.size(); i++ )
		{
			CString &sLine = vsLines[i];
			
			int iRet, iThrowAway;
			
			// Bus 002 Device 001: ID 0000:0000
			int iBus;
			iRet = sscanf( sLine.c_str(), "Bus %d", &iBus );
			if( iRet == 1 )
			{
				usbd.iBus = iBus;
				continue;       // stop processing this line
			}
			
			// Port 4
			int iPort;
			iRet = sscanf( sLine.c_str(), "Port %d", &iPort );
			if( iRet == 1 )
			{
				usbd.iPort = iPort;
				continue;       // stop processing this line
			}		
			
			//   iSerial                 3 1125198948886
			char szSerial[1024];
			iRet = sscanf( sLine.c_str(), "  iSerial %d %[^\n]", &iThrowAway, szSerial );
			if( iRet == 2 )
			{
				usbd.sSerial = szSerial;
				continue;       // stop processing this line
			}
			
			//       bInterfaceClass         8 Mass Storage
			int iClass;
			iRet = sscanf( sLine.c_str(), "      bInterfaceClass %d", &iClass );
			if( iRet == 1 )
			{
				if( iClass == 8 )       // storage class
				{		      
					vDevicesOut.push_back( usbd );
					LOG->Trace( "iScsiIndex: %d, iBus: %d, iLevel: %d, iPort: %d, sSerial  = %s",
						usbd.iScsiIndex, usbd.iBus, usbd.iLevel, usbd.iPort, usbd.sSerial.c_str() );
				}
				continue;       // stop processing this line
			}
		}
	}
	
	{
		// Find the usb-storage device index for all storage class devices.
		
		const CString sDir = "/proc/scsi/usb-storage/";
		LOG->Trace( "Scanning %s", sDir.c_str() );
		DIR *dirp = opendir( sDir );
		if( dirp )
		{
			// 2.6 kernel style
			
			struct dirent *direntp;
			while ( (direntp = readdir( dirp )) != NULL )
			{
				if( stricmp(direntp->d_name,".")==0 || stricmp(direntp->d_name,"..")==0 )
					continue;
				
				int iScsiIndex = atoi( direntp->d_name );
				CString fn = sDir + direntp->d_name;
				
				ReadUsbStorageDescriptor( fn, iScsiIndex, vDevicesOut );
			}
			closedir( dirp );
			dirp = NULL;
			
			
			// Get the mapping from Scsi device number to Scsi file device.  Requires "sg-utils".
			// sg_scan.  It looks like:
			
			// /dev/sg0: scsi47 channel=0 id=0 lun=0 [em]  type=0
			// /dev/sg1: scsi46 channel=0 id=0 lun=0 [em]  type=0
			
			CString sOutput;
			CString sCommand = "/usr/bin/sg_scan";
			LOG->Trace( sCommand );
			RunProgram( sCommand, NULL, sOutput );
			
			CStringArray vsLines;
			split( sOutput, "\n", vsLines );
			
			for( unsigned i=0; i<vsLines.size(); i++ )
			{
				const CString& sLine = vsLines[i];
				
				LOG->Trace( "sLine: %s", sLine.c_str() );
				
				int iSg;
				int iScsiIndex;
				int iRet = sscanf( sLine.c_str(), "/dev/sg%i: scsi%d", &iSg, &iScsiIndex );
				if( iRet != 2 )
					continue;       // don't process this line
				
				// search for the usb-storage device corresponding to the SCSI device
				for( unsigned i=0; i<vDevicesOut.size(); i++ )
				{
					UsbStorageDevice& usbd = vDevicesOut[i];
					if( usbd.iScsiIndex == iScsiIndex ) // found our match
					{
						usbd.sScsiDevice = ssprintf("/dev/sd%c",'a'+(char)iSg);
						
						LOG->Trace( "iScsiIndex: %d, iBus: %d, iLevel: %d, iPort: %d, sScsiDevice: %s",
							usbd.iScsiIndex, usbd.iBus, usbd.iLevel, usbd.iPort, usbd.sScsiDevice.c_str() );
						break;  // stop looking for a match
					}
				}
			}
		}
		else
		{
			// 2.4 kernel style
			
			// Find the usb-storage device index for all storage class devices.
			for( unsigned i=0; true; i++ )
			{
				CString fn = ssprintf( "/proc/scsi/usb-storage-%d/%d", i, i );
				if( !ReadUsbStorageDescriptor( fn, i, vDevicesOut ) )
					break;
			}
			
			for( unsigned i=0; i<vDevicesOut.size(); i++ )
			{
				UsbStorageDevice& usbd = vDevicesOut[i];
				usbd.sScsiDevice = ssprintf("/dev/sd%c",'a'+(char)usbd.iScsiIndex);
			}
		}
	}
	
	{
		// Find where each device is mounted. Output looks like:
		
		// /dev/sda1               /mnt/flash1             auto    noauto,owner 0 0
		// /dev/sdb1               /mnt/flash2             auto    noauto,owner 0 0
		// /dev/sdc1               /mnt/flash3             auto    noauto,owner 0 0
		
		CString fn = "/etc/fstab";
		LOG->Trace( "Reading %s", fn.c_str() );
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
			
			// search for the usb-storage device corresponding to the SCSI device
			for( unsigned i=0; i<vDevicesOut.size(); i++ )
			{
				UsbStorageDevice& usbd = vDevicesOut[i];
				if( usbd.sScsiDevice+"1" == szScsiDevice )	// found our match
				{
					usbd.sOsMountDir = sMountPoint;
					
					LOG->Trace( "iScsiIndex: %d, sScsiDevice: %s, iBus: %d, iLevel: %d, iPort: %d, sOsMountDir: %s",
						usbd.iScsiIndex, usbd.sScsiDevice.c_str(), usbd.iBus, usbd.iLevel, usbd.iPort, usbd.sOsMountDir.c_str() );
					
					break;	// stop looking for a match
				}
			}
		}
	}
	
leave:
	
	/* Remove any devices that we couldn't find a mountpoint for. */
	for( unsigned i=0; i<vDevicesOut.size(); i++ )
	{
		UsbStorageDevice& usbd = vDevicesOut[i];
		if( usbd.sOsMountDir.empty() )
		{
			LOG->Trace( "Ignoring %s (couldn't find in /etc/fstab)", usbd.sSerial.c_str() );
			
			vDevicesOut.erase( vDevicesOut.begin()+i );
			--i;
		}
	}
	
	LOG->Trace( "Done with GetNewStorageDevices" );
}


void MemoryCardDriverThreaded_Linux::Mount( UsbStorageDevice* pDevice, CString sMountPoint )
{
	ASSERT( !pDevice->sOsMountDir.empty() );
	
	UnmountMountPoint( sMountPoint );
	
	FILEMAN->Mount( "dir", pDevice->sOsMountDir, sMountPoint.c_str() );
	LOG->Trace( "FILEMAN->Mount %s %s", pDevice->sOsMountDir.c_str(), sMountPoint.c_str() );
}

void MemoryCardDriverThreaded_Linux::Unmount( UsbStorageDevice* pDevice, CString sMountPoint )
{
	if( pDevice->sOsMountDir.empty() )
		return;
	
	// already unmounted by the mounting thread
}

void MemoryCardDriverThreaded_Linux::Flush( UsbStorageDevice* pDevice )
{
	if( pDevice->sOsMountDir.empty() )
		return;
	
	// "sync" will only flush all file systems at the same time.  -Chris
	// I don't think so.  Also, sync() merely queues a flush; it doesn't guarantee
	// that the flush is completed on return.  However, we can mount the filesystem
	// with the flag "-o sync", which forces synchronous access (but that's probably
	// very slow.) -glenn
	ExecuteCommand( "mount -o remount " + pDevice->sOsMountDir );
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
