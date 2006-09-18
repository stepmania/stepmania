#include "global.h"
#include "GraphicsWindow.h"
#include "ProductInfo.h"
#include "RageLog.h"
#include "RageUtil.h"
#include "RageDisplay.h"
#include "DisplayResolutions.h"
#include "arch/ArchHooks/ArchHooks.h"
#include "archutils/Win32/AppInstance.h"
#include "archutils/Win32/Crash.h"
#include "archutils/Win32/ErrorStrings.h"
#include "archutils/Win32/WindowIcon.h"
#include "archutils/Win32/GetFileInformation.h"

#include <set>

static const RString g_sClassName = PRODUCT_ID;

static HWND g_hWndMain;
static HDC g_HDC;
static VideoModeParams g_CurrentParams;
static bool g_bResolutionChanged = false;
static bool g_bHasFocus = true;
static HICON g_hIcon = NULL;
static bool m_bWideWindowClass;
static bool g_bD3D = false;

/* If we're fullscreen, this is the mode we set. */
static DEVMODE g_FullScreenDevMode;
static bool g_bRecreatingVideoMode = false;

static UINT g_iQueryCancelAutoPlayMessage = 0;

static RString GetNewWindow()
{
	HWND h = GetForegroundWindow();
	if( h == NULL )
		return "(NULL)";

	DWORD iProcessID;
	GetWindowThreadProcessId( h, &iProcessID );

	RString sName;
	GetProcessFileName( iProcessID, sName );

	sName = Basename(sName);

	return sName;
}

static LRESULT CALLBACK GraphicsWindow_WndProc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam )
{
	CHECKPOINT_M( ssprintf("%p, %u, %08x, %08x", hWnd, msg, wParam, lParam) );

	/* Suppress autorun. */
	if( msg == g_iQueryCancelAutoPlayMessage )
		return true;

	switch( msg )
	{
	case WM_ACTIVATE:
	{
		const bool bInactive = (LOWORD(wParam) == WA_INACTIVE);
		const bool bMinimized = (HIWORD(wParam) != 0);
		const bool bHadFocus = g_bHasFocus;
		g_bHasFocus = !bInactive && !bMinimized;
		LOG->Trace( "WM_ACTIVATE (%i, %i): %s", bInactive, bMinimized, g_bHasFocus? "has focus":"doesn't have focus" );
		if( !g_bHasFocus )
		{
			RString sName = GetNewWindow();
			static set<RString> sLostFocusTo;
			sLostFocusTo.insert( sName );
			RString sStr;
			for( set<RString>::const_iterator it = sLostFocusTo.begin(); it != sLostFocusTo.end(); ++it )
				sStr += (sStr.size()?", ":"") + *it;

			LOG->MapLog( "LOST_FOCUS", "Lost focus to: %s", sStr.c_str() );
		}

		if( !g_bD3D && !g_CurrentParams.windowed && !g_bRecreatingVideoMode )
		{
			/* In OpenGL (not D3D), it's our job to unset and reset the full-screen video mode
			 * when we focus changes, and to hide and show the window.  Hiding is done in WM_KILLFOCUS,
			 * because that's where most other apps seem to do it. */
			if( g_bHasFocus && !bHadFocus )
			{
				ChangeDisplaySettings( &g_FullScreenDevMode, CDS_FULLSCREEN );
				ShowWindow( g_hWndMain, SW_SHOWNORMAL );
			}
			else if( !g_bHasFocus && bHadFocus )
			{
				ChangeDisplaySettings( NULL, 0 );
			}
		}

		return 0;
	}
	case WM_KILLFOCUS:
		if( !g_bD3D && !g_CurrentParams.windowed && !g_bRecreatingVideoMode )
			ShowWindow( g_hWndMain, SW_SHOWMINNOACTIVE );
		break;

	/* Is there any reason we should care what size the user resizes the window to? */
//	case WM_GETMINMAXINFO:

	case WM_SETCURSOR:
		if( !g_CurrentParams.windowed )
		{
			SetCursor( NULL );
			return 1;
		}
		break;

	case WM_SYSCOMMAND:
		switch( wParam&0xFFF0 )
		{
		case SC_MONITORPOWER:
		case SC_SCREENSAVE:
			return 0;
		}
		break;

	case WM_PAINT:
	{
		PAINTSTRUCT ps;
		BeginPaint( hWnd, &ps );
		EndPaint( hWnd, &ps );
		break;
	}

	case WM_KEYDOWN:
	case WM_KEYUP:
	case WM_SYSKEYDOWN:
	case WM_SYSKEYUP:
		/* We handle all input ourself, via DirectInput. */
		return 0;

	case WM_CLOSE:
		LOG->Trace("WM_CLOSE: shutting down");
		ArchHooks::SetUserQuit();
		return 0;

	case WM_WINDOWPOSCHANGED:
	{
		/* If we're fullscreen and don't have focus, our window is hidden, so GetClientRect
		 * isn't meaningful. */
		if( !g_CurrentParams.windowed && !g_bHasFocus )
			break;

		RECT rect;
		GetClientRect( hWnd, &rect );

		int iWidth = rect.right - rect.left;
		int iHeight = rect.bottom - rect.top;
		if( g_CurrentParams.width != iWidth || g_CurrentParams.height != iHeight )
		{
			g_CurrentParams.width = iWidth;
			g_CurrentParams.height = iHeight;
			g_bResolutionChanged = true;
		}
		break;
	}
	}

	CHECKPOINT_M( ssprintf("%p, %u, %08x, %08x", hWnd, msg, wParam, lParam) );

	if( m_bWideWindowClass )
		return DefWindowProcW( hWnd, msg, wParam, lParam );
	else
		return DefWindowProcA( hWnd, msg, wParam, lParam );
}

