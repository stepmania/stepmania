#include "global.h"
#include "MemoryCardDriverThreaded_MacOSX.h"
#include "RageUtil.h"
#include "RageLog.h"

#include <cstddef>
#include <cstdint>
#include <vector>

#include <Carbon/Carbon.h>
#include <IOKit/IOKitLib.h>
#include <IOKit/storage/IOMedia.h>
#include <IOKit/usb/USBSpec.h>
#include <IOKit/usb/IOUSBLib.h>
#if defined(HAVE_SYS_PARAM_H)
#include <sys/param.h>
#endif
#include <sys/ucred.h>
#include <sys/mount.h>
#include <paths.h>
#if defined(HAVE_UNISTD_H)
#include <unistd.h>
#endif

class MemoryCardDriverThreaded_MacOSX::Helper
{
public:
	Helper( MemoryCardDriverThreaded_MacOSX *driver )
	{
		m_HandlerUPP = NewEventHandlerUPP( VolumesChanged );
		EventTypeSpec types[] = { { kEventClassVolume, kEventVolumeMounted },
					  { kEventClassVolume, kEventVolumeUnmounted } };
		UInt32 numTypes = sizeof(types)/sizeof(types[0]);
		OSStatus ret = InstallApplicationEventHandler( m_HandlerUPP, numTypes, types, driver, &m_Handler );
		ASSERT( ret == noErr );
	}

	~Helper()
	{
		RemoveEventHandler( m_Handler );
		DisposeEventHandlerUPP( m_HandlerUPP );
	}

private:
	static OSStatus VolumesChanged( EventHandlerCallRef ref, EventRef event, void *p )
	{
		MemoryCardDriverThreaded_MacOSX *driver = (MemoryCardDriverThreaded_MacOSX *)p;
		LockMut( driver->m_ChangedLock );
		driver->m_bChanged = true;
		return eventNotHandledErr; // let others do something
	}

	EventHandlerUPP m_HandlerUPP;
	EventHandlerRef m_Handler;
};

MemoryCardDriverThreaded_MacOSX::MemoryCardDriverThreaded_MacOSX() : m_ChangedLock( "MC changed lock" )
{
	m_bChanged = true;
	m_pHelper = new Helper( this );
}

MemoryCardDriverThreaded_MacOSX::~MemoryCardDriverThreaded_MacOSX()
{
	delete m_pHelper;
}

void MemoryCardDriverThreaded_MacOSX::Unmount( UsbStorageDevice *pDevice )
{
#if defined(SYNC_VOLUME_FULLSYNC) && defined(SYNC_VOLUME_WAIT)

	if( sync_volume_np( pDevice->sOsMountDir.c_str(), SYNC_VOLUME_FULLSYNC | SYNC_VOLUME_WAIT ) != 0 )
		LOG->Warn( "Failed to flush the memory card." );
#else
	ParamBlockRec pb;
	Str255 name; // A pascal string.
	const RString& base = Basename( pDevice->sOsMountDir );

	memset( &pb, 0, sizeof(pb) );
	name[0] = std::min( base.length(), std::size_t(255) );
	strncpy( (char *)&name[1], base, name[0] );
	pb.volumeParam.ioNamePtr = name;
	pb.volumeParam.ioVolIndex = -1; // Use ioNamePtr to find the volume.

	if( PBFlushVolSync(&pb) != noErr )
		LOG->Warn( "Failed to flush the memory card." );

#endif
}

bool MemoryCardDriverThreaded_MacOSX::USBStorageDevicesChanged()
{
	LockMut( m_ChangedLock );
	return m_bChanged;
}

static int GetIntProperty( io_registry_entry_t entry, CFStringRef key )
{
	CFTypeRef t = IORegistryEntryCreateCFProperty( entry, key, nullptr, 0 );

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

static RString GetStringProperty( io_registry_entry_t entry, CFStringRef key )
{
	CFTypeRef t = IORegistryEntryCreateCFProperty( entry, key, nullptr, 0 );

	if( !t )
		return RString();
	if( CFGetTypeID( t ) != CFStringGetTypeID() )
	{
		CFRelease( t );
		return RString();
	}

	CFStringRef s = CFStringRef( t );
	RString ret;
	const std::size_t len = CFStringGetMaximumSizeForEncoding( CFStringGetLength(s), kCFStringEncodingUTF8 );
	char *buf = new char[len + 1];

	if( CFStringGetCString( s, buf, len + 1, kCFStringEncodingUTF8 ) )
		ret = buf;
	delete[] buf;
	CFRelease( t );
	return ret;
}

void MemoryCardDriverThreaded_MacOSX::GetUSBStorageDevices( std::vector<UsbStorageDevice>& vDevicesOut )
{
	LockMut( m_ChangedLock );
	// First, get all device paths
	struct statfs *fs;
	int num = getfsstat( nullptr, 0, MNT_NOWAIT );

	fs = new struct statfs[num];

	num = getfsstat( fs, num * sizeof(struct statfs), MNT_NOWAIT );
	ASSERT( num != -1 );

	for( int i = 0; i < num; ++i )
	{
		if( strncmp(fs[i].f_mntfromname, _PATH_DEV, strlen(_PATH_DEV)) )
			continue;

		const RString& sDevicePath = fs[i].f_mntfromname;
		const RString& sDisk = Basename( sDevicePath ); // disk#[[s#] ...]

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
		io_registry_entry_t device; // This is the same as an io_object_t.

		while( (device = IOIteratorNext(iter)) )
		{
			// Look at the parent of the device until we see an IOUSBMassStorageClass
			while( device != MACH_PORT_NULL && !IOObjectConformsTo(device, "IOUSBMassStorageClass") )
			{
				io_registry_entry_t entry;
				ret = IORegistryEntryGetParentEntry( device, kIOServicePlane, &entry );
				IOObjectRelease( device );
				device = ret == KERN_SUCCESS? entry:MACH_PORT_NULL;
			}
			// Now look for the corresponding IOUSBDevice, it's likely 2 up the tree
			while( device != MACH_PORT_NULL && !IOObjectConformsTo(device, "IOUSBDevice") )
			{
				io_registry_entry_t entry;
				ret = IORegistryEntryGetParentEntry( device, kIOServicePlane, &entry );
				IOObjectRelease( device );
				device = ret == KERN_SUCCESS? entry:MACH_PORT_NULL;
			}
			if( device == MACH_PORT_NULL )
				continue;

			// At this point, it is pretty safe to say that we've found a USB device.
			vDevicesOut.push_back( UsbStorageDevice() );
			UsbStorageDevice& usbd = vDevicesOut.back();

			LOG->Trace( "Found memory card at path: %s.", fs[i].f_mntonname );
			usbd.SetOsMountDir( fs[i].f_mntonname );
			usbd.iVolumeSizeMB = int( (std::uint64_t(fs[i].f_blocks) * fs[i].f_bsize) >> 20 );

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

bool MemoryCardDriverThreaded_MacOSX::TestWrite( UsbStorageDevice *pDevice )
{
	if( access(pDevice->sOsMountDir, W_OK) )
	{
		pDevice->SetError( "TestFailed" );
		return false;
	}
	return true;
}

/*
 * (c) 2005-2006, 2008 Steve Checkoway
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
