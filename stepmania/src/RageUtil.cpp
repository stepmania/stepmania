#include "global.h"
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

#include <numeric>
#include <time.h>
#include <math.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fstream>
#include "regex.h"
#include <map>
#include <set>

#if !defined(WIN32)
#include <dirent.h>
#endif

unsigned long randseed = time(NULL);

/* Return a positive x mod y. */
float fmodfp(float x, float y)
{
	x = fmodf(x, y);	/* x is [-y,y] */
	x += y;				/* x is [0,y*2] */
	x = fmodf(x, y);	/* x is [0,y] */
	return x;
}

int power_of_two(int input)
{
    int value = 1;

	while ( value < input ) value <<= 1;

	return value;
}

bool IsAnInt( const CString &s )
{
	if( s[0] == '\0' )
		return false;

	for( int i=0; s[i]; i++ )
		if( s[i] < '0' || s[i] > '9' )
			return false;

	return true;
}

bool IsHexVal( const CString &s )
{
	if( s[0] == '\0' )
		return false;

	for( int i=0; s[i]; i++ )
		if( !(s[i] >= '0' && s[i] <= '9') && 
			!(toupper(s[i]) >= 'A' && toupper(s[i]) <= 'F'))
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
	float fLeftoverDisplay = (fSecs - iMinsDisplay*60 - iSecsDisplay) * 100;
	CString sReturn = ssprintf( "%02d:%02d:%02.0f", iMinsDisplay, iSecsDisplay, min(99.0f,fLeftoverDisplay) );
	if( iMinsDisplay > 99 ) {
		/* Oops.  Probably a really long endless course.  Do "hh:mm.ss"; use a period
		 * to differentiate between "mm:ss:ms".  I'd much prefer reversing those, since
		 * it makes much more sense to do "mm:ss.ms", but people would probably complain
		 * about "arcade accuracy" ... */
		sReturn = ssprintf( "%02d:%02d.%02.0f", iMinsDisplay/60, iMinsDisplay%60, iSecsDisplay );
	}
	ASSERT( sReturn.GetLength() <= 8 );
	return sReturn;
}

CString ssprintf( const char *fmt, ...)
{
    va_list	va;
    va_start(va, fmt);
	return vssprintf(fmt, va);
}


CString vssprintf( const char *fmt, va_list argList)
{
	CString str;
	str.FormatV(fmt, argList);
	return str;
}

#ifdef WIN32
#include "dxerr8.h"
#pragma comment(lib, "DxErr8.lib")

CString hr_ssprintf( int hr, const char *fmt, ...)
{
    va_list	va;
    va_start(va, fmt);
    CString s = vssprintf( fmt, va );
    va_end(va);

	return s += ssprintf( " (%s)", DXGetErrorString8(hr) );
}

CString werr_ssprintf( int err, const char *fmt, ...)
{
	char buf[1024];
	FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM,
		0, err, 0, buf, sizeof(buf), NULL);

    va_list	va;
    va_start(va, fmt);
    CString s = vssprintf( fmt, va );
    va_end(va);

	return s += ssprintf( " (%s)", buf );
}

#endif

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


template <class S>
void do_split( const S &Source, const S &Deliminator, vector<S> &AddIt, const bool bIgnoreEmpty )
{
	unsigned startpos = 0;

	do {
		unsigned pos = Source.find(Deliminator, startpos);
		if ( pos == Source.npos ) pos=Source.size();

		S AddCString = Source.substr(startpos, pos-startpos);
		if( AddCString.empty() && bIgnoreEmpty )
			; // do nothing
		else
			AddIt.push_back(AddCString);

		startpos=pos+Deliminator.size();
	} while ( startpos <= Source.size() );
}


void split( const CString &Source, const CString &Deliminator, CStringArray &AddIt, const bool bIgnoreEmpty )
{
	do_split(Source, Deliminator, AddIt, bIgnoreEmpty );
}

