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

	virtual const char *what() const throw();
	virtual ~RageException() throw() { }

protected:
	CString m_sError;
};

#endif
