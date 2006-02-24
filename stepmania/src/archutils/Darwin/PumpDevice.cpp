#include "global.h"
#include "PumpDevice.h"
#include "EnumHelper.h"

void PumpDevice::Open()
{
	AddElementToQueue( 2 );
	AddElementToQueue( 3 );
	AddElementToQueue( 4 );
	AddElementToQueue( 6 );
}

void PumpDevice::GetButtonPresses( vector<pair<DeviceInput, bool> >& vPresses, int cookie,
				   int value, const RageTimer& now ) const
{
	DeviceButton db1 = DeviceButton_Invalid;
	DeviceButton db2 = DeviceButton_Invalid;
	bool pressed;
	
	LOG->Trace( "Pump button: %d, %d.", cookie, value );
	switch( cookie )
	{
	case 2:
		db1 = JOY_BUTTON_1;
		pressed = value == 0;
		break;
	case 3:
		switch( value )
		{
		case 1:
			db1 = JOY_BUTTON_5;
			pressed = true;
			break;
		case 2:
			db1 = JOY_BUTTON_4;
			pressed = true;
			break;
		case 3:
			db1 = JOY_BUTTON_5;
			db2 = JOY_BUTTON_4;
			pressed = false;
			break;
		}
		break;
	case 4:
		switch( value )
		{
		case 1:
			db1 = JOY_BUTTON_2;
			pressed = true;
			break;
		case 2:
			db1 = JOY_BUTTON_3;
			pressed = true;
			break;
		case 3:
			db1 = JOY_BUTTON_2;
			db2 = JOY_BUTTON_3;
			pressed = false;
			break;
		}
		break;
	case 6:
		db1 = JOY_BUTTON_6;
		pressed = value == 2;
		break;
	}
	
	if( db1 != DeviceButton_Invalid )
	{
		DeviceInput di( id, db1, pressed ? 1.0f : 0.0f , now );
		vPresses.push_back( pair<DeviceInput, bool>(di, pressed) );
	}
	if( db2 != DeviceButton_Invalid )
	{
		DeviceInput di( id, db2, pressed ? 1.0f : 0.0f , now );
		vPresses.push_back( pair<DeviceInput, bool>(di, pressed) );
	}
}

int PumpDevice::AssignIDs( InputDevice startID )
{
	id = DEVICE_NONE;
	if( startID < DEVICE_PUMP1 || startID > DEVICE_PUMP2 )
		return 0;
	id = startID;
	return 1;
}

void PumpDevice::GetDevicesAndDescriptions( vector<InputDevice>& dev, vector<RString>& desc ) const
{
	dev.push_back( id );
	desc.push_back( "Pump USB" );
}

/*
 * (c) 2005-2006 Steve Checkoway
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
