/* InputHandler_Linux_Event - evdev-based input driver */

#ifndef INPUT_HANDLER_LINUX_EVENT_H
#define INPUT_HANDLER_LINUX_EVENT_H

#include "InputHandler.h"
#include "RageThreads.h"

#include <vector>


class InputHandler_Linux_Event: public InputHandler
{
public:
	InputHandler_Linux_Event();
	~InputHandler_Linux_Event();
	bool TryDevice(RString devfile);
	bool DevicesChanged() { return m_bDevicesChanged; }
	void GetDevicesAndDescriptions( std::vector<InputDeviceInfo>& vDevicesOut );

private:
	void StartThread();
	void StopThread();
	static int InputThread_Start( void *p );
	void InputThread();

	RageThread m_InputThread;
	InputDevice m_NextDevice;
	bool m_bShutdown, m_bDevicesChanged;
};
#define USE_INPUT_HANDLER_LINUX_JOYSTICK

#endif

/*
 * (c) 2003-2008 Glenn Maynard
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
