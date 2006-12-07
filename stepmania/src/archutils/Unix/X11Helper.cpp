#include "global.h"
#include "X11Helper.h"

#include <X11/Xlib.h>

#include "RageLog.h"
#include "RageDisplay.h"
#include "RageThreads.h"

// Currently open masks:
static vector<long> g_aiMasks;

// Number of subsystems using the X connection:
static int g_iRefCount = 0;

Display *X11Helper::Dpy = NULL;
Window X11Helper::Win = None;

static int ErrorCallback( Display*, XErrorEvent* );
static int FatalCallback( Display* );

bool X11Helper::Go()
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

void X11Helper::Stop()
{
	g_iRefCount--;

	if( g_iRefCount == 0 )
	{
		XCloseDisplay( Dpy );
		Dpy = NULL;	// For sanity's sake
		g_aiMasks.clear();
	}
}

static void ApplyMasks();

void X11Helper::OpenMask( long mask )
{
	g_aiMasks.push_back(mask);
	ApplyMasks();
}

void X11Helper::CloseMask( long mask )
{
	vector<long>::iterator i = find( g_aiMasks.begin(), g_aiMasks.end(), mask );
	if( i == g_aiMasks.end() )
		return;

	g_aiMasks.erase( i );
	ApplyMasks();
}

static void ApplyMasks()
{
	if( X11Helper::Dpy == NULL || X11Helper::Win == None )
		return;

	LOG->Trace( "X11Helper: Reapplying event masks." );

	long iMask = 0;
	for( unsigned i = 0; i < g_aiMasks.size(); ++i )
		iMask |= g_aiMasks[i];

	XSelectInput( X11Helper::Dpy, X11Helper::Win, iMask );
}

bool X11Helper::MakeWindow( int screenNum, int depth, Visual *visual, int width, int height, bool overrideRedirect )
{
	if( g_iRefCount == 0 )
		return false;

	if( Win )
	{
		XDestroyWindow( Dpy, Win );
		Win = None;
	}
	Win = CreateWindow( screenNum, depth, visual, width, height, overrideRedirect );
	return true;
}

Window X11Helper::CreateWindow( int screenNum, int depth, Visual *visual, int width, int height, bool overrideRedirect )
{
	vector<long>::iterator i;
	
	XSetWindowAttributes winAttribs;

	winAttribs.border_pixel = 0;
	winAttribs.event_mask = 0;
	i = g_aiMasks.begin();
	while( i != g_aiMasks.end() )
	{
		winAttribs.event_mask |= *i;
		i++;
	}

	// XXX: Error catching/handling?

	winAttribs.colormap = XCreateColormap( Dpy, RootWindow(Dpy, screenNum), visual, AllocNone );

	Window Win;
	if( overrideRedirect )
	{
		winAttribs.override_redirect = True;
		Win = XCreateWindow( Dpy, RootWindow(Dpy, screenNum), 0, 0, width,
			height, 0, depth, InputOutput, visual,
			CWBorderPixel | CWColormap | CWEventMask | CWOverrideRedirect, &winAttribs );
	}
	else
	{
		Win = XCreateWindow( Dpy, RootWindow(Dpy, screenNum), 0, 0, width,
			height, 0, depth, InputOutput, visual,
			CWBorderPixel | CWColormap | CWEventMask, &winAttribs );
	}

	/* Hide the mouse cursor. */
	{
		const char pBlank[] = { 0,0,0,0,0,0,0,0 };
		Pixmap BlankBitmap = XCreateBitmapFromData( Dpy, Win, pBlank, 8, 8 );

		XColor black = { 0, 0, 0, 0, 0, 0 };
		Cursor pBlankPointer = XCreatePixmapCursor( Dpy, BlankBitmap, BlankBitmap, &black, &black, 0, 0 );
		XFreePixmap( Dpy, BlankBitmap );

		XDefineCursor( Dpy, Win, pBlankPointer );
		XFreeCursor( Dpy, pBlankPointer );
	}

	return Win;
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
