#include "X11Helper.h"

#include <X11/Xlib.h>		// Display, Window

#include "RageDisplay.h"	// RageDisplay

vector<Callback_t> pCBacks;	// Callbacks for the rendering thread
Display *pDpy;			// Running X connection
Window *pWin;			// Current window
unsigned short int pCt = 0;	// Number of subsystems using the X connection

bool X11Helper::Go()
{
	if(pCt == 0)
	{
		pDpy = XOpenDisplay(0);
		if(pDpy == NULL) { return false; }
	}
	pCt++;

	return true;
}

Display *X11Helper::Dpy()
{
	return pDpy;
}

bool X11Helper::MakeWindow(int screenNum, int depth, Visual *visual int width=64, int height=64)
{
	if(dpy == NULL) { return false; }

	XSetWindowAttributes winAttribs;

	winAttribs.border_pixel = 0;
	winAttribs.event_mask = StructureNotifyMask;

	winAttribs.colormap = XCreateColorMap(pDpy, RootWindow(pDpy, screenNum),
							visual, AllocNone);

	pWin = XCreateWindow(pDpy, RootWindow(pDpy, screenNum), 0, 0, width, height
}

bool X11Helper::Callback(Callback_t cb)
{
	pCBacks.push_back(cb);

	return true;
}

void X11Helper::Stop()
{
	pCt--;

	if(pCt == 0)
	{
		XCloseDisplay(pDpy);
	}
}

/*
 * (c) 2005 Ben Anderson
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
