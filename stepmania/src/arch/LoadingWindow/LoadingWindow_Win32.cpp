#include "../../global.h"
#include "../../RageUtil.h"

#include "LoadingWindow_Win32.h"
#include "RageFileManager.h"
#include "archutils/win32/WindowsResources.h"
#include <windows.h>
#include "RageSurface_Load.h"
#include "RageSurface.h"
#include "RageSurfaceUtils.h"

static HBITMAP g_hBitmap = NULL;

/* Load a file into a GDI surface. */
HBITMAP LoadWin32Surface( CString fn )
{
	CString error;
	RageSurface *s = RageSurfaceUtils::LoadFile( fn, error );
	if( s == NULL )
		return NULL;

	RageSurfaceUtils::ConvertSurface( s, s->w, s->h, 32, 0xFF000000, 0x00FF0000, 0x0000FF00, 0 );

	HBITMAP bitmap = CreateCompatibleBitmap( GetDC(NULL), s->w, s->h );
	ASSERT( bitmap );

	HDC BitmapDC = CreateCompatibleDC( GetDC(NULL) );
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

	SelectObject( BitmapDC, NULL );
	DeleteObject( BitmapDC );

	delete s;
	return bitmap;
}

BOOL CALLBACK LoadingWindow_Win32::WndProc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam )
{
	switch( msg )
	{
	case WM_INITDIALOG:
		g_hBitmap = LoadWin32Surface( "Data/splash.png" );
		if( g_hBitmap == NULL )
			g_hBitmap = LoadWin32Surface( "Data/splash.bmp" );
		SendMessage( 
			GetDlgItem(hWnd,IDC_SPLASH), 
			STM_SETIMAGE, 
			(WPARAM) IMAGE_BITMAP, 
			(LPARAM) (HANDLE) g_hBitmap );
		break;

	case WM_DESTROY:
		DeleteObject( g_hBitmap );
		g_hBitmap = NULL;
		break;
	}

	return FALSE;
}

LoadingWindow_Win32::LoadingWindow_Win32()
{
	hwnd = CreateDialog(handle.Get(), MAKEINTRESOURCE(IDD_LOADING_DIALOG), NULL, WndProc);
	for( unsigned i = 0; i < 3; ++i )
		text[i] = "XXX"; /* always set on first call */
	SetText("Initializing hardware...");
	Paint();
}

LoadingWindow_Win32::~LoadingWindow_Win32()
{
	if(hwnd)
		DestroyWindow( hwnd );
}

void LoadingWindow_Win32::Paint()
{
	SendMessage( hwnd, WM_PAINT, 0, 0 );

	/* Process all queued messages since the last paint.  This allows the window to
	 * come back if it loses focus during load. */
	MSG msg;
	while( PeekMessage( &msg, hwnd, 0, 0, PM_NOREMOVE ) ) {
		GetMessage(&msg, hwnd, 0, 0 );
		DispatchMessage( &msg );
	}
}

void LoadingWindow_Win32::SetText(CString str)
{
	CStringArray asMessageLines;
	split( str, "\n", asMessageLines, false );
	while( asMessageLines.size() < 3 )
		asMessageLines.push_back( "" );
	
	const int msgs[] = { IDC_STATIC_MESSAGE1, IDC_STATIC_MESSAGE2, IDC_STATIC_MESSAGE3 };
	for( unsigned i = 0; i < 3; ++i )
	{
		if( text[i] == asMessageLines[i] )
			continue;
		text[i] = asMessageLines[i];

		SendDlgItemMessage( hwnd, msgs[i], WM_SETTEXT, 0, 
			(LPARAM)asMessageLines[i].c_str());
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
