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
#include "Steps.h"
#include "Course.h"
#include "GameManager.h"
#include "ProductInfo.h"
#include "RageUtil.h"
#include "ThemeManager.h"
#include "Bookkeeper.h"
#include <time.h>
#include "MemoryCardManager.h"
#include "XmlFile.h"


ProfileManager*	PROFILEMAN = NULL;	// global and accessable from anywhere in our program


#define PROFILE_FILE		"Profile.ini"

#define CATEGORY_SCORES_FILE	"CategoryScores.dat"
#define SONG_SCORES_FILE		"SongScores.dat"
#define COURSE_SCORES_FILE		"CourseScores.dat"
#define STATS_HTML_FILE			"stats.html"
#define STYLE_CSS_FILE			"style.css"
#define NEW_MEM_CARD_NAME		""
#define NEW_PROFILE_NAME		""

#define SM_300_STATISTICS_FILE	"statistics.ini"

#define USER_PROFILES_DIR		"Data/LocalProfiles/"
#define MACHINE_PROFILE_DIR		"Data/MachineProfile/"

const int CATEGORY_RANKING_VERSION = 6;
const int SONG_SCORES_VERSION = 9;
const int COURSE_SCORES_VERSION = 8;

#define STATS_TITLE									THEME->GetMetric("ProfileManager","StatsTitle")

static const char *MEM_CARD_DIR[NUM_PLAYERS] =
{
	/* @ is importast; see RageFileManager LoadedDriver::GetPath */
	"@mc1/",
	"@mc2/",
};


ProfileManager::ProfileManager()
{
	for( int p=0; p<NUM_PLAYERS; p++ )
		m_bWasLoadedFromMemoryCard[p] = false;

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
		pro.LoadFromIni( USER_PROFILES_DIR + sProfileID + "/" + PROFILE_FILE );
		asNamesOut.push_back( pro.m_sName );
	}
}


bool ProfileManager::LoadProfile( PlayerNumber pn, CString sProfileDir, bool bIsMemCard )
{
	ASSERT( !sProfileDir.empty() );
	ASSERT( sProfileDir.Right(1) == "/" );

	m_sProfileDir[pn] = sProfileDir;
	m_bWasLoadedFromMemoryCard[pn] = bIsMemCard;

	bool bResult = m_Profile[pn].LoadFromIni( m_sProfileDir[pn]+PROFILE_FILE );
	if( !bResult )
	{
		LOG->Warn( "Attempting to load profile from '%s' and does not exist", sProfileDir.c_str() );
		UnloadProfile( pn );
		return false;
	}

	ReadCategoryScoresFromDir( m_sProfileDir[pn], (ProfileSlot)pn );
	ReadSongScoresFromDir( m_sProfileDir[pn], (ProfileSlot)pn );
	ReadCourseScoresFromDir( m_sProfileDir[pn], (ProfileSlot)pn );

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

	CString sDir = USER_PROFILES_DIR + sProfileID + "/";

	return LoadProfile( pn, sDir, false );
}

bool ProfileManager::LoadProfileFromMemoryCard( PlayerNumber pn )
{
	UnloadProfile( pn );
#ifndef _XBOX
	// moust card
	if( MEMCARDMAN->GetCardState(pn) == MEMORY_CARD_STATE_READY )
	{
		FILEMAN->Mount( "dir", MEMCARDMAN->GetOsMountDir(pn), MEM_CARD_DIR[pn] );

		CString sDir = MEM_CARD_DIR[pn];

		DEBUG_ASSERT( FILEMAN->IsMounted(sDir) );	// should be called only if we've already mousted
		
		// tack on a subdirectory so that we don't write everything to the root
		sDir += PREFSMAN->m_sMemoryCardProfileSubdir;
		sDir += '/'; 

		bool bResult;
		bResult = LoadProfile( pn, sDir, true );
		if( bResult )
			return true;
	
		CreateMemoryCardProfile( pn );
		
		bResult = LoadProfile( pn, sDir, true );
		return bResult;
	}
#endif
	return false;
}
			
bool ProfileManager::CreateMemoryCardProfile( PlayerNumber pn )
{
	CString sDir = MEM_CARD_DIR[pn];
	
	DEBUG_ASSERT( FILEMAN->IsMounted(sDir) );	// should be called only if we've already mousted

	// tack on a subdirectory so that we don't write everything to the root
	sDir += PREFSMAN->m_sMemoryCardProfileSubdir;
	sDir += '/'; 

	return CreateProfile( sDir, NEW_MEM_CARD_NAME );
}
			
bool ProfileManager::LoadFirstAvailableProfile( PlayerNumber pn )
{
	if( LoadProfileFromMemoryCard(pn) )
		return true;

	if( LoadDefaultProfileFromMachine(pn) )
		return true;
	
	return false;
}

bool ProfileManager::SaveProfile( PlayerNumber pn )
{
	if( m_sProfileDir[pn].empty() )
		return false;

	m_Profile[pn].SaveToIni( m_sProfileDir[pn]+PROFILE_FILE );

	SaveCategoryScoresToDir( m_sProfileDir[pn], (ProfileSlot)pn );
	SaveSongScoresToDir( m_sProfileDir[pn], (ProfileSlot)pn );
	SaveSongScoresToDirXml( m_sProfileDir[pn], (ProfileSlot)pn );
	SaveCourseScoresToDir( m_sProfileDir[pn], (ProfileSlot)pn );
	SaveStatsWebPageToDir( m_sProfileDir[pn], (ProfileSlot)pn );
	
	return true;
}

void ProfileManager::UnloadProfile( PlayerNumber pn )
{
	m_sProfileDir[pn] = "";
	m_bWasLoadedFromMemoryCard[pn] = false;
	m_Profile[pn].Init();
}

Profile* ProfileManager::GetProfile( PlayerNumber pn )
{
	if( m_sProfileDir[pn].empty() )
		return NULL;
	else
		return &m_Profile[pn];
}

CString ProfileManager::GetPlayerName( PlayerNumber pn )
{
	Profile *prof = ProfileManager::GetProfile( pn );
	if( prof )
		return prof->m_sLastUsedHighScoreName;

	const char *names[NUM_PLAYERS] = { "PLAYER 1", "PLAYER 2" };
	return names[pn];
}

