#include "global.h"
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

/* XXX: move to RageUtil when I feel like spending 20 minutes recompiling */
/* Return true if "dir" is empty or does not exist. */
static bool DirectoryIsEmpty( CString dir )
{
	if(dir == "")
		return true;
	if(!DoesFileExist(dir))
		return true;

	CStringArray asFileNames;
	GetDirListing( dir, asFileNames );
	return asFileNames.empty();
}

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

/* Aliases for announcer paths.  This is for compatibility, so we don't force
 * announcer changes along with everything else.  We could use it to support
 * DWI announcers transparently, too. */
static const char *aliases[][2] = {
	/* ScreenSelectDifficulty compatibility: */
	{ "ScreenSelectDifficulty comment arcade-beginner", "select difficulty comment beginner" },
	{ "ScreenSelectDifficulty comment arcade-easy", "select difficulty comment easy" },
	{ "ScreenSelectDifficulty comment arcade-medium", "select difficulty comment medium" },
	{ "ScreenSelectDifficulty comment arcade-hard", "select difficulty comment hard" },
	{ "ScreenSelectDifficulty comment oni", "select difficulty comment oni" },
	{ "ScreenSelectDifficulty comment nonstop", "select difficulty comment nonstop" },
	{ "ScreenSelectDifficulty comment endless", "select difficulty comment endless" },
	{ "ScreenSelectDifficulty intro", "select difficulty intro" },

	/* ScreenSelectStyle compatibility: */
	{ "ScreenSelectStyle intro", "select style intro" },
	{ "ScreenSelectStyle comment single", "select style comment single" },
	{ "ScreenSelectStyle comment double", "select style comment double" },
	{ "ScreenSelectStyle comment solo", "select style comment solo" },
	{ "ScreenSelectStyle comment versus", "select style comment versus" },

	{ NULL, NULL }
};

/* Find an announcer directory with sounds in it.  First search sFolderName,
 * then all aliases above.  Ignore directories that are empty, since we might
 * have "select difficulty intro" with sounds and an empty "ScreenSelectDifficulty
 * intro". */
CString AnnouncerManager::GetPathTo( CString sAnnouncerName, CString sFolderName )
{
	if(sAnnouncerName == "")
		return ""; /* announcer disabled */

	const CString AnnouncerPath = GetAnnouncerDirFromName(sAnnouncerName);

	if( !DirectoryIsEmpty(AnnouncerPath+sFolderName+"/") )
		return AnnouncerPath+sFolderName+"/";

	/* Search for the announcer folder in the list of aliases. */
	int i;
	for(i = 0; aliases[i][0] != NULL; ++i)
	{
		if(sFolderName.CompareNoCase(aliases[i][0]))
			continue; /* no match */

		if( !DirectoryIsEmpty(AnnouncerPath+aliases[i][1]+"/") )
			return AnnouncerPath+aliases[i][1]+"/";
	}

	/* No announcer directory matched.  In debug, create the directory by
	 * its preferred name. */
#ifdef DEBUG
	LOG->Trace( "The announcer in \"%s\" is missing the folder '%s'.",
		AnnouncerPath.GetString(), sFolderName.GetString() );
//	MessageBeep( MB_OK );
	CreateDirectories( AnnouncerPath+sFolderName );
#endif
	
	return "";
}

CString AnnouncerManager::GetPathTo( CString sFolderName )
{
	CString sPath = GetPathTo(m_sCurAnnouncerName, sFolderName);
	if(sPath != "")
		return sPath;

	return GetPathTo(EMPTY_ANNOUNCER_NAME, sFolderName);
}

bool AnnouncerManager::HasSoundsFor( CString sFolderName )
{
	return !DirectoryIsEmpty( GetPathTo(sFolderName) );
}
