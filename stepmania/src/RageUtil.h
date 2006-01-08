/* RageUtil - Miscellaneous helper macros and functions.  */

#ifndef RAGE_UTIL_H
#define RAGE_UTIL_H

#include <map>
#include <vector>
#include "Foreach.h"

#define SAFE_DELETE(p)       { delete (p);     (p)=NULL; }
#define SAFE_DELETE_ARRAY(p) { delete[] (p);   (p)=NULL; }

#define ZERO(x)	memset(&x, 0, sizeof(x))
#define COPY(a,b) { ASSERT(sizeof(a)==sizeof(b)); memcpy(&(a), &(b), sizeof(a)); }
#define ARRAYSIZE(a) (sizeof(a) / sizeof((a)[0]))

/* Common harmless mismatches.  All min(T,T) and max(T,T) cases are handled
 * by the generic template we get from <algorithm>. */
inline float min( float a, int b ) { return a < b? a:b; }
inline float min( int a, float b ) { return a < b? a:b; }
inline float max( float a, int b ) { return a > b? a:b; }
inline float max( int a, float b ) { return a > b? a:b; }
inline unsigned long min( unsigned int a, unsigned long b ) { return a < b? a:b; }
inline unsigned long min( unsigned long a, unsigned int b ) { return a < b? a:b; }
inline unsigned long max( unsigned int a, unsigned long b ) { return a > b? a:b; }
inline unsigned long max( unsigned long a, unsigned int b ) { return a > b? a:b; }

/* Traditional defines.  Only use this if you absolutely need
 * a constant value. */
#ifndef MAX
#define MAX(a,b)            (((a) > (b)) ? (a) : (b))
#endif

#ifndef MIN
#define MIN(a,b)            (((a) < (b)) ? (a) : (b))
#endif

#define clamp(val,low,high)		( max( (low), min((val),(high)) ) )

// Scales x so that l1 corresponds to l2 and h1 corresponds to h2.  Does not modify x, MUST assign the result to something!
// Do the multiply before the divide to that integer scales have more precision.
#define SCALE(x, l1, h1, l2, h2)	(((x) - (l1)) * ((h2) - (l2)) / ((h1) - (l1)) + (l2))

inline bool CLAMP( int &x, int l, int h )
{
	if (x > h)		{ x = h; return true; }
	else if (x < l) { x = l; return true; }
	return false;
}
inline bool CLAMP( unsigned &x, unsigned l, unsigned h )
{
	if (x > h)		{ x = h; return true; }
	else if (x < l) { x = l; return true; }
	return false;
}
inline bool CLAMP( float &x, float l, float h )
{
	if (x > h)		{ x = h; return true; }
	else if (x < l) { x = l; return true; }
	return false;
}

inline void wrap( int &x, int n )
{
	if (x<0)
		x += ((-x/n)+1)*n;
	x %= n;
}
inline void wrap( unsigned &x, unsigned n )
{
	x %= n;
}
inline void wrap( float &x, float n )
{
	if (x<0)
		x += truncf(((-x/n)+1))*n;
	x = fmodf(x,n);
}

template<class T>
void CircularShift( vector<T> &v, int dist )
{
	for( int i = abs(dist); i>0; i-- )
	{
		if( dist > 0 )
		{
			T t = v[0];
			v.erase( v.begin() );
			v.push_back( t );
		}
		else
		{
			T t = v.back();
			v.erase( v.end()-1 );
			v.insert( v.begin(), t );
		}
	}
}

/*
 * We only have unsigned swaps; byte swapping a signed value doesn't make sense. 
 *
 * Platform-specific, optimized versions are defined in arch_setup, with the names
 * ArchSwap32, ArchSwap24, and ArchSwap16; we define them to their real names here,
 * to force inclusion of this file when swaps are in use (to prevent different dependencies
 * on different systems).
 */
