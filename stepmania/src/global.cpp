#include "global.h"

#ifdef _WINDOWS
#include "windows.h"
void NORETURN sm_crash()
{
	// throws an exception that gets caught by the exception handler
	DebugBreak();

	/* Do something after calling DebugBreak, so the call/return isn't optimized
	 * to a jmp; that way, this function will appear in backtrace stack traces. */
	_asm nop;
}
#else
void NORETURN sm_crash()
{
	*(char*)0=0;

	/* This isn't actually reached.  We just do this to convince the compiler that the
	 * function really doesn't return. */
	while(1);
}
#endif 
