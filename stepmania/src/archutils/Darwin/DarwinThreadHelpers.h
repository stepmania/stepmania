#ifndef DARWIN_THREAD_HELPERS_H
#define DARWIN_THREAD_HELPERS_H

bool SuspendThread(uint64_t threadHandle);
bool ResumeThread(uint64_t threadHandle);
uint64_t GetCurrentThreadId();

#endif
