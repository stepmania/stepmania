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

/*
 * (c) 2003-2004 Glenn Maynard
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
