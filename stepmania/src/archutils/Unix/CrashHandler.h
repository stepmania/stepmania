#ifndef CRASH_HANDLER_H
#define CRASH_HANDLER_H

void ForceCrashHandler( const char *reason );
void CrashHandlerHandleArgs( int argc, char* argv[] );
void InitializeCrashHandler();
	
#if !defined(DARWIN)
#include <signal.h>
#include <ucontext.h>
void CrashSignalHandler( int signal, siginfo_t *si, const ucontext_t *uc );
#endif

#if defined(DARWIN)
#include <MachineExceptions.h>
OSStatus CrashExceptionHandler( ExceptionInformation *e );
#endif

#endif

