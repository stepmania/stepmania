#include "stdafx.h"
/*
-----------------------------------------------------------------------------
 File: RageUtil.cpp

 Desc: Helper and error-controlling function used throughout the program.

 Copyright (c) 2001 Chris Danford.  All rights reserved.
-----------------------------------------------------------------------------
*/

#include "RageUtil.h"


//-----------------------------------------------------------------------------
// UTIL globals
//-----------------------------------------------------------------------------
const CString g_sLogFileName = "log.txt";
const CString g_sErrorFileName = "error.txt";
FILE* g_fileLog = NULL;

//-----------------------------------------------------------------------------
// Name: randomf()
// Desc:
//-----------------------------------------------------------------------------
float randomf( float low, float high )
{
    return low + ( high - low ) * ( (FLOAT)rand() ) / RAND_MAX;
}

//-----------------------------------------------------------------------------
// Name: roundf()
// Desc:
//-----------------------------------------------------------------------------
int roundf( float f )
{
    return (int)((f)+0.5f);
}

int roundf( double f )
{
    return (int)((f)+0.5);
}



bool IsAnInt( CString s )
{
	if( s.GetLength() == 0 )
		return false;

	for( int i=0; i<s.GetLength(); i++ )
	{
		if( s[i] < '0' || s[i] > '9' )
			return false;
	}

	return true;
}



//-----------------------------------------------------------------------------
// Name: RageLogStart()
// Desc:
//-----------------------------------------------------------------------------
void RageLogStart()
{
	DeleteFile( g_sLogFileName );
	DeleteFile( g_sErrorFileName );

	// Open log file and leave it open.  Let the OS close it when the app exits
	g_fileLog = fopen( g_sLogFileName, "w" );


	SYSTEMTIME st;
    GetLocalTime( &st );

	RageLog( "%s: last compiled on %s.", g_sLogFileName, __TIMESTAMP__ );
	RageLog( "Log starting %.4d-%.2d-%.2d %.2d:%.2d:%.2d", 
		     st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond );
	RageLog( "\n" );
}


//-----------------------------------------------------------------------------
// Name: RageLog()
// Desc:
//-----------------------------------------------------------------------------
void RageLog( LPCTSTR fmt, ...)
{
    va_list	va;
    va_start(va, fmt);

    CString sBuff = vssprintf( fmt, va );
	sBuff += "\n";

	fprintf( g_fileLog, sBuff ); 
	fflush( g_fileLog );
//	fclose( g_fileLog );
//	g_fileLog = fopen( g_sLogFileName, "w" );
}

void RageLogHr( HRESULT hr, LPCTSTR fmt, ...)
{
    va_list	va;
    va_start(va, fmt);
    CString s = vssprintf( fmt, va );
	s += ssprintf( "(%s)", DXGetErrorString8(hr) );
	RageLog( s );
}


//-----------------------------------------------------------------------------
// Name: ssprintf()
// Desc:
//-----------------------------------------------------------------------------
CString ssprintf( LPCTSTR fmt, ...)
{
    va_list	va;
    va_start(va, fmt);
	return vssprintf(fmt, va);
}


//-----------------------------------------------------------------------------
// Name: vssprintf()
// Desc:
//-----------------------------------------------------------------------------
CString vssprintf( LPCTSTR fmt, va_list argList)
{
	CString str;
	str.FormatV(fmt, argList);
	return str;
}



//-----------------------------------------------------------------------------
// Name: join()
// Desc:
//-----------------------------------------------------------------------------
CString join(CString Deliminator, CStringArray& Source)
{
	CString csReturn;
	CString csTmp;

	// Loop through the Array and Append the Deliminator
	for( int iNum = 0; iNum < Source.GetSize(); iNum++ ) {
		csTmp += Source.GetAt(iNum);
		csTmp += Deliminator;
	}
	csReturn = csTmp.Left(csTmp.GetLength() - 1);
	return csReturn;
}


//-----------------------------------------------------------------------------
// Name: split()
// Desc:
//-----------------------------------------------------------------------------
void split( CString Source, CString Deliminator, CStringArray& AddIt, bool bIgnoreEmpty )
{
	CString		 newCString;
	CString		 tmpCString;
	CString		 AddCString;

	int pos1 = 0;
	int pos = 0;

	newCString = Source;
	do {
		pos1 = 0;
		pos = newCString.Find(Deliminator, pos1);
		if ( pos != -1 ) {
			CString AddCString = newCString.Left(pos);
				if( newCString.IsEmpty() && bIgnoreEmpty )
					; // do nothing
				else
					AddIt.Add(AddCString);

			tmpCString = newCString.Mid(pos + Deliminator.GetLength());
			newCString = tmpCString;
		}
	} while ( pos != -1 );

	if( newCString.IsEmpty() && bIgnoreEmpty )
		; // do nothing
	else
		AddIt.Add(newCString);
}


