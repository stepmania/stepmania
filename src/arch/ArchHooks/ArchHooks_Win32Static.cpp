#include "global.h"
#include "ArchHooks.h"
#include "RageUtil.h"
#include "RageLog.h"
#include "archutils/Win32/SpecialDirs.h"
#include "ProductInfo.h"
#include "RageFileManager.h"

#include <cstdint>
#include <vector>

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

std::int64_t ArchHooks::GetMicrosecondsSinceStart( bool bAccurate )
{
	if( !g_bTimerInitialized )
		InitTimer();

	std::int64_t ret = timeGetTime() * std::int64_t(1000);
	if( bAccurate )
	{
		ret = FixupTimeIfLooped( ret );
		ret = FixupTimeIfBackwards( ret );
	}

	return ret;
}

static RString GetMountDir( const RString &sDirOfExecutable )
{
	/* All Windows data goes in the directory one level above the executable. */
	CHECKPOINT_M( ssprintf( "DOE \"%s\"", sDirOfExecutable.c_str()) );
	std::vector<RString> asParts;
	split( sDirOfExecutable, "/", asParts );
	CHECKPOINT_M( ssprintf( "... %i asParts", asParts.size()) );
	ASSERT_M( asParts.size() > 1, ssprintf("Strange sDirOfExecutable: %s", sDirOfExecutable.c_str()) );
	RString sDir = join( "/", asParts.begin(), asParts.end()-1 );
	return sDir;
}

void ArchHooks::MountInitialFilesystems( const RString &sDirOfExecutable )
{
	RString sDir = GetMountDir( sDirOfExecutable );
	FILEMAN->Mount("dirro", sDir, "/");

	bool portable = DoesFileExist("/Portable.ini");
	if (portable)
	{
		FILEMAN->Mount("dir", sDir + "/Announcers", "/Announcers");
		FILEMAN->Mount("dir", sDir + "/BGAnimations", "/BGAnimations");
		FILEMAN->Mount("dir", sDir + "/BackgroundEffects", "/BackgroundEffects");
		FILEMAN->Mount("dir", sDir + "/BackgroundTransitions", "/BackgroundTransitions");
		FILEMAN->Mount("dir", sDir + "/Cache", "/Cache");
		FILEMAN->Mount("dir", sDir + "/CDTitles", "/CDTitles");
		FILEMAN->Mount("dir", sDir + "/Characters", "/Characters");
		FILEMAN->Mount("dir", sDir + "/Courses", "/Courses");
		FILEMAN->Mount("dir", sDir + "/Downloads", "/Downloads");
		FILEMAN->Mount("dir", sDir + "/Logs", "/Logs");
		FILEMAN->Mount("dir", sDir + "/NoteSkins", "/NoteSkins");
		FILEMAN->Mount("dir", sDir + "/Packages", "/Packages");
		FILEMAN->Mount("dir", sDir + "/Save", "/Save");
		FILEMAN->Mount("dir", sDir + "/Screenshots", "/Screenshots");
		FILEMAN->Mount("dir", sDir + "/Songs", "/Songs");
		FILEMAN->Mount("dir", sDir + "/RandomMovies", "/RandomMovies");
		FILEMAN->Mount("dir", sDir + "/Themes", "/Themes");
	}
}

void ArchHooks::MountUserFilesystems( const RString &sDirOfExecutable )
{
	/*
	 * Look, I know what you're thinking: "Hey, let's put all this stuff into
	 * their respective 'proper' places on the filesystem!" Stop. Now.
	 * This was done before and it was the most ungodly confusing thing to ever
	 * happen. Just don't do it, seriously. Keep them in one place.
	 * - Colby
	 */
	RString sAppDataDir = SpecialDirs::GetAppDataDir() + PRODUCT_ID;
	//RString sCommonAppDataDir = SpecialDirs::GetCommonAppDataDir() + PRODUCT_ID;
	//RString sLocalAppDataDir = SpecialDirs::GetLocalAppDataDir() + PRODUCT_ID;
	//RString sPicturesDir = SpecialDirs::GetPicturesDir() + PRODUCT_ID;

	FILEMAN->Mount( "dir", sAppDataDir + "/Announcers", "/Announcers" );
	FILEMAN->Mount( "dir", sAppDataDir + "/BGAnimations", "/BGAnimations" );
	FILEMAN->Mount( "dir", sAppDataDir + "/BackgroundEffects", "/BackgroundEffects" );
	FILEMAN->Mount( "dir", sAppDataDir + "/BackgroundTransitions", "/BackgroundTransitions" );
	FILEMAN->Mount( "dir", sAppDataDir + "/Cache", "/Cache" );
	FILEMAN->Mount( "dir", sAppDataDir + "/CDTitles", "/CDTitles" );
	FILEMAN->Mount( "dir", sAppDataDir + "/Characters", "/Characters" );
	FILEMAN->Mount( "dir", sAppDataDir + "/Courses", "/Courses" );
	FILEMAN->Mount( "dir", sAppDataDir + "/Downloads", "/Downloads" );
	FILEMAN->Mount( "dir", sAppDataDir + "/Logs", "/Logs" );
	FILEMAN->Mount( "dir", sAppDataDir + "/NoteSkins", "/NoteSkins" );
	FILEMAN->Mount( "dir", sAppDataDir + "/Packages", "/Packages" );
	FILEMAN->Mount( "dir", sAppDataDir + "/Save", "/Save" );
	FILEMAN->Mount( "dir", sAppDataDir + "/Screenshots", "/Screenshots" );
	FILEMAN->Mount( "dir", sAppDataDir + "/Songs", "/Songs" );
	FILEMAN->Mount( "dir", sAppDataDir + "/RandomMovies", "/RandomMovies" );
	FILEMAN->Mount( "dir", sAppDataDir + "/Themes", "/Themes" );
}

