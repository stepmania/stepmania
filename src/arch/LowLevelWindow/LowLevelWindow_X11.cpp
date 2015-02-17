#include "global.h"
#include "LowLevelWindow_X11.h"
#include "RageLog.h"
#include "RageException.h"
#include "archutils/Unix/X11Helper.h"
#include "PrefsManager.h" // XXX
#include "RageDisplay.h" // VideoModeParams
#include "DisplayResolutions.h"
#include "LocalizedString.h"

#include "RageDisplay_OGL_Helpers.h"
using namespace RageDisplay_Legacy_Helpers;
using namespace X11Helper;

#include <stack>
#include <math.h>	// ceil()
#include <GL/glxew.h>
#define GLX_GLXEXT_PROTOTYPES
#include <GL/glx.h>	// All sorts of stuff...
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#ifdef HAVE_XRANDR
#include <X11/extensions/Xrandr.h>
#endif
#ifdef HAVE_XF86VIDMODE
#include <X11/extensions/xf86vmode.h>
#endif

#if defined(HAVE_LIBXTST)
#include <X11/extensions/XTest.h>
#endif

static GLXContext g_pContext = NULL;
static GLXContext g_pBackgroundContext = NULL;
static Window g_AltWindow = None;
#ifdef HAVE_XRANDR
RRMode g_originalRandRMode;
RROutput g_usedCrtc;
bool g_bUseXRandR = false;
int g_iRandRVerMinor;
int g_iRandRVerMajor;
#endif
#ifdef HAVE_XF86VIDMODE
XF86VidModeModeInfo g_originalVidModeMode;
#endif

inline float calcRandRRefresh( int iPixelClock, int iHTotal, int iVTotal )
{
	// Pixel Clock divided by total pixels in mode,
	// not just those onscreen!
	return ( iPixelClock ) / ( iHTotal * iVTotal );
}

inline float calcVidModeRefresh( int iPixelClock, int hTotal, int vTotal )
{
	// the pixel clock as returned by XF86VM is in kHz.
	// We want Hz.
	return calcRandRRefresh( iPixelClock * 1000.0, hTotal, vTotal );
}

static LocalizedString FAILED_CONNECTION_XSERVER( "LowLevelWindow_X11", "Failed to establish a connection with the X server" );
LowLevelWindow_X11::LowLevelWindow_X11()
{
	if( !OpenXConnection() )
		RageException::Throw( "%s", FAILED_CONNECTION_XSERVER.GetValue().c_str() );

#ifdef HAVE_XRANDR
	if( XRRQueryVersion( Dpy, &g_iRandRVerMajor, &g_iRandRVerMinor ) && g_iRandRVerMajor >= 1 && g_iRandRVerMinor >= 2) g_bUseXRandR = true;
#endif
#ifndef HAVE_XF86VIDMODE
	ASSERT(g_bUseXRandR == true, "XRandR not present or too old.");
#endif

	const int iScreen = DefaultScreen( Dpy );
	int iXServerVersion = XVendorRelease( Dpy ); /* eg. 40201001 */
	int iMajor = iXServerVersion / 10000000; iXServerVersion %= 10000000;
	int iMinor = iXServerVersion / 100000;   iXServerVersion %= 100000;
	int iRevision = iXServerVersion / 1000;  iXServerVersion %= 1000;
	int iPatch = iXServerVersion;

	LOG->Info( "Display: %s (screen %i)", DisplayString(Dpy), iScreen );
	LOG->Info( "X server vendor: %s [%i.%i.%i.%i]", XServerVendor( Dpy ), iMajor, iMinor, iRevision, iPatch );
	LOG->Info( "Server GLX vendor: %s [%s]", glXQueryServerString( Dpy, iScreen, GLX_VENDOR ), glXQueryServerString( Dpy, iScreen, GLX_VERSION ) );
	LOG->Info( "Client GLX vendor: %s [%s]", glXGetClientString( Dpy, GLX_VENDOR ), glXGetClientString( Dpy, GLX_VERSION ) );
	m_bWasWindowed = true;
}