#ifdef HAVE_BYTE_SWAPS
#define Swap32 ArchSwap32
#define Swap24 ArchSwap24
#define Swap16 ArchSwap16
#else
inline uint32_t Swap32( uint32_t n )
{
	return (n >> 24) |
			((n >>  8) & 0x0000FF00) |
			((n <<  8) & 0x00FF0000) |
			(n << 24);
}

inline uint32_t Swap24( uint32_t n )
{
	return Swap32(n) >> 8; // xx223344 -> 443322xx -> 00443322
}

inline uint16_t Swap16( uint16_t n )
{
	return (n >>  8) | (n <<  8);
}
#endif

#if defined(ENDIAN_LITTLE)
inline uint32_t Swap32LE( uint32_t n ) { return n; }
inline uint32_t Swap24LE( uint32_t n ) { return n; }
inline uint16_t Swap16LE( uint16_t n ) { return n; }
inline uint32_t Swap32BE( uint32_t n ) { return Swap32( n ); }
inline uint32_t Swap24BE( uint32_t n ) { return Swap24( n ); }
inline uint16_t Swap16BE( uint16_t n ) { return Swap16( n ); }
#else
inline uint32_t Swap32BE( uint32_t n ) { return n; }
inline uint32_t Swap24BE( uint32_t n ) { return n; }
inline uint16_t Swap16BE( uint16_t n ) { return n; }
inline uint32_t Swap32LE( uint32_t n ) { return Swap32( n ); }
inline uint32_t Swap24LE( uint32_t n ) { return Swap24( n ); }
inline uint16_t Swap16LE( uint16_t n ) { return Swap16( n ); }
#endif

extern int randseed;

float RandomFloat( int &seed );

inline float RandomFloat()
{
	return RandomFloat( randseed );
}

// Returns a float between dLow and dHigh inclusive
inline float RandomFloat( float fLow, float fHigh )
{
	return SCALE( RandomFloat(), 0.0f, 1.0f, fLow, fHigh );
}

// Returns an integer between nLow and nHigh inclusive
inline int RandomInt( int nLow, int nHigh )
{
	return int( RandomFloat() * (nHigh - nLow + 1) + nLow );
}

/* Alternative: */
class RandomGen
{
	int seed;

public:
	RandomGen( unsigned long seed = 0 );
	int operator() ( int maximum = INT_MAX-1 );
};


// Simple function for generating random numbers
inline float randomf( const float low=-1.0f, const float high=1.0f )
{
    return RandomFloat( low, high );
}

/* return f rounded to the nearest multiple of fRoundInterval */
inline float Quantize( const float f, const float fRoundInterval )
{
	return int( (f + fRoundInterval/2)/fRoundInterval ) * fRoundInterval;
}

inline int Quantize( const int i, const int iRoundInterval )
{
	return int( (i + iRoundInterval/2)/iRoundInterval ) * iRoundInterval;
}

/* return f truncated to the nearest multiple of fTruncInterval */
inline float ftruncf( const float f, const float fTruncInterval )
{
	return int( (f)/fTruncInterval ) * fTruncInterval;
}

/* Return i rounded up to the nearest multiple of iInterval. */
inline int QuantizeUp( int i, int iInterval )
{
	return int( (i+iInterval-1)/iInterval ) * iInterval;
}

inline float QuantizeUp( float i, float iInterval )
{
	return ceilf( i/iInterval ) * iInterval;
}

/* Return i rounded down to the nearest multiple of iInterval. */
inline int QuantizeDown( int i, int iInterval )
{
	return int( (i-iInterval+1)/iInterval ) * iInterval;
}

inline float QuantizeDown( float i, float iInterval )
{
	return floorf( i/iInterval ) * iInterval;
}

// Move val toward other_val by to_move.
void fapproach( float& val, float other_val, float to_move );

/* Return a positive x mod y. */
float fmodfp( float x, float y );

