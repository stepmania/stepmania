#ifndef CRASH_HANDLER_INTERNAL_H
#define CRASH_HANDLER_INTERNAL_H

#include "Backtrace.h"

struct CrashData
{
	enum CrashType
	{
		/* We received a fatal signal.  si and uc are valid. */
		SIGNAL,

#if defined(DARWIN)
		/* We received a fatal exception. */
		OSX_EXCEPTION,
#endif

		/* We're forcing a crash (eg. failed ASSERT). */
		FORCE_CRASH_THIS_THREAD,

		/* Deadlock detected; give a stack trace for two threads. */
		// FORCE_CRASH_DEADLOCK
	} type;

	BacktraceContext ctx;

	/* FORCE_CRASH_DEADLOCK only: */
	// BacktraceContext ctx2;

	/* SIGNAL only: */
	int signal;
	siginfo_t si;

	/* OSX_EXCEPTION only: */
	int kind;
	
	/* FORCE_CRASH_THIS_THREAD only: */
	char reason[256];
};

#define BACKTRACE_MAX_SIZE 1024
#define CHILD_MAGIC_PARAMETER "--private-do-crash-handler"

const char *SignalName( int signo );

#if defined(DARWIN)
const char *ExceptionName( int signo );
#endif

#endif

