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
#include <stdarg.h>

class RageException : public exception
{
public:
	RageException( const char *fmt, ...);
	RageException( const char *fmt, va_list va);

	virtual const char *what() const throw();
	virtual ~RageException() throw() { }

	/* The only difference between these is that Throw triggers debug behavior
	 * and Nonfatal doesn't.  Nonfatal is used when the exception happens
	 * normally and will be caught, such as when a driver fails to initialize. */
	static NORETURN Throw(const char *fmt, ...);
	static NORETURN ThrowNonfatal(const char *fmt, ...);

protected:
	CString m_sError;
};


#endif
