#include "global.h"
#include "LowLevelWindow_X11.h"
#include "RageLog.h"
#include "RageException.h"
#include "archutils/Unix/X11Helper.h"
#include "PrefsManager.h" // XXX
#include "RageDisplay.h" // VideoModeParams
#include "DisplaySpec.h"
#include "LocalizedString.h"

#include "RageDisplay_OGL_Helpers.h"
using namespace RageDisplay_Legacy_Helpers;
using namespace X11Helper;

#include <set>
#include <math.h>	// ceil()
#include <GL/glxew.h>
#define GLX_GLXEXT_PROTOTYPES
#include <GL/glx.h>	// All sorts of stuff...
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/extensions/Xrandr.h>
#if defined(HAVE_XINERAMA)
#include <X11/extensions/Xinerama.h>
#endif

#if defined(HAVE_LIBXTST)
#include <X11/extensions/XTest.h>
#endif

// Display ID for treating the entire X screen as the display
const std::string ID_XSCREEN = "XSCREEN_RANDR";

static GLXContext g_pContext = nullptr;
static GLXContext g_pBackgroundContext = nullptr;
static Window g_AltWindow = None;
static bool g_bChangedScreenSize = false;
static SizeID g_iOldSize = None;
static Rotation g_OldRotation = RR_Rotate_0;
static XRRScreenConfiguration *g_pScreenConfig = nullptr;
static RRMode g_originalRandRMode = None;
static RROutput g_usedCrtc = None;
static int g_iRandRVerMinor = 0;
static int g_iRandRVerMajor = 0;
static bool g_bUseXRandR12 = false;
static bool g_bUseXinerama = false;

inline float calcRandRRefresh( unsigned long iPixelClock, int iHTotal, int iVTotal )
{
	// Pixel Clock divided by total pixels in mode,
	// not just those onscreen!
	return ( iPixelClock ) / ( iHTotal * iVTotal );
}

bool NetWMSupported(Display *Dpy, Atom feature);

static LocalizedString FAILED_CONNECTION_XSERVER( "LowLevelWindow_X11", "Failed to establish a connection with the X server" );
LowLevelWindow_X11::LowLevelWindow_X11()
{
	if( !OpenXConnection() )
		RageException::Throw( "%s", FAILED_CONNECTION_XSERVER.GetValue().c_str() );

	if( XRRQueryVersion( Dpy, &g_iRandRVerMajor, &g_iRandRVerMinor ) && g_iRandRVerMajor >= 1 && g_iRandRVerMinor >= 2) g_bUseXRandR12 = true;
#ifdef HAVE_XINERAMA
	int xinerama_event_base = 0;
	int xinerama_error_base = 0;
	Atom fullscreen_monitors = XInternAtom( Dpy, "_NET_WM_FULLSCREEN_MONITORS", False );
	if (XineramaQueryExtension( Dpy, &xinerama_event_base, &xinerama_error_base ) &&
		NetWMSupported( Dpy, fullscreen_monitors ))
	{
		g_bUseXinerama = true;
	}
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
	g_pScreenConfig = XRRGetScreenInfo( Dpy, RootWindow(Dpy, DefaultScreen(Dpy)) );
}

LowLevelWindow_X11::~LowLevelWindow_X11()
{
	// Reset the display
	if( !m_bWasWindowed )
	{
		RestoreOutputConfig();
		XUngrabKeyboard( Dpy, CurrentTime );
	}
	if( g_pContext )
	{
		glXDestroyContext( Dpy, g_pContext );
		g_pContext = nullptr;
	}
	if( g_pBackgroundContext )
	{
		glXDestroyContext( Dpy, g_pBackgroundContext );
		g_pBackgroundContext = nullptr;
	}
	XDestroyWindow( Dpy, Win );
	Win = None;
	XDestroyWindow( Dpy, g_AltWindow );
	g_AltWindow = None;
	CloseXConnection();
}

