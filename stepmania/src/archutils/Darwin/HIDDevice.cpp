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
	
	result = IORegistryEntryCreateCFProperties( device, &properties, kCFAllocatorDefault, kNilOptions );
	if ( result != KERN_SUCCESS || !properties )
	{
		LOG->Warn( "Couldn't get properties." );
		return false;
	}
	
	CFStringRef productRef = (CFStringRef)CFDictionaryGetValue( properties, CFSTR(kIOHIDProductKey) );
	
	if( productRef )
		m_sDescription = CFStringGetCStringPtr( productRef, CFStringGetSystemEncoding() );
	if( m_sDescription == "" )
	{
		CFTypeRef vidRef = (CFTypeRef)CFDictionaryGetValue( properties, CFSTR(kIOHIDVendorIDKey) );
		CFTypeRef pidRef = (CFTypeRef)CFDictionaryGetValue( properties, CFSTR(kIOHIDProductIDKey) );
		int vid, pid;
		
		if( vidRef && !IntValue(vidRef, vid) )
			vid = 0;
		if( pidRef && !IntValue(pidRef, pid) )
			pid = 0;
		m_sDescription = ssprintf( "%04x:%04x", vid, pid );
	}
	
	CFArrayRef logicalDevices;
	
	if ( !(logicalDevices = (CFArrayRef)CFDictionaryGetValue(properties, CFSTR(kIOHIDElementKey))) )
	{
		CFRelease( properties );
		return false;
	}
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
	
	CFDictionaryRef dict = CFDictionaryRef( value );
	HIDDevice *This = (HIDDevice *)context;
	CFArrayRef elements;
	CFTypeRef object;
	int usage, usagePage;
	CFTypeID numID = CFNumberGetTypeID();
	
	// Get usage page
	object = CFDictionaryGetValue( dict, CFSTR(kIOHIDElementUsagePageKey) );
	if( !object || CFGetTypeID(object) != numID || !IntValue(object, usagePage) )
		return;
	
	// Get usage
	object = CFDictionaryGetValue( dict, CFSTR(kIOHIDElementUsageKey) );
	if( !object || CFGetTypeID(object) != numID || !IntValue(object, usage) )
		return;
	
	if( !(elements = (CFArrayRef)CFDictionaryGetValue( dict, CFSTR(kIOHIDElementKey))) )
		return;
	
	if( !This->AddLogicalDevice(usagePage, usage) )
		return;
	
	CFRange r = { 0, CFArrayGetCount(elements) };
		
	CFArrayApplyFunction( elements, r, HIDDevice::AddElement, This );
}

void HIDDevice::AddElement( const void *value, void *context )
{
	if( CFGetTypeID(CFTypeRef(value)) != CFDictionaryGetTypeID() )
		return;
	
	CFDictionaryRef dict = CFDictionaryRef( value );
	HIDDevice *This = (HIDDevice *)context;
	CFTypeRef object;
	CFTypeID numID = CFNumberGetTypeID();
	int cookie, usage, usagePage;
	CFArrayRef elements;
	
	// Recursively add elements
	if( (elements = (CFArrayRef)CFDictionaryGetValue(dict, CFSTR(kIOHIDElementKey))) )
	{
		CFRange r = { 0, CFArrayGetCount(elements) };
		
		CFArrayApplyFunction( elements, r, AddElement, context );
	}
	
	// Get usage page
	object = CFDictionaryGetValue( dict, CFSTR(kIOHIDElementUsagePageKey) );
	if( !object || CFGetTypeID(object) != numID || !IntValue(object, usagePage) )
		return;
	
	// Get usage
	object = CFDictionaryGetValue( dict, CFSTR(kIOHIDElementUsageKey) );
	if( !object || CFGetTypeID(object) != numID || !IntValue(object, usage) )
		return;
	
	
	// Get cookie
	object = CFDictionaryGetValue( dict, CFSTR(kIOHIDElementCookieKey) );
	if( !object || CFGetTypeID(object) != numID || !IntValue(object, cookie) )
		return;
	This->AddElement( usagePage, usage, cookie, dict );
}

