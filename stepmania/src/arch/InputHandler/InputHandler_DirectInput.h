#ifndef INPUTHANDLER_DIRECTINPUT_H
#define INPUTHANDLER_DIRECTINPUT_H

#include "InputHandler.h"

struct DIDevice;
class InputHandler_DInput: public InputHandler
{
	bool OpenDevice(DIDevice &joystick);

public:
	InputHandler_DInput();
	~InputHandler_DInput();
	void GetDevicesAndDescriptions(vector<InputDevice>& vDevicesOut, vector<CString>& vDescriptionsOut);
	void Update(float fDeltaTime);
};

#endif
