#include "global.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <limits.h>
#include <fcntl.h>
#include <signal.h>

#include "RageLog.h" /* for RageLog::GetAdditionalLog, etc, only */
#include "RageThreads.h"
#include "Backtrace.h"

#include <sys/types.h>
#include <sys/wait.h>

#include "CrashHandler.h"
#include "CrashHandlerInternal.h"

extern const char *g_pCrashHandlerArgv0;

static void safe_print(int fd, ...)
{
	va_list ap;
	va_start(ap, fd);

	while(1)
	{
		const char *p = va_arg(ap, const char *);
		if(p == NULL)
			break;
		write(fd, p, strlen(p));
	}
	va_end(ap);
}


static void NORETURN spawn_child_process( int from_parent )
{
	/* We need to re-exec ourself, to get a clean process.  Close all
	 * FDs except for 0-2 and to_child, and then assign to_child to 3. */
	for(int fd = 3; fd < 1024; ++fd)
		if(fd != from_parent) close(fd);
       
	if(from_parent != 3)
	{
		dup2(from_parent, 3);
		close(from_parent);
	}

	execl( g_pCrashHandlerArgv0, g_pCrashHandlerArgv0, CHILD_MAGIC_PARAMETER, NULL);

	/* If we got here, the exec failed. */
	safe_print(fileno(stderr), "Crash handler execl(", g_pCrashHandlerArgv0, ") failed: ", strerror(errno), "\n", NULL);
	_exit(1);
}

/* write(), but retry a couple times on EINTR. */
static int retried_write( int fd, const void *buf, size_t count )
{
	int tries = 3, ret;
	do
	{
		ret = write( fd, buf, count );
	}
	while( ret == -1 && errno == EINTR && tries-- );
	
	return ret;
}

static void parent_write(int to_child, const void *p, size_t size)
{
	size_t ret = retried_write(to_child, p, size);
	if(ret != size)
	{
		safe_print(fileno(stderr), "Unexpected write() result (", strerror(errno), ")\n", NULL);
		exit(1);
	}
}

static void parent_process( int to_child, const void **BacktracePointers, int SignalReceived )
{
	/* 1. Write the backtrace pointers. */
	parent_write(to_child, BacktracePointers, sizeof(void *)*BACKTRACE_MAX_SIZE);

	/* 2. Write the signal. */
	parent_write(to_child, &SignalReceived, sizeof(SignalReceived));

	/* 3. Write info. */
	const char *p = RageLog::GetInfo();
	int size = strlen(p)+1;
	parent_write(to_child, &size, sizeof(size));
	parent_write(to_child, p, size);

	/* 4. Write AdditionalLog. */
	p = RageLog::GetAdditionalLog();
	size = strlen(p)+1;
	parent_write(to_child, &size, sizeof(size));
	parent_write(to_child, p, size);
	
	/* 5. Write RecentLogs. */
	int cnt = 0;
	const char *ps[1024];
	while( cnt < 1024 && (ps[cnt] = RageLog::GetRecentLog( cnt )) != NULL )
		++cnt;

	parent_write(to_child, &cnt, sizeof(cnt));
	for( int i = 0; i < cnt; ++i )
	{
		size = strlen(ps[i])+1;
		parent_write(to_child, &size, sizeof(size));
		parent_write(to_child, ps[i], size);
	}

    /* 6. Write CHECKPOINTs. */
    p = Checkpoints::GetLogs("\n");
    size = strlen(p)+1;
    parent_write(to_child, &size, sizeof(size));
    parent_write(to_child, p, size);
	
    /* 7. Write the crashed thread's name. */
    p = RageThread::GetCurThreadName();
    size = strlen(p)+1;
    parent_write(to_child, &size, sizeof(size));
    parent_write(to_child, p, size);
	
	close(to_child);
}


/* The parent process is the crashed process.  It'll send data to the
 * child, who will do stuff with it.  The parent then waits for the
 * child to quit, and exits. 
 *
 * We can do whatever fancy things we want in the child process.  However,
 * let's not open any windows until we at least try to shut down OpenGL,
 * since it may cause problems.  We don't want to try to shut down OpenGL
 * until we've sent all of our data, since it might explode.
 *
 * So, first fork off the error reporting child, send data to it, shut down
 * OpenGL, close the socket and wait for the child to shut down.
 *
 * The child reads the data from the parent, waits for the socket to close
 * (EOF), and it's then free to open windows and stuff.
 *
 * XXX: make sure the parent dying doesn't take out the child
 */

/* The x86 backtrace() in glibc doesn't make any effort at all to decode
 * signal trampolines.  The result is that it doesn't properly show the
 * function that actually caused the signal--which is the most important
 * one!  So, we have to do it all ourself. */
static const char *itoa(unsigned n)
{
	static char ret[32];
	char *p = ret;
	for( int div = 1000000000; div > 0; div /= 10 )
	{
		*p++ = (n / div) + '0';
		n %= div;
	}
	*p = 0;
	p = ret;
	while( p[0] == '0' && p[1] )
		++p;
	return p;
}

