/* RageThread - Thread, mutex, semaphore and event classes. */

#ifndef RAGE_THREADS_H
#define RAGE_THREADS_H

struct ThreadSlot;
class RageTimer;
class RageThread
{
public:
	RageThread();
	~RageThread();

	void SetName( const CString &n ) { name = n; }
	CString GetName() const { return name; }
	void Create( int (*fn)(void *), void *data );

	/* For crash handlers: kill or suspend all threads (except for
	 * the running one) immediately. */ 
	static void HaltAllThreads( bool Kill=false );

	/* If HaltAllThreads was called (with Kill==false), resume. */
	static void ResumeAllThreads();

	static uint64_t GetCurrentThreadID();

	static const char *GetCurThreadName();
	static const char *GetThreadNameByID( uint64_t iID );
	static bool EnumThreadIDs( int n, uint64_t &iID );
	int Wait();
	bool IsCreated() const { return m_pSlot != NULL; }

	/* A system can define HAVE_TLS, indicating that it can compile thread_local
	 * code, but an individual environment may not actually have functional TLS.
	 * If this returns false, thread_local variables are considered undefined. */
	static bool GetSupportsTLS() { return s_bSystemSupportsTLS; }
	static void SetSupportsTLS( bool b ) { s_bSystemSupportsTLS = b; }

	static bool GetIsShowingDialog() { return s_bIsShowingDialog; }
	static void SetIsShowingDialog( bool b ) { s_bIsShowingDialog = b; }

private:
	ThreadSlot *m_pSlot;
	CString name;

	static bool s_bSystemSupportsTLS;
	static bool s_bIsShowingDialog;
};

/* Register a thread created outside of RageThread.  This gives it a name for RageThread::GetCurThreadName,
 * and allocates a slot for checkpoints. */
class RageThreadRegister
{
public:
	RageThreadRegister( const CString &sName );
	~RageThreadRegister();

private:
	ThreadSlot *m_pSlot;
};

namespace Checkpoints
{
	void LogCheckpoints( bool yes=true );
	void SetCheckpoint( const char *file, int line, const char *message );
	void GetLogs( char *pBuf, int iSize, const char *delim );
};

#define CHECKPOINT (Checkpoints::SetCheckpoint(__FILE__, __LINE__, NULL))
#define CHECKPOINT_M(m) (Checkpoints::SetCheckpoint(__FILE__, __LINE__, m))

/* Mutex class that follows the behavior of Windows mutexes: if the same
 * thread locks the same mutex twice, we just increase a refcount; a mutex
 * is considered unlocked when the refcount reaches zero.  This is more
 * convenient, though much slower on some archs.  (We don't have any tightly-
 * coupled threads, so that's OK.) */
class MutexImpl;
class RageMutex
{
public:
	CString GetName() const { return m_sName; }
	void SetName( const CString &s ) { m_sName = s; }
	virtual void Lock();
	virtual bool TryLock();
	virtual void Unlock();
	virtual bool IsLockedByThisThread() const;

	RageMutex( CString name );
	virtual ~RageMutex();

protected:
	MutexImpl *m_pMutex;
	CString m_sName;

	int m_UniqueID;
	
	uint64_t m_LockedBy;
	int m_LockCnt;

	void MarkLockedMutex();
};

/* Lock a mutex on construction, unlock it on destruction.  Helps for functions
 * with more than one return path. */
class LockMutex
{
	RageMutex &mutex;

	const char *file;
	int line;
	float locked_at;
	bool locked;

public:
	LockMutex(RageMutex &mut, const char *file, int line);
	LockMutex(RageMutex &mut): mutex(mut), file(NULL), line(-1), locked_at(-1), locked(true) { mutex.Lock(); }
	~LockMutex();
	LockMutex(LockMutex &cpy): mutex(cpy.mutex), file(NULL), line(-1), locked_at(cpy.locked_at), locked(true) { mutex.Lock(); }

	/* Unlock the mutex (before this would normally go out of scope).  This can
	 * only be called once. */
	void Unlock();
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

class EventImpl;
class RageEvent: public RageMutex
{
public:
	RageEvent( CString name );
	~RageEvent();

	/*
	 * If pTimeout is non-NULL, the event will be automatically signalled at the given
	 * time.  Note that implementing this timeout is optional; not all archs support it. 
	 * If false is returned, the wait timed out (and the mutex is locked, as if the
	 * event had been signalled).
	 */
	bool Wait( RageTimer *pTimeout = NULL );
	void Signal();
	void Broadcast();

private:
	EventImpl *m_pEvent;
};

class SemaImpl;
class RageSemaphore
{
public:
	RageSemaphore( CString sName, int iInitialValue = 0 );
	~RageSemaphore();

	CString GetName() const { return m_sName; }
	int GetValue() const;
	void Post();
	void Wait( bool bFailOnTimeout=true );
	bool TryWait();

private:
	SemaImpl *m_pSema;
	CString m_sName;
};

#endif

/*
 * Copyright (c) 2001-2004 Glenn Maynard
 * All rights reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, and/or sell copies of the Software, and to permit persons to
 * whom the Software is furnished to do so, provided that the above
 * copyright notice(s) and this permission notice appear in all copies of
 * the Software and that both the above copyright notice(s) and this
 * permission notice appear in supporting documentation.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT OF
 * THIRD PARTY RIGHTS. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR HOLDERS
 * INCLUDED IN THIS NOTICE BE LIABLE FOR ANY CLAIM, OR ANY SPECIAL INDIRECT
 * OR CONSEQUENTIAL DAMAGES, OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS
 * OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
 * OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */
