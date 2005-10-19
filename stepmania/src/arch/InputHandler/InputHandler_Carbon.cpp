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

// Simple helper, still need to release it
static inline CFNumberRef CFInt( int n )
{
	return CFNumberCreate( kCFAllocatorDefault, kCFNumberIntType, &n );
}

static inline void PrintIOErr( IOReturn err, const char *s )
{
	LOG->Warn( "%s - %s(%x,%d)", s, mach_error_string(err), err, err & 0xFFFFFF );
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

class Device
{
private:
	IOHIDDeviceInterface **mInterface;
	IOHIDQueueInterface **mQueue;
	bool mRunning;
	CString mDescription;
	
	static void AddLogicalDevice( const void *value, void *context );
	static void AddElement( const void *value, void *context );

protected:
	virtual bool AddLogicalDevice( int usagePage, int usage ) = 0;
	virtual void AddElement( int usagePage, int usage, int cookie, const CFDictionaryRef dict ) = 0;
	virtual void Open() = 0;
	
	inline void AddElementToQueue( int cookie )
	{ CALL( mQueue, addElement, IOHIDElementCookie(cookie), 0 ); }
public:
	Device();
	virtual ~Device();
	
	bool Open( io_object_t device );
	void StartQueue( CFRunLoopRef loopRef, IOHIDCallbackFunction callback, void *target, int refCon );
	inline const CString& GetDescription() const { return mDescription; }
};

Device::Device() : mInterface( NULL ), mQueue( NULL ), mRunning( false )
{
}

Device::~Device()
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

bool Device::Open( io_object_t device )
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
	
	CFArrayRef logicalDevices;
	
	if ( !(logicalDevices = (CFArrayRef)CFDictionaryGetValue(properties, CFSTR(kIOHIDElementKey))) )
	{
		CFRelease( properties );
		return false;
	}
	CFRange r = { 0, CFArrayGetCount(logicalDevices) };
	
	CFArrayApplyFunction( logicalDevices, r, Device::AddLogicalDevice, this );
	
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
	
	Open();
	return true;
}

void Device::StartQueue( CFRunLoopRef loopRef, IOHIDCallbackFunction callback, void *target, int refCon )
{
	CFRunLoopSourceRef runLoopSource;
	IOReturn ret = CALL( mQueue, createAsyncEventSource, &runLoopSource );
	
	if( ret != kIOReturnSuccess )
	{
		PrintIOErr( ret, "Failed to create async event source." );
		return;
	}
	
	CFRunLoopRef runLoop;
	
	runLoop = CFRunLoopRef( GetCFRunLoopFromEventLoop(GetMainEventLoop()) );
	
	if( !CFRunLoopContainsSource(runLoop, runLoopSource, kCFRunLoopDefaultMode) )
		CFRunLoopAddSource( runLoop, runLoopSource, kCFRunLoopDefaultMode );
	
	CALL( mQueue, setEventCallout, callback, target, (void *)refCon );
	
	if( ret != kIOReturnSuccess )
	{
		PrintIOErr( ret, "Failed to set the call back." );
		return;
	}
	
	// start the queue
	ret = CALL( mQueue, start );
	
	if( ret != kIOReturnSuccess )
	{
		PrintIOErr( ret, "Failed to start the queue." );
		return;
	}
	mRunning = true;
}

void Device::AddLogicalDevice( const void *value, void *context )
{
	if( CFGetTypeID(CFTypeRef(value)) != CFDictionaryGetTypeID() )
		return;
	
	CFDictionaryRef dict = CFDictionaryRef( value );
	Device *This = (Device *)context;
	CFArrayRef elements;
	CFTypeRef object;
	int usage, usagePage;
	CFTypeID numID = CFNumberGetTypeID();
	
	// Get usage page
	object = CFDictionaryGetValue( dict, CFSTR(kIOHIDElementUsagePageKey) );
	if( !object || CFGetTypeID(object) != numID || !IntValue(object, &usagePage) )
		return;
	
	// Get usage
	object = CFDictionaryGetValue( dict, CFSTR(kIOHIDElementUsageKey) );
	if( !object || CFGetTypeID(object) != numID || !IntValue(object, &usage) )
		return;
	
	if( !(elements = (CFArrayRef)CFDictionaryGetValue( dict, CFSTR(kIOHIDElementKey))) )
		return;
	
	if( !This->AddLogicalDevice(usagePage, usage) )
		return;
	
	CFRange r = { 0, CFArrayGetCount(elements) };
		
	CFArrayApplyFunction( elements, r, Device::AddElement, This );
}

