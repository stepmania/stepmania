#include "global.h"

#if defined(_WINDOWS)
#define _WIN32_WINDOWS 0x0410 // include Win98 stuff
#include "windows.h"
#include "archutils/Win32/Crash.h"
void NORETURN sm_crash( const char *reason )
{
	/* If we're being debugged, throw a debug break so it'll suspend the process. */
	if( IsDebuggerPresent() )
		DebugBreak();
	else
		ForceCrashHandler( reason );

	/* Do something after the above, so the call/return isn't optimized to a jmp; that
	 * way, this function will appear in backtrace stack traces. */
	_asm nop;
}
#elif defined(LINUX) || defined(DARWIN)
#include "archutils/Unix/CrashHandler.h"
#include <unistd.h>
void NORETURN sm_crash( const char *reason )
{
	ForceCrashHandler( reason );
	_exit( 1 );
}
#else
void NORETURN sm_crash( const char *reason )
{
	*(char*)0=0;

	/* This isn't actually reached.  We just do this to convince the compiler that the
	 * function really doesn't return. */
	while(1);
}
#endif 
