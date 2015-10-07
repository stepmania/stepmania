#ifndef ARCH_SETUP_WINDOWS_H
#define ARCH_SETUP_WINDOWS_H

#if defined(_MSC_VER)
#define HAVE_FFMPEG
#endif

#define SUPPORT_OPENGL
#if defined(_MSC_VER)
#define SUPPORT_D3D
#endif

#if defined(__MINGW32__)
#if !defined(_WINDOWS)
#define _WINDOWS // This isn't defined under all versions of MinGW
#endif
#define NEED_CSTDLIB_WORKAROUND // Needed for llabs() in MinGW
#endif

#if defined(_MSC_VER)

#if _MSC_VER == 1400 // VC8 specific warnings
#pragma warning (disable : 4005) // macro redefinitions (ARRAYSIZE)
#endif

/*
The following warnings are disabled in all builds.
Enable them in the project file at your peril (or if you feel like
learning of many pedantic style warnings and want to fix them).

C2475: non dll-interface class 'stdext::exception' used as base for dll-interface class 'std::bad_cast', bug in VC <exception> when exceptions disabled
C4100: unreferenced formal parameter
  Many functions are like this, including virtual functions: unsure if it can be justified.
  "case 'aaa' is not a valid value for switch of enum 'bbb'
  Actually, this is a valid warning, but we do it all over the
  place, eg. with ScreenMessages. Those should be fixed, but later. XXX
C4127: conditional expression is constant.

C4201: nonstandard extension used : nameless struct/union (Windows headers do this)
C4786: turn off broken debugger warning
C4512: assignment operator could not be generated (so?)
 "unreachable code". This warning crops up in incorrect places (end of do ... while(0)
 blocks, try/catch blocks), and I've never found it to be useful.
C4702: assignment operator could not be generated (so?)
// "unreferenced formal parameter"; we *want* that in many cases

C4063:
C4786: VC6: identifier was truncated to '255' characters in the debug information
C4505: removed unferenced local function from integer.cpp & algebra.h
C4244: converting of data = possible data loss.  (This pragma should eventually go away)
C4355: 'this' : used in base member initializer list

*/
// Fix VC breakage.
#define PATH_MAX _MAX_PATH

// Disable false deprecation warnings in VC2005.
#define _CRT_SECURE_NO_DEPRECATE
#define _SCL_SECURE_NO_DEPRECATE

// Disable false deprecation warnings in VC2008.
#define _CRT_NONSTDC_NO_WARNINGS

// Don't include windows.h everywhere; when we do eventually include it, use these:
#define WIN32_LEAN_AND_MEAN
#define VC_EXTRALEAN

/* Pull in NT-only definitions. Note that we support Win98 and WinME; you can
 * make NT calls, but be sure to fall back on 9x if they're not supported. */
#define _WIN32_WINNT 0x0601
#define _WIN32_IE 0x0400

// If this isn't defined to 0, VC fails to define things like stat and alloca.
#define __STDC__ 0

#endif

#include <wchar.h> // needs to be included before our fixes below

#define lstat stat
#define fsync _commit
#define isnan _isnan
#define isfinite _finite

typedef time_t time_t;
struct tm;
struct tm *my_localtime_r( const time_t *timep, struct tm *result );
#define localtime_r my_localtime_r
struct tm *my_gmtime_r( const time_t *timep, struct tm *result );
#define gmtime_r my_gmtime_r
#if defined(_MSC_VER)
void my_usleep( unsigned long usec );
#define usleep my_usleep
#endif

// Missing stdint types:
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
#if defined(_MSC_VER)
#if _MSC_VER < 1700	// 1700 = VC++ 2011
#define INT64_C(i) i##i64
#ifndef UINT64_C
#define UINT64_C(i) i##ui64
#endif
#endif // #if _MSC_VER < 1700
#if (_MSC_VER >= 1400) && (_MSC_VER < 1800) // 1800 = VC++ 2013
#define llabs(i) _abs64(i)
#endif // #if (_MSC_VER >= 1400) && (_MSC_VER < 1800)
#if _MSC_VER < 1400 // 1400 = VC++ 2005
int64_t llabs( int64_t i ) { return i >= 0 ? i : -i; }
#endif // #if _MSC_VER < 1400
#endif // #if defined(_MSC_VER)
#endif // #if !defined(__MINGW32__)

#undef min
#undef max
#define NOMINMAX // make sure Windows doesn't try to define this

/* We implement the crash handler interface (though that interface isn't
 * completely uniform across platforms yet). */
#if !defined(SMPACKAGE)
#define CRASH_HANDLER
#endif

#if defined(__GNUC__) // It might be MinGW or Cygwin(?)
#include "archutils/Common/gcc_byte_swaps.h"
#elif defined(_MSC_VER) && (_MSC_VER >= 1310) // Byte swap functions were first implemented in Visual Studio .NET 2003
#define ArchSwap32(n) _byteswap_ulong(n)
#define ArchSwap24(n) _byteswap_ulong(n) >> 8
#define ArchSwap16(n) _byteswap_ushort(n)
#else
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
