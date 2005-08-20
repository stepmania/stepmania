#ifndef ARCH_SETUP_DARWIN_H
#define ARCH_SETUP_DARWIN_H

typedef unsigned char 	   UInt8;
typedef unsigned short 	   UInt16;
typedef unsigned long 	   UInt32;
typedef unsigned long long UInt64;
#define __TYPES__
#include <libkern/OSByteOrder.h>
#undef __TYPES__
#include <AvailabilityMacros.h>
#if (MAC_OS_X_VERSION_MAX_ALLOWED < MAC_OS_X_VERSION_10_3)
# define NEED_POWF
# define NEED_SQRTF
# define NEED_SINF
# define NEED_COSF
# define NEED_TANF
# define NEED_ACOSF
# define NEED_STRTOF
# include <cstddef>
# include <string>
# include <cstdlib>
namespace std
{
	typedef basic_string<wchar_t> wstring;
}
typedef int socklen_t;
/* cmath #undefs all of the math.h macros but fails to replace some of them
* with functions if _GLIBCPP_USE_C99 is not defined. As such, yank the
* definition out of math.h.
*/
# include <cmath>
# define isnan( x ) ( ( sizeof ( x ) == sizeof(double) ) ?           \
					  __isnand ( x ) :                               \
					  ( sizeof ( x ) == sizeof( float) ) ?           \
					  __isnanf ( x ) :                               \
					  __isnan  ( x ) )
// llabs is not defined either, copy it
inline long long llabs(long long __x) { return __x >= 0 ? __x : -__x; }
#else
# define NEED_CSTDLIB_WORKAROUND
#endif
#define BACKTRACE_LOOKUP_METHOD_DARWIN_DYLD
#define BACKTRACE_METHOD_POWERPC_DARWIN
#define HAVE_VERSION_INFO
#define HAVE_CXA_DEMANGLE
#define HAVE_FFMPEG
#define HAVE_SDL
#define CRASH_HANDLER
#define ENDIAN_BIG
#define CRYPTOPP_UNIX_AVAILABLE
#ifndef __MACOSX__
# define __MACOSX__
#endif
#define ArchSwap32(n) OSSwapInt32((n))
#define ArchSwap24(n) (OSSwapInt32((n)) >> 8)
#define ArchSwap16(n) OSSwapInt16((n))
#define HAVE_BYTE_SWAPS
#endif

/*
 * (c) 2003-2005 Steve Checkoway
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