LowLevelWindow_X11::~LowLevelWindow_X11()
{
	// Reset the display
	if( !m_bWasWindowed )
	{
#ifdef HAVE_XRANDR
		if(g_bUseXRandR)
		{
			XRRScreenResources *res = XRRGetScreenResources(Dpy, Win);
			XRRCrtcInfo *conf = XRRGetCrtcInfo(Dpy, res, g_usedCrtc);
			XRRSetCrtcConfig(Dpy, res, g_usedCrtc, conf->timestamp, conf->x, conf->y, g_originalRandRMode, conf->rotation, conf->outputs, conf->noutput);

			XRRFreeScreenResources(res);
			XRRFreeCrtcInfo(conf);
		}
		else
#ifndef HAVE_XF86VIDMODE
			FAIL_M("XRandR not present or too old.");
#endif
#endif
#ifdef HAVE_XF86VIDMODE
			XF86VidModeSwitchToMode( Dpy, DefaultScreen( Dpy ), &g_originalVidModeMode );
#endif

		XUngrabKeyboard( Dpy, CurrentTime );
	}
	if( g_pContext )
	{
		glXDestroyContext( Dpy, g_pContext );
		g_pContext = NULL;
	}
	if( g_pBackgroundContext )
	{
		glXDestroyContext( Dpy, g_pBackgroundContext );
		g_pBackgroundContext = NULL;
	}
	// We're supposed to XFree() the private bits of XF86VidModeModeInfo
	// structs. This isn't actually possible from C++. At all. Period.
	// Let it leak.

	XDestroyWindow( Dpy, Win );
	Win = None;
	XDestroyWindow( Dpy, g_AltWindow );
	g_AltWindow = None;
	CloseXConnection();
}

void *LowLevelWindow_X11::GetProcAddress( RString s )
{
	// XXX: We should check whether glXGetProcAddress or
	// glXGetProcAddressARB is available/not NULL, and go by that,
	// instead of assuming like this.
	return (void*) glXGetProcAddressARB( (const GLubyte*) s.c_str() );
}

