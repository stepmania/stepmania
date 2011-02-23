#include "global.h"
#include "MouseDevice.h"

bool MouseDevice::AddLogicalDevice( int usagePage, int usage )
{
	// Mice can either be kHIDUsage_GD_Mouse or kHIDUsage_GD_Pointer...
	// Let's just go with kHIDUsage_GD_Mouse for now.
	return usagePage == kHIDPage_GenericDesktop && usage == kHIDUsage_GD_Mouse;
}

void MouseDevice::AddElement( int usagePage, int usage, IOHIDElementCookie cookie, const CFDictionaryRef properties )
{
	if( usagePage != kHIDPage_GenericDesktop )
		return;

	DeviceButton button;
	if( UsbKeyToDeviceButton(usage,button) )
		m_Mapping[cookie] = button;
}

void MouseDevice::Open()
{
	// I hate coding blind...
}