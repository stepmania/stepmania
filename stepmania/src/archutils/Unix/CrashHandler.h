#ifndef CRASH_HANDLER_H
#define CRASH_HANDLER_H

#include <signal.h>

void ForceCrashHandler( const char *reason );
void CrashSignalHandler( int signal, siginfo_t *si, const ucontext_t *uc );
void CrashHandlerHandleArgs( int argc, char* argv[] );
void InitializeCrashHandler();
	
#if defined(DARWIN)
OSStatus CrashExceptionHandler( ExceptionInformation *e );
#endif

#endif

