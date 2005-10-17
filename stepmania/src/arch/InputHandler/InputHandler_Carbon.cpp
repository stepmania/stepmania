#include "global.h"
#include "RageLog.h"

#include <ext/hash_map>
#include <mach/mach.h>
#include <mach/mach_error.h>
#include <IOKit/IOKitLib.h>
#include <IOKit/IOCFPlugIn.h>
#include <IOKit/hid/IOHIDLib.h>
#include <IOKit/hid/IOHIDUsageTables.h>
#include <IOKit/usb/USB.h>
#include <CoreFoundation/CoreFoundation.h>
#include <Carbon/Carbon.h>

#include "InputHandler_Carbon.h"
#include "ForEach.h"
#include "RageUtil.h"

using namespace std;
using __gnu_cxx::hash_map;

static void AddElement( const void *value, void *context );

// Simple helper, still need to release it
static inline CFNumberRef CFInt( int n )
{
	return CFNumberCreate( kCFAllocatorDefault, kCFNumberIntType, &n );
}

static inline void PrintIOErr( IOReturn err, const char *s )
{
	LOG->Warn( "%s - %s(%x,%d)", s, mach_error_string(err), err, err & 0xFFFFFF );
}

static inline CFTypeRef DictValue( CFDictionaryRef d, const char *k )
{
	return CFDictionaryGetValue( d, k );
}

static inline int IntValue( const void *o, int *n )
{
	return CFNumberGetValue( CFNumberRef(o), kCFNumberIntType, n );
}

/* This is just awful, these aren't objects, treating them as such leads
 * to: (*object)->function(object [, argument]...)
 * Instead, do: CALL(object, function [, argument]...)
 */
#define CALL(o,f,...) (*(o))->f((o), ## __VA_ARGS__)

struct Joystick
{
	InputDevice id;
	// map cookie to button
	hash_map<int, int> mapping;
	int x_axis, y_axis, z_axis;
	int x_min, y_min, z_min;
	int x_max, y_max, z_max;

	Joystick();
};

Joystick::Joystick() : id( DEVICE_NONE ),
					   x_axis( JOYSTICK_BUTTON_INVALID ),
					   y_axis( JOYSTICK_BUTTON_INVALID ),
					   z_axis( JOYSTICK_BUTTON_INVALID ),
					   x_min( 0 ), y_min( 0 ), z_min( 0 ),
					   x_max( 0 ), y_max( 0 ), z_max( 0 )
{
}

class JoystickDevice
{
private:
	vector<Joystick> mSticks;
	IOHIDDeviceInterface **mInterface;
	IOHIDQueueInterface **mQueue;
	bool mRunning;
	CString mDescription;

	static void AddJoystick( const void *value, void *context );

public:
	JoystickDevice();
	~JoystickDevice();

	bool Open( io_object_t device, int num, InputHandler_Carbon *handler );
	// returns the number of IDs assigned starting from startID
	int AssignJoystickIDs( int startID );
	inline int NumberOfSticks() const { return mSticks.size(); }
	inline const Joystick& GetStick( int index ) const { return mSticks[index]; }
	inline const CString& GetDescription() const { return mDescription; }
};

JoystickDevice::JoystickDevice() : mInterface(NULL), mQueue(NULL), mRunning(false)
{
}

