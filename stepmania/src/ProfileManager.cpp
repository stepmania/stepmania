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
#include "RageFile.h"
#include "RageFileManager.h"
#include "IniFile.h"
#include "GameConstantsAndTypes.h"
#include "SongManager.h"
#include "GameState.h"
#include "song.h"
#include "Course.h"
#include "GameManager.h"
#include "ProductInfo.h"
#include "RageUtil.h"
#include "ThemeManager.h"


ProfileManager*	PROFILEMAN = NULL;	// global and accessable from anywhere in our program


#define PROFILE_FILE		"Profile.ini"

#define CATEGORY_SCORES_FILE	"CategoryScores.dat"
#define SONG_SCORES_FILE		"SongScores.dat"
#define COURSE_SCORES_FILE		"CourseScores.dat"
#define STATS_HTML_FILE			"stats.html"
#define STYLE_CSS_FILE			"style.css"
#define NEW_MEM_CARD_NAME		"NewCard"
#define NEW_PROFILE_NAME		"NewProfile"

#define SM_300_STATISTICS_FILE	BASE_PATH "statistics.ini"

#define USER_PROFILES_DIR		BASE_PATH "Data" SLASH "LocalProfiles" SLASH
#define MACHINE_PROFILE_DIR		BASE_PATH "Data" SLASH "MachineProfile" SLASH

const int CATEGORY_RANKING_VERSION = 4;
const int STEPS_SCORES_VERSION = 8;
const int COURSE_SCORES_VERSION = 6;


static const char *MemCardDirs[NUM_PLAYERS] =
{
	/* @ is important; see RageFileManager LoadedDriver::GetPath */
	"@mc1" SLASH,
	"@mc2" SLASH,
};

ProfileManager::ProfileManager()
{
	for( int p=0; p<NUM_PLAYERS; p++ )
	{
		m_bUsingMemoryCard[p] = false;

		if( PREFSMAN->m_sMemoryCardDir[p] != "" )
			FILEMAN->Mount( "dir", PREFSMAN->m_sMemoryCardDir[p], MemCardDirs[p] );
	}

	InitMachineScoresFromDisk();
}

ProfileManager::~ProfileManager()
{
	SaveMachineScoresToDisk();
}

void ProfileManager::GetLocalProfileIDs( vector<CString> &asProfileIDsOut )
{
	GetDirListing( USER_PROFILES_DIR "*", asProfileIDsOut, true, false );
}

