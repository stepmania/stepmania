#pragma once
/*
-----------------------------------------------------------------------------
 Class: RageLog

 Desc: Manages logging

 Copyright (c) 2001-2002 by the persons listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/


class RageLog
{
public:
	RageLog();
	~RageLog();

	void WriteLine( LPCTSTR fmt, ...);
	void WriteLineHr( HRESULT hr, LPCTSTR fmt, ...);
	void Flush();

protected:
	FILE* m_fileLog;
};

extern RageLog*	LOG;	// global and accessable from anywhere in our program
