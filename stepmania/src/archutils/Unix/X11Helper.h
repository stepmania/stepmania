/* Manages our X connection and window. */
#ifndef X11_HELPER_H
#define X11_HELPER_H

#include <X11/Xlib.h>		// Window
namespace X11Helper
{
	// All functions in here that return a bool return true on success, and
	// false on failure.

	// Create the connection, if necessary; otherwise do some important
	// internal session-tracking stuff (so you should call this anyway).
	bool Go();

	// Destroy the connection, if appropriate; otherwise do some important
	// internal session-tracking stuff (so you should call it anyway).
	void Stop();

	// The current Display (connection). Initialized by the first call to
	// Go().
	extern Display *Dpy;

	// The Window used by LowLevelWindow_X11 as the main window.
	extern Window Win;

	// (Re)create the Window win.
	bool MakeWindow( Window &win, int screenNum, int depth, Visual *visual,
			 int width, int height, bool overrideRedirect );
};

#endif

/*
 * (c) 2005, 2006 Ben Anderson, Steve Checkoway
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
