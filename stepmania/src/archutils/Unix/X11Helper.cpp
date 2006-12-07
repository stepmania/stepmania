#include "global.h"
#include "X11Helper.h"
#include "RageLog.h"

// Number of subsystems using the X connection:
static int g_iRefCount = 0;

Display *X11Helper::Dpy = NULL;
Window X11Helper::Win = None;

static int ErrorCallback( Display*, XErrorEvent* );
static int FatalCallback( Display* );

bool X11Helper::OpenXConnection()
{
	if( g_iRefCount == 0 )
	{
		Dpy = XOpenDisplay(0);
		if( Dpy == NULL )
			return false;

		XSetIOErrorHandler( FatalCallback );
		XSetErrorHandler( ErrorCallback );
	}
	g_iRefCount++;
	return true;
}

void X11Helper::CloseXConnection()
{
	g_iRefCount--;

	if( g_iRefCount == 0 )
	{
		// The window should have been shut down
		DEBUG_ASSERT( Win == None );
		XCloseDisplay( Dpy );
		Dpy = NULL;	// For sanity's sake
	}
}

bool X11Helper::MakeWindow( Window &win, int screenNum, int depth, Visual *visual, int width, int height, bool overrideRedirect )
{
	if( g_iRefCount == 0 )
		return false;

	XSetWindowAttributes winAttribs;
	winAttribs.border_pixel = 0;
	winAttribs.event_mask = 0L;

	if( win )
	{
		// Preserve the event mask.
		XWindowAttributes attribs;
		XGetWindowAttributes( Dpy, win, &attribs );
		winAttribs.event_mask = attribs.your_event_mask;
		XDestroyWindow( Dpy, win );
	}
	// XXX: Error catching/handling?
	winAttribs.colormap = XCreateColormap( Dpy, RootWindow(Dpy, screenNum), visual, AllocNone );
	unsigned long mask = CWBorderPixel | CWColormap | CWEventMask;

	if( overrideRedirect )
	{
		winAttribs.override_redirect = True;
		mask |= CWOverrideRedirect;
	}
	win = XCreateWindow( Dpy, RootWindow(Dpy, screenNum), 0, 0, width, height, 0,
			     depth, InputOutput, visual, mask, &winAttribs );
	if( win == None )
		return false;

	/* Hide the mouse cursor. */
	{
		const char pBlank[] = { 0,0,0,0,0,0,0,0 };
		Pixmap BlankBitmap = XCreateBitmapFromData( Dpy, win, pBlank, 8, 8 );

		XColor black = { 0, 0, 0, 0, 0, 0 };
		Cursor pBlankPointer = XCreatePixmapCursor( Dpy, BlankBitmap, BlankBitmap, &black, &black, 0, 0 );
		XFreePixmap( Dpy, BlankBitmap );

		XDefineCursor( Dpy, win, pBlankPointer );
		XFreeCursor( Dpy, pBlankPointer );
	}

	return true;
}

int ErrorCallback( Display *d, XErrorEvent *err )
{
	char errText[512];
	XGetErrorText( d,  err->error_code, errText, 512 );
	LOG->Warn( "X11 Protocol error %s (%d) has occurred, caused by request %d,%d, resource ID %d",
		errText, err->error_code, err->request_code, err->minor_code, err->resourceid );

	return 0; // Xlib ignores our return value
}

int FatalCallback( Display *d )
{
	RageException::Throw( "Fatal I/O error communicating with X server." );
}

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
