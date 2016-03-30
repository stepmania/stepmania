#include "global.h"
#include "MouseDevice.h"
#include "RageMath.hpp"

Mouse::Mouse() : id( InputDevice_Invalid ),
				x_axis( 0 ), y_axis( 0 ), z_axis( 0 ),
				x_min( 0 ), x_max( 0 ), y_min( 0 ),
				y_max( 0 ), z_min( 0 ), z_max( 0 )
{
}

bool MouseDevice::AddLogicalDevice( int usagePage, int usage )
{
	if( usagePage != kHIDPage_GenericDesktop )
		return false;

	switch( usage )
	{
		// Mice can either be kHIDUsage_GD_Mouse or kHIDUsage_GD_Pointer.
		case kHIDUsage_GD_Mouse:
		case kHIDUsage_GD_Pointer:
			break;
		default:
			return false;
	}
	// Init only a single mouse for now.
	m_Mouse = Mouse();
	return true;
}

void MouseDevice::AddElement( int usagePage, int usage, IOHIDElementCookie cookie, const CFDictionaryRef properties )
{
	if( usagePage >= kHIDPage_VendorDefinedStart )
		return;

	Mouse& m = m_Mouse;

	switch( usagePage )
	{
		case kHIDPage_GenericDesktop:
			{ // ease scope
				int iMin = 0;
				IntValue( CFDictionaryGetValue(properties, CFSTR(kIOHIDElementMinKey)), iMin );
				int iMax = 0;
				IntValue( CFDictionaryGetValue(properties, CFSTR(kIOHIDElementMaxKey)), iMax );

				// based on usage
				switch( usage )
				{
					case kHIDUsage_GD_X:
						m.x_axis = cookie;
						m.x_min = iMin;
						m.x_max = iMax;
						break;
					case kHIDUsage_GD_Y:
						m.y_axis = cookie;
						m.y_min = iMin;
						m.y_max = iMax;
						break;
					case kHIDUsage_GD_Z:
					case kHIDUsage_GD_Wheel: // also handle wheel
						m.z_axis = cookie;
						m.z_min = iMin;
						m.z_max = iMax;
						break;
					default:
						//LOG->Warn( "Unknown usagePage usage pair: (kHIDPage_GenericDesktop, %d).", usage );
						break;
				}
			}
			break;
		case kHIDPage_Button:
			{
				const DeviceButton buttonID = enum_add2( MOUSE_LEFT, usage - kHIDUsage_Button_1 );

				if( buttonID <= MOUSE_MIDDLE )
					m_Mapping[cookie] = buttonID;
				else
					LOG->Warn( "Button id too large: %d.", int(buttonID) );
				break;
			}
			break;
		default:
			//LOG->Warn( "Unknown usagePage usage pair: (%d, %d).", usagePage, usage );
			break;
	}

}

void MouseDevice::Open()
{
	const Mouse& m = m_Mouse;
#define ADD(x) if( m.x ) AddElementToQueue( m.x )
	ADD( x_axis );
	ADD( y_axis );
	ADD( z_axis );
#undef ADD
	for (auto &item: m_Mapping)
	{
		AddElementToQueue( item.first );
	}
}

void MouseDevice::GetButtonPresses( vector<DeviceInput>& vPresses, IOHIDElementCookie cookie, int value, const RageTimer& now ) const
{
	using std::max;
	// todo: add mouse axis stuff -aj
	const Mouse& m = m_Mouse;

	if( m.x_axis == cookie )
	{
		LOG->Trace("Mouse X: Value = %i",value);
	}
	else if( m.y_axis == cookie )
	{
		LOG->Trace("Mouse Y: Value = %i",value);
	}
	else if( m.z_axis == cookie )
	{
		float level = Rage::scale( value + 0.f, m.z_min + 0.f, m.z_max + 0.f, -1.0f, 1.0f );
		INPUTFILTER->ButtonPressed( DeviceInput(DEVICE_MOUSE, MOUSE_WHEELUP, max(-level,0.f), now) );
		INPUTFILTER->ButtonPressed( DeviceInput(DEVICE_MOUSE, MOUSE_WHEELDOWN, max(+level,0.f), now) );
	}
	else
	{
		auto iter = m_Mapping.find( cookie );
		if( iter != m_Mapping.end() )
			vPresses.push_back( DeviceInput(DEVICE_MOUSE, iter->second, value, now) );
	}
}

void MouseDevice::GetDevicesAndDescriptions( vector<InputDeviceInfo>& vDevices ) const
{
	vDevices.push_back( InputDeviceInfo(DEVICE_MOUSE, "Mouse") );
}
