#ifndef CRASH_HANDLER_H
#define CRASH_HANDLER_H

void CrashSignalHandler(int signal);
void CrashHandlerHandleArgs( int argc, char* argv[] );
void InitializeCrashHandler();
	
#endif