void split( const wstring &Source, const wstring &Deliminator, vector<wstring> &AddIt, const bool bIgnoreEmpty )
{
	do_split(Source, Deliminator, AddIt, bIgnoreEmpty );
}


/*
 * foo\fum\          -> "foo\fum\", "", ""
 * c:\foo\bar.txt    -> "c:\foo\", "bar", ".txt"
 * \\foo\fum         -> "\\foo\", "fum", ""
 */
void splitpath( CString Path, CString& Dir, CString& Filename, CString& Ext )
{
	Dir = Filename = Ext = "";

	CStringArray mat;

	/* One level of escapes for the regex, one for C. Ew. 
	 * This is really:
	 * ^(.*[\\/])?(.*)$    */
	static Regex sep("^(.*[\\\\/])?(.*)$");
	ASSERT(sep.Compare(Path, mat));

	Dir = mat[0];
	Path = mat[1];

	/* ^(.*)(\.[^\.]+)$ */
	static Regex SplitExt("^(.*)(\\.[^\\.]+)$");
	if(SplitExt.Compare(Path, mat))
	{
		Filename = mat[0];
		Ext = mat[1];
	} else
		Filename = Path;
}

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

/* mkdir -p.  Doesn't fail if Path already exists and is a directory. */
bool CreateDirectories( CString Path )
{
	CStringArray parts;
	CString curpath;
	Path.Replace("\\", "/");
	split(Path, "/", parts);

	for(unsigned i = 0; i < parts.size(); ++i)
	{
		curpath += parts[i] + "/";
		if(mkdir( curpath, 0755 ))
			continue;

		if(errno != EEXIST)
			return false;

		/* Make sure it's a directory. */
		if( !IsADirectory(curpath) )
			return false;
	}
	
	return true;
}

