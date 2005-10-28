#include "global.h"
#include "ArchHooks_Win32.h"
#include "RageUtil.h"
#include "RageLog.h"
#include "RageThreads.h"
#include "PrefsManager.h"
#include "ProductInfo.h"

#include "archutils/win32/AppInstance.h"
#include "archutils/win32/crash.h"
#include "archutils/win32/DebugInfoHunt.h"
#include "archutils/win32/GotoURL.h"
#include "archutils/win32/RestartProgram.h"
#include "archutils/win32/VideoDriverInfo.h"
#include "archutils/win32/WindowsResources.h"

#include <mmsystem.h>
#if defined(_MSC_VER)
#pragma comment(lib, "winmm.lib") // for timeGetTime
#endif

ArchHooks_Win32::ArchHooks_Win32()
{
	SetUnhandledExceptionFilter(CrashHandler);
	TimeCritMutex = new RageMutex("TimeCritMutex");

	/* Disable critical errors, and handle them internally.  We never want the
	 * "drive not ready", etc. dialogs to pop up. */
	SetErrorMode( SetErrorMode(0) | SEM_FAILCRITICALERRORS );

	/* Windows boosts priority on keyboard input, among other things.  Disable that for
	 * the main thread. */
	SetThreadPriorityBoost( GetCurrentThread(), TRUE );
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

	CheckVideoDriver();
}

static CString g_sDriverVersion, g_sURL;
static bool g_Hush;
static BOOL CALLBACK DriverWndProc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam )
{
	switch( msg )
	{
	case WM_INITDIALOG:
		{
			g_Hush = false;
			CString sMessage = ssprintf(
				"The graphics drivers you are running, %s, are very old and are known "
				"to cause problems.  Upgrading to the latest drivers is recommended.\n",
					g_sDriverVersion.c_str() );

			sMessage.Replace( "\n", "\r\n" );

			SendDlgItemMessage( hWnd, IDC_MESSAGE, WM_SETTEXT, 
				0, (LPARAM)(LPCTSTR)sMessage );
		}
		break;
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDOK:
			g_Hush = !!IsDlgButtonChecked(hWnd, IDC_HUSH);
			GotoURL( g_sURL );
			EndDialog( hWnd, 1 );
			break;

		case IDCANCEL:
			g_Hush = !!IsDlgButtonChecked(hWnd, IDC_HUSH);
			EndDialog( hWnd, 0 );
			break;
		}
	}
	return FALSE;
}

static bool MessageIsIgnored( CString ID )
{
	vector<CString> list;
	split( PREFSMAN->m_sIgnoredMessageWindows, ",", list );
	for( unsigned i = 0; i < list.size(); ++i )
		if( !ID.CompareNoCase(list[i]) )
			return true;
	return false;
}

static void IgnoreMessage( CString ID )
{
	if( ID == "" )

	if( MessageIsIgnored(ID) )
		return;

	vector<CString> list;
	split( PREFSMAN->m_sIgnoredMessageWindows, ",", list );
	list.push_back( ID );
	PREFSMAN->m_sIgnoredMessageWindows.Set( join(",",list) );
	PREFSMAN->SavePrefsToDisk();
}

/*
 * This simply does a few manual checks for known bad driver versions.  Only nag the
 * user if it's a driver that we receive many complaints about--we don't want to
 * tell users to upgrade if there's no problem, since it's likely to cause new problems.
 */
void ArchHooks_Win32::CheckVideoDriver()
{
	if( MessageIsIgnored( "OLD_DRIVER_WARNING" ) )
		return;

	CString sPrimaryDeviceName = GetPrimaryVideoName();
	if( sPrimaryDeviceName == "" )
		return;

	g_sDriverVersion = "";
	for( int i=0; true; i++ )
	{
		VideoDriverInfo info;
		if( !GetVideoDriverInfo(i, info) )
			break;
		if( info.sDescription != sPrimaryDeviceName )
			continue;

		/* "IntelR 810 Chipset Graphics Driver PV 2.1".  There are a lot of crash reports
		 * with people using this version. */
		if( Regex( "Intel.* 810 Chipset Graphics Driver PV 2.1").Compare( info.sDescription ) )
		{
			g_sDriverVersion = info.sDescription;
			g_sURL = "http://www.intel.com/design/software/drivers/platform/810.htm";
			break;
		}
	}

	if( g_sDriverVersion == "" )
		return;

	bool bExit = !!DialogBox( AppInstance(), MAKEINTRESOURCE(IDD_DRIVER), NULL, DriverWndProc );

	if( g_Hush )
		IgnoreMessage( "OLD_DRIVER_WARNING" );
	if( bExit )
		ExitProcess(0);
}


void ArchHooks_Win32::RestartProgram()
{
	Win32RestartProgram();
}

