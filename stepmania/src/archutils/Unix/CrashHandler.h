#ifndef CRASH_HANDLER_H
#define CRASH_HANDLER_H

void ForceCrashHandler( const char *reason );
struct BacktraceContext;
void ForceCrashHandlerDeadlock( const char *reason, const BacktraceContext *ctx );
void CrashHandlerHandleArgs( int argc, char* argv[] );
void InitializeCrashHandler();
	
#include <csignal>
#include <ucontext.h>
void CrashSignalHandler( int signal, siginfo_t *si, const ucontext_t *uc );

#if defined(DARWIN)
#include <MachineExceptions.h>
OSStatus CrashExceptionHandler( ExceptionInformation *e );
#endif

#endif

