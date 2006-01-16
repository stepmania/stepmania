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
#include "arch/Dialog/Dialog.h"

#if defined(CRASH_HANDLER)
#if defined(_WINDOWS)
#include "archutils/Win32/crash.h"
#elif defined(LINUX) || defined(MACOSX)
#include "archutils/Unix/CrashHandler.h"
#endif
#endif

/* Assume TLS doesn't work until told otherwise.  It's ArchHooks's job to set this. */
bool RageThread::s_bSystemSupportsTLS = false;
bool RageThread::s_bIsShowingDialog = false;

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

	void Release()
	{
		SAFE_DELETE( pImpl );
		Init();
	}

	const char *GetThreadName() const;
};


void ThreadSlot::ThreadCheckpoint::Set( const char *File_, int Line_, const char *Message_ )
{
	File=File_;
	Line=Line_;
	Message=Message_;
	sprintf( FormattedBuf, "        %s:%i %s", File, Line, Message? Message:"" );
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
static RageMutex &GetThreadSlotsLock()
{
	static RageMutex *pLock = new RageMutex( "ThreadSlots" );
	return *pLock;
}

static int FindEmptyThreadSlot()
{
	LockMut( GetThreadSlotsLock() );
	for( int entry = 0; entry < MAX_THREADS; ++entry )
	{
		if( g_ThreadSlots[entry].used )
			continue;

		g_ThreadSlots[entry].used = true;
		return entry;
	}
			
	RageException::Throw( "Out of thread slots!" );
}

static void InitThreads()
{
	/* We don't have to worry about two threads calling this at once, since it's
	 * called when we create a thread. */
	static bool bInitialized = false;
	if( bInitialized )
		return;

	GetThreadSlotsLock().Lock();

	/* Libraries might start threads on their own, which might call user callbacks,
	 * which could come back here.  Make sure we don't accidentally initialize twice. */
	if( bInitialized )
	{
		GetThreadSlotsLock().Unlock();
		return;
	}

	bInitialized = true;

	/* Register the "unknown thread" slot. */
	int slot = FindEmptyThreadSlot();
	strcpy( g_ThreadSlots[slot].name, "Unknown thread" );
	g_ThreadSlots[slot].id = GetInvalidThreadId();
	sprintf( g_ThreadSlots[slot].ThreadFormattedOutput, "Unknown thread" );
	g_pUnknownThreadSlot = &g_ThreadSlots[slot];

	GetThreadSlotsLock().Unlock();
}


static ThreadSlot *GetThreadSlotFromID( uint64_t iID )
{
	InitThreads();

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
	if( m_pSlot != NULL )
		Wait();
}

const char *ThreadSlot::GetThreadName() const
{
	/* This function may be called in crash conditions, so guarantee the string
	 * is null-terminated. */
	name[sizeof(name)-1] = 0;

	return name;
}

void RageThread::Create( int (*fn)(void *), void *data )
{
	/* Don't create a thread that's already running: */
	ASSERT( m_pSlot == NULL );

	InitThreads();

	/* Lock unused slots, so nothing else uses our slot before we mark it used. */
	LockMut(GetThreadSlotsLock());

	int slotno = FindEmptyThreadSlot();
	m_pSlot = &g_ThreadSlots[slotno];
	
	if( name == "" )
	{
		if( LOG )
			LOG->Warn( "Created a thread without naming it first." );

		/* If you don't name it, I will: */
		strcpy( m_pSlot->name, "Joe" );
	}
	else
	{
		strcpy( m_pSlot->name, name.c_str() );
	}

	if( LOG )
		LOG->Trace( "Starting thread: %s", name.c_str() );
	sprintf( m_pSlot->ThreadFormattedOutput, "Thread: %s", name.c_str() );

	/* Start a thread using our own startup function.  We pass the id to fill in,
	 * to make sure it's set before the thread actually starts.  (Otherwise, early
	 * checkpoints might not have a completely set-up thread slot.) */
	m_pSlot->pImpl = MakeThread( fn, data, &m_pSlot->id );
}

RageThreadRegister::RageThreadRegister( const CString &sName )
{
	InitThreads();
	LockMut( GetThreadSlotsLock() );
	
	int iSlot = FindEmptyThreadSlot();
	
	m_pSlot = &g_ThreadSlots[iSlot];
	
	strcpy( m_pSlot->name, sName );
	sprintf( m_pSlot->ThreadFormattedOutput, "Thread: %s", sName.c_str() );

	m_pSlot->id = GetThisThreadId();
	m_pSlot->pImpl = MakeThisThread();

}

RageThreadRegister::~RageThreadRegister()
{
	LockMut( GetThreadSlotsLock() );

	m_pSlot->Release();
	m_pSlot = NULL;
}

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

bool RageThread::EnumThreadIDs( int n, uint64_t &iID )
{
	if( n >= MAX_THREADS )
		return false;

	LockMut(GetThreadSlotsLock());
	const ThreadSlot *slot = &g_ThreadSlots[n];

	if( slot->used )
		iID = slot->id;
	else
		iID = GetInvalidThreadId();

	return true;
}

int RageThread::Wait()
{
	ASSERT( m_pSlot != NULL );
	ASSERT( m_pSlot->pImpl != NULL );
	int ret = m_pSlot->pImpl->Wait();

	LockMut( GetThreadSlotsLock() );

	m_pSlot->Release();
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
		if( ThisThreadID == g_ThreadSlots[entry].id || g_ThreadSlots[entry].pImpl == NULL )
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
		if( ThisThreadID == g_ThreadSlots[entry].id || g_ThreadSlots[entry].pImpl == NULL )
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
		sm_crash( "GetUnknownThreadSlot() returned NULL" );
	
	/* Ignore everything before the first "src/". */
	file = strstr( file, "src/" ) ?: file;
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
	ThreadSlot &slot = g_ThreadSlots[slotno];
	if( !slot.used )
		return NULL;

	/* Only show the "Unknown thread" entry if it has at least one checkpoint. */
	if( &slot == g_pUnknownThreadSlot && slot.GetFormattedCheckpoint(0) == NULL )
		return NULL;

	if( lineno != 0 )
		return slot.GetFormattedCheckpoint( lineno-1 );

	slot.ThreadFormattedOutput[sizeof(slot.ThreadFormattedOutput)-1] = 0;
	return slot.ThreadFormattedOutput;
}

/* XXX: iSize check unimplemented */
void Checkpoints::GetLogs( char *pBuf, int iSize, const char *delim )
{
	pBuf[0] = 0;

	for( int slotno = 0; slotno < MAX_THREADS; ++slotno )
	{
		const char *buf = GetCheckpointLog( slotno, 0 );
		if( buf == NULL )
			continue;
		strcat( pBuf, buf );
		strcat( pBuf, delim );
		
		for( int line = 1; (buf = GetCheckpointLog(slotno, line)) != NULL; ++line )
		{
			strcat( pBuf, buf );
			strcat( pBuf, delim );
		}
	}	
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
	for( unsigned i = 0; i < g_MutexList->size(); ++i )
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
	uint64_t iThisThreadId = GetThisThreadId();
	if( m_LockedBy == iThisThreadId )
	{
		++m_LockCnt;
		return;
	}

	if( !m_pMutex->Lock() )
	{
		const ThreadSlot *ThisSlot = GetThreadSlotFromID( GetThisThreadId() );
		const ThreadSlot *OtherSlot = GetThreadSlotFromID( m_LockedBy );

		CString ThisSlotName = "(???" ")"; // stupid trigraph warnings
		CString OtherSlotName = "(???" ")"; // stupid trigraph warnings
		if( ThisSlot )
			ThisSlotName = ssprintf( "%s (%i)", ThisSlot->GetThreadName(), (int) ThisSlot->id );
		if( OtherSlot )
			OtherSlotName = ssprintf( "%s (%i)", OtherSlot->GetThreadName(), (int) OtherSlot->id );
		const CString sReason = ssprintf( "Thread deadlock on mutex %s between %s and %s",
			GetName().c_str(), ThisSlotName.c_str(), OtherSlotName.c_str() );

#if defined(CRASH_HANDLER)
		/* Don't leave GetThreadSlotsLock() locked when we call ForceCrashHandlerDeadlock. */
		GetThreadSlotsLock().Lock();
		uint64_t CrashHandle = OtherSlot? OtherSlot->id:0;
		GetThreadSlotsLock().Unlock();

		/* Pass the crash handle of the other thread, so it can backtrace that thread. */
		CrashHandler::ForceDeadlock( sReason, CrashHandle );
#else
		FAIL_M( sReason );
#endif
	}

	m_LockedBy = iThisThreadId;

	/* This has internal thread safety issues itself (eg. one thread may delete
	 * a mutex while another locks one); disable for now. */
//	MarkLockedMutex();
}

bool RageMutex::TryLock()
{
	if( m_LockedBy == GetThisThreadId() )
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

LockMutex::LockMutex( RageMutex &pMutex, const char *file_, int line_ ):
	mutex( pMutex ),
	file( file_ ),
	line( line_ ),
	locked_at( RageTimer::GetTimeSinceStart() )
{
	mutex.Lock();
	locked = true;
}

LockMutex::~LockMutex()
{
	if( locked )
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

RageEvent::RageEvent( CString name ):
	RageMutex( name )
{
	m_pEvent = MakeEvent( m_pMutex );
}

RageEvent::~RageEvent()
{
	delete m_pEvent;
}

/* For each of these calls, the mutex must be locked, and must not be locked recursively. */
bool RageEvent::Wait( RageTimer *pTimeout )
{
	ASSERT( IsLockedByThisThread() );
	ASSERT( m_LockCnt == 0 );

	/* A zero RageTimer also means no timeout. */
	if( pTimeout != NULL && pTimeout->IsZero() )
		pTimeout = NULL;
	bool bRet = m_pEvent->Wait( pTimeout );

	m_LockedBy = GetThisThreadId();
	return bRet;
}

void RageEvent::Signal()
{
	ASSERT( IsLockedByThisThread() );
	ASSERT( m_LockCnt == 0 );
	m_pEvent->Signal();
}

void RageEvent::Broadcast()
{
	ASSERT( IsLockedByThisThread() );
	ASSERT( m_LockCnt == 0 );
	m_pEvent->Broadcast();
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

void RageSemaphore::Wait( bool bFailOnTimeout )
{
retry:
	if( m_pSema->Wait() )
		return;

	if( !bFailOnTimeout || RageThread::GetIsShowingDialog() )
		goto retry;

	/* We waited too long.  We're probably deadlocked, though unlike mutexes, we can't
	 * tell which thread we're stuck on. */
	const ThreadSlot *ThisSlot = GetThreadSlotFromID( GetThisThreadId() );
	const CString sReason = ssprintf( "Semaphore timeout on mutex %s on thread %s",
		GetName().c_str(), ThisSlot? ThisSlot->GetThreadName(): "(???" ")" ); // stupid trigraph warnings
#if defined(CRASH_HANDLER)
	CrashHandler::ForceDeadlock( sReason, GetInvalidThreadId() );
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
