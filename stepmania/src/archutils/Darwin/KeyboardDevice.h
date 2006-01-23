#ifndef KEYBOARD_DEVICE_H
#define KEYBOARD_DEVICE_H

#include "HIDDevice.h"
#include <ext/hash_map>

class KeyboardDevice : public HIDDevice
{
private:
	__gnu_cxx::hash_map<int, DeviceButton> m_Mapping;
	
protected:
	bool AddLogicalDevice( int usagePage, int usage );
	void AddElement( int usagePage, int usage, int cookie, const CFDictionaryRef dict );
	void Open();

public:
	void GetButtonPresses( vector<pair<DeviceInput, bool> >& vPresses, int cookie,
			       int value, const RageTimer& now ) const;
	void GetDevicesAndDescriptions( vector<InputDevice>& dev, vector<RString>& desc ) const;
};


#endif
