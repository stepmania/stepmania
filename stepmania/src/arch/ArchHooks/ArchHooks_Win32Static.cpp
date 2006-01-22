#include "global.h"
#include "ArchHooks.h"
#include "RageUtil.h"
#include "archutils/Win32/SpecialDirs.h"
#include "ProductInfo.h"
#include "RageFileManager.h"

// for timeGetTime
#include <windows.h>
#include <mmsystem.h>
#if defined(_MSC_VER)
#pragma comment(lib, "winmm.lib")
#endif


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

void ArchHooks::MountInitialFilesystems( const RString &sDirOfExecutable )
{
	/* All Windows data goes in the directory one level above the executable. */
	CHECKPOINT_M( ssprintf( "DOE \"%s\"", sDirOfExecutable.c_str()) );
	vector<RString> parts;
	split( sDirOfExecutable, "/", parts );
	CHECKPOINT_M( ssprintf( "... %i parts", parts.size()) );
	ASSERT_M( parts.size() > 1, ssprintf("Strange sDirOfExecutable: %s", sDirOfExecutable.c_str()) );
	RString Dir = join( "/", parts.begin(), parts.end()-1 );
	FILEMAN->Mount( "dir", Dir, "/" );

	RString sMyDocumentsDir = SpecialDirs::GetMyDocumentsDir();
	RString sApplicationDataDir = SpecialDirs::GetApplicationDataDir();
	RString sDesktopDir = SpecialDirs::GetDesktopDir();

	// Mount everything game-writable (not counting the editor) to the user's directory.
	FILEMAN->Mount( "dir", sApplicationDataDir + PRODUCT_ID + "/Cache", "/Cache" );
	FILEMAN->Mount( "dir", sMyDocumentsDir + PRODUCT_ID + "/Save", "/Save" );
	FILEMAN->Mount( "dir", sMyDocumentsDir + PRODUCT_ID + "/Screenshots", "/Screenshots" );
	FILEMAN->Mount( "dir", sDesktopDir, "/Desktop" );
}

static RString LangIdToString( LANGID l )
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
	// These aren't present in the VC6 headers. We'll never have translations to these languages anyway. -C
	//case LANG_MONGOLIAN: return "MN";
	//case LANG_GALICIAN: return "GL";
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

	return GetUserDefaultLangID();
}

RString ArchHooks::GetPreferredLanguage()
{
	return LangIdToString( GetLanguageID() );
}

/*
 * (c) 2003-2004 Chris Danford
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