//-----------------------------------------------------------------------------
// Name: splitpath()
// Desc:
//-----------------------------------------------------------------------------
void splitpath( BOOL UsingDirsOnly, CString Path, CString& Drive, CString& Dir, CString& FName, CString& Ext )
{

	int nSecond;

	// Look for a UNC Name!
	if (Path.Left(2) == "\\\\") 
	{
		int nFirst = Path.Find("\\",3);
		nSecond = Path.Find("\\",nFirst + 1);
		if (nSecond == -1) {
			Drive = Path;
			Dir = "";
			FName = "";
			Ext = "";
		}
		else if (nSecond > nFirst)
			Drive = Path.Left(nSecond);
	}
	else if (Path.Mid(1,1) == ":" )		// normal Drive Structure
	{
		nSecond = 2;
		Drive = Path.Left(2);
	}
	else	// no UNC or drive letter
	{
		nSecond = -1;
	}
	

	if (UsingDirsOnly) {
		Dir = Path.Right((Path.GetLength() - nSecond) - 1);
		FName = "";
		Ext = "";
	}
	else {
		int nDirEnd = Path.ReverseFind('\\');
		if (nDirEnd == Path.GetLength()) {
			Dir = "";
			FName = "";
			Ext = "";
		}
		else {

			Dir = Path.Mid(nSecond + 1, (nDirEnd - nSecond) - 1);

			int nFileEnd = Path.ReverseFind('.');
			if (nFileEnd != -1) {
				
				if (nDirEnd > nFileEnd) {
					FName = Path.Right(Path.GetLength() - nDirEnd);
					Ext = "";
				}
				else {
					FName = Path.Mid(nDirEnd + 1, (nFileEnd - nDirEnd) - 1);
					Ext = Path.Right((Path.GetLength() - nFileEnd) - 1);
				}
			}
			else {
				FName = Path.Right((Path.GetLength() - nDirEnd) - 1);
				Ext = "";
			}
		}
	}
}

//-----------------------------------------------------------------------------
// Name: splitpath()
// Desc:
//-----------------------------------------------------------------------------
void splitrelpath( CString Path, CString& Dir, CString& FName, CString& Ext )
{
	// need to split on both forward slashes and back slashes
	CStringArray sPathBits;

	CStringArray sBackShashPathBits;
	split( Path, "\\", sBackShashPathBits, true );

	for( int i=0; i<sBackShashPathBits.GetSize(); i++ )	// foreach backslash bit
	{
		CStringArray sForwardShashPathBits;
		split( sBackShashPathBits[i], "/", sForwardShashPathBits, true );

		sPathBits.Append( sForwardShashPathBits );
	}


	if( sPathBits.GetSize() == 0 )
	{
		Dir = FName = Ext = "";
		return;
	}

	CString sFNameAndExt = sPathBits[ sPathBits.GetSize()-1 ];

	// subtract the FNameAndExt from Path
	Dir = Path.Left( Path.GetLength()-sFNameAndExt.GetLength() );	// don't subtract out the trailing slash


	CStringArray sFNameAndExtBits;
	split( sFNameAndExt, ".", sFNameAndExtBits, false );

	if( sFNameAndExtBits.GetSize() == 0 )	// no file at the end of this path
	{
		FName = "";
		Ext = "";
	}
	else if( sFNameAndExtBits.GetSize() == 1 )	// file doesn't have extension
	{
		FName = sFNameAndExtBits[0];
		Ext = "";
	}
	else if( sFNameAndExtBits.GetSize() > 1 )	// file has extension and possibly multiple periods
	{
		Ext = sFNameAndExtBits[ sFNameAndExtBits.GetSize()-1 ];

		// subtract the Ext and last period from FNameAndExt
		FName = sFNameAndExt.Left( sFNameAndExt.GetLength()-Ext.GetLength()-1 );
	}
}


void GetDirListing( CString sPath, CStringArray &AddTo, BOOL bOnlyDirs )
{
	WIN32_FIND_DATA fd;
	HANDLE hFind = ::FindFirstFile( sPath, &fd );

	if( INVALID_HANDLE_VALUE != hFind )
	{
		do
		{
			if( bOnlyDirs  &&  !(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) )
			{
				;	// do nothing
			}
			else
			{
				// add it
				CString sDirName( fd.cFileName );
				if( sDirName != "."  &&  sDirName != ".." )
					AddTo.Add( sDirName );
			}
		} while( ::FindNextFile( hFind, &fd ) );
		::FindClose( hFind );
	}
}

