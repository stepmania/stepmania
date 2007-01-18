#include "global.h"
#include "JoystickDevice.h"
#include "RageLog.h"
#include "Foreach.h"

using namespace std;
using __gnu_cxx::hash_map;

Joystick::Joystick() :	id( InputDevice_Invalid ),
			x_axis( 0 ), x_min( 0 ), x_max( 0 ),
			y_axis( 0 ), y_min( 0 ), y_max( 0 ),
			z_axis( 0 ), z_min( 0 ), z_max( 0 ),
			x_rot( 0 ), rx_min( 0 ), rx_max( 0 ),
			y_rot( 0 ), ry_min( 0 ), ry_max( 0 ),
			z_rot( 0 ), rz_min( 0 ), rz_max( 0 ),
			hat( 0 ), hat_min( 0 ), hat_max( 0 )
{
}


bool JoystickDevice::AddLogicalDevice( int usagePage, int usage )
{
	if( usagePage != kHIDPage_GenericDesktop )
		return false;
	switch( usage )
	{
	case kHIDUsage_GD_Joystick:
	case kHIDUsage_GD_GamePad:
		break;
	default:
		return false;
	}
	m_vSticks.push_back( Joystick() );
	return true;
}

void JoystickDevice::AddElement( int usagePage, int usage, int cookie, const CFDictionaryRef properties )
{
	if( usagePage >= kHIDPage_VendorDefinedStart )
		return;

	ASSERT( m_vSticks.size() );
	Joystick& js = m_vSticks.back();

	switch( usagePage )
	{
	case kHIDPage_GenericDesktop:
	{
		int iMin = 0;
		int iMax = 0;
		
		IntValue( CFDictionaryGetValue(properties, CFSTR(kIOHIDElementMinKey)), iMin );
		IntValue( CFDictionaryGetValue(properties, CFSTR(kIOHIDElementMaxKey)), iMax );
		
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
		case kHIDUsage_GD_Rx:
			js.x_rot = cookie;
			js.rx_min = iMin;
			js.rx_max = iMax;
			break;
		case kHIDUsage_GD_Ry:
			js.y_rot = cookie;
			js.ry_min = iMin;
			js.ry_max = iMax;
			break;
		case kHIDUsage_GD_Rz:
			js.z_rot = cookie;
			js.rz_min = iMin;
			js.rz_max = iMax;
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
		case kHIDUsage_GD_Hatswitch:
		{
			if( iMax - iMin != 7 && iMax - iMin != 3 )
				break;
			js.hat = cookie;
			js.hat_min = iMin;
			js.hat_max = iMax;
			break;
		}
		default:
			LOG->Warn( "Unknown usagePage usage pair: (kHIDPage_GenericDesktop, %d).", usage );
			break;
		}
		break;
	}
	case kHIDPage_Button:
	{
		const DeviceButton buttonID = enum_add2( JOY_BUTTON_1, usage - kHIDUsage_Button_1 );
	
		if( buttonID <= JOY_BUTTON_32 )
			js.mapping[cookie] = buttonID;
		else
			LOG->Warn( "Button id too large: %d.", int(buttonID) );
		break;
	}
	default:
		//LOG->Warn( "Unknown usagePage usage pair: (%d, %d).", usagePage, usage );
		break;
	} // end switch (usagePage)
}

void JoystickDevice::Open()
{
	// Add elements to the queue for each Joystick
	FOREACH_CONST( Joystick, m_vSticks, i )
	{
		const Joystick& js = *i;
#define ADD(x) if( js.x ) AddElementToQueue( js.x )
		ADD( x_axis );	ADD( y_axis );	ADD( z_axis );
		ADD( x_rot );	ADD( y_rot );	ADD( z_rot );
		ADD( hat );
#undef ADD		
		for( hash_map<int,DeviceButton>::const_iterator j = js.mapping.begin(); j != js.mapping.end(); ++j )
			AddElementToQueue( j->first );
	}
}

bool JoystickDevice::InitDevice( int vid, int pid )
{
	if( vid != 0x0507 || pid != 0x0011 )
		return true;
	// It's a Para controller so try to power it on.
	uint8_t powerOn = 1;
	IOReturn ret = SetReport( kIOHIDReportTypeFeature, 0, &powerOn, 1, 10 );
	
	if( ret )
		LOG->Warn( "Failed to power on the Para controller: %#08x", ret );
	return ret == kIOReturnSuccess;
}