static RString LangIdToString( LANGID l )
{
	switch( PRIMARYLANGID(l) )
	{
	case LANG_ARABIC: return "ar";
	case LANG_BULGARIAN: return "bg";
	case LANG_CATALAN: return "ca";
	case LANG_CHINESE:
	{
		switch (SUBLANGID(l))
		{
		case SUBLANG_CHINESE_TRADITIONAL:
		case SUBLANG_CHINESE_HONGKONG:
		case SUBLANG_CHINESE_MACAU:
			return "zh-Hant";
		case SUBLANG_CHINESE_SIMPLIFIED:
		case SUBLANG_CHINESE_SINGAPORE:
			return "zh-Hans";
		}
	}
	case LANG_CZECH: return "cs";
	case LANG_DANISH: return "da";
	case LANG_GERMAN: return "de";
	case LANG_GREEK: return "el";
	case LANG_SPANISH: return "es";
	case LANG_FINNISH: return "fi";
	case LANG_FRENCH: return "fr";
	case LANG_HEBREW: return "iw";
	case LANG_HUNGARIAN: return "hu";
	case LANG_ICELANDIC: return "is";
	case LANG_ITALIAN: return "it";
	case LANG_JAPANESE: return "ja";
	case LANG_KOREAN: return "ko";
	case LANG_DUTCH: return "nl";
	case LANG_NORWEGIAN: return "no";
	case LANG_POLISH: return "pl";
	case LANG_PORTUGUESE: return "pt";
	case LANG_ROMANIAN: return "ro";
	case LANG_RUSSIAN: return "ru";
	case LANG_CROATIAN: return "hr";
	// case LANG_SERBIAN: return "sr"; // same as LANG_CROATIAN?
	case LANG_SLOVAK: return "sk";
	case LANG_ALBANIAN: return "sq";
	case LANG_SWEDISH: return "sv";
	case LANG_THAI: return "th";
	case LANG_TURKISH: return "tr";
	case LANG_URDU: return "ur";
	case LANG_INDONESIAN: return "in";
	case LANG_UKRAINIAN: return "uk";
	case LANG_SLOVENIAN: return "sl";
	case LANG_ESTONIAN: return "et";
	case LANG_LATVIAN: return "lv";
	case LANG_LITHUANIAN: return "lt";
	case LANG_VIETNAMESE: return "vi";
	case LANG_ARMENIAN: return "hy";
	case LANG_BASQUE: return "eu";
	case LANG_MACEDONIAN: return "mk";
	case LANG_AFRIKAANS: return "af";
	case LANG_GEORGIAN: return "ka";
	case LANG_FAEROESE: return "fo";
	case LANG_HINDI: return "hi";
	case LANG_MALAY: return "ms";
	case LANG_KAZAK: return "kk";
	case LANG_SWAHILI: return "sw";
	case LANG_UZBEK: return "uz";
	case LANG_TATAR: return "tt";
	case LANG_PUNJABI: return "pa";
	case LANG_GUJARATI: return "gu";
	case LANG_TAMIL: return "ta";
	case LANG_KANNADA: return "kn";
	case LANG_MARATHI: return "mr";
	case LANG_SANSKRIT: return "sa";
	// These aren't present in the VC6 headers. We'll never have translations to these languages anyway. -C
	//case LANG_MONGOLIAN: return "mn";
	//case LANG_GALICIAN: return "gl";
	default: LOG->Warn("Unable to determine system language. Using English.");
	case LANG_ENGLISH: return "en";
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
