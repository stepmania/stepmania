/* for dladdr: */
#define __USE_GNU
#include "global.h"

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

#if defined(DARWIN)
#include "archutils/Darwin/Crash.h"
#endif

#if defined(HAVE_VERSION_INFO)
extern const unsigned version_num;
#endif

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


#if defined(BACKTRACE_LOOKUP_METHOD_DLADDR)
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

#elif defined(BACKTRACE_LOOKUP_METHOD_BACKTRACE_SYMBOLS)
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
#elif defined(BACKTRACE_LOOKUP_METHOD_ATOS)
void BacktraceNames::FromAddr( void *p )
{
    int fds[2];
    int out = fileno(stdout);
    pid_t pid;
    pid_t ppid = getppid(); /* Do this before fork()ing! */
    
    Offset = 0;
    Address = long(p);

    if (pipe(fds) != 0)
    {
        fprintf(stderr, "FromAddr pipe() failed: %s\n", strerror(errno));
        exit(1);
    }

    pid = fork();
    if (pid == -1)
    {
        fprintf(stderr, "FromAddr fork() failed: %s\n", strerror(errno));
        exit(1);
    }

    if (pid == 0)
    {
        close(fds[0]);
        for (int fd = 3; fd < 1024; ++fd)
            if (fd != fds[1])
                close(fd);
        dup2(fds[1], out);
        close(fds[1]);

        char *addy;
        asprintf(&addy, "0x%x", long(p));
        const char *p = ssprintf("%d", ppid);
        
        fprintf(stderr, "%s\n", p);
        if (execl("/usr/bin/atos", "/usr/bin/atos", "-p", p, addy, NULL))
            fprintf(stderr, "atos failed: %s\n", strerror(errno));
        free(addy);
        close(out);
        exit(0);
    }
    
    close(fds[1]);
    char f[BACKTRACE_MAX_SIZE];
    bzero(f, BACKTRACE_MAX_SIZE);
    int len = read(fds[0], f, BACKTRACE_MAX_SIZE);
    if (len == -1)
    {
        fprintf(stderr, "FromAddr read() failed: %s\n", strerror(errno));
        Symbol = "";
        File = "";
    }
    else
    {
#if 1
        fprintf(stderr, "um, crashed\n");
        CStringArray mangledAndFile;

        split(f, " ", mangledAndFile, true);
        fprintf(stderr, "split %d: %s\n", int(mangledAndFile.size()), f);
        if (mangledAndFile.size() > 0)
        {
            fprintf(stderr, "size>0\n");
            Symbol = mangledAndFile[0];
            if (Symbol[0] != '-' && Symbol[0] != '+')
            {
                if (Symbol[0] == '_')
                    Symbol = Symbol.substr(1);
                fprintf(stderr, "Symbol: %s\n", Symbol.c_str());
                if (mangledAndFile.size() > 3)
                {
                    fprintf(stderr, "size > 3\n");
                    File = mangledAndFile[3];
                    fprintf(stderr, "File: %s\n", File.c_str());
                    unsigned pos = File.find('(');
                    unsigned start = (pos == File.npos ? 0 : pos+1);
                    pos = File.rfind(')') - 1;
                    fprintf(stderr, "start/end: %u/%u\n", start, pos);
                    File = File.substr(start, pos);
                }
                else if (mangledAndFile.size() == 3)
                {
                    fprintf(stderr, "size==3\n");
                    File = mangledAndFile[2].substr(0, mangledAndFile[2].rfind(')'));
                }
                else
                    File = "";
            }
            else
            {
                Symbol = mangledAndFile[0] + " " + mangledAndFile[1];
                if (mangledAndFile.size() == 3)
                {
                    File = mangledAndFile[2];
                    fprintf(stderr, "File: %s\n", File.c_str());
                    unsigned pos = File.find('(');
                    unsigned start = (pos == File.npos ? 0 : pos+1);
                    pos = File.rfind(')') - 1;
                    fprintf(stderr, "start/end: %u/%u\n", start, pos);
                    File = File.substr(start, pos);
                }
                 else
                     File = "";
            }
        }
        else
        {
            Symbol = "";
            File = "";
        }
        fprintf(stderr, "File: %s\n", File.c_str());
#else
        Symbol = f;
        File = "";
#endif
    }
}
#else
#error BACKTRACE_LOOKUP_METHOD_* required
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
    ret = read(3, BacktracePointers, sizeof(void *)*BACKTRACE_MAX_SIZE);

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

    fprintf(CrashDump, "StepMania crash report");
#if defined(HAVE_VERSION_INFO)
    fprintf(CrashDump, " -- build %u", version_num);
#endif
    fprintf(CrashDump, "\n--------------------------------------\n");
    fprintf(CrashDump, "\n");

    CString Signal;
#if !defined(BACKTRACE_METHOD_POWERPC_DARWIN)
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
#else
#define X(code) case k##code: Signal = #code; break;
    switch (SignalReceived)
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
        default: Signal = ssprintf("Unknown Exception %i", SignalReceived); break;
    }
#endif
    
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

#if defined(DARWIN)
    InformUserOfCrash();
#else
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
#endif

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