/*
 * Restore saved X screen/CRTC configuration
 */
void LowLevelWindow_X11::RestoreOutputConfig() {
	if (g_bChangedScreenSize) {
		XRRSetScreenConfig(Dpy, g_pScreenConfig, RootWindow(Dpy, DefaultScreen(Dpy)), g_iOldSize, g_OldRotation,
						   CurrentTime);
	}
	if (g_usedCrtc != None) {
		ASSERT(g_bUseXRandR12);
		XRRScreenResources *res = XRRGetScreenResources(Dpy, Win);
		XRRCrtcInfo *conf = XRRGetCrtcInfo(Dpy, res, g_usedCrtc);
		XRRSetCrtcConfig(Dpy, res, g_usedCrtc, conf->timestamp, conf->x, conf->y, g_originalRandRMode, conf->rotation,
						 conf->outputs, conf->noutput);
		XRRFreeScreenResources(res);
		XRRFreeCrtcInfo(conf);
	}
	g_iOldSize = None;
	g_bChangedScreenSize = false;
	g_usedCrtc = None;
	g_OldRotation = RR_Rotate_0;
}

void *LowLevelWindow_X11::GetProcAddress( RString s )
{
	// XXX: We should check whether glXGetProcAddress or
	// glXGetProcAddressARB is available/not nullptr, and go by that,
	// instead of assuming like this.
	return (void*) glXGetProcAddressARB( (const GLubyte*) s.c_str() );
}