bool Profile::LoadFromIni( CString sIniPath )
{
	Init();

	CStringArray asBits;
	split( Dirname(sIniPath), "/", asBits, true );
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

	unsigned i;
	for( i=0; i<NUM_PLAY_MODES; i++ )
		ini.GetValue( "Profile", "NumSongsPlayedByPlayMode"+Capitalize(PlayModeToString((PlayMode)i)), m_iNumSongsPlayedByPlayMode[i] );
	for( i=0; i<NUM_STYLES; i++ )
		ini.GetValue( "Profile", "NumSongsPlayedByStyle"+Capitalize(GAMEMAN->GetGameDefForGame(GAMEMAN->GetStyleDefForStyle((Style)i)->m_Game)->m_szName)+Capitalize(GAMEMAN->GetStyleDefForStyle((Style)i)->m_szName), m_iNumSongsPlayedByStyle[i] );
	for( i=0; i<NUM_DIFFICULTIES; i++ )
		ini.GetValue( "Profile", "NumSongsPlayedByDifficulty"+Capitalize(DifficultyToString((Difficulty)i)), m_iNumSongsPlayedByDifficulty[i] );
	for( i=0; i<MAX_METER+1; i++ )
		ini.GetValue( "Profile", "NumSongsPlayedByMeter"+ssprintf("%d",i), m_iNumSongsPlayedByMeter[i] );
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

	unsigned i;
	for( i=0; i<NUM_PLAY_MODES; i++ )
		ini.SetValue( "Profile", "NumSongsPlayedByPlayMode"+Capitalize(PlayModeToString((PlayMode)i)), m_iNumSongsPlayedByPlayMode[i] );
	for( i=0; i<NUM_STYLES; i++ )
		ini.SetValue( "Profile", "NumSongsPlayedByStyle"+Capitalize(GAMEMAN->GetGameDefForGame(GAMEMAN->GetStyleDefForStyle((Style)i)->m_Game)->m_szName)+Capitalize(GAMEMAN->GetStyleDefForStyle((Style)i)->m_szName), m_iNumSongsPlayedByStyle[i] );
	for( i=0; i<NUM_DIFFICULTIES; i++ )
		ini.SetValue( "Profile", "NumSongsPlayedByDifficulty"+Capitalize(DifficultyToString((Difficulty)i)), m_iNumSongsPlayedByDifficulty[i] );
	for( i=0; i<MAX_METER+1; i++ )
		ini.SetValue( "Profile", "NumSongsPlayedByMeter"+ssprintf("%d",i), m_iNumSongsPlayedByMeter[i] );
	
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
	sProfileDir += "/";

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
	CString sProfileFile = sProfileDir + "/" + PROFILE_FILE;

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
	GetDirListing( sProfileDir + "/*", asFilesToDelete, false, true );
	for( unsigned i=0; i<asFilesToDelete.size(); i++ )
		FILEMAN->Remove( asFilesToDelete[i] );

	// remove profile dir
	return FILEMAN->Remove( sProfileDir );
}

void ProfileManager::SaveMachineScoresToDisk()
{
	m_MachineProfile.SaveToIni( MACHINE_PROFILE_DIR PROFILE_FILE );

	SaveCategoryScoresToDir( MACHINE_PROFILE_DIR, PROFILE_SLOT_MACHINE );
	SaveSongScoresToDir( MACHINE_PROFILE_DIR, PROFILE_SLOT_MACHINE );
	SaveSongScoresToDirXml( MACHINE_PROFILE_DIR, PROFILE_SLOT_MACHINE );
	SaveCourseScoresToDir( MACHINE_PROFILE_DIR, PROFILE_SLOT_MACHINE );
	SaveStatsWebPageToDir( MACHINE_PROFILE_DIR, PROFILE_SLOT_MACHINE );
}


#define WARN_AND_RETURN { LOG->Warn("Error parsing file '%s' at %s:%d",fn.c_str(),__FILE__,__LINE__); return; }

void ProfileManager::ReadSongScoresFromDir( CString sDir, ProfileSlot slot )
{
	Profile* pProfile = GetProfile( slot );
	ASSERT( pProfile );

	CString fn = sDir + SONG_SCORES_FILE;

	RageFile f;
	if( !f.Open(fn, RageFile::READ) )
	{
		LOG->Trace( "Couldn't open file \"%s\": %s", fn.c_str(), f.GetError().c_str() );
		return;
	}

	int version;
	if( !FileRead(f, version) )
		WARN_AND_RETURN;
	if( version != SONG_SCORES_VERSION )
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
			StepsType st;
			if( !FileRead(f, (int&)st) )
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
					pNotes = pSong->GetStepsByDescription( st, sDescription );
				else
					pNotes = pSong->GetStepsByDifficulty( st, dc );
			}
			
			int iNumTimesPlayed;
			if( !FileRead(f, iNumTimesPlayed) )
				WARN_AND_RETURN;

			HighScoreList &hsl = pProfile->GetStepsHighScoreList(pNotes);
			
			if( pNotes )
				hsl.iNumTimesPlayed = iNumTimesPlayed;

			int iNumHighScores;
			if( !FileRead(f, iNumHighScores) )
				WARN_AND_RETURN;

			if( pNotes )
				hsl.vHighScores.resize( iNumHighScores );

			int l;
			for( l=0; l<iNumHighScores; l++ )
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

				if( pNotes == NULL )
					continue;	// ignore this high score
				
				hsl.vHighScores[l].sName = sName;
				hsl.vHighScores[l].grade = grade;
				hsl.vHighScores[l].iScore = iScore;
				hsl.vHighScores[l].fPercentDP = fPercentDP;
			}

			// ignore all high scores that are 0
			for( l=0; l<iNumHighScores; l++ )
			{
				if( pNotes && hsl.vHighScores[l].iScore <= 0 )
				{
					hsl.vHighScores.resize(l);
					break;
				}
			}
		}
	}
}


void ProfileManager::ReadCategoryScoresFromDir( CString sDir, ProfileSlot slot )
{
	Profile* pProfile = GetProfile( slot );
	ASSERT( pProfile );

	CString fn = sDir + CATEGORY_SCORES_FILE;

	RageFile f;
	if( !f.Open(fn, RageFile::READ) )
	{
		LOG->Trace( "Couldn't open file \"%s\": %s", fn.c_str(), f.GetError().c_str() );
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
			int iNumHighScores;
			if( !FileRead(f, iNumHighScores) )
				WARN_AND_RETURN;

			HighScoreList &hsl = pProfile->GetCategoryHighScoreList( (StepsType)st, (RankingCategory)rc );
			hsl.vHighScores.resize( iNumHighScores );

			for( int l=0; l<iNumHighScores; l++ )
			{
				CString sName;
				if( !FileRead(f, sName) )
					WARN_AND_RETURN;
			
				int iScore;
				if( !FileRead(f, iScore) )
					WARN_AND_RETURN;
				
				float fPercentDP;
				if( !FileRead(f, fPercentDP) )
					WARN_AND_RETURN;
				
				hsl.vHighScores[l].sName = sName;
				hsl.vHighScores[l].iScore = iScore;
				hsl.vHighScores[l].fPercentDP = fPercentDP;
			}
		}
	}
}

