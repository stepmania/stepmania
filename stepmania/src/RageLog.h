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

	void MapLog(const CString &key, const char *fmt, ...);
	void UnmapLog(const CString &key);

private:
	FILE *m_fileLog, *m_fileInfo;
	void Write( int, CString );
	void UpdateMappedLog();
};

/* Mapped log entry that's deleted on dtor.  Note that this isn't extremely
 * fast (like VDCHECKPOINT); don't use it in inner loops. */
struct Checkpoint_
{
	CString key;
	Checkpoint_(CString key, int n, CString fmt, ...);
	~Checkpoint_();
};
#define Checkpoint(t) Checkpoint_ CP_( __FUNCTION__, __LINE__, t )

extern RageLog*	LOG;	// global and accessable from anywhere in our program
#endif
