#include "global.h"
#include "Threads_Pthreads.h"
#include <sys/time.h>

#if defined(LINUX)
#define PID_BASED_THREADS
#include "archutils/Unix/LinuxThreadHelpers.h"
#endif

#if defined(HAVE_PTHREAD_MUTEX_TIMEDLOCK) && defined(CRASH_HANDLER)
#include "archutils/Unix/Backtrace.h"
#include "archutils/Unix/CrashHandler.h"
#endif

#if defined(DARWIN)
#include <mach/mach_init.h>
#include <mach/thread_act.h>
#endif

void ThreadImpl_Pthreads::Halt( bool Kill )
{
#if defined(PID_BASED_THREADS)
	/*
	 * Send a SIGSTOP to the thread.  If we send a SIGKILL, pthreads will
	 * "helpfully" propagate it to the other threads, and we'll get killed, too.
	 *
	 * This isn't ideal, since it can cause the process to background as far as
	 * the shell is concerned, so the shell prompt can display before the crash
	 * handler actually displays a message.
	 */
	SuspendThread( pid );
#elif defined(DARWIN)
	thread_suspend( MachThreadHandle );
#endif
}

void ThreadImpl_Pthreads::Resume()
{
#if defined(PID_BASED_THREADS)
	/* Send a SIGCONT to the thread. */
	ResumeThread( pid );
#elif defined(DARWIN)
	thread_resume( MachThreadHandle );
#endif
}

uint64_t ThreadImpl_Pthreads::GetThreadId() const
{
#if defined(PID_BASED_THREADS)
	return (uint64_t) pid;
#elif defined(DARWIN)
	return MachThreadHandle;
#endif
}

int ThreadImpl_Pthreads::Wait()
{
	void *val;
	int ret = pthread_join( thread, &val );
	if( ret )
		RageException::Throw( "pthread_join: %s", strerror(ret) );

	return (int) val;
}

static ThreadImpl *MakeThisThread()
{
	ThreadImpl_Pthreads *thread = new ThreadImpl_Pthreads;

	thread->thread = pthread_self();

#if defined(PID_BASED_THREADS)
	thread->pid = GetCurrentThreadId(); /* in LinuxThreadHelpers.cpp */
#endif

#if defined(DARWIN)
	MachThreadHandle = mach_thread_self();
#endif

	return thread;
}

static void *StartThread( void *pData )
{
	ThreadImpl_Pthreads *pThis = (ThreadImpl_Pthreads *) pData;

#if defined(PID_BASED_THREADS)
	pThis->pid = GetCurrentThreadId(); /* in LinuxThreadHelpers.cpp */
#endif

#if defined(DARWIN)
	MachThreadHandle = mach_thread_self();
#endif

	return (void *) pThis->m_pFunc( pThis->m_pData );
}

ThreadImpl *MakeThread( int (*pFunc)(void *pData), void *pData )
{
	ThreadImpl_Win32 *thread = new ThreadImpl_Pthreads;
	thread->m_pFunc = pFunc;
	thread->m_pData = pData;

	int ret = pthread_create( &thread->thread, NULL, StartThread, thread );
	if( ret )
		RageException::Throw( "pthread_create: %s", strerror(ret) );

	/* XXX: don't return until StartThread sets pid, etc */
	
	return thread;
}

MutexImpl_Pthreads::MutexImpl_Pthreads( RageMutex *pParent ):
	MutexImpl( pParent )
{
	pthread_mutex_init( &mutex, NULL );
	LockedBy = GetInvalidThreadId();
	LockCnt = 0;
}

MutexImpl_Pthreads::~MutexImpl_Pthreads()
{
	int ret = pthread_mutex_destroy( &mutex ) == -1;
	if( ret )
		RageException::Throw( "Error deleting mutex: %s", strerror(ret) );
}


bool MutexImpl_Pthreads::Lock()
{
	if( LockedBy == GetCurrentThreadId() )
	{
		++LockCnt;
		return true;
	}

#if defined(HAVE_PTHREAD_MUTEX_TIMEDLOCK) && defined(CRASH_HANDLER)
	int len = 10; /* seconds */
	int tries = 2;

	while( tries-- )
	{
		/* Wait for ten seconds.  If it takes longer than that, we're probably deadlocked. */
		timeval tv;
		gettimeofday( &tv, NULL );

		timespec ts;
		ts.tv_sec = tv.tv_sec + len;
		ts.tv_nsec = tv.tv_usec * 1000;
		int ret = pthread_mutex_timedlock( &mutex, &ts );
		switch( ret )
		{
		case 0:
			LockedBy = GetCurrentThreadId();
			return true;

		case ETIMEDOUT:
			/* Timed out.  Probably deadlocked.  Try again one more time, with a smaller
			 * timeout, just in case we're debugging and happened to stop while waiting
			 * on the mutex. */
			len = 1;
			break;

		default:
			RageException::Throw( "pthread_mutex_timedlock: %s", strerror(ret) );
		}
	}

	return false;
#else
	int ret = pthread_mutex_lock( &mutex );
	if( ret )
		RageException::Throw( "pthread_mutex_lock failed: %s", strerror(ret) );
	LockedBy = GetCurrentThreadId();
	return true;
#endif
}


void MutexImpl_Pthreads::Unlock()
{
	if( LockCnt )
	{
		--LockCnt;
		return;
	}

	LockedBy = GetInvalidThreadId();
	pthread_mutex_unlock( &mutex );
}

uint64_t MutexImpl_Pthreads::GetLockedByThreadId() const
{
	return LockedBy;
}

uint64_t GetThisThreadId()
{
#if defined(PID_BASED_THREADS)
	return GetCurrentThreadId();
#elif defined(DARWIN)
	return (int) mach_thread_self();
#endif
}

uint64_t GetInvalidThreadId()
{
	return 0;
}

MutexImpl *MakeMutex( RageMutex *pParent )
{
	return new MutexImpl_Pthreads( pParent );
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
