/*
 * If you're going to use threads, remember this: 
 *
 * Threads suck.
 *
 * If there's any way to avoid them, take it!  Threaded code an order of
 * magnitude more complicated, harder to debug and harder to make robust.
 *
 * That said, here are a few helpers for when they're unavoidable.
 */

#include "global.h"

#include "RageThreads.h"
#include "RageTimer.h"
#include "RageLog.h"
#include "RageUtil.h"

#include <cerrno>
#include <set>

#include "arch/Threads/Threads.h"

#if defined(CRASH_HANDLER) && !defined(DARWIN)
#if defined(WIN32)
#include "archutils/Win32/crash.h"
#elif defined(LINUX)
#include "archutils/Unix/CrashHandler.h"
#endif
#endif

#define MAX_THREADS 128
//static vector<RageMutex*> *g_MutexList = NULL; /* watch out for static initialization order problems */

struct ThreadSlot
{
	mutable char name[1024]; /* mutable so we can force nul-termination */

	/* Format this beforehand, since it's easier to do that than to do it under crash conditions. */
	char ThreadFormattedOutput[1024];

	bool used;
	uint64_t id;

	ThreadImpl *pImpl;

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

	ThreadSlot() { Init(); }
	void Init()
	{
		id = GetInvalidThreadId();
		CurCheckpoint = NumCheckpoints = 0;
		pImpl = NULL;

		/* Reset used last; otherwise, a thread creation might pick up the slot. */
		used = false;
	}

