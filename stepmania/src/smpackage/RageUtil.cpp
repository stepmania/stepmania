#include "stdafx.h"
/*
-----------------------------------------------------------------------------
 File: RageUtil

 Desc: See header.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
	Peter S. May (GetHashForString implementation)
-----------------------------------------------------------------------------
*/

#include "RageUtil.h"
#include <algorithm>

ULONG		randseed = time(NULL);



bool IsAnInt( LPCTSTR s )
{
	if( strlen(s) == 0 )
		return false;

	for( UINT i=0; i<strlen(s); i++ )
		if( s[i] < '0' || s[i] > '9' )
			return false;

	return true;
}

float TimeToSeconds( CString sHMS )
{
	CStringArray arrayBits;
	split( sHMS, ":", arrayBits, false );

	while( arrayBits.size() < 3 )
		arrayBits.insert(arrayBits.begin(), "0" );	// pad missing bits

	float fSeconds = 0;
	fSeconds += atoi( arrayBits[0] ) * 60 * 60;
	fSeconds += atoi( arrayBits[1] ) * 60;
	fSeconds += (float)atof( arrayBits[2] );

	return fSeconds;
}

CString SecondsToTime( float fSecs )
{
	int iMinsDisplay = (int)fSecs/60;
	int iSecsDisplay = (int)fSecs - iMinsDisplay*60;
	int iLeftoverDisplay = int( (fSecs - iMinsDisplay*60 - iSecsDisplay) * 100 );
	return ssprintf( "%02d:%02d:%02d", iMinsDisplay, iSecsDisplay, iLeftoverDisplay );
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
CString join( const CString &Deliminator, const CStringArray& Source)
{
	if( Source.empty() )
		return "";

	CString csTmp;

	// Loop through the Array and Append the Deliminator
	for( unsigned iNum = 0; iNum < Source.size()-1; iNum++ ) {
		csTmp += Source[iNum];
		csTmp += Deliminator;
	}
	csTmp += Source.back();
	return csTmp;
}

//-----------------------------------------------------------------------------
// Name: split()
// Desc:
//-----------------------------------------------------------------------------
void split( const CString &Source, const CString &Deliminator, CStringArray& AddIt, const bool bIgnoreEmpty )
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
					AddIt.push_back(AddCString);

			tmpCString = newCString.Mid(pos + Deliminator.GetLength());
			newCString = tmpCString;
		}
	} while ( pos != -1 );

	if( newCString.IsEmpty() && bIgnoreEmpty )
		; // do nothing
	else
		AddIt.push_back(newCString);
}




//-----------------------------------------------------------------------------
// Name: splitpath()
// Desc:
//-----------------------------------------------------------------------------
void splitpath( const bool UsingDirsOnly, const CString &Path, CString& Drive, CString& Dir, CString& FName, CString& Ext )
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
void splitrelpath( const CString &Path, CString& Dir, CString& FName, CString& Ext )
{
	/* Find the last slash or backslash. */
	int Last = max(Path.ReverseFind('/'), Path.ReverseFind('\\')); 

	/* Set 'Last' to the first character of the filename.  If we have
	 * no directory separators, this is the entire string, so set it
	 * to 0. */
	if(Last == -1) Last = 0;
	else Last++;

	CString sFNameAndExt = Path.Right(Path.GetLength()-Last);
	
	// subtract the FNameAndExt from Path
	Dir = Path.Left( Path.GetLength()-sFNameAndExt.GetLength() );	// don't subtract out the trailing slash

	CStringArray sFNameAndExtBits;
	split( sFNameAndExt, ".", sFNameAndExtBits, false );

	if( sFNameAndExt.GetLength() == 0 )	// no file at the end of this path
	{
		FName = "";
		Ext = "";
	}
	else if( sFNameAndExtBits.size() == 1 )	// file doesn't have extension
	{
		FName = sFNameAndExtBits[0];
		Ext = "";
	}
	else if( sFNameAndExtBits.size() > 1 )	// file has extension and possibly multiple periods
	{
		Ext = sFNameAndExtBits[ sFNameAndExtBits.size()-1 ];

		// subtract the Ext and last period from FNameAndExt
		FName = sFNameAndExt.Left( sFNameAndExt.GetLength()-Ext.GetLength()-1 );
	}
}


void GetDirListing( CString sPath, CStringArray &AddTo, bool bOnlyDirs, bool bReturnPathToo )
{
	CString sDir, sThrowAway;
	splitrelpath( sPath, sDir, sThrowAway, sThrowAway );

	WIN32_FIND_DATA fd;
	HANDLE hFind = ::FindFirstFile( sPath, &fd );

	if( INVALID_HANDLE_VALUE == hFind )		// no files found
		return;

	do
	{
		if( bOnlyDirs  &&  !(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) )
			continue;	// skip

		CString sDirName( fd.cFileName );

		if( sDirName == "."  ||  sDirName == ".." )
			continue;

		if( bReturnPathToo )
			AddTo.push_back( sDir + sDirName );
		else
			AddTo.push_back( sDirName );


	} while( ::FindNextFile( hFind, &fd ) );
	::FindClose( hFind );
}

