#include "global.h"
#include "Backtrace.h"
#include "RageUtil.h"

#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/ptrace.h>
#include <sys/stat.h>
#include <errno.h>
#include <fcntl.h>

#if defined(BACKTRACE_METHOD_X86_LINUX)
#include "LinuxThreadHelpers.h"

void GetSignalBacktraceContext( BacktraceContext *ctx, const ucontext_t *uc )
{
	ctx->esp = (long) uc->uc_mcontext.gregs[REG_ESP];
	ctx->eip = (long) uc->uc_mcontext.gregs[REG_EIP];
	ctx->ebp = (long) uc->uc_mcontext.gregs[REG_EBP];
	ctx->pid = GetCurrentThreadId();
}

void GetCurrentBacktraceContext( BacktraceContext *ctx )
{
	register void *esp __asm__ ("esp");
	ctx->esp = (long) esp;
	ctx->eip = (long) GetThreadBacktraceContext;
	ctx->ebp = (long) __builtin_frame_address(0);
}

int GetThreadBacktraceContext( int ThreadID, BacktraceContext *ctx )
{
	ctx->pid = ThreadID;

	if( ThreadID != GetCurrentThreadId() )
		return GetThreadContext( ThreadID, ctx );

	GetCurrentBacktraceContext( ctx );

	return 0;
}

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

enum { READABLE_ONLY=1, EXECUTABLE_ONLY=2 };
static int get_readable_ranges( const void **starts, const void **ends, int size, int type=READABLE_ONLY )
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
	while( !eof && got < size-1 )
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

			char line[1024];
			strcpy( line, file );
			memmove(file, p, file_used);
			file_used -= p-file;

			/* Search for the hyphen. */
			char *hyphen = strchr( line, '-' );
			if( hyphen == NULL )
				continue; /* Parse error. */


			/* Search for the space. */
			char *space = strchr( hyphen, ' ' );
			if( space == NULL )
				continue; /* Parse error. */

			/* " rwxp".  If space[1] isn't 'r', then the block isn't readable. */
			if( type & READABLE_ONLY )
				if( strlen(space) < 2 || space[1] != 'r' )
					continue;
			/* " rwxp".  If space[3] isn't 'x', then the block isn't readable. */
			if( type & EXECUTABLE_ONLY )
				if( strlen(space) < 4 || space[3] != 'x' )
					continue;

			*starts++ = (const void *) xtoi( line );
			*ends++ = (const void *) xtoi( hyphen+1 );
			++got;
		}

		if( file_used == sizeof(file) )
		{
			/* Line longer than the buffer.  Weird; bail. */
			break;
		}
	}

	close(fd);

	*starts++ = NULL;
	*ends++ = NULL;

	return got;
}

/* If the address is readable (eg. reading it won't cause a segfault), return
 * the block it's in.  Otherwise, return -1. */
static int find_address( const void *p, const void **starts, const void **ends )
{
	for( int i = 0; starts[i]; ++i )
	{
		/* Found it. */
		if( starts[i] <= p && p < ends[i] )
			return i;
	}

	return -1;
}

static void *SavedStackPointer = NULL;

void InitializeBacktrace()
{
	static bool bInitialized = false;
	if( bInitialized )
		return;
	bInitialized = true;

	/* We might have a different stack in the signal handler.  Record a pointer
	 * that lies in the real stack, so we can look it up later. */
	register void *esp __asm__ ("esp");
	SavedStackPointer = esp;
}

/* backtrace() for x86 Linux, tested with kernel 2.4.18, glibc 2.3.1. */
static void do_backtrace( const void **buf, size_t size, const BacktraceContext *ctx )
{
	/* Read /proc/pid/maps to find the address range of the stack. */
	const void *readable_begin[1024], *readable_end[1024];
	get_readable_ranges( readable_begin, readable_end, 1024 );
		
	/* Find the stack memory blocks. */
	const int stack_block1 = find_address( (void *) ctx->esp, readable_begin, readable_end );
	const int stack_block2 = find_address( SavedStackPointer, readable_begin, readable_end );

	/* This matches the layout of the stack.  The frame pointer makes the
	 * stack a linked list. */
	struct StackFrame
	{
		StackFrame *link;
		char *return_address;
	};

	StackFrame *frame = (StackFrame *) ctx->ebp;

	unsigned i=0;
	if( i < size-1 ) // -1 for NULL
		buf[i++] = (void *) ctx->eip;

	while( i < size-1 ) // -1 for NULL
	{
		/* Make sure that this frame address is readable, and is on the stack. */
		int val = find_address(frame, readable_begin, readable_end);
		if( val == -1 )
			break;
		if( val != stack_block1 && val != stack_block2 )
			break;

		/* XXX */
		//if( frame->return_address == (void*) CrashSignalHandler )
		//	continue;

		if( frame->return_address )
			buf[i++] = frame->return_address;

		/* frame always goes up.  Make sure it doesn't go down; that could
		 * cause an infinite loop. */
		if( frame->link <= frame )
			break;

		frame = frame->link;
	}

	buf[i] = NULL;
}

void GetBacktrace( const void **buf, size_t size, const BacktraceContext *ctx )
{
	InitializeBacktrace();
	
	BacktraceContext CurrentCtx;
	if( ctx == NULL )
	{
		ctx = &CurrentCtx;
		GetCurrentBacktraceContext( &CurrentCtx );
	}


	do_backtrace( buf, size, ctx );
}
#elif defined(BACKTRACE_METHOD_BACKTRACE)
#include <execinfo.h>

void InitializeBacktrace() { }
	
void GetBacktrace( const void **buf, size_t size, const BacktraceContext *ctx )
{
	InitializeBacktrace();

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
typedef struct Frame
{
    Frame *stackPointer;
    long conditionReg;
    void *linkReg;
} *FramePtr;

void InitializeBacktrace() { }

void GetBacktrace( const void **buf, size_t size, const BacktraceContext *ctx )
{
	Frame *frame = (Frame *) GetCrashedFramePtr();

	unsigned i = 0;
	while( frame && i < size-1 ) // -1 for NULL
	{
		if( frame->linkReg )
			buf[i++] = frame->linkReg;
		
		frame = frame->stackPointer;
	}

	buf[i] = NULL;
}

#else

#warning Undefined BACKTRACE_METHOD_*
void InitializeBacktrace() { }

void GetBacktrace( const void **buf, size_t size, const BacktraceContext *ctx )
{
    buf[0] = BACKTRACE_METHOD_NOT_AVAILABLE;
    buf[1] = NULL;
}

#endif

