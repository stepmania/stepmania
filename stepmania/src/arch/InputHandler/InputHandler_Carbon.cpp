#include "global.h"
#include "RageLog.h"

#include <Carbon/Carbon.h>

#include "InputHandler_Carbon.h"
#include "Foreach.h"
#include "RageUtil.h"
#include "PrefsManager.h"
#include "archutils/Darwin/DarwinThreadHelpers.h"
#include "archutils/Darwin/KeyboardDevice.h"
#include "archutils/Darwin/JoystickDevice.h"
#include <IOKit/IOMessage.h>

void InputHandler_Carbon::QueueCallBack( void *target, int result, void *refcon, void *sender )
{
	// The result seems useless as you can't actually return anything...
	// refcon is the Device number

	RageTimer now;
	InputHandler_Carbon *This = (InputHandler_Carbon *)target;
	IOHIDQueueInterface **queue = (IOHIDQueueInterface **)sender;
	IOHIDEventStruct event;
	AbsoluteTime zeroTime = { 0, 0 };
	HIDDevice *dev = This->m_vDevices[int( refcon )];
	vector<pair<DeviceInput, bool> > vPresses;
	
	while( (result = CALL(queue, getNextEvent, &event, zeroTime, 0)) == kIOReturnSuccess )
		dev->GetButtonPresses( vPresses, int(event.elementCookie), int(event.value), now );
	for( vector<pair<DeviceInput, bool> >::const_iterator i = vPresses.begin(); i != vPresses.end(); ++i )
		This->ButtonPressed( i->first, i->second );
}

static void RunLoopStarted( CFRunLoopObserverRef o, CFRunLoopActivity a, void *sem )
{
	CFRunLoopObserverInvalidate( o );
	CFRelease( o ); // we don't need this any longer
	((RageSemaphore *)sem)->Post();
}

int InputHandler_Carbon::Run( void *data )
{
	InputHandler_Carbon *This = (InputHandler_Carbon *)data;
	
	This->m_LoopRef = CFRunLoopGetCurrent();
	CFRetain( This->m_LoopRef );
	
	This->StartDevices();
	SetThreadPrecedence( 1.0f );

	// Add an observer for the start of the run loop
	{
		/* The function copies the information out of the structure, so the memory pointed
		 * to by context does not need to persist beyond the function call. */
		CFRunLoopObserverContext context = { 0, &This->m_Sem, NULL, NULL, NULL };
		CFRunLoopObserverRef o = CFRunLoopObserverCreate( kCFAllocatorDefault, kCFRunLoopEntry,
								  false, 0, RunLoopStarted, &context);
		CFRunLoopAddObserver( This->m_LoopRef, o, kCFRunLoopDefaultMode );
	}
	
	/* Add a source for ending the run loop. This serves two purposes:
	 * 1. it provides a way to terminate the run loop when IH_Carbon exists, and
	 * 2. it ensures that CFRunLoopRun() doesn't return immediately if there are no other sources. */
	{
		/* Being a little tricky here, the perform callback takes a void* and returns nothing.
		 * CFRunLoopStop takes a CFRunLoopRef (a pointer) so cast the function and pass the loop ref. */
		void *info = This->m_LoopRef;
		void (*perform)(void *) = (void (*)(void *))CFRunLoopStop;
		// { version, info, retain, release, copyDescription, equal, hash, schedule, cancel, perform }
		CFRunLoopSourceContext context = { 0, info, NULL, NULL, NULL, NULL, NULL, NULL, NULL, perform };
		
		// Pass 1 so that it is called after all inputs have been handled (they will have order = 0)
		This->m_SourceRef = CFRunLoopSourceCreate( kCFAllocatorDefault, 1, &context );
		
		CFRunLoopAddSource( This->m_LoopRef, This->m_SourceRef, kCFRunLoopDefaultMode );
	}
	CFRunLoopRun();
	LOG->Trace( "Shutting down input handler thread..." );
	return 0;
}

void InputHandler_Carbon::DeviceAdded( void *refCon, io_iterator_t )
{
	InputHandler_Carbon *This = (InputHandler_Carbon *)refCon;
	
	LockMut( This->m_ChangeLock );
	This->m_bChanged = true;
}

void InputHandler_Carbon::DeviceChanged( void *refCon, io_service_t service, natural_t messageType, void *arg )
{
	if( messageType == kIOMessageServiceIsTerminated )
	{
		InputHandler_Carbon *This = (InputHandler_Carbon *)refCon;
		
		LockMut( This->m_ChangeLock );
		This->m_bChanged = true;
	}
}

// m_LoopRef needs to be set before this is called
void InputHandler_Carbon::StartDevices()
{
	int n = 0;
	
	ASSERT( m_LoopRef );
	FOREACH( HIDDevice *, m_vDevices, i )
		(*i)->StartQueue( m_LoopRef, InputHandler_Carbon::QueueCallBack, this, n++ );
	
	CFRunLoopSourceRef runLoopSource = IONotificationPortGetRunLoopSource( m_NotifyPort );
	
	CFRunLoopAddSource( m_LoopRef, runLoopSource, kCFRunLoopDefaultMode );
}