void Device::AddElement( const void *value, void *context )
{
	if( CFGetTypeID(CFTypeRef(value)) != CFDictionaryGetTypeID() )
		return;
	
	CFDictionaryRef dict = CFDictionaryRef( value );
	Device *This = (Device *)context;
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
	if( !object || CFGetTypeID(object) != numID || !IntValue(object, &usagePage) )
		return;
	
	// Get usage
	object = CFDictionaryGetValue( dict, CFSTR(kIOHIDElementUsageKey) );
	if( !object || CFGetTypeID(object) != numID || !IntValue(object, &usage) )
		return;
	
	
	// Get cookie
	object = CFDictionaryGetValue( dict, CFSTR(kIOHIDElementCookieKey) );
	if( !object || CFGetTypeID(object) != numID || !IntValue(object, &cookie) )
		return;
	This->AddElement( usagePage, usage, cookie, dict );
}

class JoystickDevice : public Device
{
private:
	vector<Joystick> mSticks;
	
protected:
	bool AddLogicalDevice( int usagePage, int usage );
	void AddElement( int usagePage, int usage, int cookie, const CFDictionaryRef dict );
	void Open();
	
public:
	// returns the number of IDs assigned starting from startID
	int AssignJoystickIDs( int startID );
	inline int NumberOfSticks() const { return mSticks.size(); }
	inline const Joystick& GetStick( int index ) const { return mSticks[index]; }
};

bool JoystickDevice::AddLogicalDevice( int usagePage, int usage )
{
	if( usagePage != kHIDPage_GenericDesktop || usage != kHIDUsage_GD_Joystick )
		return false;
	mSticks.push_back( Joystick() );
	return true;
}

