#include "global.h"
#include "DialogDriver_Win32.h"
#include "RageUtil.h"
#if !defined(SMPACKAGE)
#include "CommonMetrics.h"
#endif
#include "ThemeManager.h"
#include "ProductInfo.h"

#include "archutils/win32/AppInstance.h"
#include "archutils/win32/ErrorStrings.h"
#include "archutils/win32/GotoURL.h"
#include "archutils/win32/RestartProgram.h"
#if !defined(SMPACKAGE)
#include "archutils/win32/WindowsResources.h"
#include "archutils/win32/GraphicsWindow.h"
#endif
#include "archutils/win32/DialogUtil.h"

#if defined(SMPACKAGE)
int __stdcall AfxMessageBox(LPCTSTR lpszText, UINT nType, UINT nIDHelp);
#endif

static bool g_bHush;
static RString g_sMessage;
static bool g_bAllowHush;

#if !defined(SMPACKAGE)
static BOOL CALLBACK OKWndProc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam )
{
	switch( msg )
	{
	case WM_INITDIALOG:
		{
			// Disable the parent window, like a modal MessageBox does.
			EnableWindow( GetParent(hWnd), FALSE );

			DialogUtil::LocalizeDialogAndContents( hWnd );

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
			RString sMessage = g_sMessage;
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
#endif

#if !defined(SMPACKAGE)
static HWND GetHwnd()
{
	return GraphicsWindow::GetHwnd();
}
#endif

#if !defined(SMPACKAGE)
static RString GetWindowTitle()
{
	RString s = CommonMetrics::WINDOW_TITLE.GetValue();
	return s;
}
#endif

void DialogDriver_Win32::OK( RString sMessage, RString sID )
{
	g_bAllowHush = sID != "";
	g_sMessage = sMessage;
	AppInstance handle;
#if !defined(SMPACKAGE)
	DialogBox( handle.Get(), MAKEINTRESOURCE(IDD_OK), ::GetHwnd(), OKWndProc );
#else
	::AfxMessageBox( ConvertUTF8ToACP(sMessage).c_str(), MB_OK, 0 );
#endif
	if( g_bAllowHush && g_bHush )
		Dialog::IgnoreMessage( sID );
}

#if !defined(SMPACKAGE)
static RString g_sErrorString;

static BOOL CALLBACK ErrorWndProc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam )
{
	switch( msg )
	{
	case WM_INITDIALOG:
		{
			DialogUtil::SetHeaderFont( hWnd, IDC_STATIC_HEADER_TEXT );
		
			// Set static text
			RString sMessage = g_sErrorString;
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
		break;
	case WM_CTLCOLORSTATIC:
		{
			HDC hdc = (HDC)wParam;
			HWND hwndStatic = (HWND)lParam;
			HBRUSH hbr = NULL;

			// TODO:  Change any attributes of the DC here
			switch( GetDlgCtrlID(hwndStatic) )
			{
			case IDC_STATIC_HEADER_TEXT:
			case IDC_STATIC_ICON:
				hbr = (HBRUSH)::GetStockObject(WHITE_BRUSH); 
				SetBkMode( hdc, OPAQUE );
				SetBkColor( hdc, RGB(255,255,255) );
				break;
			}

			// TODO:  Return a different brush if the default is not desired
			return (BOOL)hbr;
		}
	}
	return FALSE;
}
#endif

void DialogDriver_Win32::Error( RString sError, RString sID )
{
#if !defined(SMPACKAGE)
	g_sErrorString = sError;

	// throw up a pretty error dialog
	AppInstance handle;
	DialogBox( handle.Get(), MAKEINTRESOURCE(IDD_ERROR_DIALOG), NULL, ErrorWndProc );
#else
	::AfxMessageBox( ConvertUTF8ToACP(sError).c_str(), MB_OK, 0 );
#endif
}

Dialog::Result DialogDriver_Win32::AbortRetryIgnore( RString sMessage, RString ID )
{
	int iRet = 0;
#if !defined(SMPACKAGE)
	iRet = ::MessageBox(::GetHwnd(), ConvertUTF8ToACP(sMessage).c_str(), ConvertUTF8ToACP(::GetWindowTitle()).c_str(), MB_ABORTRETRYIGNORE|MB_DEFBUTTON3 );
#else
	iRet = ::AfxMessageBox( ConvertUTF8ToACP(sMessage).c_str(), MB_ABORTRETRYIGNORE|MB_DEFBUTTON3, 0 );
#endif
	switch( iRet )
	{
	case IDABORT:	return Dialog::abort;
	case IDRETRY:	return Dialog::retry;
	default:	ASSERT(0);
	case IDIGNORE:	return Dialog::ignore;
	}
} 

Dialog::Result DialogDriver_Win32::AbortRetry( RString sMessage, RString sID )
{
	int iRet = 0;
#if !defined(SMPACKAGE)
	iRet = ::MessageBox(::GetHwnd(), ConvertUTF8ToACP(sMessage).c_str(), ConvertUTF8ToACP(::GetWindowTitle()).c_str(), MB_RETRYCANCEL);
#else
	iRet = ::AfxMessageBox( ConvertUTF8ToACP(sMessage).c_str(), MB_RETRYCANCEL, 0 );
#endif
	switch( iRet )
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
