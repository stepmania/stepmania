/* for dladdr: */
#define __USE_GNU
#include "global.h"
#include "BacktraceNames.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

#include "RageUtil.h"

#if defined(DARWIN)
#include "archutils/Darwin/Crash.h"
#endif

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
#elif defined(HAVE_CXA_DEMANGLE)
#include <cxxabi.h>

void BacktraceNames::Demangle()
{
	/* demangle the name using __cxa_demangle() if needed */
	if( Symbol.substr(0, 2) != "_Z" )
		return;
	
	int status = 0;
	char *name = abi::__cxa_demangle( Symbol, 0, 0, &status );
	if( name )
	{
		Symbol = name;
		free( name );
		return;
	}

	switch( status )
	{
	case -1:
		fprintf( stderr, "Out of memory\n" );
		break;
	case -2:
		fprintf( stderr, "Invalid mangled name: %s.\n", Symbol.c_str() );
		break;
	case -3:
		fprintf( stderr, "Invalid arguments.\n" );
		break;
	default:
		fprintf( stderr, "Unexpected __cxa_demangle status: %i", status );
		break;
	}
}
#else
void BacktraceNames::Demangle() { }
#endif


CString BacktraceNames::Format() const
{
	CString ShortenedPath = File;
	if( ShortenedPath != "" )
	{
		/* Abbreviate the module name. */
		unsigned slash = ShortenedPath.rfind('/');
		if( slash != ShortenedPath.npos )
			ShortenedPath = ShortenedPath.substr(slash+1);
		ShortenedPath = CString("(") + ShortenedPath + ")";
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
void BacktraceNames::FromAddr( const void *p )
{
    Address = (int) p;

    Dl_info di;
    if( !dladdr(p, &di) )
        return;

    Symbol = di.dli_sname;
    File = di.dli_fname;
    Offset = (char*)(p)-(char*)di.dli_saddr;
}

#elif defined(BACKTRACE_LOOKUP_METHOD_BACKTRACE_SYMBOLS)
/* This version parses backtrace_symbols(), an doesn't need libdl. */
#include <execinfo.h>
void BacktraceNames::FromAddr( const void *p )
{
    Address = (int) p;

    char **foo = backtrace_symbols(&p, 1);
    if( foo == NULL )
        return;
    FromString( foo[0] );
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
void BacktraceNames::FromAddr( const void *p )
{
    int fds[2];
    int out = fileno(stdout);
    pid_t pid;
    pid_t ppid = getpid(); /* Do this before fork()ing! */
    
    Offset = 0;
    Address = long(p);

    if (pipe(fds) != 0)
    {
        fprintf(stderr, "FromAddr pipe() failed: %s\n", strerror(errno));
        return;
    }

    pid = fork();
    if (pid == -1)
    {
        fprintf(stderr, "FromAddr fork() failed: %s\n", strerror(errno));
        return;
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
        char *p;
        asprintf(&p, "%d", ppid);

        execl("/usr/bin/atos", "/usr/bin/atos", "-p", p, addy, NULL);
        
        fprintf(stderr, "execl(atos) failed: %s\n", strerror(errno));
        free(addy);
        free(p);
        close(out);
        _exit(1);
    }
    
    close(fds[1]);
    char f[1024];
    bzero(f, 1024);
    int len = read(fds[0], f, 1024);

    Symbol = "";
    File = "";

    if (len == -1)
    {
        fprintf(stderr, "FromAddr read() failed: %s\n", strerror(errno));
        return;
    }
    CStringArray mangledAndFile;

    split(f, " ", mangledAndFile, true);
    if (mangledAndFile.size() == 0)
        return;
    Symbol = mangledAndFile[0];
    /* eg
     * -[NSApplication run]
     * +[SomeClass initialize]
     */
    if (Symbol[0] == '-' || Symbol[0] == '+')
    {
        Symbol = mangledAndFile[0] + " " + mangledAndFile[1];
        /* eg
         * (crt.c:300)
         * (AppKit)
         */
        if (mangledAndFile.size() == 3)
        {
            File = mangledAndFile[2];
            unsigned pos = File.find('(');
            unsigned start = (pos == File.npos ? 0 : pos+1);
            pos = File.rfind(')') - 1;
            File = File.substr(start, pos);
        }
        return;
    }
    /* eg
     * __start   -> _start
     * _SDL_main -> SDL_main
     */
    if (Symbol[0] == '_')
        Symbol = Symbol.substr(1);
    
    /* eg, the full line:
     * __Z1Ci (in a.out) (asmtest.cc:33)
     * _main (in a.out) (asmtest.cc:52)
     */
    if (mangledAndFile.size() > 3)
    {
        File = mangledAndFile[3];
        unsigned pos = File.find('(');
        unsigned start = (pos == File.npos ? 0 : pos+1);
        pos = File.rfind(')') - 1;
        File = File.substr(start, pos);
    }
    /* eg, the full line:
     * _main (SDLMain.m:308)
     * __Z8GameLoopv (crt.c:300)
     */
    else if (mangledAndFile.size() == 3)
        File = mangledAndFile[2].substr(0, mangledAndFile[2].rfind(')'));
}
#else
#warning Undefined BACKTRACE_LOOKUP_METHOD_*
void BacktraceNames::FromAddr( const void *p )
{
    Address = long(p);
    Offset = 0;
    Symbol = "";
    File = "";
}
#endif