static void AdjustVideoModeParams( VideoModeParams &p )
{
	DEVMODE dm;
	ZERO( dm );
	dm.dmSize = sizeof(dm);
	if( !EnumDisplaySettings(NULL, ENUM_CURRENT_SETTINGS, &dm) )
	{
		p.rate = 60;
		LOG->Warn( "%s", werr_ssprintf(GetLastError(), "EnumDisplaySettings failed").c_str() );
		return;
	}

	/*
	 * On a nForce 2 IGP on Windows 98, dm.dmDisplayFrequency sometimes 
	 * (but not always) is 0.
	 *
	 * MSDN: When you call the EnumDisplaySettings function, the 
	 * dmDisplayFrequency member may return with the value 0 or 1. 
	 * These values represent the display hardware's default refresh rate. 
	 * This default rate is typically set by switches on a display card or 
	 * computer motherboard, or by a configuration program that does not 
	 * use Win32 display functions such as ChangeDisplaySettings.
	 */
	if( !(dm.dmFields & DM_DISPLAYFREQUENCY) ||
		dm.dmDisplayFrequency == 0 ||
		dm.dmDisplayFrequency == 1 )
	{
		p.rate = 60;
		LOG->Warn( "EnumDisplaySettings doesn't know what the refresh rate is. %d %d %d", dm.dmPelsWidth, dm.dmPelsHeight, dm.dmBitsPerPel );
	}
	else
	{
		p.rate = dm.dmDisplayFrequency;
	}
}

/* Set the display mode to the given size, bit depth and refresh.  The refresh
 * setting may be ignored. */
RString GraphicsWindow::SetScreenMode( const VideoModeParams &p )
{
	if( p.windowed )
	{
		/* We're going windowed.  If we were previously fullscreen, reset. */
		ChangeDisplaySettings( NULL, 0 );

		return RString();
	}

	DEVMODE DevMode;
	ZERO( DevMode );
	DevMode.dmSize = sizeof(DEVMODE);
	DevMode.dmPelsWidth = p.width;
	DevMode.dmPelsHeight = p.height;
	DevMode.dmBitsPerPel = p.bpp;
	DevMode.dmFields = DM_PELSWIDTH | DM_PELSHEIGHT | DM_BITSPERPEL;

	if( p.rate != REFRESH_DEFAULT )
	{
		DevMode.dmDisplayFrequency = p.rate;
		DevMode.dmFields |= DM_DISPLAYFREQUENCY;
	}
	ChangeDisplaySettings( NULL, 0 );

	int ret = ChangeDisplaySettings( &DevMode, CDS_FULLSCREEN );
	if( ret != DISP_CHANGE_SUCCESSFUL && (DevMode.dmFields & DM_DISPLAYFREQUENCY) )
	{
		DevMode.dmFields &= ~DM_DISPLAYFREQUENCY;
		ret = ChangeDisplaySettings( &DevMode, CDS_FULLSCREEN );
	}

	/* XXX: append error */
	if( ret != DISP_CHANGE_SUCCESSFUL )
		return "Couldn't set screen mode";

	g_FullScreenDevMode = DevMode;
	return RString();
}

static int GetWindowStyle( bool bWindowed )
{
	if( bWindowed )
		return WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN | WS_CLIPSIBLINGS;
	else
		return WS_POPUP;
}

/* Set the final window size, set the window text and icon, and then unhide the
 * window. */
