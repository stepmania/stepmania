#ifndef INPUT_HANDLER_H
#define INPUT_HANDLER_H 1

/*
 * This is a simple class to handle special input devices.  Update()
 * will be called during the input update; the derived class should
 * send appropriate events to InputHandler. 
 *
 * Note that, if the underlying device is capable of it, you're free to
 * start a blocking thread; just store inputs in your class and send them
 * off in a batch on the next Update.  Later on, this method will let us
 * get much more accurate timestamps, especially if we have access to realtime
 * threading: we can get events more quickly and timestamp them, instead of
 * having a rough timing granularity due to the framerate.  We can't do this
 * with SDL, though.
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
	RageTimer m_LastUpdate;

public:
	virtual ~InputHandler() { }
	virtual void Update(float fDeltaTime) { }
	virtual void GetDevicesAndDescriptions(vector<InputDevice>& vDevicesOut, vector<CString>& vDescriptionsOut) = 0;

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
};

#endif

/*
 * Copyright (c) 2003 by the person(s) listed below.  All rights reserved.
 *	Glenn Maynard
 */