static CString LangIdToString( LANGID l )
{
	switch( PRIMARYLANGID(l) )
	{
	case LANG_ARABIC: return "AR";
	case LANG_BULGARIAN: return "BG";
	case LANG_CATALAN: return "CA";
	case LANG_CHINESE: return "ZH";
	case LANG_CZECH: return "CS";
	case LANG_DANISH: return "DA";
	case LANG_GERMAN: return "DE";
	case LANG_GREEK: return "EL";
	case LANG_SPANISH: return "ES";
	case LANG_FINNISH: return "FI";
	case LANG_FRENCH: return "FR";
	case LANG_HEBREW: return "IW";
	case LANG_HUNGARIAN: return "HU";
	case LANG_ICELANDIC: return "IS";
	case LANG_ITALIAN: return "IT";
	case LANG_JAPANESE: return "JA";
	case LANG_KOREAN: return "KO";
	case LANG_DUTCH: return "NL";
	case LANG_NORWEGIAN: return "NO";
	case LANG_POLISH: return "PL";
	case LANG_PORTUGUESE: return "PT";
	case LANG_ROMANIAN: return "RO";
	case LANG_RUSSIAN: return "RU";
	case LANG_CROATIAN: return "HR";
	// case LANG_SERBIAN: return "SR"; // same as LANG_CROATIAN?
	case LANG_SLOVAK: return "SK";
	case LANG_ALBANIAN: return "SQ";
	case LANG_SWEDISH: return "SV";
	case LANG_THAI: return "TH";
	case LANG_TURKISH: return "TR";
	case LANG_URDU: return "UR";
	case LANG_INDONESIAN: return "IN";
	case LANG_UKRAINIAN: return "UK";
	case LANG_SLOVENIAN: return "SL";
	case LANG_ESTONIAN: return "ET";
	case LANG_LATVIAN: return "LV";
	case LANG_LITHUANIAN: return "LT";
	case LANG_VIETNAMESE: return "VI";
	case LANG_ARMENIAN: return "HY";
	case LANG_BASQUE: return "EU";
	case LANG_MACEDONIAN: return "MK";
	case LANG_AFRIKAANS: return "AF";
	case LANG_GEORGIAN: return "KA";
	case LANG_FAEROESE: return "FO";
	case LANG_HINDI: return "HI";
	case LANG_MALAY: return "MS";
	case LANG_KAZAK: return "KK";
	case LANG_SWAHILI: return "SW";
	case LANG_UZBEK: return "UZ";
	case LANG_TATAR: return "TT";
	case LANG_PUNJABI: return "PA";
	case LANG_GUJARATI: return "GU";
	case LANG_TAMIL: return "TA";
	case LANG_KANNADA: return "KN";
	case LANG_MARATHI: return "MR";
	case LANG_SANSKRIT: return "SA";
	case LANG_MONGOLIAN: return "MN";
	case LANG_GALICIAN: return "GL";
	default:
	case LANG_ENGLISH: return "EN";
	}
}

static LANGID GetLanguageID()
{
	HINSTANCE hDLL = LoadLibrary( "kernel32.dll" );
	if( hDLL )
	{
		typedef LANGID(GET_USER_DEFAULT_UI_LANGUAGE)(void);

		GET_USER_DEFAULT_UI_LANGUAGE *pGetUserDefaultUILanguage = (GET_USER_DEFAULT_UI_LANGUAGE*) GetProcAddress( hDLL, "GetUserDefaultUILanguage" );
		if( pGetUserDefaultUILanguage )
		{
			LANGID ret = pGetUserDefaultUILanguage();
			FreeLibrary( hDLL );
			return ret;
		}
		FreeLibrary( hDLL );
	}

	return GetSystemDefaultLangID();
}

CString ArchHooks_Win32::GetPreferredLanguage()
{
	return LangIdToString( GetLanguageID() );
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

void ArchHooks_Win32::BoostPriority()
{
	/* We just want a slight boost, so we don't skip needlessly if something happens
	 * in the background.  We don't really want to be high-priority--above normal should
	 * be enough.  However, ABOVE_NORMAL_PRIORITY_CLASS is only supported in Win2000
	 * and later. */
	OSVERSIONINFO version;
	version.dwOSVersionInfoSize=sizeof(version);
	if( !GetVersionEx(&version) )
	{
		LOG->Warn( werr_ssprintf(GetLastError(), "GetVersionEx failed") );
		return;
	}

#ifndef ABOVE_NORMAL_PRIORITY_CLASS
#define ABOVE_NORMAL_PRIORITY_CLASS 0x00008000
#endif

	DWORD pri = HIGH_PRIORITY_CLASS;
	if( version.dwMajorVersion >= 5 )
		pri = ABOVE_NORMAL_PRIORITY_CLASS;

	/* Be sure to boost the app, not the thread, to make sure the
	 * sound thread stays higher priority than the main thread. */
	SetPriorityClass( GetCurrentProcess(), pri );
}

void ArchHooks_Win32::UnBoostPriority()
{
	SetPriorityClass( GetCurrentProcess(), NORMAL_PRIORITY_CLASS );
}

void ArchHooks_Win32::SetupConcurrentRenderingThread()
{
	SetThreadPriority( GetCurrentThread(), THREAD_PRIORITY_ABOVE_NORMAL );
}

static bool g_bTimerInitialized;

static void InitTimer()
{
	if( g_bTimerInitialized )
		return;
	g_bTimerInitialized = true;

	timeBeginPeriod( 1 );
}

int64_t ArchHooks::GetMicrosecondsSinceStart( bool bAccurate )
{
	if( !g_bTimerInitialized )
		InitTimer();

	int64_t ret = timeGetTime() * int64_t(1000);
	if( bAccurate )
	{
		ret = FixupTimeIfLooped( ret );
		ret = FixupTimeIfBackwards( ret );
	}
	
	return ret;
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
