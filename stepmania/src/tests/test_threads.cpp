#include "global.h"
#include "RageLog.h"
#include "RageUtil.h"
#include "RageThreads.h"
#include "archutils/Unix/Backtrace.h"
#include "archutils/Unix/BacktraceNames.h"
#include "test_misc.h"

#include <unistd.h>
#include "archutils/Unix/LinuxThreadHelpers.h"

/* These are volatile, so writes to them aren't optimized. */
volatile int g_ThreadId = -1;
volatile int g_Counter = 0;
volatile bool g_Finish = false;

void TestSuspendIncrLoop()
{
	while( !g_Finish )
		++g_Counter;
}

int TestSuspendThread( void *p )
{
	printf("Test thread started\n");

	g_ThreadId = GetCurrentThreadId();
	TestSuspendIncrLoop();

	return 0;
}

void test_suspend_threadid( int ThreadId )
{
	/* Wait for g_Counter to increment a bit. */
	usleep( 100000 );
	       
	/* Stop the thread. */
	SuspendThread( ThreadId );

	int OldCounter = g_Counter;

	/* Wait a while.  g_Counter shouldn't change. */
	usleep( 100000 );

	ASSERT( g_Counter == OldCounter );

	/* Start it again, and wait. */
	ResumeThread( ThreadId );
	usleep( 100000 );

	/* g_Counter should change. */
	ASSERT( g_Counter != OldCounter );

	/* Stop all other threads. */
	RageThread::HaltAllThreads();

	OldCounter = g_Counter;

	/* Wait a while.  g_Counter shouldn't change. */
	usleep( 100000 );
	ASSERT( g_Counter == OldCounter );

	/* Start it again, and wait. */
	RageThread::ResumeAllThreads();
	usleep( 100000 );

	/* g_Counter should change. */
	ASSERT( g_Counter != OldCounter );
}

/* Test whether suspending a child thread works. */
void test_suspend_secondary_thread()
{
	ASSERT( !g_Finish );
	ASSERT( g_ThreadId == -1 );

	RageThread testing;
	testing.SetName( "TestSuspend" );
	testing.Create( TestSuspendThread, NULL );

	while( g_ThreadId == -1 )
		;

	test_suspend_threadid( g_ThreadId );
	
	g_Finish = true;
	testing.Wait();
	g_Finish = false;
	g_ThreadId = -1;
}

int TestSuspendMainThread( void *p )
{
	ASSERT( !g_Finish );

	printf("Test thread started\n");

	ASSERT( g_ThreadId != -1 );
	test_suspend_threadid( g_ThreadId );
	g_Finish = true;

	return 0;
}

/* Test whether suspending the main thread works. */
void test_suspend_main_thread()
{
	ASSERT( !g_Finish );
	ASSERT( g_ThreadId == -1 );

	g_ThreadId = GetCurrentThreadId();

	RageThread testing;
	testing.SetName( "TestSuspend" );
	testing.Create( TestSuspendMainThread, NULL );

	TestSuspendIncrLoop();

	testing.Wait();
	g_Finish = false;
	g_ThreadId = -1;
}

/* Run a second function, so we have two symbols to search for. */
void TestBacktraceThreadLoop() __attribute__ ((noinline));
void TestBacktraceThreadLoop()
{
	g_ThreadId = GetCurrentThreadId();
	while( !g_Finish )
		;
}

int TestBacktraceThread( void *p ) __attribute__ ((noinline));
int TestBacktraceThread( void *p )
{
	TestBacktraceThreadLoop();
	return 0;
}

