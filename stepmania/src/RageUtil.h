/*
-----------------------------------------------------------------------------
 File: RageUtil.h

 Desc: Helper and error-controlling function used throughout the program.

 Copyright (c) 2001 Chris Danford.  All rights reserved.
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

#define clamp(val,low,high)		( min( (val), max((low),(high)) ) )


//-----------------------------------------------------------------------------
// Misc helper functions
//-----------------------------------------------------------------------------

// Simple function for generating random numbers
float randomf( float low=-1.0f, float high=1.0f );
int roundf( FLOAT f );
int roundf( double f );

bool IsAnInt( CString s );

CString ssprintf( LPCTSTR fmt, ...);
CString vssprintf( LPCTSTR fmt, va_list argList );


/*
 @FUNCTION: Splits a Path into 4 parts (Directory, Drive, Filename, Extention).
      NOTE: Supports UNC path names.
   @PARAM1: Whether the Supplied Path (PARAM2) contains a directory name only
            or a file name (Reason: some directories will end with "xxx.xxx"
            which is like a file name).
   @PARAM2: Path to Split.
   @PARAM3: (Referenced) Directory.
   @PARAM4: (Referenced) Drive.
   @PARAM5: (Referenced) Filename.
   @PARAM6: (Referenced) Extention.
*/
void splitpath(BOOL UsingDirsOnly, CString Path, CString& Drive, CString& Dir, CString& FName, CString& Ext);


void splitrelpath( CString Path, CString& Dir, CString& FName, CString& Ext );


/*
 @FUNCTION: Splits a CString into an CStringArray according the Deliminator.
	  NOTE: Supports UNC path names.
   @PARAM1: Source string to be Split.
   @PARAM2: Deliminator.
   @PARAM3: (Referenced) CStringArray to Add to.
*/
void split(CString Source, CString Deliminator, CStringArray& AddIt, bool bIgnoreEmpty = true );

/*
 @FUNCTION: Joins a CStringArray to create a CString according the Deliminator.
   @PARAM1: Deliminator.
   @PARAM2: (Referenced) CStringArray to Add to.
*/
CString join(CString Deliminator, CStringArray& Source);

void GetDirListing( CString sPath, CStringArray &AddTo, BOOL bOnlyDirs=FALSE );

bool DoesFileExist( CString sPath );
DWORD GetFileSizeInBytes( CString sFilePath );

int CompareCStrings(const void *arg1, const void *arg2);
void SortCStringArray( CStringArray &AddTo, BOOL bSortAcsending = TRUE );

//-----------------------------------------------------------------------------
// Log helpers
//-----------------------------------------------------------------------------
void RageLogStart();
void RageLog( LPCTSTR fmt, ...);

//-----------------------------------------------------------------------------
// Error helpers
//-----------------------------------------------------------------------------
#include "dxerr8.h"
#pragma comment(lib, "DxErr8.lib")


VOID DisplayErrorAndDie( CString sError );


//#if defined(DEBUG) | defined(_DEBUG)
    #define RageError(str)		DisplayErrorAndDie( ssprintf(     "%s\n\n%s(%d)", str,						 __FILE__, (DWORD)__LINE__, str) )
    #define RageErrorHr(str,hr)	DisplayErrorAndDie( ssprintf("%s (%s)\n\n%s(%d)", str, DXGetErrorString8(hr), __FILE__, (DWORD)__LINE__, str) )
//#else
//    #define RageError(str)		(0L)
//    #define RageErrorHr(str,hr)	(hr)
//#endif


LONG GetRegKey(HKEY key, LPCTSTR subkey, LPTSTR retdata);
HINSTANCE GotoURL(LPCTSTR url);


#endif
