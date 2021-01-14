#ifndef JOYSTICK_DEVICE_H
#define JOYSTICK_DEVICE_H

#include "HIDDevice.h"

struct Joystick
{
	InputDevice id;
	// map cookie to button
	std::unordered_map<IOHIDElementCookie, DeviceButton> mapping;
	IOHIDElementCookie x_axis, y_axis, z_axis, x_rot, y_rot, z_rot, hat;
	int x_min, x_max;
	int y_min, y_max;
	int z_min, z_max;
	int rx_min, rx_max;
	int ry_min, ry_max;
	int rz_min, rz_max;
	int hat_min, hat_max;

	Joystick();
};

class JoystickDevice : public HIDDevice
{
private:
	vector<Joystick> m_vSticks;

protected:
	bool AddLogicalDevice( int usagePage, int usage );
	void AddElement( int usagePage, int usage, IOHIDElementCookie cookie, const CFDictionaryRef properties );
	void Open();
	bool InitDevice( int vid, int pid );

public:
	void GetButtonPresses( vector<DeviceInput>& vPresses, IOHIDElementCookie cookie, int value, const RageTimer& now ) const;
	int AssignIDs( InputDevice startID );
	void GetDevicesAndDescriptions( vector<InputDeviceInfo>& vDevices ) const;
};

#endif

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
