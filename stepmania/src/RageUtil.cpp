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

#include <numeric>
#include <time.h>
#include <math.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fstream>
#include "regex.h"
#include <map>
#include <set>

#include "RageTimer.h"
#include "RageLog.h"

#if !defined(WIN32)
#include <dirent.h>
#endif

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
//#include "dxerr8.h"
//#pragma comment(lib, "DxErr8.lib")

CString hr_ssprintf( int hr, const char *fmt, ...)
{
    va_list	va;
    va_start(va, fmt);
    CString s = vssprintf( fmt, va );
    va_end(va);

	return s;// += ssprintf( " (%s)", DXGetErrorString8(hr) );
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

void GetCwd(CString &s)
{
	char buf[PATH_MAX];
	bool ret = getcwd(buf, PATH_MAX) != NULL;
	ASSERT(ret);

	s = buf;
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

bool DoStat(CString sPath, struct stat *st)
{
	TrimRight(sPath, "/\\");
    return stat(sPath.c_str(), st) != -1;
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

bool utf8_is_valid(const CString &str)
{
	unsigned pos = 0;
	while(pos < str.size())
	{
		int len = utf8_get_char_len(str.c_str() + pos);
		if(len == -1)
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
				lst[0].c_str(), lst[1].c_str());

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


CString Capitalize( CString s )	
{
	if( s.GetLength()==0 )
		return "";
	CString s1 = s.Left(1);
	s1.MakeUpper();
	CString s2 = s.Right( s.GetLength()-1 );
	return s1+s2;
}
