#include "global.h"
#include "ArchHooks_Win32.h"
#include "RageUtil.h"
#include "PrefsManager.h"

#include "resource.h"

#include "archutils/win32/AppInstance.h"
#include "archutils/win32/crash.h"
#include "archutils/win32/DebugInfoHunt.h"
#include "archutils/win32/GotoURL.h"
#include "archutils/win32/RestartProgram.h"
#include "ProductInfo.h"

#include "RageThreads.h"
ArchHooks_Win32::ArchHooks_Win32()
{
	SetUnhandledExceptionFilter(CrashHandler);
	TimeCritMutex = new RageMutex("TimeCritMutex");

	/* Disable critical errors, and handle them internally.  We never want the
	 * "drive not ready", etc. dialogs to pop up. */
	SetErrorMode( SetErrorMode(0) | SEM_FAILCRITICALERRORS );
}

ArchHooks_Win32::~ArchHooks_Win32()
{
	delete TimeCritMutex;
}

void ArchHooks_Win32::DumpDebugInfo()
{
	/* This is a good time to do the debug search: before we actually
	 * start OpenGL (in case something goes wrong). */
	SearchForDebugInfo();
}

static CString g_sMessage;
static bool g_AllowHush;
static bool g_Hush;
static BOOL CALLBACK OKWndProc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam )
{
	switch( msg )
	{
	case WM_INITDIALOG:
		{
			g_Hush = false;
			CString sMessage = g_sMessage;

			sMessage.Replace( "\n", "\r\n" );
			HWND hush = GetDlgItem( hWnd, IDC_HUSH );
	        int style = GetWindowLong(hush, GWL_STYLE);

			if( g_AllowHush )
				style |= WS_VISIBLE;
			else
				style &= ~WS_VISIBLE;
	        SetWindowLong( hush, GWL_STYLE, style );

			SendDlgItemMessage( 
				hWnd, 
				IDC_MESSAGE, 
				WM_SETTEXT, 
				0, 
				(LPARAM)(LPCTSTR)sMessage
				);
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




void ArchHooks_Win32::MessageBoxOKPrivate( CString sMessage, CString ID )
{
	g_AllowHush = ID != "";
	g_sMessage = sMessage;
	AppInstance handle;
	DialogBox(handle.Get(), MAKEINTRESOURCE(IDD_OK), NULL, OKWndProc);
	if( g_AllowHush && g_Hush )
		IgnoreMessage( ID );
}

static CString g_sErrorString;

static BOOL CALLBACK ErrorWndProc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam )
{
	switch( msg )
	{
	case WM_INITDIALOG:
		{
			CString sMessage = g_sErrorString;

			sMessage.Replace( "\n", "\r\n" );
			
			SendDlgItemMessage( 
				hWnd, 
				IDC_EDIT_ERROR, 
				WM_SETTEXT, 
				0, 
				(LPARAM)(LPCTSTR)sMessage
				);
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

void ArchHooks_Win32::MessageBoxErrorPrivate( CString error, CString ID )
{
	g_sErrorString = error;
 	// throw up a pretty error dialog
	AppInstance handle;
	DialogBox(handle.Get(), MAKEINTRESOURCE(IDD_ERROR_DIALOG),
		NULL, ErrorWndProc);
}

ArchHooks::MessageBoxResult ArchHooks_Win32::MessageBoxAbortRetryIgnorePrivate( CString sMessage, CString ID )
{
	switch( MessageBox(NULL, sMessage, PRODUCT_NAME, MB_ABORTRETRYIGNORE|MB_DEFBUTTON2 ) )
	{
	case IDABORT:	return abort;
	case IDRETRY:	return retry;
	default:	ASSERT(0);
	case IDIGNORE:	return ignore;
	}
} 

ArchHooks::MessageBoxResult ArchHooks_Win32::MessageBoxRetryCancelPrivate( CString sMessage, CString ID )
{
	switch( MessageBox(NULL, sMessage, PRODUCT_NAME, MB_RETRYCANCEL ) )
	{
	case IDRETRY:	return retry;
	default:	ASSERT(0);
	case IDCANCEL:	return cancel;
	}
} 

void ArchHooks_Win32::RestartProgram()
{
	Win32RestartProgram();
}

void ArchHooks_Win32::EnterTimeCriticalSection()
{
	TimeCritMutex->Lock();

	OldThreadPriority = GetThreadPriority( GetCurrentThread() );
	SetThreadPriority( GetCurrentThread(), THREAD_PRIORITY_TIME_CRITICAL );
}

void ArchHooks_Win32::ExitTimeCriticalSection()
{
	SetThreadPriority( GetCurrentThread(), OldThreadPriority );
	OldThreadPriority = 0;
	TimeCritMutex->Unlock();
}

void ArchHooks_Win32::SetTime( tm newtime )
{
	SYSTEMTIME st;
	ZERO( st );
	st.wYear = (WORD)newtime.tm_year+1900;
    st.wMonth = (WORD)newtime.tm_mon+1;
    st.wDay = (WORD)newtime.tm_mday;
    st.wHour = (WORD)newtime.tm_hour;
    st.wMinute = (WORD)newtime.tm_min;
    st.wSecond = (WORD)newtime.tm_sec;
    st.wMilliseconds = 0;
	SetLocalTime( &st ); 
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
