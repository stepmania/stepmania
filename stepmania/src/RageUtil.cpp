#include "global.h"
#include "RageUtil.h"
#include "RageMath.h"
#include "RageLog.h"
#include "RageFile.h"

#include <numeric>
#include <ctime>
#include <map>
#include <sys/types.h>
#include <sys/stat.h>

int randseed = time(NULL);

// From "Numerical Recipes in C".
float RandomFloat( int &seed )
{
	const int MASK = 123459876;
	seed ^= MASK;

	const int IA = 16807;
	const int IM = 2147483647;

	const int IQ = 127773;
	const int IR = 2836;

	long k = seed / IQ;
	seed = IA*(seed-k*IQ)-IR*k;
	if( seed < 0 )
			seed += IM;

	const float AM = .999999f / IM;
	float ans = AM * seed;

	seed ^= MASK;
	return ans;
}

RandomGen::RandomGen( unsigned long seed_ )
{
	seed = seed_;
	if(seed == 0)
		seed = time(NULL);
}

int RandomGen::operator() ( int maximum )
{
	return int(RandomFloat( seed ) * maximum);
}


void fapproach( float& val, float other_val, float to_move )
{
	ASSERT_M( to_move >= 0, ssprintf("to_move: %f", to_move) );
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

float HHMMSSToSeconds( const CString &sHHMMSS )
{
	CStringArray arrayBits;
	split( sHHMMSS, ":", arrayBits, false );

	while( arrayBits.size() < 3 )
		arrayBits.insert(arrayBits.begin(), "0" );	// pad missing bits

	float fSeconds = 0;
	fSeconds += atoi( arrayBits[0] ) * 60 * 60;
	fSeconds += atoi( arrayBits[1] ) * 60;
	fSeconds += strtof( arrayBits[2], NULL );

	return fSeconds;
}

CString SecondsToHHMMSS( float fSecs )
{
	const int iMinsDisplay = (int)fSecs/60;
	const int iSecsDisplay = (int)fSecs - iMinsDisplay*60;
	CString sReturn = ssprintf( "%02d:%02d:%02d", iMinsDisplay/60, iMinsDisplay%60, iSecsDisplay );
	return sReturn;
}

CString SecondsToMMSSMsMs( float fSecs )
{
	const int iMinsDisplay = (int)fSecs/60;
	const int iSecsDisplay = (int)fSecs - iMinsDisplay*60;
	const int iLeftoverDisplay = (int) ( (fSecs - iMinsDisplay*60 - iSecsDisplay) * 100 );
	CString sReturn = ssprintf( "%02d:%02d.%02d", iMinsDisplay, iSecsDisplay, min(99,iLeftoverDisplay) );
	return sReturn;
}

CString SecondsToMSSMsMs( float fSecs )
{
	const int iMinsDisplay = (int)fSecs/60;
	const int iSecsDisplay = (int)fSecs - iMinsDisplay*60;
	const int iLeftoverDisplay = (int) ( (fSecs - iMinsDisplay*60 - iSecsDisplay) * 100 );
	CString sReturn = ssprintf( "%01d:%02d.%02d", iMinsDisplay, iSecsDisplay, min(99,iLeftoverDisplay) );
	return sReturn;
}

#include "LuaFunctions.h"
#include "LuaManager.h"
LuaFunction( SecondsToMSSMsMs, SecondsToMSSMsMs( FArg(1) ) )

CString SecondsToMMSSMsMsMs( float fSecs )
{
	const int iMinsDisplay = (int)fSecs/60;
	const int iSecsDisplay = (int)fSecs - iMinsDisplay*60;
	const int iLeftoverDisplay = (int) ( (fSecs - iMinsDisplay*60 - iSecsDisplay) * 1000 );
	CString sReturn = ssprintf( "%02d:%02d.%03d", iMinsDisplay, iSecsDisplay, min(999,iLeftoverDisplay) );
	return sReturn;
}

CString PrettyPercent( float fNumerator, float fDenominator)
{
	return ssprintf("%0.2f%%",fNumerator/fDenominator*100);
}

CString Commify( int iNum ) 
{
	CString sNum = ssprintf("%d",iNum);
	CString sReturn;
	for( unsigned i=0; i<sNum.length(); i++ )
	{
		char cDigit = sNum[sNum.length()-1-i];
		if( i!=0 && i%3 == 0 )
			sReturn = ',' + sReturn;
		sReturn = cDigit + sReturn;
	}
	return sReturn;
}

CString FormatNumberAndSuffix( int i )
{
	CString sSuffix;
	switch( i%10 )
	{
	case 1:         sSuffix = "st"; break;
	case 2:         sSuffix = "nd"; break;
	case 3:         sSuffix = "rd"; break;
	default:        sSuffix = "th"; break;
	}

	// "11th", "113th", etc.
	if( ((i%100) / 10) == 1 )
		sSuffix = "th";

	return ssprintf("%i", i) + sSuffix;
}

struct tm GetLocalTime()
{
	const time_t t = time(NULL);
	struct tm tm;
	localtime_r( &t, &tm );
	return tm;
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

#ifdef _XBOX
#  include <D3DX8Core.h>
#else
#  include <windows.h>
#  include <dxerr8.h>
#  if defined(_MSC_VER) && !defined(_XBOX)
#    pragma comment(lib, "dxerr8.lib")
#  endif
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

	/* Why is FormatMessage returning text ending with \r\n? */
	CString text = buf;
	text.Replace( "\n", "" );
	text.Replace( "\r", " " ); /* foo\r\nbar -> foo bar */
	TrimRight( text ); /* "foo\r\n" -> "foo" */

	va_list	va;
    va_start(va, fmt);
    CString s = vssprintf( fmt, va );
    va_end(va);

	return s += ssprintf( " (%s)", text.c_str() );
}

CString ConvertWstringToACP( wstring s )
{
	if( s.empty() )
		return "";

	int iBytes = WideCharToMultiByte( CP_ACP, 0, s.data(), s.size(), 
					NULL, 0, NULL, FALSE );
	ASSERT_M( iBytes > 0, werr_ssprintf( GetLastError(), "WideCharToMultiByte" ).c_str() );

	CString ret;
	WideCharToMultiByte( CP_ACP, 0, s.data(), s.size(), 
					ret.GetBuf( iBytes ), iBytes, NULL, FALSE );
	ret.RelBuf( iBytes );

	return ret;
}

CString ConvertUTF8ToACP( CString s )
{
	return ConvertWstringToACP( CStringToWstring(s) );
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

CString join( const CString &Delimitor, CStringArray::const_iterator begin, CStringArray::const_iterator end )
{
	if( begin == end )
		return "";

	CString ret;
	while( begin != end )
	{
		ret += *begin;
		++begin;
		if( begin != end )
			ret += Delimitor;
	}

	return ret;
}

template <class S>
static int DelimitorLength( const S &Delimitor )
{
	return Delimitor.size();
}

static int DelimitorLength( char Delimitor )
{
	return 1;
}

static int DelimitorLength( wchar_t Delimitor )
{
	return 1;
}

template <class S, class C>
void do_split( const S &Source, const C Delimitor, vector<S> &AddIt, const bool bIgnoreEmpty )
{
	/* Short-circuit if the source is empty; we want to return an empty vector if
	 * the string is empty, even if bIgnoreEmpty is true. */
	if( Source.empty() )
		return;

	size_t startpos = 0;

	do {
		size_t pos;
		pos = Source.find( Delimitor, startpos );
		if( pos == Source.npos )
			pos = Source.size();

		if( pos-startpos > 0 || !bIgnoreEmpty )
		{
			/* Optimization: if we're copying the whole string, avoid substr; this
			 * allows this copy to be refcounted, which is much faster. */
			if( startpos == 0 && pos-startpos == Source.size() )
				AddIt.push_back(Source);
			else
			{
				const S AddCString = Source.substr(startpos, pos-startpos);
				AddIt.push_back(AddCString);
			}
		}

		startpos = pos+DelimitorLength(Delimitor);
	} while ( startpos <= Source.size() );
}


void split( const CString &Source, const CString &Delimitor, CStringArray &AddIt, const bool bIgnoreEmpty )
{
	if( Delimitor.size() == 1 )
		do_split( Source, Delimitor[0], AddIt, bIgnoreEmpty );
	else
		do_split( Source, Delimitor, AddIt, bIgnoreEmpty );
}

void split( const wstring &Source, const wstring &Delimitor, vector<wstring> &AddIt, const bool bIgnoreEmpty )
{
	if( Delimitor.size() == 1 )
		do_split( Source, Delimitor[0], AddIt, bIgnoreEmpty );
	else
		do_split( Source, Delimitor, AddIt, bIgnoreEmpty );
}

/* Use:

CString str="a,b,c";
int start = 0, size = -1;
while( 1 )
{
	do_split( str, ",", begin, size );
	if( begin == str.end() )
		break;
	str[begin] = 'Q';
}

 */

template <class S>
void do_split( const S &Source, const S &Delimitor, int &begin, int &size, int len, const bool bIgnoreEmpty )
{
	if( size != -1 )
	{
		/* Start points to the beginning of the last delimiter.  Move it up. */
		begin += size+Delimitor.size();
		begin = min( begin, len );
	}

	size = 0;

	if( bIgnoreEmpty )
	{
		/* Skip delims. */
		while( begin + Delimitor.size() < Source.size() &&
			!Source.compare( begin, Delimitor.size(), Delimitor ) )
			++begin;
	}

	/* Where's the string function to find within a substring?  C++ strings apparently
	 * are missing that ... */
	size_t pos;
	if( Delimitor.size() == 1 )
		pos = Source.find( Delimitor[0], begin );
	else
		pos = Source.find( Delimitor, begin );
	if( pos == Source.npos || (int) pos > len )
		pos = len;
	size = pos - begin;
}

void split( const CString &Source, const CString &Delimitor, int &begin, int &size, int len, const bool bIgnoreEmpty )
{
	do_split( Source, Delimitor, begin, size, len, bIgnoreEmpty );

}

void split( const wstring &Source, const wstring &Delimitor, int &begin, int &size, int len, const bool bIgnoreEmpty )
{
	do_split( Source, Delimitor, begin, size, len, bIgnoreEmpty );
}

void split( const CString &Source, const CString &Delimitor, int &begin, int &size, const bool bIgnoreEmpty )
{
	do_split( Source, Delimitor, begin, size, Source.size(), bIgnoreEmpty );
}

void split( const wstring &Source, const wstring &Delimitor, int &begin, int &size, const bool bIgnoreEmpty )
{
	do_split( Source, Delimitor, begin, size, Source.size(), bIgnoreEmpty );
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

CString GetExtension( const CString &sPath )
{
	size_t pos = sPath.rfind( '.' );
	if( pos == sPath.npos )
		return "";

	size_t slash = sPath.find( '/', pos );
	if( slash != sPath.npos )
		return ""; /* rare: path/dir.ext/fn */

	return sPath.substr( pos+1, sPath.size()-pos+1 );
}

CString GetFileNameWithoutExtension( const CString &sPath )
{
	CString sThrowAway, sFName;
	splitpath( sPath, sThrowAway, sFName, sThrowAway );
	return sFName;
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

/*
 * Calculate a standard CRC32.  iCRC should be initialized to 0.
 * References:
 *   http://www.theorem.com/java/CRC32.java,
 *   http://www.faqs.org/rfcs/rfc1952.html
 */
void CRC32( unsigned int &iCRC, const void *pVoidBuffer, size_t iSize )
{
	static unsigned tab[256];
	static bool initted = false;
	if( !initted )
	{
		initted = true;
		const unsigned POLY = 0xEDB88320;

		for( int i = 0; i < 256; ++i )
		{
			tab[i] = i;
			for( int j = 0; j < 8; ++j )
			{
				if( tab[i] & 1 )
					tab[i] = (tab[i] >> 1) ^ POLY;
				else
					tab[i] >>= 1;
			}
		}
	}

	iCRC ^= 0xFFFFFFFF;

	const char *pBuffer = (const char *) pVoidBuffer;
	for( unsigned i = 0; i < iSize; ++i )
		iCRC = (iCRC >> 8) ^ tab[(iCRC ^ pBuffer[i]) & 0xFF];

	iCRC ^= 0xFFFFFFFF;
}

unsigned int GetHashForString ( const CString &s )
{
	unsigned crc = 0;
	CRC32( crc, s.data(), s.size() );
	return crc;
}


/* Return true if "dir" is empty or does not exist. */
bool DirectoryIsEmpty( const CString &dir )
{
	if(dir == "")
		return true;
	if(!DoesFileExist(dir))
		return true;

	CStringArray asFileNames;
	GetDirListing( dir, asFileNames );
	return asFileNames.empty();
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
CString DerefRedir(const CString &_path)
{
	CString path = _path;

	for( int i=0; i<100; i++ )
	{
		if( GetExtension(path) != "redir" )
		{
			return path;
		}

		CString sNewFileName;
		GetFileContents( path, sNewFileName, true );

		/* Empty is invalid. */
		if( sNewFileName == "" )
			return "";

		FixSlashesInPlace( sNewFileName );

		CString path2 = Dirname(path) + sNewFileName;

		CollapsePath( path2 );

		path2 += "*";

		vector<CString> matches;
		GetDirListing( path2, matches, false, true );

		if( matches.empty() )
			RageException::Throw( "The redirect '%s' references a file '%s' which doesn't exist.", path.c_str(), path2.c_str() );
		else if( matches.size() > 1 )
			RageException::Throw( "The redirect '%s' references a file '%s' with multiple matches.", path.c_str(), path2.c_str() );

		path = matches[0];
	}

	RageException::Throw( "Circular redirect '%s'", path.c_str() );
}

bool GetFileContents( const CString &sPath, CString &sOut, bool bOneLine )
{
	/* Don't warn if the file doesn't exist, but do warn if it exists and fails to open. */
	if( !IsAFile(sPath) )
		return false;
	
	RageFile file;
	if( !file.Open(sPath) )
	{
		LOG->Warn( "GetFileContents(%s): %s", sPath.c_str(), file.GetError().c_str() );
		return false;
	}
	
	CString sData;
	int iGot;
	if( bOneLine )
		iGot = file.GetLine( sData );
	else
		iGot = file.Read( sData, file.GetFileSize() );

	if( iGot == -1 )
	{
		LOG->Warn( "GetFileContents(%s): %s", sPath.c_str(), file.GetError().c_str() );
		return false;
	}

	if( bOneLine )
		StripCrnl( sData );
	
	sOut = sData;
	return true;
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




/* Given a UTF-8 byte, return the length of the codepoint (if a start code)
 * or 0 if it's a continuation byte. */
int utf8_get_char_len( char p )
{
	if( !(p & 0x80) ) return 1; /* 0xxxxxxx - 1 */
	if( !(p & 0x40) ) return 1; /* 10xxxxxx - continuation */
	if( !(p & 0x20) ) return 2; /* 110xxxxx */
	if( !(p & 0x10) ) return 3; /* 1110xxxx */
	if( !(p & 0x08) ) return 4; /* 11110xxx */
	if( !(p & 0x04) ) return 5; /* 111110xx */
	if( !(p & 0x02) ) return 6; /* 1111110x */
	return 1; /* 1111111x */
}

static inline bool is_utf8_continuation_byte( char c )
{
	return (c & 0xC0) == 0x80;
}

/* Decode one codepoint at start; advance start and place the result in ch.  If
 * the encoded string is invalid, false is returned. */
bool utf8_to_wchar_ec( const CString &s, unsigned &start, wchar_t &ch )
{
	if( start >= s.size() )
		return false;

	if( is_utf8_continuation_byte( s[start] ) || /* misplaced continuation byte */
	    (s[start] & 0xFE) == 0xFE ) /* 0xFE, 0xFF */
	{
		start += 1;
		return false;
	}

	int len = utf8_get_char_len( s[start] );

	const int first_byte_mask[] = { -1, 0x7F, 0x1F, 0x0F, 0x07, 0x03, 0x01 };

	ch = wchar_t(s[start] & first_byte_mask[len]);

	for( int i = 1; i < len; ++i )
	{
		if( start+i >= s.size() )
		{
			/* We expected a continuation byte, but didn't get one.  Return error, and point
			 * start at the unexpected byte; it's probably a new sequence. */
			start += i;
			return false;
		}

		char byte = s[start+i];
		if( !is_utf8_continuation_byte(byte) )
		{
			/* We expected a continuation byte, but didn't get one.  Return error, and point
			 * start at the unexpected byte; it's probably a new sequence. */
			start += i;
			return false;
		}
		ch = (ch << 6) | byte & 0x3F;
	}

	bool bValid = true;
	{
		unsigned c1 = (unsigned) s[start] & 0xFF;
		unsigned c2 = (unsigned) s[start+1] & 0xFF;
		int c = (c1 << 8) + c2;
		if( (c & 0xFE00) == 0xC000 ||
		    (c & 0xFFE0) == 0xE080 ||
		    (c & 0xFFF0) == 0xF080 ||
		    (c & 0xFFF8) == 0xF880 ||
		    (c & 0xFFFC) == 0xFC80 )
	    {
		    bValid = false;
	    }
	}

	if( ch == 0xFFFE || ch == 0xFFFF )
		bValid = false;

	start += len;
	return bValid;
}

/* Like utf8_to_wchar_ec, but only does enough error checking to prevent crashing. */
bool utf8_to_wchar( const CString &s, unsigned &start, wchar_t &ch )
{
	if( start >= s.size() )
		return false;

	int len = utf8_get_char_len( s[start] );

	if( start+len > s.size() )
	{
		/* We don't have room for enough continuation bytes.  Return error. */
		start += len;
		ch = L'?';
		return false;
	}

	switch( len )
	{
	case 1:
		ch = (s[start+0] & 0x7F);
		break;
	case 2:
		ch = ( (s[start+0] & 0x1F) << 6 ) |
		       (s[start+1] & 0x3F);
		break;
	case 3:
		ch = ( (s[start+0] & 0x0F) << 12 ) |
		     ( (s[start+1] & 0x3F) << 6 ) |
		       (s[start+2] & 0x3F);
		break;
	case 4:
		ch = ( (s[start+0] & 0x07) << 18 ) |
		     ( (s[start+1] & 0x3F) << 12 ) |
		     ( (s[start+2] & 0x3F) << 6 ) |
		     (s[start+3] & 0x3F);
		break;
	case 5:
		ch = ( (s[start+0] & 0x03) << 24 ) |
		     ( (s[start+1] & 0x3F) << 18 ) |
		     ( (s[start+2] & 0x3F) << 12 ) |
		     ( (s[start+3] & 0x3F) << 6 ) |
		     (s[start+4] & 0x3F);
		break;

	case 6:
		ch = ( (s[start+0] & 0x01) << 30 ) |
		     ( (s[start+1] & 0x3F) << 24 ) |
		     ( (s[start+2] & 0x3F) << 18 ) |
		     ( (s[start+3] & 0x3F) << 12) |
		     ( (s[start+4] & 0x3F) << 6 ) |
		     (s[start+5] & 0x3F);
		break;

	}

	start += len;
	return true;
}


/* UTF-8 encode ch and append to out. */
void wchar_to_utf8( wchar_t ch, CString &out )
{
	if( ch < 0x80 ) { out.append( 1, (char) ch ); return; }

	int cbytes = 0;
	if( ch < 0x800 ) cbytes = 1;
	else if( ch < 0x10000 )    cbytes = 2;
	else if( ch < 0x200000 )   cbytes = 3;
	else if( ch < 0x4000000 )  cbytes = 4;
	else cbytes = 5;

	{
		int shift = cbytes*6;
		const int init_masks[] = { 0xC0, 0xE0, 0xF0, 0xF8, 0xFC };
		out.append( 1, (char) (init_masks[cbytes-1] | (ch>>shift)) );
	}

	for( int i = 0; i < cbytes; ++i )
	{
		int shift = (cbytes-i-1)*6;
		out.append( 1, (char) (0x80 | ((ch>>shift)&0x3F)) );
	}
}


wchar_t utf8_get_char( const CString &s )
{
	unsigned start = 0;
	wchar_t ret;
	if( !utf8_to_wchar_ec( s, start, ret ) )
		return INVALID_CHAR;
	return ret;
}



/* Replace invalid sequences in s. */
void utf8_sanitize( CString &s )
{
	CString ret;
	for( unsigned start = 0; start < s.size(); )
	{
		wchar_t ch;
		if( !utf8_to_wchar_ec( s, start, ch ) )
			ch = INVALID_CHAR;

		wchar_to_utf8( ch, ret );
	}

	s = ret;
}


bool utf8_is_valid( const CString &s )
{
	for( unsigned start = 0; start < s.size(); )
	{
		wchar_t ch;
		if( !utf8_to_wchar_ec( s, start, ch ) )
			return false;
	}
	return true;
}

/* Windows tends to drop garbage BOM characters at the start of UTF-8 text files.
 * Remove them. */
void utf8_remove_bom( CString &sLine )
{
	if( !sLine.compare(0, 3, "\xef\xbb\xbf") )
		sLine.erase(0, 3);
}

const wchar_t INVALID_CHAR = 0xFFFD; /* U+FFFD REPLACEMENT CHARACTER */

wstring CStringToWstring( const CString &s )
{
	wstring ret;
	ret.reserve( s.size() );
	for( unsigned start = 0; start < s.size(); )
	{
		char c = s[start];
		if( !(c&0x80) )
		{
			/* ASCII fast path */
			ret += c;
			++start;
			continue;
		}
		
		wchar_t ch;
		if( !utf8_to_wchar( s, start, ch ) )
			ch = INVALID_CHAR;
		ret += ch;
	}

	return ret;
}

CString WStringToCString(const wstring &str)
{
	CString ret;

	for(unsigned i = 0; i < str.size(); ++i)
		wchar_to_utf8( str[i], ret );

	return ret;
}


CString WcharToUTF8( wchar_t c )
{
	CString ret;
	wchar_to_utf8( c, ret );
	return ret;
}


/* Replace &#nnnn; (decimal) &xnnnn; (hex) with corresponding UTF-8 characters. */
void Replace_Unicode_Markers( CString &Text )
{
	unsigned start = 0;
	while(start < Text.size())
	{
		/* Look for &#digits; */
		bool hex = false;
		size_t pos = Text.find("&#", start);
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
	size_t  end = dir.find_last_not_of( "/\\" );
	if( end == dir.npos )
		return "";

	size_t  start = dir.find_last_of( "/\\", end );
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
        if( dir.size() == 1 && dir[0] == '/' )
                return "/";

        int pos = dir.size()-1;
        /* Skip trailing slashes. */
        while( pos >= 0 && dir[pos] == '/' )
                --pos;

        /* Skip the last component. */
        while( pos >= 0 && dir[pos] != '/' )
                --pos;

        if( pos < 0 )
                return "./";

        return dir.substr(0, pos+1);
}

CString Capitalize( const CString &s )	
{
	if( s.GetLength()==0 )
		return "";
	CString s2 = s;
	/* XXX: utf-8 */
	if( !(s2[0] & 0x80) )
		s2[0] = (char) toupper( s2[0] );
	return s2;
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

void FixSlashesInPlace( CString &sPath )
{
	for( unsigned i = 0; i < sPath.size(); ++i )
		if( sPath[i] == '\\' )
			sPath[i] = '/';
}

CString FixSlashes( CString sPath )
{
	FixSlashesInPlace( sPath );
    return sPath;
}

/*
 * Keep trailing slashes, since that can be used to illustrate that a path always
 * represents a directory.
 *
 * foo/bar -> foo/bar
 * foo/bar/ -> foo/bar/
 * foo///bar/// -> foo/bar/
 * foo/bar/./baz -> foo/bar/baz
 * foo/bar/../baz -> foo/baz
 * ../foo -> ../foo
 * ../../foo -> ../../foo
 * ./foo -> foo (if bRemoveLeadingDot), ./foo (if !bRemoveLeadingDot)
 * ./ -> .
 * ./// -> .
 */

void CollapsePath( CString &sPath, bool bRemoveLeadingDot )
{
	/* Don't ignore empty: we do want to keep trailing slashes. */
	CStringArray as;
	split( sPath, "/", as, false );

	for( unsigned i=0; i<as.size(); i++ )
	{
		if( as[i] == ".." && i != 0 )
		{
			/* If the previous element is also "..", then we have a path beginning
			 * with multiple "../"--one .. can't eat another .., since that would
			 * cause "../../foo" to become "foo". */
			if( as[i-1] != ".." )
			{
				as.erase( as.begin()+i-1 );
				as.erase( as.begin()+i-1 );
				i -= 2;
			}
		}
		else if( as[i] == "" && i != 0 && i+1 < as.size() )
		{
			/* Remove empty parts that aren't at the beginning or end;
			 * "foo//bar/" -> "foo/bar/", but "/foo" -> "/foo" and "foo/"
			 * to "foo/". */
			as.erase( as.begin()+i );
			i -= 1;
		}
		else if( as[i] == "." && (bRemoveLeadingDot || i != 0) )
		{
			as.erase( as.begin()+i );
			i -= 1;
		}
	}
	sPath = join( "/", as );
}


bool FromString( const CString &sValue, int &out )
{
	if( sscanf( sValue.c_str(), "%d", &out ) == 1 )
		return true;

	out = 0;
	return false;
}

bool FromString( const CString &sValue, unsigned  &out )
{
	if( sscanf( sValue.c_str(), "%u", &out ) == 1 )
		return true;

	out = 0;
	return false;
}

bool FromString( const CString &sValue, float &out )
{
	const char *endptr = sValue.data() + sValue.size();
	out = strtof( sValue, (char **) &endptr );
	return endptr != sValue.data();
}

bool FromString( const CString &sValue, bool &out )
{
	if( sValue.size() == 0 )
		return false;

	out = (atoi(sValue) != 0);
	return true;
}

CString ToString( int value )
{
	return ssprintf( "%i", value );
}

CString ToString( unsigned value )
{
	return ssprintf( "%u", value );
}

CString ToString( float value )
{
	return ssprintf( "%f", value );
}

CString ToString( bool value )
{
	return ssprintf( "%i", value );
}


//
// Helper function for reading/writing scores
//
bool FileRead(RageFileBasic& f, CString& sOut)
{
	if (f.AtEOF())
		return false;
	if( f.GetLine(sOut) == -1 )
		return false;
	return true;
}

bool FileRead(RageFileBasic& f, int& iOut)
{
	CString s;
	if (!FileRead(f, s))
		return false;
	iOut = atoi(s);
	return true;
}

bool FileRead(RageFileBasic& f, unsigned& uOut)
{
	CString s;
	if (!FileRead(f, s))
		return false;
	uOut = atoi(s);
	return true;
}

bool FileRead(RageFileBasic& f, float& fOut)
{
	CString s;
	if (!FileRead(f, s))
		return false;
	fOut = strtof( s, NULL );
	return true;
}

void FileWrite(RageFileBasic& f, const CString& sWrite)
{
	f.PutLine( sWrite );
}

void FileWrite(RageFileBasic& f, int iWrite)
{
	f.PutLine( ssprintf("%d", iWrite) );
}

void FileWrite(RageFileBasic& f, size_t uWrite)
{
	f.PutLine( ssprintf("%i", (int)uWrite) );
}

void FileWrite(RageFileBasic& f, float fWrite)
{
	f.PutLine( ssprintf("%f", fWrite) );
}

bool FileCopy( CString sSrcFile, CString sDstFile )
{
	if( !sSrcFile.CompareNoCase(sDstFile) )
	{
		LOG->Warn( "Tried to copy \"%s\" over itself", sSrcFile.c_str() );
		return false;
	}

	RageFile in;
	if( !in.Open(sSrcFile, RageFile::READ) )
		return false;

	RageFile out;
	if( !out.Open(sDstFile, RageFile::WRITE) )
		return false;

	CString sError;
	if( !FileCopy(in, out, sError) )
	{
		LOG->Warn( "FileCopy(%s,%s): %s",
				sSrcFile.c_str(), sDstFile.c_str(), sError.c_str() );
		return false;
	}

	return true;
}

bool FileCopy( RageFileBasic &in, RageFileBasic &out, CString &sError, bool *bReadError )
{
	while(1)
	{
		CString data;
		if( in.Read(data, 1024*32) == -1 )
		{
			sError = ssprintf( "read error: %s", in.GetError().c_str() );
			if( bReadError != NULL )
				*bReadError = true;
			return false;
		}
		if( data.empty() )
			break;
		int i = out.Write(data);
		if( i == -1 )
		{
			sError = ssprintf( "write error: %s", out.GetError().c_str() );
			if( bReadError != NULL )
				*bReadError = false;
			return false;
		}
	}

	if( out.Flush() == -1 )
	{
		sError = ssprintf( "write error: %s", out.GetError().c_str() );
		if( bReadError != NULL )
			*bReadError = false;
		return false;
	}

	return true;
}

/*
 * Copyright (c) 2001-2004 Chris Danford, Glenn Maynard
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