int power_of_two( int input );
bool IsAnInt( const RString &s );
bool IsHexVal( const RString &s );
float HHMMSSToSeconds( const RString &sHMS );
RString SecondsToHHMMSS( float fSecs );
RString SecondsToMSSMsMs( float fSecs );
RString SecondsToMMSSMsMs( float fSecs );
RString SecondsToMMSSMsMsMs( float fSecs );
RString PrettyPercent( float fNumerator, float fDenominator );
inline RString PrettyPercent( int fNumerator, int fDenominator ) { return PrettyPercent( float(fNumerator), float(fDenominator) ); }
RString Commify( int iNum );
RString FormatNumberAndSuffix( int i );


struct tm GetLocalTime();

RString ssprintf( const RString &fmt, ...) PRINTF(1,2);
RString ssprintf( const char *fmt, ...) PRINTF(1,2);
RString vssprintf( const char *fmt, va_list argList );

#ifdef WIN32
RString hr_ssprintf( int hr, const char *fmt, ...);
RString werr_ssprintf( int err, const char *fmt, ...);
RString ConvertWstringToACP( wstring s );
RString ConvertUTF8ToACP( RString s );
#endif

/*
 * Splits a Path into 4 parts (Directory, Drive, Filename, Extention).  Supports UNC path names.
 * If Path is a directory (eg. c:\games\stepmania"), append a slash so the last
 * element will end up in Dir, not FName: "c:\games\stepmania\".
 * */
void splitpath( const RString &Path, RString &Dir, RString &Filename, RString &Ext );

RString SetExtension( const RString &path, const RString &ext );
RString GetExtension( const RString &sPath );
RString GetFileNameWithoutExtension( const RString &sPath );
void MakeValidFilename( CString &sName );

typedef int longchar;
extern const wchar_t INVALID_CHAR;

int utf8_get_char_len( char p );
bool utf8_to_wchar( const RString &s, unsigned &start, wchar_t &ch );
bool utf8_to_wchar_ec( const RString &s, unsigned &start, wchar_t &ch );
void wchar_to_utf8( wchar_t ch, RString &out );
wchar_t utf8_get_char( const RString &s );
bool utf8_is_valid( const RString &s );
void utf8_remove_bom( RString &s );

RString WStringToRString( const wstring &sString );
RString WcharToUTF8( wchar_t c );
wstring RStringToWstring( const RString &sString );

struct LanguageInfo
{
	const char *szIsoCode;
	const char *szEnglishName;
	const char *szNativeName;	// empty string if not available
};
void GetLanguageInfos( vector<const LanguageInfo*> &vAddTo );
const LanguageInfo *GetLanguageInfo( const RString &sIsoCode );
RString GetLanguageNameFromISO639Code( RString sName );

// Splits a RString into an vector<RString> according the Delimitor.
void split( const RString &sSource, const RString &sDelimitor, vector<RString>& asAddIt, const bool bIgnoreEmpty = true );
void split( const wstring &sSource, const wstring &sDelimitor, vector<wstring> &asAddIt, const bool bIgnoreEmpty = true );

/* In-place split. */
void split( const RString &sSource, const RString &sDelimitor, int &iBegin, int &iSize, const bool bIgnoreEmpty = true );
void split( const wstring &sSource, const wstring &sDelimitor, int &iBegin, int &iSize, const bool bIgnoreEmpty = true );

/* In-place split of partial string. */
void split( const RString &sSource, const RString &sDelimitor, int &iBegin, int &iSize, int iLen, const bool bIgnoreEmpty ); /* no default to avoid ambiguity */
void split( const wstring &sSource, const wstring &sDelimitor, int &iBegin, int &iSize, int iLen, const bool bIgnoreEmpty );

// Joins a vector<RString> to create a RString according the Deliminator.
RString join( const RString &sDelimitor, const vector<RString>& sSource );
RString join( const RString &sDelimitor, vector<RString>::const_iterator begin, vector<RString>::const_iterator end );