//-----------------------------------------------------------------------------
// Name: GetHashForString( CString s )
// Desc: This new version of GetHashForString uses a stronger hashing algorithm
//       than the former, assuring to a greater degree that two distinct
//       strings do not produce the same result.
//       The hashing algorithm is a modified CRC-32 (modified in that the
//       characters in the CString are recast as unsigned char and that the
//       unsigned int result from the hash is recast as a signed int).
//-----------------------------------------------------------------------------
int GetHashForString ( CString string )
{
/*
 *	RageCRC32.cpp
 *
 *	Original code
 *	COPYRIGHT (C) 1986 Gary S. Brown.  You may use this program, or
 *	code or tables extracted from it, as desired without restriction.
 *
 *	Code was extracted from IETF Internet Draft
 *	draft-ietf-tsvwg-sctpcsum-00 at URI
 *	http://www.ietf.org/proceedings/01dec/I-D/draft-ietf-tsvwg-sctpcsum-00.txt
 *
 *	Adaptation
 *	Copyright (C) 2002 Peter S. May.
 *	SourceForge ID: drokulix
 *
 *	- Header file added
 *	- Name of function changed to GetCrc32
 *	- Values in table changed from long int to int
 *
 *	Chris:
 *  Moved this code out of RageCRC32 since it's not terribly long.  Thanks Dro Kulix!
 */

/*
 *  COPYRIGHT (C) 1986 Gary S. Brown.  You may use this program, or
 *  code or tables extracted from it, as desired without restriction.
 *
 *  First, the polynomial itself and its table of feedback terms.  The
 *  polynomial is
 *  X^32+X^26+X^23+X^22+X^16+X^12+X^11+X^10+X^8+X^7+X^5+X^4+X^2+X^1+X^0
 *
 *  Note that we take it "backwards" and put the highest-order term in
 *  the lowest-order bit.  The X^32 term is "implied"; the LSB is the
 *  X^31 term, etc.  The X^0 term (usually shown as "+1") results in
 *  the MSB being 1
 *
 *  Note that the usual hardware shift register implementation, which
 *  is what we're using (we're merely optimizing it by doing eight-bit
 *  chunks at a time) shifts bits into the lowest-order term.  In our
 *  implementation, that means shifting towards the right.  Why do we
 *  do it this way?  Because the calculated CRC must be transmitted in
 *  order from highest-order term to lowest-order term.  UARTs transmit
 *  characters in order from LSB to MSB.  By storing the CRC this way
 *  we hand it to the UART in the order low-byte to high-byte; the UART
 *  sends each low-bit to hight-bit; and the result is transmission bit
 *  by bit from highest- to lowest-order term without requiring any bit
 *  shuffling on our part.  Reception works similarly
 *
 *  The feedback terms table consists of 256, 32-bit entries.  Notes
 *
 *      The table can be generated at runtime if desired; code to do so
 *      is shown later.  It might not be obvious, but the feedback
 *      terms simply represent the results of eight shift/xor opera
 *      tions for all combinations of data and CRC register values
 *
 *      The values must be right-shifted by eight bits by the "updcrc
 *      logic; the shift must be unsigned (bring in zeroes).  On some
 *      hardware you could probably optimize the shift in assembler by
 *      using byte-swap instructions
 *      polynomial $edb88320
 */

static const unsigned int crc32_tab[] = {
	0x00000000, 0x77073096, 0xee0e612c, 0x990951ba, 0x076dc419,
	0x706af48f, 0xe963a535, 0x9e6495a3, 0x0edb8832, 0x79dcb8a4,
	0xe0d5e91e, 0x97d2d988, 0x09b64c2b, 0x7eb17cbd, 0xe7b82d07,
	0x90bf1d91, 0x1db71064, 0x6ab020f2, 0xf3b97148, 0x84be41de,
	0x1adad47d, 0x6ddde4eb, 0xf4d4b551, 0x83d385c7, 0x136c9856,
	0x646ba8c0, 0xfd62f97a, 0x8a65c9ec, 0x14015c4f, 0x63066cd9,
	0xfa0f3d63, 0x8d080df5, 0x3b6e20c8, 0x4c69105e, 0xd56041e4,
	0xa2677172, 0x3c03e4d1, 0x4b04d447, 0xd20d85fd, 0xa50ab56b,
	0x35b5a8fa, 0x42b2986c, 0xdbbbc9d6, 0xacbcf940, 0x32d86ce3,
	0x45df5c75, 0xdcd60dcf, 0xabd13d59, 0x26d930ac, 0x51de003a,
	0xc8d75180, 0xbfd06116, 0x21b4f4b5, 0x56b3c423, 0xcfba9599,
	0xb8bda50f, 0x2802b89e, 0x5f058808, 0xc60cd9b2, 0xb10be924,
	0x2f6f7c87, 0x58684c11, 0xc1611dab, 0xb6662d3d, 0x76dc4190,
	0x01db7106, 0x98d220bc, 0xefd5102a, 0x71b18589, 0x06b6b51f,
	0x9fbfe4a5, 0xe8b8d433, 0x7807c9a2, 0x0f00f934, 0x9609a88e,
	0xe10e9818, 0x7f6a0dbb, 0x086d3d2d, 0x91646c97, 0xe6635c01,
	0x6b6b51f4, 0x1c6c6162, 0x856530d8, 0xf262004e, 0x6c0695ed,
	0x1b01a57b, 0x8208f4c1, 0xf50fc457, 0x65b0d9c6, 0x12b7e950,
	0x8bbeb8ea, 0xfcb9887c, 0x62dd1ddf, 0x15da2d49, 0x8cd37cf3,
	0xfbd44c65, 0x4db26158, 0x3ab551ce, 0xa3bc0074, 0xd4bb30e2,
	0x4adfa541, 0x3dd895d7, 0xa4d1c46d, 0xd3d6f4fb, 0x4369e96a,
	0x346ed9fc, 0xad678846, 0xda60b8d0, 0x44042d73, 0x33031de5,
	0xaa0a4c5f, 0xdd0d7cc9, 0x5005713c, 0x270241aa, 0xbe0b1010,
	0xc90c2086, 0x5768b525, 0x206f85b3, 0xb966d409, 0xce61e49f,
	0x5edef90e, 0x29d9c998, 0xb0d09822, 0xc7d7a8b4, 0x59b33d17,
	0x2eb40d81, 0xb7bd5c3b, 0xc0ba6cad, 0xedb88320, 0x9abfb3b6,
	0x03b6e20c, 0x74b1d29a, 0xead54739, 0x9dd277af, 0x04db2615,
	0x73dc1683, 0xe3630b12, 0x94643b84, 0x0d6d6a3e, 0x7a6a5aa8,
	0xe40ecf0b, 0x9309ff9d, 0x0a00ae27, 0x7d079eb1, 0xf00f9344,
	0x8708a3d2, 0x1e01f268, 0x6906c2fe, 0xf762575d, 0x806567cb,
	0x196c3671, 0x6e6b06e7, 0xfed41b76, 0x89d32be0, 0x10da7a5a,
	0x67dd4acc, 0xf9b9df6f, 0x8ebeeff9, 0x17b7be43, 0x60b08ed5,
	0xd6d6a3e8, 0xa1d1937e, 0x38d8c2c4, 0x4fdff252, 0xd1bb67f1,
	0xa6bc5767, 0x3fb506dd, 0x48b2364b, 0xd80d2bda, 0xaf0a1b4c,
	0x36034af6, 0x41047a60, 0xdf60efc3, 0xa867df55, 0x316e8eef,
	0x4669be79, 0xcb61b38c, 0xbc66831a, 0x256fd2a0, 0x5268e236,
	0xcc0c7795, 0xbb0b4703, 0x220216b9, 0x5505262f, 0xc5ba3bbe,
	0xb2bd0b28, 0x2bb45a92, 0x5cb36a04, 0xc2d7ffa7, 0xb5d0cf31,
	0x2cd99e8b, 0x5bdeae1d, 0x9b64c2b0, 0xec63f226, 0x756aa39c,
	0x026d930a, 0x9c0906a9, 0xeb0e363f, 0x72076785, 0x05005713,
	0x95bf4a82, 0xe2b87a14, 0x7bb12bae, 0x0cb61b38, 0x92d28e9b,
	0xe5d5be0d, 0x7cdcefb7, 0x0bdbdf21, 0x86d3d2d4, 0xf1d4e242,
	0x68ddb3f8, 0x1fda836e, 0x81be16cd, 0xf6b9265b, 0x6fb077e1,
	0x18b74777, 0x88085ae6, 0xff0f6a70, 0x66063bca, 0x11010b5c,
	0x8f659eff, 0xf862ae69, 0x616bffd3, 0x166ccf45, 0xa00ae278,
	0xd70dd2ee, 0x4e048354, 0x3903b3c2, 0xa7672661, 0xd06016f7,
	0x4969474d, 0x3e6e77db, 0xaed16a4a, 0xd9d65adc, 0x40df0b66,
	0x37d83bf0, 0xa9bcae53, 0xdebb9ec5, 0x47b2cf7f, 0x30b5ffe9,
	0xbdbdf21c, 0xcabac28a, 0x53b39330, 0x24b4a3a6, 0xbad03605,
	0xcdd70693, 0x54de5729, 0x23d967bf, 0xb3667a2e, 0xc4614ab8,
	0x5d681b02, 0x2a6f2b94, 0xb40bbe37, 0xc30c8ea1, 0x5a05df1b,
	0x2d02ef8d
};

/* Return a 32-bit CRC of the contents of the buffer. */

//unsigned int
//GetCrc32(const unsigned char *s, unsigned int len)
//{
	const char* s = string;
	int len = string.GetLength();

	unsigned int crc32val = 0;
	for( int i=0; i<len; i++ )
		crc32val = crc32_tab[(crc32val ^ s[i]) & 0xff] ^ (crc32val >> 8);

	return crc32val;
//}
}

