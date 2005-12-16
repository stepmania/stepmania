#include "global.h"
#include "RageUtil.h"
#include "StepMania.h"
#include "archutils/Unix/EmergencyShutdown.h"
#include <unistd.h>
#include <assert.h>

/* We can define this symbol to catch failed assert() calls.  This is only used
 * for library code that uses assert(); internally we always use ASSERT, which
 * does this for all platforms, not just glibc. */

extern "C" void __assert_fail( const char *assertion, const char *file, unsigned int line, const char *function ) throw()
{
	const RString error = ssprintf( "Assertion failure: %s: %s", function, assertion );

#if defined(CRASH_HANDLER)
	Checkpoints::SetCheckpoint( file, line, error );
	sm_crash( assertion );
#else
	/* It'd be nice to just throw an exception here, but throwing an exception
	 * through C code sometimes explodes. */
	HandleException( error );

	DoEmergencyShutdown();

	_exit(0);
#endif
}


extern "C" void __assert_perror_fail( int errnum, const char *file, unsigned int line, const char *function ) throw()
{
	const RString error = ssprintf( "Assertion failure: %s: %s", function, strerror(errnum) );

#if defined(CRASH_HANDLER)
	Checkpoints::SetCheckpoint( file, line, error );
	sm_crash( strerror(errnum) );
#else
	HandleException( error );

	DoEmergencyShutdown();

	_exit(0);
#endif
}

/* Catch unhandled C++ exceptions.  Note that this works in g++ even with -fno-exceptions, in
 * which case it'll be called if any exceptions are thrown at all. */
#include <cxxabi.h>
void UnexpectedExceptionHandler()
{
	type_info *pException = abi::__cxa_current_exception_type();
	char const *pName = pException->name();
	int iStatus = -1;
	char *pDem = abi::__cxa_demangle( pName, 0, 0, &iStatus );

	const CString error = ssprintf("Unhandled exception: %s", iStatus? pName:pDem);
#if defined(CRASH_HANDLER)
	sm_crash( error );
#else
	HandleException( error );
#endif
}

void InstallExceptionHandler()
{
	set_terminate( UnexpectedExceptionHandler );
}

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
