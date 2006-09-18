#include "global.h"
#include "Threads_Win32.h"
#include "RageUtil.h"
#include "RageThreads.h"
#include "RageTimer.h"
#include "archutils/Win32/ErrorStrings.h"

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

/* SetThreadName magic comes from VirtualDub. */
#define MS_VC_EXCEPTION 0x406d1388

typedef struct tagTHREADNAME_INFO
{
	DWORD dwType;        // must be 0x1000
	LPCSTR szName;       // pointer to name (in same addr space)
	DWORD dwThreadID;    // thread ID (-1 caller thread)
	DWORD dwFlags;       // reserved for future use, most be zero
} THREADNAME_INFO;

static void SetThreadName( DWORD dwThreadID, LPCTSTR szThreadName )
{
	THREADNAME_INFO info;
	info.dwType = 0x1000;
	info.szName = szThreadName;
	info.dwThreadID = dwThreadID;
	info.dwFlags = 0;

	__try {
		RaiseException(MS_VC_EXCEPTION, 0, sizeof(info) / sizeof(DWORD), (ULONG_PTR *)&info);
	} __except (EXCEPTION_CONTINUE_EXECUTION) {
	}
}

static DWORD WINAPI StartThread( LPVOID pData )
{
	ThreadImpl_Win32 *pThis = (ThreadImpl_Win32 *) pData;

	SetThreadName( GetCurrentThreadId(), RageThread::GetCurThreadName() );

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

	SetThreadName( GetCurrentThreadId(), RageThread::GetCurThreadName() );

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

	thread->ThreadHandle = CreateThread( NULL, 0, &StartThread, thread, CREATE_SUSPENDED, &thread->ThreadId );
	*piThreadID = (uint64_t) thread->ThreadId;
	ASSERT_M( thread->ThreadHandle, ssprintf("%s", werr_ssprintf(GetLastError(), "CreateThread")) );

	int slot = GetOpenSlot( thread->ThreadId );
	g_ThreadHandles[slot] = thread->ThreadHandle;

	int iRet = ResumeThread( thread->ThreadHandle );
	ASSERT_M( iRet == 1, ssprintf("%s", werr_ssprintf(GetLastError(), "ResumeThread")) );

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

EventImpl_Win32::EventImpl_Win32( MutexImpl_Win32 *pParent )
{
	m_pParent = pParent;
	m_iNumWaiting = 0;
	m_WakeupSema = CreateSemaphore( NULL, 0, 0x7fffffff, NULL );
	InitializeCriticalSection( &m_iNumWaitingLock );
	m_WaitersDone = CreateEvent( NULL, FALSE, FALSE, NULL );
}

EventImpl_Win32::~EventImpl_Win32()
{
	ASSERT_M( m_iNumWaiting == 0, ssprintf("event destroyed while still in use (%i)", m_iNumWaiting) );

	/* We don't own m_pParent; don't free it. */
	CloseHandle( m_WakeupSema );
	DeleteCriticalSection( &m_iNumWaitingLock );
	CloseHandle( m_WaitersDone );
}

/* SignalObjectAndWait is atomic, which leads to more fair event handling.  However,
 * we don't guarantee or depend upon fair events, and SignalObjectAndWait is only
 * available in NT.  I also can't find a single function to signal an object like
 * SignalObjectAndWait, so we need to know if the object is a mutex or an event. */
static bool PortableSignalObjectAndWait( HANDLE hObjectToSignal, HANDLE hObjectToWaitOn, bool bFirstParamIsMutex, unsigned iMilliseconds = INFINITE )
{
	static bool bSignalObjectAndWaitUnavailable = false;
	/* Watch out: SignalObjectAndWait doesn't work when iMilliseconds is zero. */
	if( !bSignalObjectAndWaitUnavailable && iMilliseconds != 0 )
	{
		DWORD ret = SignalObjectAndWait( hObjectToSignal, hObjectToWaitOn, iMilliseconds, false );
		switch( ret )
		{
		case WAIT_OBJECT_0:
			return true;

		case WAIT_ABANDONED:
			/* The docs aren't particular about what this does, but it should never happen. */
			FAIL_M( "WAIT_ABANDONED" );

		case 1: /* bogus Win98 return value */
		case WAIT_FAILED:
			if( GetLastError() == ERROR_CALL_NOT_IMPLEMENTED )
			{
				/* We're probably on 9x. */
				bSignalObjectAndWaitUnavailable = true;
				break;
			}

			FAIL_M( werr_ssprintf(GetLastError(), "SignalObjectAndWait") );

		case WAIT_TIMEOUT:
			return false;

		default:
			FAIL_M( ssprintf("Unexpected code from SignalObjectAndWait: %d",ret ));
		}
	}

	if( bFirstParamIsMutex )
	{
		const bool bRet = !!ReleaseMutex( hObjectToSignal );
		if( !bRet )
			sm_crash( werr_ssprintf( GetLastError(), "ReleaseMutex failed" ) );
	}
	else
		SetEvent( hObjectToSignal );

	DWORD ret = WaitForSingleObject( hObjectToWaitOn, iMilliseconds );
	switch( ret )
	{
	case WAIT_OBJECT_0:
		return true;

	case WAIT_ABANDONED:
		/* The docs aren't particular about what this does, but it should never happen. */
		FAIL_M( "WAIT_ABANDONED" );

	case WAIT_TIMEOUT:
		return false;

	default:
		FAIL_M( "unknown" );
	}
}

/* Event logic from http://www.cs.wustl.edu/~schmidt/win32-cv-1.html.
 * pTimeout is not currently implemented. */
bool EventImpl_Win32::Wait( RageTimer *pTimeout )
{
	EnterCriticalSection( &m_iNumWaitingLock );
	++m_iNumWaiting;
	LeaveCriticalSection( &m_iNumWaitingLock );

	unsigned iMilliseconds = INFINITE;
	if( pTimeout != NULL )
	{
		float fSecondsInFuture = -pTimeout->Ago();
		iMilliseconds = (unsigned) max( 0, int( fSecondsInFuture * 1000 ) );
	}

	/* Unlock the mutex and wait for a signal. */
	bool bSuccess = PortableSignalObjectAndWait( m_pParent->mutex, m_WakeupSema, true, iMilliseconds );

	EnterCriticalSection( &m_iNumWaitingLock );
	if( !bSuccess )
	{
		/* Avoid a race condition: someone may have signalled the object between PortableSignalObjectAndWait
		 * and EnterCriticalSection.  While we hold m_iNumWaitingLock, poll (with a zero timeout) the
		 * object one last time. */
		if( WaitForSingleObject( m_WakeupSema, 0 ) == WAIT_OBJECT_0 )
			bSuccess = true;
	}
	--m_iNumWaiting;
	bool bLastWaiting = m_iNumWaiting == 0;
	LeaveCriticalSection( &m_iNumWaitingLock );

	/* If we're the last waiter to wake up, and we were actually woken by another
	 * thread (not by timeout), wake up the signaller. */
	if( bLastWaiting && bSuccess )
		PortableSignalObjectAndWait( m_WaitersDone, m_pParent->mutex, false );
	else
		WaitForSingleObject( m_pParent->mutex, INFINITE );

	return bSuccess;
}

void EventImpl_Win32::Signal()
{
	EnterCriticalSection( &m_iNumWaitingLock );
	bool bHaveWaiters = (m_iNumWaiting > 0);
	LeaveCriticalSection( &m_iNumWaitingLock );

	if( bHaveWaiters )
	{
		ReleaseSemaphore( m_WakeupSema, 1, 0 );

		/* The waiter will touch m_WaitersDone. */
		WaitForSingleObject( m_WaitersDone, INFINITE );
	}
}

void EventImpl_Win32::Broadcast()
{
	EnterCriticalSection( &m_iNumWaitingLock );

	if( m_iNumWaiting == 0 )
	{
		LeaveCriticalSection( &m_iNumWaitingLock );
		return;
	}

	ReleaseSemaphore( m_WakeupSema, m_iNumWaiting, 0 );

	LeaveCriticalSection( &m_iNumWaitingLock );

	/* The last waiter will touch m_WaitersDone, so we wait for all waiters to wake up and
	 * start waiting for the mutex before returning. */
	WaitForSingleObject( m_WaitersDone, INFINITE );
}

EventImpl *MakeEvent( MutexImpl *pMutex )
{
	MutexImpl_Win32 *pWin32Mutex = (MutexImpl_Win32 *) pMutex;

	return new EventImpl_Win32( pWin32Mutex );
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
