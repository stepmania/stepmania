#pragma once
/*
-----------------------------------------------------------------------------
 Class: RageError

 Desc: Class for thowing fatal error exceptions

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/


class RageException 
{
public:
	RageException( LPCTSTR fmt, ...);
	RageException( HRESULT hr, LPCTSTR fmt, ...);

	CString GetError()	{ return m_sError; };

protected:
	CString m_sError;
};

