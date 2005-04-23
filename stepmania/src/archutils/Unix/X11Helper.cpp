#include "global.h"
#include "X11Helper.h"

#include <X11/Xlib.h>		// Display, Window

#include "RageLog.h"
#include "RageDisplay.h"
#include "RageThreads.h"

static vector<long>		pMasks;				// Currently open masks
static unsigned short int	pCt		= 0;		// Number of subsystems
							// using the X connection

Display *X11Helper::Dpy = NULL;
Window X11Helper::Win;

int protoErrorCallback(Display*, XErrorEvent*);
int protoFatalCallback(Display*);

bool X11Helper::Go()
{
	int i;

	if(pCt == 0)
	{
		Dpy = XOpenDisplay(0);
		if(Dpy == NULL) { return false; }

		XSetIOErrorHandler(&protoFatalCallback);
		XSetErrorHandler(&protoErrorCallback);
	}
	pCt++;

	return true;
}

void X11Helper::Stop()
{
	pCt--;

	if(pCt == 0)
	{
		XCloseDisplay(Dpy);
		Dpy = NULL;	// For sanity's sake
		pMasks.clear();
	}
}

static bool pApplyMasks();

bool X11Helper::OpenMask(long mask)
{
	pMasks.push_back(mask);
	return pApplyMasks();
}

bool X11Helper::CloseMask(long mask)
{
	vector<long>::iterator i;
	
	i = pMasks.begin();
	while(true)
	{
		if(*i == mask)
		{
			pMasks.erase(i);
			break;
		}
		if(i == pMasks.end() ) { return true; }	// Never set in the
							// first place...
		i++;
	}

	return pApplyMasks();
}

static bool pApplyMasks()
{
	if(X11Helper::Dpy != NULL)
	{
		long finalMask = 0;

		LOG->Trace("X11Helper: Reapplying event masks.");

		unsigned int i = 0;
		while(i < pMasks.size() )
		{
			finalMask |= pMasks[i];
			i++;
		}

		if(XSelectInput(X11Helper::Dpy, X11Helper::Win, finalMask) == 0) { return false; }
	}

	return true;
}

bool X11Helper::MakeWindow(int screenNum, int depth, Visual *visual, int width, int height)
{
	static bool pHaveWin = false;
	vector<long>::iterator i;
	
	if(pCt == 0) { return false; }

	if(pHaveWin) { XDestroyWindow(Dpy, Win); pHaveWin = false; }
		// pHaveWin will stay false if an error occurs once I do error
		// checking here...

	XSetWindowAttributes winAttribs;

	winAttribs.border_pixel = 0;
	winAttribs.event_mask = 0;
	i = pMasks.begin();
	while(i != pMasks.end() )
	{
		winAttribs.event_mask |= *i;
		i++;
	}

	// XXX: Error catching/handling?

	winAttribs.colormap = XCreateColormap(Dpy, RootWindow(Dpy, screenNum),
							visual, AllocNone);

	Win = XCreateWindow(Dpy, RootWindow(Dpy, screenNum), 0, 0, width,
		height, 0, depth, InputOutput, visual,
		CWBorderPixel | CWColormap | CWEventMask, &winAttribs);

	pHaveWin = true;

	return true;
}

int protoErrorCallback(Display *d, XErrorEvent *err)
{
	char errText[512];
	XGetErrorText(d,  err->error_code, errText, 512);
	LOG->Warn("X11 Protocol error %s (%d) has occurred, caused by request %d,%d, resource ID %d",
		errText, err->error_code, err->request_code, err->minor_code, err->resourceid);

	return 0; // Xlib ignores our return value
}

int protoFatalCallback(Display *d)
{
	RageException::Throw("Fatal I/O error communicating with X server.");
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
