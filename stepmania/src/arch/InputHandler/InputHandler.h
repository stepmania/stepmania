#ifndef INPUT_HANDLER_H
#define INPUT_HANDLER_H 1

/*
 * This is a simple class to handle special input devices.  Update()
 * will be called during the input update; the derived class should
 * send appropriate events to InputHandler. 
 *
 * Send input events for a specific type of device.  Only one driver
 * for a given set of InputDevice types should be loaded for a given
 * arch.  For example, any number of drivers may produce DEVICE_PUMPn
 * events, but only one may be loaded at a time.
 *
 * DEVICE_KEYBOARD and DEVICE_JOYn are fed from SDL and should not
 * be used by RageInputFeeders.
 */

class InputHandler
{
public:
	virtual void Update(float fDeltaTime) { }
	virtual ~InputHandler() { }
};

#endif

/*
 * Copyright (c) 2003 by the person(s) listed below.  All rights reserved.
 *	Glenn Maynard
 */
