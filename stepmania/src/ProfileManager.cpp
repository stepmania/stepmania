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

ProfileManager*	PROFILEMAN = NULL;	// global and accessable from anywhere in our program


#define PROFILES_DIR		BASE_PATH "Data" SLASH "Profiles" SLASH
#define PROFILE_FILE		"Profile.ini"


ProfileManager::ProfileManager()
{
	for( int p=0; p<NUM_PLAYERS; p++ )
		m_bUsingMemoryCard[p] = false;
}

ProfileManager::~ProfileManager()
{

}

void ProfileManager::GetProfileNames( vector<CString> &asNamesOut )
{
	// don't pick up the profile named "Machine"
	GetDirListing( PROFILES_DIR "0*", asNamesOut, true, false );
}

void ProfileManager::TryLoadProfile( PlayerNumber pn )
{
	CString sProfile = PREFSMAN->m_sDefaultProfile[pn];
	if( sProfile.empty() )
		return;

	vector<CString> vsProfiles;
	GetProfileNames( vsProfiles );

	if( find(vsProfiles.begin(), vsProfiles.end(), "sProfile") == vsProfiles.end() )
	{
		LOG->Warn( "Default profile '%s' does not exist", sProfile.c_str() );
		return;
	}

	m_sProfileDir[pn] = PROFILES_DIR + sProfile + SLASH;

	IniFile ini( m_sProfileDir[pn]+PROFILE_FILE );
	ini.ReadFile();

	m_sDisplayName[pn] = ssprintf("No Name");
	ini.GetValue( "Profile", "DisplayName", m_sDisplayName[pn] );
}

void ProfileManager::UnloadProfile( PlayerNumber pn )
{
	if( m_sProfileDir[pn].empty() )
		return;

	IniFile ini( m_sProfileDir[pn]+PROFILE_FILE );

	ini.SetValue( "Profile", "DisplayName", m_sDisplayName[pn] );

	ini.WriteFile();
}