RString LowLevelWindow_X11::TryVideoMode( const VideoModeParams &p, bool &bNewDeviceOut )
{
	XWindowAttributes winAttrib;

	if( g_pContext == NULL || p.bpp != CurrentParams.bpp || m_bWasWindowed != p.windowed )
	{
		bool bFirstRun = g_pContext == NULL;
		// Different depth, or we didn't make a window before. New context.
		bNewDeviceOut = true;

		int visAttribs[32];
		int i = 0;
		ASSERT( p.bpp == 16 || p.bpp == 32 );

		if( p.bpp == 32 )
		{
			visAttribs[i++] = GLX_RED_SIZE;	visAttribs[i++] = 8;
			visAttribs[i++] = GLX_GREEN_SIZE;	visAttribs[i++] = 8;
			visAttribs[i++] = GLX_BLUE_SIZE;	visAttribs[i++] = 8;
		}
		else
		{
			visAttribs[i++] = GLX_RED_SIZE;	visAttribs[i++] = 5;
			visAttribs[i++] = GLX_GREEN_SIZE;	visAttribs[i++] = 6;
			visAttribs[i++] = GLX_BLUE_SIZE;	visAttribs[i++] = 5;
		}

		visAttribs[i++] = GLX_DEPTH_SIZE;	visAttribs[i++] = 16;
		visAttribs[i++] = GLX_RGBA;
		visAttribs[i++] = GLX_DOUBLEBUFFER;

		visAttribs[i++] = None;

		XVisualInfo *xvi = glXChooseVisual( Dpy, DefaultScreen(Dpy), visAttribs );
		if( xvi == NULL )
			return "No visual available for that depth.";

		// I get strange behavior if I add override redirect after creating the window.
		// So, let's recreate the window when changing that state.
		if( !MakeWindow(Win, xvi->screen, xvi->depth, xvi->visual, p.width, p.height, !p.windowed) )
			return "Failed to create the window.";

		if( !MakeWindow(g_AltWindow, xvi->screen, xvi->depth, xvi->visual, p.width, p.height, !p.windowed) )
			FAIL_M( "Failed to create the alt window." ); // Should this be fatal?

		char *szWindowTitle = const_cast<char *>( p.sWindowTitle.c_str() );
		XChangeProperty( Dpy, Win, XA_WM_NAME, XA_STRING, 8, PropModeReplace,
				reinterpret_cast<unsigned char*>(szWindowTitle), strlen(szWindowTitle) );

		if( g_pContext )
			glXDestroyContext( Dpy, g_pContext );
		if( g_pBackgroundContext )
			glXDestroyContext( Dpy, g_pBackgroundContext );
		g_pContext = glXCreateContext( Dpy, xvi, NULL, True );
		g_pBackgroundContext = glXCreateContext( Dpy, xvi, g_pContext, True );

		glXMakeCurrent( Dpy, Win, g_pContext );

		// Map the window, ensuring we get the MapNotify event
		XGetWindowAttributes( Dpy, Win, &winAttrib );
		XSelectInput( Dpy, Win, winAttrib.your_event_mask | StructureNotifyMask );
		XMapWindow( Dpy, Win );

		// We can wait for the MapNotify later. We've got work to do!

		// I can't find official docs saying what happens if you re-init GLEW.
		// I'll just assume the behavior is undefined.
		if(bFirstRun)
		{
			GLenum err = glewInit();
			ASSERT( err == GLEW_OK );
		}
	}
	else
	{
		// We're remodeling the existing window, and not touching the context.
		bNewDeviceOut = false;

		if( !p.windowed )
		{
			// X11 is an asynchronous beast. If we're resizing an existing
			// window directly (i.e. override-redirect as opposed to asking the
			// WM to do it) and don't wait for the window to actually be
			// resized, we'll get unexpected results from glViewport() etc. I
			// don't know why, or why it *doesn't* break in the slower process
			// of waiting for the WM to resize the window.

			// So, set the event mask so we're notified when the window is resized...
			XGetWindowAttributes( Dpy, Win, &winAttrib );
			XSelectInput( Dpy, Win, winAttrib.your_event_mask | StructureNotifyMask );

			// Send the resize command...
			XResizeWindow( Dpy, Win, p.width, p.height );

			// We'll wait for the notification once we've done everything else,
			// to save time.
		}
	}

	float rate = 60; // Will be unchanged if windowed. Not sure I care.

	if( !p.windowed )
	{
		/* === Changing Resolution === */
#ifdef HAVE_XRANDR
		// Arcane and undocumented but PROPER XRandR 1.2 method.
		// What we do is directly reconfigure the CRTC of the primary display,
		// Which prevents the (RandR) screen itself from resizing, and therefore
		// leaving user's desktop unmolested.
		if(g_bUseXRandR)
		{
			// XRandR is present and at least 1.2. Continue.
			LOG->Info("LowLevelWindow_X11: Using XRandR");

			XRRScreenResources *scrRes = XRRGetScreenResources(Dpy, Win);
			ASSERT(scrRes != NULL);
			ASSERT(scrRes->ncrtc > 0);
			ASSERT(scrRes->noutput > 0);
			ASSERT(scrRes->nmode > 0);

			RROutput primaryOut;
			if(g_iRandRVerMajor >= 1 && g_iRandRVerMinor >= 3)
				// RandR 1.3 can tell us what the primary display is.
				primaryOut = XRRGetOutputPrimary(Dpy, Win);
			else // Only RandR 1.2. We have to guess.
				primaryOut = scrRes->outputs[0];
			
			XRROutputInfo *outInfo = XRRGetOutputInfo(Dpy, scrRes, primaryOut);
			ASSERT(outInfo->ncrtc > 0);
			XRRCrtcInfo *oldConf = XRRGetCrtcInfo( Dpy, scrRes, outInfo->crtcs[0] );
			
			float fRefreshDiff = 99999;
			float fRefreshRate = 0;
			RRMode mode;
			// A quirk of XRandR is that the width and height are as the display
			// controller ("CRTC") sees it, which means height and width are
			// flipped if there's rotation going on.
			bool bPortrait = oldConf->rotation & ( RR_Rotate_90 | RR_Rotate_270 );
#define REAL_WIDTH(x)( bPortrait ? x.height : x.width )
#define REAL_HEIGHT(x) ( bPortrait ? x.width : x.height )
			
			// Find a mode that matches our exact wanted resolution,
			// with as close to our desired refresh rate as possible.
			for(int i = 0; i < scrRes->nmode; i++)
				if(REAL_WIDTH(scrRes->modes[i]) == p.width && REAL_HEIGHT(scrRes->modes[i]) == p.height)
				{
					XRRModeInfo *thisMI = &scrRes->modes[i];
					float fTempRefresh = calcRandRRefresh( thisMI->dotClock, thisMI->hTotal, thisMI->vTotal );
					float fTempDiff = abs( fRefreshRate - fTempRefresh );
					if(fTempDiff < fRefreshDiff)
					{
						int j;
						// Ensure that the output supports the mode
						for(j = 0; j < outInfo->nmode; j++)
							if(outInfo->modes[j] == scrRes->modes[i].id)
							{
								mode = outInfo->modes[j];
								break;
							}

						if(j < outInfo->nmode)
						{
							fRefreshRate = fTempRefresh;
							fRefreshDiff = fTempDiff;
						}
					}
				}

#undef REAL_WIDTH
#undef REAL_HEIGHT
			rate = roundf(fRefreshRate);
			
			ASSERT(fRefreshRate > 0);

			g_usedCrtc = outInfo->crtcs[0];
			if(m_bWasWindowed)
				// Save the old mode to restore later.
				g_originalRandRMode = oldConf->mode;

			// and FIRE!
			XRRSetCrtcConfig(Dpy, scrRes, g_usedCrtc, oldConf->timestamp, oldConf->x, oldConf->y, mode, oldConf->rotation, oldConf->outputs, oldConf->noutput);
			
			// We don't move to absolute 0,0 because that may be in the area of a different output.
			// Instead we preserved the corner of our CRTC; go to that.
			XMoveWindow(Dpy, Win, oldConf->x, oldConf->y);

			// Final cleanup
			XRRFreeScreenResources(scrRes);
			XRRFreeOutputInfo(outInfo);
			XRRFreeCrtcInfo(oldConf);
		}
		else
#ifndef HAVE_XF86VIDMODE
			FAIL_M("XRandR extension not present or too old.");
#endif
#endif // HAVE_XRANDR
#ifdef HAVE_XF86VIDMODE
		{
			// Legacy probably-deprecated XFree86-VidModeExtension method.
			// Some X servers will resize the root window in response to this.
			// Some won't. We hope for the latter. Hope is all we got.
			if( m_bWasWindowed )
			{
				// We're supposed to XFree() the private bits of XF86VidModeModeInfo
				// structs. This isn't actually possible from C++. At all. Period.
				// Let it leak.

				// XFree86-VideoMode can't make up its mind what struct it wants to
				// use. This function returns an XF86VidModeModeLine when almost
				// everything else wants XF86VidModeModeInfo. And there's no
				// conversion function. Our only option is to do the conversion
				// ourselves and assume that the private bytes don't differ
				// between the two.

				XF86VidModeModeLine tempMode;
				int iPixelClock;

				XF86VidModeGetModeLine( Dpy, DefaultScreen(Dpy), &iPixelClock, &tempMode );

				g_originalVidModeMode.dotclock = iPixelClock;
				g_originalVidModeMode.hdisplay = tempMode.hdisplay;
				g_originalVidModeMode.hsyncstart = tempMode.hsyncstart;
				g_originalVidModeMode.hsyncend = tempMode.hsyncend;
				g_originalVidModeMode.htotal = tempMode.htotal;
				g_originalVidModeMode.vdisplay = tempMode.vdisplay;
				g_originalVidModeMode.vsyncstart = tempMode.vsyncstart;
				g_originalVidModeMode.vsyncend = tempMode.vsyncend;
				g_originalVidModeMode.vtotal = tempMode.vtotal;
				g_originalVidModeMode.flags = tempMode.flags;
				// Once again, we can't actually address XF86VidModeModeInfo::private.
				// I hope the server doesn't need its private bits, because it's
				// not getting 'em.
				g_originalVidModeMode.privsize = 0;
			}

			// Find a matching mode.
			int iNumModes;
			XF86VidModeModeInfo **aModes_;
			XF86VidModeGetAllModeLines( Dpy, DefaultScreen(Dpy), &iNumModes, &aModes_ );
			ASSERT_M( iNumModes > 0, "Couldn't get resolution list from X server" );
			XF86VidModeModeInfo *aModes = *aModes_;

			int iSizeMatch = -1;
			float fRefreshCloseness;

			for( int i = 0; i < iNumModes; ++i )
			{
				if( aModes[i].hdisplay == p.width && aModes[i].vdisplay == p.height )
				{
					// We're not told the refresh rate; we're expected to calculate
					// it ourselves.
					float fCheckRefresh = calcVidModeRefresh( aModes[i].dotclock, aModes[i].htotal, aModes[i].vtotal );

					// And of course it's rarely if ever an exact integer. Just get the nearest match.
					float fTempClose = fabs( p.rate - rate );
					if( iSizeMatch == -1 || fTempClose < fRefreshCloseness )
					{
						rate = fCheckRefresh;
						fRefreshCloseness = fTempClose;
						iSizeMatch = i;
					}
				}
			}
		
			ASSERT( iSizeMatch != -1 );

			// Set this mode.
			// XXX How do we detect failure?
			XF86VidModeSwitchToMode( Dpy, DefaultScreen(Dpy), &( aModes[iSizeMatch] ) );

			// Move the viewport to the corner where we are.
			XF86VidModeSetViewPort( Dpy, DefaultScreen(Dpy), 0, 0);

			// We're supposed to XFree() the private bits of XF86VidModeModeInfo
			// structs. This isn't actually possible from C++. At all. Period.
			// Let it leak.

			XFree( aModes_ );
		}
#endif // HAVE_XF86VIDMODE

		m_bWasWindowed = false;

		XRaiseWindow( Dpy, Win );

		// We want to prevent the WM from catching anything that comes from the keyboard.
		// We should do this every time on fullscreen and not only we entering from windowed mode because we could lose focus at resolution change and that will leave the user input locked.
		XGrabKeyboard( Dpy, Win, True, GrabModeAsync, GrabModeAsync, CurrentTime );
	}
	else // if(p.windowed)
	{
		if( !m_bWasWindowed )
		{
			// Return the display to the mode it was in before we fullscreened.
#ifdef HAVE_XRANDR
			if(g_bUseXRandR)
			{
				XRRScreenResources *res = XRRGetScreenResources(Dpy, Win);
				XRRCrtcInfo *conf = XRRGetCrtcInfo(Dpy, res, g_usedCrtc);
				XRRSetCrtcConfig(Dpy, res, g_usedCrtc, conf->timestamp, conf->x, conf->y, g_originalRandRMode, conf->rotation, conf->outputs, conf->noutput);

				XRRFreeScreenResources(res);
				XRRFreeCrtcInfo(conf);
			}
			else
#ifndef HAVE_XF86VIDMODE
				FAIL_M("XRandR not present or too old.");
#endif
#endif
#ifdef HAVE_XF86VIDMODE
				XF86VidModeSwitchToMode( Dpy, DefaultScreen(Dpy), &g_originalVidModeMode );
#endif
			// In windowed mode, we actually want the WM to function normally.
			// Release any previous grab.
			XUngrabKeyboard( Dpy, CurrentTime );
			m_bWasWindowed = true;
		}
	}

	// Make a window fixed size, don't let resize it or maximize it.
	// Do this before resizing the window so that pane-style WMs (Ion,
	// ratpoison) don't resize us back inappropriately.
	{
		XSizeHints hints;

		hints.flags = PMinSize|PMaxSize|PWinGravity;
		hints.min_width = hints.max_width = p.width;
		hints.min_height = hints.max_height = p.height;
		hints.win_gravity = CenterGravity;

		XSetWMNormalHints( Dpy, Win, &hints );
	}

	/* Workaround for metacity and compiz: if the window have the same
	 * resolution or higher than the screen, it gets automaximized even
	 * when the window is set to not let it happen. This happens when
	 * changing from fullscreen to window mode and our screen resolution
	 * is bigger. */
	{
		XEvent xev;
		Atom wm_state = XInternAtom(Dpy, "_NET_WM_STATE", False);
		Atom maximized_vert = XInternAtom(Dpy, "_NET_WM_STATE_MAXIMIZED_VERT", False);
		Atom maximized_horz = XInternAtom(Dpy, "_NET_WM_STATE_MAXIMIZED_HORZ", False);

		memset(&xev, 0, sizeof(xev));
		xev.type = ClientMessage;
		xev.xclient.window = Win;
		xev.xclient.message_type = wm_state;
		xev.xclient.format = 32;
		xev.xclient.data.l[0] = 1;
		xev.xclient.data.l[1] = maximized_vert;
		xev.xclient.data.l[2] = 0;

		XSendEvent(Dpy, DefaultRootWindow(Dpy), False, SubstructureNotifyMask, &xev);
		xev.xclient.data.l[1] = maximized_horz;
		XSendEvent(Dpy, DefaultRootWindow(Dpy), False, SubstructureNotifyMask, &xev);

		// This one is needed for compiz, if the window reaches out of bounds of the screen it becames destroyed, only the window, the program is left running.
		// Commented out per the patch at http://ssc.ajworld.net/sm-ssc/bugtracker/view.php?id=398
		//XMoveWindow( Dpy, Win, 0, 0 );
	}

	CurrentParams = p;
	ASSERT( rate > 0 );
	CurrentParams.rate = roundf(rate);

	// Wait for window to be ready for drawing, before setting v-sync hint.
	// Just in case the GLX impl is sensitive to it.
	if( bNewDeviceOut == true )
	{
		XEvent ev;
		do {
			XMaskEvent(Dpy, StructureNotifyMask, &ev);
		} while(ev.type != MapNotify);

		// And set the mask back to what it was.
		XSelectInput( Dpy, Win, winAttrib.your_event_mask );
	}
	else if( !p.windowed )
	{
		XEvent ev;
		do {
			XMaskEvent(Dpy, StructureNotifyMask, &ev);
		} while(ev.type != ConfigureNotify);

		// And set the mask back to what it was.
		XSelectInput( Dpy, Win, winAttrib.your_event_mask );
	}

	// Set our V-sync hint.
	if(GLXEW_EXT_swap_control) // I haven't seen this actually implemented yet, but why not.
		glXSwapIntervalEXT( Dpy, Win, CurrentParams.vsync ? 1 : 0 );
	// XXX: These two might be server-global. I should look into whether
	// to try to preserve the original value on exit.
#ifdef GLXEW_MESA_swap_control // Added in 1.7. 1.6 is still common out there apparently.
	else if(GLXEW_MESA_swap_control) // Haven't seen this NOT implemented yet
		glXSwapIntervalMESA( CurrentParams.vsync ? 1 : 0 );
#endif
	else if(GLXEW_SGI_swap_control) // But old GLEW.
		glXSwapIntervalSGI( CurrentParams.vsync ? 1 : 0 );
	else
		CurrentParams.vsync = false; // Assuming it's not on

	return ""; // Success
}

