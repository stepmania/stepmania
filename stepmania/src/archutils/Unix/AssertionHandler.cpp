#include "global.h"
#include "RageUtil.h"
#include <assert.h>

/* We can define this symbol to catch failed assert() calls.  This is only used
 * for library code that uses assert(); internally we always use ASSERT, which
 * does this for all platforms, not just glibc. */

extern "C" void __assert_fail( const char *assertion, const char *file, unsigned int line, const char *function ) __THROW
{
	Checkpoints::SetCheckpoint( file, line, ssprintf("%s: %s", function, assertion ) );
	sm_crash( assertion );
}

extern "C" void __assert_perror_fail( int errnum, const char *file, unsigned int line, const char *function ) __THROW
{
	Checkpoints::SetCheckpoint( file, line, ssprintf("%s: %s", function, strerror(errnum) ) );
	sm_crash( strerror(errnum) );
}

