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

#ifdef _WINDOWS
#include "archutils/win32/tls.h"
#endif

#include "SDL_utils.h"

#include <signal.h>

/* SDL threads aren't quite enough.  We need to be able to suspend or
 * kill all threads, including the main one.  SDL doesn't count the
 * main thread as a thread.  So, we'll have to do this nonportably. */
#if defined(LINUX)
#define PID_BASED_THREADS
#endif

#define MAX_THREADS 128

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

	#undef CHECKPOINT_COUNT
	#define CHECKPOINT_COUNT 2
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
	pid = getpid();
#endif

#ifdef _WINDOWS
	InitThreadData( name );
#endif

	threadid = SDL_ThreadID();

	sprintf(ThreadFormattedOutput, "Thread %08x (%s)", threadid, name);
	CHECKPOINT;
}

void ThreadSlot::ShutdownThisThread()
{
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

struct SetupMainThread
{
	SetupMainThread()
	{
		int slot = FindEmptyThreadSlot();
		strcpy( g_ThreadSlots[slot].name, "Main thread" );
		g_ThreadSlots[slot].SetupThisThread();
	}
} SetupMainThreadObj;

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
	int ThisThread = getpid();
	for( int entry = 0; entry < MAX_THREADS; ++entry )
	{
		if( !g_ThreadSlots[entry].used )
			continue;
		const int pid = g_ThreadSlots[entry].pid;
		if( pid <= 0 || pid == ThisThread )
			continue;
		kill( pid, Kill? SIGKILL:SIGSTOP );
	}
#endif
}


void SetCheckpoint( const char *file, int line, const char *message )
{
	int slotno = GetCurThreadSlot();
	ASSERT( slotno != -1 );

	ThreadSlot &slot = g_ThreadSlots[slotno];
	
	slot.Checkpoints[slot.CurCheckpoint].Set( file, line, message );

	++slot.CurCheckpoint;
	slot.NumCheckpoints = max( slot.NumCheckpoints, slot.CurCheckpoint );
	slot.CurCheckpoint %= CHECKPOINT_COUNT;
}

/* This is called under crash conditions.  Be careful. */
const char *GetCheckpointLog( int slotno, int lineno )
{
	static char ret[1024*32];
	ret[0] = 0;

	ThreadSlot &slot = g_ThreadSlots[slotno];
	if( !slot.used )
		return NULL;

	if( lineno != 0 )
		return slot.GetFormattedCheckpoint( lineno-1 );

	slot.ThreadFormattedOutput[sizeof(slot.ThreadFormattedOutput)-1] = 0;
	strcat(ret, slot.ThreadFormattedOutput);
	return ret;
}

const char *GetCheckpointLogs( const char *delim )
{
	static char ret[1024*32];
	ret[0] = 0;

	for( int slotno = 0; slotno < MAX_THREADS; ++slotno )
	{
		const char *buf = GetCheckpointLog( slotno, 0 );
		if( buf == NULL )
			break;
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


RageMutex::RageMutex()
{
	Locked = 0;
	mut = SDL_CreateMutex();
	mutwait = SDL_CreateMutex();
}

RageMutex::~RageMutex()
{
	SDL_DestroyMutex(mut);
	SDL_DestroyMutex(mutwait);
}

void RageMutex::Lock()
{
	while(1)
	{
		SDL_LockMutex(mut);
		if(!Locked || LockedBy == SDL_ThreadID())
		{
			if(!Locked)
			{
				/* This mutex is now locked. */
				SDL_LockMutex(mutwait);
				LockedBy = SDL_ThreadID();
			} /* (else it was already locked and we're just increasing the counter) */
			Locked++;
			SDL_UnlockMutex(mut);
			return;
		}

		SDL_UnlockMutex(mut);

		/* Someone else is locking it.  Wait until it's available and try again. */
		SDL_LockMutex(mutwait);
		SDL_UnlockMutex(mutwait);
	}
}

void RageMutex::Unlock()
{
	SDL_LockMutex(mut);
	ASSERT(Locked);
	ASSERT(LockedBy == SDL_ThreadID());

	Locked--;
	if(!Locked)
	{
		LockedBy = 0;
		SDL_UnlockMutex(mutwait);
	}

	SDL_UnlockMutex(mut);
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
	ASSERT(locked);
	locked = false;

	mutex.Unlock();

	if(file && locked_at != -1)
	{
		float dur = RageTimer::GetTimeSinceStart() - locked_at;
		if(dur > 0.015)
			LOG->Trace(ssprintf("Lock at %s:%i took %f", file, line, dur));
	}
}


/*
-----------------------------------------------------------------------------
 File: RageThreads

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
    Glenn Maynard
-----------------------------------------------------------------------------
*/
