#include "global.h"
#include "Threads_Pthreads.h"
#include "RageTimer.h"
#include "RageUtil.h"
#include <sys/time.h>
#include <errno.h>

#if defined(LINUX)
#include "archutils/Unix/LinuxThreadHelpers.h"
#include "archutils/Unix/RunningUnderValgrind.h"
#endif

#if defined(MACOSX)
#include "archutils/Darwin/DarwinThreadHelpers.h"
#endif

void ThreadImpl_Pthreads::Halt( bool Kill )
{
	/* Linux:
	 * Send a SIGSTOP to the thread.  If we send a SIGKILL, pthreads will
	 * "helpfully" propagate it to the other threads, and we'll get killed, too.
	 *
	 * This isn't ideal, since it can cause the process to background as far as
	 * the shell is concerned, so the shell prompt can display before the crash
	 * handler actually displays a message.
	 */
	SuspendThread( threadHandle );
}

void ThreadImpl_Pthreads::Resume()
{
	/* Linux: Send a SIGCONT to the thread. */
	ResumeThread( threadHandle );
}

uint64_t ThreadImpl_Pthreads::GetThreadId() const
{
	return threadHandle;
}

int ThreadImpl_Pthreads::Wait()
{
	int *val;
	int ret = pthread_join( thread, (void **) &val );
	if( ret )
		RageException::Throw( "pthread_join: %s", strerror(errno) );

	int iRet = *val;
	delete val;
	return iRet;
}

ThreadImpl *MakeThisThread()
{
	ThreadImpl_Pthreads *thread = new ThreadImpl_Pthreads;

	thread->thread = pthread_self();
	thread->threadHandle = GetCurrentThreadId();

	return thread;
}

static void *StartThread( void *pData )
{
	ThreadImpl_Pthreads *pThis = (ThreadImpl_Pthreads *) pData;

	pThis->threadHandle = GetCurrentThreadId();
	*pThis->m_piThreadID = pThis->threadHandle;
	
	/* Tell MakeThread that we've set m_piThreadID, so it's safe to return. */
	pThis->m_StartFinishedSem->Post();

	int iRet = pThis->m_pFunc( pThis->m_pData );
	
	return new int(iRet);
}

ThreadImpl *MakeThread( int (*pFunc)(void *pData), void *pData, uint64_t *piThreadID )
{
	ThreadImpl_Pthreads *thread = new ThreadImpl_Pthreads;
	thread->m_pFunc = pFunc;
	thread->m_pData = pData;
	thread->m_piThreadID = piThreadID;

	thread->m_StartFinishedSem = new SemaImpl_Pthreads( 0 );

	int ret = pthread_create( &thread->thread, NULL, StartThread, thread );
	if( ret )
		FAIL_M( ssprintf( "MakeThread: pthread_create: %s", strerror(errno)) );

	/* Don't return until StartThread sets m_piThreadID. */
	thread->m_StartFinishedSem->Wait();
	delete thread->m_StartFinishedSem;
	
	return thread;
}

MutexImpl_Pthreads::MutexImpl_Pthreads( RageMutex *pParent ):
	MutexImpl( pParent )
{
	pthread_mutex_init( &mutex, NULL );
}

MutexImpl_Pthreads::~MutexImpl_Pthreads()
{
	int ret = pthread_mutex_destroy( &mutex ) == -1;
	if( ret )
		RageException::Throw( "Error deleting mutex: %s", strerror(errno) );
}


#if defined(HAVE_PTHREAD_MUTEX_TIMEDLOCK) || defined(MACOSX)
static bool UseTimedlock()
{
#if defined(LINUX)
	/* Valgrind crashes and burns on pthread_mutex_timedlock. */
	if( RunningUnderValgrind() )
		return false;
#endif

	return true;
}
#endif

