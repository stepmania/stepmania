#ifndef THREADS_H
#define THREADS_H

/* This is the low-level implementation; you probably want RageThreads. */
class RageMutex;


class ThreadImpl
{
public:
	virtual ~ThreadImpl() { }
	virtual void Halt( bool Kill ) = 0;
	virtual void Resume() = 0;

	/* Get the identifier for this thread.  The actual meaning of this is implementation-
	 * defined, except that each thread has exactly one ID and each ID corresponds to
	 * one thread.  (This means that Win32 thread handles are not acceptable as ThreadIds.) */
	virtual uint64_t GetThreadId() const = 0;

	virtual int Wait() = 0;
};

class MutexImpl
{
public:
	RageMutex *m_Parent;

	MutexImpl( RageMutex *pParent ) { m_Parent = pParent; }
	virtual ~MutexImpl() { }

	/* Lock the mutex.  If mutex timeouts are implemented, and the mutex times out,
	 * return false and do not lock the mutex.  No other failure return is allowed;
	 * all other errors should fail with an assertion. */
	virtual bool Lock() = 0;

	/* Non-blocking lock.  If locking the mutex would block because the mutex is already
	 * locked by another thread, return false; otherwise return true and lock the mutex. */
	virtual bool TryLock() = 0;

	/* Unlock the mutex.  This must only be called when the mutex is locked; implementations
	 * may fail with an assertion if the mutex is not locked. */
	virtual void Unlock() = 0;
};

class SemaImpl
{
public:
	virtual ~SemaImpl() { }
	virtual int GetValue() const = 0;
	virtual void Post() = 0;
	virtual bool Wait() = 0;
	virtual bool TryWait() = 0;
};

/* These functions must be implemented by the thread implementation. */
ThreadImpl *MakeThread( int (*fn)(void *), void *data, uint64_t *piThreadID );
ThreadImpl *MakeThisThread();
MutexImpl *MakeMutex( RageMutex *pParent );
SemaImpl *MakeSemaphore( int iInitialValue );
uint64_t GetThisThreadId();

/* Since ThreadId is implementation-defined, we can't define a universal invalid
 * value.  Return the invalid value for this implementation. */
uint64_t GetInvalidThreadId();

#endif

/*
 * (c) 2001-2004 Glenn Maynard
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
