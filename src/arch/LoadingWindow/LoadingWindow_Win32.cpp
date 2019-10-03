#include "global.h"
#include "RageUtil.h"

#include "LoadingWindow_Win32.h"
#include "RageFileManager.h"
#include "archutils/win32/WindowsResources.h"
#include "archutils/win32/WindowIcon.h"
#include "archutils/win32/ErrorStrings.h"
#include "arch/ArchHooks/ArchHooks.h"
#include <windows.h>
#include "CommCtrl.h"
#include "RageSurface_Load.h"
#include "RageSurface.h"
#include "RageSurfaceUtils.h"
#include "RageLog.h"
#include "ProductInfo.h"
#include "LocalizedString.h"

#include "RageSurfaceUtils_Zoom.h"
static HBITMAP g_hBitmap = nullptr;

/* Load a RageSurface into a GDI surface. */
static HBITMAP LoadWin32Surface( const RageSurface *pSplash, HWND hWnd )
{
	RageSurface *s = CreateSurface( pSplash->w, pSplash->h, 32,
		0xFF000000, 0x00FF0000, 0x0000FF00, 0 );
	RageSurfaceUtils::Blit( pSplash, s , -1, -1 );

	/* Resize the splash image to fit the dialog.  Stretch to fit horizontally,
	 * maintaining aspect ratio. */
	{
		RECT r;
		GetClientRect( hWnd, &r );

		int iWidth = r.right;
		float fRatio = (float) iWidth / s->w;
		int iHeight = lrintf( s->h * fRatio );

		RageSurfaceUtils::Zoom( s, iWidth, iHeight );
	}

	HDC hScreen = GetDC(nullptr);
	ASSERT_M( hScreen != nullptr, werr_ssprintf(GetLastError(), "hScreen") );

	HBITMAP bitmap = CreateCompatibleBitmap( hScreen, s->w, s->h );
	ASSERT_M( bitmap != nullptr, werr_ssprintf(GetLastError(), "CreateCompatibleBitmap") );

	HDC BitmapDC = CreateCompatibleDC( hScreen );
	SelectObject( BitmapDC, bitmap );

	/* This is silly, but simple.  We only do this once, on a small image. */
	for( int y = 0; y < s->h; ++y )
	{
		unsigned const char *line = ((unsigned char *) s->pixels) + (y * s->pitch);
		for( int x = 0; x < s->w; ++x )
		{
			unsigned const char *data = line + (x*s->format->BytesPerPixel);
			
			SetPixelV( BitmapDC, x, y, RGB( data[3], data[2], data[1] ) );
		}
	}

	SelectObject( BitmapDC, nullptr );
	DeleteObject( BitmapDC );

	ReleaseDC( nullptr, hScreen );

	delete s;
	return bitmap;
}

static HBITMAP LoadWin32Surface( RString sFile, HWND hWnd )
{
        RString error;
        RageSurface *pSurface = RageSurfaceUtils::LoadFile( sFile, error );
        if( pSurface == nullptr )
                return nullptr;

        HBITMAP ret = LoadWin32Surface( pSurface, hWnd );
        delete pSurface;
        return ret;
}

INT_PTR CALLBACK LoadingWindow_Win32::WndProc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam )
{
	switch( msg )
	{
	case WM_INITDIALOG:
		{
			vector<RString> vs;
			GetDirListing( "Data/splash*.png", vs, false, true );
			if( !vs.empty() )
				g_hBitmap = LoadWin32Surface( vs[0], hWnd );
		}
		if( g_hBitmap == nullptr )
			g_hBitmap = LoadWin32Surface( "Data/splash.bmp", hWnd );
		SendMessage( 
			GetDlgItem(hWnd,IDC_SPLASH), 
			STM_SETIMAGE, 
			(WPARAM) IMAGE_BITMAP, 
			(LPARAM) (HANDLE) g_hBitmap );
		SetWindowTextA( hWnd, PRODUCT_ID );
		break;

	case WM_DESTROY:
		DeleteObject( g_hBitmap );
		g_hBitmap = nullptr;
		break;
	}

	return FALSE;
}

void LoadingWindow_Win32::SetIcon( const RageSurface *pIcon )
{
	if( m_hIcon != nullptr )
		DestroyIcon( m_hIcon );

	m_hIcon = IconFromSurface( pIcon );
	if( m_hIcon != nullptr )
 		SetClassLongPtrA( hwnd, GCLP_HICON, reinterpret_cast<LONG_PTR>(m_hIcon) );
}

void LoadingWindow_Win32::SetSplash( const RageSurface *pSplash )
{
	if( g_hBitmap != nullptr )
	{
		DeleteObject( g_hBitmap );
		g_hBitmap = nullptr;
	}

	g_hBitmap = LoadWin32Surface( pSplash, hwnd );
	if( g_hBitmap != nullptr )
	{
		SendDlgItemMessage(
			hwnd, IDC_SPLASH,
			STM_SETIMAGE,
			(WPARAM) IMAGE_BITMAP,
			(LPARAM) (HANDLE) g_hBitmap
		);
	}
}

LoadingWindow_Win32::LoadingWindow_Win32()
{
	m_hIcon = nullptr;
	hwnd = CreateDialog( handle.Get(), MAKEINTRESOURCE(IDD_LOADING_DIALOG), nullptr, WndProc );
	ASSERT( hwnd != nullptr );
	for( unsigned i = 0; i < 3; ++i )
		text[i] = "ABC"; /* always set on first call */
	SetText( "" );
	Paint();
}

LoadingWindow_Win32::~LoadingWindow_Win32()
{
	if( hwnd )
		DestroyWindow( hwnd );
	if( m_hIcon != nullptr )
		DestroyIcon( m_hIcon );
}

void LoadingWindow_Win32::Paint()
{
	SendMessage( hwnd, WM_PAINT, 0, 0 );

	/* Process all queued messages since the last paint.  This allows the window to
	 * come back if it loses focus during load. */
	MSG msg;
	while( PeekMessage( &msg, hwnd, 0, 0, PM_NOREMOVE ) )
	{
		GetMessage(&msg, hwnd, 0, 0 );
		DispatchMessage( &msg );
	}
}

void LoadingWindow_Win32::SetText( RString sText )
{
	vector<RString> asMessageLines;
	split( sText, "\n", asMessageLines, false );
	while( asMessageLines.size() < 3 )
		asMessageLines.push_back( "" );

	const int msgid[] = { IDC_STATIC_MESSAGE1, IDC_STATIC_MESSAGE2, IDC_STATIC_MESSAGE3 };
	for( unsigned i = 0; i < 3; ++i )
	{
		if( text[i] == asMessageLines[i] )
			continue;
		text[i] = asMessageLines[i];

		HWND hwndItem = ::GetDlgItem( hwnd, msgid[i] );
		::SetWindowText( hwndItem, ConvertUTF8ToACP(asMessageLines[i]).c_str() );
	}
}

void LoadingWindow_Win32::SetProgress(const int progress)
{
	m_progress=progress;
	HWND hwndItem = ::GetDlgItem( hwnd, IDC_PROGRESS );
	::SendMessage(hwndItem,PBM_SETPOS,progress,0);
	Paint();
}

void LoadingWindow_Win32::SetTotalWork(const int totalWork)
{
	m_totalWork=totalWork;
	HWND hwndItem = ::GetDlgItem( hwnd, IDC_PROGRESS );
	SendMessage(hwndItem,PBM_SETRANGE32,0,totalWork);
}

void LoadingWindow_Win32::SetIndeterminate(bool indeterminate)
{
	m_indeterminate=indeterminate;

	HWND hwndItem = ::GetDlgItem( hwnd, IDC_PROGRESS );

	if(indeterminate) {
		SetWindowLong(hwndItem,GWL_STYLE, PBS_MARQUEE | GetWindowLong(hwndItem,GWL_STYLE));
		SendMessage(hwndItem,PBM_SETMARQUEE,1,0);
	} else {
		SendMessage(hwndItem,PBM_SETMARQUEE,0,0);
		SetWindowLong(hwndItem,GWL_STYLE, (~PBS_MARQUEE) & GetWindowLong(hwndItem,GWL_STYLE));
	}
}

/*
 * (c) 2001-2004 Chris Danford, Glenn Maynard
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
