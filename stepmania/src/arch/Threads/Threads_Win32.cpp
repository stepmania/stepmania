#include "global.h"
#include "Threads_Win32.h"
#include "RageUtil.h"
#include "RageThreads.h"

const int MAX_THREADS=128;

static MutexImpl_Win32 *g_pThreadIdMutex = NULL;
static void InitThreadIdMutex()
{
	if( g_pThreadIdMutex != NULL )
		return;
	g_pThreadIdMutex = new MutexImpl_Win32(NULL);
}

static uint64_t g_ThreadIds[MAX_THREADS];
static HANDLE g_ThreadHandles[MAX_THREADS];

HANDLE Win32ThreadIdToHandle( uint64_t iID )
{
	for( int i = 0; i < MAX_THREADS; ++i )
	{
		if( g_ThreadIds[i] == iID )
			return g_ThreadHandles[i];
	}

	return NULL;
}

void ThreadImpl_Win32::Halt( bool Kill )
{
#ifndef _XBOX
	if( Kill )
		TerminateThread( ThreadHandle, 0 );
	else
#endif
		SuspendThread( ThreadHandle );
}

void ThreadImpl_Win32::Resume()
{
	ResumeThread( ThreadHandle );
}

uint64_t ThreadImpl_Win32::GetThreadId() const
{
	return (uint64_t) ThreadId;
}

int ThreadImpl_Win32::Wait()
{
	WaitForSingleObject( ThreadHandle, INFINITE );

	DWORD ret;
	GetExitCodeThread( ThreadHandle, &ret );

	CloseHandle( ThreadHandle );
	ThreadHandle = NULL;

	return ret;
}

static DWORD WINAPI StartThread( LPVOID pData )
{
	ThreadImpl_Win32 *pThis = (ThreadImpl_Win32 *) pData;

	DWORD ret = (DWORD) pThis->m_pFunc( pThis->m_pData );

	for( int i = 0; i < MAX_THREADS; ++i )
	{
		if( g_ThreadIds[i] == RageThread::GetCurrentThreadID() )
		{
			g_ThreadHandles[i] = NULL;
			g_ThreadIds[i] = 0;
			break;
		}
	}

	return ret;
}

static int GetOpenSlot( uint64_t iID )
{
	InitThreadIdMutex();

	g_pThreadIdMutex->Lock();

	/* Find an open slot in g_ThreadIds. */
	int slot = 0;
	while( slot < MAX_THREADS && g_ThreadIds[slot] != 0 )
		++slot;
	ASSERT( slot < MAX_THREADS );

	g_ThreadIds[slot] = iID;

	g_pThreadIdMutex->Unlock();

	return slot;
}

ThreadImpl *MakeThisThread()
{
	ThreadImpl_Win32 *thread = new ThreadImpl_Win32;

	const HANDLE CurProc = GetCurrentProcess();
	int ret = DuplicateHandle( CurProc, GetCurrentThread(), CurProc, 
		&thread->ThreadHandle, 0, false, DUPLICATE_SAME_ACCESS );

	if( !ret )
	{
//		LOG->Warn( werr_ssprintf( GetLastError(), "DuplicateHandle(%p, %p) failed",
//			CurProc, GetCurrentThread() ) );

		thread->ThreadHandle = NULL;
	}

	thread->ThreadId = GetCurrentThreadId();

	int slot = GetOpenSlot( GetCurrentThreadId() );
	g_ThreadHandles[slot] = thread->ThreadHandle;

	return thread;
}

ThreadImpl *MakeThread( int (*pFunc)(void *pData), void *pData, uint64_t *piThreadID )
{
	ThreadImpl_Win32 *thread = new ThreadImpl_Win32;
	thread->m_pFunc = pFunc;
	thread->m_pData = pData;

	thread->ThreadHandle = CreateThread( NULL, 0, &StartThread, thread, 0, &thread->ThreadId );
	*piThreadID = (uint64_t) thread->ThreadId;

	ASSERT_M( thread->ThreadHandle, ssprintf("%s", werr_ssprintf(GetLastError(), "CreateThread")) );

	int slot = GetOpenSlot( thread->ThreadId );
	g_ThreadHandles[slot] = thread->ThreadHandle;

	return thread;
}