void GraphicsWindow::CreateGraphicsWindow( const VideoModeParams &p, bool bForceRecreateWindow )
{
	g_CurrentParams = p;

	// Adjust g_CurrentParams to reflect the actual display settings.
	AdjustVideoModeParams( g_CurrentParams );

	if( g_hWndMain == NULL || bForceRecreateWindow )
	{
		int iWindowStyle = GetWindowStyle( p.windowed );

		AppInstance inst;
		HWND hWnd = CreateWindow( g_sClassName, "app", iWindowStyle,
						0, 0, 0, 0, NULL, NULL, inst, NULL );
		if( hWnd == NULL )
			RageException::Throw( "%s", werr_ssprintf( GetLastError(), "CreateWindow" ).c_str() );

		/* If an old window exists, transfer focus to the new window before deleting
		 * it, or some other window may temporarily get focus, which can cause it
		 * to be resized. */
		if( g_hWndMain != NULL )
		{
			/* While we change to the new window, don't do ChangeDisplaySettings in WM_ACTIVATE. */
			g_bRecreatingVideoMode = true;
			SetForegroundWindow( hWnd );
			g_bRecreatingVideoMode = false;

			GraphicsWindow::DestroyGraphicsWindow();
		}

		g_hWndMain = hWnd;
		CrashHandler::SetForegroundWindow( g_hWndMain );
		g_HDC = GetDC( g_hWndMain );
	}

	/* Update the window title. */
	do
	{
		if( m_bWideWindowClass )
		{
			if( SetWindowText( g_hWndMain, ConvertUTF8ToACP(p.sWindowTitle).c_str() ) )
				break;
		}

		SetWindowTextA( g_hWndMain, ConvertUTF8ToACP(p.sWindowTitle) );
	} while(0);

	/* Update the window icon. */
	if( g_hIcon != NULL )
	{
		SetClassLong( g_hWndMain, GCL_HICON, (LONG) LoadIcon(NULL,IDI_APPLICATION) );
		DestroyIcon( g_hIcon );
		g_hIcon = NULL;
	}
	g_hIcon = IconFromFile( p.sIconFile );
	if( g_hIcon != NULL )
		SetClassLong( g_hWndMain, GCL_HICON, (LONG) g_hIcon );

	/* The window style may change as a result of switching to or from fullscreen;
	 * apply it.  Don't change the WS_VISIBLE bit. */
	int iWindowStyle = GetWindowStyle( p.windowed );
	if( GetWindowLong( g_hWndMain, GWL_STYLE ) & WS_VISIBLE )
		iWindowStyle |= WS_VISIBLE;
	SetWindowLong( g_hWndMain, GWL_STYLE, iWindowStyle );

	RECT WindowRect;
	SetRect( &WindowRect, 0, 0, p.width, p.height );
	AdjustWindowRect( &WindowRect, iWindowStyle, FALSE );

	//LOG->Warn( "w = %d, h = %d", p.width, p.height );

	const int iWidth = WindowRect.right - WindowRect.left;
	const int iHeight = WindowRect.bottom - WindowRect.top;

	/* If windowed, center the window. */
	int x = 0, y = 0;
	if( p.windowed )
	{
		x = GetSystemMetrics(SM_CXSCREEN)/2-iWidth/2;
		y = GetSystemMetrics(SM_CYSCREEN)/2-iHeight/2;
	}

	/* Move and resize the window.  SWP_FRAMECHANGED causes the above SetWindowLong
	 * to take effect. */
	if( !SetWindowPos( g_hWndMain, HWND_NOTOPMOST, x, y, iWidth, iHeight, SWP_FRAMECHANGED|SWP_SHOWWINDOW ) )
		LOG->Warn( "%s", werr_ssprintf( GetLastError(), "SetWindowPos" ).c_str() );

	SetForegroundWindow( g_hWndMain );

	/* Pump messages quickly, to make sure the window is completely set up.
	 * If we don't do this, then starting up in a D3D fullscreen window may
	 * cause all other windows on the system to be resized. */
	MSG msg;
	while( PeekMessage( &msg, NULL, 0, 0, PM_NOREMOVE ) )
	{
		GetMessage( &msg, NULL, 0, 0 );
		DispatchMessage( &msg );
	}
}

/* Shut down the window, but don't reset the video mode. */
void GraphicsWindow::DestroyGraphicsWindow()
{
	if( g_HDC != NULL )
	{
		ReleaseDC( g_hWndMain, g_HDC );
		g_HDC = NULL;
	}

	CHECKPOINT;

	if( g_hWndMain != NULL )
	{
		DestroyWindow( g_hWndMain );
		g_hWndMain = NULL;
		CrashHandler::SetForegroundWindow( g_hWndMain );
	}

	CHECKPOINT;

	if( g_hIcon != NULL )
	{
		DestroyIcon( g_hIcon );
		g_hIcon = NULL;
	}

	CHECKPOINT;

	MSG msg;
	while( PeekMessage( &msg, NULL, 0, 0, PM_NOREMOVE ) )
	{
		CHECKPOINT;
		GetMessage( &msg, NULL, 0, 0 );
		CHECKPOINT;
		DispatchMessage( &msg );
	}

	CHECKPOINT;
}

