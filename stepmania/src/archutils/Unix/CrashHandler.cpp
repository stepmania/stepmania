#include "global.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <limits.h>
#include <fcntl.h>

#include "StepMania.h" /* for g_argv */

#include "RageLog.h" /* for RageLog::GetAdditionalLog, etc, only */
#include <RageThreads.h>

#include <sys/types.h>
#include <sys/wait.h>

#include "CrashHandler.h"
#include "CrashHandlerInternal.h"

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


static void spawn_child_process(int from_parent)
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

	execl(g_argv[0], g_argv[0], CHILD_MAGIC_PARAMETER, NULL);

	/* If we got here, the exec failed. */
	safe_print(fileno(stderr), "Crash handler execl(", g_argv[0], ") failed: ", strerror(errno), "\n", NULL);
	_exit(1);
}


static void parent_write(int to_child, const void *p, size_t size)
{
	size_t ret = write(to_child, p, size);
	if(ret != size)
	{
		safe_print(fileno(stderr), "Unexpected write() result (", strerror(errno), ")\n", NULL);
		exit(1);
	}
}

static void parent_process( int to_child, void **BacktracePointers, int SignalReceived )
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
    p = GetCheckpointLogs("\n");
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

#if defined(BACKTRACE_METHOD_X86_LINUX)
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

static int xtoi( const char *hex )
{
	int ret = 0;
	while( 1 )
	{
		int val = -1;
		if( *hex >= '0' && *hex <= '9' )
			val = *hex - '0';
		else if( *hex >= 'A' && *hex <= 'F' )
			val = *hex - 'A' + 10;
		else if( *hex >= 'a' && *hex <= 'f' )
			val = *hex - 'a' + 10;
		else
			break;
		hex++;

		ret *= 16;
		ret += val;
	}
	return ret;
}

static int get_readable_ranges( const void **starts, const void **ends, int size )
{
    char path[PATH_MAX] = "/proc/";
    strcat( path, itoa(getpid()) );
    strcat( path, "/maps" );

    int fd = open(path, O_RDONLY);
    if( fd == -1 )
		return false;

    /* Format:
     *
     * 402dd000-402de000 rw-p 00010000 03:03 16815669   /lib/libnsl-2.3.1.so
     * or
     * bfffb000-c0000000 rwxp ffffc000 00:00 0
     *
     * Look for the range that includes the stack pointer. */
    char file[1024];
	int file_used = 0;
    bool eof = false;
	int got = 0;
    while(!eof && got < size-1 )
    {
		int ret = read( fd, file+file_used, sizeof(file) - file_used);
		if( ret < int(sizeof(file)) - file_used)
			eof = true;

		file_used += ret;

		/* Parse lines. */
		while( got < size )
		{
			char *p = (char *) memchr( file, '\n', file_used );
			if( p == NULL )
				break;
			*p++ = 0;

			/* Search for the hyphen. */
			char *hyphen = strchr( file, '-' );
			if( hyphen == NULL )
			{
				/* Parse error. */
				continue;
			}


			/* Search for the space. */
			char *space = strchr( hyphen, ' ' );
			if( space == NULL )
			{
				/* Parse error. */
				continue;
			}

			/* " rwxp".  If space[1] isn't 'r', then the block isn't readable. */
			if( strlen(space) < 2 || space[1] != 'r' )
				continue;

			*starts++ = (const void *) xtoi( file );
			*ends++ = (const void *) xtoi( hyphen+1 );

			file_used -= p-file;
			memmove(file, p, file_used);
		}

		if( file_used == sizeof(file) )
		{
			/* Line longer than the buffer.  Weird; bail. */
			close(fd);
			return false;
		}
	}

	*starts++ = NULL;
	*ends++ = NULL;

	return got;
}

/* If the address is readable (eg. reading it won't cause a segfault), return
 * the block it's in.  Otherwise, return -1. */
static int find_address( void *p, const void **starts, const void **ends )
{
	for( int i = 0; starts[i]; ++i )
	{
		/* Found it. */
		if( starts[i] <= p && p <= ends[i] )
			return i;
	}

	return -1;
}

