/* for dladdr: */
#define __USE_GNU
#include "global.h"
#include "BacktraceNames.h"

#include <cerrno>
#include <cstdarg>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#if defined(HAVE_UNISTD_H)
#include <unistd.h>
#endif

#include "RageUtil.h"

#if defined(MACOSX)
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
	char *name = abi::__cxa_demangle( Symbol, nullptr, nullptr, &status );
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
		fprintf( stderr, "Unexpected __cxa_demangle status: %i\n", status );
		break;
	}
}
#else
void BacktraceNames::Demangle() { }
#endif


RString BacktraceNames::Format() const
{
	RString ShortenedPath = File;
	if( ShortenedPath != "" )
	{
		/* Abbreviate the module name. */
		std::size_t slash = ShortenedPath.rfind('/');
		if( slash != ShortenedPath.npos )
			ShortenedPath = ShortenedPath.substr(slash+1);
		ShortenedPath = RString("(") + ShortenedPath + ")";
	}

	RString ret = ssprintf( "%0*lx: ", int(sizeof(void*)*2), (long) Address );
	if( Symbol != "" )
		ret += Symbol + " ";
	ret += ShortenedPath;

	return ret;
}


#if defined(BACKTRACE_LOOKUP_METHOD_DLADDR)
/* This version simply asks libdl, which is more robust. */
#include <dlfcn.h>
void BacktraceNames::FromAddr( void * const p )
{
    Address = (std::intptr_t) p;

    /*
     * When calling a function that doesn't return, gcc will truncate a function.
     * This results in our return addresses lying just beyond the end of a function.
     * Those addresses are correct; our backtrace addresses are usually a list of
     * return addresses (that's the same thing you'll see in gdb).  dladdr won't work,
     * because they're not actually within the bounds of the function.
     *
     * A generic fix for this would be to adjust the backtrace to be a list of
     * places where control was lost, instead of where it'll come back to; to point
     * to the CALL instead of just beyond the CALL.  That's hard to do generically;
     * the stack only contains the return pointers.  (Some archs don't even contain
     * that: on archs where gcc has to manually store the return address--by pushing
     * the IP--it will omit the push when calling a function that never returns.)
     *
     * Let's try the address we're given, and if that fails to find a symbol, try
     * moving up a few bytes.  This usually makes "__cxa_throw, std::terminate,
     * gsignal" stacks come out right.  It probably won't work if there's no space
     * between one function and the next, because the first lookup will succeed.
     */
    Dl_info di;
    if( !dladdr((void *) p, &di) || di.dli_sname == nullptr )
    {
		if( !dladdr( ((char *) p) - 8, &di) )
			return;
    }

    Symbol = di.dli_sname? di.dli_sname:"";
    File = di.dli_fname;
    Offset = (char*)(p)-(char*)di.dli_saddr;
}

#elif defined(BACKTRACE_LOOKUP_METHOD_DARWIN_DYLD)
/*
Copyright (c) 2002 Jorge Acereda  <jacereda@users.sourceforge.net> &
                   Peter O'Gorman <ogorman@users.sourceforge.net>

Permission is hereby granted, free of charge, to any person obtaining
a copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

 Cleaned and adapted.  -glenn
*/

#include <mach-o/dyld.h>
#include <mach-o/nlist.h>

static const load_command *next_load_command( const load_command *cmd )
{
	const char *p = (const char *) cmd;
	p += cmd->cmdsize;
	return (load_command *) p;
}

/* Return the image index of the given pointer, or -1 if not found. */
static int osx_find_image( const void *p )
{
	unsigned long image_count = _dyld_image_count();

	for( unsigned i = 0; i < image_count; i++ )
	{
		const struct mach_header *header = _dyld_get_image_header(i);
		if( header == nullptr )
			continue;

		/* The load commands directly follow the mach_header. */
		const struct load_command *cmd = (struct load_command *) &header[1];

		for( unsigned j = 0; j < header->ncmds; j++ )
		{
			/* We only care about mapped segments. */
			if( cmd->cmd != LC_SEGMENT )
			{
				cmd = next_load_command(cmd);
				continue;
			}

			const segment_command *scmd = (const segment_command *) cmd;

			const unsigned long bottom = scmd->vmaddr + _dyld_get_image_vmaddr_slide( i );
			const unsigned long top = scmd->vmaddr + scmd->vmsize + _dyld_get_image_vmaddr_slide( i );
			if ( (unsigned long) p >= bottom && (unsigned long) p < top )
				return i;

			cmd = next_load_command(cmd);
		}
	}

	return -1;
}

