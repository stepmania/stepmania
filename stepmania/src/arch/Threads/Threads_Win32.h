#ifndef THREADS_WIN32_H
#define THREADS_WIN32_H

#include "Threads.h"
#include <windows.h>

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
	uint64_t GetCrashHandle() const;
	int Wait();
};

struct MutexImpl_Win32: public MutexImpl
{
	uint64_t LockedBy;
	volatile int LockCnt;

	HANDLE mutex;

	MutexImpl_Win32( RageMutex *parent );
	~MutexImpl_Win32();

	bool Lock();
	void Unlock();
	uint64_t GetLockedByThreadId() const;
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