int GetHashForFile( CString sPath )
{
	int hash = 0;

	hash += GetHashForString( sPath );

	hash += GetFileSizeInBytes( sPath ); 

	CFileStatus status;
	if( CFile::GetStatus(sPath, status) )
		hash += status.m_mtime.GetHour() * 3600 + status.m_mtime.GetMinute() * 60 + status.m_mtime.GetSecond();

	return abs(hash);
}

int GetHashForDirectory( CString sDir )
{
	int hash = 0;

	hash += GetHashForFile( sDir );

	CStringArray arrayFiles;
	GetDirListing( sDir+"\\*.*", arrayFiles, false );
	for( unsigned i=0; i<arrayFiles.size(); i++ )
	{
		const CString sFilePath = sDir + arrayFiles[i];
		hash += GetHashForFile( sFilePath );
	}

	return abs(hash); 
}

DWORD GetFileSizeInBytes( const CString &sFilePath )
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

	DWORD dwSize = GetFileSize( hFile, NULL );
	CloseHandle( hFile );
	return dwSize;
}

bool DoesFileExist( const CString &sPath )
{
    DWORD dwAttr = GetFileAttributes( sPath );
    return bool(dwAttr != (DWORD)-1);
}

bool IsAFile( const CString &sPath )
{
    return DoesFileExist(sPath)  &&  ! IsADirectory(sPath);
}