void LowLevelWindow_X11::LogDebugInformation() const
{
	LOG->Info( "Direct rendering: %s", glXIsDirect( Dpy, glXGetCurrentContext() )? "yes":"no" );
}

bool LowLevelWindow_X11::IsSoftwareRenderer( RString &sError )
{
	if( glXIsDirect( Dpy, glXGetCurrentContext() ) )
		return false;

	sError = "Direct rendering is not available.";
	return true;
}

void LowLevelWindow_X11::SwapBuffers()
{
	glXSwapBuffers( Dpy, Win );

	if( PREFSMAN->m_bDisableScreenSaver )
	{
		// Disable the screensaver.
#if defined(HAVE_LIBXTST)
		// This causes flicker.
		// XForceScreenSaver( Dpy, ScreenSaverReset );

		/* Instead, send a null relative mouse motion, to trick X into thinking
		 * there has been user activity.
		 *
		 * This also handles XScreenSaver; XForceScreenSaver only handles the
		 * internal X11 screen blanker.
		 *
		 * This will delay the X blanker, DPMS and XScreenSaver from activating,
		 * and will disable the blanker and XScreenSaver if they're already active
		 * (unless XSS is locked). For some reason, it doesn't un-blank DPMS if
		 * it's already active.
		 */

		XLockDisplay( Dpy );

		int event_base, error_base, major, minor;
		if( XTestQueryExtension( Dpy, &event_base, &error_base, &major, &minor ) )
		{
			XTestFakeRelativeMotionEvent( Dpy, 0, 0, 0 );
			XSync( Dpy, False );
		}

		XUnlockDisplay( Dpy );
#endif
	}
}

