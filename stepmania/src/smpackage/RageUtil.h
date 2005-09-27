#pragma once 
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
#define ARRAYSIZE(a) (sizeof(a) / sizeof((a)[0]))

#define RECTWIDTH(rect)   ((rect).right  - (rect).left)
#define RECTHEIGHT(rect)  ((rect).bottom - (rect).top)
inline int RECTCENTERX(RECT rect) { return rect.left + (rect.right-rect.left)/2; }
inline int RECTCENTERY(RECT rect) { return rect.top + (rect.bottom-rect.top)/2; }

#ifndef max
#define max(a,b)            (((a) > (b)) ? (a) : (b))
#endif

#ifndef min
#define min(a,b)            (((a) < (b)) ? (a) : (b))
#endif

#ifndef MAX
#define MAX(a,b)            (((a) > (b)) ? (a) : (b))
#endif

#ifndef MIN
#define MIN(a,b)            (((a) < (b)) ? (a) : (b))
#endif

#define clamp(val,low,high)		( max( (low), min((val),(high)) ) )

#define PI		D3DX_PI
#define DEG		(PI / 180.0f)
#define RAD		(180.0f / PI)
// Scales x so that l1 corresponds to l2 and h1 corresponds to h2.  Does not modify x, MUST assign the result to something!
#define SCALE(x, l1, h1, l2, h2)	(((x) - (l1)) / ((h1) - (l1)) * ((h2) - (l2)) + (l2))
// Clamps x
#define CLAMP(x, l, h)	{if (x > h) x = h; else if (x < l) x = l;}


//-----------------------------------------------------------------------------
// Misc helper functions
//-----------------------------------------------------------------------------

// Fast random number generators
// Taken from "Numerical Recipes in C"

extern ULONG randseed;

inline ULONG Random()
{
	randseed = 1664525L * randseed + 1013904223L;
	return randseed;
}

inline float RandomFloat()
{
	randseed = 1664525L * randseed + 1013904223L;
	ULONG itemp = 0x3f800000 | (0x007fffff & randseed);
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
    return low + ( high - low ) * ( (FLOAT)rand() ) / RAND_MAX;
}
inline int roundf( const float f )	{ return (int)((f)+0.5f); };
inline int roundf( const double f )	{ return (int)((f)+0.5);  };
inline float froundf( const float f, const float fRoundInterval )
{
	return int( (f + fRoundInterval/2)/fRoundInterval ) * fRoundInterval;
}

bool IsAnInt( LPCTSTR s );
float TimeToSeconds( CString sHMS );
CString SecondsToTime( float fSecs );

CString ssprintf( LPCTSTR fmt, ...);
CString vssprintf( LPCTSTR fmt, va_list argList );


// Splits a Path into 4 parts (Directory, Drive, Filename, Extention).  Supports UNC path names.
// param1: Whether the Supplied Path (PARAM2) contains a directory name only
//            or a file name (Reason: some directories will end with "xxx.xxx"
//            which is like a file name).
void splitpath( 
	const bool UsingDirsOnly, 
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

void GetDirListing( CString sPath, CStringArray &AddTo, bool bOnlyDirs=false, bool bReturnPathToo=false );
int GetHashForString( CString s );
int GetHashForFile( CString sPath );
int GetHashForDirectory( CString sDir );	// a hash value that remains the same as long as nothing in the directory has changed

bool DoesFileExist( const CString &sPath );
bool IsAFile( const CString &sPath );
bool IsADirectory( const CString &sPath );
DWORD GetFileSizeInBytes( const CString &sFilePath );

void SortCStringArray( CStringArray &AddTo, const bool bSortAcsending = true );
void StripCrnl(CString &s);


LONG GetRegKey(HKEY key, LPCTSTR subkey, LPTSTR retdata);
HINSTANCE GotoURL(LPCTSTR url);

void WriteStringToFile( FILE* file, CString s );
void ReadStringFromFile( FILE* file, CString& s );
void WriteIntToFile( FILE* file, int i );
void ReadIntFromFile( FILE* file, int& i );
void WriteFloatToFile( FILE* file, float f );
void ReadFloatFromFile( FILE* file, float& f );
void WriteUlongToFile( FILE* file, ULONG u );
void ReadUlongFromFile( FILE* file, ULONG& u );

