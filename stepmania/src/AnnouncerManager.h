#ifndef ANNOUNCER_MANAGER_H
#define ANNOUNCER_MANAGER_H
/*
-----------------------------------------------------------------------------
 Class: AnnouncerManager

 Desc: Manages which graphics and sounds are chosed to load.  Every time 
	a sound or graphic is loaded, it gets the path from the AnnouncerManager.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "RageUtil.h"


class AnnouncerManager
{
public:
	AnnouncerManager();

	void GetAnnouncerNames( CStringArray& AddTo );
	bool DoesAnnouncerExist( CString sAnnouncerName );
	void SwitchAnnouncer( CString sNewAnnouncerName );
	CString GetCurAnnouncerName() { return m_sCurAnnouncerName; };

	CString GetPathTo( CString sFolderName );
	bool HasSoundsFor( CString sFolderName );

protected:
	static CString GetAnnouncerDirFromName( CString sAnnouncerName );
	CString GetPathTo( CString AnnouncerPath, CString sFolderName );

	CString m_sCurAnnouncerName;
};



extern AnnouncerManager*	ANNOUNCER;	// global and accessable from anywhere in our program
	
#endif