void ProfileManager::ReadCourseScoresFromDir( CString sDir, ProfileSlot slot )
{
	Profile* pProfile = GetProfile( slot );
	ASSERT( pProfile );

	CString fn = sDir + COURSE_SCORES_FILE;

	RageFile f;
	if( !f.Open(fn, RageFile::READ) )
	{
		LOG->Trace( "Couldn't open file \"%s\": %s", fn.c_str(), f.GetError().c_str() );
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
	
		int NumStepsTypesPlayed = 0;
		if( !FileRead(f, NumStepsTypesPlayed) )
			WARN_AND_RETURN;

		while( NumStepsTypesPlayed-- )
		{
			int st;
			if( !FileRead(f, st) )
				WARN_AND_RETURN;

			int iNumTimesPlayed;
			if( !FileRead(f, iNumTimesPlayed) )
				WARN_AND_RETURN;

			HighScoreList &hsl = pProfile->GetCourseHighScoreList( pCourse, (StepsType)st );

			if( pCourse )
				hsl.iNumTimesPlayed = iNumTimesPlayed;

			int iNumHighScores;
			if( !FileRead(f, iNumHighScores) )
				WARN_AND_RETURN;

			if( pCourse )
				hsl.vHighScores.resize(iNumHighScores);

			for( int l=0; l<iNumHighScores; l++ )
			{
				CString sName;
				if( !FileRead(f, sName) )
					WARN_AND_RETURN;

				int iScore;
				if( !FileRead(f, iScore) )
					WARN_AND_RETURN;

				float fPercentDP;
				if( !FileRead(f, fPercentDP) )
					WARN_AND_RETURN;

				float fSurviveTime;
				if( !FileRead(f, fSurviveTime) )
					WARN_AND_RETURN;

				if( pCourse && st < NUM_STEPS_TYPES )
				{
					hsl.vHighScores[l].sName = sName;
					hsl.vHighScores[l].iScore = iScore;
					hsl.vHighScores[l].fPercentDP = fPercentDP;
					hsl.vHighScores[l].fSurviveTime = fSurviveTime;
				}
			}
		}
	}
}

void ProfileManager::InitMachineScoresFromDisk()
{
	// read old style notes scores
//	ReadSM300NoteScores();

	// category ranking
	ReadCategoryScoresFromDir( MACHINE_PROFILE_DIR, PROFILE_SLOT_MACHINE );
	ReadSongScoresFromDir( MACHINE_PROFILE_DIR, PROFILE_SLOT_MACHINE );
	ReadCourseScoresFromDir( MACHINE_PROFILE_DIR, PROFILE_SLOT_MACHINE );

	if( !m_MachineProfile.LoadFromIni(MACHINE_PROFILE_DIR PROFILE_FILE) )
	{
		CreateProfile(MACHINE_PROFILE_DIR, "Machine");
		m_MachineProfile.LoadFromIni(MACHINE_PROFILE_DIR PROFILE_FILE);
	}
}

/*
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
			
			// Search for the corresponding Song poister.
			Song* pSong = SONGMAN->GetSongFromDir( sSongDir );
			if( pSong == NULL )	// didn't find a match
				continue;	// skip this estry

			StepsType st = GAMEMAN->StringToNotesType( szStepsType );
			Difficulty dc = StringToDifficulty( szStepsDescription );

			// Search for the corresponding Notes poister.
			Steps* pNotes = pSong->GetStepsByDifficulty( st, dc );
			if( pNotes == NULL )	// didn't find a match
				continue;	// skip this estry


			// Parse the Notes statistics.
			char szGradeLetters[10];	// longest possible string is "AAA"
			int iMaxCombo;	// throw away

			pNotes->m_MemCardDatas[PROFILE_SLOT_MACHINE].vHighScores.resize(1);

			iRetVal = sscanf( 
				value, 
				"%d::%[^:]::%d::%d", 
				&pNotes->m_MemCardDatas[PROFILE_SLOT_MACHINE].iNumTimesPlayed,
				szGradeLetters,
				&pNotes->m_MemCardDatas[PROFILE_SLOT_MACHINE].vHighScores[0].iScore,
				&iMaxCombo
			);
			if( iRetVal != 4 )
				continue;

			pNotes->m_MemCardDatas[PROFILE_SLOT_MACHINE].vHighScores[0].grade = StringToGrade( szGradeLetters );
		}
	}
}
*/

void ProfileManager::SaveCategoryScoresToDir( CString sDir, ProfileSlot slot )
{
	Profile* pProfile = GetProfile( slot );
	ASSERT( pProfile );

	CString fn = sDir + CATEGORY_SCORES_FILE;

	LOG->Trace("SongManager::SaveCategoryRankingsToFile");

	RageFile f;
	if( !f.Open(fn, RageFile::WRITE) )
	{
		LOG->Warn( "Couldn't open file \"%s\" for writing: %s", fn.c_str(), f.GetError().c_str() );
		return;
	}

	FileWrite( f, CATEGORY_RANKING_VERSION );

	for( int st=0; st<NUM_STEPS_TYPES; st++ )
	{
		for( int rc=0; rc<NUM_RANKING_CATEGORIES; rc++ )
		{
			HighScoreList &hsl = pProfile->GetCategoryHighScoreList( (StepsType)st, (RankingCategory)rc );

			FileWrite( f, hsl.vHighScores.size() );

			for( unsigned l=0; l<hsl.vHighScores.size(); l++ )
			{
				// tricky:  wipe out "name to fill in" markers
				if( IsRankingToFillIn(hsl.vHighScores[l].sName) )
					hsl.vHighScores[l].sName = "";

				FileWrite( f, hsl.vHighScores[l].sName );
				FileWrite( f, hsl.vHighScores[l].iScore );
				FileWrite( f, hsl.vHighScores[l].fPercentDP );
			}
		}
	}
}

