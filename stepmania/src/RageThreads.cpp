/*
 * If you're going to use threads, remember this: 
 *
 * Threads suck.
 *
 * If there's any way to avoid them, take it!  Threaded code an order of
 * magnitude more complicated, harder to debug and harder to make robust.
 *
 * That said, here are a few helpers for when they're unavoidable.  (Use
 * SDL for the rest.)
 */

#include "global.h"

#include "RageThreads.h"
#include "RageTimer.h"
#include "RageLog.h"
#include "RageUtil.h"
#include "SDL_thread.h"
#include "SDL_utils.h"

#include <signal.h>

/* SDL threads aren't quite enough.  We need to be able to suspend or
 * kill all threads, including the main one.  SDL doesn't count the
 * main thread as a thread.  So, we'll have to do this nonportably. */
#if defined(LINUX)
#define PID_BASED_THREADS
#include "archutils/Unix/LinuxThreadHelpers.h"
#endif

#if defined(WIN32)
#include "archutils/Win32/crash.h"
#endif

#define MAX_THREADS 128

static const unsigned int UnknownThreadID = 0xFFFFFFFF;
struct ThreadSlot
{
	char name[1024];
	Uint32 threadid;

	/* Format this beforehand, since it's easier to do that than to do it under crash conditions. */
	char ThreadFormattedOutput[1024];

	bool used;

#if defined(PID_BASED_THREADS)
	/* Keep a list of child PIDs, so we can send them SIGKILL.  This has an
	 * added bonus: if this is corrupted, we'll just send signals and they'll
	 * fail; we won't blow up (unless we're root). */
	int pid;
#endif

#if defined(WIN32)
	HANDLE ThreadHandle;
#endif

	#undef CHECKPOINT_COUNT
	#define CHECKPOINT_COUNT 5
	struct ThreadCheckpoint
	{
		const char *File, *Message;
		int Line;
		char FormattedBuf[1024];

		ThreadCheckpoint() { Set( NULL, 0, NULL ); }
		void Set(const char *File_, int Line_, const char *Message_=NULL);
		const char *GetFormattedCheckpoint();
	};
	ThreadCheckpoint Checkpoints[CHECKPOINT_COUNT];
	int CurCheckpoint, NumCheckpoints;
	const char *GetFormattedCheckpoint( int lineno );

	/* Used to bootstrap the thread: */
	int (*fn)(void *);
	void *data;

	ThreadSlot() { Init(); }
	void Init()
	{
		used = false;
		CurCheckpoint = NumCheckpoints = 0;
#if defined(PID_BASED_THREADS)
		pid = -1;
#endif
	}

	void SetupThisThread();
	void ShutdownThisThread();
	void SetupUnknownThread();
};

void ThreadSlot::ThreadCheckpoint::Set(const char *File_, int Line_, const char *Message_)
{
	File=File_;
	Line=Line_;
	Message=Message_;
	sprintf( FormattedBuf, "        %s:%i %s",
		File, Line, Message? Message:"" );
}

const char *ThreadSlot::ThreadCheckpoint::GetFormattedCheckpoint()
{
	if( File == NULL )
		return NULL;

	/* Make sure it's terminated: */
	FormattedBuf [ sizeof(FormattedBuf)-1 ] = 0;

	return FormattedBuf;
}

const char *ThreadSlot::GetFormattedCheckpoint( int lineno )
{
	if( lineno >= CHECKPOINT_COUNT || lineno >= NumCheckpoints )
		return NULL;

	if( NumCheckpoints == CHECKPOINT_COUNT )
	{
		lineno += CurCheckpoint;
		lineno %= CHECKPOINT_COUNT;
	}

	return Checkpoints[lineno].GetFormattedCheckpoint();
}

static ThreadSlot g_ThreadSlots[MAX_THREADS];
static RageMutex g_ThreadSlotsLock;

static int FindEmptyThreadSlot()
{
	LockMut(g_ThreadSlotsLock);
	for( int entry = 0; entry < MAX_THREADS; ++entry )
	{
		if( g_ThreadSlots[entry].used )
			continue;

		g_ThreadSlots[entry].used = true;
		return entry;
	}
			
	RageException::Throw("Out of thread slots!");
}

static int GetCurThreadSlot()
{
	Uint32 ThisThread = SDL_ThreadID();

	for( int entry = 0; entry < MAX_THREADS; ++entry )
	{
		if( !g_ThreadSlots[entry].used )
			continue;
		if( g_ThreadSlots[entry].threadid == ThisThread )
			return entry;
	}
	return -1;
}

