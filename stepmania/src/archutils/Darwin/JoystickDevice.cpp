#include "global.h"
#include "JoystickDevice.h"
#include "Foreach.h"
#include "EnumHelper.h"
#include "RageUtil.h"

using namespace std;
using __gnu_cxx::hash_map;

Joystick::Joystick() :	id( DEVICE_NONE ),
			x_axis( DeviceButton_Invalid ),
			y_axis( DeviceButton_Invalid ),
			z_axis( DeviceButton_Invalid ),
			x_min( 0 ), y_min( 0 ), z_min( 0 ),
			x_max( 0 ), y_max( 0 ), z_max( 0 )
{
}


bool JoystickDevice::AddLogicalDevice( int usagePage, int usage )
{
	if( usagePage != kHIDPage_GenericDesktop || usage != kHIDUsage_GD_Joystick )
		return false;
	m_vSticks.push_back( Joystick() );
	return true;
}

void JoystickDevice::AddElement( int usagePage, int usage, int cookie, const CFDictionaryRef dict )
{
	CFTypeRef object;
	CFTypeID numID = CFNumberGetTypeID();
	
	ASSERT( m_vSticks.size() );
	Joystick& js = m_vSticks.back();
	
	switch( usagePage )
	{
	case kHIDPage_GenericDesktop:
	{
		int iMin = 0;
		int iMax = 0;
		
		object = CFDictionaryGetValue( dict, CFSTR(kIOHIDElementMinKey) );
		if( object && CFGetTypeID(object) == numID )
			IntValue( object, iMin );
		
		object = CFDictionaryGetValue( dict, CFSTR(kIOHIDElementMaxKey) );
		if( object && CFGetTypeID(object) == numID )
			IntValue( object, iMax );
		
		switch( usage )
		{
		case kHIDUsage_GD_X:
			js.x_axis = cookie;
			js.x_min = iMin;
			js.x_max = iMax;
			break;
		case kHIDUsage_GD_Y:
			js.y_axis = cookie;
			js.y_min = iMin;
			js.y_max = iMax;
			break;
		case kHIDUsage_GD_Z:
			js.z_axis = cookie;
			js.z_min = iMin;
			js.z_max = iMax;
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
		const DeviceButton buttonID = enum_add2( JOY_BUTTON_1, usage - 1 );
	
		if( buttonID <= JOY_BUTTON_32 )
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
	FOREACH_CONST( Joystick, m_vSticks, i )
	{
		const Joystick& js = *i;
		
		if( js.x_axis )
			AddElementToQueue( js.x_axis );
		if( js.y_axis )
			AddElementToQueue( js.y_axis );
		if( js.z_axis )
			AddElementToQueue( js.z_axis );
		
		for( hash_map<int,DeviceButton>::const_iterator j = js.mapping.begin(); j != js.mapping.end(); ++j )
			AddElementToQueue( j->first );
	}
}

void JoystickDevice::GetButtonPresses( vector<pair<DeviceInput, bool> >& vPresses, int cookie,
				       int value, const RageTimer& now ) const
{
	typedef pair<DeviceInput, bool> dib;
	FOREACH_CONST( Joystick, m_vSticks, i )
	{
		const Joystick& js = *i;
		
		if( js.x_axis == cookie )
		{
			float level = SCALE( value, js.x_min, js.x_max, -1.0f, 1.0f );
			
			vPresses.push_back( dib(DeviceInput(js.id, JOY_LEFT, max(-level, 0.0f), now), level < -0.5f) );
			vPresses.push_back( dib(DeviceInput(js.id, JOY_RIGHT, max(level, 0.0f), now), level > 0.5f) );
			break;
		}
		else if( js.y_axis == cookie )
		{
			float level = SCALE( value, js.y_min, js.y_max, -1.0f, 1.0f );
			
			vPresses.push_back( dib(DeviceInput(js.id, JOY_UP, max(-level, 0.0f), now), level < -0.5f) );
			vPresses.push_back( dib(DeviceInput(js.id, JOY_DOWN, max(level, 0.0f), now), level > 0.5f) );
			break;
		}
		else if( js.z_axis == cookie )
		{
			float level = SCALE( value, js.z_min, js.z_max, -1.0f, 1.0f );
			
			vPresses.push_back( dib(DeviceInput(js.id, JOY_Z_UP, max(-level, 0.0f), now), level < -0.5f) );
			vPresses.push_back( dib(DeviceInput(js.id, JOY_Z_DOWN, max(level, 0.0f), now), level > 0.5f) );
			break;
		}
		else
		{
			// hash_map<T,U>::operator[] is not const
			hash_map<int, DeviceButton>::const_iterator iter;
			
			iter = js.mapping.find( cookie );
			if( iter != js.mapping.end() )
			{
				vPresses.push_back( dib(DeviceInput(js.id, iter->second, value, now), value) );
				break;
			}
		}
	}
}

int JoystickDevice::AssignIDs( InputDevice startID )
{
	FOREACH( Joystick, m_vSticks, i )
	{
		i->id = InputDevice( startID );
		enum_add( startID, 1 );
	}
	return m_vSticks.size();
}

void JoystickDevice::GetDevicesAndDescriptions( vector<InputDevice>& dev, vector<RString>& desc ) const
{
	FOREACH_CONST( Joystick, m_vSticks, i )
	{
		dev.push_back( i->id );
		const RString& description = GetDescription();
		
		// XXX hack around remapping problem.
		if( description == "0b43:0003" )
			desc.push_back( "EMS USB2" );
		else
			desc.push_back( description );
	}
}

