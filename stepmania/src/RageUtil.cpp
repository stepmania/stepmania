#include "global.h"
/*
-----------------------------------------------------------------------------
 File: RageUtil

 Desc: See header.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/
#include "RageMath.h"
#include "RageUtil.h"
#include "RageUtil_FileDB.h"
#include "RageLog.h"
#include "arch/arch.h"

#include <numeric>
#include <time.h>
#include <math.h>
#include "RageFile.h"
#include <map>
#include <sys/types.h>
#include <sys/stat.h>

unsigned long randseed = time(NULL);

RandomGen::RandomGen( unsigned long seed_ )
{
	seed = seed_;
	if(seed == 0)
		seed = time(NULL);
}

int RandomGen::operator() ( int maximum )
{
	seed = 1664525L * seed + 1013904223L;
	return (seed >> 2) % maximum;
}


void fapproach( float& val, float other_val, float to_move )
{
	RAGE_ASSERT_M( to_move >= 0, ssprintf("to_move: %f", to_move) );
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
#include "windows.h"
#ifdef _XBOX
#include "D3DX8Core.h"
#else
#include "Dxerr8.h"
#pragma comment(lib, "Dxerr8.lib")
#endif

CString hr_ssprintf( int hr, const char *fmt, ...)
{
    va_list	va;
    va_start(va, fmt);
    CString s = vssprintf( fmt, va );
    va_end(va);

#ifdef _XBOX
	char szError[1024] = "";
	D3DXGetErrorString( hr, szError, sizeof(szError) );
#else	
	const char *szError = DXGetErrorString8( hr );
#endif

	return s + ssprintf( " (%s)", szError );
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
void do_split( const S &Source, const S &Delimitor, vector<S> &AddIt, const bool bIgnoreEmpty )
{
	unsigned startpos = 0;

	do {
		unsigned pos;
		if( Delimitor.size() == 1 )
			pos = Source.find( Delimitor[0], startpos );
		else
			pos = Source.find( Delimitor, startpos );
		if( pos == Source.npos )
			pos = Source.size();

		if( pos-startpos > 0 || !bIgnoreEmpty )
		{
			const S AddCString = Source.substr(startpos, pos-startpos);
			AddIt.push_back(AddCString);
		}

		startpos = pos+Delimitor.size();
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
void splitpath( const CString &Path, CString& Dir, CString& Filename, CString& Ext )
{
	Dir = Filename = Ext = "";

	CStringArray mat;

	/* One level of escapes for the regex, one for C. Ew. 
	 * This is really:
	 * ^(.*[\\/])?(.*)$    */
	static Regex sep("^(.*[\\\\/])?(.*)$");
	bool check = sep.Compare(Path, mat);
	ASSERT(check);

	Dir = mat[0];
	const CString Base = mat[1];

	/* ^(.*)(\.[^\.]+)$ */
	static Regex SplitExt("^(.*)(\\.[^\\.]+)$");
	if( SplitExt.Compare(Base, mat) )
	{
		Filename = mat[0];
		Ext = mat[1];
	} else
		Filename = Base;
}


/* "foo.bar", "baz" -> "foo.baz"
 * "foo", "baz" -> "foo.baz"
 * "foo.bar", "" -> "foo" */
CString SetExtension( const CString &path, const CString &ext )
{
	CString Dir, FName, OldExt;
	splitpath( path, Dir, FName, OldExt );
	return Dir + FName + (ext.size()? ".":"") + ext;
}

CString GetExtension( CString sPath )
{
	unsigned pos = sPath.rfind( '.' );
	if( pos == sPath.npos )
		return "";

	unsigned slash = sPath.find( '/', pos );
	if( slash != sPath.npos )
		return ""; /* rare: path/dir.ext/fn */

	return sPath.substr( pos+1, sPath.size()-pos+1 );
}

CString GetCwd()
{
#ifdef _XBOX
	return SYS_BASE_PATH;
#else
	char buf[PATH_MAX];
	bool ret = getcwd(buf, PATH_MAX) != NULL;
	ASSERT(ret);
	return buf;
#endif
}

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
	hash += GetFileModTime( sPath );

	return hash;
}

unsigned int GetHashForDirectory( CString sDir )
{
	unsigned int hash = 0;

	hash += GetHashForFile( sDir );

	CStringArray arrayFiles;
	GetDirListing( sDir+"*", arrayFiles, false );
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
	while( s.size() && (s[s.size()-1] == '\r' || s[s.size()-1] == '\n') )
		s.erase(s.size()-1);
}

