/* for dladdr: */
#define __USE_GNU
#include "global.h"

#include <execinfo.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

#include <sys/types.h>
#include <sys/wait.h>

#include "StepMania.h" /* for g_argv */
#include "RageUtil.h"
#include "CrashHandler.h"
#include "CrashHandlerInternal.h"
#include "RageLog.h" /* for RageLog::GetAdditionalLog, etc. only */

struct BacktraceNames
{
	CString Symbol, File;
	int Address;
	int Offset;
	void FromAddr( void *p );
	void FromString( CString str );
	void Demangle();
	CString Format() const;
};

#if defined(HAVE_LIBIBERTY)
#include "libiberty.h"

/* This is in libiberty. Where is it declared? */
extern "C" {
char *cplus_demangle (const char *mangled, int options);
}

void BacktraceNames::Demangle()
{
	char *f = cplus_demangle(Symbol, 0);
	if(!f)
		return;
	Symbol = f;
	free(f);
}
#else
void BacktraceNames::Demangle() { }
#endif


CString BacktraceNames::Format() const
{
	CString ShortenedPath = File;
	if( Symbol != "" )
	{
		/* We have some sort of symbol name, so abbreviate or elide the module name. */
		if( ShortenedPath == g_argv[0] )
			ShortenedPath = "";
		else
		{
			unsigned slash = ShortenedPath.rfind('/');
			if( slash != ShortenedPath.npos )
				ShortenedPath = ShortenedPath.substr(slash+1);
			ShortenedPath = CString("(") + ShortenedPath + ")";
		}
	}

	CString ret = ssprintf( "%08x: ", Address );
	if( Symbol != "" )
	ret += Symbol + " ";
	ret += ShortenedPath;
	
	return ret;
}


#if BACKTRACE_LOOKUP_METHOD == DLADDR
/* This version simply asks libdl, which is more robust. */
#include <dlfcn.h>
void BacktraceNames::FromAddr( void *p )
{
	Dl_info di;
	if( !dladdr(p, &di) ) 
		return;

	Symbol = di.dli_sname;
	File = di.dli_fname;
	Address = (int) p;
	Offset = (char*)(p)-(char*)di.dli_saddr;
}

#elif BACKTRACE_LOOKUP_METHOD == BACKTRACE_SYMBOLS
/* This version parses backtrace_symbols(), an doesn't need libdl. */ 
#include <execinfo.h>
void BacktraceNames::FromAddr( void *p )
{
	char **foo = backtrace_symbols(&p, 1);
	if( foo == NULL )
		return;
	FromString( foo[0] );
	Address = (int) p;
	free(foo);
}

/* "path(mangled name+offset) [address]" */
void BacktraceNames::FromString( CString s )
{
	/* Hacky parser.  I don't want to use regexes in the crash handler. */
	CString MangledAndOffset, sAddress;
	unsigned pos = 0;
	while( pos < s.size() && s[pos] != '(' && s[pos] != '[' )
		File += s[pos++];
	TrimRight( File );	
	TrimLeft( File );	

	if( pos < s.size() && s[pos] == '(' )
	{
		pos++;
		while( pos < s.size() && s[pos] != ')' )
			MangledAndOffset += s[pos++];
	}

	if( MangledAndOffset != "" )
	{
		unsigned plus = MangledAndOffset.rfind('+');

		if(plus == MangledAndOffset.npos)
		{
			Symbol = MangledAndOffset;
			Offset = 0;
		}
		else
		{
			Symbol = MangledAndOffset.substr(0, plus);
			CString str = MangledAndOffset.substr(plus);
			if( sscanf(str, "%i", &Offset) != 1 )
				Offset=0;
		}
	}
}
#else
#error BACKTRACE_LOOKUP_METHOD required
#endif


static void output_stack_trace( FILE *out, void **BacktracePointers )
{
	for( int i = 0; BacktracePointers[i]; ++i)
	{
		BacktraceNames bn;
		bn.FromAddr( BacktracePointers[i] );
		bn.Demangle();

		if( bn.Symbol == "__libc_start_main" )
			break;

		fprintf( out, "%s\n", bn.Format().c_str() );
	}
}

/* Once we get here, we should be * safe to do whatever we want;
 * heavyweights like malloc and CString are OK. (Don't crash!) */
