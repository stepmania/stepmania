#include "global.h"
/*
-----------------------------------------------------------------------------
 Class: ProfileManager

 Desc: See header.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "ProfileManager.h"
#include "RageUtil.h"
#include "arch/arch.h"
#include "PrefsManager.h"
#include "RageLog.h"
#include "IniFile.h"
#include "GameConstantsAndTypes.h"
#include "SongManager.h"

ProfileManager*	PROFILEMAN = NULL;	// global and accessable from anywhere in our program


#define PROFILE_FILE		"Profile.ini"

#define NOTES_SCORES_FILE	"NotesScores.dat"
#define COURSE_SCORES_FILE	"CourseScores.dat"

ProfileManager::ProfileManager()
{
	for( int p=0; p<NUM_PLAYERS; p++ )
		m_bUsingMemoryCard[p] = false;
}

ProfileManager::~ProfileManager()
{

}

void ProfileManager::GetProfiles( vector<CString> &asNamesOut )
{
	// don't pick up the profile named "Machine"
	GetDirListing( PROFILES_DIR "0*", asNamesOut, true, false );
}

void ProfileManager::GetProfileDisplayNames( vector<CString> &asNamesOut )
{
	CStringArray vsProfiles;
	GetProfiles( vsProfiles );
	for( unsigned i=0; i<vsProfiles.size(); i++ )
	{
		CString sProfile = vsProfiles[i];

		Profile pro;
		pro.LoadFromIni( PROFILES_DIR + sProfile + SLASH + PROFILE_FILE );
		asNamesOut.push_back( pro.m_sDisplayName );
	}
}

bool ProfileManager::DoesProfileExist( CString sProfile )
{
	vector<CString> vsProfiles;
	GetProfiles( vsProfiles );

	return find(vsProfiles.begin(), vsProfiles.end(), sProfile) != vsProfiles.end();
}


void ProfileManager::TryLoadProfile( PlayerNumber pn )
{
	CString sProfile = PREFSMAN->m_sDefaultProfile[pn];
	if( sProfile.empty() )
	{
		m_sProfileDir[pn] = "";
		return;
	}

	if( !DoesProfileExist(sProfile) )
	{
		LOG->Warn( "Default profile '%s' does not exist", sProfile.c_str() );
		return;
	}

	m_sProfileDir[pn] = PROFILES_DIR + sProfile + SLASH;

	m_Profile[pn].LoadFromIni( m_sProfileDir[pn]+PROFILE_FILE );


	//
	// Load scores into SONGMAN
	//
	SONGMAN->ReadNoteScoresFromFile( m_sProfileDir[pn]+NOTES_SCORES_FILE, (MemoryCard)pn );
	SONGMAN->ReadCourseScoresFromFile( m_sProfileDir[pn]+COURSE_SCORES_FILE, (MemoryCard)pn );
}

void ProfileManager::UnloadProfile( PlayerNumber pn )
{
	if( m_sProfileDir[pn].empty() )
		return;

	m_Profile[pn].WriteToIni( m_sProfileDir[pn]+PROFILE_FILE );

	//
	// Save scores into SONGMAN
	//
	SONGMAN->SaveNoteScoresToFile( m_sProfileDir[pn]+NOTES_SCORES_FILE, (MemoryCard)pn );
	SONGMAN->SaveCourseScoresToFile( m_sProfileDir[pn]+COURSE_SCORES_FILE, (MemoryCard)pn );

	// clear out the displayname and highscore name when we unload the profile.
	m_Profile[pn].Init();
}

CString ProfileManager::GetDisplayName( PlayerNumber pn )
{
	ASSERT(!m_sProfileDir[pn].empty()); 
	return m_Profile[pn].m_sDisplayName;
}

bool Profile::LoadFromIni( CString sIniPath )
{
	Init();

	CString sDir, sFName, sExt;
	splitrelpath( sIniPath, sDir, sFName, sExt );
	
	CStringArray asBits;
	split( sDir, SLASH, asBits, true );
	CString sLastDir = asBits.back();	// this is a number name, e.g. "0000001"

	// Fill in a default value in case ini doesn't have it.
	m_sDisplayName = ssprintf("Profile%d", atoi(sLastDir));	


	//
	// read ini
	//
	IniFile ini( sIniPath );
	if( !ini.ReadFile() )
		return false;

	ini.GetValue( "Profile", "DisplayName", m_sDisplayName );
	ini.GetValue( "Profile", "LastUsedHighScoreName", m_sLastUsedHighScoreName );
	return true;
}

bool Profile::WriteToIni( CString sIniPath )
{
	IniFile ini( sIniPath );
	ini.SetValue( "Profile", "DisplayName", m_sDisplayName );
	ini.SetValue( "Profile", "LastUsedHighScoreName", m_sLastUsedHighScoreName );
	ini.WriteFile();
	return true;
}

void ProfileManager::CreateProfile( CString sDisplayName )
{
	//
	// Find a free directory name in the profiles directory
	//
	CString sProfile, sDir;
	for( int i=0; i<1000; i++ )
	{
		sProfile = ssprintf("%08d",i);
		sDir = PROFILES_DIR + sProfile;
		if( !DoesFileExist(sDir) )
			break;
	}


	CreateDirectories( sDir );

	sDir += SLASH;
	
	Profile pro;
	pro.m_sDisplayName = sDisplayName;
	pro.WriteToIni( sDir + PROFILE_FILE );

	FlushDirCache();
}
