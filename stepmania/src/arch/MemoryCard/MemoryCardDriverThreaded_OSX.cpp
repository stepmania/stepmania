#include "global.h"
#include "MemoryCardDriverThreaded_OSX.h"
#include "Foreach.h"
#include "RageUtil.h"
#include "RageLog.h"

#include <Carbon/Carbon.h>
#include <IOKit/IOKitLib.h>
#include <IOKit/storage/IOMedia.h>
#include <IOKit/usb/USBSpec.h>
#include <IOKit/usb/IOUSBLib.h>
#include <sys/param.h>
#include <sys/ucred.h>
#include <sys/mount.h>
#include <paths.h>
#include <unistd.h>

OSStatus MemoryCardDriverThreaded_OSX::VolumeChanged( EventHandlerCallRef ref, EventRef event, void *p )
{
	MemoryCardDriverThreaded_OSX *This = (MemoryCardDriverThreaded_OSX *)p;
	LockMut( This->m_ChangedLock );
	
	This->m_bChanged = true;
	return eventNotHandledErr; // let others do something
}

MemoryCardDriverThreaded_OSX::MemoryCardDriverThreaded_OSX() : m_ChangedLock( "MC changed lock" )
{
	m_bChanged = true;
	m_HandlerUPP = NewEventHandlerUPP( VolumeChanged );
	
	EventTypeSpec types[] = { { kEventClassVolume, kEventVolumeMounted },
	{ kEventClassVolume, kEventVolumeUnmounted } };
	UInt32 numTypes = sizeof(types)/sizeof(types[0]);
	OSStatus ret = InstallApplicationEventHandler( m_HandlerUPP, numTypes, types, this, &m_Handler );
	
	ASSERT( ret == noErr );
}

MemoryCardDriverThreaded_OSX::~MemoryCardDriverThreaded_OSX()
{
	RemoveEventHandler( m_Handler );
	DisposeEventHandlerUPP( m_HandlerUPP );
}

void MemoryCardDriverThreaded_OSX::Unmount( UsbStorageDevice *pDevice )
{
	if( pDevice->iRefNum == -1 )
		return;
	
	ParamBlockRec pb;
	
	memset( &pb, 0, sizeof(pb) );
	pb.volumeParam.ioNamePtr = NULL;
	pb.volumeParam.ioVRefNum = pDevice->iRefNum;
	
	if( PBFlushVolSync(&pb) != noErr )
		LOG->Warn( "Failed to flush the memory card." );
}

bool MemoryCardDriverThreaded_OSX::USBStorageDevicesChanged()
{
	LockMut( m_ChangedLock );
	return m_bChanged;
}

static int GetIntProperty( io_registry_entry_t entry, CFStringRef key )
{
	CFTypeRef t = IORegistryEntryCreateCFProperty( entry, key, NULL, 0 );
	
	if( !t )
		return -1;
	if( CFGetTypeID( t ) != CFNumberGetTypeID() )
	{
		CFRelease( t );
		return -1;
	}
	int num;
	
	if( !CFNumberGetValue(CFNumberRef(t), kCFNumberIntType, &num) )
		num = -1;
	CFRelease( t );
	return num;
}

static CString GetStringProperty( io_registry_entry_t entry, CFStringRef key )
{
	CFTypeRef t = IORegistryEntryCreateCFProperty( entry, key, NULL, 0 );
	
	if( !t )
		return CString();
	if( CFGetTypeID( t ) != CFStringGetTypeID() )
	{
		CFRelease( t );
		return CString();
	}
	
	CFStringRef s = CFStringRef( t );
	CString ret;
	const size_t len = CFStringGetMaximumSizeForEncoding( CFStringGetLength(s), kCFStringEncodingUTF8 );
	char *buf = new char[len + 1];
		
	if( CFStringGetCString( s, buf, len + 1, kCFStringEncodingUTF8 ) )
		ret = buf;
	delete[] buf;
	CFRelease( t );
	return ret;
}