bool IsADirectory( const CString &sPath )
{
    DWORD dwAttr = GetFileAttributes( sPath );
	if( dwAttr == 0xFFFFFFFF )	// failed
		return false;
	else
		return (dwAttr & FILE_ATTRIBUTE_DIRECTORY) != 0;
}


bool CompareCStringsAsc(const CString &str1, const CString &str2)
{
	return str1.CompareNoCase( str2 ) < 0;
}

bool CompareCStringsDesc(const CString &str1, const CString &str2)
{
	return str1.CompareNoCase( str2 ) > 0;
}

void SortCStringArray( CStringArray &arrayCStrings, const bool bSortAscending )
{
	sort( arrayCStrings.begin(), arrayCStrings.end(),
			CompareCStringsDesc);
}

void StripCrnl(CString &s)
{
	while( s.GetLength() && (s[s.GetLength()-1] == '\r' || s[s.GetLength()-1] == '\n') )
		s.Delete(s.GetLength()-1);
}

LONG GetRegKey(HKEY key, LPCTSTR subkey, LPTSTR retdata)
{
	HKEY hkey;
    LONG retval = RegOpenKeyEx(key, subkey, 0, KEY_QUERY_VALUE, &hkey);

    if (retval == ERROR_SUCCESS) {
		long datasize = MAX_PATH;
		TCHAR data[MAX_PATH];
		RegQueryValue(hkey, "Favorites", data, &datasize);
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

void WriteStringToFile( FILE* file, CString s )
{
	if( s == "" )
		s = "_";
	fprintf( file, "%s\n", s );
}

void ReadStringFromFile( FILE* file, CString& s )
{
	char szTemp[MAX_PATH];
	fscanf( file, "%[^\n]\n", szTemp );
	s = szTemp;
	if( s == "_" )
		s = "";
}


void WriteIntToFile( FILE* file, int i )
{
	fprintf( file, "%d\n", i );
}

void ReadIntFromFile( FILE* file, int& i )
{
	fscanf( file, "%d\n", &i );
}


void WriteFloatToFile( FILE* file, float f )
{
	fprintf( file, "%f\n", f );
}

void ReadFloatFromFile( FILE* file, float& f )
{
	fscanf( file, "%f\n", &f );
}



void WriteUlongToFile( FILE* file, ULONG u )
{
	fprintf( file, "%u\n", u );
}

void ReadUlongFromFile( FILE* file, ULONG& u )
{
	fscanf( file, "%u\n", &u );
}



