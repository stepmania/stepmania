#ifndef MOUSE_DEVICE_H
#define MOUSE_DEVICE_H

#include "HIDDevice.h"

class MouseDevice : public HIDDevice
{
private:
	// stuff

protected:
	bool AddLogicalDevice( int usagePage, int usage );
	void AddElement( int usagePage, int usage, IOHIDElementCookie cookie, const CFDictionaryRef properties )
	void Open()

public:
	// even more stuff
};

#endif