static const char *osx_find_link_edit( const struct mach_header *header )
{
	const struct load_command *cmd = (struct load_command *) &header[1];
	for( unsigned i = 0; i < header->ncmds; i++, cmd = next_load_command(cmd) )
	{
		if( cmd->cmd != LC_SEGMENT )
			continue;
		const segment_command *scmd = (const segment_command *) cmd;

		if( !strcmp(scmd->segname, "__LINKEDIT") )
			return (char *) ( scmd->vmaddr - scmd->fileoff );
	}

	return nullptr;
}

void BacktraceNames::FromAddr( void * const p )
{
	Address = (std::intptr_t) p;

	/* Find the image with the given pointer. */
	int index = osx_find_image( p );
	if( index == -1 )
		return;

	File = _dyld_get_image_name( index );

	/* Find the link-edit pointer. */
	const char *link_edit = osx_find_link_edit( _dyld_get_image_header(index) );
	if( link_edit == nullptr )
		return;
	link_edit += _dyld_get_image_vmaddr_slide( index );

	unsigned long addr = (unsigned long)p - _dyld_get_image_vmaddr_slide(index);
	const struct mach_header *header = _dyld_get_image_header( index );
	const struct load_command *cmd = (struct load_command *) &header[1];
	unsigned long diff = 0xffffffff;

	const char *dli_sname = nullptr;
	void *dli_saddr = nullptr;

	for( unsigned long i = 0; i < header->ncmds; i++, cmd = next_load_command(cmd) )
	{
		if( cmd->cmd != LC_SYMTAB )
			continue;

		const symtab_command *scmd = (const symtab_command *) cmd;
		struct nlist *symtable = (struct nlist *)( link_edit + scmd->symoff );
		for( unsigned long  j = 0; j < scmd->nsyms; j++ )
		{
			if( !symtable[j].n_value )
				continue; /* Undefined */
			if( symtable[j].n_type >= N_PEXT )
				continue; /* Debug symbol */

			if( addr >= symtable[j].n_value && diff >= (addr - symtable[j].n_value) )
			{
				diff = addr - (unsigned long)symtable[j].n_value;
				dli_saddr = symtable[j].n_value + ((char *)p - addr);
				dli_sname = (const char *)( link_edit + scmd->stroff + symtable[j].n_un.n_strx );
			}
		}
	}

	if( diff == 0xffffffff )
		return;

	Symbol = dli_sname;
	Offset = (char*)(p)-(char*)dli_saddr;

	/*
	 * __start   -> _start
	 * __ZN7RageLog5TraceEPKcz -> _ZN7RageLog5TraceEPKcz (so demangling will work)
	 */
	if( Symbol.Left(1) == "_" )
		Symbol = Symbol.substr(1);
	/* After stripping off the leading _
	 * _GLOBAL__I__ZN5ModelC2Ev -> _ZN5ModelC2Ev
	 * _GLOBAL__D__Z12ForceToAsciiR7CStdStrIcE -> _Z12ForceToAsciiR7CStdStrIcE
	 */
	if( Symbol.Left(9) == "_GLOBAL__" )
		Symbol = Symbol.substr(11);

}

#elif defined(BACKTRACE_LOOKUP_METHOD_BACKTRACE_SYMBOLS)
/* This version parses backtrace_symbols(), an doesn't need libdl. */
#include <execinfo.h>
void BacktraceNames::FromAddr( void * const p )
{
    Address = (std::intptr_t) p;

    char **foo = backtrace_symbols(&p, 1);
    if( foo == nullptr )
        return;
    FromString( foo[0] );
    free(foo);
}

/* "path(mangled name+offset) [address]" */
void BacktraceNames::FromString( RString s )
{
    /* Hacky parser.  I don't want to use regexes in the crash handler. */
    RString MangledAndOffset, sAddress;
    unsigned pos = 0;
    while( pos < s.size() && s[pos] != '(' && s[pos] != '[' )
        File += s[pos++];
    Trim( File );

    if( pos < s.size() && s[pos] == '(' )
    {
        pos++;
        while( pos < s.size() && s[pos] != ')' )
            MangledAndOffset += s[pos++];
    }

    if( MangledAndOffset != "" )
    {
        std::size_t plus = MangledAndOffset.rfind('+');

        if(plus == MangledAndOffset.npos)
        {
            Symbol = MangledAndOffset;
            Offset = 0;
        }
        else
        {
            Symbol = MangledAndOffset.substr(0, plus);
            RString str = MangledAndOffset.substr(plus);
            if( sscanf(str, "%i", &Offset) != 1 )
                Offset=0;
        }
    }
}
#else
#warning Undefined BACKTRACE_LOOKUP_METHOD_*
void BacktraceNames::FromAddr( void * const p )
{
    Address = std::intptr_t(p);
    Offset = 0;
    Symbol = "";
    File = "";
}
#endif

/*
 * (c) 2003-2004 Glenn Maynard
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
