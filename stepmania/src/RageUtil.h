/* RageUtil - Miscellaneous helper macros and functions.  */

#ifndef RAGE_UTIL_H
#define RAGE_UTIL_H

#define SAFE_DELETE(p)       { delete (p);     (p)=NULL; }
#define SAFE_DELETE_ARRAY(p) { delete[] (p);   (p)=NULL; }

#define ZERO(x)	memset(&x, 0, sizeof(x))
#define COPY(a,b) { ASSERT(sizeof(a)==sizeof(b)); memcpy(&(a), &(b), sizeof(a)); }
#define ARRAYSIZE(a) (sizeof(a) / sizeof((a)[0]))

/* Common harmless mismatches.  All min(T,T) and max(T,T) cases are handled
 * by the generic template we get from <algorithm>. */
inline float min(float a, int b) { return a < b? a:b; }
inline float min(int a, float b) { return a < b? a:b; }
inline float max(float a, int b) { return a > b? a:b; }
inline float max(int a, float b) { return a > b? a:b; }
inline unsigned long min(unsigned int a, unsigned long b) { return a < b? a:b; }
inline unsigned long min(unsigned long a, unsigned int b) { return a < b? a:b; }
inline unsigned long max(unsigned int a, unsigned long b) { return a > b? a:b; }
inline unsigned long max(unsigned long a, unsigned int b) { return a > b? a:b; }

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

inline bool CLAMP(int &x, int l, int h)
{
	if (x > h)		{ x = h; return true; }
	else if (x < l) { x = l; return true; }
	return false;
}
inline bool CLAMP(unsigned &x, unsigned l, unsigned h)
{
	if (x > h)		{ x = h; return true; }
	else if (x < l) { x = l; return true; }
	return false;
}
inline bool CLAMP(float &x, float l, float h)
{
	if (x > h)		{ x = h; return true; }
	else if (x < l) { x = l; return true; }
	return false;
}

inline void wrap(int &x, int n)
{
	if (x<0)
		x += ((-x/n)+1)*n;
	x %= n;
}
inline void wrap(unsigned &x, unsigned n)
{
	x %= n;
}
inline void wrap(float &x, float n)
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
inline float RandomFloat(float fLow, float fHigh)
{
	return SCALE( RandomFloat(), 0.0f, 1.0f, fLow, fHigh );
}

// Returns an integer between nLow and nHigh inclusive
inline int RandomInt(int nLow, int nHigh)
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

// Move val toward other_val by to_move.
void fapproach( float& val, float other_val, float to_move );

/* Return a positive x mod y. */
float fmodfp(float x, float y);

int power_of_two(int input);
bool IsAnInt( const CString &s );
bool IsHexVal( const CString &s );
float HHMMSSToSeconds( const CString &sHMS );
CString SecondsToHHMMSS( float fSecs );
CString SecondsToMSSMsMs( float fSecs );
CString SecondsToMMSSMsMs( float fSecs );
CString SecondsToMMSSMsMsMs( float fSecs );
CString PrettyPercent( float fNumerator, float fDenominator );
inline CString PrettyPercent( int fNumerator, int fDenominator ) { return PrettyPercent( float(fNumerator), float(fDenominator) ); }
CString Commify( int iNum );
CString AddNumberSuffix( int i );


struct tm GetLocalTime();

CString ssprintf( const char *fmt, ...) PRINTF(1,2);
CString vssprintf( const char *fmt, va_list argList );

#ifdef WIN32
CString hr_ssprintf( int hr, const char *fmt, ...);
CString werr_ssprintf( int err, const char *fmt, ...);
CString ConvertWstringToACP( wstring s );
CString ConvertUTF8ToACP( CString s );
#endif

// Splits a Path into 4 parts (Directory, Drive, Filename, Extention).  Supports UNC path names.
/* If Path is a directory (eg. c:\games\stepmania"), append a slash so the last
 * element will end up in Dir, not FName: "c:\games\stepmania\". */
void splitpath( const CString &Path, CString &Dir, CString &Filename, CString &Ext );

CString SetExtension( const CString &path, const CString &ext );
CString GetExtension( const CString &sPath );

typedef int longchar;
extern const wchar_t INVALID_CHAR;

int utf8_get_char_len( char p );
bool utf8_to_wchar( const CString &s, unsigned &start, wchar_t &ch );
bool utf8_to_wchar_ec( const CString &s, unsigned &start, wchar_t &ch );
void wchar_to_utf8( wchar_t ch, CString &out );
wchar_t utf8_get_char( const CString &s );
bool utf8_is_valid( const CString &s );
void utf8_remove_bom( CString &s );

