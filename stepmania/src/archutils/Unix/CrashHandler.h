#ifndef CRASH_HANDLER_H
#define CRASH_HANDLER_H

#include <ucontext.h>

void CrashSignalHandler( int signal, siginfo_t *si, const ucontext_t *uc );
void CrashHandlerHandleArgs( int argc, char* argv[] );
void InitializeCrashHandler();
	
#endif

