#ifndef RAGEUTIL_H
#define RAGEUTIL_H

/*
-----------------------------------------------------------------------------
 File: RageUtil

 Desc: Miscellaneous helper macros and functions.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include <map>
#include "RageFile.h"

//-----------------------------------------------------------------------------
// SAFE_ Macros
//-----------------------------------------------------------------------------
#define SAFE_DELETE(p)       { if(p) { delete (p);     (p)=NULL; } }
#define SAFE_DELETE_ARRAY(p) { if(p) { delete[] (p);   (p)=NULL; } }
#define SAFE_RELEASE(p)      { if(p) { (p)->Release(); (p)=NULL; } }

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

#define PI		((float)3.1415926535897932384626433832795)
#define DegreeToRadian( degree ) ((degree) * (PI / 180.0f))
#define RadianToDegree( radian ) ((radian) * (180.0f / PI))

// Scales x so that l1 corresponds to l2 and h1 corresponds to h2.  Does not modify x, MUST assign the result to something!
#define SCALE(x, l1, h1, l2, h2)	(((x) - (l1)) / ((h1) - (l1)) * ((h2) - (l2)) + (l2))

#define CLAMP(x, l, h)	{if (x > h) x = h; else if (x < l) x = l;}

inline void wrap( int &x, int n)
{
	if (x<0)
		x += ((-x/n)+1)*n;
	
	x %= n;
}



//-----------------------------------------------------------------------------
// Misc helper functions
//-----------------------------------------------------------------------------

// Fast random number generators
// Taken from "Numerical Recipes in C"

extern unsigned long randseed;

inline unsigned long Random()
{
	randseed = 1664525L * randseed + 1013904223L;
	return randseed;
}

inline float RandomFloat()
{
	randseed = 1664525L * randseed + 1013904223L;
	unsigned long itemp = 0x3f800000 | (0x007fffff & randseed);
	return (*(float *)&itemp) - 1.0f;
}

// Returns a float between dLow and dHigh inclusive
inline float RandomFloat(float fLow, float fHigh)
{
	return RandomFloat() * (fHigh - fLow) + fLow;
}

// Returns an integer between nLow and nHigh inclusive
inline int RandomInt(int nLow, int nHigh)
{
	return ((Random() >> 2) % (nHigh - nLow + 1)) + nLow;
}

/* Alternative: */
class RandomGen
{
	unsigned long seed;

public:
	RandomGen( unsigned long seed = 0 );
	int operator() ( int maximum = INT_MAX-1 );
};


// Simple function for generating random numbers
inline float randomf( const float low=-1.0f, const float high=1.0f )
{
    return low + ( high - low ) * ( (float)rand() ) / RAND_MAX;
}

inline float froundf( const float f, const float fRoundInterval )
{
	return int( (f + fRoundInterval/2)/fRoundInterval ) * fRoundInterval;
}

// Move val toward other_val by to_move.
void fapproach( float& val, float other_val, float to_move );

/* Return a positive x mod y. */
float fmodfp(float x, float y);

int power_of_two(int input);
bool IsAnInt( const CString &s );
bool IsHexVal( const CString &s );
float TimeToSeconds( const CString &sHMS );
CString SecondsToTime( float fSecs );

CString ssprintf( const char *fmt, ...);
CString vssprintf( const char *fmt, va_list argList );

#ifdef WIN32
CString hr_ssprintf( int hr, const char *fmt, ...);
CString werr_ssprintf( int err, const char *fmt, ...);
#endif

// Splits a Path into 4 parts (Directory, Drive, Filename, Extention).  Supports UNC path names.
/* If Path is a directory (eg. c:\games\stepmania"), append a slash so the last
 * element will end up in Dir, not FName: "c:\games\stepmania\". */
void splitpath( const CString &Path, CString &Dir, CString &Filename, CString &Ext );

CString SetExtension( const CString &path, const CString &ext );
CString GetExtension( const CString &sPath );

typedef int longchar;
extern const wchar_t INVALID_CHAR;

CString WStringToCString(const wstring &str);
CString WcharToUTF8( wchar_t c );
wstring CStringToWstring(const CString &str);
wchar_t utf8_get_char (const char *p);
int utf8_get_char_len (const char *p);
bool utf8_is_valid(const CString &str);

// Splits a CString into an CStringArray according the Deliminator.
void split( const CString &Source, const CString &Delimitor, CStringArray& AddIt, const bool bIgnoreEmpty = true );
void split( const wstring &Source, const wstring &Deliminator, vector<wstring> &AddIt, const bool bIgnoreEmpty = true );

// Joins a CStringArray to create a CString according the Deliminator.
CString join( const CString &Delimitor, const CStringArray& Source );
CString join( const CString &Delimitor, CStringArray::const_iterator begin, CStringArray::const_iterator end );

CString GetCwd();

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
void ReplaceText( CString &Text, const map<CString,CString> &m );
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
bool ResolvePath(CString &path);
unsigned GetFileSizeInBytes( const CString &sFilePath );
void FlushDirCache();

// helper file functions used by Bookkeeper and ProfileManager
//
// Helper function for reading/writing scores
//
bool FileRead(RageFile& f, CString& sOut);
bool FileRead(RageFile& f, int& iOut);
bool FileRead(RageFile& f, unsigned& uOut);
bool FileRead(RageFile& f, float& fOut);
void FileWrite(RageFile& f, const CString& sWrite);
void FileWrite(RageFile& f, int iWrite);
void FileWrite(RageFile& f, size_t uWrite);
void FileWrite(RageFile& f, float fWrite);

bool CopyFile2( CString sSrcFile, CString sDstFile );	// using "CopyFile" gives "unresolved external" link error in VC6. Grr. -Chris

#endif