void MemoryCardDriverThreaded_OSX::GetUSBStorageDevices( vector<UsbStorageDevice>& vDevicesOut )
{
	LOG->Trace( "GetUSBStorageDevices." );
	LockMut( m_ChangedLock );
	// First, get all device paths
	struct statfs *fs;
	int num = getfsstat( NULL, 0, MNT_NOWAIT );
	
	fs = new struct statfs[num];
	
	num = getfsstat( fs, num * sizeof(struct statfs), MNT_NOWAIT );
	ASSERT( num != -1 );
	
	for( int i = 0; i < num; ++i )
	{
		if( strncmp(fs[i].f_mntfromname, _PATH_DEV, strlen(_PATH_DEV)) )
			continue;
		
		const CString& sDevicePath = fs[i].f_mntfromname;
		const CString& sDisk = Basename( sDevicePath ); // disk#[[s#] ...]
		
		// Now that we have the disk name, look up the IOServices associated with it.
		CFMutableDictionaryRef dict;
		
		if( !(dict = IOBSDNameMatching(kIOMasterPortDefault, 0, sDisk)) )
			continue;
		
		// Look for certain properties: Leaf, Ejectable, Writable.
		CFDictionarySetValue( dict, CFSTR(kIOMediaLeafKey), kCFBooleanTrue );
		CFDictionarySetValue( dict, CFSTR(kIOMediaEjectableKey), kCFBooleanTrue );
		CFDictionarySetValue( dict, CFSTR(kIOMediaWritableKey), kCFBooleanTrue );
		
		// Get the matching iterator. As always, this consumes a reference to dict.
		io_iterator_t iter;
		kern_return_t ret = IOServiceGetMatchingServices( kIOMasterPortDefault, dict, &iter );
		
		if( ret != KERN_SUCCESS || iter == 0 )
			continue;
		
		// I'm not quite sure what it means to have two services with this device.
		// Iterate over them all. If one contains what we want, stop.
		io_registry_entry_t entry; // This is the same as an io_object_t.
		
		while( (entry = IOIteratorNext(iter)) )
		{
			// Get the path in the IOService plane.
			io_string_t path; // Some c string.
			
			ret = IORegistryEntryGetPath( entry, kIOServicePlane, path );
			IOObjectRelease( entry );
			
			if( ret != KERN_SUCCESS )
			{
				// XXX maybe I should just walk back myself.
				LOG->Warn( "Device \"%s\" (%s) has an IORegistry path that is too long.",
						   fs[i].f_mntfromname, fs[i].f_mntonname );
				continue;
			}
			const CString& sRegistryPath = path;
			CString::size_type pos = sRegistryPath.rfind( "/IOUSBMassStorageClass" );
			
			if( pos == CString::npos )
				continue; // Probably not a USB device.
			// The path does not start with / so pos - 1 >= 0.
			pos = sRegistryPath.rfind( '/', pos - 1 );
			if( pos == CString::npos )
				continue; // Something is horribly wrong at this point.
			path[pos] = '\0';
			
			io_registry_entry_t device = IORegistryEntryFromPath( kIOMasterPortDefault, path );
			
			// MACH_PORT_NULL?
			if( device == MACH_PORT_NULL )
			{
				LOG->Warn( "Couldn't create IORegistry entry from: %s", path );
				continue;
			}
			
			// At this point, it is pretty safe to say that we've found a USB device.
			vDevicesOut.push_back( UsbStorageDevice() );
			
			UsbStorageDevice& usbd = vDevicesOut.back();
			
			LOG->Trace( "Found memory card at path: %s.", fs[i].f_mntonname );
			usbd.SetOsMountDir( fs[i].f_mntonname );
			
			// Find volume reference number for flushing.
			XVolumeParam param;
			Str255 name; // A pascal string.
			const CString& base = Basename( fs[i].f_mntonname );
			
			memset( &param, 0, sizeof(param) );
			name[0] = min( base.length(), size_t(255) );
			strncpy( (char *)&name[1], base, name[0] );
			param.ioNamePtr = name;
			param.ioVolIndex = -1; // Use ioNamePtr to find the volume.

			/* At this point, we have 3 methods available to get the volume size.
			 * we can use:
			 * param.ioVTotalBytes,
			 * IORegistryEntryCreateCFProperty( entry, CFSTR(kIOMediaSizeKey), NULL, 0 ),
			 * or fs[i].f_blocks * fs[i].f_bsize, however, we released entry already. */
			// XXX PBXGetVolInfoSync is apparently deprecated.
			if( PBXGetVolInfoSync(&param) == noErr )
			{
				usbd.iRefNum = param.ioVRefNum;
				usbd.iVolumeSizeMB = param.ioVTotalBytes >> 20;
			}
			else
			{
				/* We could fall back on one of the other methods but if we can't
				 * get the volume info then something is wrong so give up. */
				usbd.SetError( "Failed to get volume info." );
				IOObjectRelease( device );
				break;
			}
			
			// Now we can get some more information from the registry tree.
			usbd.iBus = GetIntProperty( device, CFSTR("USB Address") );
			usbd.iPort = GetIntProperty( device, CFSTR("PortNum") );
			// usbd.iLevel ?
			usbd.sSerial = GetStringProperty( device, CFSTR("USB Serial Number") );
			usbd.sDevice = fs[i].f_mntfromname;
			usbd.idVendor = GetIntProperty( device, CFSTR(kUSBVendorID) );
			usbd.idProduct = GetIntProperty( device, CFSTR(kUSBProductID) );
			usbd.sVendor = GetStringProperty( device, CFSTR("USB Vendor Name") );
			usbd.sProduct = GetStringProperty( device, CFSTR("USB Product Name") );
			IOObjectRelease( device );
			break; // We found what we wanted
		}
		IOObjectRelease( iter );
	}
	m_bChanged = false;
	delete[] fs;
}

bool MemoryCardDriverThreaded_OSX::TestWrite( UsbStorageDevice *pDevice )
{
	if( access(pDevice->sOsMountDir, W_OK) )
	{
		pDevice->SetError( "Write test failed." );
		return false;
	}
	return true;
}

/*
 * (c) 2005-2006 Steve Checkoway
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
