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
	SavedStackPointer = __builtin_frame_address(0);
}

/* backtrace() for x86 Linux, tested with kernel 2.4.18, glibc 2.3.1. */
const void *g_ReadableBegin[1024], *g_ReadableEnd[1024];
const void *g_ExecutableBegin[1024], *g_ExecutableEnd[1024];
/* Indexes in g_ReadableBegin of the stack(s), or -1: */
int g_StackBlock1, g_StackBlock2;

/* This matches the layout of the stack.  The frame pointer makes the
 * stack a linked list. */
struct StackFrame
{
	const StackFrame *link;
	const void *return_address;
};

/* Return true if the given stack pointer is in readable memory. */
bool IsOnStack( const void *p )
{
	int val = find_address( p, g_ReadableBegin, g_ReadableEnd );
	return val != -1 && (val == g_StackBlock1 || val == g_StackBlock2 );
}

/* Return true if the given stack frame is in readable memory. */
bool IsReadableFrame( const StackFrame *frame )
{
	if( !IsOnStack( &frame->link ) )
		return false;
	if( !IsOnStack( &frame->return_address ) )
		return false;
	return true;
}

/* The following from VirtualDub: */
/* ptr points to a return address, and does not have to be word-aligned. */
static bool PointsToValidCall( const void *ptr )
{
	const char *buf = (char *) ptr;

	/* We're reading buf backwards, between buf[-7] and buf[-1].  Find out how
	 * far we can read. */
	int len = 7;
	while( len )
	{
		int val = find_address( buf-len, g_ReadableBegin, g_ReadableEnd );
		if( val != -1 )
			break;
		--len;
	}

	// Permissible CALL sequences that we care about:
	//
	//	E8 xx xx xx xx			CALL near relative
	//	FF (group 2)			CALL near absolute indirect
	//
	// Minimum sequence is 2 bytes (call eax).
	// Maximum sequence is 7 bytes (call dword ptr [eax+disp32]).

	if (len >= 5 && buf[-5] == '\xe8')
		return true;

	// FF 14 xx					CALL [reg32+reg32*scale]
	if (len >= 3 && buf[-3] == '\xff' && buf[-2]=='\x14')
		return true;

	// FF 15 xx xx xx xx		CALL disp32
	if (len >= 6 && buf[-6] == '\xff' && buf[-5]=='\x15')
		return true;

	// FF 00-3F(!14/15)			CALL [reg32]
	if (len >= 2 && buf[-2] == '\xff' && (unsigned char)buf[-1] < '\x40')
		return true;

	// FF D0-D7					CALL reg32
	if (len >= 2 && buf[-2] == '\xff' && (buf[-1]&0xF8) == '\xd0')
		return true;

	// FF 50-57 xx				CALL [reg32+reg32*scale+disp8]
	if (len >= 3 && buf[-3] == '\xff' && (buf[-2]&0xF8) == '\x50')
		return true;

	// FF 90-97 xx xx xx xx xx	CALL [reg32+reg32*scale+disp32]
	if (len >= 7 && buf[-7] == '\xff' && (buf[-6]&0xF8) == '\x90')
		return true;

	return false;
}


/* Return true if frame appears to be a legitimate, readable stack frame. */
bool IsValidFrame( const StackFrame *frame )
{
	if( !IsReadableFrame( frame ) )
		return false;

	/* The frame link should only go upwards. */
	if( frame->link <= frame )
		return false;

	/* The link should be on the stack. */
	if( !IsOnStack( frame->link ) )
		return false;

	/* The return address should be in a readable, executable page. */
	int val = find_address( frame->return_address, g_ExecutableBegin, g_ExecutableEnd );
	if( val == -1 )
		return false;

	/* The return address should follow a CALL opcode. */
	if( !PointsToValidCall(frame->return_address) )
		return false;

	return true;
}

/* This x86 backtracer attempts to walk the stack frames.  If we come to a
 * place that doesn't look like a valid frame, we'll look forward and try
 * to find it again. */
static void do_backtrace( const void **buf, size_t size, const BacktraceContext *ctx )
{
	/* Read /proc/pid/maps to find the address range of the stack. */
	get_readable_ranges( g_ReadableBegin, g_ReadableEnd, 1024 );
	get_readable_ranges( g_ExecutableBegin, g_ExecutableEnd, 1024, READABLE_ONLY|EXECUTABLE_ONLY );

	/* Find the stack memory blocks. */
	g_StackBlock1 = find_address( ctx->esp, g_ReadableBegin, g_ReadableEnd );
	g_StackBlock2 = find_address( SavedStackPointer, g_ReadableBegin, g_ReadableEnd );

	/* If eip is valid, put it at the top of the backtrace. */
	unsigned i=0;
	if( i < size-1 && ctx->eip ) // -1 for NULL
		buf[i++] = ctx->eip;

	/* ebp is usually the frame pointer. */
	const StackFrame *frame = (StackFrame *) ctx->ebp;

	/* If ebp doesn't point to a valid stack frame, we're probably in
	 * -fomit-frame-pointer code.  Ignore it; use esp instead.  It probably
	 * won't * point to a stack frame, but it should at least give us a starting
	 * point in the stack. */
	if( !IsValidFrame( frame ) )
		frame = (StackFrame *) ctx->esp;

	while( i < size-1 ) // -1 for NULL
	{
		/* Make sure that this frame address is readable, and is on the stack. */
		if( !IsReadableFrame( frame ) )
			break;

		if( !IsValidFrame( frame ) )
		{
			/* The frame pointer is invalid.  Just move forward one word. */
			frame = (StackFrame *) (((char *)frame)+4);
			continue;
		}

		/* Valid frame.  Store the return address, and hop forward. */
		buf[i++] = frame->return_address;
		frame = frame->link;
	}

	buf[i] = NULL;
}

void GetSignalBacktraceContext( BacktraceContext *ctx, const ucontext_t *uc )
{
	ctx->eip = (void *) uc->uc_mcontext.gregs[REG_EIP];
	ctx->ebp = (void *) uc->uc_mcontext.gregs[REG_EBP];
	ctx->esp = (void *) uc->uc_mcontext.gregs[REG_ESP];
	ctx->pid = GetCurrentThreadId();
}

void GetBacktrace( const void **buf, size_t size, const BacktraceContext *ctx )
{
	InitializeBacktrace();
	
	BacktraceContext CurrentCtx;
	if( ctx == NULL )
	{
		ctx = &CurrentCtx;

		CurrentCtx.eip = NULL;
		CurrentCtx.ebp = __builtin_frame_address(0);
		CurrentCtx.esp = __builtin_frame_address(0);
		CurrentCtx.pid = GetCurrentThreadId();
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
typedef struct Frame
{
    Frame *stackPointer;
    long conditionReg;
    void *linkReg;
} *FramePtr;

void GetExceptionBacktraceContext( BacktraceContext *ctx, const ExceptionInformation *exception )
{
	ctx->FramePtr = (void *) exception->registerImage->R1.lo;
}

void InitializeBacktrace() { }

void GetBacktrace( const void **buf, size_t size, const BacktraceContext *ctx )
{
	BacktraceContext CurrentCtx;
	if( ctx == NULL )
	{
		ctx = &CurrentCtx;

		/* __builtin_frame_address is broken on OS X; it sometimes returns bogus results. */
		register void *r1 __asm__ ("r1");
		CurrentCtx.FramePtr = (void *) r1;
	}
	
	const Frame *frame = (Frame *) ctx->FramePtr;

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