/* backtrace() for x86 Linux, tested with kernel 2.4.18, glibc 2.3.1. */
static void do_backtrace( void **buf, size_t size, bool ignore_before_sig = true )
{
	/* Make room for a NULL terminator. */
	--size;

	/* Read /proc/pid/maps to find the address range of the stack. */
    const void *readable_begin[1024], *readable_end[1024];
	get_readable_ranges( readable_begin, readable_end, 1024 );
		
	/* Find the stack memory block. */
	register void *esp __asm__ ("esp");
	int stack_block=find_address(esp, readable_begin, readable_end);

	/* This matches the layout of the stack.  The frame pointer makes the
	 * stack a linked list. */
	struct StackFrame
	{
		StackFrame *link;
		char *return_address;

		/* These are only relevant if the frame is a signal trampoline. */
		int signal;
		sigcontext sig;
	};

	StackFrame *frame = (StackFrame *) __builtin_frame_address(0);

	unsigned i=0;
	/* If ignore_before_sig is true, don't return stack frames before we find a signal trampoline. */
	bool got_signal_return = !ignore_before_sig;
	while( i < size )
	{
		/* Make sure that this frame address is readable, and is on the stack. */
		int val = find_address(frame, readable_begin, readable_end);
		if( val != stack_block )
			break;

		if( frame->return_address == (void*) CrashSignalHandler )
			continue;

		/*
		 * The stack return stub is:
		 *
		 * 0x401139d8 <sigaction+280>:     pop    %eax
		 * 0x401139d9 <sigaction+281>:     mov    $0x77,%eax
		 * 0x401139de <sigaction+286>:     int    $0x80
		 *
		 * This will be different if using realtime signals, as will the stack layout.
		 *
		 * If we detect this, it means this is a stack frame returning from a signal.
		 * Ignore the return_address and use the sigcontext instead.
		 */
		const char comp[] = { 0x58, 0xb8, 0x77, 0x0, 0x0, 0x0, 0xcd, 0x80, 0x55, 0x89 };
		bool is_signal_return = true;
		if( find_address(frame->return_address, readable_begin, readable_end) == -1)
			is_signal_return = false;

		for( unsigned pos = 0; is_signal_return && pos < sizeof(comp); ++pos )
			if(frame->return_address[pos] != comp[pos])
					is_signal_return = false;

		void *to_add = NULL;
		if( is_signal_return )
		{
			got_signal_return = true;
			to_add = (void *) frame->sig.eip;
		}
		/* Ignore stack frames before the signal. */
		else if( got_signal_return )
			to_add = frame->return_address;

		if( to_add )
			buf[i++] = to_add;

		/* frame always goes down.  Make sure it doesn't go up; that could
		 * cause an infinite loop. */
		if( frame->link <= frame )
			break;

		frame = frame->link;
	}

	buf[i] = NULL;
}
#elif defined(BACKTRACE_METHOD_BACKTRACE)
static void do_backtrace(void **buf, size_t size)
{
	int retsize = backtrace( buf, size-1 );

	/* Remove any NULL entries.  We want to null-terminate the list, and null entries are useless. */
	for( int i = retsize-1; i >= 0; --i )
	{
		if( buf[i] != NULL )
			continue;

		memmove( &buf[i], &buf[i]+1, retsize-i-1 );
	}

	buf[retsize] = NULL;
}
#elif defined(BACKTRACE_METHOD_POWERPC_DARWIN)
#include "archutils/Darwin/Crash.h"
typedef struct Frame {
    Frame *stackPointer;
    long conditionReg;
    void *linkReg;
} *FramePtr;

static void do_backtrace(void **buf, size_t size)
{
    FramePtr frame = FramePtr(GetCrashedFramePtr());
    unsigned i;
    for (i=0; frame && frame->linkReg && i<size; ++i, frame=frame->stackPointer)
        buf[i] = frame->linkReg;
    i = (i == size ? size - 1 : i);
    buf[i] = NULL;
}
#else
#warning Undefined BACKTRACE_METHOD_*
static void do_backtrace(void **buf, size_t size)
{
    buf[0] = BACKTRACE_METHOD_NOT_AVAILABLE;
    buf[1] = NULL;
}
#endif


void CrashSignalHandler( int signal )
{
#if !defined(BACKTRACE_METHOD_POWERPC_DARWIN)
	/* Don't dump a debug file if the user just hit ^C. */
	if( signal == SIGINT || signal == SIGTERM || signal == SIGHUP )
		return;
#endif

	static bool received = false;

	if( received )
	{
		/* We've received a second signal.  This may mean that another thread
		 * crashed before we stopped it, or it may mean that the crash handler
		 * crashed. */
		const char *str = "Oops! Fatal signal received while still in the crash handler\n";
		write( fileno(stdout), str, strlen(str) );
		_exit(1);
	}
	received = true;

	RageThread::HaltAllThreads();
	
	/* Do this early, so functions called below don't end up on the backtrace. */
	void *BacktracePointers[BACKTRACE_MAX_SIZE];
	do_backtrace(BacktracePointers, BACKTRACE_MAX_SIZE);
	
	/* We need to be very careful, since we're under crash conditions.  Let's fork
	 * a process and exec ourself to get a clean environment to work in. */
	//const char *my_fn = g_argv[0];

	int fds[2];
	if(pipe(fds) != 0)
	{
		safe_print(fileno(stderr), "Crash handler pipe() failed: ", strerror(errno), "\n", NULL);
		exit(1);
	}

	pid_t pid = fork();
	if(pid == -1)
	{
		safe_print(fileno(stderr), "Crash handler fork() failed: ", strerror(errno), "\n", NULL);
		exit(1);
	}

	if(pid == 0)
	{
		close(fds[1]);
		spawn_child_process(fds[0]);
	}
	else
	{
		close(fds[0]);
		parent_process( fds[1], BacktracePointers, signal );
		waitpid(pid, NULL, 0);
		/* Now that we're done, actually kill all threads. */
		RageThread::HaltAllThreads( true );
	}
}


