/*
-----------------------------------------------------------------------------
 File: RageUtil

 Desc: Miscellaneous helper functions.

 Copyright (c) 2001-2002 by the names listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#ifndef _RAGEUTIL_H_
#define _RAGEUTIL_H_



//-----------------------------------------------------------------------------
// SAFE_ Macros
//-----------------------------------------------------------------------------
#define SAFE_DELETE(p)       { if(p) { delete (p);     (p)=NULL; } }
#define SAFE_DELETE_ARRAY(p) { if(p) { delete[] (p);   (p)=NULL; } }
#define SAFE_RELEASE(p)      { if(p) { (p)->Release(); (p)=NULL; } }


//-----------------------------------------------------------------------------
// Other Macros
//-----------------------------------------------------------------------------
#define RECTWIDTH(rect)   ((rect).right  - (rect).left)
#define RECTHEIGHT(rect)  ((rect).bottom - (rect).top)

#ifndef max
#define max(a,b)            (((a) > (b)) ? (a) : (b))
#endif

#ifndef min
#define min(a,b)            (((a) < (b)) ? (a) : (b))
#endif

#define clamp(val,low,high)		( max( (low), min((val),(high)) ) )


//-----------------------------------------------------------------------------
// Misc helper functions
//-----------------------------------------------------------------------------

// Simple function for generating random numbers
inline float randomf( const float low=-1.0f, const float high=1.0f )
{
    return low + ( high - low ) * ( (FLOAT)rand() ) / RAND_MAX;
}
inline int roundf( const float f )	{ return (int)((f)+0.5f); };
inline int roundf( const double f )	{ return (int)((f)+0.5);  };


bool IsAnInt( LPCTSTR s );


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

// Splits a CString into an CStringArray according the Deliminator.  Supports UNC path names.
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

void GetDirListing( CString sPath, CStringArray &AddTo, bool bOnlyDirs=false );

bool DoesFileExist( const CString &sPath );
DWORD GetFileSizeInBytes( const CString &sFilePath );

int CompareCStringsAsc(const void *arg1, const void *arg2);
int CompareCStringsDesc(const void *arg1, const void *arg2);
void SortCStringArray( CStringArray &AddTo, const bool bSortAcsending = true );


LONG GetRegKey(HKEY key, LPCTSTR subkey, LPTSTR retdata);
HINSTANCE GotoURL(LPCTSTR url);


#endif
