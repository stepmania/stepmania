#ifndef HIDDEVICE_H
#define HIDDEVICE_H

#include <CoreFoundation/CoreFoundation.h>
#include <IOKit/hid/IOHIDLib.h>
#include <IOKit/IOKitLib.h>
#include <IOKit/IOCFPlugIn.h>
#include <IOKit/hid/IOHIDUsageTables.h>
#include <IOKit/usb/USB.h>
#include <mach/mach.h>
#include <mach/mach_error.h>
#include <vector>
#include <utility>

#include "RageLog.h"
#include "RageInputDevice.h"

/* A few helper functions. */

// The result needs to be released.
static inline CFNumberRef CFInt( int n )
{
	return CFNumberCreate( kCFAllocatorDefault, kCFNumberIntType, &n );
}

static inline void PrintIOErr( IOReturn err, const char *s )
{
	LOG->Warn( "%s - %s(%x,%d)", s, mach_error_string(err), err, err & 0xFFFFFF );
}

static inline Boolean IntValue( const void *o, int *n )
{
	return CFNumberGetValue( CFNumberRef(o), kCFNumberIntType, n );
}

/*
 * This is just awful, these aren't objects, treating them as such leads
 * to: (*object)->function(object [, argument]...)
 * Instead, do: CALL(object, function [, argument]...)
 */
#define CALL(o,f,...) (*(o))->f((o), ## __VA_ARGS__)

class HIDDevice
{
private:
	IOHIDDeviceInterface **m_Interface;
	IOHIDQueueInterface **m_Queue;
	bool m_bRunning;
	RString m_sDescription;
	
	static void AddLogicalDevice( const void *value, void *context );
	static void AddElement( const void *value, void *context );
	
protected:
	virtual bool AddLogicalDevice( int usagePage, int usage ) = 0;
	virtual void AddElement( int usagePage, int usage, int cookie, const CFDictionaryRef dict ) = 0;
	virtual void Open() = 0;
	
	inline void AddElementToQueue( int cookie )
	{
		CALL( m_Queue, addElement, IOHIDElementCookie(cookie), 0 );
	}
public:
	HIDDevice();
	virtual ~HIDDevice();
	
	bool Open( io_object_t device );
	void StartQueue( CFRunLoopRef loopRef, IOHIDCallbackFunction callback, void *target, int refCon );
	inline const RString& GetDescription() const { return m_sDescription; }
	
	
	virtual void GetButtonPresses( vector<pair<DeviceInput, bool> >& vPresses, int cookie,
				       int value, const RageTimer& now ) const = 0;
	
	/*
	 * Returns the number of IDs assigned starting from startID. This is not meaningful for devices like
	 * keyboards that all share the same InputDevice id.
	 */
	virtual int AssignIDs( InputDevice startID ) { return 0; }
	virtual void GetDevicesAndDescriptions( vector<InputDevice>& dev, vector<RString>& desc ) const = 0;
};



#endif