RString LowLevelWindow_X11::TryVideoMode( const VideoModeParams &p, bool &bNewDeviceOut )
{
	// We're going to be interested in MapNotify/ConfigureNotify events in this routine,
	// so ensure our event mask includes these, restore it on exit
	XWindowAttributes winAttrib;
	auto restore = [&](XWindowAttributes *attr) { XSelectInput( Dpy, Win, attr->your_event_mask );};
	auto restoreAttrib = std::unique_ptr<XWindowAttributes, decltype(restore)>(&winAttrib, restore);

	// These might change if we're rendering at different resolution than window
	int windowWidth = p.width;
	int windowHeight = p.height;
	bool renderOffscreen = false;

	if( g_pContext == nullptr || p.bpp != CurrentParams.bpp || m_bWasWindowed != p.windowed )
	{
		bool bFirstRun = g_pContext == nullptr;
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
		if( xvi == nullptr )
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
		g_pContext = glXCreateContext( Dpy, xvi, nullptr, True );
		g_pBackgroundContext = glXCreateContext( Dpy, xvi, g_pContext, True );

		glXMakeCurrent( Dpy, Win, g_pContext );

		XGetWindowAttributes( Dpy, Win, &winAttrib );
		XSelectInput( Dpy, Win, winAttrib.your_event_mask | StructureNotifyMask | PropertyChangeMask );

		XMapWindow( Dpy, Win );

		XEvent ev;
		do {XWindowEvent( Dpy, Win, StructureNotifyMask, &ev );}
		while ( ev.type != MapNotify);

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

		XGetWindowAttributes( Dpy, Win, &winAttrib );
		XSelectInput( Dpy, Win, winAttrib.your_event_mask | StructureNotifyMask | PropertyChangeMask );

		if( !p.windowed )
		{
			// X11 is an asynchronous beast. If we're resizing an existing
			// window directly (i.e. override-redirect as opposed to asking the
			// WM to do it) and don't wait for the window to actually be
			// resized, we'll get unexpected results from glViewport() etc. I
			// don't know why, or why it *doesn't* break in the slower process
			// of waiting for the WM to resize the window.

			// So, set the event mask so we're notified when the window is resized...
			// Send the resize command...
			XResizeWindow( Dpy, Win, static_cast<unsigned int> (p.width), static_cast<unsigned int> (p.height) );

			// We'll wait for the notification once we've done everything else,
			// to save time.
		}
	}

	float rate = 60; // Will be unchanged if windowed. Not sure I care.

	if( !p.windowed )
	{
		RestoreOutputConfig();

		if (p.sDisplayId == ID_XSCREEN || p.sDisplayId.empty()) {
			// If the user changed the resolution while StepMania was windowed we overwrite the resolution to restore with it at exit.
			g_iOldSize = XRRConfigCurrentConfiguration( g_pScreenConfig, &g_OldRotation );
			m_bWasWindowed = false;

			// Find a matching mode.
			int iSizesXct;
			XRRScreenSize *pSizesX = XRRSizes( Dpy, DefaultScreen(Dpy), &iSizesXct );
			ASSERT_M( iSizesXct != 0, "Couldn't get resolution list from X server" );

			int iSizeMatch = -1;

			for (int i = 0; i < iSizesXct; ++i) {
				if (pSizesX[i].width == p.width && pSizesX[i].height == p.height) {
					iSizeMatch = i;
					break;
				}
			}
			if (iSizeMatch != g_iOldSize) {
				g_bChangedScreenSize = true;
			}

			// Set this mode.
			// XXX: This doesn't handle if the config has changed since we queried it (see man Xrandr)
			Status s = XRRSetScreenConfig( Dpy, g_pScreenConfig, RootWindow(Dpy, DefaultScreen(Dpy)), iSizeMatch, 1, CurrentTime );
			if (s)
			{
				return "Failed to set screen config";
			}

			XMoveWindow( Dpy, Win, 0, 0 );

			XRaiseWindow( Dpy, Win );

			// We want to prevent the WM from catching anything that comes from the keyboard.
			// We should do this every time on fullscreen and not only we entering from windowed mode because we could lose focus at resolution change and that will leave the user input locked.
			while (XGrabKeyboard( Dpy, Win, True, GrabModeAsync, GrabModeAsync, CurrentTime ));

		} else {
			ASSERT(g_bUseXRandR12);
			/* === Configuring a specific CRTC === */
			// Arcane and undocumented but PROPER XRandR 1.2 method.
			// What we do is directly reconfigure the CRTC of the primary display,
			// Which prevents the (RandR) screen itself from resizing, and therefore
			// leaving user's desktop unmolested.
			LOG->Info("LowLevelWindow_X11: Using XRandR");

			XRRScreenResources *scrRes = XRRGetScreenResources(Dpy, Win);
			ASSERT(scrRes != nullptr);
			ASSERT(scrRes->ncrtc > 0);
			ASSERT(scrRes->noutput > 0);
			ASSERT(scrRes->nmode > 0);

			// If an output name has been specified, search for it
			RROutput targetOut = None;
			if (p.sDisplayId.length() > 0) {
				for (unsigned int i = 0; i < scrRes->noutput && targetOut == None; ++i) {
					XRROutputInfo *outInfo = XRRGetOutputInfo(Dpy, scrRes, scrRes->outputs[i]);
					std::string outName = std::string(outInfo->name, static_cast<unsigned int> (outInfo->nameLen));
					if (p.sDisplayId == outName) {
						targetOut = scrRes->outputs[i];
					}
					XRRFreeOutputInfo(outInfo);
				}
			}
			if (targetOut == None) {
				LOG->Info("Did not find display output %s, trying another", p.sDisplayId.c_str());
				// didn't find named output, pick primary/or at least one that works
				if (g_iRandRVerMajor >= 1 && g_iRandRVerMinor >= 3) {
					// RandR 1.3 can tell us what the primary display is.
					targetOut = XRRGetOutputPrimary(Dpy, Win);
				} else {
					// Only RandR 1.2. We'll look for a "Connected" output, or if we can't find that,
					// (it is possible the connection state could be unknown), we'll at least
					// look for an output with a CRTC driving it
					RROutput connected = None, hasCrtc = None;
					for (unsigned int i = 0; i < scrRes->noutput; ++i) {
						XRROutputInfo *outInfo = XRRGetOutputInfo(Dpy, scrRes, scrRes->outputs[i]);
						if (outInfo->connection == RR_Connected) { // Check for CONNECTED state: Connected == 0
							connected = scrRes->outputs[i];
						}
						if (outInfo->crtc != None) {
							hasCrtc = outInfo->crtc;
						}
						XRRFreeOutputInfo(outInfo);
					}
					targetOut = connected != None ? connected : hasCrtc;
					ASSERT(targetOut != None);
				}
			}

			// if the target output is not currently being driven by a crtc,
			// find an unused crtc that can be connected to it
			XRROutputInfo *tgtOutInfo = XRRGetOutputInfo( Dpy, scrRes, targetOut );
			if (tgtOutInfo == nullptr)
			{
				XRRFreeScreenResources(scrRes);
				return "Failed to find XRROutput";
			}

			RRCrtc tgtOutCrtc = tgtOutInfo->crtc;
			if (tgtOutCrtc == None)
			{
				for (unsigned int i = 0; i < tgtOutInfo->ncrtc; ++i)
				{
					XRRCrtcInfo *crtcInfo = XRRGetCrtcInfo( Dpy, scrRes, tgtOutInfo->crtcs[i] );
					if (crtcInfo->mode == None)
					{
						tgtOutCrtc = tgtOutInfo->crtcs[i];
					}
					XRRFreeCrtcInfo( crtcInfo );
				}
			}
			ASSERT(tgtOutCrtc != None);


			XRRCrtcInfo *oldConf = XRRGetCrtcInfo( Dpy, scrRes, tgtOutCrtc );

			float fRefreshDiff = 99999;
			float fRefreshRate = 0;
			RRMode mode = None;
			// A quirk of XRandR is that the width and height are as the display
			// controller ("CRTC") sees it, which means height and width are
			// flipped if there's rotation going on.
			const bool bPortrait = (oldConf->rotation & (RR_Rotate_90 | RR_Rotate_270)) != 0;
			// Find a mode that matches our exact wanted resolution,
			// with as close to our desired refresh rate as possible.
			for (int i = 0; i < scrRes->nmode; i++) {
				const XRRModeInfo &thisMI = scrRes->modes[i];
				const unsigned int modeWidth = bPortrait ? thisMI.height : thisMI.width;
				const unsigned int modeHeight = bPortrait ? thisMI.width : thisMI.height;
				if (modeWidth == p.width && modeHeight == p.height) {
					float fTempRefresh = calcRandRRefresh(thisMI.dotClock, thisMI.hTotal, thisMI.vTotal);
					float fTempDiff = std::abs(p.rate - fTempRefresh);
					if ((p.rate != REFRESH_DEFAULT && fTempDiff < fRefreshDiff) ||
						(p.rate == REFRESH_DEFAULT && fTempRefresh > fRefreshRate)) {
						int j;
						// Ensure that the output supports the mode
						for (j = 0; j < tgtOutInfo->nmode; j++)
							if (tgtOutInfo->modes[j] == scrRes->modes[i].id) {
								mode = tgtOutInfo->modes[j];
								break;
							}

						if (j < tgtOutInfo->nmode) {
							fRefreshRate = fTempRefresh;
							fRefreshDiff = fTempDiff;
						}
					}
				}
			}
			rate = roundf(fRefreshRate);

			g_usedCrtc = tgtOutCrtc;
			g_originalRandRMode = oldConf->mode;

			const std::string tgtOutName = std::string(tgtOutInfo->name, static_cast<unsigned int> (tgtOutInfo->nameLen));
			LOG->Info("XRandR output config using CRTC %lu in mode %lu, driving output %s",
					  g_usedCrtc, mode, tgtOutName.c_str());
			// and FIRE!
			Status s = XRRSetCrtcConfig(Dpy, scrRes, g_usedCrtc, oldConf->timestamp, oldConf->x, oldConf->y, mode,
							 oldConf->rotation, oldConf->outputs, oldConf->noutput);
			if (s) {
				XRRFreeCrtcInfo(oldConf);
				XRRFreeOutputInfo(tgtOutInfo);
				XRRFreeScreenResources(scrRes);
				return "Failed to set CRTC config";
			}

			// We don't move to absolute 0,0 because that may be in the area of a different output.
			// Instead we preserved the corner of our CRTC; go to that.
			XMoveWindow(Dpy, Win, oldConf->x, oldConf->y);

			// Final cleanup
			XRRFreeCrtcInfo(oldConf);
			XRRFreeOutputInfo(tgtOutInfo);
			XRRFreeScreenResources(scrRes);
		}
		m_bWasWindowed = false;

		XRaiseWindow( Dpy, Win );

		// We want to prevent the WM from catching anything that comes from the keyboard.
		// We should do this every time on fullscreen and not only we entering from windowed mode because we could lose focus at resolution change and that will leave the user input locked.
		while (XGrabKeyboard( Dpy, Win, True, GrabModeAsync, GrabModeAsync, CurrentTime ));
	}
	else // if(p.windowed)
	{
		if( !m_bWasWindowed )
		{
			// Return the display to the mode it was in before we fullscreened.
			RestoreOutputConfig();
			XUngrabKeyboard( Dpy, CurrentTime );
			m_bWasWindowed = true;
		}

		Atom net_wm_state = XInternAtom( Dpy, "_NET_WM_STATE", False );
		Atom fullscreen_state = XInternAtom( Dpy, "_NET_WM_STATE_FULLSCREEN", False );
		Atom maximized_vert = XInternAtom( Dpy, "_NET_WM_STATE_MAXIMIZED_VERT", False );
		Atom maximized_horz = XInternAtom( Dpy, "_NET_WM_STATE_MAXIMIZED_HORZ", False );
		// if FSBW, find matching monitor, move window to its origin,
		// then set fullscreen hint, and set the CurrentParams.outWidth, CurrentParams.outHeight to the values of that display
		// otherwise set the size hints and disable MAXIMIZED_*
		if (p.bWindowIsFullscreenBorderless)
		{
			auto specs = DisplaySpecs{};
			GetDisplaySpecs( specs );
			auto target = std::find_if( specs.begin(), specs.end(), [&]( const DisplaySpec &spec ) {
				return p.sDisplayId == spec.id() && spec.currentMode() != nullptr;
			} );
			// If we didn't find a matching DisplaySpec for the requested ID, pick the first one with a current mode
			if (target == specs.end())
			{
				target = std::find_if( specs.begin(), specs.end(), [&]( const DisplaySpec &spec ) {
					return spec.currentMode() != nullptr;
				} );
			}
			// If we _still_ haven't found anything (unlikely), then just give up
			if (target == specs.end())
			{
				return "Unable to find destination monitor for fullscreen borderless";
			}

			windowWidth = target->currentMode()->width;
			windowHeight = target->currentMode()->height;

			if (windowWidth != p.width || windowHeight != p.height)
			{
				renderOffscreen = true;
			}

			// Reset anything that might've been set previously:
			// (1) Undo Min/Max size bounds
			// (2) Remove FULLSCREEN/MAXIMIZED_{HORIZ,VERT} hints
			// Without doing this, WM may not let us move/resize window to new display
			// Give Window manager the chance to react to changes (otherwise, Mutter had problems
			// properly reacting to moving a _NET_WM_STATE_FULLSCREEN window to a different output
			//   and fullscreen resetting FULLSCREEN hint.
			XSizeHints hints;
			hints.flags = 0;
			XSetWMNormalHints( Dpy, Win, &hints );
#if defined(HAVE_XINERAMA)
			if (!g_bUseXinerama || !SetWMFullscreenMonitors( *target ))
#endif
			{
				SetWMState( winAttrib.root, Win, 0, maximized_horz );
				SetWMState( winAttrib.root, Win, 0, maximized_vert );
				SetWMState( winAttrib.root, Win, 0, fullscreen_state );

				XFlush( Dpy );
				XResizeWindow( Dpy, Win, static_cast<unsigned int> (windowWidth), static_cast<unsigned int> (windowHeight) );
				XMoveWindow( Dpy, Win, target->currentBounds().left, target->currentBounds().top );
				XRaiseWindow( Dpy, Win );

				SetWMState( winAttrib.root, Win, 1, fullscreen_state );
				SetWMState( winAttrib.root, Win, 1, maximized_horz );
				SetWMState( winAttrib.root, Win, 1, maximized_vert );
			}
		} else
		{
			windowWidth = p.width;
			windowHeight = p.height;

			SetWMState( winAttrib.root, Win, 0, fullscreen_state );
			// Make a window fixed size, don't let resize it or maximize it.
			// Do this before resizing the window so that pane-style WMs (Ion,
			// ratpoison) don't resize us back inappropriately.
			{
				XSizeHints hints;

				hints.flags = PMinSize|PMaxSize|PWinGravity;
				hints.min_width = hints.max_width = windowWidth;
				hints.min_height = hints.max_height = windowHeight;
				hints.win_gravity = CenterGravity;

				XSetWMNormalHints( Dpy, Win, &hints );
			}
			/* Workaround for metacity and compiz: if the window have the same
			 * resolution or higher than the screen, it gets automaximized even
			 * when the window is set to not let it happen. This happens when
			 * changing from fullscreen to window mode and our screen resolution
			 * is bigger. */
			{
				SetWMState( winAttrib.root, Win, 1, maximized_vert );
				SetWMState( winAttrib.root, Win, 1, maximized_horz );

				// This one is needed for compiz, if the window reaches out of bounds of the screen it becames destroyed, only the window, the program is left running.
				// Commented out per the patch at http://ssc.ajworld.net/sm-ssc/bugtracker/view.php?id=398
				//XMoveWindow( Dpy, Win, 0, 0 );
			}
		}


	}

	CurrentParams = p;
	CurrentParams.windowWidth = windowWidth;
	CurrentParams.windowHeight = windowHeight;
	CurrentParams.renderOffscreen = renderOffscreen;
	ASSERT( rate > 0 );
	CurrentParams.rate = static_cast<int> (roundf(rate));

	if (!p.windowed)
	{
		// Set our V-sync hint.
		if (GLXEW_EXT_swap_control) // I haven't seen this actually implemented yet, but why not.
			glXSwapIntervalEXT( Dpy, Win, CurrentParams.vsync ? 1 : 0 );
			// XXX: These two might be server-global. I should look into whether
			// to try to preserve the original value on exit.
#ifdef GLXEW_MESA_swap_control // Added in 1.7. 1.6 is still common out there apparently.
			else if(GLXEW_MESA_swap_control) // Haven't seen this NOT implemented yet
				glXSwapIntervalMESA( CurrentParams.vsync ? 1 : 0 );
#endif
		else if (GLXEW_SGI_swap_control) // But old GLEW.
			glXSwapIntervalSGI( CurrentParams.vsync ? 1 : 0 );
		else
			CurrentParams.vsync = false; // Assuming it's not on
	}




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

void LowLevelWindow_X11::GetDisplaySpecs(DisplaySpecs &out) const {
	int screenNum = DefaultScreen(Dpy);
	Screen *screen = ScreenOfDisplay(Dpy, screenNum);

	XWindowAttributes winAttr = XWindowAttributes();
	if (XGetWindowAttributes(Dpy, Win, &winAttr)) {
		screen = winAttr.screen;
		screenNum = XScreenNumberOfScreen(screen);
	}

	// Create a display spec for the entire X screen itself
	// First get current config
	Rotation curRotation;
	XRRScreenConfiguration *screenConf = XRRGetScreenInfo(Dpy, Win);
	const short curRate = XRRConfigCurrentRate(screenConf);
	SizeID curSizeId = XRRConfigCurrentConfiguration(screenConf, &curRotation);
	// curRotation does not factor into how we report supported XScreen sizes:
	// XRR reports the supported *screen* sizes with height/width swapped appropriately
	// for currently configured rotation. Supported sizes for *output* modes (below)
	// DO NOT account for screen rotation

	std::set<DisplayMode> screenModes;
	int nsizes = 0;
	XRRScreenSize *screenSizes = XRRSizes( Dpy, screenNum, &nsizes);
	DisplayMode screenCurMode = {0};
	for (unsigned int szIdx = 0, mode_idx = 0; szIdx < nsizes; ++szIdx) {
		XRRScreenSize &size = screenSizes[szIdx];
		int nrates = 0;
		short *rates = XRRRates(Dpy, screenNum, szIdx, &nrates);
		for (unsigned int rIdx = 0; rIdx < nrates; ++rIdx, ++mode_idx) {
			DisplayMode m = {static_cast<unsigned int> (size.width), static_cast<unsigned int> (size.height), static_cast<double> (rates[rIdx])};
			screenModes.insert(m);
			if (rates[rIdx] == curRate && szIdx == curSizeId) {
				screenCurMode = m;
			}
		}
	}
	const RectI screenBounds( 0, 0, screenSizes[curSizeId].width, screenSizes[curSizeId].height);
	const DisplaySpec screenSpec( ID_XSCREEN, "X Screen", screenModes, screenCurMode, screenBounds, true);
	out.insert(screenSpec);
	// XRRScreenSize array from XRRSizes does *not* have to be returned (valgrind said XFree was an invalid
	// free in a small test program, there is no XRRFreeScreenSize, etc)
	XRRFreeScreenConfigInfo(screenConf);

	if (g_bUseXRandR12) {
		// Build per-output DisplaySpecs

		// First, get the list of resolutions that'll be referenced (by RRMode) in each
		// OutputInfo
		XRRScreenResources *scrRes = XRRGetScreenResources(Dpy, Win);
		std::map<RRMode, DisplayMode> outputModes;
		for (unsigned int i = 0; i < scrRes->nmode; ++i) {
			const XRRModeInfo &mode = scrRes->modes[i];
			DisplayMode m = {mode.width, mode.height,
							 calcRandRRefresh(mode.dotClock, mode.hTotal, mode.vTotal)};
			outputModes[mode.id] = m;
		}

		// Now, for each output, build a corresponding DisplaySpec
		for (unsigned int outIdx = 0; outIdx < scrRes->noutput; ++outIdx)
		{
			XRROutputInfo *outInfo = XRRGetOutputInfo( Dpy, scrRes, scrRes->outputs[outIdx] );
			if (outInfo->nmode > 0)
			{
				// Get the current configuration of the Output, if it's being driven by
				// a crtc
				RRMode curRRMode = None;
				bool bPortrait = false;
				int crtcX = 0, crtcY = 0;
				if (outInfo->crtc != None)
				{
					XRRCrtcInfo *conf = XRRGetCrtcInfo( Dpy, scrRes, outInfo->crtc );
					curRRMode = conf->mode;
					bPortrait = (conf->rotation & (RR_Rotate_90 | RR_Rotate_270)) != 0;
					crtcX = conf->x;
					crtcY = conf->y;
					XRRFreeCrtcInfo( conf );
				}
				// Get all supported modes, noting which one, if any, is currently active
				std::set<DisplayMode> outputSupported;
				DisplayMode outputCurMode = {0};
				RectI outBounds;
				for (unsigned int modeIdx = 0; modeIdx < outInfo->nmode; ++modeIdx)
				{
					DisplayMode mode = outputModes[outInfo->modes[modeIdx]];
					unsigned int modeWidth = bPortrait ? mode.height : mode.width;
					unsigned int modeHeight = bPortrait ? mode.width : mode.height;
					DisplayMode m = {modeWidth, modeHeight, mode.refreshRate};
					outputSupported.insert( m );
					if (curRRMode != None && outInfo->modes[modeIdx] == curRRMode)
					{
						outputCurMode = m;
						outBounds = RectI( crtcX, crtcY, crtcX + modeWidth, crtcY + modeHeight);
					}
				}
				const std::string outId( outInfo->name, static_cast<unsigned int> (outInfo->nameLen) );
				const std::string outName( outId );
				if (curRRMode != None)
				{
					out.insert( DisplaySpec( outId, outName, outputSupported, outputCurMode, outBounds ));
				} else
				{
					out.insert( DisplaySpec( outId, outName, outputSupported ));
				}
			}
			XRRFreeOutputInfo( outInfo );
		}
		XRRFreeScreenResources( scrRes );
	}
}

bool LowLevelWindow_X11::SupportsThreadedRendering()
{
	return g_pBackgroundContext != nullptr;
}

class RenderTarget_X11: public RenderTarget
{
public:
	RenderTarget_X11( LowLevelWindow_X11 *pWind );
	~RenderTarget_X11();

	void Create( const RenderTargetParam &param, int &iTextureWidthOut, int &iTextureHeightOut );
	uintptr_t GetTexture() const { return static_cast<uintptr_t>(m_iTexHandle); }
	void StartRenderingTo();
	void FinishRenderingTo();

	// Copying from the Pbuffer to the texture flips Y.
	virtual bool InvertY() const { return true; }

private:
	int m_iWidth, m_iHeight;
	LowLevelWindow_X11 *m_pWind;
	GLXPbuffer m_iPbuffer;
	GLXContext m_pPbufferContext;
	GLuint m_iTexHandle;

	GLXContext m_pOldContext;
	GLXDrawable m_pOldDrawable;
};

RenderTarget_X11::RenderTarget_X11( LowLevelWindow_X11 *pWind )
{
	m_pWind = pWind;
	m_iPbuffer = 0;
	m_pPbufferContext = nullptr;
	m_iTexHandle = 0;
	m_pOldContext = nullptr;
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
			iTextureWidth, iTextureHeight, 0, param.bWithAlpha? GL_RGBA:GL_RGB, GL_UNSIGNED_BYTE, nullptr );
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
	m_pOldContext = nullptr;
	m_pOldDrawable = 0;

}

