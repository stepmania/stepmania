#include "LowLevelWindow_X11.h"
#include "archutils/Unix/X11Helper.h"
#include "RageException.h"

#include <stack>
#include <math.h>	// ceil()
#include <GL/glx.h>	// All sorts of stuff...

LowLevelWindow_X11::LowLevelWindow_X11()
{
	if(!X11Helper::Go() ) { RageException::Throw("Failed to establish a connection with the X server."); }
}

LowLevelWindow_X11::~LowLevelWindow_X11()
{
	X11Helper::Stop();	// Xlib cleans up the window for us
}

void *LowLevelWindow_X11::GetProcAddress(CString s)
{
	return (void*) glXGetProcAddress( (GLubyte) s.c_str() );
}

CString LowLevelWindow_X11::TryVideoMode(RageDisplay::VideoModeParams p, bool &bNewDeviceOut)
{
	int i;
	int bpppc;	// Bits per pixel per channel
	int visAttribs[11];
	XVisualInfo *xvi;
	XEvent ev;
	XSizeHints hints;
	GLXContext ctxt;
	stack<XEvent> otherEvs;

	// XXX: LLW_SDL allows the window to be resized. Do we really want to?
	hints.flags = PMinSize | PMaxSize | PBaseSize;
	hints.min_width = hints.max_width = hints.base_width = p.width;
	hints.min_height = hints.max_height = hints.base_height = p.height;

	if(!windowIsOpen || p.bpp != CurrentParams.bpp)
	{
		// Different depth, or we didn't make a window before. New context.
		bNewDeviceOut = true;
	
		bpppc = ceil(p.bpp / 4.0);

		visAttribs[0] = GLX_RGBA;
		visAttribs[1] = GLX_DOUBLEBUFFER;
		visAttribs[2] = GLX_RED_SIZE;		visAttribs[3] = bpppc;
		visAttribs[4] = GLX_GREEN_SIZE;		visAttribs[5] = bpppc;
		visAttribs[6] = GLX_BLUE_SIZE;		visAttribs[7] = bpppc;
		visAttribs[8] = GLX_ALPHA_SIZE;		visAttribs[9] = p.bpp
								- (bpppc * 3);
					// Alpha gets however many bits are left...
		visAttribs[10] = None;

		xvi = glXChooseVisual(X11Helper::Dpy(),
				DefaultScreen(X11Helper::Dpy() ), visAttribs);

		if(!vinfo) { return "No visual available for that depth."; }

		X11Helper::OpenMask(StructureNotifyMask);

		if(!X11Helper::MakeWindow(xvi->screen, xvi->depth, xvi->visual,
							p.width, p.height) )
			{ return "Failed to create the window."; }

		ctxt = glXCreateContext(X11Helper::Dpy(), xvi, NULL, True);

		// HACK: Wait for the MapNotify event, without spinning and
		// eating CPU unnecessarily, and without smothering other
		// events. Do this by grabbing all events, remembering
		// uninteresting events, and putting them back on the queue
		// after MapNotify arrives.
		while(true)
		{
			XNextEvent(X11Helper::Dpy(), &ev);
			if(ev.type == MapNotify) { break; }
			else
			{ otherEvs.push(ev); }
		}
		while(!otherEvs.empty() )
		{
			XPutBackEvent(X11Helper::Dpy(), otherEvs.top() );
			otherEvs.pop();
		}

		X11Helper::CloseMask(StructureNotifyMask);

		glXMakeCurrent(X11Helper::Dpy(), X11Helper::Win(), ctxt);

		windowIsOpen = true;
	} else { bNewDeviceOut = false; }

	XSetWMNormalHints(X11Helper::Dpy(), X11Helper::Win(), &hints);

	return ""; // Success
}

void LowLevelWindow_X11::SwapBuffers()
{
	glXSwapBuffers(X11Helper::Dpy(), X11Helper::Win() );
}

void LowLevelWindow_X11::Update()
{
	// XXX: Handle close events, connection lost events, etc.
	return;
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
