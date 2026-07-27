#ifndef ARCH_SETUP_DARWIN_H
#define ARCH_SETUP_DARWIN_H

// Replace the main function.
extern "C" int sm_main( int argc, char *argv[] );

#define HAVE_CXA_DEMANGLE
/* This must be defined to 1 because autoconf's AC_CHECK_DECLS macro decides to define
 * this in all cases. If only they could be consistent... */
#define HAVE_DECL_SIGUSR1 1

#define __STDC_FORMAT_MACROS
#define CRASH_HANDLER

#define GL_GET_ERROR_IS_SLOW
// CGFlushDrawable() performs a glFlush() and the docs say not to call glFlush()
#define NO_GL_FLUSH

#define CPU_X86
#ifndef BACKTRACE_METHOD_X86_DARWIN
#define BACKTRACE_METHOD_X86_DARWIN
#endif
#define BACKTRACE_LOOKUP_METHOD_DLADDR

#ifndef MACOSX
# define MACOSX
#endif
#ifndef __MACOSX__
# define __MACOSX__
#endif

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
