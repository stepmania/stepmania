#ifndef ARCH_SETUP_DARWIN_H
#define ARCH_SETUP_DARWIN_H

typedef unsigned char      UInt8;
typedef signed char        SInt8;
typedef unsigned short     UInt16;
typedef signed short       SInt16;
typedef unsigned long      UInt32;
typedef signed long        SInt32;
typedef signed long long   SInt64;
typedef unsigned long long UInt64;

#define __MACTYPES__
#include <libkern/OSByteOrder.h>
#undef __MACTYPES__
#define BACKTRACE_LOOKUP_METHOD_DARWIN_DYLD
#define BACKTRACE_METHOD_POWERPC_DARWIN
#define HAVE_VERSION_INFO
#define HAVE_CXA_DEMANGLE
#define HAVE_CRYPTOPP
#define HAVE_THEORA
#define HAVE_FFMPEG
#define HAVE_SDL
#define HAVE_PTHREAD_COND_TIMEDWAIT
#define CRASH_HANDLER
// Looking ahead to "Universal binaries."
#ifdef __BIG_ENDIAN__
# define ENDIAN_BIG
#else
# define ENDIAN_LITTLE
#endif
#define ENDIAN_BIG

#ifndef MACOSX
# define MACOSX
#endif
#ifndef __MACOSX__
# define __MACOSX__
#endif
#define ArchSwap32(n) OSSwapInt32((n))
#define ArchSwap24(n) (OSSwapInt32((n)) >> 8)
#define ArchSwap16(n) OSSwapInt16((n))
#define HAVE_BYTE_SWAPS

#include <bits/c++config.h>
#if ! _GLIBCPP_HAVE_POWF
# define NEED_POWF
#endif

#if ! _GLIBCPP_HAVE_SQRTF
# define NEED_SQRTF
#endif

#if ! _GLIBCPP_HAVE_SINF
# define NEED_SINF
#endif

#if ! _GLIBCPP_HAVE_TANF
# define NEED_TANF
#endif

#if ! _GLIBCPP_HAVE_COSF
# define NEED_COSF
#endif

#if ! _GLIBCPP_HAVE_ACOSF
# define NEED_ACOSF
#endif

#if ! _GLIBCPP_HAVE_STRTOF
# define NEED_STRTOF
#endif

// Define the work around if needed.
#include <stdint.h>
#if _GLIBCPP_USE_C99
# define NEED_CSTDLIB_WORKAROUND
#else
inline int64_t llabs( int64_t x ) { return x < 0LL ? -x : x; }

/* This is not correct. It's possible that _GLIBCPP_USE_C99 could be false
 * and yet have socklen_t defined. In the BSD world, this seems to come from
 * <machine/ansi.h>. However, all apple headers that include this have:
 * #ifndef __APPLE__
 * #include <machine/ansi.h>
 * #endif
 * Since it appears we shouldn't include that for OS X builds, let's just
 * make the assumption that if we can use c99 that socklen_t is defined and
 * if we cannot, then it isn't. For the only two builds (on OS X) we care about,
 * namely supporting 10.2.8 and the current OS, this _should_ hold true.
 */
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
#endif

#if ! _GLIBCPP_USE_WCHAR_T
#include <string>

extern "C"
{
	extern size_t wcslen( const wchar_t *ws );
	extern wchar_t *wmemchr( const wchar_t *ws, wchar_t wc, size_t n );
	extern int wmemcmp( const wchar_t *ws1, const wchar_t *ws2, size_t n );
	extern wchar_t *wmemcpy( wchar_t *ws1, const wchar_t *ws2, size_t n );
	extern wchar_t *wmemmove( wchar_t *ws1, const wchar_t *ws2, size_t n );
	extern wchar_t *wmemset( wchar_t *ws , wchar_t wc, size_t n );
}
// C++ defines the wchar_t type even if glibc++ doesn't have the functions
namespace std
{
	/* This is actually illegal but it should work. =)
	* In addition, it's not complete, but it's good enough for us.
	* http://www.unl.csi.cuny.edu/faqs/g++-faq/wp/sep96/lib-strings.html
	*/
	template<> struct char_traits<wchar_t>
	{
		typedef wchar_t    char_type;
		typedef wchar_t    int_type; // it's big enough
		/*
		 * streampos and wstreampos are both required to be typedefs for
		 * fpos<mbstate_t> as per clause 27.2--whatever that is.
		 */
		typedef streampos  pos_type;
		typedef streamoff  off_type;
		typedef mbstate_t  state_type;
		
		static void assign( char_type& c1, const char_type c2 )
		{
			c1 = c2;
		}
		
		static void assign( char_type *s, size_t n, char_type c )
		{
			wmemset( s, c, n );
		}
		
		static int compare( const char_type *s1, const char_type *s2, size_t n )
		{
			return wmemcmp( s1, s2, n );
		}
		
		static char_type *copy( char_type *s1, const char_type *s2, size_t n )
		{
			return wmemcpy( s1, s2, n );
		}
		
		static const char_type *find( const char_type *s, size_t n, const char_type& c )
		{
			return wmemchr( s, c, n );
		}
		
		static size_t length( const char_type *s )
		{
			return wcslen( s );
		}
		
		static char_type *move( char_type *s1, const char_type *s2, size_t n )
		{
			return wmemmove( s1, s2, n );
		}
	};
	
	typedef basic_string<wchar_t> wstring;
}
#endif

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
