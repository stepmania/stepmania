#ifndef INPUTHANDLER_DIRECTINPUT_HELPER_H
#define INPUTHANDLER_DIRECTINPUT_HELPER_H

#include "InputFilter.h"

#include <vector>

#define DIRECTINPUT_VERSION 0x0800
#include <dinput.h>
extern LPDIRECTINPUT8 g_dinput;

#define INPUT_QSIZE	32

struct input_t
{
	// DirectInput offset for this input type:
	DWORD ofs;

	// Button, axis or hat:
	enum Type { KEY, BUTTON, AXIS, HAT } type;

	int num;

	// Comparitor for finding the input_t with the matching ofs member in std containers.
	class Compare
	{
	public:

		Compare(DWORD _ofs) : ofs(_ofs) { }

		bool operator()(const input_t & input) const { return input.ofs == ofs; }

	private:

		DWORD ofs;
	};
};

struct DIDevice
{
	DIDEVICEINSTANCE JoystickInst;
	LPDIRECTINPUTDEVICE8 Device;
	RString m_sName;

	enum { KEYBOARD, JOYSTICK, MOUSE } type;

	bool buffered;
	int buttons, axes, hats;
	std::vector<input_t> Inputs;
	InputDevice dev;

	DIDevice();

	bool Open();
	void Close();
};

struct XIDevice
{
	std::string m_sName;
	DWORD m_dwXInputSlot;
	InputDevice dev;

	XIDevice();
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