void LowLevelWindow_X11::GetDisplayResolutions( DisplayResolutions &out ) const
{
#ifdef HAVE_XRANDR
	if(g_bUseXRandR)
	{
		XRRScreenResources *scrRes = XRRGetScreenResources(Dpy, Win);
		RROutput primaryOut;
		if(g_iRandRVerMajor >= 1 && g_iRandRVerMinor >= 3)
			// RandR 1.3 can tell us what the primary display is.
			primaryOut = XRRGetOutputPrimary(Dpy, Win);
		else // Only RandR 1.2. We have to guess.
			primaryOut = scrRes->outputs[0];

		XRROutputInfo *outInfo = XRRGetOutputInfo(Dpy, scrRes, primaryOut);
		ASSERT(outInfo->ncrtc > 0);
		XRRCrtcInfo *conf = XRRGetCrtcInfo( Dpy, scrRes, outInfo->crtcs[0] );

		// A quirk of XRandR is that the width and height are as the display
		// controller ("CRTC") sees it, which means height and width are
		// flipped if there's rotation going on.
		bool bPortrait = conf->rotation & ( RR_Rotate_90 | RR_Rotate_270 );
#define REAL_WIDTH(x) bPortrait ? x.height : x.width
#define REAL_HEIGHT(x) bPortrait ? x.width : x.height

		for(int i = 0; i < scrRes->nmode; i++)
			// Ensure that the output supports the mode
			for(int j = 0; j < outInfo->nmode; j++)
				if(outInfo->modes[j] == scrRes->modes[i].id)
				{
					DisplayResolution res = { REAL_WIDTH(scrRes->modes[i]), REAL_HEIGHT(scrRes->modes[i]), true };
					out.insert( res );
					break;
				}
#undef REAL_WIDTH
#undef REAL_HEIGHT

		XRRFreeScreenResources(scrRes);
		XRRFreeOutputInfo(outInfo);
		XRRFreeCrtcInfo(conf);
	}
	else
#ifndef HAVE_XF86VIDMODE
		FAIL_M("XRandR not present or too old.");
#endif
#endif
#ifdef HAVE_XF86VIDMODE
	{
		int iNumModes = 0;
		XF86VidModeModeInfo **aModes_;
		XF86VidModeGetAllModeLines( Dpy, DefaultScreen( Dpy ), &iNumModes, &aModes_ );
		ASSERT_M( iNumModes != 0, "Couldn't get resolution list from X server" );
		XF86VidModeModeInfo *aModes = *aModes_;

		for( int i = 0; i < iNumModes; ++i )
		{
			// XXX: bStretched doesn't appear to actually be used anywhere?
			DisplayResolution res = { aModes[i].hdisplay, aModes[i].vdisplay, true };
			out.insert( res );

			// We're supposed to XFree() the private bits of XF86VidModeModeInfo
			// structs. This isn't actually possible from C++. At all. Period.
			// Let it leak.
		}

		XFree( aModes_ );
	}
#endif
}