void ProfileManager::SaveCourseScoresToDir( CString sDir, ProfileSlot slot )
{
	Profile* pProfile = GetProfile( slot );
	ASSERT( pProfile );

	CString fn = sDir + COURSE_SCORES_FILE;

	LOG->Trace("SongManager::SaveCourseScoresToFile");

	RageFile f;
	if( !f.Open(fn, RageFile::WRITE) )
	{
		LOG->Warn( "Couldn't open file \"%s\" for writing: %s", fn.c_str(), f.GetError().c_str() );
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

		int NumStepsTypesPlayed = 0;
		int st;
		for( st=0; st<NUM_STEPS_TYPES; st++ )
		{
			HighScoreList &hsl = pProfile->GetCourseHighScoreList( pCourse, (StepsType)st );
			if( hsl.iNumTimesPlayed )
				++NumStepsTypesPlayed;
		}
		FileWrite( f, NumStepsTypesPlayed );

		for( st=0; st<NUM_STEPS_TYPES; st++ )
		{
			HighScoreList &hsl = pProfile->GetCourseHighScoreList( pCourse, (StepsType)st );
			if( hsl.iNumTimesPlayed == 0 )
				continue;
			--NumStepsTypesPlayed;

			FileWrite( f, st );
			FileWrite( f, hsl.iNumTimesPlayed );
			FileWrite( f, hsl.vHighScores.size() );
			for( unsigned l=0; l<hsl.vHighScores.size(); l++ )
			{
				// tricky:  wipe out "name to fill in" markers
				if( IsRankingToFillIn(hsl.vHighScores[l].sName) )
					hsl.vHighScores[l].sName = "";

				FileWrite( f, hsl.vHighScores[l].sName );
				FileWrite( f, hsl.vHighScores[l].iScore );
				FileWrite( f, hsl.vHighScores[l].fPercentDP );
				FileWrite( f, hsl.vHighScores[l].fSurviveTime );
			}
		}
		ASSERT( !NumStepsTypesPlayed );
	}
}

void ProfileManager::SaveSongScoresToDir( CString sDir, ProfileSlot slot )
{
	Profile* pProfile = GetProfile( slot );
	ASSERT( pProfile );

	CString fn = sDir + SONG_SCORES_FILE;

	LOG->Trace("SongManager::SaveSongScoresToFile %s", fn.c_str());

	RageFile f;
	if( !f.Open(fn, RageFile::WRITE) )
	{
		LOG->Warn( "Couldn't open file \"%s\" for writing: %s", fn.c_str(), f.GetError().c_str() );
		return;
	}
	
	FileWrite( f, SONG_SCORES_VERSION );

	const vector<Song*> &vpSongs = SONGMAN->GetAllSongs();
	
	FileWrite( f, vpSongs.size() );

	for( unsigned s=0; s<vpSongs.size(); s++ )	// foreach song
	{
		Song* pSong = vpSongs[s];
		ASSERT(pSong);

		/* If the song has never been played, don't write anything.  This keeps
		 * us from saving a dozen copies of each song for all autogen difficulties,
		 * since most people only use a couple game modes. */
		vector<Steps*> vNotesToWrite;
		for( unsigned i=0; i<pSong->m_apNotes.size(); ++i )
		{
			Steps* pNotes = pSong->m_apNotes[i];
			HighScoreList &hsl = pProfile->GetStepsHighScoreList( pNotes );
			if( hsl.iNumTimesPlayed == 0  &&  hsl.vHighScores.empty() )
				continue;
			vNotesToWrite.push_back( pNotes );
		}

		FileWrite( f, pSong->GetSongDir() );
		FileWrite( f, vNotesToWrite.size() );

		for( unsigned n=0; n<vNotesToWrite.size(); n++ )
		{
			Steps* pNotes = vNotesToWrite[n];
			ASSERT(pNotes);
		
			HighScoreList &hsl = pProfile->GetStepsHighScoreList( pNotes );

			FileWrite( f, pNotes->m_StepsType );
			FileWrite( f, pNotes->GetDifficulty() );
			FileWrite( f, pNotes->GetDescription() );
			FileWrite( f, hsl.iNumTimesPlayed );

			FileWrite( f, hsl.vHighScores.size() );

			for( int l=0; l<(int)hsl.vHighScores.size(); l++ )
			{
				// tricky:  wipe out "name to fill in" markers
				if( IsRankingToFillIn(hsl.vHighScores[l].sName) )
					hsl.vHighScores[l].sName = "";

				FileWrite( f, hsl.vHighScores[l].sName );
				FileWrite( f, hsl.vHighScores[l].grade );
				FileWrite( f, hsl.vHighScores[l].iScore );
				FileWrite( f, hsl.vHighScores[l].fPercentDP );
			}
		}
	}
}

void ProfileManager::SaveSongScoresToDirXml( CString sDir, ProfileSlot slot )
{
	Profile* pProfile = GetProfile( slot );
	ASSERT( pProfile );

	CString fn = sDir + SONG_SCORES_FILE+".xml";

	LOG->Trace("SongManager::SaveSongScoresToFile %s", fn.c_str());

	RageFile f;
	if( !f.Open(fn, RageFile::WRITE) )
	{
		LOG->Warn( "Couldn't open file \"%s\" for writing: %s", fn.c_str(), f.GetError().c_str() );
		return;
	}
	
	XNode xml;
	xml.name = "SongScores";

	const vector<Song*> &vpSongs = SONGMAN->GetAllSongs();
	
	for( unsigned s=0; s<vpSongs.size(); s++ )	// foreach song
	{
		Song* pSong = vpSongs[s];
		ASSERT(pSong);

		/* If the song has never been played, don't write anything.  This keeps
		 * us from saving a dozen copies of each song for all autogen difficulties,
		 * since most people only use a couple game modes. */
		vector<Steps*> vNotesToWrite;
		for( unsigned i=0; i<pSong->m_apNotes.size(); ++i )
		{
			Steps* pNotes = pSong->m_apNotes[i];
			HighScoreList &hsl = pProfile->GetStepsHighScoreList( pNotes );
			if( hsl.iNumTimesPlayed == 0  &&  hsl.vHighScores.empty() )
				continue;
			vNotesToWrite.push_back( pNotes );
		}

		if( vNotesToWrite.empty() )
			continue;

		LPXNode pSongNode = xml.AppendChild( "Song" );
		pSongNode->AppendChild( "Dir", pSong->GetSongDir() );

		for( unsigned n=0; n<vNotesToWrite.size(); n++ )
		{
			LPXNode pStepsNode = pSongNode->AppendChild( "Steps" );

			Steps* pNotes = vNotesToWrite[n];
			ASSERT(pNotes);
		
			HighScoreList &hsl = pProfile->GetStepsHighScoreList( pNotes );
		
			pStepsNode->AppendChild( "StepsType", pNotes->m_StepsType );
			pStepsNode->AppendChild( "Difficulty", pNotes->GetDifficulty() );
			pStepsNode->AppendChild( "Description", pNotes->GetDescription() );
			pStepsNode->AppendChild( "NumTimesPlayed", hsl.iNumTimesPlayed );

			for( int l=0; l<(int)hsl.vHighScores.size(); l++ )
			{
				HighScore &hs = hsl.vHighScores[l];

				LPXNode pHighScoreNode = pSongNode->AppendChild( "HighScore" );

				// tricky:  wipe out "name to fill in" markers
				if( IsRankingToFillIn(hs.sName) )
					hs.sName = "";

				pHighScoreNode->AppendChild( "Name", hs.sName );
				pHighScoreNode->AppendChild( "Grade", hs.grade );
				pHighScoreNode->AppendChild( "Score", hs.iScore );
				pHighScoreNode->AppendChild( "Percent", hs.fPercentDP );
			}
		}
	}
	
	FileWrite( f, xml.GetXML() );
}

