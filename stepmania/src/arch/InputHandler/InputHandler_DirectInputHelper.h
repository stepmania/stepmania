#ifndef INPUTHANDLER_DIRECTINPUT_HELPER_H
#define INPUTHANDLER_DIRECTINPUT_HELPER_H

#include "InputFilter.h"

#define DIRECTINPUT_VERSION 0x0500
#include <dinput.h>
extern LPDIRECTINPUT dinput;

#define INPUT_QSIZE	32

typedef struct input_t
{
	/* DirectInput offset for this input type: */
	DWORD ofs;

	/* Button, axis or hat: */
	enum Type { KEY, BUTTON, AXIS, HAT } type;

	int num;
} input_t;

struct DIDevice
{
	DIDEVICEINSTANCE JoystickInst;
	LPDIRECTINPUTDEVICE2 Device;

	enum { KEYBOARD, JOYSTICK } type;

	bool buffered;
	int buttons, axes, hats;
	vector<input_t> Inputs;
	InputDevice dev;
	
	DIDevice();

	bool Open();
	void Close();
};

#endif
