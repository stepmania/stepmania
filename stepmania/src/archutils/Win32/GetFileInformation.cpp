#include "global.h"
#include "GetFileInformation.h"

#include "RageUtil.h"
#include <sys/stat.h>
#include <windows.h>

#pragma comment(lib, "version.lib")

bool GetFileVersion( CString fn, CString &out )
{
	DWORD ignore;
	DWORD iSize = GetFileVersionInfoSize( fn, &ignore );
	if( !iSize )
		return false;

	CString VersionBuffer( iSize, ' ' );
	if( !GetFileVersionInfo( fn, NULL, iSize, (char *) VersionBuffer.c_str() ) )
		return false;

	WORD *iTrans;
	UINT iTransCnt;

	if( !VerQueryValue( (void *) VersionBuffer.c_str() , "\\VarFileInfo\\Translation",
			(void **) &iTrans, &iTransCnt ) )
		return false;

	if( iTransCnt == 0 )
		return false;

	char *str;
	UINT len;

	CString sRes = ssprintf( "\\StringFileInfo\\%04x%04x\\FileVersion",
		iTrans[0], iTrans[1] );
	if( !VerQueryValue( (void *) VersionBuffer.c_str(), (char *) sRes.c_str(),
			(void **) &str,  &len ) )
		return false;

	out = CString( str, len );

	return true;
}

CString FindSystemFile( CString fn )
{
	char path[MAX_PATH];
	GetSystemDirectory( path, MAX_PATH );

	CString sPath = ssprintf( "%s\\%s", path, fn.c_str() );
	struct stat buf;
	if( !stat( sPath, &buf ) )
		return sPath;

	sPath = ssprintf( "%s\\drivers\\%s", path, fn.c_str() );
	if( !stat( sPath, &buf ) )
		return sPath;

	GetWindowsDirectory( path, MAX_PATH );

	sPath = ssprintf( "%s\\%s", path, fn.c_str() );
	if( !stat( sPath, &buf ) )
		return sPath;

	return "";
}

/*
 * (c) 2004 Glenn Maynard
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
