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

	const char *file;
	int line;
	float locked_at;

public:
	LockMutex(RageMutex &mut, const char *file, int line);
	LockMutex(RageMutex &mut): mutex(mut), file(NULL), line(-1), locked_at(0) { mutex.Lock(); }
	~LockMutex();
	LockMutex(LockMutex &cpy): mutex(cpy.mutex), file(cpy.file), line(cpy.line), locked_at(cpy.locked_at) { mutex.Lock(); }
};

/* Double-abstracting __LINE__ lets us append it to other text, to generate
 * locally unique variable names.  (Otherwise we get "LocalLock__LINE__".) I'm
 * not sure why this works, but it does, in both VC and GCC. */

#if 0
#ifdef DEBUG
/* Use the debug version, which logs if something holds a lock for a long time.
 * __FUNCTION__ is nonstandard, but both GCC and VC support it; VC doesn't
 * support the standard, __func__. */
#define LockMutL2(m, l) LockMutex LocalLock ## l (m, __FUNCTION__, __LINE__)
#else
#define LockMutL2(m, l) LockMutex LocalLock ## l (m)
#endif

#define LockMutL(m, l) LockMutL2(m, l)
#define LockMut(m) LockMutL(m, __LINE__)
#endif

/* Gar.  It works in VC7, but not VC6, so for now this can only be used once
 * per function.  If you need more than that, declare LockMutexes yourself. 
 * Also, VC6 doesn't support __FUNCTION__. */
#if _MSC_VER < 1300 /* VC6, not VC7 */
#define LockMut(m) LockMutex LocalLock(m, __FILE__, __LINE__)
#else
#define LockMut(m) LockMutex LocalLock(m, __FUNCTION__, __LINE__)
#endif

#endif