bool MutexImpl_Pthreads::Lock()
{
#if defined(HAVE_PTHREAD_MUTEX_TIMEDLOCK)
	if( UseTimedlock() )
	{
		int len = 10; /* seconds */
		int tries = 2;

		while( tries-- )
		{
			/* Wait for ten seconds.  If it takes longer than that, we're 
			 * probably deadlocked. */
			timeval tv;
			gettimeofday( &tv, NULL );

			timespec ts;
			ts.tv_sec = tv.tv_sec + len;
			ts.tv_nsec = tv.tv_usec * 1000;
			int ret = pthread_mutex_timedlock( &mutex, &ts );
			switch( ret )
			{
			case 0:
				return true;

			case EINTR:
				/* Ignore it. */
				++tries;
				continue;

			case ETIMEDOUT:
				/* Timed out.  Probably deadlocked.  Try again one more time, with a smaller
				 * timeout, just in case we're debugging and happened to stop while waiting
				 * on the mutex. */
				len = 1;
				break;

			default:
				FAIL_M( ssprintf("pthread_mutex_timedlock: %s", strerror(errno)) );
			}
		}

		return false;
	}
#endif

	int ret;
	do
	{
		ret = pthread_mutex_lock( &mutex );
	}
	while( ret == -1 && ret == EINTR );

	ASSERT_M( ret == 0, ssprintf("pthread_mutex_lock: %s", strerror(errno)) );

	return true;
}

bool MutexImpl_Pthreads::TryLock()
{
	int ret = pthread_mutex_trylock( &mutex );
	if( ret == EBUSY )
		return false;
	if( ret )
		RageException::Throw( "pthread_mutex_lock failed: %s", strerror(errno) );
	return true;
}

void MutexImpl_Pthreads::Unlock()
{
	pthread_mutex_unlock( &mutex );
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
	return new MutexImpl_Pthreads( pParent );
}

EventImpl_Pthreads::EventImpl_Pthreads( MutexImpl_Pthreads *pParent )
{
	m_pParent = pParent;
	pthread_cond_init( &m_Cond, NULL );
}

EventImpl_Pthreads::~EventImpl_Pthreads()
{
	pthread_cond_destroy( &m_Cond );
}

#if defined(HAVE_PTHREAD_COND_TIMEDWAIT)
bool EventImpl_Pthreads::Wait( RageTimer *pTimeout )
{
	if( pTimeout == NULL )
	{
		pthread_cond_wait( &m_Cond, &m_pParent->mutex );
		return true;
	}

	timespec abstime;
	abstime.tv_sec = pTimeout->m_secs;
	abstime.tv_nsec = pTimeout->m_us * 1000;

	int iRet = pthread_cond_timedwait( &m_Cond, &m_pParent->mutex, &abstime );
	return iRet != ETIMEDOUT;
}
#else
bool EventImpl_Pthreads::Wait( RageTimer *pTimeout )
{
	pthread_cond_wait( &m_Cond, &m_pParent->mutex );
	return true;
}
#endif

void EventImpl_Pthreads::Signal()
{
	pthread_cond_signal( &m_Cond );
}

void EventImpl_Pthreads::Broadcast()
{
	pthread_cond_broadcast( &m_Cond );
}

EventImpl *MakeEvent( MutexImpl *pMutex )
{
	MutexImpl_Pthreads *pPthreadsMutex = (MutexImpl_Pthreads *) pMutex;

	return new EventImpl_Pthreads( pPthreadsMutex );
}

#if 0
SemaImpl_Pthreads::SemaImpl_Pthreads( int iInitialValue )
{
	sem_init( &sem, 0, iInitialValue );
}

SemaImpl_Pthreads::~SemaImpl_Pthreads()
{
	sem_destroy( &sem );
}

int SemaImpl_Pthreads::GetValue() const
{
	int ret;
	sem_getvalue( const_cast<sem_t *>(&sem), &ret );
	return ret;
}

void SemaImpl_Pthreads::Post()
{
	sem_post( &sem );
}