void JoystickDevice::AddElement( int usagePage, int usage, int cookie, const CFDictionaryRef dict )
{
	CFTypeRef object;
	CFTypeID numID = CFNumberGetTypeID();
	
	Joystick& js = mSticks.back();
	
	switch( usagePage )
	{
	case kHIDPage_GenericDesktop:
	{
		int min = 0;
		int max = 0;
		
		object = CFDictionaryGetValue( dict, CFSTR(kIOHIDElementMinKey) );
		if( object && CFGetTypeID(object) == numID )
			IntValue( object, &min );
		
		object = CFDictionaryGetValue( dict, CFSTR(kIOHIDElementMaxKey) );
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

void JoystickDevice::Open()
{
	// Add elements to the queue for each Joystick
	FOREACH_CONST( Joystick, mSticks, i )
	{
		const Joystick& js = *i;

		if( js.x_axis )
			AddElementToQueue( js.x_axis );
		if( js.y_axis )
			AddElementToQueue( js.y_axis );
		if( js.z_axis )
			AddElementToQueue( js.z_axis );

		for( hash_map<int,int>::const_iterator j = js.mapping.begin(); j != js.mapping.end(); ++j )
			AddElementToQueue( j->first );
	}
}

int JoystickDevice::AssignJoystickIDs( int startID )
{
	for( vector<Joystick>::iterator i = mSticks.begin(); i != mSticks.end(); ++i )
		i->id = InputDevice( startID++ );
	return mSticks.size();
}

class KeyboardDevice : public Device
{
private:
	hash_map<int, int> mMapping;
	
protected:
	bool AddLogicalDevice( int usagePage, int usage );
	void AddElement( int usagePage, int usage, int cookie, const CFDictionaryRef dict );
	void Open();
	
	friend void InputHandler_Carbon::QueueCallBack(void *, int, void *, void *);
};

bool KeyboardDevice::AddLogicalDevice( int usagePage, int usage )
{
	return usagePage == kHIDPage_GenericDesktop && usage == kHIDUsage_GD_Keyboard;
}

void KeyboardDevice::AddElement( int usagePage, int usage, int cookie, const CFDictionaryRef dict )
{
	if( usagePage != kHIDPage_KeyboardOrKeypad )
		return;
	
	if( usage < kHIDUsage_KeyboardA )
		return;

	if( usage <= kHIDUsage_KeyboardZ )
	{
		mMapping[cookie] = usage - kHIDUsage_KeyboardA + KEY_Ca;
		return;
	}
	
	// KEY_C0 = KEY_C1 - 1, kHIDUsage_Keyboard0 = kHIDUsage_Keyboard9 + 1
	if( usage <= kHIDUsage_Keyboard9 )
	{
		mMapping[cookie] = usage - kHIDUsage_Keyboard1 + KEY_C1;
		return;
	}
	
	if( usage >= kHIDUsage_KeyboardF1 && usage <= kHIDUsage_KeyboardF12 )
	{
		mMapping[cookie] = usage - kHIDUsage_KeyboardF1 + KEY_F1;
		return;
	}
	
	if( usage >= kHIDUsage_KeyboardF13 && usage <= kHIDUsage_KeyboardF16 )
	{
		mMapping[cookie] = usage - kHIDUsage_KeyboardF13 + KEY_F13;
		return;
	}
	
	// keypad 0 is again backward
	if( usage >= kHIDUsage_Keypad1 && usage <= kHIDUsage_Keypad9 )
	{
		mMapping[cookie] = usage - kHIDUsage_Keypad1 + KEY_KP_C1;
		return;
	}
	
#define OTHER(n) (KEY_OTHER_0 + (n))
	
	// [0, 8]
	if( usage >= kHIDUsage_KeyboardF17 && usage <= kHIDUsage_KeyboardExecute )
	{
		mMapping[cookie] = usage - kHIDUsage_KeyboardF17 + OTHER(0);
		return;
	}
	
	// [9, 19]
	if( usage >= kHIDUsage_KeyboardSelect && usage <= kHIDUsage_KeyboardVolumeDown )
	{
		mMapping[cookie] = usage - kHIDUsage_KeyboardSelect + OTHER(9);
		return;
	}
	
	// [10, 31]
	if( usage >= kHIDUsage_KeypadEqualSignAS400 && usage <= kHIDUsage_KeyboardCancel )
	{
		mMapping[cookie] = usage - kHIDUsage_KeypadEqualSignAS400 + OTHER(10);
		return;
	}

	// [32, 37] 
	// XXX kHIDUsage_KeyboardClearOrAgain
	if( usage >= kHIDUsage_KeyboardSeparator && usage <= kHIDUsage_KeyboardExSel )
	{
		mMapping[cookie] = usage - kHIDUsage_KeyboardSeparator + OTHER(32);
		return;
	}
	
#define X(x,y) case x: mMapping[cookie] = y; return
	
	// Time for the special cases
	switch( usage )
	{
		X( kHIDUsage_Keyboard0, KEY_C0 );
		X( kHIDUsage_Keypad0, KEY_KP_C0 );
		X( kHIDUsage_KeyboardReturnOrEnter, KEY_ENTER );
		X( kHIDUsage_KeyboardEscape, KEY_ESC );
		X( kHIDUsage_KeyboardDeleteOrBackspace, KEY_BACK );
		X( kHIDUsage_KeyboardTab, KEY_TAB );
		X( kHIDUsage_KeyboardSpacebar, KEY_SPACE );
		X( kHIDUsage_KeyboardHyphen, KEY_HYPHEN );
		X( kHIDUsage_KeyboardEqualSign, KEY_EQUAL );
		X( kHIDUsage_KeyboardOpenBracket, KEY_LBRACKET );
		X( kHIDUsage_KeyboardCloseBracket, KEY_RBRACKET );
		X( kHIDUsage_KeyboardBackslash, KEY_BACKSLASH );
		X( kHIDUsage_KeyboardNonUSPound, KEY_HASH );
		X( kHIDUsage_KeyboardSemicolon, KEY_SEMICOLON );
		X( kHIDUsage_KeyboardQuote, KEY_SQUOTE );
		X( kHIDUsage_KeyboardGraveAccentAndTilde, KEY_ACCENT );
		X( kHIDUsage_KeyboardComma, KEY_COMMA );
		X( kHIDUsage_KeyboardPeriod, KEY_PERIOD );
		X( kHIDUsage_KeyboardSlash, KEY_SLASH );
		X( kHIDUsage_KeyboardCapsLock, KEY_CAPSLOCK );
		X( kHIDUsage_KeyboardPrintScreen, KEY_PRTSC );
		X( kHIDUsage_KeyboardScrollLock, KEY_SCRLLOCK );
		X( kHIDUsage_KeyboardPause, OTHER(0) );
		X( kHIDUsage_KeyboardInsert, KEY_INSERT );
		X( kHIDUsage_KeyboardHome, KEY_HOME );
		X( kHIDUsage_KeyboardPageUp, KEY_PGUP );
		X( kHIDUsage_KeyboardDeleteForward, KEY_DEL );
		X( kHIDUsage_KeyboardEnd, KEY_END );
		X( kHIDUsage_KeyboardPageDown, KEY_PGDN );
		X( kHIDUsage_KeyboardRightArrow, KEY_RIGHT );
		X( kHIDUsage_KeyboardLeftArrow, KEY_LEFT );
		X( kHIDUsage_KeyboardDownArrow, KEY_DOWN );
		X( kHIDUsage_KeyboardUpArrow, KEY_UP );
		X( kHIDUsage_KeypadNumLock, KEY_NUMLOCK );
		X( kHIDUsage_KeypadSlash, KEY_KP_SLASH );
		X( kHIDUsage_KeypadEqualSign, KEY_KP_EQUAL );
		X( kHIDUsage_KeypadAsterisk, KEY_KP_ASTERISK );
		X( kHIDUsage_KeypadHyphen, KEY_KP_HYPHEN );
		X( kHIDUsage_KeypadPlus, KEY_KP_PLUS );
		X( kHIDUsage_KeypadEnter, KEY_KP_ENTER );
		X( kHIDUsage_KeypadPeriod, KEY_KP_PERIOD );
		X( kHIDUsage_KeyboardNonUSBackslash, OTHER(38) );
		X( kHIDUsage_KeyboardApplication, OTHER(39) );
		X( kHIDUsage_KeyboardClear, KEY_NUMLOCK ); // XXX
		X( kHIDUsage_KeyboardHelp, KEY_INSERT );
		X( kHIDUsage_KeyboardMenu, KEY_MENU );
		// XXX kHIDUsage_KeyboardLockingCapsLock
		// XXX kHIDUsage_KeyboardLockingNumLock
		// XXX kHIDUsage_KeyboardLockingScrollLock
		X( kHIDUsage_KeypadComma, KEY_KP_PERIOD ); // XXX
		X( kHIDUsage_KeyboardReturn, KEY_ENTER );
		X( kHIDUsage_KeyboardPrior, OTHER(40) );
		X( kHIDUsage_KeyboardLeftControl, KEY_LCTRL );
		X( kHIDUsage_KeyboardLeftShift, KEY_LSHIFT );
		X( kHIDUsage_KeyboardLeftAlt, KEY_LALT );
		X( kHIDUsage_KeyboardLeftGUI, KEY_LMETA ); // XXX GUI??
		X( kHIDUsage_KeyboardRightControl, KEY_RCTRL );
		X( kHIDUsage_KeyboardRightShift, KEY_RSHIFT );
		X( kHIDUsage_KeyboardRightAlt, KEY_RALT );
		X( kHIDUsage_KeyboardRightGUI, KEY_RMETA ); // XXX
	}
#undef X
#undef OTHER
}

void KeyboardDevice::Open()
{
	for (hash_map<int, int>::const_iterator i = mMapping.begin(); i != mMapping.end(); ++i)
		AddElementToQueue( i->first );
}

void InputHandler_Carbon::QueueCallBack( void *target, int result, void *refcon, void *sender )
{
	// The result seems useless as you can't actually return anything...
	// refcon is the Device number

	RageTimer now;
	InputHandler_Carbon *This = (InputHandler_Carbon *)target;
	IOHIDQueueInterface **queue = (IOHIDQueueInterface **)sender;
	IOHIDEventStruct event;
	AbsoluteTime zeroTime = { 0, 0 };
	Device *dev = This->mDevices[int( refcon )];
	KeyboardDevice *kd = dynamic_cast<KeyboardDevice *>(dev);
	JoystickDevice *jd = dynamic_cast<JoystickDevice *>(dev);
	
	ASSERT( kd || jd );
	
	while( (result = CALL(queue, getNextEvent, &event, zeroTime, 0)) == kIOReturnSuccess )
	{
		int cookie = int( event.elementCookie );
		int value = event.value;
		
		if( kd )
		{
			hash_map<int, int>::const_iterator iter = kd->mMapping.find( cookie );
			
			if( iter != kd->mMapping.end() )
				This->ButtonPressed( DeviceInput(DEVICE_KEYBOARD, iter->second, value, now), value );
			continue;
		}

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

static void RunLoopStarted( CFRunLoopObserverRef o, CFRunLoopActivity a, void *sem )
{
	CFRelease(o); // we don't need this any longer
	((RageSemaphore *)sem)->Post();
}

int InputHandler_Carbon::Run(void *data)
{
	InputHandler_Carbon *This = (InputHandler_Carbon *)data;
	CFRunLoopRef loopRef = CFRunLoopGetCurrent();
	int n = 0;
	
	CFRetain(loopRef);
	FOREACH( Device *, This->mDevices, i )
		(*i)->StartQueue( loopRef, InputHandler_Carbon::QueueCallBack, This, n++ );
	This->mLoopRef = loopRef;
	
	/* The function copies the information out of the structure, so the memory pointed
	 * to by context does not need to persist beyond the function call.
	 */
	CFRunLoopObserverContext context = { 0, &This->mSem, NULL, NULL, NULL };
	CFRunLoopObserverRef o = CFRunLoopObserverCreate( kCFAllocatorDefault, kCFRunLoopEntry,
													  false, 0, RunLoopStarted, &context);
	CFRunLoopAddObserver( loopRef, o, kCFRunLoopDefaultMode );
	CFRunLoopRun();
	return 0;
}

InputHandler_Carbon::~InputHandler_Carbon()
{
	CFRunLoopStop( CFRunLoopRef(mLoopRef) );
	mInputThread.Wait();
	FOREACH( Device *, mDevices, i )
		delete *i;
	if( mMasterPort )
		mach_port_deallocate( mach_task_self(), mMasterPort );
}

static io_iterator_t GetDeviceIterator( mach_port_t mp, int usagePage, int usage )
{
	// Build the matching dictionary.
	CFMutableDictionaryRef dict;
	
	if( (dict = IOServiceMatching(kIOHIDDeviceKey)) == NULL )
	{
		LOG->Warn( "Couldn't create a matching dictionary." );
		return NULL;
	}
	// Refine the search by only looking for joysticks
	CFNumberRef usagePageRef = CFInt( usagePage );
	CFNumberRef usageRef = CFInt( usage );
	
	CFDictionarySetValue( dict, CFSTR(kIOHIDPrimaryUsagePageKey), usagePageRef );
	CFDictionarySetValue( dict, CFSTR(kIOHIDPrimaryUsageKey), usageRef );
	
	// Cleanup after ourselves
	CFRelease( usagePageRef );
	CFRelease( usageRef );
	
	// Find the HID devices.
	io_iterator_t iter;
	
	/* Get an iterator to the matching devies
	 * This consumes a reference to the dictionary so we should retain it
	 * have to Release() later.
	 */
	if( IOServiceGetMatchingServices(mp, dict, &iter) != kIOReturnSuccess )
	{
		LOG->Warn( "Couldn't get matching services" );
		return NULL;
	}
	return iter;
}

InputHandler_Carbon::InputHandler_Carbon() : mMasterPort(0), mSem("Input thread started")
{
	// Get a Mach port to initiate communication with I/O Kit.
	mach_port_t masterPort;

	IOReturn ret = IOMasterPort( MACH_PORT_NULL, &masterPort );

	if( ret != kIOReturnSuccess )
	{
		PrintIOErr( ret, "Couldn't create a master I/O Kit port." );
		return;
	}
	mMasterPort = masterPort;

	// Find the joysticks
	io_iterator_t iter = GetDeviceIterator(masterPort, kHIDPage_GenericDesktop, kHIDUsage_GD_Joystick );
	
	if( iter )
	{
		// Iterate over the devices and add them
		io_object_t device;
		int id = DEVICE_JOY1;
		
		while( (device = IOIteratorNext(iter)) )
		{
			JoystickDevice *jd = new JoystickDevice;
			
			if( static_cast<Device *>(jd)->Open(device) )
			{
				id += jd->AssignJoystickIDs( id );
				mDevices.push_back( jd );
			}
			else
			{
				delete jd;
			}
			
			IOObjectRelease( device );
			
		}
		IOObjectRelease( iter );
	}
	// Now find the keyboards
	iter = GetDeviceIterator( masterPort, kHIDPage_GenericDesktop, kHIDUsage_GD_Keyboard );
	
	if( iter )
	{
		io_object_t device;
		
		while( (device = IOIteratorNext(iter)) )
		{
			Device *kd = new KeyboardDevice;
			
			if( kd->Open(device) )
				mDevices.push_back( kd );
			else
				delete kd;
			
			IOObjectRelease( device );
		}
		IOObjectRelease( iter );
	}
	
	mInputThread.SetName( "Input thread." );
	mInputThread.Create(InputHandler_Carbon::Run, this);
	// Wait for the run loop to start before returning.
	mSem.Wait();
}

void InputHandler_Carbon::GetDevicesAndDescriptions( vector<InputDevice>& dev, vector<CString>& desc )
{
	dev.push_back(DEVICE_KEYBOARD);
	desc.push_back("Keyboard");

	FOREACH_CONST( Device *, mDevices, i )
	{
		const JoystickDevice *jd = dynamic_cast<const JoystickDevice *>(*i);
		
		/* This could be break since right now KeyboardDevices follow
		 * the JoystickDevices, but that is brittle.
		 */
		if (!jd)
			continue;
		
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