DWORD GetFileSizeInBytes( CString sFilePath )
{
	HANDLE hFile = CreateFile(
	  sFilePath,          // pointer to name of the file
	  GENERIC_READ,       // access (read-write) mode
	  FILE_SHARE_READ|FILE_SHARE_WRITE,	// share mode
	  NULL,				   // pointer to security attributes
	  OPEN_EXISTING,  // how to create
	  FILE_ATTRIBUTE_NORMAL,  // file attributes
	  NULL         // handle to file with attributes to 
	);

	return GetFileSize( hFile, NULL );
}


bool DoesFileExist( CString sPath )
{
	//RageLog( "DoesFileExist(%s)", sPath );

    DWORD dwAttr = GetFileAttributes( sPath );
    if( dwAttr == (DWORD)-1 )
        return FALSE;
	else
		return TRUE;

}

int CompareCStrings(const void *arg1, const void *arg2)
{
	CString str1 = *(CString *)arg1;
	CString str2 = *(CString *)arg2;
	return str1.CompareNoCase( str2 );
}

void SortCStringArray( CStringArray &arrayCStrings, BOOL bSortAcsending )
{
	qsort( arrayCStrings.GetData(), arrayCStrings.GetSize(), sizeof(CString), CompareCStrings );
}

//-----------------------------------------------------------------------------
// Name: DisplayErrorAndDie()
// Desc:
//-----------------------------------------------------------------------------

VOID DisplayErrorAndDie( CString sError )
{
	/*
	////////////////////////
	// get a stack trace
	////////////////////////
	AfxDumpStack( AFX_STACK_DUMP_TARGET_CLIPBOARD );

	//open the clipboard
	CString fromClipboard;
	if( OpenClipboard(NULL) ) 
	{
		HANDLE hData = GetClipboardData( CF_TEXT );
		char *buffer = (char*)GlobalLock( hData );
		fromClipboard = buffer;
		GlobalUnlock( hData );
		CloseClipboard();
	}
	sError += "\n\n" + fromClipboard;
	*/
#ifdef DEBUG	// don't use error handler in Release mode
	AfxMessageBox( sError );
	AfxDebugBreak();
	exit(1);
#else
	FILE* fp = fopen( g_sErrorFileName, "w" );
	fprintf( fp, sError );
	fclose( fp );
	// generate an exception so the error handler shows
	throw(1);
#endif
}


LONG GetRegKey(HKEY key, LPCTSTR subkey, LPTSTR retdata)
{
	HKEY hkey;
    LONG retval = RegOpenKeyEx(key, subkey, 0, KEY_QUERY_VALUE, &hkey);

    if (retval == ERROR_SUCCESS) {
		long datasize = MAX_PATH;
		TCHAR data[MAX_PATH];
		RegQueryValue(hkey, NULL, data, &datasize);
		lstrcpy(retdata,data);
		RegCloseKey(hkey);
	}

    return retval;
}

HINSTANCE GotoURL(LPCTSTR url)
{
	TCHAR key[MAX_PATH + MAX_PATH];

	// First try ShellExecute()
	HINSTANCE result = ShellExecute(NULL, _T("open"), url, NULL,NULL, SW_SHOWDEFAULT);

	// If it failed, get the .htm regkey and lookup the program
	if ((UINT)result <= HINSTANCE_ERROR) {

		if (GetRegKey(HKEY_CLASSES_ROOT, _T(".htm"), key) == ERROR_SUCCESS) {
			lstrcat(key, _T("\\shell\\open\\command"));

			if (GetRegKey(HKEY_CLASSES_ROOT,key,key) == ERROR_SUCCESS) {
				TCHAR *pos;
				pos = _tcsstr(key, _T("\"%1\""));
				if (pos == NULL) {                       // No quotes found
					pos = strstr(key, _T("%1"));       // Check for %1, without quotes 
					if (pos == NULL)                   // No parameter at all...
						pos = key+lstrlen(key)-1;
					else
						*pos = '\0';                 // Remove the parameter
				}
				else
					*pos = '\0';                       // Remove the parameter

				lstrcat(pos, _T(" "));
				lstrcat(pos, url);
				result = (HINSTANCE) WinExec(key,SW_SHOWDEFAULT);
			}
		}
	}

	return result;
}
