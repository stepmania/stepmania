#ifndef BACKTRACE_H
#define BACKTRACE_H

/* This API works like backtrace_pointers(), to retrieve a stack trace. */

/* This contains the information necessary to backtrace a thread. */
struct BacktraceContext
{
#if defined(LINUX)
	long eip, esp, ebp;
	pid_t pid;
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
void GetBacktrace( const void **buf, size_t size, BacktraceContext *ctx = NULL );

/* Set up a BacktraceContext to get a backtrace for a thread.  ThreadID may
 * be the current thread. */
int GetThreadBacktraceContext( int ThreadID, BacktraceContext *ctx );
void GetCurrentBacktraceContext( BacktraceContext *ctx );

#define BACKTRACE_METHOD_NOT_AVAILABLE ((void*) -1)
#define BACKTRACE_SIGNAL_TRAMPOLINE ((void*) -2)

#endif

