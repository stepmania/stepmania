#include "global.h"
#include "ArchHooks_Win32.h"
#include "RageUtil.h"
#include "PrefsManager.h"

#include "resource.h"

#include "archutils/win32/AppInstance.h"
#include "archutils/win32/tls.h"
#include "archutils/win32/crash.h"
#include "archutils/win32/DebugInfoHunt.h"

ArchHooks_Win32::ArchHooks_Win32()
{
	SetUnhandledExceptionFilter(CrashHandler);
	InitThreadData("Main thread");
	VDCHECKPOINT;
}

void ArchHooks_Win32::Log(CString str, bool important)
{
	/* It's OK to send to both of these; it'll let us know when important
	 * events occurred in crash dumps if they're recent. */
	if(important)
		StaticLog(str);

	CrashLog(str);
}

void ArchHooks_Win32::AdditionalLog(CString str)
{
	AdditionalLog(str);
}

void ArchHooks_Win32::DumpDebugInfo()
{
	/* This is a good time to do the debug search: before we actually
	 * start OpenGL (in case something goes wrong). */
	SearchForDebugInfo();
}

bool ArchHooks_Win32::MessageIsIgnored( CString ID )
{
	vector<CString> list;
	split( PREFSMAN->m_sIgnoredMessageWindows, ",", list );
	for( unsigned i = 0; i < list.size(); ++i )
		if( !ID.CompareNoCase(list[i]) )
			return true;
	return false;
}

void ArchHooks_Win32::IgnoreMessage( CString ID )
{
	if( MessageIsIgnored(ID) )
		return;
	vector<CString> list;
	split( PREFSMAN->m_sIgnoredMessageWindows, ",", list );
	list.push_back( ID );
	PREFSMAN->m_sIgnoredMessageWindows = join( ",", list );
	PREFSMAN->SaveGlobalPrefsToDisk();
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




void ArchHooks_Win32::MessageBoxOK( CString sMessage, CString ID )
{
	g_AllowHush = ID != "";
	if( g_AllowHush && MessageIsIgnored( ID ) )
		return;
	g_sMessage = sMessage;
	AppInstance handle;
	DialogBox(handle.Get(), MAKEINTRESOURCE(IDD_OK), NULL, OKWndProc);
	if( g_AllowHush && g_Hush )
		IgnoreMessage( ID );
}

ArchHooks::MessageBoxResult ArchHooks_Win32::MessageBoxAbortRetryIgnore( CString sMessage, CString ID )
{
	switch( MessageBox(NULL, sMessage, "StepMania", MB_ABORTRETRYIGNORE ) )
	{
	case IDABORT:	return abort;
	case IDRETRY:	return retry;
	default:	ASSERT(0);
	case IDIGNORE:	return ignore;
	}
} 

/*
 * Copyright (c) 2002-2003 by the person(s) listed below.  All rights reserved.
 *
 * Glenn Maynard
 */