#if 0
void GetDirListing( CString sPath, CStringArray &AddTo, bool bOnlyDirs, bool bReturnPathToo )
{
	CString sDir, sThrowAway;
	splitrelpath( sPath, sDir, sThrowAway, sThrowAway );

	/* XXX: We should use Find* to get a file list only, and handle
	 * wildcard matching ourself.  Windows matching is braindead.  For
	 * example, *.dwi matches "foo.dwi~". */
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
#endif

void GetCwd(CString &s)
{
	char buf[PATH_MAX];
	bool ret = getcwd(buf, PATH_MAX) != NULL;
	ASSERT(ret);

	s = buf;
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
unsigned int GetHashForString ( CString string )
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

bool DoStat(CString sPath, struct stat *st)
{
	TrimRight(sPath, "/\\");
    return stat(sPath.GetString(), st) != -1;
}

unsigned int GetHashForFile( CString sPath )
{
	unsigned int hash = 0;

	hash += GetHashForString( sPath );

	hash += GetFileSizeInBytes( sPath ); 

	struct stat st;
	if( DoStat(sPath, &st))
		hash += st.st_mtime;

	return hash;
}

unsigned int GetHashForDirectory( CString sDir )
{
	unsigned int hash = 0;

	hash += GetHashForFile( sDir );

	CStringArray arrayFiles;
	GetDirListing( sDir+"\\*", arrayFiles, false );
	for( unsigned i=0; i<arrayFiles.size(); i++ )
	{
		const CString sFilePath = sDir + arrayFiles[i];
		hash += GetHashForFile( sFilePath );
	}

	return hash; 
}

unsigned GetFileSizeInBytes( const CString &sFilePath )
{
	struct stat st;
	if(!DoStat(sFilePath, &st))
		return 0;
	
	return st.st_size;
}

bool DoesFileExist( const CString &sPath )
{
	if(sPath.empty()) return false;
	struct stat st;
    return DoStat(sPath, &st);
}

bool IsAFile( const CString &sPath )
{
    return DoesFileExist(sPath)  &&  ! IsADirectory(sPath);
}

bool IsADirectory( const CString &sPath )
{
	if(sPath.empty()) return false;
	struct stat st;
    if (!DoStat(sPath, &st))
		return false;

	return !!(st.st_mode & S_IFDIR);
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
			bSortAscending?CompareCStringsAsc:CompareCStringsDesc);
}

float calc_mean(const float *start, const float *end)
{
	return accumulate(start, end, 0.f) / distance(start, end);
}

float calc_stddev(const float *start, const float *end)
{
	/* Calculate the mean. */
	float mean = calc_mean(start, end);

	/* Calculate stddev. */
	float dev = 0.0f;
	for( const float *i=start; i != end; ++i )
		dev += (*i - mean) * (*i - mean);
	dev /= distance(start, end) - 1;
	dev = sqrtf(dev);

	return dev;
}

void TrimLeft(CString &str, const char *s)
{
	int n = 0;
	while(n < int(str.size()) && strchr(s, str[n]))
		n++;

	str.erase(str.begin(), str.begin()+n);
}

void TrimRight(CString &str, const char *s)
{
	int n = str.size();
	while(n > 0 && strchr(s, str[n-1]))
		n--;

	/* Delete from n to the end.  If n == str.size(), nothing is deleted;
	 * if n == 0, the whole string is erased. */
	str.erase(str.begin()+n, str.end());
}

void StripCrnl(CString &s)
{
	while(s[s.size()-1] == '\r' || s[s.size()-1] == '\n')
		s.erase(s.size()-1);
}

/* path is a .redir pathname.  Read it and return the real one. */
CString DerefRedir(const CString &path)
{
	CString sDir, sFName, sExt;
	splitrelpath( path, sDir, sFName, sExt );

	if(sExt != "redir") return path;

	CString sNewFileName;
	{
		ifstream file(path);
		getline(file, sNewFileName);
	}

	StripCrnl(sNewFileName);

	/* Empty is invalid. */
	if(sNewFileName == "")
		return "";

	return sDir+sNewFileName;
}

CString GetRedirContents(const CString &path)
{
	CString sDir, sFName, sExt;
	splitrelpath( path, sDir, sFName, sExt );

	ASSERT( sExt == "redir");	// don't ever call this on a non-redirect

	CString sNewFileName;
	{
		ifstream file(path);
		getline(file, sNewFileName);
	}

	StripCrnl(sNewFileName);

	return sNewFileName;
}

void Regex::Compile()
{
    reg = new regex_t;

    int ret = regcomp((regex_t *) reg, pattern.c_str(), REG_EXTENDED|REG_ICASE);
    if(ret != 0)
		RageException::Throw("Invalid regex: '%s'", pattern.c_str());

    /* Count the number of backreferences. */
    backrefs = 0;
    for(int i = 0; i < int(pattern.size()); ++i)
        if(pattern[i] == '(') backrefs++;
    ASSERT(backrefs+1 < 128);
}

void Regex::Set(const CString &str)
{
	Release();
    pattern=str;
	Compile();
}

void Regex::Release()
{
    delete (regex_t *)reg;
	reg = NULL;
	pattern = "";
}

Regex::Regex(const CString &str)
{
	reg = NULL;
	Set(str);
}

Regex::Regex(const Regex &rhs)
{
	reg = NULL;
    Set(rhs.pattern);
}

Regex &Regex::operator=(const Regex &rhs)
{
	if(this != &rhs) Set(rhs.pattern);
	return *this;
}

Regex::~Regex()
{
    Release();
}

bool Regex::Compare(const CString &str)
{
    return regexec((regex_t *) reg, str.c_str(), 0, NULL, 0) != REG_NOMATCH;
}

bool Regex::Compare(const CString &str, vector<CString> &matches)
{
    matches.clear();

    regmatch_t mat[128];
    int ret = regexec((regex_t *) reg, str.c_str(), 128, mat, 0);

	if(ret == REG_NOMATCH)
        return false;

    for(unsigned i = 1; i < backrefs+1; ++i)
    {
        if(mat[i].rm_so == -1)
            matches.push_back(""); /* no match */
        else
            matches.push_back(str.substr(mat[i].rm_so, mat[i].rm_eo - mat[i].rm_so));
    }

    return true;
}

/* UTF-8 decoding code from glib. */
char *utf8_find_next_char (const char *p, const char *end)
{
	if (end)
		for (++p; p < end && (*p & 0xc0) == 0x80; ++p)
			;
	else if (*p) {
		for (++p; (*p & 0xc0) == 0x80; ++p)
			;
	}
	return (p == end) ? NULL : (char *)p;
}

int masks[7] = { 0, 0x7f, 0x1f, 0x0f, 0x07, 0x03, 0x01 };

int utf8_get_char_len (const char *pp)
{
  const unsigned char *p = (const unsigned char *) pp;
  if (*p < 128)					 return 1;
  else if ((*p & 0xe0) == 0xc0)  return 2;
  else if ((*p & 0xf0) == 0xe0)  return 3;
  else if ((*p & 0xf8) == 0xf0)  return 4;
  else if ((*p & 0xfc) == 0xf8)  return 5;
  else if ((*p & 0xfe) == 0xfc)  return 6;
  return -1;
}

wchar_t utf8_get_char (const char *p)
{
  int len = utf8_get_char_len(p);
  if(len == -1)
	  return INVALID_CHAR;

  int mask = masks[len];
  wchar_t result = wchar_t(p[0] & mask);
  for (int i = 1; i < len; ++i) {
      if ((p[i] & 0xc0) != 0x80)
          return INVALID_CHAR;

	  result <<= 6;
      result |= p[i] & 0x3f;
  }

  return result;
}

const wchar_t INVALID_CHAR = 0xFFFF;

wstring CStringToWstring(const CString &str)
{
	const char *ptr = str.c_str(), *end = str.c_str()+str.size();

	wstring ret;

	while(ptr && ptr != end)
	{
		wchar_t c = utf8_get_char (ptr);
		if(c == -1)
			ret += INVALID_CHAR;
		else
			ret += c;
		ptr = utf8_find_next_char (ptr, end);
	}

	return ret;
}

CString WStringToCString(const wstring &str)
{
	CString ret;

	for(unsigned i = 0; i < str.size(); ++i)
		ret.append(WcharToUTF8(str[i]));

	return ret;
}

static int unichar_to_utf8 (wchar_t c, char *outbuf)
{
	unsigned int len = 0;
	int first;

	if (c < 0x80) {
		first = 0;
		len = 1;
	} else if (c < 0x800) {
		first = 0xc0;
		len = 2;
	} else if (c < 0x10000) {
		first = 0xe0;
		len = 3;
	} else if (c < 0x200000) {
		first = 0xf0;
		len = 4;
	} else if (c < 0x4000000) {
		first = 0xf8;
		len = 5;
	} else {
		first = 0xfc;
		len = 6;
	}

	if (outbuf)
	{
		for (int i = len - 1; i > 0; --i)
		{
			outbuf[i] = char((c & 0x3f) | 0x80);
			c >>= 6;
		}
		outbuf[0] = char(c | first);
	}

	return len;
}

CString WcharToUTF8( wchar_t c )
{
	char buf[6];
	int cnt = unichar_to_utf8(c, buf);
	return CString(buf, cnt);
}

#if !defined(WIN32)
/* XXX autoconf this */
int memicmp(const char *s1, const char *s2, size_t n)
{
	for(size_t i = 0; i < n; ++i)
	{
		char c1 = tolower(s1[i]);
		char c2 = tolower(s2[i]);
		if(c1 < c2) return -1;
		if(c1 > c2) return 1;
	}
	return 0;
}
#endif

/* ASCII-only case insensitivity. */
struct char_traits_char_nocase: public char_traits<char>
{
    static bool eq( char c1, char c2 )
    { return toupper(c1) == toupper(c2); }

    static bool ne( char c1, char c2 )
    { return toupper(c1) != toupper(c2); }

    static bool lt( char c1, char c2 )
    { return toupper(c1) <  toupper(c2); }

    static int compare( const char* s1, const char* s2, size_t n ) {
		return memicmp( s1, s2, n );
    }

	static inline char fasttoupper(char a)
	{
		if(a < 'a' || a > 'z')
			return a;
		return a+('A'-'a');
	}
	
    static const char *find( const char* s, int n, char a ) {
	  a = fasttoupper(a);
      while( n-- > 0 && fasttoupper(*s) != a ) {
          ++s;
      }
	  if(fasttoupper(*s) == a)
		return s;
	  return NULL;
    }
};


/* Replace &#nnnn; (decimal) &xnnnn; (hex) with corresponding UTF-8 characters. */
void Replace_Unicode_Markers( CString &Text )
{
	unsigned start = 0;
	while(start < Text.size())
	{
		/* Look for &#digits; */
		bool hex = false;
		unsigned pos = Text.find("&#", start);
		if(pos == Text.npos) {
			hex = true;
			pos = Text.find("&x", start);
		}

		if(pos == Text.npos) break;
		start = pos+1;

		unsigned p = pos;
		p += 2;

		/* Found &# or &x.  Is it followed by digits and a semicolon? */
		if(p >= Text.size()) continue;

		int numdigits = 0;
		while(p < Text.size() &&
			(hex && isxdigit(Text[p])) || (!hex && isdigit(Text[p])))
		{
		   p++;
		   numdigits++;
		}
		if(!numdigits) continue; /* must have at least one digit */
		if(p >= Text.size() || Text[p] != ';') continue;
		p++;

		int num;
		if(hex) sscanf(Text.c_str()+pos, "&x%x;", &num);
		else sscanf(Text.c_str()+pos, "&#%i;", &num);
		if(num > 0xFFFF)
			num = INVALID_CHAR;

		Text.replace(pos, p-pos, WcharToUTF8(wchar_t(num)));
	}
}

void ReplaceText( CString &Text, const map<CString,CString> &m )
{
	basic_string<char,char_traits_char_nocase> txt(Text);
	
	for(map<CString,CString>::const_iterator it = m.begin(); it != m.end(); ++it)
	{
		unsigned start = 0;
		while(1)
		{
			unsigned pos = txt.find(it->first, start);
			if(pos == txt.npos)
				break;

			txt.replace(pos, it->first.size(), it->second);
			start = pos+it->second.size();
		}
	}

	Text = txt.c_str();
}

/* Form a string to identify a wchar_t with ASCII. */
CString WcharDisplayText(wchar_t c)
{
	CString chr;
	chr = ssprintf("U+%4.4x", c);
	if(c < 128) chr += ssprintf(" ('%c')", char(c));
	return chr;
}







typedef basic_string<char,char_traits_char_nocase> istring;
struct File {
	istring name;
	bool dir;
	
	File() { dir=false; }
	File(istring fn, bool dir_=false): name(fn), dir(dir) { }
	
	bool operator== (const File &rhs) const { return name==rhs.name; }
	bool operator< (const File &rhs) const { return name<rhs.name; }

	bool equal(const File &rhs) const { return name == rhs.name; }
	bool equal(const CString &rhs) const {
		return !stricmp(name.c_str(), rhs.c_str());
	}
};

#include "RageTimer.h"
#include "RageLog.h"

struct FileSet
{
	set<File> files;
	RageTimer age;
	void LoadFromDir(const CString &dir);
	void GetFilesMatching(
		const CString &beginning, const CString &containing, const CString &ending,
		vector<CString> &out, bool bOnlyDirs) const;
	void GetFilesEqualTo(const CString &pat, vector<CString> &out, bool bOnlyDirs) const;
};

void FileSet::LoadFromDir(const CString &dir)
{
	age.GetDeltaTime(); /* reset */
	files.clear();

	CString oldpath;
	GetCwd(oldpath);
	if(chdir(dir) == -1) return;

#if defined(WIN32)
	WIN32_FIND_DATA fd;
	HANDLE hFind = FindFirstFile( "*", &fd );

	if( hFind == INVALID_HANDLE_VALUE )
	{
		chdir(oldpath);
		return;
	}

	do {
		if(!strcmp(fd.cFileName, ".") || !strcmp(fd.cFileName, ".."))
			continue;

		File f;
		if(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
			f.dir = true;
		f.name=fd.cFileName;

		files.insert(f);
	} while( FindNextFile( hFind, &fd ) );
	FindClose(hFind);
#else
	DIR *d = opendir(".");

	while(struct dirent *ent = readdir(d))
	{
		if(!strcmp(ent->d_name, ".")) continue;
		if(!strcmp(ent->d_name, "..")) continue;
		
		File f;
		f.dir = IsADirectory(ent->d_name);
		f.name=ent->d_name;

		files.insert(f);
	}
	       
	closedir(d);
#endif
	chdir(oldpath);
}

/* Search for "beginning*containing*ending". */
void FileSet::GetFilesMatching(const CString &beginning, const CString &containing, const CString &ending, vector<CString> &out, bool bOnlyDirs) const
{
	set<File>::const_iterator i = files.lower_bound(File(beginning.c_str()));
	for( ; i != files.end(); ++i)
	{
		if(bOnlyDirs && !i->dir) continue;

		/* Check beginning. */
		if(beginning.size() > i->name.size()) continue; /* can't start with it */
		if(strnicmp(i->name.c_str(), beginning.c_str(), beginning.size())) continue; /* doesn't start with it */

		/* Position the end starts on: */
		int end_pos = int(i->name.size())-int(ending.size());

		/* Check end. */
		if(end_pos < 0) continue; /* can't end with it */
		if(stricmp(i->name.c_str()+end_pos, ending.c_str())) continue; /* doesn't end with it */

		/* Check containing.  Do this last, since it's the slowest (substring
		 * search instead of string match). */
		if(containing.size())
		{
			unsigned pos = i->name.find(containing, beginning.size());
			if(pos == i->name.npos) continue; /* doesn't contain it */
			if(pos + containing.size() > unsigned(end_pos)) continue; /* found it but it overlaps with the end */
		}

		out.push_back(i->name.c_str());
	}
}

void FileSet::GetFilesEqualTo(const CString &str, vector<CString> &out, bool bOnlyDirs) const
{
	set<File>::const_iterator i = files.find(File(str.c_str()));
	if(i == files.end())
		return;

	if(bOnlyDirs && !i->dir)
		return;

	out.push_back(i->name.c_str());
}

/* XXX: this won't work right for URIs, eg \\foo\bar */
bool FilenameDB::ResolvePath(CString &path)
{
	if(path == ".") return true;
	if(path == "") return true;

	path.Replace("\\", "/");

	/* Split path into components. */
	vector<CString> p;
	split(path, "/", p, true);

	/* Resolve each component.  Assume the first component is correct. */
	CString ret = p[0];
	for(unsigned i = 1; i < p.size(); ++i)
	{
		ret += "/";

		vector<CString> lst;
		FileSet &fs = GetFileSet(ret);
		fs.GetFilesEqualTo(p[i], lst, false);

		/* If there were no matches, the path isn't found. */
		if(lst.empty()) return false;

		if(lst.size() > 1)
			LOG->Warn("Ambiguous filenames \"%s\" and \"%s\"",
				lst[0].GetString(), lst[1].GetString());

		ret += lst[0];
	}

	if(path[path.size()-1] == '/')
		path = ret + "/";
	else
		path = ret;
	return true;
}

void FilenameDB::GetFilesMatching(const CString &dir, const CString &beginning, const CString &containing, const CString &ending, vector<CString> &out, bool bOnlyDirs)
{
	FileSet &fs = GetFileSet(dir);
	fs.GetFilesMatching(beginning, containing, ending, out, bOnlyDirs);
}

void FilenameDB::GetFilesEqualTo(const CString &dir, const CString &fn, vector<CString> &out, bool bOnlyDirs)
{
	FileSet &fs = GetFileSet(dir);
	fs.GetFilesEqualTo(fn, out, bOnlyDirs);
}


void FilenameDB::GetFilesSimpleMatch(const CString &dir, const CString &fn, vector<CString> &out, bool bOnlyDirs)
{
	/* Does this contain a wildcard? */
	unsigned first_pos = fn.find_first_of('*');
	if(first_pos == fn.npos)
	{
		/* No; just do a regular search. */
		GetFilesEqualTo(dir, fn, out, bOnlyDirs);
	} else {
		unsigned second_pos = fn.find_first_of('*', first_pos+1);
		if(second_pos == fn.npos)
		{
			/* Only one *: "A*B". */
			GetFilesMatching(dir, fn.substr(0, first_pos), "", fn.substr(first_pos+1), out, bOnlyDirs);
		} else {
			/* Two *s: "A*B*C". */
			GetFilesMatching(dir, 
				fn.substr(0, first_pos),
				fn.substr(first_pos+1, second_pos-first_pos-1),
				fn.substr(second_pos+1), out, bOnlyDirs);
		}
	}
}

FileSet &FilenameDB::GetFileSet(CString dir, bool ResolveCase)
{
	/* Normalize the path. */
	dir.Replace("\\", "/"); /* foo\bar -> foo/bar */
	dir.Replace("//", "/"); /* foo//bar -> foo/bar */

	FileSet *ret;
	map<CString, FileSet *>::iterator i = dirs.find(dir);
	bool reload = false;
	if(i == dirs.end())
	{
		ret = new FileSet;
		dirs[dir] = ret;
		reload = true;
	}
	else
	{
		ret = i->second;
		if(ret->age.PeekDeltaTime() > 30)
			reload = true;
	}

	if(reload)
	{
		CString RealDir = dir;
		if(ResolveCase)
		{
			/* Resolve path cases (path/Path -> PATH/path). */
			ResolvePath(RealDir);

			/* Alias this name, too. */
			dirs[RealDir] = ret;
		}

		ret->LoadFromDir(RealDir);
	}
	return *ret;
}

FilenameDB FDB;

void GetDirListing( CString sPath, CStringArray &AddTo, bool bOnlyDirs, bool bReturnPathToo )
{
	/* If you want the CWD, use ".". */
	ASSERT(!sPath.empty());

	/* XXX: for case-insensitive resolving, we assume the first element is
	 * correct (we need a place to start from); so if sPath is relative,
	 * prepend "./" */

	/* Strip off the last path element and use it as a mask. */
	unsigned pos = sPath.find_last_of("/\\");
	CString fn;
	if(pos != sPath.npos)
	{
		fn = sPath.substr(pos+1);
		sPath = sPath.substr(0, pos+1);
	}

	/* If there was only one path element, or if the last element was empty,
	 * use "*". */
	if(fn.size() == 0)
		fn = "*";

	unsigned start = AddTo.size();
	FDB.GetFilesSimpleMatch(sPath, fn, AddTo, bOnlyDirs);

	if(bReturnPathToo && start < AddTo.size())
	{
		FDB.ResolvePath(sPath);
		while(start < AddTo.size())
		{
			AddTo[start] = sPath + AddTo[start];
			start++;
		}
	}
}

void FilenameDB::FlushDirCache()
{
	dirs.clear();
}

/* Return the last named component of dir:
 * a/b/c -> c
 * a/b/c/ -> c
 */
CString Basename(CString dir)
{
	TrimRight(dir, "/\\");

	unsigned pos = dir.find_last_of("/\\");
	if(pos != dir.npos)
		return dir.substr(pos+1);

	return dir;
}


