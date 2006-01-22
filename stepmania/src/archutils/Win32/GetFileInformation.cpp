#include "global.h"
#include "GetFileInformation.h"

#include "RageUtil.h"
#include <sys/stat.h>
#include <windows.h>
#include <tlhelp32.h>

#if defined(_MSC_VER)
#pragma comment(lib, "version.lib")
#endif

bool GetFileVersion( RString sFile, RString &sOut )
{
	do {
		/* Cast away const to work around header bug in VC6. */
		DWORD ignore;
		DWORD iSize = GetFileVersionInfoSize( const_cast<char *>(sFile.c_str()), &ignore );
		if( !iSize )
			break;

		RString VersionBuffer( iSize, ' ' );
		/* Also VC6: */
		if( !GetFileVersionInfo( const_cast<char *>(sFile.c_str()), NULL, iSize, VersionBuffer.GetBuf() ) )
			break;

		WORD *iTrans;
		UINT iTransCnt;

		if( !VerQueryValue( (void *) VersionBuffer.c_str() , "\\VarFileInfo\\Translation",
				(void **) &iTrans, &iTransCnt ) )
			break;

		if( iTransCnt == 0 )
			break;

		char *str;
		UINT len;

		RString sRes = ssprintf( "\\StringFileInfo\\%04x%04x\\FileVersion",
			iTrans[0], iTrans[1] );
		if( !VerQueryValue( (void *) VersionBuffer.c_str(), (char *) sRes.c_str(),
				(void **) &str,  &len ) || len < 1)
			break;

		sOut = RString( str, len-1 );
	} while(0);

	/* Get the size and date. */
	struct stat st;
	if( stat( sFile, &st ) != -1 )
	{
		struct tm t;
		gmtime_r( &st.st_mtime, &t );
		if( !sOut.empty() )
			sOut += " ";
		sOut += ssprintf( "[%ib, %02i-%02i-%04i]", st.st_size, t.tm_mon+1, t.tm_mday, t.tm_year+1900 );
	}

	return true;
}

RString FindSystemFile( RString sFile )
{
	char szWindowsPath[MAX_PATH];
	GetWindowsDirectory( szWindowsPath, MAX_PATH );

	const char *szPaths[] =
	{
		"/system32/",
		"/system32/drivers/",
		"/system/",
		"/system/drivers/",
		"/",
		NULL
	};

	for( int i = 0; szPaths[i]; ++i )
	{
		RString sPath = ssprintf( "%s%s%s", szWindowsPath, szPaths[i], sFile.c_str() );
		struct stat buf;
		if( !stat(sPath, &buf) )
			return sPath;
	}

	return RString();
}

/* Get the full path of the process running in iProcessID.  On error, false is
 * returned and an error message is placed in sName. */
bool GetProcessFileName( uint32_t iProcessID, RString &sName )
{
	/* This method works in everything except for NT4, and only uses kernel32.lib functions. */
	do {
		HANDLE hSnap = CreateToolhelp32Snapshot( TH32CS_SNAPMODULE, iProcessID );
		if( hSnap == NULL )
		{
			sName = werr_ssprintf( GetLastError(), "OpenProcess" );
			break;
		}

		MODULEENTRY32 me;
		ZERO( me );
		me.dwSize = sizeof(MODULEENTRY32);
		bool bRet = !!Module32First( hSnap, &me );
		CloseHandle( hSnap );

		if( bRet )
		{
			sName = me.szExePath;
			return true;
		}

		sName = werr_ssprintf( GetLastError(), "Module32First" );
	} while(0);

	/* This method only works in NT/2K/XP. */
	do {
		static HINSTANCE hPSApi = NULL;
	    typedef DWORD (WINAPI* pfnGetModuleFileNameEx)(HANDLE,HMODULE,LPSTR,DWORD);
		static pfnGetModuleFileNameEx pGetModuleFileNameEx = NULL;
		static bool bTried = false;

		if( !bTried )
		{
			bTried = true;

			hPSApi = LoadLibrary("psapi.dll");
			if( hPSApi == NULL )
			{
				sName = werr_ssprintf( GetLastError(), "LoadLibrary" );
				break;
			}
			else
			{
				pGetModuleFileNameEx = (pfnGetModuleFileNameEx) GetProcAddress( hPSApi, "GetModuleFileNameExA" );
				if( pGetModuleFileNameEx == NULL )
				{
					sName = werr_ssprintf( GetLastError(), "GetProcAddress" );
					break;
				}
			}
		}

		if( pGetModuleFileNameEx != NULL )
		{
			HANDLE hProc = OpenProcess( PROCESS_VM_READ|PROCESS_QUERY_INFORMATION, NULL, iProcessID );
			if( hProc == NULL )
			{
				sName = werr_ssprintf( GetLastError(), "OpenProcess" );
				break;
			}

			char buf[1024];
			int iRet = pGetModuleFileNameEx( hProc, NULL, buf, 1024 );
			CloseHandle( hProc );

			if( iRet )
			{
				buf[iRet] = 0;
				sName = buf;
				return true;
			}

			sName = werr_ssprintf( GetLastError(), "GetModuleFileNameEx" );
		}
	} while(0);

	return false;
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
