#include "global.h"

#if defined(_WINDOWS)
#include "windows.h"
void NORETURN sm_crash( const char *reason )
{
	// throws an exception that gets caught by the exception handler
	DebugBreak();

	/* Do something after calling DebugBreak, so the call/return isn't optimized
	 * to a jmp; that way, this function will appear in backtrace stack traces. */
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
