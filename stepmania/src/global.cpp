#include "global.h"

#if defined(_WINDOWS)
#define _WIN32_WINDOWS 0x0410 // include Win98 stuff
#include "windows.h"
#include "archutils/Win32/Crash.h"
#endif

#if defined(CRASH_HANDLER) && (defined(LINUX) || defined(DARWIN))
#include "archutils/Unix/CrashHandler.h"
#include <unistd.h>
#endif

void NORETURN sm_crash( const char *reason )
{
#if defined(_WINDOWS)
	/* If we're being debugged, throw a debug break so it'll suspend the process. */
	if( IsDebuggerPresent() )
	{
		DebugBreak();
		while(1); /* don't return */
	}
#endif

#if defined(CRASH_HANDLER)
	ForceCrashHandler( reason );
#else
	*(char*)0=0;

	/* This isn't actually reached.  We just do this to convince the compiler that the
	 * function really doesn't return. */
	while(1);
#endif

#if defined(_WINDOWS)
	/* Do something after the above, so the call/return isn't optimized to a jmp; that
	 * way, this function will appear in backtrace stack traces. */
	_asm nop;
#else
	_exit( 1 );
#endif
}