CString WStringToCString(const wstring &str);
CString WcharToUTF8( wchar_t c );
wstring CStringToWstring(const CString &str);

// Splits a CString into an CStringArray according the Delimitor.
void split( const CString &Source, const CString &Delimitor, CStringArray& AddIt, const bool bIgnoreEmpty = true );
void split( const wstring &Source, const wstring &Delimitor, vector<wstring> &AddIt, const bool bIgnoreEmpty = true );

/* In-place split. */
void split( const CString &Source, const CString &Delimitor, int &begin, int &size, const bool bIgnoreEmpty = true );
void split( const wstring &Source, const wstring &Delimitor, int &begin, int &size, const bool bIgnoreEmpty = true );

/* In-place split of partial string. */
void split( const CString &Source, const CString &Delimitor, int &begin, int &size, int len, const bool bIgnoreEmpty ); /* no default to avoid ambiguity */
void split( const wstring &Source, const wstring &Delimitor, int &begin, int &size, int len, const bool bIgnoreEmpty );

// Joins a CStringArray to create a CString according the Deliminator.
CString join( const CString &Delimitor, const CStringArray& Source );
CString join( const CString &Delimitor, CStringArray::const_iterator begin, CStringArray::const_iterator end );

CString GetCwd();

void CRC32( unsigned int &iCRC, const void *pBuffer, size_t iSize );
unsigned int GetHashForString( const CString &s );
unsigned int GetHashForFile( const CString &sPath );
unsigned int GetHashForDirectory( const CString &sDir );	// a hash value that remains the same as long as nothing in the directory has changed
bool DirectoryIsEmpty( const CString &dir );

bool CompareCStringsAsc(const CString &str1, const CString &str2);
bool CompareCStringsDesc(const CString &str1, const CString &str2);
void SortCStringArray( CStringArray &AddTo, const bool bSortAscending = true );

/* Find the mean and standard deviation of all numbers in [start,end). */
float calc_mean(const float *start, const float *end);
float calc_stddev(const float *start, const float *end);

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

void TrimLeft(CString &str, const char *s = "\r\n\t ");
void TrimRight(CString &str, const char *s = "\r\n\t ");
void StripCrnl(CString &s);

CString DerefRedir(const CString &path);
CString GetRedirContents(const CString &path);

class Regex {
	void *reg;
	unsigned backrefs;
    CString pattern;
    void Compile();
    void Release();
public:
	Regex(const CString &pat = "");
	Regex(const Regex &rhs);
	Regex &operator=(const Regex &rhs);
	~Regex();
	void Set(const CString &str);
	bool Compare(const CString &str);
	bool Compare(const CString &str, vector<CString> &matches);
};


void Replace_Unicode_Markers( CString &Text );
CString WcharDisplayText(wchar_t c);

CString Basename( const CString &dir );
CString Dirname( const CString &dir );
CString Capitalize( const CString &s );

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
void GetDirListing( const CString &sPath, CStringArray &AddTo, bool bOnlyDirs=false, bool bReturnPathToo=false );
bool DoesFileExist( const CString &sPath );
bool IsAFile( const CString &sPath );
bool IsADirectory( const CString &sPath );
unsigned GetFileSizeInBytes( const CString &sFilePath );
void FlushDirCache();

// call FixSlashes on any path that came from the user
void FixSlashesInPlace( CString &sPath );
CString FixSlashes( CString sPath );
void CollapsePath( CString &sPath, bool bRemoveLeadingDot=false );

bool FromString( const CString &sValue, int &out );
bool FromString( const CString &sValue, unsigned  &out );
bool FromString( const CString &sValue, float &out );
bool FromString( const CString &sValue, bool &out );
inline bool FromString( const CString &sValue, CString &out ) { out = sValue; return true; }

CString ToString( int value );
CString ToString( unsigned value );
CString ToString( float value );
CString ToString( bool value );
inline CString ToString( const CString &value ) { return value; }

// helper file functions used by Bookkeeper and ProfileManager
//
// Helper function for reading/writing scores
//
class RageFile;
bool FileRead(RageFile& f, CString& sOut);
bool FileRead(RageFile& f, int& iOut);
bool FileRead(RageFile& f, unsigned& uOut);
bool FileRead(RageFile& f, float& fOut);
void FileWrite(RageFile& f, const CString& sWrite);
void FileWrite(RageFile& f, int iWrite);
void FileWrite(RageFile& f, size_t uWrite);
void FileWrite(RageFile& f, float fWrite);

bool FileCopy( CString sSrcFile, CString sDstFile );

#endif

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
