#ifndef RAGEEXCEPTION_H
#define RAGEEXCEPTION_H
/*
-----------------------------------------------------------------------------
 Class: RageError

 Desc: Class for thowing fatal error exceptions

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include <exception>

class RageException : public exception
{
public:
	RageException( const char *fmt, ...);
	RageException( HRESULT hr, const char *fmt, ...);

	virtual const char *what() const;

protected:
	CString m_sError;
};

//#include "crash.h"

/* Assertion that sets an optional message and brings up the crash handler, so
 * we get a backtrace.  This should probably be used instead of throwing an
 * exception in most cases we expect never to happen (but not in cases that
 * we do expect, such as d3d init failure.) */
//#define RAGE_ASSERT_M(COND, MESSAGE) { if(!(COND)) { VDCHECKPOINT_M(MESSAGE); *(char*)0=0; } }
//#define RAGE_ASSERT(COND) RAGE_ASSERT_M((COND), "Assertion failure")

/* Make this the default assert handler. */
//#ifdef ASSERT
//#undef ASSERT
//#endif
//#define ASSERT RAGE_ASSERT


#endif