MutexImpl_Win32::MutexImpl_Win32( RageMutex *pParent ):
	MutexImpl( pParent )
{
	mutex = CreateMutex( NULL, false, NULL );
	ASSERT_M( mutex != NULL, werr_ssprintf(GetLastError(), "CreateMutex") );
}

MutexImpl_Win32::~MutexImpl_Win32()
{
	CloseHandle( mutex );
}

static bool SimpleWaitForSingleObject( HANDLE h, DWORD ms )
{
	ASSERT( h != NULL );

	DWORD ret = WaitForSingleObject( h, ms );
	switch( ret )
	{
	case WAIT_OBJECT_0:
		return true;

	case WAIT_TIMEOUT:
		return false;

	case WAIT_ABANDONED:
		/* The docs aren't particular about what this does, but it should never happen. */
		FAIL_M( "WAIT_ABANDONED" );

	case WAIT_FAILED:
		FAIL_M( werr_ssprintf(GetLastError(), "WaitForSingleObject") );

	default:
		FAIL_M( "unknown" );
	}
}

bool MutexImpl_Win32::Lock()
{
	int len = 15000;
	int tries = 5;

	while( tries-- )
	{
		/* Wait for fifteen seconds.  If it takes longer than that, we're probably deadlocked. */
		if( SimpleWaitForSingleObject( mutex, len ) )
			return true;

		/* Timed out; probably deadlocked.  Try a couple more times, with a smaller
		 * timeout, just in case we're debugging and happened to stop while waiting
		 * on the mutex. */
		len = 1000;
	}

	return false;
}


bool MutexImpl_Win32::TryLock()
{
	return SimpleWaitForSingleObject( mutex, 0 );
}

void MutexImpl_Win32::Unlock()
{
	const bool ret = !!ReleaseMutex( mutex );

	/* We can't ASSERT here, since this is called from checkpoints, which is
	 * called from ASSERT. */
	if( !ret )
		sm_crash( werr_ssprintf( GetLastError(), "ReleaseMutex failed" ) );
}

uint64_t GetThisThreadId()
{
	return GetCurrentThreadId();
}

uint64_t GetInvalidThreadId()
{
	return 0;
}

MutexImpl *MakeMutex( RageMutex *pParent )
{
	return new MutexImpl_Win32( pParent );
}

SemaImpl_Win32::SemaImpl_Win32( int iInitialValue )
{
	sem = CreateSemaphore( NULL, iInitialValue, 999999999, NULL );
	m_iCounter = iInitialValue;
}

SemaImpl_Win32::~SemaImpl_Win32()
{
	CloseHandle( sem );
}

void SemaImpl_Win32::Post()
{
	++m_iCounter;
	ReleaseSemaphore( sem, 1, NULL );
}

bool SemaImpl_Win32::Wait()
{
	int len = 15000; 
	int tries = 5;

	while( tries-- )
	{
		/* Wait for 15 seconds.  If it takes longer than that, we're 
		 * probably deadlocked. */
		if( SimpleWaitForSingleObject( sem, len ) )
		{
			--m_iCounter;
			return true;
		}

		/* Timed out; probably deadlocked.  Try again a few more times, with a smaller
		 * timeout, just in case we're debugging and happened to stop while waiting
		 * on the mutex. */
		len = 1000;
	}

	return false;
}

bool SemaImpl_Win32::TryWait()
{
	if( !SimpleWaitForSingleObject( sem, 0 ) )
		return false;

	--m_iCounter;
	return true;
}

SemaImpl *MakeSemaphore( int iInitialValue )
{
	return new SemaImpl_Win32( iInitialValue );
}

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