bool JoystickDevice::Open( io_object_t device, int num, InputHandler_Carbon *handler )
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

	// XXX Get Device Info
	mDescription = "XXX Device Info";

	CFArrayRef sticks;

	if ( !(sticks = (CFArrayRef)DictValue(properties, kIOHIDElementKey)) )
	{
		CFRelease( properties );
		return false;
	}
	size_t count = CFArrayGetCount( sticks );
	CFRange r = { 0, count };

	mSticks.reserve( count );
	CFArrayApplyFunction( sticks, r, JoystickDevice::AddJoystick, this );

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

	hresult = CALL( plugInInterface, QueryInterface, bytes, (void **)&mInterface );

	CALL( plugInInterface, Release );

	if( hresult != S_OK )
	{
		LOG->Warn( "Couldn't get device interface from plugin interface." );
		mInterface = NULL;
		return false;
	}
		
	// open the interface
	if( (ret = CALL(mInterface, open, 0)) != kIOReturnSuccess )
	{
		PrintIOErr(ret, "Failed to open the interface.");
		CALL(mInterface, Release);
		mInterface = NULL;
		return false;
	}

	// alloc/create queue
	mQueue = CALL( mInterface, allocQueue );
	if( !mQueue )
	{
		LOG->Warn( "Couldn't allocate a queue." );
		return false;
	}

	if( (ret = CALL(mQueue, create, 0, 32)) != kIOReturnSuccess )
	{
		PrintIOErr(ret, "Failed to create the queue.");
		CALL(mQueue, Release);
		mQueue = NULL;
		return false;
	}

	// Add elements to the queue for each Joystick
	FOREACH_CONST( Joystick, mSticks, i )
	{
		const Joystick& js = *i;

		if( js.x_axis )
			CALL( mQueue, addElement, IOHIDElementCookie(js.x_axis), 0 );
		if( js.y_axis )
			CALL( mQueue, addElement, IOHIDElementCookie(js.y_axis), 0 );
		if( js.z_axis )
			CALL( mQueue, addElement, IOHIDElementCookie(js.z_axis), 0 );

		for( hash_map<int, int>::const_iterator j = js.mapping.begin(); j != js.mapping.end(); ++j)
			CALL( mQueue, addElement, IOHIDElementCookie(j->first), 0 );
	}
	// add the callback
	CFRunLoopSourceRef runLoopSource;

	ret = CALL( mQueue, createAsyncEventSource, &runLoopSource );

	if( ret != kIOReturnSuccess )
	{
		PrintIOErr( ret, "Failed to create async event source." );
		return false;
	}

	CFRunLoopRef runLoop;

	runLoop = CFRunLoopRef( GetCFRunLoopFromEventLoop(GetMainEventLoop()) );

	if( !CFRunLoopContainsSource(runLoop, runLoopSource, kCFRunLoopDefaultMode) )
		CFRunLoopAddSource( runLoop, runLoopSource, kCFRunLoopDefaultMode );

	CALL( mQueue, setEventCallout, InputHandler_Carbon::QueueCallBack, handler, (void *)num );

	if( ret != kIOReturnSuccess )
	{
		PrintIOErr( ret, "Failed to set the call back." );
		return false;
	}

	// start the queue
	ret = CALL( mQueue, start );

	if( ret != kIOReturnSuccess )
	{
		PrintIOErr( ret, "Failed to start the queue." );
		return false;
	}
	mRunning = true;
	return true;
}

JoystickDevice::~JoystickDevice()
{
	if( mQueue )
	{
		CFRunLoopSourceRef runLoopSource;

		if( mRunning )
			CALL( mQueue, stop );
		if( (runLoopSource = CALL(mQueue, getAsyncEventSource)) )
		{
			CFRunLoopRef ref;

			ref = CFRunLoopRef( GetCFRunLoopFromEventLoop(GetMainEventLoop()) );

			if( CFRunLoopContainsSource(ref, runLoopSource, kCFRunLoopDefaultMode) )
				CFRunLoopRemoveSource( ref, runLoopSource, kCFRunLoopDefaultMode );
			CFRelease( runLoopSource );
		}

		CALL( mQueue, dispose );
		CALL( mQueue, Release );
	}
	if( mInterface )
	{
		CALL( mInterface, close );
		CALL( mInterface, Release );
	}
}

void JoystickDevice::AddJoystick( const void *value, void *context )
{
	if( CFGetTypeID(CFTypeRef(value)) != CFDictionaryGetTypeID() )
		return;

	CFDictionaryRef dict = CFDictionaryRef( value );
	JoystickDevice *This = (JoystickDevice *)context;
	CFArrayRef elements;
	CFTypeRef object;
	int usage, usagePage;
	CFTypeID numID = CFNumberGetTypeID();

	// Get usage page
	object = DictValue( dict, kIOHIDElementUsagePageKey );
	if( !object || CFGetTypeID(object) != numID || !IntValue(object, &usagePage) )
		return;

	// Get usage
	object = DictValue( dict, kIOHIDElementUsageKey );
	if( !object || CFGetTypeID(object) != numID || !IntValue(object, &usage) )
		return;

	if( usagePage != kHIDPage_GenericDesktop || usage != kHIDUsage_GD_Joystick )
		return;

	if( !(elements = (CFArrayRef)DictValue(dict, kIOHIDElementKey)) )
		return;
	
	Joystick js;
	CFRange r = { 0, CFArrayGetCount(elements) };

	CFArrayApplyFunction( elements, r, AddElement, &js );
	This->mSticks.push_back( js );
}


int JoystickDevice::AssignJoystickIDs( int startID )
{
	for( vector<Joystick>::iterator i = mSticks.begin(); i != mSticks.end(); ++i )
		i->id = InputDevice( startID++ );
	return mSticks.size();
}