/* static CString HTMLQuoteDoubleQuotes( CString str )
{
	str.Replace( "\"", "&quot;" );
	return str;
} */

void ProfileManager::SaveStatsWebPageToDir( CString sDir, ProfileSlot slot )
{
	CString fn = sDir + STATS_HTML_FILE;

	LOG->Trace( "Writing %s ...", fn.c_str() );
	//
	// Get Profile
	//
	Profile* pProfile;
	if( slot == PROFILE_SLOT_MACHINE )
		pProfile = GetMachineProfile();
	else
		pProfile = GetProfile( (PlayerNumber)slot );
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
	// Gather data
	//
	vector<Song*> vpSongs = SONGMAN->GetAllSongs();
	vector<Steps*> vpAllSteps;
	map<Steps*,Song*> mapStepsToSong;
	{
		for( unsigned i=0; i<vpSongs.size(); i++ )
		{
			Song* pSong = vpSongs[i];
			vector<Steps*> vpSteps = pSong->GetAllSteps();
			for( unsigned j=0; j<vpSteps.size(); j++ )
			{
				Steps* pSteps = vpSteps[j];
				if( pSteps->IsAutogen() )
					continue;	// skip
				vpAllSteps.push_back( pSteps );
				mapStepsToSong[pSteps] = pSong;
			}
		}
	}
	vector<Course*> vpCourses;
	SONGMAN->GetAllCourses( vpCourses, false );

	//
	// Calculate which StepTypes to show
	//
	vector<StepsType> vStepsTypesToShow;
	{
		for( StepsType st=(StepsType)0; st<NUM_STEPS_TYPES; st=(StepsType)(st+1) )
		{
			// don't show if there are no Steps of this StepsType 
			bool bOneSongHasStepsForThisStepsType = false;
			for( unsigned i=0; i<vpSongs.size(); i++ )
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

			if( bOneSongHasStepsForThisStepsType )
				vStepsTypesToShow.push_back( st );
		}
	}

	//
	// prist HTML headers
	//
	{
		f.PutLine( "<html>" );
		f.PutLine( "<head>" );
		f.PutLine( "<META HTTP-EQUIV=\"Costest-Type\" COstEst=\"text/html; charset=UTF-8\">" );
		f.PutLine( ssprintf("<title>%s</title>", STATS_TITLE.c_str() ) );
		f.PutLine( ssprintf("<link rel='stylesheet' type='text/css' href='%s'>",STYLE_CSS_FILE) );
		f.PutLine( "</head>" );
		f.PutLine( "<body>" );
	}

#define PRIst_SECTION_START(szName)				f.Write( ssprintf("<h2><a name='%s'>"szName"</a> <a href='#top'>(top)</a></h2>\n", szName) )
#define PRIst_SECTION_END						f.Write( "\n" )
#define PRIst_DIV_START(szName)					f.Write( ssprintf("<div class='section1'>\n" "<h3>%s</h3>\n", szName) )
#define PRIst_DIV_START_ANCHOR(uAnchor,szName)	f.Write( ssprintf("<div class='section1'>\n" "<h3><a name='%u'>%s</a></h3>\n", (unsigned)uAnchor, szName) )
#define PRIst_DIV_END							f.Write( "</div>\n" )
#define PRIst_DIV2_START(szName)				f.Write( ssprintf("<div class='section2'>\n" "<h3>%s</h3>\n", szName) )
#define PRIst_DIV2_START_ANCHOR(uAnchor,szName)	f.Write( ssprintf("<div class='section2'>\n" "<h3><a name='%u'>%s</a></h3>\n", (unsigned)uAnchor, szName) )
#define PRIst_DIV2_END							f.Write( "</div>\n" )
#define PRIst_LINK(szName,szLink)				f.Write( ssprintf("<p><a href='%s'>%s</a></p>\n",szLink,szName) )
#define PRIst_LINE_S(szName,sVal) 				f.Write( ssprintf("<p>%s = <b>%s</b></p>\n",szName,sVal.c_str()) )
#define PRIst_LINE_B(szName,bVal) 				f.Write( ssprintf("<p>%s = <b>%s</b></p>\n",szName,(bVal)?"yes":"no") )
#define PRIst_LINE_I(szName,iVal) 				f.Write( ssprintf("<p>%s = <b>%d</b></p>\n",szName,iVal) )
#define PRIst_LINE_RANK(iRank,sName,iVal)		f.Write( ssprintf("<p><b>%d</b> - %s (%d)</p>\n",iRank,sName.c_str(),iVal) )
#define PRIst_LINE_RANK_LINK(iRank,sName,szLink,iVal)		f.Write( ssprintf("<p><b>%d</b> - <a href='%s'>%s</a> (%d)</p>\n",iRank,szLink,sName.c_str(),iVal) )

	//
	// Prist table of costests
	//
	{
		CString sName = 
			pProfile->m_sLastUsedHighScoreName.empty() ? 
			pProfile->m_sName :
			pProfile->m_sLastUsedHighScoreName;
	    time_t ltime = time( NULL );
		CString sTime = ctime( &ltime );

		f.Write( ssprintf("<h1><a name='top'>%s for %s - %s</a></h1>\n",STATS_TITLE.c_str(), sName.c_str(), sTime.c_str()) );
		PRIst_DIV_START("Table of Costests");
		PRIst_LINK( "Statistics", "#Statistics" );
		PRIst_LINK( "Popularity Lists", "#Popularity Lists" );
		PRIst_LINK( "Difficulty Table", "#Difficulty Table" );
		PRIst_LINK( "High Scores Table", "#High Scores Table" );
		PRIst_LINK( "Song/Steps List", "#Song/Steps List" );
		PRIst_LINK( "Bookkeeping", "#Bookkeeping" );
		PRIst_DIV_END;
	}

	//
	// Prist Statistics
	//
	LOG->Trace( "Writing stats ..." );
	{
		PRIst_SECTION_START( "Statistics" );

		// Memory card stats
		{
			PRIst_DIV_START( "This Profile" );
			PRIst_LINE_S( "Name", pProfile->m_sName );
			PRIst_LINE_S( "LastUsedHighScoreName", pProfile->m_sLastUsedHighScoreName );
			PRIst_LINE_B( "UsingProfileDefaultModifiers", pProfile->m_bUsingProfileDefaultModifiers );
			PRIst_LINE_S( "DefaultModifiers", pProfile->m_sDefaultModifiers );
			PRIst_LINE_I( "TotalPlays", pProfile->m_iTotalPlays );
			PRIst_LINE_I( "TotalPlaySeconds", pProfile->m_iTotalPlaySeconds );
			PRIst_LINE_I( "TotalGameplaySeconds", pProfile->m_iTotalGameplaySeconds );
			PRIst_LINE_I( "CurrentCombo", pProfile->m_iCurrentCombo );
			PRIst_DIV_END;
		}
		
		// Num Songs Played by PlayMode
		{
			PRIst_DIV_START( "Num Songs Played by PlayMode" );
			for( int i=0; i<NUM_PLAY_MODES; i++ )
				PRIst_LINE_I( PlayModeToString((PlayMode)i).c_str(), pProfile->m_iNumSongsPlayedByPlayMode[i] );
			PRIst_DIV_END;
		}

		// Num Songs Played by Style
		{
			PRIst_DIV_START( "Num Songs Played by Style" );
			for( int i=0; i<NUM_STYLES; i++ )
			{
				Style style = (Style)i;
				const StyleDef* pStyleDef = GAMEMAN->GetStyleDefForStyle(style);
				StepsType st = pStyleDef->m_StepsType;
				if( !pStyleDef->m_bUsedForGameplay )
					continue;	// skip
				// only show if this style plays a StepsType that we're showing
				if( find(vStepsTypesToShow.begin(),vStepsTypesToShow.end(),st) == vStepsTypesToShow.end() )
					continue;	// skip
				PRIst_LINE_I( pStyleDef->m_szName, pProfile->m_iNumSongsPlayedByStyle[i] );
			}
			PRIst_DIV_END;
		}

		// Num Songs Played by Difficulty
		{
			PRIst_DIV_START( "Num Songs Played by Difficulty" );
			for( int i=0; i<NUM_DIFFICULTIES; i++ )
				PRIst_LINE_I( DifficultyToString((Difficulty)i).c_str(), pProfile->m_iNumSongsPlayedByDifficulty[i] );
			PRIst_DIV_END;
		}

		// Num Songs Played by Meter
		{
			PRIst_DIV_START( "Num Songs Played by Meter" );
			for( int i=MAX_METER; i>=MIN_METER; i-- )
				PRIst_LINE_I( ssprintf("%d",i).c_str(), pProfile->m_iNumSongsPlayedByMeter[i] );
			PRIst_DIV_END;
		}

		PRIst_SECTION_END;
	}

	//
	// Prist Popularity Lists
	//
	{
		PRIst_SECTION_START( "Popularity Lists" );

		// Songs by popularity
		{
			unsigned uNumToShow = min( vpSongs.size(), (unsigned)100 );

			SortSongPointerArrayByMostPlayed( vpSongs, slot );
			PRIst_DIV_START( "Songs by Popularity" );
			for( unsigned i=0; i<uNumToShow; i++ )
			{
				Song* pSong = vpSongs[i];
				PRIst_LINE_RANK_LINK( i+1, pSong->GetFullDisplayTitle(), ssprintf("#%u",(unsigned)pSong).c_str(), PROFILEMAN->GetSongNumTimesPlayed(pSong,slot) );
			}
			PRIst_DIV_END;
		}

		// Steps by popularity
		{
			unsigned uNumToShow = min( vpAllSteps.size(), (unsigned)100 );

			SortStepsPointerArrayByMostPlayed( vpAllSteps, slot );
			PRIst_DIV_START( "Steps by Popularity" );
			for( unsigned i=0; i<uNumToShow; i++ )
			{
				Steps* pSteps = vpAllSteps[i];
				Song* pSong = mapStepsToSong[pSteps];
				CString s;
				s += pSong->GetFullDisplayTitle();
				s += " - ";
				s += GAMEMAN->NotesTypeToString(pSteps->m_StepsType);
				s += " ";
				s += DifficultyToString(pSteps->GetDifficulty());
				PRIst_LINE_RANK_LINK( i+1, s, ssprintf("#%u",(unsigned)pSteps).c_str(), pProfile->GetStepsNumTimesPlayed(pSteps) );
			}
			PRIst_DIV_END;
		}

		// Course by popularity
		{
			unsigned uNumToShow = min( vpCourses.size(), (unsigned)100 );

			SortCoursePointerArrayByMostPlayed( vpCourses, slot );
			PRIst_DIV_START( "Courses by Popularity" );
			for( unsigned i=0; i<uNumToShow; i++ )
			{
				Course* pCourse = vpCourses[i];
				PRIst_LINE_RANK_LINK( i+1, pCourse->m_sName, ssprintf("#%u",(unsigned)pCourse).c_str(), pProfile->GetCourseNumTimesPlayed(pCourse) );
			}
			PRIst_DIV_END;
		}

		PRIst_SECTION_END;
	}

	//
	// Prist High score tables
	//
	{
		SortSongPointerArrayByGroupAndTitle( vpSongs );

		PRIst_SECTION_START( "High Scores Table" );
		for( unsigned s=0; s<vStepsTypesToShow.size(); s++ )
		{
			StepsType st = vStepsTypesToShow[s];

			unsigned i;

 			PRIst_DIV_START( GAMEMAN->NotesTypeToString(st).c_str() );
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
					(unsigned)pSong,	// use poister value as the hash
					pSong->GetFullDisplayTitle().c_str()) );

				for( Difficulty dc=(Difficulty)0; dc<NUM_DIFFICULTIES; dc=(Difficulty)(dc+1) )
				{
					Steps* pSteps = pSong->GetStepsByDifficulty( st, dc, false );
					if( pSteps )
					{
						HighScoreList &hsl = pProfile->GetStepsHighScoreList( pSteps );
						CString sHighScore;
						if( !hsl.vHighScores.empty() )
						{
							sHighScore += hsl.vHighScores[0].sName;
							sHighScore += "<br>";
							sHighScore += GradeToString( hsl.vHighScores[0].grade );
							sHighScore += "<br>";
							sHighScore += ssprintf("%d",hsl.vHighScores[0].iScore);
						}
						f.PutLine( ssprintf("<td><p align='right'><a href='#%u'>%s</a></p></td>", 
							(unsigned)pSteps,		// use poister value as the hash
							sHighScore.c_str()) );
					}
					else
					{
						f.PutLine( "<td>&nbsp;</td>" );
					}
				}

				f.Write( "</tr>" );
			}

			f.PutLine( "</table>\n" );
			PRIst_DIV_END;
		}
		PRIst_SECTION_END;
	}

	//
	// Prist Difficulty tables
	//
	{
		SortSongPointerArrayByGroupAndTitle( vpSongs );

		PRIst_SECTION_START( "Difficulty Table" );
		for( unsigned s=0; s<vStepsTypesToShow.size(); s++ )
		{
			StepsType st = vStepsTypesToShow[s];

			unsigned i;

 			PRIst_DIV_START( GAMEMAN->NotesTypeToString(st).c_str() );
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
					(unsigned)pSong,	// use poister value as the hash
					pSong->GetFullDisplayTitle().c_str()) );

				for( Difficulty dc=(Difficulty)0; dc<NUM_DIFFICULTIES; dc=(Difficulty)(dc+1) )
				{
					Steps* pSteps = pSong->GetStepsByDifficulty( st, dc, false );
					if( pSteps )
					{
						f.PutLine( ssprintf("<td><p align='right'><a href='#%u'>%d</a></p></td>", 
						(unsigned)pSteps,		// use poister value as the hash
						pSteps->GetMeter()) );
					}
					else
					{
						f.PutLine( "<td>&nbsp;</td>" );
					}
				}

				f.Write( "</tr>" );
			}

			f.PutLine( "</table>\n" );
			PRIst_DIV_END;
		}
		PRIst_SECTION_END;
	}

	//
	// Prist song list
	//
	LOG->Trace( "Writing song list ..." );
	{
		PRIst_SECTION_START( "Song/Steps List" );
		for( unsigned i=0; i<vpSongs.size(); i++ )
		{
			Song* pSong = vpSongs[i];
			vector<Steps*> vpSteps = pSong->GetAllSteps();

			/* XXX: We can't call pSong->HasBanner on every song; it'll effectively re-traverse the estire
			 * song directory tree checking if each banner file really exists.
			 *
			 * (Note for testing this: remember that we'll cache directories for a time; this is only slow if
			 * the directory cache expires before we get here.) */
			/* Don't prist the song banner anyway since this is going on the memory card. -Chris */
			//CString sImagePath = pSong->HasBanner() ? pSong->GetBannerPath() : (pSong->HasBackground() ? pSong->GetBackgroundPath() : "" );
			PRIst_DIV_START_ANCHOR( /*Song primary key*/pSong, pSong->GetFullDisplayTitle().c_str() );
			PRIst_LINE_S( "Artist", pSong->GetDisplayArtist() );
			PRIst_LINE_S( "GroupName", pSong->m_sGroupName );
			float fMinBPM, fMaxBPM;
			pSong->GetDisplayBPM( fMinBPM, fMaxBPM );
			CString sBPM = (fMinBPM==fMaxBPM) ? ssprintf("%.1f",fMinBPM) : ssprintf("%.1f - %.1f",fMinBPM,fMaxBPM);
			PRIst_LINE_S( "BPM", sBPM );
			PRIst_LINE_I( "NumTimesPlayed", PROFILEMAN->GetSongNumTimesPlayed(pSong,slot) );
			PRIst_LINE_S( "Credit", pSong->m_sCredit );
			PRIst_LINE_S( "MusicLength", SecondsToTime(pSong->m_fMusicLengthSeconds) );
			PRIst_LINE_B( "Lyrics", !pSong->m_sLyricsFile.empty() );
			PRIst_DIV_END;

			//
			// Prist Steps list
			//
			for( unsigned j=0; j<vpSteps.size(); j++ )
			{
				Steps* pSteps = vpSteps[j];
				if( pSteps->IsAutogen() )
					continue;	// skip autogen
				HighScoreList &hsl = pProfile->GetStepsHighScoreList( pSteps );
				CString s = 
					GAMEMAN->NotesTypeToString(pSteps->m_StepsType) + 
					" - " +
					DifficultyToString(pSteps->GetDifficulty());
				PRIst_DIV2_START_ANCHOR( /*Steps primary key*/pSteps, s.c_str() );	// use poister value as the hash
				PRIst_LINE_I( "NumTimesPlayed", hsl.iNumTimesPlayed );
				
				for( unsigned i=0; i<hsl.vHighScores.size(); i++ )
				{
					HighScore &hs = hsl.vHighScores[i];
					CString sName = ssprintf("#%d",i+1);
					CString sHSName = hs.sName.empty() ? "????" : hs.sName;
					CString sValue = ssprintf("%s, %s, %i, %.2f%%", sHSName.c_str(), GradeToString(hs.grade).c_str(), hs.iScore, hs.fPercentDP*100);
					PRIst_LINE_S( sName.c_str(), sValue );
				}
				f.PutLine( "</div>\n" );
				PRIst_DIV2_END;
			}
		}
		PRIst_SECTION_END;
	}

	//
	// Prist Bookkeeping
	//
	{
		PRIst_SECTION_START( "Bookkeeping" );

		// GetCoinsLastDays
		{
			int coins[NUM_LAST_DAYS];
			BOOKKEEPER->GetCoinsLastDays( coins );
			PRIst_DIV_START( ssprintf("Coins for Last %d Days",NUM_LAST_DAYS).c_str() );
			for( int i=0; i<NUM_LAST_DAYS; i++ )
			{
				CString sDay = (i==0) ? "Today" : ssprintf("%d day(s) ago",i);
				PRIst_LINE_I( sDay.c_str(), coins[i] );
			}
			PRIst_DIV_END;
		}

		// GetCoinsLastWeeks
		{
			int coins[NUM_LAST_WEEKS];
			BOOKKEEPER->GetCoinsLastWeeks( coins );
			PRIst_DIV_START( ssprintf("Coins for Last %d Weeks",NUM_LAST_WEEKS).c_str() );
			for( int i=0; i<NUM_LAST_WEEKS; i++ )
			{
				CString sWeek = (i==0) ? "This week" : ssprintf("%d week(s) ago",i);
				PRIst_LINE_I( sWeek.c_str(), coins[i] );
			}
			PRIst_DIV_END;
		}

		// GetCoinsByDayOfWeek
		{
			int coins[DAYS_IN_WEEK];
			BOOKKEEPER->GetCoinsByDayOfWeek( coins );
			PRIst_DIV_START( "Coins by Day of Week" );
			for( int i=0; i<DAYS_IN_WEEK; i++ )
			{
				CString sDay = DAY_OF_WEEK_TO_NAME[i];
				PRIst_LINE_I( sDay.c_str(), coins[i] );
			}
			PRIst_DIV_END;
		}

		// GetCoinsByHour
		{
			int coins[HOURS_PER_DAY];
			BOOKKEEPER->GetCoinsByHour( coins );
			PRIst_DIV_START( ssprintf("Coins for Last %d Hours",HOURS_PER_DAY).c_str() );
			for( int i=0; i<HOURS_PER_DAY; i++ )
			{
				CString sHour = ssprintf("hour %d",i);
				PRIst_LINE_I( sHour.c_str(), coins[i] );
			}
			PRIst_DIV_END;
		}


		PRIst_SECTION_END;
	}
	
	PRIst_SECTION_START( "End of File" );
	PRIst_SECTION_END;

	f.PutLine( "</body>" );
	f.PutLine( "</html>" );

	//
	// Copy CSS file from theme.  If the copy fails, oh well...
	// 
	CString sStyleFile = THEME->GetPathToO("ProfileManager style.css");
	FileCopy( sStyleFile, sDir+STYLE_CSS_FILE );
	LOG->Trace( "Done." );
		
}

