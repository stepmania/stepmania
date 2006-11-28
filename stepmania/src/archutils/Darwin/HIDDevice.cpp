#include "global.h"
#include "HIDDevice.h"
#include "RageUtil.h"

HIDDevice::HIDDevice() : m_Interface( NULL ), m_Queue( NULL ), m_bRunning( false )
{
}

HIDDevice::~HIDDevice()
{
	if( m_Queue )
	{
		CFRunLoopSourceRef runLoopSource;
		
		if( m_bRunning )
		{
			CALL( m_Queue, stop );
			runLoopSource = CALL( m_Queue, getAsyncEventSource );
			mach_port_deallocate( mach_task_self(), CALL(m_Queue, getAsyncPort) );
			CFRunLoopSourceInvalidate( runLoopSource );
			CFRelease( runLoopSource );
		}
		
		CALL( m_Queue, dispose );
		CALL( m_Queue, Release );
	}
	if( m_Interface )
	{
		CALL( m_Interface, close );
		CALL( m_Interface, Release );
	}
}

bool HIDDevice::Open( io_object_t device )
{
	IOReturn ret;
	CFMutableDictionaryRef properties;
	kern_return_t result;
	CFTypeRef object;
	
	result = IORegistryEntryCreateCFProperties( device, &properties, kCFAllocatorDefault, kNilOptions );
	if( result != KERN_SUCCESS || !properties )
	{
		LOG->Warn( "Couldn't get properties." );
		return false;
	}
	
	object = CFDictionaryGetValue( properties, CFSTR(kIOHIDProductKey) );
	
	CFTypeRef vidRef = CFDictionaryGetValue( properties, CFSTR(kIOHIDVendorIDKey) );
	CFTypeRef pidRef = CFDictionaryGetValue( properties, CFSTR(kIOHIDProductIDKey) );
	int vid, pid;
	
	if( !IntValue(vidRef, vid) )
		vid = 0;
	if( !IntValue(pidRef, pid) )
		pid = 0;
	InitDevice( vid, pid );
	
	if( object && CFGetTypeID(object) == CFStringGetTypeID() )
		m_sDescription = CFStringGetCStringPtr( CFStringRef(object), CFStringGetSystemEncoding() );
	if( m_sDescription == "" )
		m_sDescription = ssprintf( "%04x:%04x", vid, pid );
	
	object = CFDictionaryGetValue( properties, CFSTR(kIOHIDElementKey) );
	if ( !object || CFGetTypeID(object) != CFArrayGetTypeID() )
	{
		CFRelease( properties );
		return false;
	}
	
	CFArrayRef logicalDevices = CFArrayRef( object );
	CFRange r = { 0, CFArrayGetCount(logicalDevices) };
	
	CFArrayApplyFunction( logicalDevices, r, HIDDevice::AddLogicalDevice, this );
	
	CFRelease( properties );
	
	// Create the interface
	IOCFPlugInInterface **plugInInterface;
	HRESULT hresult;
	SInt32 score;
	
	ret = IOCreatePlugInInterfaceForService( device, kIOHIDDeviceUserClientTypeID,
						 kIOCFPlugInInterfaceID, &plugInInterface, &score );
	if( ret != kIOReturnSuccess )
	{
		PrintIOErr( ret, "Failed to create plugin interface." );
		return false;
	}
	
	// Call a method of the plugin to create the device interface
	CFUUIDBytes bytes = CFUUIDGetUUIDBytes( kIOHIDDeviceInterfaceID );
	
	hresult = CALL( plugInInterface, QueryInterface, bytes, (void **)&m_Interface );
	
	CALL( plugInInterface, Release );
	
	if( hresult != S_OK )
	{
		LOG->Warn( "Couldn't get device interface from plugin interface." );
		m_Interface = NULL;
		return false;
	}
	
	// open the interface
	if( (ret = CALL(m_Interface, open, 0)) != kIOReturnSuccess )
	{
		PrintIOErr( ret, "Failed to open the interface." );
		CALL( m_Interface, Release );
		m_Interface = NULL;
		return false;
	}
	
	// alloc/create queue
	m_Queue = CALL( m_Interface, allocQueue );
	if( !m_Queue )
	{
		LOG->Warn( "Couldn't allocate a queue." );
		return false;
	}
	
	if( (ret = CALL(m_Queue, create, 0, 32)) != kIOReturnSuccess )
	{
		PrintIOErr( ret, "Failed to create the queue." );
		CALL( m_Queue, Release );
		m_Queue = NULL;
		CALL( m_Interface, Release );
		m_Interface = NULL;
		return false;
	}
	
	Open();
	return true;
}