bool test_thread_backtrace( int ThreadId, const void *expect1, const void *expect2 )
{
	BacktraceContext ctx;
	int ret = GetThreadBacktraceContext( ThreadId, &ctx );
	ASSERT( ret );
	
	const void *BacktracePointers[1024];
	GetBacktrace( BacktracePointers, 1024, &ctx );

	ResumeThread( ThreadId );

	bool bFound1 = false, bFound2 = false;
	for( int i = 0; BacktracePointers[i]; ++i)
	{
		BacktraceNames bn;
		bn.FromAddr( BacktracePointers[i] );

//		printf("want %p, %p: %p, %p\n", expect1, expect2, bn.Address-bn.Offset, BacktracePointers[i] );
//		printf("     %s\n", bn.Format().c_str() );

		/* bn.Address is the current address; Offset is the distance to the beginning
		 * of the symbol, so bn.Address-bn.Offset is the actual symbol. */
		if( bn.Address-bn.Offset == (int) expect1 )
			bFound1 = true;
		if( bn.Address-bn.Offset == (int) expect2 )
			bFound2 = true;
	}

	return bFound1 && bFound2;
}

void test_backtracing_secondary_thread()
{
	ASSERT( !g_Finish );

	RageThread testing;
	testing.SetName( "TestBacktrace" );
	testing.Create( TestBacktraceThread, NULL );

	while( g_ThreadId == -1 )
		;
	
	if( !test_thread_backtrace( g_ThreadId, (void *) TestBacktraceThread, (void *) TestBacktraceThreadLoop ) )
	{
		printf( "test_backtracing_secondary_thread failed\n" );
		exit( 1 );
	}

	g_Finish = true;
	testing.Wait();
	g_Finish = false;
	g_ThreadId = -1;
}

int TestBacktraceMainThread( void *p )
{
	printf("Test thread started\n");

	while( g_ThreadId == -1 )
		;
	
	if( !test_thread_backtrace( g_ThreadId, (void *) TestBacktraceThread, (void *) TestBacktraceThreadLoop ) )
	{
		printf( "test_backtracing_main_thread failed\n" );
		exit( 1 );
	}

	g_Finish = true;

	return 0;
}

void test_backtracing_main_thread()
{
	ASSERT( !g_Finish );
	ASSERT( g_ThreadId == -1 );

	RageThread testing;
	testing.SetName( "TestBacktrace" );
	testing.Create( TestBacktraceMainThread, NULL );

	TestBacktraceThread( NULL );

	testing.Wait();
	g_Finish = false;
	g_ThreadId = -1;
}

static RageMutex g_Mutex("test");

int TestLocksThread( void *p )
{
	printf("Test thread started\n");

	g_ThreadId = GetCurrentThreadId();

	while( !g_Finish )
	{
		g_Mutex.Lock();
		++g_Counter;
		g_Mutex.Unlock();
	}

	return 0;
}

void test_locks()
{
	ASSERT( !g_Finish );
	ASSERT( g_ThreadId == -1 );

	RageThread testing;
	testing.SetName( "TestLocks" );
	testing.Create( TestLocksThread, NULL );

	while( g_ThreadId == -1 )
		;

	/* Stop the thread. */
	g_Mutex.Lock();

	int OldCounter = g_Counter;

	/* Wait a while.  g_Counter shouldn't change. */
	usleep( 100000 );

	ASSERT( g_Counter == OldCounter );

	/* Start it again, and wait. */
	g_Mutex.Unlock();
	usleep( 100000 );

	/* g_Counter should change. */
	ASSERT( g_Counter != OldCounter );

	g_Finish = true;
	testing.Wait();
	g_Finish = false;
	g_ThreadId = -1;
}

void go()
{
	/* Test the main thread suspending a secondary thread, and vice versa. */
	test_suspend_secondary_thread();
	test_suspend_main_thread();
	
	/* Test the main thread backtracing a secondary thread, and vice versa. */
	test_backtracing_secondary_thread();
	test_backtracing_main_thread();

	test_locks();
}

int main( int argc, char *argv[] )
{
	test_handle_args( argc, argv );

	test_init();

	InitializeBacktrace();

	printf("'%s'\n", ThreadsVersion().c_str());

	go();

	test_deinit();

	exit(0);
}

