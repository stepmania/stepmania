#include "global.h"
#include "DialogDriver_Win32.h"
#include "RageUtil.h"
#if !defined(SMPACKAGE)
#include "CommonMetrics.h"
#endif
#include "ThemeManager.h"
#include "ProductInfo.h"

#include "archutils/win32/AppInstance.h"
#include "archutils/win32/GotoURL.h"
#include "archutils/win32/RestartProgram.h"
#include "archutils/win32/WindowsResources.h"
#if !defined(SMPACKAGE)
#include "archutils/win32/GraphicsWindow.h"
#endif

static bool g_bHush;
static CString g_sMessage;
static bool g_bAllowHush;

static BOOL CALLBACK OKWndProc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam )
{
	switch( msg )
	{
	case WM_INITDIALOG:
		{
			// Disable the parent window, like a modal MessageBox does.
			EnableWindow( GetParent(hWnd), FALSE );

			// Hide or display "Don't show this message."
			g_bHush = false;
			HWND hHushButton = GetDlgItem( hWnd, IDC_HUSH );
	        int iStyle = GetWindowLong( hHushButton, GWL_STYLE );

			if( g_bAllowHush )
				iStyle |= WS_VISIBLE;
			else
				iStyle &= ~WS_VISIBLE;
	        SetWindowLong( hHushButton, GWL_STYLE, iStyle );

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
		// Re-enable the parent window.
		EnableWindow( GetParent(hWnd), TRUE );
		break;

	case WM_COMMAND:
		switch( LOWORD(wParam) )
		{
		case IDOK:
			g_bHush = !!IsDlgButtonChecked( hWnd, IDC_HUSH );
			/* fall through */
		case IDCANCEL:
			EndDialog( hWnd, 0 );
			break;
		}
	}
	return FALSE;
}

static HWND GetHwnd()
{
#if !defined(SMPACKAGE)
	return GraphicsWindow::GetHwnd();
#else
	return NULL;
#endif
}

static CString GetWindowTitle()
{
#if !defined(SMPACKAGE)
	return CommonMetrics::WINDOW_TITLE.IsLoaded() ? CommonMetrics::WINDOW_TITLE.GetValue() : PRODUCT_NAME;
#else
	return PRODUCT_NAME;
#endif
}

void DialogDriver_Win32::OK( CString sMessage, CString sID )
{
	g_bAllowHush = sID != "";
	g_sMessage = sMessage;
	AppInstance handle;
	DialogBox( handle.Get(), MAKEINTRESOURCE(IDD_OK), ::GetHwnd(), OKWndProc );
	if( g_bAllowHush && g_bHush )
		Dialog::IgnoreMessage( sID );
}

#if !defined(SMPACKAGE)
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
		switch( LOWORD(wParam) )
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
			GotoURL( REPORT_BUG_URL );
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
#endif

void DialogDriver_Win32::Error( CString sError, CString sID )
{
#if !defined(SMPACKAGE)
	g_sErrorString = sError;

	// throw up a pretty error dialog
	AppInstance handle;
	DialogBox( handle.Get(), MAKEINTRESOURCE(IDD_ERROR_DIALOG), NULL, ErrorWndProc );
#endif
}

Dialog::Result DialogDriver_Win32::AbortRetryIgnore( CString sMessage, CString ID )
{
	switch( MessageBox(::GetHwnd(), sMessage, ::GetWindowTitle(), MB_ABORTRETRYIGNORE|MB_DEFBUTTON3 ) )
	{
	case IDABORT:	return Dialog::abort;
	case IDRETRY:	return Dialog::retry;
	default:	ASSERT(0);
	case IDIGNORE:	return Dialog::ignore;
	}
} 

Dialog::Result DialogDriver_Win32::AbortRetry( CString sMessage, CString sID )
{
	switch( MessageBox(::GetHwnd(), sMessage, ::GetWindowTitle(), MB_RETRYCANCEL) )
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
