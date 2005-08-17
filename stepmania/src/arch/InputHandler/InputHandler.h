#ifndef INPUT_HANDLER_H
#define INPUT_HANDLER_H

/*
 * This is a simple class to handle special input devices.  Update()
 * will be called during the input update; the derived class should
 * send appropriate events to InputHandler. 
 *
 * Note that, if the underlying device is capable of it, you're free to
 * start a blocking thread; just store inputs in your class and send them
 * off in a batch on the next Update.  This gets much more accurate timestamps;
 * we get events more quickly and timestamp them, instead of having a rough timing
 * granularity due to the framerate.
 *
 * Send input events for a specific type of device.  Only one driver
 * for a given set of InputDevice types should be loaded for a given
 * arch.  For example, any number of drivers may produce DEVICE_PUMPn
 * events, but only one may be loaded at a time.  (This will be inconvenient
 * if, for example, we have two completely distinct methods of getting
 * input for the same device; we have no method to allocate device numbers.
 * We don't need this now; I'll write it if it becomes needed.)
 */
#include "RageInputDevice.h"	// for InputDevice

class InputHandler
{
public:
	InputHandler() { m_iInputsSinceUpdate = 0; }
	virtual ~InputHandler() { }
	virtual void Update( float fDeltaTime ) { }
	virtual void GetDevicesAndDescriptions( vector<InputDevice>& vDevicesOut, vector<CString>& vDescriptionsOut ) = 0;
	
	// override to return a pretty string that's specific to the controller type
	virtual CString GetDeviceSpecificInputString( const DeviceInput &di ) { return di.toString(); }

	/* In Windows, some devices need to be recreated if we recreate our main window.
	 * Override this if you need to do that. */
	virtual void WindowReset() { }

protected:
	/* Convenience function: Call this to queue a received event.  This may be called
	 * in a thread. 
	 *
	 * Important detail: If the timestamp, di.ts, is zero, then it is assumed that
	 * this is not a threaded event handler.  In that case, input is being polled,
	 * and the actual time the button was pressed may be any time since the last
	 * poll.  In this case, ButtonPressed will pretend the button was pressed at
	 * the midpoint since the last update, which will smooth out the error.
	 *
	 * Note that timestamps are set to the current time by default, so for this to
	 * happen, you need to explicitly call di.ts.SetZero(). 
	 *
	 * If the timestamp is set, it'll be left alone. */
	void ButtonPressed( DeviceInput di, bool Down );

	/* Call this at the end of polling input. */
	void UpdateTimer();

private:
	RageTimer m_LastUpdate;
	int m_iInputsSinceUpdate;
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
