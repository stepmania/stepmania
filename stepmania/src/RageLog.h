#ifndef RAGELOG_H
#define RAGELOG_H

/*
-----------------------------------------------------------------------------
 Class: RageLog

 Desc: Manages logging

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include <stdio.h>

class RageLog
{
public:
	RageLog();
	~RageLog();

	void Trace( const char *fmt, ...);
	void Warn( const char *fmt, ...);
	void Info( const char *fmt, ...);
	void Flush();

	void ShowConsole();
	void HideConsole();

private:
	FILE *m_fileLog, *m_fileInfo;
	void Write( int, CString );
};

extern RageLog*	LOG;	// global and accessable from anywhere in our program

#endif