static int GetUnknownThreadSlot()
{
	for( int entry = 0; entry < MAX_THREADS; ++entry )
	{
		if( !g_ThreadSlots[entry].used )
			continue;
		if( g_ThreadSlots[entry].threadid == UnknownThreadID )
			return entry;
	}

	sm_crash();
}

RageThread::RageThread()
{
	thr = NULL;
}

RageThread::~RageThread()
{

}


void ThreadSlot::SetupThisThread()
{
#if defined(PID_BASED_THREADS)
	pid = GetCurrentThreadId();
#endif

#ifdef _WINDOWS
	const HANDLE CurProc = GetCurrentProcess();
	int ret = DuplicateHandle( CurProc, GetCurrentThread(), CurProc, 
		&ThreadHandle, 0, false, DUPLICATE_SAME_ACCESS );
	if( !ret )
		LOG->Warn( werr_ssprintf( GetLastError(), "DuplicateHandle(%p, %p) failed",
			CurProc, GetCurrentThread() ) );
#endif

	threadid = SDL_ThreadID();

	sprintf(ThreadFormattedOutput, "Thread %08x (%s)", threadid, name);
	CHECKPOINT;
}

void ThreadSlot::SetupUnknownThread()
{
	threadid = UnknownThreadID;

	sprintf(ThreadFormattedOutput, "Unknown thread");
}

void ThreadSlot::ShutdownThisThread()
{
	ASSERT( threadid != UnknownThreadID );

#ifdef _WINDOWS
	CloseHandle( ThreadHandle );
#endif

	Init();
}

static int StartThread( void *p )
{
	ThreadSlot *thr = (ThreadSlot *) p;

	thr->SetupThisThread();

	int ret = thr->fn(thr->data);

	thr->ShutdownThisThread();

	return ret;
}

void RageThread::Create( int (*fn)(void *), void *data )
{
	/* Don't create a thread that's already running: */
	ASSERT( thr == NULL );

	int slotno = FindEmptyThreadSlot();
	ThreadSlot &slot = g_ThreadSlots[slotno];
	slot.fn = fn;
	slot.data = data;
	
	if( name == "" )
	{
		LOG->Warn("Created a thread without naming it first.");

		/* If you don't name it, I will: */
		strcpy(slot.name, "Joe");
	} else {
		strcpy(slot.name, name.c_str());
	}

	/* Start a thread using our own startup function. */
	thr = SDL_CreateThread( StartThread, &slot );
	if( thr == NULL )
		RageException::Throw( "Thread creation failed: %s", SDL_GetError() );
}

/* On startup, register the main thread's slot. */
static struct SetupMainThread
{
	SetupMainThread()
	{
		int slot = FindEmptyThreadSlot();
		strcpy( g_ThreadSlots[slot].name, "Main thread" );
		g_ThreadSlots[slot].SetupThisThread();
	}
} SetupMainThreadObj;

/* Register the "unknown thread" slot. */
static struct SetupUnknownThread
{
	SetupUnknownThread()
	{
		int slot = FindEmptyThreadSlot();
		strcpy( g_ThreadSlots[slot].name, "Unknown thread" );
		g_ThreadSlots[slot].SetupUnknownThread();
	}
} SetupUnknownThreadObj;

const char *RageThread::GetCurThreadName()
{
	int slot = GetCurThreadSlot();
	if(slot==-1)
		return "???";

	/* This function may be called in crash conditions, so guarantee the string
	 * is null-terminated. */
	g_ThreadSlots[slot].name[ sizeof(g_ThreadSlots[slot].name)-1] = 0;

	return g_ThreadSlots[slot].name;
}

int RageThread::Wait()
{
	ASSERT( thr != NULL );

	int ret;
	SDL_WaitThread(thr, &ret);
	thr = NULL;
	return ret;
}

