#include "../../stdafx.h"
#include "../../RageUtil.h"

#include "LoadingWindow_Win32.h"
#include "../../resource.h"
#include <windows.h>

BOOL CALLBACK LoadingWindow_Win32::WndProc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam )
{
	return FALSE;
}

LoadingWindow_Win32::LoadingWindow_Win32()
{
	handle = NULL;
	hwnd = NULL;

	/* Little trick to get an HINSTANCE of ourself without having access to the hwnd ... */
	TCHAR szFullAppPath[MAX_PATH];
	GetModuleFileName(NULL, szFullAppPath, MAX_PATH);
	handle = LoadLibrary(szFullAppPath);

	hwnd = CreateDialog(handle,	MAKEINTRESOURCE(IDD_LOADING_DIALOG), NULL, WndProc);

	SetText("Initializing hardware...");
	Paint();
}

LoadingWindow_Win32::~LoadingWindow_Win32()
{
	if(hwnd)
		EndDialog(hwnd, 0);
	if(handle)
		FreeLibrary(handle);
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

	SendDlgItemMessage( hwnd, IDC_STATIC_MESSAGE1, WM_SETTEXT, 0, 
		(LPARAM)(LPCTSTR)asMessageLines[0]);
	SendDlgItemMessage( hwnd, IDC_STATIC_MESSAGE2, WM_SETTEXT, 0, 
		(LPARAM)(LPCTSTR)(asMessageLines.size()>=2 ? asMessageLines[1] : ""));
	SendDlgItemMessage( hwnd, IDC_STATIC_MESSAGE3, WM_SETTEXT, 0, 
		(LPARAM)(LPCTSTR)(asMessageLines.size()>=3 ? asMessageLines[2] : ""));
}

/*
 * Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
 *
 * Chris Danford
 * Glenn Maynard
 */
