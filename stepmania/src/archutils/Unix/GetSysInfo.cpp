#include "global.h"
#include "GetSysInfo.h"
#include "RageUtil.h"

#include <sys/utsname.h>

void GetKernel( CString &sys, int &vers )
{
	utsname uts;
	uname( &uts );

	sys = uts.sysname;
	vers = 0;

	if( sys == "Linux" )
	{
		static Regex ver( "([0-9]+)\\.([0-9]+)\\.([0-9]+)" );
		vector<CString> matches;
		if( ver.Compare(uts.release, matches) )
		{
			ASSERT( matches.size() >= 2 );
			int major = atoi(matches[0]);
			int minor = atoi(matches[1]);
			int revision = atoi(matches[2]);
			vers = (major * 10000) + (minor * 100) + (revision);
		}
	}
}

