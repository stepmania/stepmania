#pragma once
/*
-----------------------------------------------------------------------------
 Class: RageLog

 Desc: Manages logging

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/


class RageLog
{
public:
	RageLog();
	~RageLog();

	void Trace( LPCTSTR fmt, ...);
	void Trace( HRESULT hr, LPCTSTR fmt, ...);
	void Warn( LPCTSTR fmt, ...);
	void Flush();

	void ShowConsole();
	void HideConsole();

protected:
	FILE* m_fileLog;
};

extern RageLog*	LOG;	// global and accessable from anywhere in our program