InputHandler_Carbon::~InputHandler_Carbon()
{
	FOREACH( HIDDevice *, m_vDevices, i )
		delete *i;
	if( PREFSMAN->m_bThreadedInput )
	{
		CFRunLoopSourceSignal( m_SourceRef );
		CFRunLoopWakeUp( m_LoopRef );
		m_InputThread.Wait();
		CFRelease( m_SourceRef );
		// Don't release the loop ref.
		LOG->Trace( "Input handler thread shut down." );
	}
	
	FOREACH( io_iterator_t, m_vIters, i )
		IOObjectRelease( *i );
	IONotificationPortDestroy( m_NotifyPort );
}

static CFDictionaryRef GetMatchingDictionary( int usagePage, int usage )
{
	// Build the matching dictionary.
	CFMutableDictionaryRef dict;
	
	if( (dict = IOServiceMatching(kIOHIDDeviceKey)) == NULL )
		FAIL_M( "Couldn't create a matching dictionary." );
	// Refine the search by only looking for joysticks
	CFNumberRef usagePageRef = CFInt( usagePage );
	CFNumberRef usageRef = CFInt( usage );
	
	CFDictionarySetValue( dict, CFSTR(kIOHIDPrimaryUsagePageKey), usagePageRef );
	CFDictionarySetValue( dict, CFSTR(kIOHIDPrimaryUsageKey), usageRef );
	
	// Cleanup after ourselves
	CFRelease( usagePageRef );
	CFRelease( usageRef );
	
	return dict;
}

InputHandler_Carbon::InputHandler_Carbon() : m_Sem( "Input thread started" ), m_ChangeLock( "Input handler change lock" )
{
	// Set up the notify ports
	m_NotifyPort = IONotificationPortCreate( kIOMasterPortDefault );
	
	// Find the keyboards.
	io_iterator_t iter;
	CFDictionaryRef dict = GetMatchingDictionary( kHIDPage_GenericDesktop, kHIDUsage_GD_Keyboard );
	kern_return_t ret = IOServiceAddMatchingNotification( m_NotifyPort, kIOFirstMatchNotification, dict,
							      InputHandler_Carbon::DeviceAdded, this, &iter );
	
	if( ret == KERN_SUCCESS )
	{
		m_vIters.push_back( iter );
		// Iterate over the keyboards and add them.
		io_object_t device;
		
		while( (device = IOIteratorNext(iter)) )
		{
			HIDDevice *kd = new KeyboardDevice;
			
			if( !kd->Open(device) )
			{
				delete kd;
				IOObjectRelease( device );
				continue;
			}
			
			m_vDevices.push_back( kd );
			
			io_iterator_t i;
			ret = IOServiceAddInterestNotification( m_NotifyPort, device, kIOGeneralInterest,
								InputHandler_Carbon::DeviceChanged, this, &i );
			
			if( ret == KERN_SUCCESS )
				m_vIters.push_back( i );
			
			IOObjectRelease( device );
		}
	}
	
	// Find the joysticks.
	dict = GetMatchingDictionary( kHIDPage_GenericDesktop, kHIDUsage_GD_Joystick );
	ret = IOServiceAddMatchingNotification( m_NotifyPort, kIOFirstMatchNotification, dict,
						InputHandler_Carbon::DeviceAdded, this, &iter );
	
	if( ret == KERN_SUCCESS )
	{
		m_vIters.push_back( iter );
		// Iterate over the joysticks and add them.
		io_object_t device;
		InputDevice id = DEVICE_JOY1;
		
		while( (device = IOIteratorNext(iter)) )
		{
			HIDDevice *jd = new JoystickDevice;
			
			if( !jd->Open(device) )
			{
				delete jd;
				IOObjectRelease( device );
				continue;
			}
	
			int num = jd->AssignIDs( id );
			
			enum_add( id, num );
			m_vDevices.push_back( jd );
			
			io_iterator_t i;
			ret = IOServiceAddInterestNotification( m_NotifyPort, device, kIOGeneralInterest,
								InputHandler_Carbon::DeviceChanged, this, &i );

			if( ret == KERN_SUCCESS )
				m_vIters.push_back( i );
			IOObjectRelease( device );
		}
	}
	
	m_bChanged = false;
	if( PREFSMAN->m_bThreadedInput )
	{
		m_InputThread.SetName( "Input thread" );
		m_InputThread.Create( InputHandler_Carbon::Run, this );
		// Wait for the run loop to start before returning.
		m_Sem.Wait();
	}
	else
	{
		m_LoopRef = CFRunLoopRef( GetCFRunLoopFromEventLoop(GetMainEventLoop()) );
		StartDevices();
	}
}

void InputHandler_Carbon::GetDevicesAndDescriptions( vector<InputDevice>& dev, vector<RString>& desc )
{
	FOREACH_CONST( HIDDevice *, m_vDevices, i )
		(*i)->GetDevicesAndDescriptions( dev, desc );
}

/*
 * (c) 2005, 2006 Steve Checkoway
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
