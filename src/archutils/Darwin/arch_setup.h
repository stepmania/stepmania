#ifndef ARCH_SETUP_DARWIN_H
#define ARCH_SETUP_DARWIN_H

// Replace the main function.
extern "C" int SM_main( int argc, char *argv[] );
#define main(x,y) SM_main(x,y)

#define HAVE_VERSION_INFO
#define HAVE_CXA_DEMANGLE
#define HAVE_FFMPEG
#define HAVE_PTHREAD_COND_TIMEDWAIT
/* This must be defined to 1 because autoconf's AC_CHECK_DECLS macro decides to define
 * this in all cases. If only they could be consistent... */
#define HAVE_DECL_SIGUSR1 1

/* We have <machine/endian.h> which gets pulled in when we use gcc 4.0's <cstdlib>
 * but no <endian.h>. The definitions of LITTLE_ENDIAN, BIG_ENDIAN and end up conflicting
 * even though they resolve to the same thing (bug in gcc?). */
#define HAVE_MACHINE_ENDIAN_H
#define HAVE_INTTYPES_H
#define __STDC_FORMAT_MACROS
#define CRASH_HANDLER

#define GL_GET_ERROR_IS_SLOW
// CGFlushDrawable() performs a glFlush() and the docs say not to call glFlush()
#define NO_GL_FLUSH

#if defined(__ppc__)
# define CPU_PPC
# define ENDIAN_BIG
# define BACKTRACE_LOOKUP_METHOD_DARWIN_DYLD
# define BACKTRACE_METHOD_POWERPC_DARWIN
#elif defined(__i386__)
# define CPU_X86
# define ENDIAN_LITTLE
# define BACKTRACE_METHOD_X86_DARWIN
# define BACKTRACE_LOOKUP_METHOD_DLADDR
#endif

#ifndef MACOSX
# define MACOSX
#endif
#ifndef __MACOSX__
# define __MACOSX__
#endif

#include <libkern/OSByteOrder.h>
#define ArchSwap32(n) OSSwapInt32((n))
#define ArchSwap24(n) (ArchSwap32((n)) >> 8)
#define ArchSwap16(n) OSSwapInt16((n))
#define HAVE_BYTE_SWAPS

// Define the work around if needed.
#include <bits/c++config.h>
#include <stdint.h>
#if _GLIBCXX_USE_C99
# define NEED_CSTDLIB_WORKAROUND
#else
inline int64_t llabs( int64_t x ) { return x < 0LL ? -x : x; }
#endif

#define attribute_deprecated // Shut ffmpeg up!

#endif

/*
 * (c) 2003-2006 Steve Checkoway
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