RString GetCwd();

void SetCommandlineArguments( int argc, char **argv );
bool GetCommandlineArgument( const RString &option, RString *argument=NULL, int iIndex=0 );
extern int g_argc;
extern char **g_argv;

void CRC32( unsigned int &iCRC, const void *pBuffer, size_t iSize );
unsigned int GetHashForString( const RString &s );
unsigned int GetHashForFile( const RString &sPath );
unsigned int GetHashForDirectory( const RString &sDir );	// a hash value that remains the same as long as nothing in the directory has changed
bool DirectoryIsEmpty( const RString &sPath );

bool CompareRStringsAsc( const RString &sStr1, const RString &sStr2 );
bool CompareRStringsDesc( const RString &sStr1, const RString &sStr2 );
void SortRStringArray( vector<RString> &asAddTo, const bool bSortAscending = true );

/* Find the mean and standard deviation of all numbers in [start,end). */
float calc_mean( const float *pStart, const float *pEnd );
float calc_stddev( const float *pStart, const float *pEnd );

template<class T1, class T2>
int FindIndex( T1 begin, T1 end, const T2 *p )
{
	T1 iter = find( begin, end, p );
	if( iter == end )
		return -1;
	return iter - begin;
}

/* Useful for objects with no operator-, eg. map::iterator (more convenient than advance). */
template<class T>
inline T Increment( T a ) { ++a; return a; }
template<class T>
inline T Decrement( T a ) { --a; return a; }

void TrimLeft( RString &sStr, const char *szTrim = "\r\n\t " );
void TrimRight( RString &sStr, const char *szTrim = "\r\n\t " );
void StripCrnl( RString &sStr );
bool BeginsWith( const RString &sTestThis, const RString &sBeginning );
bool EndsWith( const RString &sTestThis, const RString &sEnding );

void StripCvs( vector<RString> &vs );	// remove all items that end in "cvs"

RString DerefRedir( const RString &sPath );
bool GetFileContents( const RString &sPath, RString &sOut, bool bOneLine = false );

class Regex
{
public:
	Regex( const RString &sPat = "" );
	Regex( const Regex &rhs );
	Regex &operator=( const Regex &rhs );
	~Regex();
	bool IsSet() const { return !m_sPattern.empty(); }
	void Set( const RString &str );
	bool Compare( const RString &sStr );
	bool Compare( const RString &sStr, vector<RString> &asMatches );
	bool Replace( const RString &sReplacement, const RString &sSubject, RString &sOut );

private:
    void Compile();
    void Release();

	void *m_pReg;
	unsigned m_iBackrefs;
    RString m_sPattern;
};


void ReplaceEntityText( RString &sText, const map<RString,RString> &m );
void ReplaceEntityText( RString &sText, const map<char,RString> &m );
void Replace_Unicode_Markers( RString &Text );
RString WcharDisplayText( wchar_t c );

RString Basename( const RString &dir );
RString Dirname( const RString &dir );
RString Capitalize( const RString &s );

#ifndef WIN32
#include <unistd.h> /* correct place with correct definitions */
#endif

/* ASCII-only case insensitivity. */
struct char_traits_char_nocase: public char_traits<char>
{
	static char uptab[256];

	static inline bool eq( char c1, char c2 )
	{ return uptab[(unsigned char)c1] == uptab[(unsigned char)c2]; }

	static inline bool ne( char c1, char c2 )
	{ return uptab[(unsigned char)c1] != uptab[(unsigned char)c2]; }

	static inline bool lt( char c1, char c2 )
	{ return uptab[(unsigned char)c1] < uptab[(unsigned char)c2]; }

	static int compare( const char* s1, const char* s2, size_t n )
	{
		int ret = 0;
		while( n-- )
		{
			ret = fasttoupper(*s1++) - fasttoupper(*s2++);
			if( ret != 0 )
				break;
		}
		return ret;
	}

