#include "global.h"
#include "DialogDriver_Win32.h"
#include "RageUtil.h"
#include "CommonMetrics.h"	// for WINDOW_TITLE
#include "ThemeManager.h"

#include "archutils/win32/AppInstance.h"
#include "archutils/win32/GotoURL.h"
#include "archutils/win32/RestartProgram.h"
#include "archutils/win32/WindowsResources.h"
#include "archutils/win32/GraphicsWindow.h"

static bool g_Hush;
static CString g_sMessage;
static bool g_AllowHush;

static BOOL CALLBACK OKWndProc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam )
{
	switch( msg )
	{
	case WM_INITDIALOG:
		{
			// Disable parent, like a modal MessageBox does.
			EnableWindow( GetParent(hWnd), FALSE );

			g_Hush = false;
			HWND hush = GetDlgItem( hWnd, IDC_HUSH );
	        int style = GetWindowLong(hush, GWL_STYLE);

			if( g_AllowHush )
				style |= WS_VISIBLE;
			else
				style &= ~WS_VISIBLE;
	        SetWindowLong( hush, GWL_STYLE, style );

			// Set static text.
			CString sMessage = g_sMessage;
			sMessage.Replace( "\n", "\r\n" );
			SetWindowText( GetDlgItem(hWnd, IDC_MESSAGE), sMessage );
			
			// Focus is on any of the controls in the dialog by default.
			// I'm not sure why.  Set focus to the button manually. -Chris
			SetFocus( GetDlgItem(hWnd, IDOK) );
		}
		break;
	case WM_DESTROY:
		{
			// Re-enable parent
			EnableWindow( GetParent(hWnd), TRUE );
		}
		break;
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDOK:
			g_Hush = !!IsDlgButtonChecked(hWnd, IDC_HUSH);
			/* fall through */
		case IDCANCEL:
			EndDialog( hWnd, 0 );
			break;
		}
	}
	return FALSE;
}


void DialogDriver_Win32::OK( CString sMessage, CString ID )
{
	g_AllowHush = ID != "";
	g_sMessage = sMessage;
	AppInstance handle;
	DialogBox(handle.Get(), MAKEINTRESOURCE(IDD_OK), GraphicsWindow::GetHwnd(), OKWndProc);
	if( g_AllowHush && g_Hush )
		Dialog::IgnoreMessage( ID );
}

static CString g_sErrorString;

static BOOL CALLBACK ErrorWndProc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam )
{
	switch( msg )
	{
	case WM_INITDIALOG:
		{
			// Set static text
			CString sMessage = g_sErrorString;
			sMessage.Replace( "\n", "\r\n" );
			SetWindowText( GetDlgItem(hWnd, IDC_EDIT_ERROR), sMessage );
		}
		break;
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDC_BUTTON_VIEW_LOG:
			{
				PROCESS_INFORMATION pi;
				STARTUPINFO	si;
				ZeroMemory( &si, sizeof(si) );

				CreateProcess(
					NULL,		// pointer to name of executable module
					"notepad.exe log.txt",		// pointer to command line string
					NULL,  // process security attributes
					NULL,   // thread security attributes
					false,  // handle inheritance flag
					0, // creation flags
					NULL,  // pointer to new environment block
					NULL,   // pointer to current directory name
					&si,  // pointer to STARTUPINFO
					&pi  // pointer to PROCESS_INFORMATION
				);
			}
			break;
		case IDC_BUTTON_REPORT:
			GotoURL( "http://sourceforge.net/tracker/?func=add&group_id=37892&atid=421366" );
			break;
		case IDC_BUTTON_RESTART:
			Win32RestartProgram();
			/* not reached */
			ASSERT( 0 );

			EndDialog( hWnd, 0 );
			break;

		case IDOK:
			EndDialog( hWnd, 0 );
			break;
		}
	}
	return FALSE;
}

void DialogDriver_Win32::Error( CString error, CString ID )
{
	g_sErrorString = error;
 	// throw up a pretty error dialog
	AppInstance handle;
	DialogBox(handle.Get(), MAKEINTRESOURCE(IDD_ERROR_DIALOG),
		NULL, ErrorWndProc);
}

Dialog::Result DialogDriver_Win32::AbortRetryIgnore( CString sMessage, CString ID )
{
	CString sWindowTitle = WINDOW_TITLE.IsLoaded() ? WINDOW_TITLE.GetValue() : "";

	switch( MessageBox(GraphicsWindow::GetHwnd(), sMessage, sWindowTitle, MB_ABORTRETRYIGNORE|MB_DEFBUTTON3 ) )
	{
	case IDABORT:	return Dialog::abort;
	case IDRETRY:	return Dialog::retry;
	default:	ASSERT(0);
	case IDIGNORE:	return Dialog::ignore;
	}
} 

Dialog::Result DialogDriver_Win32::AbortRetry( CString sMessage, CString ID )
{
	CString sWindowTitle = WINDOW_TITLE.IsLoaded() ? WINDOW_TITLE.GetValue() : "";

	switch( MessageBox(GraphicsWindow::GetHwnd(), sMessage, sWindowTitle, MB_RETRYCANCEL ) )
	{
	case IDRETRY:	return Dialog::retry;
	default:	ASSERT(0);
	case IDCANCEL:	return Dialog::abort;
	}
} 

/*
 * (c) 2003-2004 Glenn Maynard, Chris Danford
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
