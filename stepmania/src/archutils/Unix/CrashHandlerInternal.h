#ifndef CRASH_HANDLER_INTERNAL_H
#define CRASH_HANDLER_INTERNAL_H

#include "Backtrace.h"
#define BACKTRACE_MAX_SIZE 128

struct CrashData
{
	enum CrashType
	{
		/* We received a fatal signal.  si and uc are valid. */
		SIGNAL,

		/* We're forcing a crash (eg. failed ASSERT). */
		FORCE_CRASH,
	} type;

	/* Everything except FORCE_CRASH_THIS_THREAD: */
	enum { MAX_BACKTRACE_THREADS = 32 };
	const void *BacktracePointers[MAX_BACKTRACE_THREADS][BACKTRACE_MAX_SIZE];
	char m_ThreadName[MAX_BACKTRACE_THREADS][128];

	/* SIGNAL only: */
	int signal;
	siginfo_t si;
	
	/* FORCE_CRASH_THIS_THREAD and FORCE_CRASH_DEADLOCK only: */
	char reason[256];
};

#define CHILD_MAGIC_PARAMETER "--private-do-crash-handler"

const char *SignalName( int signo );

#endif
