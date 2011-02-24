#include "global.h"
#include "MouseDevice.h"

using __gnu_cxx::hash_map;

bool MouseDevice::AddLogicalDevice( int usagePage, int usage )
{
	// Mice can either be kHIDUsage_GD_Mouse or kHIDUsage_GD_Pointer...
	// Let's just go with kHIDUsage_GD_Mouse for now. -aj
	return usagePage == kHIDPage_GenericDesktop && usage == kHIDUsage_GD_Mouse;
}

void MouseDevice::AddElement( int usagePage, int usage, IOHIDElementCookie cookie, const CFDictionaryRef properties )
{
	if( usagePage == kHIDPage_Button )
	{
		const DeviceButton buttonID = enum_add2( MOUSE_LEFT, usage - kHIDUsage_Button_1 );

		bool mouseWheel = (buttonID == MOUSE_WHEELUP || buttonID == MOUSE_WHEELDOWN);
		if( buttonID <= MOUSE_MIDDLE || mouseWheel )
			mapping[cookie] = buttonID;
		else
			LOG->Warn( "Button id too large: %d.", int(buttonID) );
		break;
	}
}

void MouseDevice::Open()
{
	for( hash_map<IOHIDElementCookie,DeviceButton>::const_iterator i = m_Mapping.begin(); i != m_Mapping.end(); ++i )
	{
		AddElementToQueue( i->first );
	}
}

void MouseDevice::GetButtonPresses( vector<DeviceInput>& vPresses, IOHIDElementCookie cookie, int value, const RageTimer& now ) const
{
	hash_map<IOHIDElementCookie, DeviceButton>::const_iterator iter = m_Mapping.find( cookie );
	if( iter != m_Mapping.end() )
	{
		vPresses.push_back( DeviceInput(DEVICE_MOUSE, iter->second, value, now) );
	}
}

void MouseDeviceDevice::GetDevicesAndDescriptions( vector<InputDeviceInfo>& vDevices ) const
{
	vDevices.push_back( InputDeviceInfo(DEVICE_MOUSE, "Mouse") );
}