// Callback
static void AddElement( const void *value, void *context )
{
	if( CFGetTypeID(CFTypeRef(value)) != CFDictionaryGetTypeID() )
		return;

	CFDictionaryRef dict = CFDictionaryRef( value );
	Joystick& js = *(Joystick *)context;
	CFTypeRef object;
	CFTypeID numID = CFNumberGetTypeID();
	int cookie, usage, usagePage;
	CFArrayRef elements;

	// Recursively add elements
	if( (elements = (CFArrayRef)DictValue(dict, kIOHIDElementKey)) )
	{
		CFRange r = { 0, CFArrayGetCount(elements) };

		CFArrayApplyFunction( elements, r, AddElement, context );
	}

	// Get usage page
	object = DictValue( dict, kIOHIDElementUsagePageKey );
	if( !object || CFGetTypeID(object) != numID || !IntValue(object, &usagePage) )
		return;

	// Get usage
	object = DictValue( dict, kIOHIDElementUsageKey );
	if( !object || CFGetTypeID(object) != numID || !IntValue(object, &usage) )
		return;


	// Get cookie
	object = DictValue( dict, kIOHIDElementCookieKey );
	if( !object || CFGetTypeID(object) != numID || !IntValue(object, &cookie) )
		return;

	switch( usagePage )
	{
	case kHIDPage_GenericDesktop:
	{
		int min = 0;
		int max = 0;

		object = DictValue( dict, kIOHIDElementMinKey );
		if( object && CFGetTypeID(object) == numID )
			IntValue( object, &min );

		object = DictValue( dict, kIOHIDElementMaxKey );
		if( object && CFGetTypeID(object) == numID )
			IntValue( object, &max );

		switch( usage )
		{
		case kHIDUsage_GD_X:
			js.x_axis = cookie;
			js.x_min = min;
			js.x_max = max;
			break;
		case kHIDUsage_GD_Y:
			js.y_axis = cookie;
			js.y_min = min;
			js.y_max = max;
			break;
		case kHIDUsage_GD_Z:
			js.z_axis = cookie;
			js.z_min = min;
			js.z_max = max;
			break;
		case kHIDUsage_GD_DPadUp:
			js.mapping[cookie] = JOY_UP;
			break;
		case kHIDUsage_GD_DPadDown:
			js.mapping[cookie] = JOY_DOWN;
			break;
		case kHIDUsage_GD_DPadRight:
			js.mapping[cookie] = JOY_RIGHT;
			break;
		case kHIDUsage_GD_DPadLeft:
			js.mapping[cookie] = JOY_LEFT;
			break;
		default:
			return;
		}
		break;
	}
	case kHIDPage_Button:
	{
		// button n has usage = n, subtract 1 to ensure 
		// button 1 = JOY_BUTTON_1
		const int buttonID = usage + JOY_BUTTON_1 - 1;

		if( buttonID < NUM_JOYSTICK_BUTTONS )
			js.mapping[cookie] = buttonID;
		break;
	}
	default:
		return;
	} // end switch (usagePage)
}

void InputHandler_Carbon::QueueCallBack( void *target, int result, void *refcon, void *sender )
{
	// The result seems useless as you can't actually return anything...
	// refcon is the JoystickDevice number

	RageTimer now;
	InputHandler_Carbon *This = (InputHandler_Carbon *)target;
	IOHIDQueueInterface **queue = (IOHIDQueueInterface **)sender;
	IOHIDEventStruct event;
	AbsoluteTime zeroTime = { 0, 0 };
	JoystickDevice *jd = This->mDevices[int( refcon )];

	while( (result = CALL(queue, getNextEvent, &event, zeroTime, 0)) == kIOReturnSuccess )
	{
		int cookie = int( event.elementCookie );
		int value = event.value;

		for( int i = 0; i < jd->NumberOfSticks(); ++i )
		{
			const Joystick& js = jd->GetStick( i );

			if( js.x_axis == cookie )
			{
				float level = SCALE( value, js.x_min, js.x_max, -1.0f, 1.0f );
				
				This->ButtonPressed( DeviceInput(js.id, JOY_LEFT, max(-level, 0.0f), now), level < -0.5f );
				This->ButtonPressed( DeviceInput(js.id, JOY_RIGHT, max(level, 0.0f), now), level > 0.5f );
				break;
			}
			else if( js.y_axis == cookie )
			{
				float level = SCALE( value, js.y_min, js.y_max, -1.0f, 1.0f );
				
				This->ButtonPressed( DeviceInput(js.id, JOY_UP, max(-level, 0.0f), now), level < -0.5f );
				This->ButtonPressed( DeviceInput(js.id, JOY_DOWN, max(level, 0.0f), now), level > 0.5f );
				break;
			}
			else if( js.z_axis == cookie )
			{
				float level = SCALE( value, js.z_min, js.z_max, -1.0f, 1.0f );
				
				This->ButtonPressed( DeviceInput(js.id, JOY_Z_UP, max(-level, 0.0f), now), level < -0.5f );
				This->ButtonPressed( DeviceInput(js.id, JOY_Z_DOWN, max(level, 0.0f), now), level > 0.5f );
				break;
			}
			else
			{
				// hash_map<T,U>::operator[] is not const
				hash_map<int, int>::const_iterator iter;

				iter = js.mapping.find( cookie );
				if( iter != js.mapping.end() )
				{
					This->ButtonPressed( DeviceInput(js.id, iter->second, value, now), value );
					break;
				}
			}
		}
	}
}

