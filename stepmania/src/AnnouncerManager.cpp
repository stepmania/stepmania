#include "stdafx.h"
/*
-----------------------------------------------------------------------------
 Class: AnnouncerManager

 Desc: See header.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "AnnouncerManager.h"
#include "RageLog.h"


AnnouncerManager*	ANNOUNCER = NULL;	// global object accessable from anywhere in the program


const CString EMPTY_ANNOUNCER_NAME = "Empty";
const CString ANNOUNCERS_DIR  = "Announcers\\";


AnnouncerManager::AnnouncerManager()
{
	LOG->Trace("AnnouncerManager::AnnouncerManager()");
}

void AnnouncerManager::GetAnnouncerNames( CStringArray& AddTo )
{
	GetDirListing( ANNOUNCERS_DIR+"*", AddTo, true );
	
	// strip out the folder called "CVS" and EMPTY_ANNOUNCER_NAME
	for( int i=AddTo.GetSize()-1; i>=0; i-- )
	{
		if( 0 == stricmp( AddTo[i], "cvs" ) )
			AddTo.RemoveAt(i);
		if( 0 == stricmp( AddTo[i], EMPTY_ANNOUNCER_NAME ) )
			AddTo.RemoveAt(i);
	}
}

bool AnnouncerManager::DoesAnnouncerExist( CString sAnnouncerName )
{
	if( sAnnouncerName == "" )
		return true;

	CStringArray asAnnouncerNames;
	GetAnnouncerNames( asAnnouncerNames );
	for( int i=0; i<asAnnouncerNames.GetSize(); i++ )
		if( 0==stricmp(sAnnouncerName, asAnnouncerNames[i]) )
			return true;
	return false;
}

CString AnnouncerManager::GetAnnouncerDirFromName( CString sAnnouncerName )
{
	return ANNOUNCERS_DIR + sAnnouncerName + "\\";
}

void AnnouncerManager::SwitchAnnouncer( CString sNewAnnouncerName )
{
	if( !DoesAnnouncerExist(sNewAnnouncerName) )
		m_sCurAnnouncerName = "";
	else
		m_sCurAnnouncerName = sNewAnnouncerName;
}

CString AnnouncerManager::GetPathTo( CString sFolderName )
{
#ifdef _DEBUG
try_again:
#endif

	CString sPathToFolderCurrent = GetAnnouncerDirFromName(m_sCurAnnouncerName) + "\\" + sFolderName;
	CString sPathToFolderEmpty = GetAnnouncerDirFromName(EMPTY_ANNOUNCER_NAME) + "\\" + sFolderName;

#ifdef _DEBUG
	if( m_sCurAnnouncerName!=""  &&  !DoesFileExist(sPathToFolderCurrent) )
	{
		int iResult = AfxMessageBox( 
			ssprintf(
			"The current announcer is missing the folder '%s'.\n"
			"It may be that these sounds were never implemented in this announcer,\n"
			"Or the folder may be misnamed.\n\n"
			"Click Abort to break.\n"
			"Click Retry after adding the folder menually.\n"
			"Click Ignore to to automatically create the folder.\n\n", sFolderName), MB_ABORTRETRYIGNORE );
		switch( iResult )
		{
		case IDABORT:
			DebugBreak();
			break;
		case IDRETRY:
			goto try_again;
			break;
		case IDIGNORE:
			CreateDirectory( sPathToFolderCurrent, NULL );
			break;
		}
	}
	if( !DoesFileExist(sPathToFolderEmpty) )
	{
		int iResult = AfxMessageBox( 
			ssprintf(
			"The empty announcer is missing the folder '%s'.\n"
			"This announcer should have empty folders for every saying.\n\n"
			"Click Abort to break.\n"
			"Click Retry after adding the folder menually.\n"
			"Click Ignore to to automatically create the folder.\n\n", sFolderName), MB_ABORTRETRYIGNORE );
		switch( iResult )
		{
		case IDABORT:
			DebugBreak();
			break;
		case IDRETRY:
			goto try_again;
			break;
		case IDIGNORE:
			CreateDirectory( sPathToFolderEmpty, NULL );
			break;
		}
	}
#endif

	return sPathToFolderCurrent;
}