/* path is a .redir pathname.  Read it and return the real one. */
CString DerefRedir(const CString &path)
{
	if( GetExtension(path) != "redir" )
	{
		CString path2 = path;
		ResolvePath( path2 );
		return path2;
	}

	CString sNewFileName = GetRedirContents( path );

	/* Empty is invalid. */
	if(sNewFileName == "")
		return "";

	FixSlashesInPlace( sNewFileName );

	CString path2 = Dirname(path) + sNewFileName;

	CollapsePath( path2 );

	ResolvePath( path2 );

	if( !DoesFileExist(path2) )
		RageException::Throw( "The redirect '%s' references a file '%s' which doesn't exist.", path.c_str(), path2.c_str() );

	return path2;
}

/* XXX: This can be used to read one line from any file; rename it. */
CString GetRedirContents(const CString &path)
{
	RageFile file(path);
	CString sNewFileName = file.GetLine();
	
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
CString Basename( const CString &dir )
{
	unsigned end = dir.find_last_not_of( "/\\" );
	if( end == dir.npos )
		return "";

	unsigned start = dir.find_last_of( "/\\", end );
	if( start == dir.npos )
		start = 0;
	else
		++start;

	return dir.substr( start, end-start+1 );
}

/* Return all but the last named component of dir:
 * a/b/c -> a/b/
 * a/b/c/ -> a/b/
 * c/ -> ./
 * /foo -> /
 * / -> /
 */
CString Dirname( const CString &dir )
{
        /* Special case: "/" -> "/". */
        if( dir.size() == 1 && dir[0] == SLASH[0] )
                return SLASH;

        int pos = dir.size()-1;
        /* Skip trailing slashes. */
        while( pos >= 0 && dir[pos] == SLASH[0] )
                --pos;

        /* Skip the last component. */
        while( pos >= 0 && dir[pos] != SLASH[0] )
                --pos;

        if( pos < 0 )
                return "." SLASH;

        return dir.substr(0, pos+1);
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

char char_traits_char_nocase::uptab[256] =
{
	'\x00','\x01','\x02','\x03','\x04','\x05','\x06','\x07','\x08','\x09','\x0A','\x0B','\x0C','\x0D','\x0E','\x0F',
	'\x10','\x11','\x12','\x13','\x14','\x15','\x16','\x17','\x18','\x19','\x1A','\x1B','\x1C','\x1D','\x1E','\x1F',
	'\x20','\x21','\x22','\x23','\x24','\x25','\x26','\x27','\x28','\x29','\x2A','\x2B','\x2C','\x2D','\x2E','\x2F',
	'\x30','\x31','\x32','\x33','\x34','\x35','\x36','\x37','\x38','\x39','\x3A','\x3B','\x3C','\x3D','\x3E','\x3F',
	'\x40','\x41','\x42','\x43','\x44','\x45','\x46','\x47','\x48','\x49','\x4A','\x4B','\x4C','\x4D','\x4E','\x4F',
	'\x50','\x51','\x52','\x53','\x54','\x55','\x56','\x57','\x58','\x59','\x5A','\x5B','\x5C','\x5D','\x5E','\x5F',
	'\x60','\x41','\x42','\x43','\x44','\x45','\x46','\x47','\x48','\x49','\x4A','\x4B','\x4C','\x4D','\x4E','\x4F',
	'\x50','\x51','\x52','\x53','\x54','\x55','\x56','\x57','\x58','\x59','\x5A','\x7B','\x7C','\x7D','\x7E','\x7F',
	'\x80','\x81','\x82','\x83','\x84','\x85','\x86','\x87','\x88','\x89','\x8A','\x8B','\x8C','\x8D','\x8E','\x8F',
	'\x90','\x91','\x92','\x93','\x94','\x95','\x96','\x97','\x98','\x99','\x9A','\x9B','\x9C','\x9D','\x9E','\x9F',
	'\xA0','\xA1','\xA2','\xA3','\xA4','\xA5','\xA6','\xA7','\xA8','\xA9','\xAA','\xAB','\xAC','\xAD','\xAE','\xAF',
	'\xB0','\xB1','\xB2','\xB3','\xB4','\xB5','\xB6','\xB7','\xB8','\xB9','\xBA','\xBB','\xBC','\xBD','\xBE','\xBF',
	'\xC0','\xC1','\xC2','\xC3','\xC4','\xC5','\xC6','\xC7','\xC8','\xC9','\xCA','\xCB','\xCC','\xCD','\xCE','\xCF',
	'\xD0','\xD1','\xD2','\xD3','\xD4','\xD5','\xD6','\xD7','\xD8','\xD9','\xDA','\xDB','\xDC','\xDD','\xDE','\xDF',
	'\xE0','\xE1','\xE2','\xE3','\xE4','\xE5','\xE6','\xE7','\xE8','\xE9','\xEA','\xEB','\xEC','\xED','\xEE','\xEF',
	'\xF0','\xF1','\xF2','\xF3','\xF4','\xF5','\xF6','\xF7','\xF8','\xF9','\xFA','\xFB','\xFC','\xFD','\xFE','\xFF',
};
