#pragma once
/*
-----------------------------------------------------------------------------
 Class: RageError

 Desc: Class for thowing fatal error exceptions

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include <exception>

class RageException : exception
{
public:
	RageException( LPCTSTR fmt, ...);
	RageException( HRESULT hr, LPCTSTR fmt, ...);

	virtual const char *what() const;

protected:
	CString m_sError;
};

