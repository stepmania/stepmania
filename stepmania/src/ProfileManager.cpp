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

#define STEPS_MEM_CARD_DATA_FILE	"StepsMemCardData.dat"
#define COURSE_MEM_CARD_DATA_FILE	"CourseMemCardData.dat"
#define NEW_MEM_CARD_NAME			"NewCard"
#define NEW_PROFILE_NAME			"NewProfile"

ProfileManager::ProfileManager()
{
	for( int p=0; p<NUM_PLAYERS; p++ )
		m_bUsingMemoryCard[p] = false;
}

ProfileManager::~ProfileManager()
{

}

void ProfileManager::GetMachineProfileIDs( vector<CString> &asProfileIDsOut )
{
	GetDirListing( PROFILES_DIR "*", asProfileIDsOut, true, false );
}

void ProfileManager::GetMachineProfileNames( vector<CString> &asNamesOut )
{
	CStringArray vsProfileIDs;
	GetMachineProfileIDs( vsProfileIDs );
	for( unsigned i=0; i<vsProfileIDs.size(); i++ )
	{
		CString sProfileID = vsProfileIDs[i];

		Profile pro;
		pro.LoadFromIni( PROFILES_DIR + sProfileID + SLASH + PROFILE_FILE );
		asNamesOut.push_back( pro.m_sName );
	}
}


bool ProfileManager::LoadProfile( PlayerNumber pn, CString sProfileDir, bool bIsMemCard )
{
	ASSERT( !sProfileDir.empty() );
	ASSERT( sProfileDir.Right(1) == SLASH );

	m_sProfileDir[pn] = sProfileDir;
	m_bUsingMemoryCard[pn] = bIsMemCard;

	bool bResult = m_Profile[pn].LoadFromIni( m_sProfileDir[pn]+PROFILE_FILE );
	if( !bResult )
	{
		LOG->Warn( "Attempting to load profile from '%s' and does not exist", sProfileDir.c_str() );
		UnloadProfile( pn );
		return false;
	}

	//
	// Load scores into SONGMAN
	//
	SONGMAN->ReadStepsMemCardDataFromFile( m_sProfileDir[pn]+STEPS_MEM_CARD_DATA_FILE, (MemoryCard)pn );
	SONGMAN->ReadCourseMemCardDataFromFile( m_sProfileDir[pn]+COURSE_MEM_CARD_DATA_FILE, (MemoryCard)pn );

	return true;
}

bool ProfileManager::CreateProfile( CString sProfileDir, CString sName )
{
	bool bResult;

	CreateDirectories( sProfileDir );
	
	Profile pro;
	pro.m_sName = sName;
	bResult = pro.SaveToIni( sProfileDir + PROFILE_FILE );
	if( !bResult )
		return false;

	FlushDirCache();
	return true;	
}

bool ProfileManager::LoadDefaultProfileFromMachine( PlayerNumber pn )
{
	CString sProfileID = PREFSMAN->m_sDefaultMachineProfileID[pn];
	if( sProfileID.empty() )
	{
		m_sProfileDir[pn] = "";
		return false;
	}

	CString sDir = PROFILES_DIR + sProfileID + SLASH;

	return LoadProfile( pn, sDir, false );
}

CString GetMemCardDir( PlayerNumber pn )
{
	CString sDir = PREFSMAN->m_sMemoryCardDir[pn];
	if( sDir.empty() )
		return sDir;
	if( sDir.Right(1) != SLASH )
		sDir += SLASH;
	return sDir;
}

bool ProfileManager::IsMemoryCardInserted( PlayerNumber pn )
{
	CString sDir = GetMemCardDir( pn );
	if( sDir.empty() )
		return false;

#ifdef _WINDOWS
	// Windows will throw up a message box if we try to write to a
	// removable drive with no disk inserted.  Find out whether there's a 
	// disk in the drive w/o writing a file.

	// find drive letter
	vector<CString> matches;
	static Regex parse("^([A-Za-z]+):");
	parse.Compare(sDir, matches);
	if( matches.size() != 1 )
	{
		return false;
	}
	else
	{
		CString sDrive = matches[0];
		TCHAR szVolumeNameBuffer[MAX_PATH];
		DWORD dwVolumeSerialNumber;
		DWORD dwMaximumComponentLength;
		DWORD lpFileSystemFlags;
		TCHAR szFileSystemNameBuffer[MAX_PATH];
		BOOL bResult = GetVolumeInformation( 
			sDrive + ":",
			szVolumeNameBuffer,
			sizeof(szVolumeNameBuffer),
			&dwVolumeSerialNumber,
			&dwMaximumComponentLength,
			&lpFileSystemFlags,
			szFileSystemNameBuffer,
			sizeof(szFileSystemNameBuffer) );
		return !!bResult;
	}
#else

	// Try to create directory before writing a temp file.
	CreateDirectories( sDir );
	
	// Test whether a memory card is usable by trying to write a file.
	CString sFile = sDir + "temp";
	FILE* fp = fopen( sFile, "w" );
	if( fp )
	{
		fclose( fp );
		remove( sFile );
		return true;
	}
	else
	{
		return false;
	}
#endif
}

