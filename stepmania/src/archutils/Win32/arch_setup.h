#ifndef ARCH_SETUP_WINDOWS_H
#define ARCH_SETUP_WINDOWS_H

#if !defined(XBOX)
#define HAVE_FFMPEG
#define HAVE_CRYPTOPP
#endif

#if defined(__MINGW32__)
#define _WINDOWS // This isn't defined under MinGW
#define NEED_CSTDLIB_WORKAROUND // Needed for llabs() in MinGW
#endif

#if defined(_MSC_VER)

/* Fix VC breakage. */
#define PATH_MAX _MAX_PATH

#if _MSC_VER < 1300 /* VC6, not VC7 */
template<class T>
inline const T& max(const T &a, const T &b)			{ return a < b? b:a; }
template<class T, class P>
inline const T& max(const T &a, const T &b, P Pr)	{ return Pr(a, b)? b:a; }
template<class T>
inline const T& min(const T &a, const T &b)			{ return b < a? b:a; }
template<class T, class P>
inline const T& min(const T &a, const T &b, P Pr)	{ return Pr(b, a)? b:a; }
#endif

// HACK: Fake correct scoping rules in VC6.
#if _MSC_VER == 1200 
#define for if(0); else for
#endif

/* Don't include windows.h everywhere; when we do eventually include it, use these: */
#define WIN32_LEAN_AND_MEAN
#define VC_EXTRALEAN

/* Pull in NT-only definitions.  Note that we support Win98 and WinME; you can make
 * NT calls, but be sure to fall back on 9x if they're not supported. */
#define _WIN32_WINNT 0x0400

/* If this isn't defined to 0, VC fails to define things like stat and alloca. */
#define __STDC__ 0

#endif

#include <direct.h> /* has stuff that should be in unistd.h */
#include <wchar.h> /* needs to be included before our fixes below */

#define lstat stat
#define fsync _commit
#define isnan _isnan
#define finite _finite

/* mkdir is missing the mode arg */
#define mkdir(p,m) mkdir(p)

typedef time_t time_t;
struct tm;
struct tm *my_localtime_r( const time_t *timep, struct tm *result );
#define localtime_r my_localtime_r
struct tm *my_gmtime_r( const time_t *timep, struct tm *result );
#define gmtime_r my_gmtime_r
void my_usleep( unsigned long usec );
#define usleep my_usleep



/* Missing stdint types: */
#if !defined(__MINGW32__) // MinGW headers define these for us
typedef signed char int8_t;
typedef signed short int16_t;
typedef int int32_t;
typedef __int64 int64_t;
typedef unsigned char uint8_t;
typedef signed short int16_t;
typedef unsigned short uint16_t;
typedef int int32_t;
typedef unsigned int uint32_t;
typedef __int64 int64_t;
typedef unsigned __int64 uint64_t;
static inline int64_t llabs( int64_t i ) { return i >= 0? i: -i; }

#if _MSC_VER < 1300 /* VC6 */
typedef unsigned int uintptr_t;
#endif

#endif

#if defined(_MSC_VER)
#pragma warning (disable : 4201) // nonstandard extension used : nameless struct/union (Windows headers do this)
#pragma warning (disable : 4786) // turn off broken debugger warning
#pragma warning (disable : 4512) // assignment operator could not be generated (so?)
/* "unreachable code".  This warning crops up in incorrect places (end of do ... while(0)
 * blocks, try/catch blocks), and I've never found it to be useful. */
#pragma warning (disable : 4702) // assignment operator could not be generated (so?)
/* "unreferenced formal parameter"; we *want* that in many cases */
#pragma warning (disable : 4100)
/* "case 'aaa' is not a valid value for switch of enum 'bbb'
 * Actually, this is a valid warning, but we do it all over the
 * place, eg. with ScreenMessages.  Those should be fixed, but later. XXX */
#pragma warning (disable : 4063)
#pragma warning (disable : 4127)
#pragma warning (disable : 4786) /* VC6: identifier was truncated to '255' characters in the debug information */
#endif

#undef min
#undef max
#define NOMINMAX /* make sure Windows doesn't try to define this */

/* Windows is missing some basic math functions: */
// But MinGW isn't.
#if !defined(__MINGW32__)
#define NEED_TRUNCF
#define NEED_ROUNDF
#define NEED_STRTOF
#define MISSING_STDINT_H
#endif

// MinGW provides us with this function already
#if !defined(__MINGW32__)
inline int lrintf( float f )
{
	int retval;
	
	_asm fld f;
	_asm fistp retval;

	return retval;
}
#endif

/* For RageLog. */
#define HAVE_VERSION_INFO

/* We implement the crash handler interface (though that interface isn't completely
 * uniform across platforms yet). */
#if !defined(_XBOX)
#define CRASH_HANDLER
#endif

#define ENDIAN_LITTLE

#if defined(_XBOX)
#if defined(_DEBUG)
#define XIPH_LIB_DIR "vorbis/xbox/debug/"
#else
#define XIPH_LIB_DIR "vorbis/xbox/release/"
#endif
#else
#define XIPH_LIB_DIR "vorbis/win32/"
#endif

#if defined(XBOX)
#include "ArchUtils/Xbox/arch_setup.h"
#endif


#if defined(__GNUC__) // It might be MinGW or Cygwin(?)
#include "archutils/Common/gcc_byte_swaps.h"
#else // XXX: Should we test for MSVC?
#define HAVE_BYTE_SWAPS

inline uint32_t ArchSwap32( uint32_t n )
{
	__asm
	{
		mov eax, n
		xchg al, ah
		ror eax, 16
		xchg al, ah
		mov n, eax
	};
	return n;
}

inline uint32_t ArchSwap24( uint32_t n )
{
	__asm
	{
		mov eax, n
		xchg al, ah
		ror eax, 16
		xchg al, ah
		ror eax, 8
		mov n, eax
	};
	return n;
}

inline uint16_t ArchSwap16( uint16_t n )
{
	__asm
	{
		mov ax, n
		xchg al, ah
		mov n, ax
	};
	return n;
}
#endif

#endif

/*
 * (c) 2002-2004 Glenn Maynard
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