bool ProfileManager::ProfileWasLoadedFromMemoryCard( PlayerNumber pn )
{
	return GetProfile(pn) && m_bWasLoadedFromMemoryCard[pn];
}

CString ProfileManager::GetProfileDir( ProfileSlot slot )
{
	switch( slot )
	{
	case PROFILE_SLOT_PLAYER_1:
	case PROFILE_SLOT_PLAYER_2:
		return m_sProfileDir[slot];
	case PROFILE_SLOT_MACHINE:
		return MACHINE_PROFILE_DIR;
	default:
		ASSERT(0);
	}
}

Profile* ProfileManager::GetProfile( ProfileSlot slot )
{
	switch( slot )
	{
	case PROFILE_SLOT_PLAYER_1:
	case PROFILE_SLOT_PLAYER_2:
		if( m_sProfileDir[slot].empty() )
			return NULL;
		else
			return &m_Profile[slot];
	case PROFILE_SLOT_MACHINE:
		return &m_MachineProfile;
	default:
		ASSERT(0);
	}
}


//
// Song stats
//
int ProfileManager::GetSongNumTimesPlayed( Song* pSong, ProfileSlot card ) const
{
	int iTotalNumTimesPlayed = 0;
	vector<Steps*> vpSteps;
	pSong->GetSteps( vpSteps );
	for( unsigned i=0; i<vpSteps.size(); i++ )
	{
		Steps* pSteps = vpSteps[i];
		iTotalNumTimesPlayed += PROFILEMAN->GetMachineProfile()->GetStepsHighScoreList(pSteps).iNumTimesPlayed;
	}

	return iTotalNumTimesPlayed;
}

