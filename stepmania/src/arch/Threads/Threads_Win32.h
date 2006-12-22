#ifndef THREADS_WIN32_H
#define THREADS_WIN32_H

#include "Threads.h"
#if defined(_WINDOWS)
#  include <windows.h>
#else if
#  include <windef.h>
#endif

class ThreadImpl_Win32: public ThreadImpl
{
public:
	HANDLE ThreadHandle;
	DWORD ThreadId;

	int (*m_pFunc)( void *pData );
	void *m_pData;

	void Halt( bool Kill );
	void Resume();
	uint64_t GetThreadId() const;
	int Wait();
};

HANDLE Win32ThreadIdToHandle( uint64_t iID );

class MutexImpl_Win32: public MutexImpl
{
	friend class EventImpl_Win32;
public:
	MutexImpl_Win32( RageMutex *parent );
	~MutexImpl_Win32();

	bool Lock();
	bool TryLock();
	void Unlock();

private:
	HANDLE mutex;
};

class EventImpl_Win32: public EventImpl
{
public:
	EventImpl_Win32( MutexImpl_Win32 *pParent );
	~EventImpl_Win32();

	bool Wait( RageTimer *pTimeout );
	void Signal();
	void Broadcast();
	bool WaitTimeoutSupported() const { return true; }

private:
	MutexImpl_Win32 *m_pParent;

	int m_iNumWaiting;
	CRITICAL_SECTION m_iNumWaitingLock;
	HANDLE m_WakeupSema;
	HANDLE m_WaitersDone;
};

class SemaImpl_Win32: public SemaImpl
{
public:
	SemaImpl_Win32( int iInitialValue );
	~SemaImpl_Win32();
	int GetValue() const { return m_iCounter; }
	void Post();
	bool Wait();
	bool TryWait();

private:
	HANDLE sem;

	/* We have to track the count ourself, since Windows gives no way to query it. */
	int m_iCounter;
};

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