void GraphicsWindow::Initialize( bool bD3D )
{
	/* A few things need to be handled differently for D3D. */
	g_bD3D = bD3D;

	AppInstance inst;
	do
	{
		const wstring wsClassName = RStringToWstring( g_sClassName );
		WNDCLASSW WindowClassW =
		{
			CS_OWNDC | CS_BYTEALIGNCLIENT,
			GraphicsWindow_WndProc,
			0,				/* cbClsExtra */
			0,				/* cbWndExtra */
			inst,			/* hInstance */
			NULL,			/* set icon later */
			LoadCursor( NULL, IDC_ARROW ),	/* default cursor */
			NULL,			/* hbrBackground */
			NULL,			/* lpszMenuName */
			wsClassName.c_str()	/* lpszClassName */
		}; 

		m_bWideWindowClass = true;
		if( RegisterClassW( &WindowClassW ) )
			break;

		WNDCLASS WindowClassA =
		{
			CS_OWNDC | CS_BYTEALIGNCLIENT,
			GraphicsWindow_WndProc,
			0,				/* cbClsExtra */
			0,				/* cbWndExtra */
			inst,			/* hInstance */
			NULL,			/* set icon later */
			LoadCursor( NULL, IDC_ARROW ),	/* default cursor */
			NULL,			/* hbrBackground */
			NULL,			/* lpszMenuName */
			g_sClassName	/* lpszClassName */
		}; 

		m_bWideWindowClass = false;
		if( !RegisterClassA( &WindowClassA ) )
			RageException::Throw( "%s", werr_ssprintf( GetLastError(), "RegisterClass" ).c_str() );
	} while(0);

	g_iQueryCancelAutoPlayMessage = RegisterWindowMessage( "QueryCancelAutoPlay" );
}

void GraphicsWindow::Shutdown()
{
	DestroyGraphicsWindow();

	/*
	 * Return to the desktop resolution, if needed.
	 *
	 * It'd be nice to not do this: Windows will do it when we quit, and if we're
	 * shutting down OpenGL to try D3D, this will cause extra mode switches.  However,
	 * we need to do this before displaying dialogs.
	 */
	ChangeDisplaySettings( NULL, 0 );

	AppInstance inst;
	UnregisterClass( g_sClassName, inst );
}

HDC GraphicsWindow::GetHDC()
{
	ASSERT( g_HDC != NULL );
	return g_HDC;
}

const VideoModeParams &GraphicsWindow::GetParams()
{
	return g_CurrentParams;
}

void GraphicsWindow::Update()
{
	MSG msg;
	while( PeekMessage( &msg, NULL, 0, 0, PM_NOREMOVE ) )
	{
		GetMessage( &msg, NULL, 0, 0 );
		DispatchMessage( &msg );
	}

	HOOKS->SetHasFocus( g_bHasFocus );

	if( g_bResolutionChanged && DISPLAY != NULL )
	{
		//LOG->Warn( "Changing resolution" );

		/* Let DISPLAY know that our resolution has changed.  (Note that ResolutionChanged()
		 * can come back here, so reset g_bResolutionChanged first.) */
		g_bResolutionChanged = false;
		DISPLAY->ResolutionChanged();
	}
}

HWND GraphicsWindow::GetHwnd()
{
	return g_hWndMain;
}

void GraphicsWindow::GetDisplayResolutions( DisplayResolutions &out )
{
	DEVMODE dm;
	ZERO( dm );
	dm.dmSize = sizeof(dm);
	int i=0;
	while(EnumDisplaySettings(NULL, i++, &dm))
	{
		if(ChangeDisplaySettings(&dm, CDS_TEST)==DISP_CHANGE_SUCCESSFUL)
		{
			DisplayResolution res = { dm.dmPelsWidth, dm.dmPelsHeight };
			out.insert( res );
		}
	}
}

float GraphicsWindow::GetMonitorAspectRatio()
{
	// There's no way to query the monitor for this, so assume that the
	// normal desktop resolution is using square pixels.
	DEVMODE dm;
	ZERO( dm );
	dm.dmSize = sizeof(dm);
	BOOL bResult = EnumDisplaySettings( NULL, ENUM_REGISTRY_SETTINGS, &dm );
	ASSERT( bResult );
	return dm.dmPelsWidth / (float)dm.dmPelsHeight;
}


/*
 * (c) 2004 Glenn Maynard
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