bool LowLevelWindow_X11::SupportsThreadedRendering()
{
	return g_pBackgroundContext != NULL;
}

class RenderTarget_X11: public RenderTarget
{
public:
	RenderTarget_X11( LowLevelWindow_X11 *pWind );
	~RenderTarget_X11();

	void Create( const RenderTargetParam &param, int &iTextureWidthOut, int &iTextureHeightOut );
	unsigned GetTexture() const { return m_iTexHandle; }
	void StartRenderingTo();
	void FinishRenderingTo();

	// Copying from the Pbuffer to the texture flips Y.
	virtual bool InvertY() const { return true; }

private:
	int m_iWidth, m_iHeight;
	LowLevelWindow_X11 *m_pWind;
	GLXPbuffer m_iPbuffer;
	GLXContext m_pPbufferContext;
	unsigned int m_iTexHandle;

	GLXContext m_pOldContext;
	GLXDrawable m_pOldDrawable;
};

RenderTarget_X11::RenderTarget_X11( LowLevelWindow_X11 *pWind )
{
	m_pWind = pWind;
	m_iPbuffer = 0;
	m_pPbufferContext = NULL;
	m_iTexHandle = 0;
	m_pOldContext = NULL;
	m_pOldDrawable = 0;
}

RenderTarget_X11::~RenderTarget_X11()
{
	if( m_pPbufferContext )
		glXDestroyContext( Dpy, m_pPbufferContext );
	if( m_iPbuffer )
		glXDestroyPbuffer( Dpy, m_iPbuffer );
	if( m_iTexHandle )
		glDeleteTextures( 1, reinterpret_cast<GLuint*>(&m_iTexHandle) );
}

