#ifndef BACKTRACE_H
#define BACKTRACE_H

/* This API works like backtrace_pointers(), to retrieve a stack trace. */

/* This contains the information necessary to backtrace a thread. */
struct BacktraceContext
{
#if defined(CPU_X86) || defined(CPU_X86_64)
	const void *ip, *bp, *sp;
#endif

#if defined(LINUX)
	pid_t pid;
#endif

#if defined(DARWIN)
	void *FramePtr, *PC;
#endif
};

/* Initialize.  This is optional.  If not called explicitly, it will be
 * called as necessary.  This may do things that are not safe to do in
 * crash conditions. */
void InitializeBacktrace();

/* Retrieve up to size-1 backtrace pointers in buf.  The array will be
 * null-terminated.  If ctx is NULL, retrieve the current backtrace; otherwise
 * retrieve a backtrace for the given context.  (Not all backtracers may
 * support contexts.) */
void GetBacktrace( const void **buf, size_t size, const BacktraceContext *ctx = NULL );

/* Set up a BacktraceContext to get a backtrace for a thread.  ThreadID may
 * not be the current thread.  True is returned on success, false on failure. */
bool GetThreadBacktraceContext( uint64_t ThreadID, BacktraceContext *ctx );

/* Set up a BacktraceContext to get a backtrace after receiving a signal, given
 * a ucontext_t (see sigaction(2)).  (This interface is UNIX-specific.) */
#include <ucontext.h>
void GetSignalBacktraceContext( BacktraceContext *ctx, const ucontext_t *uc );

#define BACKTRACE_METHOD_NOT_AVAILABLE ((void*) -1)

#endif

/*
 * (c) 2003-2004 Glenn Maynard
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
