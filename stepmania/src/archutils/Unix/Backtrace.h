#ifndef BACKTRACE_H
#define BACKTRACE_H

/* This API works like backtrace_pointers(), to retrieve a stack trace. */

/* This contains the information necessary to backtrace a thread. */
struct BacktraceContext
{
#if defined(LINUX)
	const void *eip, *ebp, *esp;
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
bool GetThreadBacktraceContext( int ThreadID, BacktraceContext *ctx );

/* Set up a BacktraceContext to get a backtrace after receiving a signal, given
 * a ucontext_t (see sigaction(2)).  (This interface is UNIX-specific.) */
#include <ucontext.h>
void GetSignalBacktraceContext( BacktraceContext *ctx, const ucontext_t *uc );

#if defined(DARWIN)
#include <MachineExceptions.h>

/* Set up a BacktraceContext to get a backtrace after receiving an exception, given
 * an ExceptionInformation*. */
void GetExceptionBacktraceContext( BacktraceContext *ctx, const ExceptionInformation *exception );
#endif

#define BACKTRACE_METHOD_NOT_AVAILABLE ((void*) -1)

#endif

