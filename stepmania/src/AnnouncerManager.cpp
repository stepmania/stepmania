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
const CString ANNOUNCERS_DIR  = "Announcers/";


AnnouncerManager::AnnouncerManager()
{
	LOG->Trace("AnnouncerManager::AnnouncerManager()");
}

void AnnouncerManager::GetAnnouncerNames( CStringArray& AddTo )
{
	GetDirListing( ANNOUNCERS_DIR+"*", AddTo, true );
	
	// strip out the folder called "CVS" and EMPTY_ANNOUNCER_NAME
	int i;
	for( i=AddTo.size()-1; i>=0; i-- )
		if( !stricmp( AddTo[i], "cvs" ) )
			AddTo.erase(AddTo.begin()+i, AddTo.begin()+i+1 );

	for( i=AddTo.size()-1; i>=0; i-- )
		if( !stricmp( AddTo[i], EMPTY_ANNOUNCER_NAME ) )
			AddTo.erase(AddTo.begin()+i, AddTo.begin()+i+1 );
}

bool AnnouncerManager::DoesAnnouncerExist( CString sAnnouncerName )
{
	if( sAnnouncerName == "" )
		return true;

	CStringArray asAnnouncerNames;
	GetAnnouncerNames( asAnnouncerNames );
	for( unsigned i=0; i<asAnnouncerNames.size(); i++ )
		if( 0==stricmp(sAnnouncerName, asAnnouncerNames[i]) )
			return true;
	return false;
}

CString AnnouncerManager::GetAnnouncerDirFromName( CString sAnnouncerName )
{
	return ANNOUNCERS_DIR + sAnnouncerName + "/";
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
	CString sPathToFolderCurrent = GetAnnouncerDirFromName(m_sCurAnnouncerName) + sFolderName;
	CString sPathToFolderEmpty = GetAnnouncerDirFromName(EMPTY_ANNOUNCER_NAME) + sFolderName;

#ifdef DEBUG
	if( m_sCurAnnouncerName!=""  &&  !DoesFileExist(sPathToFolderCurrent) )
	{
		LOG->Trace( "The current announcer is missing the folder '%s'.", sFolderName.GetString() );
//		MessageBeep( MB_OK );
		CreateDirectories( sPathToFolderCurrent );
	}
	if( !DoesFileExist(sPathToFolderEmpty) )
	{
		LOG->Trace( "The empty announcer is missing the folder '%s'.", sFolderName.GetString() );
//		MessageBeep( MB_OK );
		CreateDirectories( sPathToFolderEmpty );
		CreateDirectories( sPathToFolderEmpty );
	}
#endif

	return sPathToFolderCurrent;
}

bool AnnouncerManager::HasSoundsFor( CString sFolderName )
{
	CStringArray asFileNames;
	GetDirListing( GetPathTo(sFolderName), asFileNames );
	return !asFileNames.empty();
}