	const char *GetThreadName() const;
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
struct ThreadSlot *g_pUnknownThreadSlot = NULL;

/* Lock this mutex before using or modifying pImpl.  Other values are just identifiers,
 * so possibly racing over them is harmless (simply using a stale thread ID, etc). */
static RageMutex g_ThreadSlotsLock("ThreadSlots");

static int FindEmptyThreadSlot()
{
	LockMut(g_ThreadSlotsLock);
	for( int entry = 0; entry < MAX_THREADS; ++entry )
	{
		if( g_ThreadSlots[entry].used )
			continue;

		return entry;
	}
			
	RageException::Throw("Out of thread slots!");
}

static ThreadSlot *GetThreadSlotFromID( uint64_t iID )
{
	for( int entry = 0; entry < MAX_THREADS; ++entry )
	{
		if( !g_ThreadSlots[entry].used )
			continue;
		if( g_ThreadSlots[entry].id == iID )
			return &g_ThreadSlots[entry];
	}
	return NULL;
}

static ThreadSlot *GetCurThreadSlot()
{
	return GetThreadSlotFromID( RageThread::GetCurrentThreadID() );
}

static ThreadSlot *GetUnknownThreadSlot()
{
	return g_pUnknownThreadSlot;
}

RageThread::RageThread()
{
	m_pSlot = NULL;
}

RageThread::~RageThread()
{

}

const char *ThreadSlot::GetThreadName() const
{
	/* This function may be called in crash conditions, so guarantee the string
	 * is null-terminated. */
	name[ sizeof(name)-1] = 0;

	return name;
}

void RageThread::Create( int (*fn)(void *), void *data )
{
	/* Don't create a thread that's already running: */
	ASSERT( m_pSlot == NULL );

	/* Lock unused slots, so nothing else uses our slot before we mark it used. */
	LockMut(g_ThreadSlotsLock);

	int slotno = FindEmptyThreadSlot();
	m_pSlot = &g_ThreadSlots[slotno];
	
	if( name == "" )
	{
		LOG->Warn("Created a thread without naming it first.");

		/* If you don't name it, I will: */
		strcpy( m_pSlot->name, "Joe" );
	} else {
		strcpy( m_pSlot->name, name.c_str() );
	}

	/* Start a thread using our own startup function. */
	m_pSlot->pImpl = MakeThread( fn, data );
	m_pSlot->id = m_pSlot->pImpl->GetThreadId();
	sprintf( m_pSlot->ThreadFormattedOutput, "Thread: %s", name.c_str() );

	/* Only after everything in the slot is valid, mark the slot used, so all
	 * "used" slots have a valid pImpl. */
	g_ThreadSlots[slotno].used = true;
}

/* On startup, register the main thread's slot. */
static struct SetupMainThread
{
	SetupMainThread()
	{
		LockMut(g_ThreadSlotsLock);
		int slot = FindEmptyThreadSlot();
		strcpy( g_ThreadSlots[slot].name, "Main thread" );
		sprintf( g_ThreadSlots[slot].ThreadFormattedOutput, "Thread: %s", g_ThreadSlots[slot].name );
		g_ThreadSlots[slot].pImpl = MakeThisThread();
		g_ThreadSlots[slot].used = true;
	}
} SetupMainThreadObj;

/* Register the "unknown thread" slot. */
static struct SetupUnknownThread
{
	SetupUnknownThread()
	{
		LockMut(g_ThreadSlotsLock);
		int slot = FindEmptyThreadSlot();
		strcpy( g_ThreadSlots[slot].name, "Unknown thread" );
		g_ThreadSlots[slot].id = GetInvalidThreadId();
		g_pUnknownThreadSlot = &g_ThreadSlots[slot];
		sprintf( g_ThreadSlots[slot].ThreadFormattedOutput, "Unknown thread" );
		g_ThreadSlots[slot].used = true;
	}
} SetupUnknownThreadObj;

const char *RageThread::GetCurThreadName()
{
	return GetThreadNameByID( GetCurrentThreadID() );
}

const char *RageThread::GetThreadNameByID( uint64_t iID )
{
	ThreadSlot *slot = GetThreadSlotFromID( iID );
	if( slot == NULL )
		return "???";

	return slot->GetThreadName();
}

int RageThread::Wait()
{
	ASSERT( m_pSlot != NULL );
	ASSERT( m_pSlot->pImpl != NULL );
	int ret = m_pSlot->pImpl->Wait();

	LockMut(g_ThreadSlotsLock);

	delete m_pSlot->pImpl;
	m_pSlot->pImpl = NULL;
	m_pSlot->Init();
	m_pSlot = NULL;

	return ret;
}


void RageThread::HaltAllThreads( bool Kill )
{
	const uint64_t ThisThreadID = GetThisThreadId();
	for( int entry = 0; entry < MAX_THREADS; ++entry )
	{
		if( !g_ThreadSlots[entry].used )
			continue;
		if( ThisThreadID == g_ThreadSlots[entry].id )
			continue;
		g_ThreadSlots[entry].pImpl->Halt( Kill );
	}
}

void RageThread::ResumeAllThreads()
{
	const uint64_t ThisThreadID = GetThisThreadId();
	for( int entry = 0; entry < MAX_THREADS; ++entry )
	{
		if( !g_ThreadSlots[entry].used )
			continue;
		if( ThisThreadID == g_ThreadSlots[entry].id )
			continue;

		g_ThreadSlots[entry].pImpl->Resume();
	}
}

uint64_t RageThread::GetCurrentThreadID()
{
	return GetThisThreadId();
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
	ThreadSlot *slot = GetCurThreadSlot();
	if( slot == NULL )
		slot = GetUnknownThreadSlot();
	/* We can't ASSERT here, since that uses checkpoints. */
	if( slot == NULL )
		sm_crash();

	slot->Checkpoints[slot->CurCheckpoint].Set( file, line, message );

	if( g_LogCheckpoints )
		LOG->Trace( "%s", slot->Checkpoints[slot->CurCheckpoint].FormattedBuf );

	++slot->CurCheckpoint;
	slot->NumCheckpoints = max( slot->NumCheckpoints, slot->CurCheckpoint );
	slot->CurCheckpoint %= CHECKPOINT_COUNT;
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
	if( &slot == g_pUnknownThreadSlot && slot.GetFormattedCheckpoint( 0 ) == NULL )
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

#if 0
static const int MAX_MUTEXES = 256;

/* g_MutexesBefore[n] is a list of mutex IDs which must be locked before n (if at all). 
 * The array g_MutexesBefore[n] is locked for writing by locking mutex n, so lock that
 * mutex *before* calling MarkLockedMutex(). */
bool g_MutexesBefore[MAX_MUTEXES][MAX_MUTEXES];

void RageMutex::MarkLockedMutex()
{
	/* This only makes locking take about 25% longer, and we generally don't lock in
	 * inner loops, so this is enabled by default for now. */
//	if( !g_bEnableMutexOrderChecking )
//		return;

	const int ID = this->m_UniqueID;
	ASSERT( ID < MAX_MUTEXES );

	/* This is a queue of all mutexes that must be locked before ID, if at all. */
	vector<const RageMutex *> before;

	/* Iterate over all locked mutexes that are locked by this thread. */
	unsigned i;
	for( i = 0; i < g_MutexList->size(); ++i )
	{
		const RageMutex *mutex = (*g_MutexList)[i];
		
		if( mutex->m_UniqueID == this->m_UniqueID )
			continue;

		if( !mutex->IsLockedByThisThread() )
			continue;

		/* mutex must be locked before this.  If we've previously marked the opposite,
		 * then we have an inconsistent lock order. */
		if( g_MutexesBefore[mutex->m_UniqueID][this->m_UniqueID] )
		{
			LOG->Warn( "Mutex lock inconsistency: mutex \"%s\" must be locked before \"%s\"",
				this->GetName().c_str(), mutex->GetName().c_str() );
			
			break;
		}
		
		/* Optimization: don't add it to the queue if it's already been done. */
		if( !g_MutexesBefore[this->m_UniqueID][mutex->m_UniqueID] )
			before.push_back( mutex );
	}
	
	while( before.size() )
	{
		const RageMutex *mutex = before.back();
		before.pop_back();
		
		g_MutexesBefore[this->m_UniqueID][mutex->m_UniqueID] = 1;

		/* All IDs which must be locked before mutex must also be locked before
		 * this.  That is, if A < mutex, because mutex < this, mark A < this. */
		for( i = 0; i < g_MutexList->size(); ++i )
		{
			const RageMutex *mutex2 = (*g_MutexList)[i];
			if( g_MutexesBefore[mutex->m_UniqueID][mutex2->m_UniqueID] )
				before.push_back( mutex2 );
		}
	}
}

/* XXX: How can g_FreeMutexIDs and g_MutexList be threadsafed? */
static set<int> *g_FreeMutexIDs = NULL;
#endif

RageMutex::RageMutex( const CString name ):
	m_sName( name )
{
	m_pMutex = MakeMutex( this );
	m_LockedBy = GetInvalidThreadId();
	m_LockCnt = 0;


/*	if( g_FreeMutexIDs == NULL )
	{
		g_FreeMutexIDs = new set<int>;
		for( int i = 0; i < MAX_MUTEXES; ++i )
			g_FreeMutexIDs->insert( i );
	}

	if( g_FreeMutexIDs->empty() )
	{
		ASSERT_M( g_MutexList, "!g_FreeMutexIDs but !g_MutexList?" ); // doesn't make sense to be out of mutexes yet never created any
		CString s;
		for( unsigned i = 0; i < g_MutexList->size(); ++i )
		{
			if( i )
				s += ", ";
			s += ssprintf( "\"%s\"", (*g_MutexList)[i]->GetName().c_str() );
		}
		LOG->Trace( "%s", s.c_str() );
		FAIL_M( ssprintf("MAX_MUTEXES exceeded creating \"%s\"", name.c_str() ) );
	}

	m_UniqueID = *g_FreeMutexIDs->begin();

	g_FreeMutexIDs->erase( g_FreeMutexIDs->begin() );

	if( g_MutexList == NULL )
		g_MutexList = new vector<RageMutex*>;

	g_MutexList->push_back( this );
*/
}

RageMutex::~RageMutex()
{
	delete m_pMutex;
/*
	vector<RageMutex*>::iterator it = find( g_MutexList->begin(), g_MutexList->end(), this );
	ASSERT( it != g_MutexList->end() );
	g_MutexList->erase( it );
	if( g_MutexList->empty() )
	{
		delete g_MutexList;
		g_MutexList = NULL;
	}

	delete m_pMutex;

	g_FreeMutexIDs->insert( m_UniqueID );
*/
}

void RageMutex::Lock()
{
	if( m_LockedBy == (uint64_t) GetThisThreadId() )
	{
		++m_LockCnt;
		return;
	}

	if( !m_pMutex->Lock() )
	{
		const ThreadSlot *ThisSlot = GetThreadSlotFromID( GetThisThreadId() );
		const ThreadSlot *OtherSlot = GetThreadSlotFromID( m_LockedBy );

		const CString sReason = ssprintf( "Thread deadlock on mutex %s between %s and %s", GetName().c_str(),
			ThisSlot? ThisSlot->GetThreadName(): "(???" ")", // stupid trigraph warnings
			OtherSlot? OtherSlot->GetThreadName(): "(???" ")" );

#if defined(CRASH_HANDLER) && !defined(DARWIN)
		/* Don't leave g_ThreadSlotsLock when we call ForceCrashHandlerDeadlock. */
		g_ThreadSlotsLock.Lock();
		uint64_t CrashHandle = OtherSlot? OtherSlot->pImpl->GetCrashHandle():0;
		g_ThreadSlotsLock.Unlock();

		/* Pass the crash handle of the other thread, so it can backtrace that thread. */
		ForceCrashHandlerDeadlock( sReason, CrashHandle );
#else
		RageException::Throw( "%s", sReason.c_str() );
#endif
	}

	m_LockedBy = GetThisThreadId();

	/* This has internal thread safety issues itself (eg. one thread may delete
	 * a mutex while another locks one); disable for now. */
//	MarkLockedMutex();
}

bool RageMutex::TryLock()
{
	if( m_LockedBy == (uint64_t) GetThisThreadId() )
	{
		++m_LockCnt;
		return true;
	}

	if( !m_pMutex->TryLock() )
		return false;

	m_LockedBy = GetThisThreadId();

	return true;
}

void RageMutex::Unlock()
{
	if( m_LockCnt )
	{
		--m_LockCnt;
		return;
	}

	m_LockedBy = GetInvalidThreadId();

	m_pMutex->Unlock();
}

bool RageMutex::IsLockedByThisThread() const
{
	return m_LockedBy == GetThisThreadId();
}

LockMutex::LockMutex(RageMutex &pMutex, const char *file_, int line_): 
	mutex(pMutex),
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

RageSemaphore::RageSemaphore( CString sName, int iInitialValue ):
	m_sName( sName )
{
	m_pSema = MakeSemaphore( iInitialValue );
}

RageSemaphore::~RageSemaphore()
{
	delete m_pSema;
}

int RageSemaphore::GetValue() const
{
	return m_pSema->GetValue();
}

void RageSemaphore::Post()
{
	m_pSema->Post();
}

void RageSemaphore::Wait()
{
	if( m_pSema->Wait() )
		return;

	/* We waited too long.  We're probably deadlocked, though unlike mutexes, we can't
	 * tell which thread we're stuck on. */
	const ThreadSlot *ThisSlot = GetThreadSlotFromID( GetThisThreadId() );
	const CString sReason = ssprintf( "Semaphore timeout on mutex %s on thread %s",
		GetName().c_str(), ThisSlot? ThisSlot->GetThreadName(): "(???" ")" ); // stupid trigraph warnings
#if defined(CRASH_HANDLER)
	ForceCrashHandler( sReason );
#else
	RageException::Throw( "%s", sReason.c_str() );
#endif
}

bool RageSemaphore::TryWait()
{
	return m_pSema->TryWait();
}


/*
 * Copyright (c) 2001-2004 Glenn Maynard
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