	static inline char fasttoupper(char a)
	{
		return uptab[(unsigned char)a];
	}
	
	static const char *find( const char* s, int n, char a )
	{
		a = fasttoupper(a);
		while( n-- > 0 && fasttoupper(*s) != a )
			++s;

		if(fasttoupper(*s) == a)
			return s;
		return NULL;
	}
};
typedef basic_string<char,char_traits_char_nocase> istring;

/* Compatibility/convenience shortcuts.  These are actually defined in RageFileManager.h, but
 * declared here since they're used in many places. */
void GetDirListing( const RString &sPath, vector<RString> &AddTo, bool bOnlyDirs=false, bool bReturnPathToo=false );
void GetDirListingRecursive( const RString &sDir, const RString &sMatch, vector<RString> &AddTo );	/* returns path too */
bool DeleteRecursive( const RString &sDir );	/* delete the dir and all files/subdirs inside it */
bool DoesFileExist( const RString &sPath );
bool IsAFile( const RString &sPath );
bool IsADirectory( const RString &sPath );
unsigned GetFileSizeInBytes( const RString &sFilePath );
void FlushDirCache();

// call FixSlashes on any path that came from the user
void FixSlashesInPlace( RString &sPath );
RString FixSlashes( RString sPath );
void CollapsePath( RString &sPath, bool bRemoveLeadingDot=false );

bool FromString( const RString &sValue, int &out );
bool FromString( const RString &sValue, unsigned  &out );
bool FromString( const RString &sValue, float &out );
bool FromString( const RString &sValue, bool &out );
inline bool FromString( const RString &sValue, RString &out ) { out = sValue; return true; }

RString ToString( int value );
RString ToString( unsigned value );
RString ToString( float value );
RString ToString( bool value );
inline RString ToString( const RString &value ) { return value; }

// helper file functions used by Bookkeeper and ProfileManager
class RageFileBasic;
bool FileRead( RageFileBasic& f, RString& sOut );
bool FileRead( RageFileBasic& f, int& iOut );
bool FileRead( RageFileBasic& f, unsigned& uOut );
bool FileRead( RageFileBasic& f, float& fOut );
void FileWrite( RageFileBasic& f, const RString& sWrite );
void FileWrite( RageFileBasic& f, int iWrite );
void FileWrite( RageFileBasic& f, size_t uWrite );
void FileWrite( RageFileBasic& f, float fWrite );

bool FileCopy( RString sSrcFile, RString sDstFile );
bool FileCopy( RageFileBasic &in, RageFileBasic &out, RString &sError, bool *bReadError = NULL );

template<class T>
bool VectorsAreEqual( const T &a, const T &b )
{
	if( a.size() != b.size() )
		return false;
	
	for( unsigned i=0; i<a.size(); i++ )
	{
		if( a[i] != b[i] )
			return false;
	}
	
	return true;
}

template<class T>
void GetAsNotInBs( const vector<T> &as, const vector<T> &bs, vector<T> &difference )
{
	vector<T> bsUnmatched = bs;
	// Cannot use FOREACH_CONST here because vector<T>::const_iterator is an implicit type.
	for( typename vector<T>::const_iterator a = as.begin(); a != as.end(); ++a )
	{
		typename vector<T>::iterator iter = find( bsUnmatched.begin(), bsUnmatched.end(), *a );
		if( iter != bsUnmatched.end() )
			bsUnmatched.erase( iter );
		else
			difference.push_back( *a );
	}
}

template<class T>
void GetConnectsDisconnects( const vector<T> &before, const vector<T> &after, vector<T> &disconnects, vector<T> &connects )
{
	GetAsNotInBs( before, after, disconnects );
	GetAsNotInBs( after, before, connects );
}


#endif

/*
 * Copyright (c) 2001-2005 Chris Danford, Glenn Maynard
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