OSStatus InputHandler_Carbon::EventHandler( EventHandlerCallRef callRef, EventRef event, void *data )
{
	InputHandler_Carbon *This = (InputHandler_Carbon *)data;
	UInt32 kind = GetEventKind(event);
	UInt32 keyCode;
	char charCode;

	GetEventParameter( event, kEventParamKeyCode, typeUInt32, NULL, sizeof(keyCode), NULL, &keyCode );
	GetEventParameter( event, kEventParamKeyMacCharCodes, typeChar, NULL, sizeof(charCode), NULL, &charCode );

	const char *type;

	switch( kind )
	{
	case kEventRawKeyDown:
		type = "down";
		break;
	case kEventRawKeyRepeat:
		type = "repeat";
		break;
	case kEventRawKeyUp:
		type = "up";
	default:
		type = "unknown";
	}

	LOG->Trace( "(0) %u %c: %s\n", unsigned(keyCode), charCode, type );
	return 0;
}

InputHandler_Carbon::~InputHandler_Carbon()
{
	FOREACH( JoystickDevice *, mDevices, i )
		delete *i;
	if( mMasterPort )
		mach_port_deallocate( mach_task_self(), mMasterPort );
	RemoveEventHandler( mEventHandlerRef );
	DisposeEventHandlerUPP( mEventHandlerUPP );
}

InputHandler_Carbon::InputHandler_Carbon()
{
	// Install a Carbon Event handler
	mEventHandlerUPP = NewEventHandlerUPP( EventHandler );
	EventTypeSpec typeList[] = {
		{ kEventClassKeyboard, kEventRawKeyDown   },
		{ kEventClassKeyboard, kEventRawKeyRepeat },
		{ kEventClassKeyboard, kEventRawKeyUp     } };


	if( InstallEventHandler(GetApplicationEventTarget(), mEventHandlerUPP, 3, typeList, this, &mEventHandlerRef) )
		LOG->Warn("Failed to install the Event Handler.");

	// Get a Mach port to initiate communication with I/O Kit.
	mach_port_t masterPort;

	IOReturn ret = IOMasterPort( MACH_PORT_NULL, &masterPort );

	if( ret != kIOReturnSuccess )
	{
		PrintIOErr( ret, "Couldn't create a master I/O Kit port." );
		return;
	}
	mMasterPort = masterPort;

	// Build the matching dictionary.
	CFMutableDictionaryRef dict;

	if( (dict = IOServiceMatching(kIOHIDDeviceKey)) == NULL )
	{
		LOG->Warn( "Couldn't create a matching dictionary." );
		mach_port_deallocate( mach_task_self(), masterPort );
		return;
	}
	// Refine the search by only looking for joysticks
	CFNumberRef usagePage = CFInt( kHIDPage_GenericDesktop );
	CFNumberRef usage = CFInt( kHIDUsage_GD_Joystick );
			
	CFDictionarySetValue( dict, CFSTR(kIOHIDPrimaryUsagePageKey), usagePage );
	CFDictionarySetValue( dict, CFSTR(kIOHIDPrimaryUsageKey), usage);
	
	// Cleanup after ourselves
	CFRelease(usagePage);
	CFRelease(usage);

	// Find the HID devices.
	io_iterator_t iter;

	/* Get an iterator to the matching devies
	 * This consumes a reference to the dictionary so we don't
	 * have to Release() later.
	 */
	ret = IOServiceGetMatchingServices( masterPort, dict, &iter );
	if( (ret != kIOReturnSuccess) || !iter )
	{
		mach_port_deallocate( mach_task_self(), masterPort );
		masterPort = 0;
		return;
	}

	// Iterate over the devices and add them
	io_object_t device;
	int id = DEVICE_JOY1;
	
	while( (device = IOIteratorNext(iter)) )
	{
		JoystickDevice *jd = new JoystickDevice;
		
		if( jd->Open(device, mDevices.size(), this) )
		{
			id += jd->AssignJoystickIDs( id );
			mDevices.push_back( jd );
			puts( "Added device." );
		}
		else
		{
			delete jd;
		}
		
		IOObjectRelease( device );

	}
	IOObjectRelease( iter );
}

void InputHandler_Carbon::GetDevicesAndDescriptions( vector<InputDevice>& dev, vector<CString>& desc )
{
	FOREACH_CONST( JoystickDevice *, mDevices, i )
	{
		const JoystickDevice *jd = *i;
		
		for( int j = 0; j < jd->NumberOfSticks(); ++j )
		{
			dev.push_back( jd->GetStick(j).id );
			desc.push_back( jd->GetDescription() );
		}
	}
}

/*
 * (c) 2005 Steve Checkoway
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