void ProfileManager::AddStepsHighScore( const Steps* pSteps, PlayerNumber pn, HighScore hs, int &iPersonalIndexOut, int &iMachineIndexOut )
{
	hs.sName = RANKING_TO_FILL_IN_MARKER[pn];
	if( PROFILEMAN->IsUsingProfile(pn) )
		PROFILEMAN->GetProfile(pn)->AddStepsHighScore( pSteps, hs, iPersonalIndexOut );
	else
		iPersonalIndexOut = -1;
	PROFILEMAN->GetMachineProfile()->AddStepsHighScore( pSteps, hs, iMachineIndexOut );
}

void ProfileManager::IncrementStepsPlayCount( const Steps* pSteps, PlayerNumber pn )
{
	if( PROFILEMAN->IsUsingProfile(pn) )
		PROFILEMAN->GetProfile(pn)->IncrementStepsPlayCount( pSteps );
	PROFILEMAN->GetMachineProfile()->IncrementStepsPlayCount( pSteps );
}

HighScore ProfileManager::GetHighScoreForDifficulty( const Song *s, const StyleDef *st, ProfileSlot slot, Difficulty dc )
{
	// return max grade of notes in difficulty class
	vector<Steps*> aNotes;
	s->GetSteps( aNotes, st->m_StepsType );
	SortNotesArrayByDifficulty( aNotes );

	Steps* pSteps = s->GetStepsByDifficulty( st->m_StepsType, dc );

	if( PROFILEMAN->IsUsingProfile(slot) )
		return PROFILEMAN->GetProfile(slot)->GetStepsHighScoreList(pSteps).GetTopScore();
	else
		return HighScore();
}