static void child_process()
{
	int ret;
	
	/* 1. Read the backtrace pointers. */
	void *BacktracePointers[BACKTRACE_MAX_SIZE];
	ret = read(3, BacktracePointers, sizeof(BacktracePointers));

	/* 2. Read the signal. */
	int SignalReceived;
	ret = read(3, &SignalReceived, sizeof(SignalReceived));

	/* 3. Read info. */
	int size;
	ret = read(3, &size, sizeof(size));
	char *Info = new char [size];
	ret = read(3, Info, size);

	/* 4. Read AdditionalLog. */
	ret = read(3, &size, sizeof(size));
	char *AdditionalLog = new char [size];
	ret = read(3, AdditionalLog, size);

	/* 5. Read RecentLogs. */
	int cnt = 0;
	ret = read(3, &cnt, sizeof(cnt));
	char *Recent[1024];
	for( int i = 0; i < cnt; ++i )
	{
		ret = read(3, &size, sizeof(size));
		Recent[i] = new char [size];
		ret = read(3, Recent[i], size);
	}

	/* Wait for the child to either finish cleaning up or die.  XXX:
	 * This should time out, in case something deadlocks. */

	char x;
	ret = read(3, &x, sizeof(x));
	if( (ret == -1 && errno != EPIPE) || ret != 0 )
	{
		/* We expect an EOF or EPIPE.  What happened? */
		fprintf(stderr, "Unexpected child read() result: %i (%s)\n", ret, strerror(errno));
		/* keep going */
	}
 
	FILE *CrashDump = fopen("/tmp/crashinfo.txt", "w+");
	if(CrashDump == NULL)
	{
		fprintf(stderr, "Couldn't open crashinfo.txt: %s", strerror(errno));
		exit(1);
	}

	fprintf(CrashDump, "StepMania crash report\n");
	fprintf(CrashDump, "--------------------------------------\n");
	fprintf(CrashDump, "\n");

	CString Signal;

#define X(a) case a: Signal = #a; break;
	switch(SignalReceived)
	{
		case SIGALRM: Signal = "Alarm"; break;
		case SIGBUS: Signal = "Bus error"; break;
		case SIGFPE: Signal = "Floating point exception"; break;
		X(SIGHUP)
		case SIGILL: Signal = "Illegal instruction"; break;
		X(SIGINT) X(SIGIOT)
		case SIGPIPE: Signal = "Broken pipe"; break;
		X(SIGQUIT)
		case SIGSEGV: Signal = "Segmentation fault"; break;
		X(SIGTRAP) X(SIGTERM) X(SIGVTALRM) X(SIGXCPU) X(SIGXFSZ)
#if defined(HAVE_DECL_SIGPWR) && HAVE_DECL_SIGPWR
		X(SIGPWR)
#endif
		default: Signal = ssprintf("Unknown signal %i", SignalReceived); break;
	}

	fprintf( CrashDump, "Crash reason: %s\n\n", Signal.c_str() );

	output_stack_trace( CrashDump, BacktracePointers );
	fprintf(CrashDump, "\n");

	fprintf(CrashDump, "Static log:\n");
	fprintf(CrashDump, "%s", Info);
	fprintf(CrashDump, "%s", AdditionalLog);
	fprintf(CrashDump, "\nPartial log:\n");
	for( int i = 0; i < cnt; ++i )
		fprintf(CrashDump, "%s\n", Recent[i] );
	fprintf(CrashDump, "\n");
	fprintf(CrashDump, "-- End of report\n");
	fclose(CrashDump);

	fprintf(stderr, 
			"\n"
			"StepMania has crashed.  Debug information has been output to\n"
			"\n"
			"    /tmp/crashinfo.txt\n"
			"\n"
			"Please report a bug at:\n"
			"\n"
			"    http://sourceforge.net/tracker/?func=add&group_id=37892&atid=421366\n"
			"\n"
		   );


	/* Forcibly kill our parent. */
//	kill( getppid(), SIGKILL );
}


void CrashHandlerHandleArgs()
{
	if(g_argc == 2 && !strcmp(g_argv[1], CHILD_MAGIC_PARAMETER))
	{
		child_process();
		exit(0);
	}
}