/* Note that although the texture size may need to be a power of 2,
 * the Pbuffer does not. */
void RenderTarget_X11::Create( const RenderTargetParam &param, int &iTextureWidthOut, int &iTextureHeightOut )
{
	//ASSERT( param.iWidth == power_of_two(param.iWidth) && param.iHeight == power_of_two(param.iHeight) );

	m_iWidth = param.iWidth;
	m_iHeight = param.iHeight;

	/* NOTE: int casts on GLX_DONT_CARE are for -Werror=narrowing */
	int pConfigAttribs[] =
	{
		GLX_DRAWABLE_TYPE, GLX_PBUFFER_BIT,
		GLX_RENDER_TYPE, GLX_RGBA_BIT,

		GLX_RED_SIZE, 8,
		GLX_GREEN_SIZE, 8,
		GLX_BLUE_SIZE, 8,
		GLX_ALPHA_SIZE, param.bWithAlpha? 8: (int) GLX_DONT_CARE,

		GLX_DOUBLEBUFFER, False,
		GLX_DEPTH_SIZE, param.bWithDepthBuffer? 16: (int) GLX_DONT_CARE,
		None
	};
	int iConfigs;
	GLXFBConfig *pConfigs = glXChooseFBConfig( Dpy, DefaultScreen(Dpy), pConfigAttribs, &iConfigs );
	ASSERT( pConfigs );

	const int pPbufferAttribs[] =
	{
		GLX_PBUFFER_WIDTH, param.iWidth,
		GLX_PBUFFER_HEIGHT, param.iHeight,
		None
	};

	for( int i = 0; i < iConfigs; ++i )
	{
		m_iPbuffer = glXCreatePbuffer( Dpy, pConfigs[i], pPbufferAttribs );
		if( m_iPbuffer == 0 )
			continue;

		XVisualInfo *pVisual = glXGetVisualFromFBConfig( Dpy, pConfigs[i] );
		m_pPbufferContext = glXCreateContext( Dpy, pVisual, g_pContext, True );
		ASSERT( m_pPbufferContext );
		XFree( pVisual );
		break;
	}

	ASSERT( m_iPbuffer );

	// allocate OpenGL texture resource
	glGenTextures( 1, reinterpret_cast<GLuint*>(&m_iTexHandle) );
	glBindTexture( GL_TEXTURE_2D, m_iTexHandle );

	LOG->Trace( "n %i, %ix%i", m_iTexHandle, param.iWidth, param.iHeight );
		while( glGetError() != GL_NO_ERROR )
		;

	int iTextureWidth = power_of_two( param.iWidth );
	int iTextureHeight = power_of_two( param.iHeight );
	iTextureWidthOut = iTextureWidth;
	iTextureHeightOut = iTextureHeight;

	glTexImage2D( GL_TEXTURE_2D, 0, param.bWithAlpha? GL_RGBA8:GL_RGB8,
			iTextureWidth, iTextureHeight, 0, param.bWithAlpha? GL_RGBA:GL_RGB, GL_UNSIGNED_BYTE, NULL );
	GLenum error = glGetError();
	ASSERT_M( error == GL_NO_ERROR, GLToString(error) );

	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );

	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
}

