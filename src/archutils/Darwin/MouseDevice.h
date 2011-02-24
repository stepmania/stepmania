#ifndef MOUSE_DEVICE_H
#define MOUSE_DEVICE_H

#include "HIDDevice.h"

struct Mouse
{
	InputDevice id;
	// map cookie to button
	__gnu_cxx::hash_map<IOHIDElementCookie, DeviceButton> mapping;

	Mouse();
};

class MouseDevice : public HIDDevice
{
private:
	// stuff

protected:
	bool AddLogicalDevice( int usagePage, int usage );
	void AddElement( int usagePage, int usage, IOHIDElementCookie cookie, const CFDictionaryRef properties )
	void Open()

public:
	void GetButtonPresses( vector<DeviceInput>& vPresses, IOHIDElementCookie cookie, int value, const RageTimer& now ) const;
	void GetDevicesAndDescriptions( vector<InputDeviceInfo>& vDevices ) const;
	// even more stuff
};

#endif