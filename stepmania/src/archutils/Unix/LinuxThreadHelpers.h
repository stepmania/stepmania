#ifndef PID_THREAD_HELPERS_H
#define PID_THREAD_HELPERS_H

CString ThreadsVersion();

/* Get the current thread's ThreadID. */
int GetCurrentThreadId();

int SuspendThread( int ThreadID );
int ResumeThread( int ThreadID );

struct BacktraceContext;
int GetThreadContext( int ThreadID, BacktraceContext *ctx );
	
#endif