void RenderTarget_X11::StartRenderingTo()
{
	m_pOldContext = glXGetCurrentContext();
	m_pOldDrawable = glXGetCurrentDrawable();
	glXMakeCurrent( Dpy, m_iPbuffer, m_pPbufferContext );

	glViewport( 0, 0, m_iWidth, m_iHeight );
}

void RenderTarget_X11::FinishRenderingTo()
{
	glFlush();

	glBindTexture( GL_TEXTURE_2D, m_iTexHandle );

		while( glGetError() != GL_NO_ERROR )
		;

	glCopyTexSubImage2D( GL_TEXTURE_2D, 0, 0, 0, 0, 0, m_iWidth, m_iHeight );

	GLenum error = glGetError();
	ASSERT_M( error == GL_NO_ERROR, GLToString(error) );

	glBindTexture( GL_TEXTURE_2D, 0 );

	glXMakeCurrent( Dpy, m_pOldDrawable, m_pOldContext );
	m_pOldContext = NULL;
	m_pOldDrawable = 0;

}

bool LowLevelWindow_X11::SupportsRenderToTexture() const
{
	// Server must support pbuffers:
	const int iScreen = DefaultScreen( Dpy );
	float fVersion = strtof( glXQueryServerString(Dpy, iScreen, GLX_VERSION), NULL );
	if( fVersion < 1.3f )
		return false;

	return true;
}

RenderTarget *LowLevelWindow_X11::CreateRenderTarget()
{
	return new RenderTarget_X11( this );
}

void LowLevelWindow_X11::BeginConcurrentRenderingMainThread()
{
	/* Move the main thread, which is going to be loading textures, etc.
	 * but not rendering, to an undisplayed window. This results in
	 * smoother rendering. */
	bool b = glXMakeCurrent( Dpy, g_AltWindow, g_pContext );
	ASSERT(b);
}

void LowLevelWindow_X11::EndConcurrentRenderingMainThread()
{
	bool b = glXMakeCurrent( Dpy, Win, g_pContext );
	ASSERT(b);
}

void LowLevelWindow_X11::BeginConcurrentRendering()
{
	bool b = glXMakeCurrent( Dpy, Win, g_pBackgroundContext );
	ASSERT(b);
}

void LowLevelWindow_X11::EndConcurrentRendering()
{
	bool b = glXMakeCurrent( Dpy, None, NULL );
	ASSERT(b);
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