void RageThread::HaltAllThreads( bool Kill )
{
#if defined(PID_BASED_THREADS)
	/* Send a SIGSTOP to all other threads.  If we send a SIGKILL, pthreads
	 * will "helpfully" propagate it to the other threads, and we'll get
	 * killed, too.
	 *
	 * This isn't ideal, since it can cause the process to background
	 * as far as the shell is concerned, so the shell prompt can display
	 * before the crash handler actually displays a message. */
	int ThisThreadID = GetCurrentThreadId();
	for( int entry = 0; entry < MAX_THREADS; ++entry )
	{
		if( !g_ThreadSlots[entry].used )
			continue;
		const int pid = g_ThreadSlots[entry].pid;
		if( pid <= 0 || pid == ThisThreadID )
			continue;
		SuspendThread( pid );
	}
#elif defined(WIN32)
	const int ThisThreadID = GetCurrentThreadId();
	for( int entry = 0; entry < MAX_THREADS; ++entry )
	{
		if( !g_ThreadSlots[entry].used )
			continue;
		if( ThisThreadID == (int) g_ThreadSlots[entry].threadid )
			continue;
#ifndef _XBOX
		if( Kill )
			TerminateThread( g_ThreadSlots[entry].ThreadHandle, 0 );
		else
#endif
			SuspendThread( g_ThreadSlots[entry].ThreadHandle );
	}
#endif
}

void RageThread::ResumeAllThreads()
{
#if defined(PID_BASED_THREADS)
	/* Send a SIGCONT to all other threads. */
	int ThisThreadID = GetCurrentThreadId();
	for( int entry = 0; entry < MAX_THREADS; ++entry )
	{
		if( !g_ThreadSlots[entry].used )
			continue;
		const int pid = g_ThreadSlots[entry].pid;
		if( pid <= 0 || pid == ThisThreadID )
			continue;
		ResumeThread( pid );
	}
#elif defined(WIN32)
	const int ThisThreadID = GetCurrentThreadId();
	for( int entry = 0; entry < MAX_THREADS; ++entry )
	{
		if( !g_ThreadSlots[entry].used )
			continue;
		if( ThisThreadID == (int) g_ThreadSlots[entry].threadid )
			continue;

		ResumeThread( g_ThreadSlots[entry].ThreadHandle );
	}
#endif
}

/* Normally, checkpoints are only seen in crash logs.  It's occasionally useful
 * to see them in logs, but this outputs a huge amount of text. */
static bool g_LogCheckpoints = false;
void Checkpoints::LogCheckpoints( bool on )
{
	g_LogCheckpoints = on;
}

void Checkpoints::SetCheckpoint( const char *file, int line, const char *message )
{
	int slotno = GetCurThreadSlot();
	if( slotno == -1 )
		slotno = GetUnknownThreadSlot();
	/* We can't ASSERT here, since that uses checkpoints. */
	if( slotno == -1 )
		sm_crash();

	ThreadSlot &slot = g_ThreadSlots[slotno];
	
	slot.Checkpoints[slot.CurCheckpoint].Set( file, line, message );

	if( g_LogCheckpoints )
		LOG->Trace( "%s", slot.Checkpoints[slot.CurCheckpoint].FormattedBuf );

	++slot.CurCheckpoint;
	slot.NumCheckpoints = max( slot.NumCheckpoints, slot.CurCheckpoint );
	slot.CurCheckpoint %= CHECKPOINT_COUNT;
}

/* This is called under crash conditions.  Be careful. */
static const char *GetCheckpointLog( int slotno, int lineno )
{
	static char ret[1024*32];
	ret[0] = 0;

	ThreadSlot &slot = g_ThreadSlots[slotno];
	if( !slot.used )
		return NULL;

	/* Only show the "Unknown thread" entry if it has at least one checkpoint. */
	if( slot.threadid == UnknownThreadID && slot.GetFormattedCheckpoint( 0 ) == NULL )
		return NULL;

	if( lineno != 0 )
		return slot.GetFormattedCheckpoint( lineno-1 );

	slot.ThreadFormattedOutput[sizeof(slot.ThreadFormattedOutput)-1] = 0;
	strcat(ret, slot.ThreadFormattedOutput);
	return ret;
}

const char *Checkpoints::GetLogs( const char *delim )
{
	static char ret[1024*32];
	ret[0] = 0;

	for( int slotno = 0; slotno < MAX_THREADS; ++slotno )
	{
		const char *buf = GetCheckpointLog( slotno, 0 );
		if( buf == NULL )
			continue;
		strcat( ret, buf );
		strcat( ret, delim );
		
		for( int line = 1; (buf = GetCheckpointLog( slotno, line )) != NULL; ++line )
		{
			strcat( ret, buf );
			strcat( ret, delim );
		}
	}	
	return ret;
}

