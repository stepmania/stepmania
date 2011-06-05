#include "global.h"
#include "RageUtil.h"

#include "LoadingWindow_Win32.h"
#include "RageFileManager.h"
#include "archutils/win32/WindowsResources.h"
#include "archutils/win32/WindowIcon.h"
#include "archutils/win32/ErrorStrings.h"
#include <windows.h>
#include "CommCtrl.h"
#include "RageSurface_Load.h"
#include "RageSurface.h"
#include "RageSurfaceUtils.h"
#include "RageLog.h"
#include "ProductInfo.h"
#include "LocalizedString.h"

#include "RageSurfaceUtils_Zoom.h"
static HBITMAP g_hBitmap = NULL;

#pragma comment(linker,"\"/manifestdependency:type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")


/* Load a RageSurface into a GDI surface. */
static HBITMAP LoadWin32Surface( RageSurface *&s )
{
	RageSurfaceUtils::ConvertSurface( s, s->w, s->h, 32, 0xFF000000, 0x00FF0000, 0x0000FF00, 0 );

	HDC hScreen = GetDC(NULL);
	ASSERT_M( hScreen, werr_ssprintf(GetLastError(), "hScreen") );

	HBITMAP bitmap = CreateCompatibleBitmap( hScreen, s->w, s->h );
	ASSERT_M( bitmap, werr_ssprintf(GetLastError(), "CreateCompatibleBitmap") );

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

	SelectObject( BitmapDC, NULL );
	DeleteObject( BitmapDC );

	ReleaseDC( NULL, hScreen );

	return bitmap;
}

static HBITMAP LoadWin32Surface( RString sFile, HWND hWnd )
{
	RString error;
	RageSurface *pSurface = RageSurfaceUtils::LoadFile( sFile, error );
	if( pSurface == NULL )
		return NULL;

	/* Resize the splash image to fit the dialog.  Stretch to fit horizontally,
	 * maintaining aspect ratio. */
	{
		RECT r;
		GetClientRect( hWnd, &r );

		int iWidth = r.right;
		float fRatio = (float) iWidth / pSurface->w;
		int iHeight = lrintf( pSurface->h * fRatio );

		RageSurfaceUtils::Zoom( pSurface, iWidth, iHeight );
	}

	HBITMAP ret = LoadWin32Surface( pSurface );
	delete pSurface;
	return ret;
}

INT_PTR CALLBACK LoadingWindow_Win32::DlgProc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam )
{

	LoadingWindow_Win32 *self;

	if(msg==WM_INITDIALOG) {
		self=(LoadingWindow_Win32 *)lParam;
		SetWindowLong(hWnd,DWL_USER,(LONG)self);
	} else {
		self=(LoadingWindow_Win32 *)GetWindowLong(hWnd,DWL_USER);
	}

	switch( msg )
	{
	case WM_INITDIALOG:
		{
			vector<RString> vs;
			GetDirListing( "Data/splash*.png", vs, false, true );
			if( !vs.empty() )
				g_hBitmap = LoadWin32Surface( vs[0], hWnd );
		}
		if( g_hBitmap == NULL )
			g_hBitmap = LoadWin32Surface( "Data/splash.bmp", hWnd );
		SendMessage( 
			GetDlgItem(hWnd,IDC_SPLASH), 
			STM_SETIMAGE, 
			(WPARAM) IMAGE_BITMAP, 
			(LPARAM) (HANDLE) g_hBitmap );
		SetWindowTextA( hWnd, PRODUCT_ID );

		break;

	case WM_CLOSE:
		return FALSE;

	case WM_DESTROY:
		DeleteObject( g_hBitmap );
		g_hBitmap = NULL;
		self->runMessageLoop=false;
		self->hwnd=NULL;
		return TRUE;
		break;

	case WM_APP:
		DestroyWindow(hWnd);
		self->runMessageLoop=false;
		ExitThread(0);
		return TRUE;
		break;
	}

	return FALSE;
}

void LoadingWindow_Win32::SetIcon( const RageSurface *pIcon )
{
	if( m_hIcon != NULL )
		DestroyIcon( m_hIcon );

	m_hIcon = IconFromSurface( pIcon );
	if( m_hIcon != NULL )
		SetClassLong( hwnd, GCL_HICON, (LONG) m_hIcon );
}

LoadingWindow_Win32::LoadingWindow_Win32()
{
	INITCOMMONCONTROLSEX cceData;
	cceData.dwSize=sizeof(INITCOMMONCONTROLSEX);
	cceData.dwICC=ICC_PROGRESS_CLASS;
	InitCommonControlsEx(&cceData);

	m_hIcon = NULL;
	
	runMessageLoop=true;

	guiReadyEvent=CreateEvent(NULL,FALSE,FALSE,NULL);

	pumpThread=CreateThread(NULL, NULL,	MessagePump, (void *)this, 0,	&pumpThreadId);

	WaitForSingleObject(guiReadyEvent,INFINITE);

	for( unsigned i = 0; i < 3; ++i )
		text[i] = "ABC"; /* always set on first call */
	SetText( "" );
}

LoadingWindow_Win32::~LoadingWindow_Win32()
{
	SendMessage(hwnd,WM_APP,0,0);
	//SendMessage(hwnd,WM_NULL,0,0);
	WaitForSingleObject(pumpThread,INFINITE);
	if(guiReadyEvent) 
		CloseHandle(guiReadyEvent);
	if( m_hIcon != NULL )
		DestroyIcon( m_hIcon );
}

DWORD WINAPI LoadingWindow_Win32::MessagePump(LPVOID thisAsVoidPtr)
{
	LoadingWindow_Win32 *self=(LoadingWindow_Win32 *)thisAsVoidPtr;

	self->hwnd = CreateDialogParam( self->handle.Get(), MAKEINTRESOURCE(IDD_LOADING_DIALOG), NULL, DlgProc, (LPARAM)thisAsVoidPtr);

	SetEvent(self->guiReadyEvent);

	// Run the message loop in a separate thread to keep the gui responsive during the loading
	MSG msg;
	while(self->runMessageLoop && GetMessage(&msg, self->hwnd, 0, 0 ) )
	{
		if(IsDialogMessage(self->hwnd,&msg)) continue;
		DispatchMessage( &msg );
	}

	return msg.wParam;
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
}

void LoadingWindow_Win32::SetTotalWork(const int totalWork)
{
	m_totalWork=totalWork;
	HWND hwndItem = ::GetDlgItem( hwnd, IDC_PROGRESS );
	SendMessage(hwndItem,PBM_SETRANGE32,0,totalWork);
}

void LoadingWindow_Win32::SetIndeterminate(bool indeterminate) {
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
