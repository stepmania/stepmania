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


//-----------------------------------------------------------------------------
// SAFE_ Macros
//-----------------------------------------------------------------------------
#define SAFE_DELETE(p)       { if(p) { delete (p);     (p)=NULL; } }
#define SAFE_DELETE_ARRAY(p) { if(p) { delete[] (p);   (p)=NULL; } }
#define SAFE_RELEASE(p)      { if(p) { (p)->Release(); (p)=NULL; } }

#define ZERO(x)					memset(&x, 0, sizeof(x))
#define COPY(a,b)				{ ASSERT(sizeof(a)==sizeof(b)); memcpy(&a, &b, sizeof(a)); }

/* Common harmless mismatches. */
#ifndef min
inline float min(float a, int b) { return a < b? a:b; }
inline float min(int a, float b) { return a < b? a:b; }
#endif

#ifndef max
inline float max(float a, int b) { return a > b? a:b; }
inline float max(int a, float b) { return a > b? a:b; }
#endif

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
// Clamps x
#define CLAMP(x, l, h)	{if (x > h) x = h; else if (x < l) x = l;}


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

// Debug new for memory leak tracing
//#ifdef _DEBUG
//#define new new(_NORMAL_BLOCK, __FILE__, __LINE__)
//#endif


// Simple function for generating random numbers
inline float randomf( const float low=-1.0f, const float high=1.0f )
{
    return low + ( high - low ) * ( (float)rand() ) / RAND_MAX;
}

/* XXX: These are C99 functions (except for the roundf(double) overload); what's
 * the C99 define we can test for? */
inline double trunc( double f )	{ return float(int(f)); };
inline float truncf( float f )	{ return float(int(f)); };
inline float roundf( float f )	{ if(f < 0) return truncf(f-0.5f); return truncf(f+0.5f); };
inline double roundf( double f ){ if(f < 0) return trunc(f-0.5); return trunc(f+0.5);  };
inline float froundf( const float f, const float fRoundInterval )
{
	return int( (f + fRoundInterval/2)/fRoundInterval ) * fRoundInterval;
}

bool IsAnInt( const char *s );
float TimeToSeconds( CString sHMS );
CString SecondsToTime( float fSecs );

CString ssprintf( const char *fmt, ...);
CString vssprintf( const char *fmt, va_list argList );

#ifdef WIN32
CString hr_ssprintf( int hr, const char *fmt, ...);
#endif

// Splits a Path into 4 parts (Directory, Drive, Filename, Extention).  Supports UNC path names.
// param1: Whether the Supplied Path (PARAM2) contains a directory name only
//            or a file name (Reason: some directories will end with "aaa.bbb"
//            which is like a file name).
// We should just make sure all pathnames end with a slash, not special case it here.
// -glenn
void splitpath( 
	bool UsingDirsOnly, 
	const CString &Path, 
	CString &Drive, 
	CString &Dir, 
	CString &FName, 
	CString &Ext
);

void splitrelpath( 
	const CString &Path, 
	CString& Dir, 
	CString& FName, 
	CString& Ext 
);

// Splits a CString into an CStringArray according the Deliminator.
void split(
	const CString &Source, 
	const CString &Deliminator, 
	CStringArray& AddIt, 
	const bool bIgnoreEmpty = true 
);

// Joins a CStringArray to create a CString according the Deliminator.
CString join(
	const CString &Deliminator,
	const CStringArray& Source
);

bool CreateDirectories( CString Path );
void GetDirListing( CString sPath, CStringArray &AddTo, bool bOnlyDirs=false, bool bReturnPathToo=false );

bool GetExtDirListing( const CString &sPath, CStringArray &AddTo, bool bOnlyDirs, bool bReturnPathToo, ... );
bool GetExtDirListingV( const CString &sPath, CStringArray &AddTo, bool bOnlyDirs, bool bReturnPathToo, const char *masks[] );
void FlushDirCache();

unsigned int GetHashForString( CString s );
unsigned int GetHashForFile( CString sPath );
unsigned int GetHashForDirectory( CString sDir );	// a hash value that remains the same as long as nothing in the directory has changed

bool DoesFileExist( const CString &sPath );
bool IsAFile( const CString &sPath );
bool IsADirectory( const CString &sPath );
unsigned GetFileSizeInBytes( const CString &sFilePath );

bool CompareCStringsAsc(const CString &str1, const CString &str2);
bool CompareCStringsDesc(const CString &str1, const CString &str2);
void SortCStringArray( CStringArray &AddTo, const bool bSortAcsending = true );

/* Find the mean and standard deviation of all numbers in [start,end). */
float calc_mean(const float *start, const float *end);
float calc_stddev(const float *start, const float *end);

void TrimLeft(CString &str, const char *s = "\r\n\t ");
void TrimRight(CString &str, const char *s = "\r\n\t ");

/* Fix Windows breakage ... */
#ifdef WIN32
#include <direct.h> /* has stuff that should be in unistd.h */
#define getcwd _getcwd
#define wgetcwd _wgetcwd
#define chdir _chdir
#define wchdir _wchdir
#define alloca _alloca
#define stat _stat
#else
#include <unistd.h> /* correct place with correct definitions */
#endif

#endif
