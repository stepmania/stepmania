#ifndef JOYSTICK_DEVICE_H
#define JOYSTICK_DEVICE_H

#include "HIDDevice.h"
#include <ext/hash_map>

struct Joystick
{
	InputDevice id;
	// map cookie to button
	__gnu_cxx::hash_map<int, DeviceButton> mapping;
	int x_axis, y_axis, z_axis;
	int x_min, y_min, z_min;
	int x_max, y_max, z_max;
	
	Joystick();
};

class JoystickDevice : public HIDDevice
{
private:
	vector<Joystick> m_vSticks;
	
protected:
	bool AddLogicalDevice( int usagePage, int usage );
	void AddElement( int usagePage, int usage, int cookie, const CFDictionaryRef dict );
	void Open();
	
public:
	void GetButtonPresses( vector<pair<DeviceInput, bool> >& vPresses, int cookie,
			       int value, const RageTimer& now ) const;
	int AssignIDs( InputDevice startID );
	void GetDevicesAndDescriptions( vector<InputDevice>& dev, vector<RString>& desc ) const;
};


#endif