void HIDDevice::StartQueue( CFRunLoopRef loopRef, IOHIDCallbackFunction callback, void *target, int refCon )
{
	CFRunLoopSourceRef runLoopSource;
	// This creates a run loop source and a mach port. They are released in the dtor.
	IOReturn ret = CALL( m_Queue, createAsyncEventSource, &runLoopSource );
	
	if( ret != kIOReturnSuccess )
	{
		PrintIOErr( ret, "Failed to create async event source." );
		return;
	}
	
	if( !CFRunLoopContainsSource(loopRef, runLoopSource, kCFRunLoopDefaultMode) )
		CFRunLoopAddSource( loopRef, runLoopSource, kCFRunLoopDefaultMode );
	
	ret = CALL( m_Queue, setEventCallout, callback, target, (void *)refCon );
	
	if( ret != kIOReturnSuccess )
	{
		PrintIOErr( ret, "Failed to set the call back." );
		return;
	}
	
	// start the queue
	ret = CALL( m_Queue, start );
	
	if( ret != kIOReturnSuccess )
	{
		mach_port_deallocate( mach_task_self(), CALL(m_Queue, getAsyncPort) );
		CFRunLoopSourceInvalidate( runLoopSource );
		CFRelease( runLoopSource );
		
		PrintIOErr( ret, "Failed to start the queue." );
		return;
	}
	m_bRunning = true;
}

void HIDDevice::AddLogicalDevice( const void *value, void *context )
{
	if( CFGetTypeID(CFTypeRef(value)) != CFDictionaryGetTypeID() )
		return;
	
	CFDictionaryRef properties = CFDictionaryRef( value );
	HIDDevice *This = (HIDDevice *)context;
	CFTypeRef object;
	int usage, usagePage;
	
	// Get usage page
	object = CFDictionaryGetValue( properties, CFSTR(kIOHIDElementUsagePageKey) );
	if( !IntValue(object, usagePage) )
		return;
	
	// Get usage
	object = CFDictionaryGetValue( properties, CFSTR(kIOHIDElementUsageKey) );
	if( !IntValue(object, usage) )
		return;
	
	object = CFDictionaryGetValue( properties, CFSTR(kIOHIDElementKey) );
	if( !object || CFGetTypeID(object) != CFArrayGetTypeID() )
		return;
	
	if( !This->AddLogicalDevice(usagePage, usage) )
		return;
	
	CFArrayRef elements = CFArrayRef( object );
	CFRange r = { 0, CFArrayGetCount(elements) };
		
	CFArrayApplyFunction( elements, r, HIDDevice::AddElement, This );
}

void HIDDevice::AddElement( const void *value, void *context )
{
	if( CFGetTypeID(CFTypeRef(value)) != CFDictionaryGetTypeID() )
		return;
	
	CFDictionaryRef properties = CFDictionaryRef( value );
	HIDDevice *This = (HIDDevice *)context;
	CFTypeRef object;
	int cookie, usage, usagePage;
	
	// Recursively add elements
	object = CFDictionaryGetValue( properties, CFSTR(kIOHIDElementKey) );
	if( object && CFGetTypeID(object) == CFArrayGetTypeID() )
	{
		CFArrayRef elements = CFArrayRef( object );
		CFRange r = { 0, CFArrayGetCount(elements) };
		
		CFArrayApplyFunction( elements, r, AddElement, context );
	}
	
	// Get usage page
	object = CFDictionaryGetValue( properties, CFSTR(kIOHIDElementUsagePageKey) );
	if( !IntValue(object, usagePage) )
		return;
	
	// Get usage
	object = CFDictionaryGetValue( properties, CFSTR(kIOHIDElementUsageKey) );
	if( !IntValue(object, usage) )
		return;
	
	
	// Get cookie
	object = CFDictionaryGetValue( properties, CFSTR(kIOHIDElementCookieKey) );
	if( !IntValue(object, cookie) )
		return;
	This->AddElement( usagePage, usage, cookie, properties );
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