bool SemaImpl_Pthreads::Wait()
{
	int ret;
	do
	{
		ret = sem_wait( &sem );
	}
	while( ret == -1 && errno == EINTR );

	ASSERT_M( ret == 0, ssprintf("Wait: sem_wait: %s", strerror(errno)) );

	return true;
}

bool SemaImpl_Pthreads::TryWait()
{
	int ret = sem_trywait( &sem );
	if( ret == EBUSY )
		return false;
	if( ret )
		RageException::Throw( "TryWait: sem_trywait failed: %s", strerror(errno) );
	return true;
}
#else
/* Use conditions, to work around OSX "forgetting" to implement semaphores. */
SemaImpl_Pthreads::SemaImpl_Pthreads( int iInitialValue )
{
	int ret = pthread_cond_init( &m_Cond, NULL );
	ASSERT_M( ret == 0, ssprintf( "SemaImpl_Pthreads: pthread_cond_init: %s", strerror(errno)) );
	ret = pthread_mutex_init( &m_Mutex, NULL );
	ASSERT_M( ret == 0, ssprintf( "SemaImpl_Pthreads: pthread_mutex_init: %s", strerror(errno)) );
		
	m_iValue = iInitialValue;
}

SemaImpl_Pthreads::~SemaImpl_Pthreads()
{
	pthread_cond_destroy( &m_Cond );
	pthread_mutex_destroy( &m_Mutex );
}

void SemaImpl_Pthreads::Post()
{
	pthread_mutex_lock( &m_Mutex );
	++m_iValue;
	if( m_iValue == 1 )
		pthread_cond_signal( &m_Cond );
	pthread_mutex_unlock( &m_Mutex );
}

bool SemaImpl_Pthreads::Wait()
{
#if defined(HAVE_PTHREAD_MUTEX_TIMEDLOCK) || defined(MACOSX)
	if( UseTimedlock() )
	{
		timeval tv;
		gettimeofday( &tv, NULL );

		/* Wait for ten seconds.  If it takes longer than that, we're probably deadlocked. */
		timespec ts;
		ts.tv_sec = tv.tv_sec + 10;
		ts.tv_nsec = tv.tv_usec * 1000;

		pthread_mutex_lock( &m_Mutex );

		int tries = 2;
		while( !m_iValue && tries )
		{
			int ret = pthread_cond_timedwait( &m_Cond, &m_Mutex, &ts );

			switch( ret )
			{
			case 0:
			case EINTR:
				break;

			case ETIMEDOUT:
				/* Timed out.  Probably deadlocked.  Try again one more time, with a smaller
				 * timeout, just in case we're debugging and happened to stop while waiting
				 * on the mutex. */
				++ts.tv_sec;
				tries--;
				break;

			default:
				FAIL_M( ssprintf("pthread_mutex_timedlock: %s", strerror(errno)) );
			}
		}

		if( !m_iValue )
		{
			/* Timed out. */
			pthread_mutex_unlock( &m_Mutex );
			return false;
		}
		else
		{
			--m_iValue;
			pthread_mutex_unlock( &m_Mutex );
			return true;
		}
	}
#endif

	pthread_mutex_lock( &m_Mutex );
	while( !m_iValue )
		pthread_cond_wait( &m_Cond, &m_Mutex );

	--m_iValue;
	pthread_mutex_unlock( &m_Mutex);

	return true;
}

bool SemaImpl_Pthreads::TryWait()
{
	pthread_mutex_lock( &m_Mutex );
	if( !m_iValue )
	{
		pthread_mutex_unlock( &m_Mutex);
		return false;
	}
	
	--m_iValue;
	pthread_mutex_unlock( &m_Mutex);

	return true;
}

#endif

SemaImpl *MakeSemaphore( int iInitialValue )
{
	return new SemaImpl_Pthreads( iInitialValue );
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