/*
 * "Safe" mutexes: locking the same mutex more than once from the same thread
 * is refcounted and does not deadlock. 
 *
 * Only actually lock the mutex once; when we do so, remember which thread locked it.
 * Then, when we lock in the future, only increment a counter, with no locks.
 *
 * We must be holding the real mutex to write to LockedBy and LockCnt.  However,
 * we can look at LockedBy to see if it's us that owns it (in which case, we already
 * hold the mutex).
 *
 * In Windows, this helps smooth out performance: for some reason, Windows likes
 * to yank the scheduler away from a thread that locks a mutex that it already owns.
 */
#if defined(WIN32)
struct RageMutexImpl
{
	HANDLE mutex;
	DWORD LockedBy;
	volatile int LockCnt;

	RageMutexImpl();
	~RageMutexImpl();

	void Lock();
	void Unlock();
};

RageMutexImpl::RageMutexImpl()
{
	mutex = CreateMutex( NULL, false, NULL );
	LockedBy = NULL;
	LockCnt = 0;
}

RageMutexImpl::~RageMutexImpl()
{
	CloseHandle( mutex );
}


static ThreadSlot *FindThread( DWORD id )
{
	for( int i = 0; i < MAX_THREADS; ++i )
		if( g_ThreadSlots[i].threadid == id )
			return &g_ThreadSlots[i];
	return NULL;
}

void RageMutexImpl::Lock()
{
	if( LockedBy == GetCurrentThreadId() )
	{
		++LockCnt;
		return;
	}

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
			LockedBy = GetCurrentThreadId();
			return;

		case WAIT_TIMEOUT:
			/* Timed out.  Probably deadlocked.  Try again one more time, with a smaller
			 * timeout, just in case we're debugging and happened to stop while waiting
			 * on the mutex. */
			len = 1000;
			break;
		}
	}

	ThreadSlot *slot = FindThread( LockedBy );
	Crash_BacktraceThread( slot? slot->ThreadHandle:NULL );
}

void RageMutexImpl::Unlock()
{
	if( LockCnt )
	{
		--LockCnt;
		return;
	}

	LockedBy = NULL;
	const bool ret = !!ReleaseMutex( mutex );

	/* We can't ASSERT here, since this is called from checkpoints, which is
	 * called from ASSERT. */
	if( !ret )
		sm_crash();
}

#else
/* SDL implementation. */
struct RageMutexImpl
{
	unsigned LockedBy;
	volatile int LockCnt;

	SDL_mutex *mutex;

	RageMutexImpl();
	~RageMutexImpl();

	void Lock();
	void Unlock();
};

RageMutexImpl::RageMutexImpl()
{
	mutex = SDL_CreateMutex();
	LockedBy = 0;
	LockCnt = 0;
}

RageMutexImpl::~RageMutexImpl()
{
	SDL_DestroyMutex(mutex);
}


void RageMutexImpl::Lock()
{
	if( LockedBy == SDL_ThreadID() )
	{
		++LockCnt;
		return;
	}

	SDL_LockMutex( mutex );
	LockedBy = SDL_ThreadID();
}

void RageMutexImpl::Unlock()
{
	if( LockCnt )
	{
		--LockCnt;
		return;
	}

	LockedBy = 0;
	SDL_UnlockMutex( mutex );
}
#endif



RageMutex::RageMutex()
{
	mut = new RageMutexImpl;
}

RageMutex::~RageMutex()
{
	delete mut;
}

void RageMutex::Lock()
{
	mut->Lock();
}

void RageMutex::Unlock()
{
	mut->Unlock();
}

LockMutex::LockMutex(RageMutex &mut, const char *file_, int line_): 
	mutex(mut),
	file(file_),
	line(line_),
	locked_at(RageTimer::GetTimeSinceStart())
{
	mutex.Lock();
	locked = true;
}

LockMutex::~LockMutex()
{
	if(locked)
		mutex.Unlock();
}

void LockMutex::Unlock()
{
	ASSERT( locked );
	locked = false;

	mutex.Unlock();

	if( file && locked_at != -1 )
	{
		const float dur = RageTimer::GetTimeSinceStart() - locked_at;
		if( dur > 0.015f )
			LOG->Trace( "Lock at %s:%i took %f", file, line, dur );
	}
}


/*
-----------------------------------------------------------------------------
 File: RageThreads

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
    Glenn Maynard
-----------------------------------------------------------------------------
*/