void JoystickDevice::GetButtonPresses( vector<DeviceInput>& vPresses, int cookie, int value, const RageTimer& now ) const
{
	FOREACH_CONST( Joystick, m_vSticks, i )
	{
		const Joystick& js = *i;
		
		if( js.x_axis == cookie )
		{
			float level = SCALE( value, js.x_min, js.x_max, -1.0f, 1.0f );
			
			vPresses.push_back( DeviceInput(js.id, JOY_LEFT, max(-level, 0.0f), now) );
			vPresses.push_back( DeviceInput(js.id, JOY_RIGHT, max(level, 0.0f), now) );
			break;
		}
		else if( js.y_axis == cookie )
		{
			float level = SCALE( value, js.y_min, js.y_max, -1.0f, 1.0f );
			
			vPresses.push_back( DeviceInput(js.id, JOY_UP, max(-level, 0.0f), now) );
			vPresses.push_back( DeviceInput(js.id, JOY_DOWN, max(level, 0.0f), now) );
			break;
		}
		else if( js.z_axis == cookie )
		{
			float level = SCALE( value, js.z_min, js.z_max, -1.0f, 1.0f );
			
			vPresses.push_back( DeviceInput(js.id, JOY_Z_UP, max(-level, 0.0f), now) );
			vPresses.push_back( DeviceInput(js.id, JOY_Z_DOWN, max(level, 0.0f), now) );
			break;
		}
		else if( js.x_rot == cookie )
		{
			float level = SCALE( value, js.rx_min, js.rx_max, -1.0f, 1.0f );
			
			vPresses.push_back( DeviceInput(js.id, JOY_ROT_LEFT, max(-level, 0.0f), now) );
			vPresses.push_back( DeviceInput(js.id, JOY_ROT_RIGHT, max(level, 0.0f), now) );
			break;
		}
		else if( js.y_rot == cookie )
		{
			float level = SCALE( value, js.ry_min, js.ry_max, -1.0f, 1.0f );
			
			vPresses.push_back( DeviceInput(js.id, JOY_ROT_UP, max(-level, 0.0f), now) );
			vPresses.push_back( DeviceInput(js.id, JOY_ROT_DOWN, max(level, 0.0f), now) );
			break;
		}
		else if( js.z_rot == cookie )
		{
			float level = SCALE( value, js.rz_min, js.rz_max, -1.0f, 1.0f );
			
			vPresses.push_back( DeviceInput(js.id, JOY_ROT_Z_UP, max(-level, 0.0f), now) );
			vPresses.push_back( DeviceInput(js.id, JOY_ROT_Z_DOWN, max(level, 0.0f), now) );
			break;
		}
		else if( js.hat == cookie )
		{
			float levelUp = 0.f, levelRight = 0.f, levelDown = 0.f, levelLeft = 0.f;
			
			value -= js.hat_min; // Probably just subtracting 0.
			if( js.hat_max - js.hat_min == 3 )
				value *= 2;
			switch( value )
			{
			case 0:	levelUp = 1.f;					break;	// U
			case 1:	levelUp = 1.f;		levelRight = 1.f;	break;	// UR
			case 2:				levelRight = 1.f;	break;	// R
			case 3:	levelDown = 1.f;	levelRight = 1.f;	break;	// DR
			case 4:	levelDown = 1.f;				break;	// D
			case 5:	levelDown = 1.f;	levelLeft = 1.f;	break;	// DL
			case 6:				levelLeft = 1.f;	break;	// L
			case 7: levelUp = 1.f;		levelLeft = 1.f;	break;	// UL
			}
			vPresses.push_back( DeviceInput(js.id, JOY_HAT_UP,	levelUp,	now) );
			vPresses.push_back( DeviceInput(js.id, JOY_HAT_RIGHT,	levelRight,	now) );
			vPresses.push_back( DeviceInput(js.id, JOY_HAT_DOWN,	levelDown,	now) );
			vPresses.push_back( DeviceInput(js.id, JOY_HAT_LEFT,	levelLeft,	now) );
			break;
		}
		else
		{
			// hash_map<T,U>::operator[] is not const
			hash_map<int, DeviceButton>::const_iterator iter;
			
			iter = js.mapping.find( cookie );
			if( iter != js.mapping.end() )
			{
				vPresses.push_back( DeviceInput(js.id, iter->second, value, now) );
				break;
			}
		}
	}
}

int JoystickDevice::AssignIDs( InputDevice startID )
{
	if( !IsJoystick(startID) )
		return -1;
	FOREACH( Joystick, m_vSticks, i )
	{
		if( !IsJoystick(startID) )
		{
			m_vSticks.erase( i, m_vSticks.end() );
			break;
		}
		i->id = startID;
		enum_add( startID, 1 );
	}
	return m_vSticks.size();
}

void JoystickDevice::GetDevicesAndDescriptions( vector<InputDeviceInfo>& vDevices ) const
{
	FOREACH_CONST( Joystick, m_vSticks, i )
		vDevices.push_back( InputDeviceInfo(i->id,GetDescription()) );
}

/*
 * (c) 2005-2007 Steve Checkoway
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
