#include "stdafx.h"
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

RageException::ThrowFatal(const char *fmt, ...)
{
    va_list	va;
    va_start(va, fmt);
    CString error = vssprintf( fmt, va );
    va_end(va);

#if defined(WIN32) && defined(DEBUG)
	MessageBox( NULL, error, "Fatal Error", MB_OK );
	DebugBreak();
#endif

	throw RageException("%s", error);
}

RageException::ThrowNonfatal(const char *fmt, ...)
{
    va_list	va;
    va_start(va, fmt);

	throw RageException(fmt, va);
}