bool ProfileManager::LoadProfileFromMemoryCard( PlayerNumber pn )
{
	CString sDir = GetMemCardDir( pn );
	if( sDir.empty() )
		return false;
	
	m_bUsingMemoryCard[pn] = true;
	bool bResult;
	bResult = LoadProfile( pn, sDir, false );
	return bResult;
}

bool ProfileManager::LoadFirstAvailableProfile( PlayerNumber pn )
{
	// mount card
	if( !PREFSMAN->m_sMemoryCardMountCommand[pn].empty() )
		system( PREFSMAN->m_sMemoryCardMountCommand[pn] );

	if( IsMemoryCardInserted(pn) )
	{
		if( LoadProfileFromMemoryCard(pn) )
			return true;
	
		CString sDir = GetMemCardDir( pn );
		CreateProfile( sDir, NEW_MEM_CARD_NAME );
		if( LoadProfileFromMemoryCard(pn) )
			return true;
	}

	if( LoadDefaultProfileFromMachine(pn) )
		return true;
	
	return false;
}

bool ProfileManager::SaveProfile( PlayerNumber pn )
{
	if( m_sProfileDir[pn].empty() )
		return false;

	m_Profile[pn].SaveToIni( m_sProfileDir[pn]+PROFILE_FILE );

	//
	// Save scores into SONGMAN
	//
	// TODO: move record data to this class
	SONGMAN->SaveStepsMemCardDataToFile( m_sProfileDir[pn]+STEPS_MEM_CARD_DATA_FILE, (MemoryCard)pn );
	SONGMAN->SaveCourseMemCardDataToFile( m_sProfileDir[pn]+COURSE_MEM_CARD_DATA_FILE, (MemoryCard)pn );
	
	return true;
}

void ProfileManager::UnloadProfile( PlayerNumber pn )
{
	m_sProfileDir[pn] = "";
	m_bUsingMemoryCard[pn] = false;
	m_Profile[pn].Init();
}

Profile* ProfileManager::GetProfile( PlayerNumber pn )
{
	if( m_sProfileDir[pn].empty() )
		return NULL;
	else
		return &m_Profile[pn];
}

bool Profile::LoadFromIni( CString sIniPath )
{
	Init();

	CStringArray asBits;
	split( Dirname(sIniPath), SLASH, asBits, true );
	CString sLastDir = asBits.back();	// this is a number name, e.g. "0000001"

	// Fill in a default value in case ini doesn't have it.
	m_sName = NEW_PROFILE_NAME;	


	//
	// read ini
	//
	IniFile ini( sIniPath );
	if( !ini.ReadFile() )
		return false;

	ini.GetValue( "Profile", "DisplayName", m_sName );
	ini.GetValue( "Profile", "LastUsedHighScoreName", m_sLastUsedHighScoreName );
	return true;
}

bool Profile::SaveToIni( CString sIniPath )
{
	IniFile ini( sIniPath );
	ini.SetValue( "Profile", "DisplayName", m_sName );
	ini.SetValue( "Profile", "LastUsedHighScoreName", m_sLastUsedHighScoreName );
	ini.WriteFile();
	return true;
}

bool ProfileManager::CreateMachineProfile( CString sName )
{
	if( sName.empty() )
		sName = "Machine";

	//
	// Find a free directory name in the profiles directory
	//
	CString sProfileID, sProfileDir;
	const int MAX_TRIES = 1000;
        int i;
	for( i=0; i<MAX_TRIES; i++ )
	{
		sProfileID = ssprintf("%08d",i);
		sProfileDir = PROFILES_DIR + sProfileID;
		if( !DoesFileExist(sProfileDir) )
			break;
	}
	if( i == MAX_TRIES )
		return false;
	sProfileDir += SLASH;

	bool bResult;
	bResult = CreateDirectories( sProfileDir );
	if( !bResult )
		return false;

	Profile pro;
	pro.m_sName = sName;
	bResult = pro.SaveToIni( sProfileDir + PROFILE_FILE );
	if( !bResult )
		return false;

	FlushDirCache();
	return true;
}

bool ProfileManager::RenameMachineProfile( CString sProfileID, CString sNewName )
{
	ASSERT( !sProfileID.empty() );

	CString sProfileDir = PROFILES_DIR + sProfileID;
	CString sProfileFile = sProfileDir + SLASH PROFILE_FILE;

	Profile pro;
	bool bResult;
	bResult = pro.LoadFromIni( sProfileFile );
	if( !bResult )
		return false;
	pro.m_sName = sNewName;
	bResult = pro.SaveToIni( sProfileFile );
	if( !bResult )
		return false;

	return true;
}

bool ProfileManager::DeleteMachineProfile( CString sProfileID )
{
	// delete all files in profile dir
	CString sProfileDir = PROFILES_DIR + sProfileID;
	CStringArray asFilesToDelete;
	GetDirListing( sProfileDir + SLASH "*", asFilesToDelete, false, true );
	for( unsigned i=0; i<asFilesToDelete.size(); i++ )
		remove( asFilesToDelete[i] );

	// remove profile dir
	// FIXME for non Win32 platforms
	int ret = rmdir( sProfileDir );
	FlushDirCache();
	if( ret != 0 )
		return false;
	else
		return true;
}
