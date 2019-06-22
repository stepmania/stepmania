#include "global.h"
#include "SpecialDirs.h"
#include <shlobj.h>

static RString GetSpecialFolderPath( int csidl )
{
	RString sDir;
	TCHAR szDir[MAX_PATH] = "";
	HRESULT hResult = SHGetFolderPath( nullptr, csidl, nullptr, SHGFP_TYPE_CURRENT, szDir );
	ASSERT( hResult == S_OK );
	sDir = szDir;
	sDir += "/";
	return sDir;
}

RString SpecialDirs::GetAppDataDir()
{
	return GetSpecialFolderPath( CSIDL_APPDATA );
}

RString SpecialDirs::GetLocalAppDataDir()
{
	return GetSpecialFolderPath( CSIDL_LOCAL_APPDATA );
}

RString SpecialDirs::GetCommonAppDataDir()
{
	return GetSpecialFolderPath( CSIDL_COMMON_APPDATA );
}

RString SpecialDirs::GetPicturesDir()
{
	return GetSpecialFolderPath( CSIDL_MYPICTURES );
}

RString SpecialDirs::GetDesktopDir()
{
	return GetSpecialFolderPath( CSIDL_DESKTOP );
}


/*
 * (c) 2002-2004 Chris Danford
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