bool LowLevelWindow_X11::SupportsRenderToTexture() const
{
	// Server must support pbuffers:
	const int iScreen = DefaultScreen( Dpy );
	float fVersion = strtof( glXQueryServerString(Dpy, iScreen, GLX_VERSION), nullptr );
	if( fVersion < 1.3f )
		return false;

	return true;
}

bool NetWMSupported(Display *Dpy, Atom feature)
{
	Atom net_supported = XInternAtom( Dpy, "_NET_SUPPORTED", False );
	Atom actual_type_return = BadAtom;
	int actual_format_return = 0;
	unsigned long nitems_return = 0;
	unsigned long bytes_after_return = 0;
	Atom *prop_return;
	Status status = XGetWindowProperty( Dpy, RootWindow( Dpy, DefaultScreen( Dpy )), net_supported, 0, 8192, False,
										XA_ATOM, &actual_type_return,
										&actual_format_return, &nitems_return, &bytes_after_return,
										reinterpret_cast<unsigned char **> (&prop_return));
	if (status != Success)
	{
		return false;
	}

	auto supported = std::find( prop_return, prop_return + nitems_return, feature ) != prop_return + nitems_return;
	XFree( prop_return );
	return supported;
}

bool LowLevelWindow_X11::SupportsFullscreenBorderlessWindow() const
{
	Atom fullscreen = XInternAtom( Dpy, "_NET_WM_STATE_FULLSCREEN", False );
	return NetWMSupported( Dpy, fullscreen );
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
	bool b = glXMakeCurrent( Dpy, None, nullptr );
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