void ProfileManager::GetLocalProfileNames( vector<CString> &asNamesOut )
{
	CStringArray vsProfileIDs;
	GetLocalProfileIDs( vsProfileIDs );
	for( unsigned i=0; i<vsProfileIDs.size(); i++ )
	{
		CString sProfileID = vsProfileIDs[i];

		Profile pro;
		pro.LoadFromIni( USER_PROFILES_DIR + sProfileID + SLASH + PROFILE_FILE );
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

	// Load scores into SONGMAN
	PROFILEMAN->ReadCategoryScoresFromDir( m_sProfileDir[pn], (MemoryCard)pn );
	PROFILEMAN->ReadSongScoresFromDir( m_sProfileDir[pn], (MemoryCard)pn );
	PROFILEMAN->ReadCourseScoresFromDir( m_sProfileDir[pn], (MemoryCard)pn );

	// apply saved default modifiers if any
	if( m_Profile[pn].m_bUsingProfileDefaultModifiers )
	{
		GAMESTATE->m_PlayerOptions[pn].Init();
		GAMESTATE->ApplyModifiers( pn, m_Profile[pn].m_sDefaultModifiers );
	}

	return true;
}

bool ProfileManager::CreateProfile( CString sProfileDir, CString sName )
{
	ASSERT( !sName.empty() );

	bool bResult;

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
	CString sProfileID = PREFSMAN->m_sDefaultLocalProfileID[pn];
	if( sProfileID.empty() )
	{
		m_sProfileDir[pn] = "";
		return false;
	}

	CString sDir = USER_PROFILES_DIR + sProfileID + SLASH;

	return LoadProfile( pn, sDir, false );
}

bool ProfileManager::IsMemoryCardInserted( PlayerNumber pn )
{
	return FILEMAN->MountpointIsReady( MemCardDirs[pn] );
}

bool ProfileManager::LoadProfileFromMemoryCard( PlayerNumber pn )
{
	CString sDir = MemCardDirs[pn];
	if( !FILEMAN->IsMounted(sDir) )
		return false;
	
	m_bUsingMemoryCard[pn] = true;
	bool bResult;
	bResult = LoadProfile( pn, sDir, false );
	return bResult;
}
			
bool ProfileManager::LoadFirstAvailableProfile( PlayerNumber pn )
{
#ifndef _XBOX
	// mount card
	if( !PREFSMAN->m_sMemoryCardMountCommand[pn].empty() )
		system( PREFSMAN->m_sMemoryCardMountCommand[pn] );

	if( IsMemoryCardInserted(pn) )
	{
		if( LoadProfileFromMemoryCard(pn) )
			return true;
	
		CString sDir = MemCardDirs[pn];
		CreateProfile( sDir, NEW_MEM_CARD_NAME );
		if( LoadProfileFromMemoryCard(pn) )
			return true;
	}
#endif

	if( LoadDefaultProfileFromMachine(pn) )
		return true;
	
	return false;
}

bool ProfileManager::SaveProfile( PlayerNumber pn )
{
	if( m_sProfileDir[pn].empty() )
		return false;

	m_Profile[pn].SaveToIni( m_sProfileDir[pn]+PROFILE_FILE );

	SaveCategoryScoresToDir( m_sProfileDir[pn], (MemoryCard)pn );
	SaveSongScoresToDir( m_sProfileDir[pn], (MemoryCard)pn );
	SaveCourseScoresToDir( m_sProfileDir[pn], (MemoryCard)pn );
	SaveStatsWebPageToDir( m_sProfileDir[pn], (MemoryCard)pn );
	
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

	ini.GetValue( "Profile", "DisplayName",						m_sName );
	ini.GetValue( "Profile", "LastUsedHighScoreName",			m_sLastUsedHighScoreName );
	ini.GetValue( "Profile", "UsingProfileDefaultModifiers",	m_bUsingProfileDefaultModifiers );
	ini.GetValue( "Profile", "DefaultModifiers",				m_sDefaultModifiers );
	ini.GetValue( "Profile", "TotalPlays",						m_iTotalPlays );
	ini.GetValue( "Profile", "TotalPlaySeconds",				m_iTotalPlaySeconds );
	ini.GetValue( "Profile", "TotalGameplaySeconds",			m_iTotalGameplaySeconds );
	ini.GetValue( "Profile", "CurrentCombo",					m_iCurrentCombo );
	return true;
}

bool Profile::SaveToIni( CString sIniPath )
{
	IniFile ini( sIniPath );
	ini.SetValue( "Profile", "DisplayName",						m_sName );
	ini.SetValue( "Profile", "LastUsedHighScoreName",			m_sLastUsedHighScoreName );
	ini.SetValue( "Profile", "UsingProfileDefaultModifiers",	m_bUsingProfileDefaultModifiers );
	ini.SetValue( "Profile", "DefaultModifiers",				m_sDefaultModifiers );
	ini.SetValue( "Profile", "TotalPlays",						m_iTotalPlays );
	ini.SetValue( "Profile", "TotalPlaySeconds",				m_iTotalPlaySeconds );
	ini.SetValue( "Profile", "TotalGameplaySeconds",			m_iTotalGameplaySeconds );
	ini.SetValue( "Profile", "CurrentCombo",					m_iCurrentCombo );
	ini.WriteFile();
	return true;
}

bool ProfileManager::CreateLocalProfile( CString sName )
{
	ASSERT( !sName.empty() );

	//
	// Find a free directory name in the profiles directory
	//
	CString sProfileID, sProfileDir;
	const int MAX_TRIES = 1000;
    int i;
	for( i=0; i<MAX_TRIES; i++ )
	{
		sProfileID = ssprintf("%08d",i);
		sProfileDir = USER_PROFILES_DIR + sProfileID;
		if( !DoesFileExist(sProfileDir) )
			break;
	}
	if( i == MAX_TRIES )
		return false;
	sProfileDir += SLASH;

	Profile pro;
	pro.m_sName = sName;

	bool bResult;
	bResult = pro.SaveToIni( sProfileDir + PROFILE_FILE );
	if( !bResult )
		return false;

	FlushDirCache();
	return true;
}

bool ProfileManager::RenameLocalProfile( CString sProfileID, CString sNewName )
{
	ASSERT( !sProfileID.empty() );

	CString sProfileDir = USER_PROFILES_DIR + sProfileID;
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

bool ProfileManager::DeleteLocalProfile( CString sProfileID )
{
	// delete all files in profile dir
	CString sProfileDir = USER_PROFILES_DIR + sProfileID;
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

void ProfileManager::SaveMachineScoresToDisk()
{
	m_MachineProfile.SaveToIni( MACHINE_PROFILE_DIR PROFILE_FILE );

	SaveCategoryScoresToDir( MACHINE_PROFILE_DIR, MEMORY_CARD_MACHINE );
	SaveSongScoresToDir( MACHINE_PROFILE_DIR, MEMORY_CARD_MACHINE );
	SaveCourseScoresToDir( MACHINE_PROFILE_DIR, MEMORY_CARD_MACHINE );
	SaveStatsWebPageToDir( MACHINE_PROFILE_DIR, MEMORY_CARD_MACHINE );
}

void ProfileManager::CategoryData::AddHighScore( HighScore hs, int &iIndexOut )
{
	int i;
	for( i=0; i<(int)vHighScores.size(); i++ )
	{
		if( hs >= vHighScores[i] )
			break;
	}
	if( i < NUM_RANKING_LINES )
	{
		vHighScores.insert( vHighScores.begin()+i, hs );
		iIndexOut = i;
		if( int(vHighScores.size()) > NUM_RANKING_LINES )
			vHighScores.erase( vHighScores.begin()+NUM_RANKING_LINES, vHighScores.end() );
	}
}


#define WARN_AND_RETURN { LOG->Warn("Error parsing file '%s' at %s:%d",fn.c_str(),__FILE__,__LINE__); return; }


void ProfileManager::ReadSongScoresFromDir( CString sDir, MemoryCard mc )
{
	CString fn = sDir + SONG_SCORES_FILE;

	RageFile f;
	if( !f.Open(fn, RageFile::READ) )
	{
		LOG->Warn( "Couldn't open file '%s'", fn.c_str() );
		return;
	}

	int version;
	if( !FileRead(f, version) )
		WARN_AND_RETURN;
	if( version != STEPS_SCORES_VERSION )
		WARN_AND_RETURN;

	int iNumSongs;
	if( !FileRead(f, iNumSongs) )
		WARN_AND_RETURN;

	for( int s=0; s<iNumSongs; s++ )
	{
		CString sSongDir;
		if( !FileRead(f, sSongDir) )
			WARN_AND_RETURN;

		Song* pSong = SONGMAN->GetSongFromDir( sSongDir );

		int iNumNotes;
		if( !FileRead(f, iNumNotes) )
			WARN_AND_RETURN;

		for( int n=0; n<iNumNotes; n++ )
		{
			StepsType nt;
			if( !FileRead(f, (int&)nt) )
				WARN_AND_RETURN;

			Difficulty dc;
			if( !FileRead(f, (int&)dc) )
				WARN_AND_RETURN;
		
			CString sDescription;
			if( !FileRead(f, sDescription) )
				WARN_AND_RETURN;

			// Even if pSong or pNotes is null, we still have to skip over that data.

			Steps* pNotes = NULL;
			if( pSong )
			{
				if( dc==DIFFICULTY_INVALID )
					pNotes = pSong->GetStepsByDescription( nt, sDescription );
				else
					pNotes = pSong->GetStepsByDifficulty( nt, dc );
			}
			
			int iNumTimesPlayed;
			if( !FileRead(f, iNumTimesPlayed) )
				WARN_AND_RETURN;

			if( pNotes )
				pNotes->m_MemCardDatas[mc].iNumTimesPlayed = iNumTimesPlayed;

			if( pNotes )
				pNotes->m_MemCardDatas[mc].vHighScores.resize(NUM_RANKING_LINES);

			for( int l=0; l<NUM_RANKING_LINES; l++ )
			{
				CString sName;
				if( !FileRead(f, sName) )
					WARN_AND_RETURN;

				Grade grade;
				if( !FileRead(f, (int&)grade) )
					WARN_AND_RETURN;
				CLAMP( grade, (Grade)0, (Grade)(NUM_GRADES-1) );

				int iScore;
				if( !FileRead(f, iScore) )
					WARN_AND_RETURN;

				float fPercentDP;
				if( !FileRead(f, fPercentDP) )
					WARN_AND_RETURN;

				if( pNotes )
				{
					pNotes->m_MemCardDatas[mc].vHighScores[l].sName = sName;
					pNotes->m_MemCardDatas[mc].vHighScores[l].grade = grade;
					pNotes->m_MemCardDatas[mc].vHighScores[l].iScore = iScore;
					pNotes->m_MemCardDatas[mc].vHighScores[l].fPercentDP = fPercentDP;
				}
			}
		}
	}
}


void ProfileManager::ReadCategoryScoresFromDir( CString sDir, MemoryCard mc )
{
	CString fn = sDir + CATEGORY_SCORES_FILE;

	RageFile f;
	if( !f.Open(fn, RageFile::READ) )
	{
		LOG->Warn( "Couldn't open file '%s'", fn.c_str() );
		return;
	}
	
	int version;
	if( !FileRead(f, version) )
		WARN_AND_RETURN;
	if( version != CATEGORY_RANKING_VERSION )
		WARN_AND_RETURN;

	for( int st=0; st<NUM_STEPS_TYPES; st++ )
	{
		for( int rc=0; rc<NUM_RANKING_CATEGORIES; rc++ )
		{
			m_CategoryDatas[mc][st][rc].vHighScores.resize(NUM_RANKING_LINES);
			for( int l=0; l<NUM_RANKING_LINES; l++ )
			{
				CString sName;
				if( !FileRead(f, sName) )
					WARN_AND_RETURN;
				int iScore;
				if( !FileRead(f, iScore) )
					WARN_AND_RETURN;
				m_CategoryDatas[mc][st][rc].vHighScores[l].sName = sName;
				m_CategoryDatas[mc][st][rc].vHighScores[l].iScore = iScore;
			}
		}
	}
}

void ProfileManager::ReadCourseScoresFromDir( CString sDir, MemoryCard mc )
{
	CString fn = sDir + COURSE_SCORES_FILE;

	RageFile f;
	if( !f.Open(fn, RageFile::READ) )
	{
		LOG->Warn( "Couldn't open file '%s'", fn.c_str() );
		return;
	}
	
	int version;
	if( !FileRead(f, version) )
		WARN_AND_RETURN;
	if( version != COURSE_SCORES_VERSION )
		WARN_AND_RETURN;

	int iNumCourses;
	if( !FileRead(f, iNumCourses) )
		WARN_AND_RETURN;

	for( int c=0; c<iNumCourses; c++ )
	{
		CString sPath;
		if( !FileRead(f, sPath) )
			WARN_AND_RETURN;

		Course* pCourse = SONGMAN->GetCourseFromPath( sPath );
		if( pCourse == NULL )
			pCourse = SONGMAN->GetCourseFromName( sPath );
		
		// even if we don't find the Course*, we still have to read past the input
	
		int NumStepsPlayed = 0;
		if( !FileRead(f, NumStepsPlayed) )
			WARN_AND_RETURN;

		while( NumStepsPlayed-- )
		{
			int st;
			if( !FileRead(f, st) )
				WARN_AND_RETURN;

			int iNumTimesPlayed;
			if( !FileRead(f, iNumTimesPlayed) )
				WARN_AND_RETURN;

			if( pCourse )
				pCourse->m_MemCardDatas[st][mc].iNumTimesPlayed = iNumTimesPlayed;
			if( pCourse )
				pCourse->m_MemCardDatas[st][mc].vHighScores.resize(NUM_RANKING_LINES);

			for( int l=0; l<NUM_RANKING_LINES; l++ )
			{
				CString sName;
				if( !FileRead(f, sName) )
					WARN_AND_RETURN;

				int iScore;
				if( !FileRead(f, iScore) )
					WARN_AND_RETURN;

				float fSurviveTime;
				if( !FileRead(f, fSurviveTime) )
					WARN_AND_RETURN;

				if( pCourse && st < NUM_STEPS_TYPES )
				{
					pCourse->m_MemCardDatas[st][mc].vHighScores[l].sName = sName;
					pCourse->m_MemCardDatas[st][mc].vHighScores[l].iScore = iScore;
					pCourse->m_MemCardDatas[st][mc].vHighScores[l].fSurviveTime = fSurviveTime;
				}
			}
		}
	}
}

void ProfileManager::InitMachineScoresFromDisk()
{
	// read old style notes scores
	ReadSM300NoteScores();

	// category ranking
	ReadCategoryScoresFromDir( MACHINE_PROFILE_DIR, MEMORY_CARD_MACHINE );
	ReadSongScoresFromDir( MACHINE_PROFILE_DIR, MEMORY_CARD_MACHINE );
	ReadCourseScoresFromDir( MACHINE_PROFILE_DIR, MEMORY_CARD_MACHINE );

	if( !m_MachineProfile.LoadFromIni(MACHINE_PROFILE_DIR PROFILE_FILE) )
	{
		CreateProfile(MACHINE_PROFILE_DIR, "Machine");
		m_MachineProfile.LoadFromIni(MACHINE_PROFILE_DIR PROFILE_FILE);
	}
}

void ProfileManager::ReadSM300NoteScores()
{
	if( !DoesFileExist(SM_300_STATISTICS_FILE) )
		return;

	IniFile ini;
	ini.SetPath( SM_300_STATISTICS_FILE );

	// load song statistics
	const IniFile::key* pKey = ini.GetKey( "Statistics" );
	if( pKey )
	{
		for( IniFile::key::const_iterator iter = pKey->begin();
			iter != pKey->end();
			iter++ )
		{
			CString name = iter->first;
			CString value = iter->second;

			// Each value has the format "SongName::StepsType::StepsDescription=TimesPlayed::TopGrade::TopScore::MaxCombo".
			char szSongDir[256];
			char szStepsType[256];
			char szStepsDescription[256];
			int iRetVal;

			// Parse for Song name and Notes name
			iRetVal = sscanf( name, "%[^:]::%[^:]::%[^:]", szSongDir, szStepsType, szStepsDescription );
			if( iRetVal != 3 )
				continue;	// this line doesn't match what is expected
	
			CString sSongDir = FixSlashes( szSongDir );
			
			// Search for the corresponding Song pointer.
			Song* pSong = SONGMAN->GetSongFromDir( sSongDir );
			if( pSong == NULL )	// didn't find a match
				continue;	// skip this entry

			StepsType st = GAMEMAN->StringToNotesType( szStepsType );
			Difficulty dc = StringToDifficulty( szStepsDescription );

			// Search for the corresponding Notes pointer.
			Steps* pNotes = pSong->GetStepsByDifficulty( st, dc );
			if( pNotes == NULL )	// didn't find a match
				continue;	// skip this entry


			// Parse the Notes statistics.
			char szGradeLetters[10];	// longest possible string is "AAA"
			int iMaxCombo;	// throw away

			pNotes->m_MemCardDatas[MEMORY_CARD_MACHINE].vHighScores.resize(1);

			iRetVal = sscanf( 
				value, 
				"%d::%[^:]::%d::%d", 
				&pNotes->m_MemCardDatas[MEMORY_CARD_MACHINE].iNumTimesPlayed,
				szGradeLetters,
				&pNotes->m_MemCardDatas[MEMORY_CARD_MACHINE].vHighScores[0].iScore,
				&iMaxCombo
			);
			if( iRetVal != 4 )
				continue;

			pNotes->m_MemCardDatas[MEMORY_CARD_MACHINE].vHighScores[0].grade = StringToGrade( szGradeLetters );
		}
	}
}


void ProfileManager::SaveCategoryScoresToDir( CString sDir, MemoryCard mc )
{
	CString fn = sDir + CATEGORY_SCORES_FILE;

	LOG->Trace("SongManager::SaveCategoryRankingsToFile");

	RageFile f;
	if( !f.Open(fn, RageFile::WRITE) )
	{
		LOG->Warn( "Couldn't open file '%s'", fn.c_str() );
		return;
	}

	FileWrite( f, CATEGORY_RANKING_VERSION );

	for( int st=0; st<NUM_STEPS_TYPES; st++ )
	{
		for( int rc=0; rc<NUM_RANKING_CATEGORIES; rc++ )
		{
			m_CategoryDatas[mc][st][rc].vHighScores.resize(NUM_RANKING_LINES);
			for( int l=0; l<NUM_RANKING_LINES; l++ )
			{
				// tricky:  wipe out "name to fill in" markers
				if( IsRankingToFillIn(m_CategoryDatas[mc][st][rc].vHighScores[l].sName) )
					m_CategoryDatas[mc][st][rc].vHighScores[l].sName = "";

				FileWrite( f, m_CategoryDatas[mc][st][rc].vHighScores[l].sName );
				FileWrite( f, m_CategoryDatas[mc][st][rc].vHighScores[l].iScore );
			}
		}
	}
}

void ProfileManager::SaveCourseScoresToDir( CString sDir, MemoryCard mc )
{
	CString fn = sDir + COURSE_SCORES_FILE;

	LOG->Trace("SongManager::SaveCourseScoresToFile");

	RageFile f;
	if( !f.Open(fn, RageFile::WRITE) )
	{
		LOG->Warn( "Couldn't open file '%s'", fn.c_str() );
		return;
	}
	
	FileWrite( f, COURSE_SCORES_VERSION );

	vector<Course*> vpCourses;
	SONGMAN->GetAllCourses( vpCourses, true );

	FileWrite( f, vpCourses.size() );

	for( unsigned c=0; c<vpCourses.size(); c++ )	// foreach course
	{
		Course* pCourse = vpCourses[c];
		ASSERT(pCourse);

		if( pCourse->m_bIsAutogen )
			FileWrite( f, pCourse->m_sName );
		else
			FileWrite( f, pCourse->m_sPath );

		int NumStepsPlayed = 0;
		int st;
		for( st=0; st<NUM_STEPS_TYPES; st++ )
			if( pCourse->m_MemCardDatas[st][mc].iNumTimesPlayed )
				++NumStepsPlayed;
		FileWrite( f, NumStepsPlayed );

		for( st=0; st<NUM_STEPS_TYPES; st++ )
		{
			if( !pCourse->m_MemCardDatas[st][mc].iNumTimesPlayed )
				continue;
			--NumStepsPlayed;

			FileWrite( f, st );
			FileWrite( f, pCourse->m_MemCardDatas[st][mc].iNumTimesPlayed );
			pCourse->m_MemCardDatas[st][mc].vHighScores.resize(NUM_RANKING_LINES);
			for( int l=0; l<NUM_RANKING_LINES; l++ )
			{
				// tricky:  wipe out "name to fill in" markers
				if( IsRankingToFillIn(pCourse->m_MemCardDatas[st][mc].vHighScores[l].sName) )
					pCourse->m_MemCardDatas[st][mc].vHighScores[l].sName = "";

				FileWrite( f, pCourse->m_MemCardDatas[st][mc].vHighScores[l].sName );
				FileWrite( f, pCourse->m_MemCardDatas[st][mc].vHighScores[l].iScore );
				FileWrite( f, pCourse->m_MemCardDatas[st][mc].vHighScores[l].fSurviveTime );
			}
		}
		ASSERT( !NumStepsPlayed );
	}
}

void ProfileManager::SaveSongScoresToDir( CString sDir, MemoryCard mc )
{
	CString fn = sDir + SONG_SCORES_FILE;

	LOG->Trace("SongManager::SaveSongScoresToFile %s", fn.c_str());

	RageFile f;
	if( !f.Open(fn, RageFile::WRITE) )
	{
		LOG->Warn( "Couldn't open file '%s'", fn.c_str() );
		return;
	}
	
	FileWrite( f, STEPS_SCORES_VERSION );

	const vector<Song*> &vpSongs = SONGMAN->GetAllSongs();
	
	FileWrite( f, vpSongs.size() );

	for( unsigned s=0; s<vpSongs.size(); s++ )	// foreach song
	{
		Song* pSong = vpSongs[s];
		ASSERT(pSong);

		/* If the song has never been played, don't write anything.  This keeps
		 * us from saving a dozen copies of each song for all autogen difficulties,
		 * since most people only use a couple game modes. */
		vector<Steps*> vNotes;
		for( unsigned i=0; i<pSong->m_apNotes.size(); ++i )
		{
			Steps* pNotes = pSong->m_apNotes[i];
			if( !pNotes->m_MemCardDatas[mc].iNumTimesPlayed )
				continue;
			vNotes.push_back( pNotes );
		}

		FileWrite( f, pSong->GetSongDir() );
		FileWrite( f, vNotes.size() );

		if( vNotes.size() == 0 )
			continue;	// skip	

		for( unsigned n=0; n<vNotes.size(); n++ )
		{
			Steps* pNotes = vNotes[n];
			ASSERT(pNotes);

			FileWrite( f, pNotes->m_StepsType );
			FileWrite( f, pNotes->GetDifficulty() );
			FileWrite( f, pNotes->GetDescription() );
			FileWrite( f, pNotes->m_MemCardDatas[mc].iNumTimesPlayed );

			pNotes->m_MemCardDatas[mc].vHighScores.resize(NUM_RANKING_LINES);
			for( int l=0; l<NUM_RANKING_LINES; l++ )
			{
				// tricky:  wipe out "name to fill in" markers
				if( IsRankingToFillIn(pNotes->m_MemCardDatas[mc].vHighScores[l].sName) )
					pNotes->m_MemCardDatas[mc].vHighScores[l].sName = "";

				FileWrite( f, pNotes->m_MemCardDatas[mc].vHighScores[l].sName );
				FileWrite( f, pNotes->m_MemCardDatas[mc].vHighScores[l].grade );
				FileWrite( f, pNotes->m_MemCardDatas[mc].vHighScores[l].iScore );
				FileWrite( f, pNotes->m_MemCardDatas[mc].vHighScores[l].fPercentDP );
			}
		}
	}
}

static CString HTMLQuoteDoubleQuotes( CString str )
{
	str.Replace( "\"", "&quot;" );
	return str;
}

void ProfileManager::SaveStatsWebPageToDir( CString sDir, MemoryCard mc )
{
	CString fn = sDir + STATS_HTML_FILE;

	//
	// Get Profile
	//
	Profile* pProfile;
	if( mc == MEMORY_CARD_MACHINE )
		pProfile = GetMachineProfile();
	else
		pProfile = GetProfile( (PlayerNumber)mc );
	ASSERT(pProfile);

	//
	// Open file
	//
	RageFile f;
	if( !f.Open( fn, RageFile::WRITE ) )
	{
		LOG->Warn( "Couldn't open file '%s'", fn.c_str() );
		return;
	}

	//
	// Make local song list
	//
	vector<Song*> vpSongs = SONGMAN->GetAllSongs();
	SortSongPointerArrayByGroupAndTitle( vpSongs );

	//
	// print HTML headers
	//
	f.PutLine( "<html>" );
	f.PutLine( "<head>" );
	f.PutLine( "<META HTTP-EQUIV=\"Content-Type\" CONTENT=\"text/html; charset=UTF-8\">" );
	f.PutLine( ssprintf("<title>%s</title>", PRODUCT_NAME_VER) );
	f.PutLine( "<link rel='stylesheet' type='text/css' href='style.css'>" );
	f.PutLine( "</head>" );
	f.PutLine( "<body>" );

#define PRINT_LINK(szName,szLink)	f.Write( ssprintf("<p><a href='%s'>%s</a></p>\n",szLink,szName) )
#define PRINT_LINE_S(szName,sVal) 	f.Write( ssprintf("<p>%s = <b>%s</b></p>\n",szName,sVal.c_str()) )
#define PRINT_LINE_B(szName,bVal) 	f.Write( ssprintf("<p>%s = <b>%s</b></p>\n",szName,(bVal)?"yes":"no") )
#define PRINT_LINE_I(szName,iVal) 	f.Write( ssprintf("<p>%s = <b>%d</b></p>\n",szName,iVal) )
	//
	// Print table of contents
	//
	f.PutLine( "<h2><a name='Table of Contents'>Table of Contents</a></h2>\n" );
	f.PutLine( "<div class='section1'>\n" );
	f.PutLine( "<h3>Sections</h3>\n" );
	PRINT_LINK( "My Statistics", "#My Statistics" );
	PRINT_LINK( "Difficulty Table", "#Difficulty Table" );
	PRINT_LINK( "Song/Steps List", "#Song/Steps List" );
	f.PutLine( "</div>\n" );



	//
	// Print My Statistics
	//
	f.PutLine( "<h2><a name='My Statistics'>My Statistics</a> <a href='#Table of Contents'>(top)</a></h2>\n" );
	f.PutLine( "<div class='section1'>\n" );
	CString sName = pProfile->m_sLastUsedHighScoreName.empty() ?
		pProfile->m_sName :
		pProfile->m_sLastUsedHighScoreName;
	f.PutLine( ssprintf("<h3>%s</h3>\n",sName.c_str()) );
	PRINT_LINE_S( "LastUsedHighScoreName", pProfile->m_sLastUsedHighScoreName );
	PRINT_LINE_B( "UsingProfileDefaultModifiers", pProfile->m_bUsingProfileDefaultModifiers );
	PRINT_LINE_S( "DefaultModifiers", pProfile->m_sDefaultModifiers );
	PRINT_LINE_I( "TotalPlays", pProfile->m_iTotalPlays );
	PRINT_LINE_I( "TotalPlaySeconds", pProfile->m_iTotalPlaySeconds );
	PRINT_LINE_I( "TotalGameplaySeconds", pProfile->m_iTotalGameplaySeconds );
	PRINT_LINE_I( "CurrentCombo", pProfile->m_iCurrentCombo );
	f.PutLine( "</div>\n" );

	//
	// Print Difficulty tables
	//
	f.PutLine( "<h2><a name='Difficulty Table'>Difficulty Table</a> <a href='#Table of Contents'>(top)</a></h2>\n" );
	for( StepsType st=(StepsType)0; st<NUM_STEPS_TYPES; st=(StepsType)(st+1) )
	{
		unsigned i;

		// don't print this table if there are no songs that have this StepsType 
		bool bOneSongHasStepsForThisStepsType = false;
		for( i=0; i<vpSongs.size(); i++ )
		{
			Song* pSong = vpSongs[i];
			vector<Steps*> vpSteps;
			pSong->GetSteps( vpSteps, st, DIFFICULTY_INVALID, -1, -1, "", false );
			if( !vpSteps.empty() )
			{
				bOneSongHasStepsForThisStepsType = true;
				break;
			}
		}
		
		if( !bOneSongHasStepsForThisStepsType )
			continue;	// don't print this table

 		f.PutLine( "<div class='section1'>\n" );
		f.PutLine( ssprintf("<h3>%s</h3>\n", GAMEMAN->NotesTypeToString(st).c_str()) );
		f.PutLine( "<table border='1' cellpadding='2' cellspacing='0'>\n" );

		// table header row
		f.Write( "<tr><td>&nbsp;</td>" );
		for( unsigned k=0; k<NUM_DIFFICULTIES; k++ )
		{
			Difficulty d = (Difficulty)k;
			f.PutLine( ssprintf("<td>%s</td>", Capitalize(DifficultyToString(d).Left(3)).c_str()) );
		}
		f.PutLine( "</tr>" );

		// table body rows
		for( i=0; i<vpSongs.size(); i++ )
		{
			Song* pSong = vpSongs[i];

			f.PutLine( "<tr>" );
			
			f.Write( ssprintf("<td><a href='#%u'>%s</a></td>", 
				pSong,	// use pointer value as the hash
				pSong->GetFullDisplayTitle().c_str()) );

			for( Difficulty dc=(Difficulty)0; dc<NUM_DIFFICULTIES; dc=(Difficulty)(dc+1) )
			{
				Steps* pSteps = pSong->GetStepsByDifficulty( st, dc, false );
				if( pSteps )
					f.PutLine( ssprintf("<td><p align='right'><a href='#%u'>%d</a></p></td>", 
					pSteps,		// use pointer value as the hash
					pSteps->GetMeter()) );
				else
					f.PutLine( "<td>&nbsp;</td>" );
			}

			f.Write( "</tr>" );
		}

		f.PutLine( "</table>\n" );
		f.PutLine( "</div>\n" );
	}


	//
	// Print song list
	//
	f.PutLine( "<h2><a name='Song/Steps List'>Song/Steps List</a> <a href='#Table of Contents'>(top)</a></h2>\n" );
	for( unsigned i=0; i<vpSongs.size(); i++ )
	{
		Song* pSong = vpSongs[i];
		vector<Steps*> vpSteps = pSong->GetAllSteps();

		/* XXX: We can't call pSong->HasBanner on every song; it'll effectively re-traverse the entire
		 * song directory tree checking if each banner file really exists.
		 *
		 * (Note for testing this: remember that we'll cache directories for a time; this is only slow if
		 * the directory cache expires before we get here.) */
		/* Don't print the song banner anyway since this is going on the memory card. -Chris */
		//CString sImagePath = pSong->HasBanner() ? pSong->GetBannerPath() : (pSong->HasBackground() ? pSong->GetBackgroundPath() : "" );
		f.PutLine( "<div class='section1'>\n" );
		f.PutLine( ssprintf("<h3><a name='%u'>%s</a></h3>\n",
			pSong,	// use pointer value as the hash
			pSong->GetFullDisplayTitle().c_str()) );
		PRINT_LINE_S( "Artist", pSong->GetDisplayArtist() );
		PRINT_LINE_S( "GroupName", pSong->m_sGroupName );
		float fMinBPM, fMaxBPM;
		pSong->GetDisplayBPM( fMinBPM, fMaxBPM );
		CString sBPM = (fMinBPM==fMaxBPM) ? ssprintf("%.1f",fMinBPM) : ssprintf("%.1f - %.1f",fMinBPM,fMaxBPM);
		PRINT_LINE_S( "BPM", sBPM );
		PRINT_LINE_I( "NumTimesPlayed", pSong->GetNumTimesPlayed(mc) );
		PRINT_LINE_S( "Credit", pSong->m_sCredit );
		PRINT_LINE_S( "MusicLength", SecondsToTime(pSong->m_fMusicLengthSeconds) );
		PRINT_LINE_B( "Lyrics", !pSong->m_sLyricsFile.empty() );
		f.PutLine( "</div>\n" );

		//
		// Print Steps list
		//
		for( unsigned j=0; j<vpSteps.size(); j++ )
		{
			Steps* pSteps = vpSteps[j];
			if( pSteps->IsAutogen() )
				continue;	// skip autogen
			f.PutLine( "<div class='section2'>\n" );
			f.PutLine( ssprintf("<h4><a name='%u'>%s - %s</a></h4>\n",
				pSteps,	// use pointer value as the hash
				GAMEMAN->NotesTypeToString(pSteps->m_StepsType).c_str(),
				DifficultyToString(pSteps->GetDifficulty()).c_str()) );
			PRINT_LINE_I( "NumTimesPlayed", pSteps->m_MemCardDatas[mc].iNumTimesPlayed );
			f.PutLine( "</div>\n" );
		}

	}

	f.PutLine( "</body>" );
	f.PutLine( "</html>" );

	//
	// Copy CSS file from theme.  If the copy fails, oh well...
	// 
	CString sStyleFile = THEME->GetPathToO(STYLE_CSS_FILE);
	CopyFile2( sStyleFile, sDir+STYLE_CSS_FILE );
		
}
