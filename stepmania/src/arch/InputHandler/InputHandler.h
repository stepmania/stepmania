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