//
// Course stats
//
void ProfileManager::AddCourseHighScore( const Course* pCourse, StepsType st, PlayerNumber pn, HighScore hs, int &iPersonalIndexOut, int &iMachineIndexOut )
{
	hs.sName = RANKING_TO_FILL_IN_MARKER[pn];
	if( PROFILEMAN->IsUsingProfile(pn) )
		PROFILEMAN->GetProfile(pn)->AddCourseHighScore( pCourse, st, hs, iPersonalIndexOut );
	else
		iPersonalIndexOut = -1;
	PROFILEMAN->GetMachineProfile()->AddCourseHighScore( pCourse, st, hs, iMachineIndexOut );
}

void ProfileManager::IncrementCoursePlayCount( const Course* pCourse, StepsType st, PlayerNumber pn )
{
	if( PROFILEMAN->IsUsingProfile(pn) )
		PROFILEMAN->GetProfile(pn)->IncrementCoursePlayCount( pCourse, st );
	PROFILEMAN->GetMachineProfile()->IncrementCoursePlayCount( pCourse, st );
}


//
// Category stats
//
void ProfileManager::AddCategoryHighScore( StepsType st, RankingCategory rc, PlayerNumber pn, HighScore hs, int &iPersonalIndexOut, int &iMachineIndexOut )
{
	hs.sName = RANKING_TO_FILL_IN_MARKER[pn];
	if( PROFILEMAN->IsUsingProfile(pn) )
		PROFILEMAN->GetProfile(pn)->AddCategoryHighScore( st, rc, hs, iPersonalIndexOut );
	else
		iPersonalIndexOut = -1;
	PROFILEMAN->GetMachineProfile()->AddCategoryHighScore( st, rc, hs, iMachineIndexOut );
}

void ProfileManager::IncrementCategoryPlayCount( StepsType st, RankingCategory rc, PlayerNumber pn )
{
	if( PROFILEMAN->IsUsingProfile(pn) )
		PROFILEMAN->GetProfile(pn)->IncrementCategoryPlayCount( st, rc );
	PROFILEMAN->GetMachineProfile()->IncrementCategoryPlayCount( st, rc );
}

