/* RageThreads helpers for threads in Linux, which are based on PIDs and TIDs. */

#include "global.h"
#include "LinuxThreadHelpers.h"
#include "RageUtil.h"

#include "Backtrace.h"

#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/ptrace.h>
#include <sys/stat.h>
#include <linux/unistd.h>
#define _LINUX_PTRACE_H // hack to prevent broken linux/ptrace.h from conflicting with sys/ptrace.h
#include <linux/user.h>

/* In Linux, we might be using PID-based or TID-based threads.  With PID-based
 * threads, getpid() returns a unique value for each thread; each thread is a
 * separate process.  Newer kernels using NPTL return the same PID for all
 * threads; these systems support a gettid() call to get a unique TID for each
 * thread. */



static bool g_bUsingNPTL = false;

static _syscall0(pid_t,gettid)

static bool UsingNPTL()
{
	return ThreadsVersion().Left(4) == "NPTL";
}

void InitializePidThreadHelpers()
{
	bool bInitialized = false;
	if( bInitialized )
		return;
	bInitialized = true;

	g_bUsingNPTL = UsingNPTL();
}

/* waitpid(); ThreadID can be a PID or (in NPTL) a TID; doesn't care if the ID
 * is a clone() or not. */
static int waittid( int ThreadID, int *status, int options )
{
	static bool bSupportsWall = true;

	if( bSupportsWall )
	{
		int ret = waitpid( ThreadID, status, options | __WALL );
		if( ret != -1 || errno != EINVAL )
			return ret;
		bSupportsWall = false;
	}
			
	/* XXX: on 2.2, we need to use __WCLONE only if ThreadID isn't the main thread;
	 * perhaps wait and retry without it if errno == ECHILD? */
	int ret;
	ret = waitpid( ThreadID, status, options );

	return ret;
}

/* Attempt to PTRACE_ATTACH to a thread, and wait for the SIGSTOP. */
static int PtraceAttach( int ThreadID )
{
	int ret;
	ret = ptrace( PTRACE_ATTACH, ThreadID, NULL, NULL );
	if( ret == -1 )
	{
		printf("ptrace failed: %s\n", strerror(errno) );
		return -1;
	}

	/* Wait for the SIGSTOP from the ptrace call. */
	int status;
	ret = waittid( ThreadID, &status, 0 );
	if( ret == -1 )
		return -1;

//	printf( "ret %i, exited %i, signalled %i, sig %i, stopped %i, stopsig %i\n", ret, WIFEXITED(status),
//			WIFSIGNALED(status), WTERMSIG(status), WIFSTOPPED(status), WSTOPSIG(status));
	return 0;
}

static int PtraceDetach( int ThreadID )
{
	return ptrace( PTRACE_DETACH, ThreadID, NULL, NULL );
}


CString ThreadsVersion()
{
// Not all systems have a header that declares _CS_GNU_LIBPTHREAD_VERSION.
// Deal with it.
#if defined(_CS_GNU_LIBPTHREAD_VERSION)
	char buf[1024];
	int ret = confstr( _CS_GNU_LIBPTHREAD_VERSION, buf, sizeof(buf) );
	RAGE_ASSERT_M( ret != -1, ssprintf( "%i", ret) );
	return buf;
#else
	return "(unknown)";
#endif
}

/* Get this thread's ID (this may be a TID or a PID). */
int GetCurrentThreadId()
{
	InitializePidThreadHelpers(); // for g_bUsingNPTL

	pid_t ret = gettid();

	/* If this fails with ENOSYS, we're on a kernel before gettid.  If we
	 * don't have NPTL, then just use getpid().  If we're on NPTL and don't
	 * have gettid(), something's wrong. */
	if( ret != -1 )
		return ret;

	ASSERT( !g_bUsingNPTL );

	return getpid();
}

int SuspendThread( int ThreadID )
{
	/*
	 * Linux: We can't simply kill(SIGSTOP) (or tkill), since that will stop all processes
	 * (grr).  We can ptrace(PTRACE_ATTACH) the process to stop it, and PTRACE_DETACH
	 * to restart it.
	 */
	return PtraceAttach( ThreadID );
	// kill( ThreadID, SIGSTOP );
}

int ResumeThread( int ThreadID )
{
	return PtraceDetach( ThreadID );
	// kill( ThreadID, SIGSTOP );
}


/* Get a BacktraceContext for a thread.  ThreadID must not be the current thread.
 *
 * tid() is a PID (from getpid) or a TID (from gettid).  Note that this may have kernel compatibility
 * problems, because NPTL is new and its interactions with ptrace() aren't well-defined.
 * If we're on a non-NPTL system, tid is a regular PID.
 *
 * This call leaves the given thread suspended, so the returned context doesn't become invalid.
 * ResumeThread() can be used to resume a thread after this call. */
bool GetThreadBacktraceContext( int ThreadID, BacktraceContext *ctx )
{
	/* Can't GetThreadBacktraceContext the current thread. */
	ASSERT( ThreadID != GetCurrentThreadId() );

	/* Attach to the thread.  This may fail with EPERM.  This can happen for at least
	 * two common reasons: the process might be in a debugger already, or *we* might
	 * already have attached to it via SuspendThread.
	 *
	 * If it's in a debugger, we won't be able to ptrace(PTRACE_GETREGS). If
	 * it's us that attached, we will. */
	if( PtraceAttach( ThreadID ) == -1 )
	{
		if( errno != EPERM )
		{
			CHECKPOINT_M( ssprintf( "%s (pid %i tid %i locking tid %i)", strerror(errno), getpid(), GetCurrentThreadId(), ThreadID ) );
			return false;
		}
	}

	user_regs_struct regs;
	if( ptrace( PTRACE_GETREGS, ThreadID, NULL, &regs ) == -1 )
		return false;

	ctx->pid = ThreadID;
	ctx->eip = (void *) regs.eip;
	ctx->ebp = (void *) regs.ebp;
	ctx->esp = (void *) regs.esp;

	return true;
}
