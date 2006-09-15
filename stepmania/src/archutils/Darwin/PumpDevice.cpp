#include "global.h"
#include "PumpDevice.h"

void PumpDevice::Open()
{
	AddElementToQueue( 2 );
	AddElementToQueue( 3 );
	AddElementToQueue( 4 );
	AddElementToQueue( 6 );
	AddElementToQueue( 7 );
	AddElementToQueue( 8 );
}

void PumpDevice::GetButtonPresses( vector<DeviceInput>& vPresses, int cookie, int value, const RageTimer& now ) const
{
	DeviceButton db1 = DeviceButton_Invalid;
	DeviceButton db2 = DeviceButton_Invalid;
	bool pressed1 = !(value & 0x1);
	bool pressed2 = !(value & 0x2);
	
	switch( cookie )
	{
	case 2:
		db2 = JOY_BUTTON_1; // bit 9
		break;
	case 3:
		db1 = JOY_BUTTON_5; // bit 10
		db2 = JOY_BUTTON_4; // bit 11
		break;
	case 4:
		db1 = JOY_BUTTON_2; // bit 12
		db2 = JOY_BUTTON_3; // bit 13
		break;
	case 6:
		db1 = JOY_BUTTON_6; // bit 16
		db2 = JOY_BUTTON_7; // bit 17
		break;
	case 7:
		db1 = JOY_BUTTON_11; // bit 18
		db2 = JOY_BUTTON_10; // bit 19
		break;
	case 8:
		db1 = JOY_BUTTON_8; // bit 20
		db2 = JOY_BUTTON_9; // bit 21
		break;
	}
	if( db1 != DeviceButton_Invalid )
		vPresses.push_back( DeviceInput(m_Id, db1, pressed1 ? 1.0f : 0.0f , now) );
	if( db2 != DeviceButton_Invalid )
		vPresses.push_back( DeviceInput(m_Id, db2, pressed2 ? 1.0f : 0.0f , now) );
}

int PumpDevice::AssignIDs( InputDevice startID )
{
	if( startID < DEVICE_PUMP1 || startID > DEVICE_PUMP2 )
		return -1;
	m_Id = startID;
	return 1;
}

void PumpDevice::GetDevicesAndDescriptions( vector<InputDeviceInfo>& vDevices ) const
{
	vDevices.push_back( InputDeviceInfo(m_Id, "Pump USB") );
}

/*
 * (c) 2006 Steve Checkoway
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
