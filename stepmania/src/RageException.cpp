#include "global.h"
/*
-----------------------------------------------------------------------------
 Class: RageLog

 Desc: See header.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "RageException.h"
#include "RageUtil.h"
#include "RageLog.h"

RageException::RageException( const char *fmt, ...)
{
    va_list	va;
    va_start(va, fmt);
    m_sError = vssprintf( fmt, va );
    va_end(va);
}

RageException::RageException( const char *fmt, va_list va)
{
    m_sError = vssprintf( fmt, va );
}

const char* RageException::what() const throw ()
{
	return m_sError;
}

void RageException::Throw(const char *fmt, ...)
{
    va_list	va;
    va_start(va, fmt);
    CString error = vssprintf( fmt, va );
    va_end(va);

    CString msg = ssprintf(
				"\n"
				"//////////////////////////////////////////////////////\n"
				"Exception: %s\n"
				"//////////////////////////////////////////////////////\n"
				"",
				error.c_str());
	if(LOG)
	{
		LOG->Trace("%s", msg.c_str());
		LOG->Flush();
	}
	else
	{
		printf("%s\n", msg.c_str());
		fflush(stdout);
	}

#if defined(WIN32) && defined(DEBUG)
	MessageBox( NULL, error, "Fatal Error", MB_OK );
	DebugBreak();
#endif

	throw RageException("%s", error.c_str());
}

void RageException::ThrowNonfatal(const char *fmt, ...)
{
    va_list	va;
    va_start(va, fmt);

	if(LOG)
	{
		LOG->Trace("Nonfatal exception thrown: %s", vssprintf( fmt, va ).c_str());
		LOG->Flush();
	}
	
	throw RageException(fmt, va);
}
