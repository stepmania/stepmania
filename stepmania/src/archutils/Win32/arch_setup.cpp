#include "global.h"
#include "arch_setup.h"
#include "RageThreads.h"
#include <time.h>

struct tm *my_localtime_r( const time_t *timep, struct tm *result )
{
	static RageMutex mut;
	LockMut(mut);

	*result = *localtime( timep );
	return result;
}

struct tm *my_gmtime_r( const time_t *timep, struct tm *result )
{
	static RageMutex mut;
	LockMut(mut);

	*result = *gmtime( timep );
	return result;
}
