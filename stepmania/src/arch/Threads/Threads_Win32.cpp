#include "global.h"
#include "Threads_Win32.h"
#include "RageUtil.h"
#include "RageThreads.h"

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

uint64_t ThreadImpl_Win32::GetCrashHandle() const
{
	return (uint64_t) ThreadHandle;
}

int ThreadImpl_Win32::Wait()
{
	WaitForSingleObject( ThreadHandle, INFINITE );

	DWORD ret;
	GetExitCodeThread( ThreadHandle, &ret );

	return ret;
}

static DWORD WINAPI StartThread( LPVOID pData )
{
	ThreadImpl_Win32 *pThis = (ThreadImpl_Win32 *) pData;

	return (DWORD)pThis->m_pFunc( pThis->m_pData );
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

	return thread;
}

ThreadImpl *MakeThread( int (*pFunc)(void *pData), void *pData )
{
	ThreadImpl_Win32 *thread = new ThreadImpl_Win32;
	thread->m_pFunc = pFunc;
	thread->m_pData = pData;

	/* XXX: does ThreadHandle need to be dup'd? */
	thread->ThreadHandle = CreateThread( NULL, 0, &StartThread, thread, 0, &thread->ThreadId );

	ASSERT_M( thread->ThreadHandle, ssprintf("%s", werr_ssprintf(GetLastError(), "CreateThread")) );

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

bool MutexImpl_Win32::Lock()
{
	int len = 15000;
	int tries = 2;

	while( tries-- )
	{
		/* Wait for fifteen seconds.  If it takes longer than that, we're probably deadlocked. */
		DWORD ret = WaitForSingleObject( mutex, len );

		switch( ret )
		{
		case WAIT_ABANDONED:
			/* The docs aren't particular about what this does, but it should never happen. */
			ASSERT( 0 );
			break;

		case WAIT_OBJECT_0:
			return true;

		case WAIT_TIMEOUT:
			/* Timed out.  Probably deadlocked.  Try again one more time, with a smaller
			 * timeout, just in case we're debugging and happened to stop while waiting
			 * on the mutex. */
			len = 1000;
			break;

		case WAIT_FAILED:
			FAIL_M( werr_ssprintf(GetLastError(), "WaitForSingleObject(%s)", this->m_Parent->GetName().c_str()) );
		}
	}

	return false;
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
