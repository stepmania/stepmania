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

	bool used;

#if defined(PID_BASED_THREADS)
	/* Keep a list of child PIDs, so we can send them SIGKILL.  This has an
	 * added bonus: if this is corrupted, we'll just send signals and they'll
	 * fail; we won't blow up (unless we're root). */
	int pid;
#endif

	/* Used to bootstrap the thread: */
	int (*fn)(void *);
	void *data;

	ThreadSlot()
	{
		used = false;
#if defined(PID_BASED_THREADS)
		pid = -1;
#endif
	}

	void SetupThisThread();
	void ShutdownThisThread();
};


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
	threadid = SDL_ThreadID();
}

void ThreadSlot::ShutdownThisThread()
{
#if defined(PID_BASED_THREADS)
	pid = -1;
#endif
	used = false;
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

void RageThread::HaltAllThreads()
{
#if defined(PID_BASED_THREADS)
	int ThisThread = getpid();
	for( int entry = 0; entry < MAX_THREADS; ++entry )
	{
		if( !g_ThreadSlots[entry].used )
			continue;
		const int pid = g_ThreadSlots[entry].pid;
		if( pid <= 0 || pid == ThisThread )
			continue;
		kill( pid, SIGKILL );
	}
#endif
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
