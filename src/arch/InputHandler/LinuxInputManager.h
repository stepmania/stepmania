#ifndef LINUX_INPUT_MANAGER
#define LINUX_INPUT_MANAGER 1

#include <vector>
using namespace std;

#include "global.h"
class InputHandler_Linux_Joystick;
class InputHandler_Linux_Event;

// Enumerates the input devices on the system and dispatches them to 
// IH_Linux_Event and IH_Linux_Joystick as appropriate.

class LinuxInputManager
{
public:
	LinuxInputManager();
	void InitDriver(InputHandler_Linux_Joystick* drv);
	void InitDriver(InputHandler_Linux_Event* drv);
	~LinuxInputManager();
private:
	bool m_bEventEnabled;
	InputHandler_Linux_Event* m_EventDriver;
	vector<RString> m_vsPendingEventDevices;
	
	bool m_bJoystickEnabled;
	InputHandler_Linux_Joystick* m_JoystickDriver;
	vector<RString> m_vsPendingJoystickDevices;
};

extern LinuxInputManager* LINUXINPUT; // global and accessible from anywhere in our program

#endif // LINUX_INPUT_MANAGER

/*
 * (c) 2013 Ben "root" Anderson
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