#include "global.h"
/*
-----------------------------------------------------------------------------
 File: RageUtil

 Desc: See header.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "RageUtil.h"
#include "RageUtil_FileDB.h"
#include "RageTimer.h"
#include "RageLog.h"

#include <numeric>
#include <time.h>
#include <math.h>
#include <fstream>
#include <map>

unsigned long randseed = time(NULL);

void fapproach( float& val, float other_val, float to_move )
{
	ASSERT( to_move >= 0 );
	if( val == other_val )
		return;
	float fDelta = other_val - val;
	float fSign = fDelta / fabsf( fDelta );
	float fToMove = fSign*to_move;
	if( fabsf(fToMove) > fabsf(fDelta) )
		fToMove = fDelta;	// snap
	val += fToMove;
}

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
	if( !s.size() )
		return false;

	for( int i=0; s[i]; i++ )
		if( s[i] < '0' || s[i] > '9' )
			return false;

	return true;
}

bool IsHexVal( const CString &s )
{
	if( !s.size() )
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
	const int iMinsDisplay = (int)fSecs/60;
	const int iSecsDisplay = (int)fSecs - iMinsDisplay*60;
	const float fLeftoverDisplay = (fSecs - iMinsDisplay*60 - iSecsDisplay) * 100;
	CString sReturn = ssprintf( "%02d:%02d:%02.0f", iMinsDisplay, iSecsDisplay, min(99.0f,fLeftoverDisplay) );
	if( iMinsDisplay >= 60 )
	{
		/* Oops.  Probably a really long endless course.  Do "hh:mm.ss"; use a period
		 * to differentiate between "mm:ss:ms".  I'd much prefer reversing those, since
		 * it makes much more sense to do "mm:ss.ms", but people would probably complain
		 * about "arcade accuracy" ... 
		 */
		sReturn = ssprintf( "%02d:%02d.%02d", iMinsDisplay/60, iMinsDisplay%60, iSecsDisplay );
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

	/* Why was this commented out?  -glenn */
	return s + ssprintf( " (%s)", DXGetErrorString8(hr) );
//	return s;// += ssprintf( " (%s)", DXGetErrorString8(hr) );
}

CString werr_ssprintf( int err, const char *fmt, ...)
{
	char buf[1024] = "";
#ifndef _XBOX
	FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM,
		0, err, 0, buf, sizeof(buf), NULL);
#endif

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

/* Reference: http://www.theorem.com/java/CRC32.java, rewritten by Glenn Maynard.
 * Public domain. */
unsigned int GetHashForString ( CString s )
{
	static unsigned tab[256];
	static bool initted = false;
	if(!initted)
	{
		initted = true;
		const unsigned POLY = 0xEDB88320;

		for(int i = 0; i < 256; ++i)
		{
			tab[i] = i;
			for(int j = 0; j < 8; ++j)
			{
				if(tab[i] & 1) tab[i] = (tab[i] >> 1) ^ POLY;
				else tab[i] >>= 1;
			}
		}
	}

	unsigned crc = 0;
	for(unsigned i = 0; i < s.size(); ++i)
        crc = (crc >> 8) ^ tab[(crc ^ s[i]) & 0xFF];
	return crc;
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
	GetDirListing( sDir+"/*", arrayFiles, false );
	for( unsigned i=0; i<arrayFiles.size(); i++ )
	{
		const CString sFilePath = sDir + arrayFiles[i];
		hash += GetHashForFile( sFilePath );
	}

	return hash; 
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

#if 1
/* PCRE */
#include "pcre/pcre.h"
void Regex::Compile()
{
	const char *error;
	int offset;
	reg = pcre_compile(pattern.c_str(), PCRE_CASELESS, &error, &offset, NULL);

    if(reg == NULL)
		RageException::Throw("Invalid regex: '%s' (%s)", pattern.c_str(), error);

	int ret = pcre_fullinfo( (pcre *) reg, NULL, PCRE_INFO_CAPTURECOUNT, &backrefs);
	ASSERT(ret >= 0);

	backrefs++;
    ASSERT(backrefs < 128);
}

void Regex::Set(const CString &str)
{
	Release();
    pattern=str;
	Compile();
}

void Regex::Release()
{
    pcre_free(reg);
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
    int mat[128*3];
	int ret = pcre_exec( (pcre *) reg, NULL,
		str.data(), str.size(), 0, 0, mat, 128*3);

	if( ret < -1 )
		RageException::Throw("Unexpected return from pcre_exec('%s'): %i",
			pattern.c_str(), ret);

	return ret >= 0;
}

bool Regex::Compare(const CString &str, vector<CString> &matches)
{
    matches.clear();

    int mat[128*3];
	int ret = pcre_exec( (pcre *) reg, NULL,
		str.data(), str.size(), 0, 0, mat, 128*3);

	if( ret < -1 )
		RageException::Throw("Unexpected return from pcre_exec('%s'): %i",
			pattern.c_str(), ret);

	if(ret == -1)
		return false;

    for(unsigned i = 1; i < backrefs; ++i)
    {
		const int start = mat[i*2], end = mat[i*2+1];
        if(start == -1)
            matches.push_back(""); /* no match */
        else
            matches.push_back(str.substr(start, end - start));
    }

    return true;
}
#else
/* GNU regex */
#include "regex.h"
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
#endif

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

bool utf8_is_valid(const CString &str)
{
	unsigned pos = 0;
	while(pos < str.size())
	{
		int len = utf8_get_char_len(str.c_str() + pos);
		if(len == -1)
			return false;
		if( utf8_get_char( str.c_str() + pos ) == INVALID_CHAR )
			return false;

		pos += len;
	}

	return true;
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

CString Capitalize( CString s )	
{
	if( s.GetLength()==0 )
		return "";
	CString s1 = s.Left(1);
	s1.MakeUpper();
	CString s2 = s.Right( s.GetLength()-1 );
	return s1+s2;
}
