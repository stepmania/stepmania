#include "global.h"
#include "RageUtil.h"
#include "StepMania.h"
#include <assert.h>

/* We can define this symbol to catch failed assert() calls.  This is only used
 * for library code that uses assert(); internally we always use ASSERT, which
 * does this for all platforms, not just glibc. */

extern "C" void __assert_fail( const char *assertion, const char *file, unsigned int line, const char *function ) throw()
{
	const CString error = ssprintf( "Assertion failure: %s: %s", function, assertion );

#if defined(CRASH_HANDLER)
	Checkpoints::SetCheckpoint( file, line, error );
	sm_crash( assertion );
#else
	/* It'd be nice to just throw an exception here, but throwing an exception
	 * through C code sometimes explodes. */
	HandleException( error );

	/* XXX: do EmergencyShutdown */
	exit(0);
#endif
}


extern "C" void __assert_perror_fail( int errnum, const char *file, unsigned int line, const char *function ) throw()
{
	const CString error = ssprintf( "Assertion failure: %s: %s", function, strerror(errnum) );

#if defined(CRASH_HANDLER)
	Checkpoints::SetCheckpoint( file, line, error );
	sm_crash( strerror(errnum) );
#else
	HandleException( error );

	/* XXX: do EmergencyShutdown */
	exit(0);
#endif
}

