#ifndef RAGE_THREADS_H
#define RAGE_THREADS_H

#include "SDL_Thread.h"

/* Mutex class that follows the behavior of Windows mutexes: if the same
 * thread locks the same mutex twice, we just increase a refcount; a mutex
 * is considered unlocked when the refcount reaches zero.  This is more
 * convenient, though much slower on some archs.  (We don't have any tightly-
 * coupled threads, so that's OK.) */
class RageMutex
{
	Uint32 LockedBy;
	int Locked;
	SDL_mutex *mut, *mutwait;

public:
	void Lock();
	void Unlock();
	RageMutex();
	~RageMutex();
};

/* Lock a mutex on construction, unlock it on destruction.  Helps for functions
 * with more than one return path. */
class LockMutex
{
	RageMutex &mutex;

public:
	LockMutex(RageMutex &mut): mutex(mut) { mutex.Lock(); }
	~LockMutex() { mutex.Unlock(); }
	LockMutex(LockMutex &cpy): mutex(cpy.mutex) { mutex.Lock(); }
};

#endif