const char *SignalName( int signo )
{
#if !defined(DARWIN)
#define X(a) case a: return #a;
	switch( signo )
	{
	case SIGALRM: return "Alarm";
	case SIGBUS: return "Bus error";
	case SIGFPE: return "Floating point exception";
	X(SIGHUP)
	case SIGILL: return "Illegal instruction";
	X(SIGINT)
	case SIGPIPE: return "Broken pipe";
	case SIGABRT: return "Aborted";
	X(SIGQUIT)
	case SIGSEGV: return "Segmentation fault";
	X(SIGTRAP) X(SIGTERM) X(SIGVTALRM) X(SIGXCPU) X(SIGXFSZ)
#if defined(HAVE_DECL_SIGPWR) && HAVE_DECL_SIGPWR
	X(SIGPWR)
#endif
	default:
	{
		static char buf[128];
		strcpy( buf, "Unknown signal " );
		strcat( buf, itoa(signo) );
		return buf;
	}
	}
#else
#define X(code) case k##code: return #code;
	switch( signo )
	{
	X(UnknownException)
	X(IllegalInstructionException)
	X(TrapException)
	X(AccessException)
	X(UnmappedMemoryException)
	X(ExcludedMemoryException)
	X(ReadOnlyMemoryException)
	X(UnresolvablePageFaultException)
	X(PrivilegeViolationException)
	X(TraceException)
	X(InstructionBreakpointException)
	X(DataBreakpointException)
	X(FloatingPointException)
	X(StackOverflowException)
	default:
	{
		static char buf[128];
		strcpy( buf, "Unknown exception " );
		strcat( buf, itoa(signo) );
		return buf;
	}
	}
#endif
}

void CrashSignalHandler( int signal, siginfo_t *si, const ucontext_t *uc )
{
	if( g_pCrashHandlerArgv0 == NULL )
	{
		safe_print(fileno(stderr), "Crash handler failed: CrashHandlerHandleArgs was not called\n", NULL);
		_exit(1);
	}
	
	/* Block SIGPIPE, so we get EPIPE. */
	struct sigaction sa;
	memset( &sa, 0, sizeof(sa) );
	sa.sa_handler = SIG_IGN;
	if( sigaction( SIGPIPE, &sa, NULL ) != 0 )
	{
		safe_print(fileno(stderr), "sigaction() failed: %s", strerror(errno), NULL);
		/* non-fatal */
	}
	       
	static int received = 0;
	static pid_t childpid = 0;

	if( received )
	{
		/* We've received a second signal.  This may mean that another thread
		 * crashed before we stopped it, or it may mean that the crash handler
		 * crashed. */
		const char *str;
		if( received == getpid() )
			safe_print( fileno(stderr), "Oops! Fatal signal (", SignalName(signal), ") received while still in the crash handler\n", NULL);
		else if( childpid == getpid() )
			safe_print( fileno(stderr), "Oops! Fatal signal (", SignalName(signal), ") received while in the crash handler child\n", NULL);
		else
			safe_print( fileno(stderr), "Extra fatal signal (", SignalName(signal), ") received\n", NULL);
		_exit(1);
	}
	received = getpid();

	/* We want to stop other threads when crashing.  However, sending SIGSTOPs is messy and
	 * tends to do more harm than good.  Let's just try to get the crashdump written quickly. */
	// RageThread::HaltAllThreads();
	
	/* Do this early, so functions called below don't end up on the backtrace. */
	BacktraceContext ctx;
	GetSignalBacktraceContext( &ctx, uc );
	const void *BacktracePointers[BACKTRACE_MAX_SIZE];
	GetBacktrace( BacktracePointers, BACKTRACE_MAX_SIZE, &ctx );
	
	/* We need to be very careful, since we're under crash conditions.  Let's fork
	 * a process and exec ourself to get a clean environment to work in. */
	int fds[2];
	if(pipe(fds) != 0)
	{
		safe_print(fileno(stderr), "Crash handler pipe() failed: ", strerror(errno), "\n", NULL);
		exit(1);
	}

	childpid = fork();
	if( childpid == -1 )
	{
		safe_print(fileno(stderr), "Crash handler fork() failed: ", strerror(errno), "\n", NULL);
		_exit(1);
	}

	if( childpid == 0 )
	{
		close(fds[1]);
		spawn_child_process(fds[0]);
	}
	else
	{
		close(fds[0]);
		parent_process( fds[1], BacktracePointers, signal );
		int status = 0;
		waitpid( childpid, &status, 0 );
		if( WIFSIGNALED(status) )
			safe_print( fileno(stderr), "Crash handler child exited with signal ", itoa(WTERMSIG(status)), "\n", NULL);
	}
}

void InitializeCrashHandler()
{
	InitializeBacktrace();
}

