#include "global.h"
/*
-----------------------------------------------------------------------------
 Class: Profile

 Desc: See header.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "Profile.h"
#include "RageUtil.h"
#include "PrefsManager.h"
#include "XmlFile.h"
#include "IniFile.h"
#include "GameManager.h"
#include "RageLog.h"
#include "RageFile.h"
#include "song.h"
#include "SongManager.h"
#include "Steps.h"
#include "Course.h"
#include <time.h>
#include "ThemeManager.h"
#include "Bookkeeper.h"
#include "CryptManager.h"
#include "PrefsManager.h"

//
// Old file versions for backward compatibility
//
#define SM_300_STATISTICS_INI	"statistics.ini"

#define SM_390A12_CATEGORY_SCORES_DAT	"CategoryScores.dat"
#define SM_390A12_SONG_SCORES_DAT		"SongScores.dat"
#define SM_390A12_COURSE_SCORES_DAT		"CourseScores.dat"
const int SM_390A12_CATEGORY_RANKING_VERSION = 6;
const int SM_390A12_SONG_SCORES_VERSION = 9;
const int SM_390A12_COURSE_SCORES_VERSION = 8;

//
// Current file versions
//
#define PROFILE_INI			"Profile.ini"
#define CATEGORY_SCORES_XML	"CategoryScores.xml"
#define SONG_SCORES_XML		"SongScores.xml"
#define COURSE_SCORES_XML	"CourseScores.xml"
#define STATS_HTML			"stats.html"
#define STYLE_CSS			"style.css"


#define DEFAULT_PROFILE_NAME	""

#define STATS_TITLE				THEME->GetMetric("ProfileManager","StatsTitle")


void Profile::InitGeneralData()
{
	// Fill in a default value in case ini doesn't have it.
	m_sName = DEFAULT_PROFILE_NAME;	
	m_sLastUsedHighScoreName = "";
	m_bUsingProfileDefaultModifiers = false;
	m_sDefaultModifiers = "";
	m_iTotalPlays = 0;
	m_iTotalPlaySeconds = 0;
	m_iTotalGameplaySeconds = 0;
	m_iCurrentCombo = 0;
	m_fWeightPounds = 0;
	m_fCaloriesBurned = 0;

	int i;
	for( i=0; i<NUM_PLAY_MODES; i++ )
		m_iNumSongsPlayedByPlayMode[i] = 0;
	for( i=0; i<NUM_STYLES; i++ )
		m_iNumSongsPlayedByStyle[i] = 0;
	for( i=0; i<NUM_DIFFICULTIES; i++ )
		m_iNumSongsPlayedByDifficulty[i] = 0;
	for( i=0; i<MAX_METER+1; i++ )
		m_iNumSongsPlayedByMeter[i] = 0;
}

void Profile::InitSongScores()
{
	m_StepsHighScores.clear();
}

void Profile::InitCourseScores()
{
	m_CourseHighScores.clear();
}

void Profile::InitCategoryScores()
{
	for( int st=0; st<NUM_STEPS_TYPES; st++ )
		for( int rc=0; rc<NUM_RANKING_CATEGORIES; rc++ )
			m_CategoryHighScores[st][rc].Init();
}

CString Profile::GetDisplayName()
{
	if( !m_sName.empty() )
		return m_sName;
	else if( !m_sLastUsedHighScoreName.empty() )
		return m_sLastUsedHighScoreName;
	else
		return "NO NAME";
}

CString Profile::GetDisplayCaloriesBurned()
{
	if( m_fWeightPounds == 0 )	// weight not entered
		return "N/A";
	else 
		return ssprintf("%iCal",m_fCaloriesBurned);
}

int Profile::GetTotalNumSongsPlayed()
{
	int iTotal = 0;
	for( int i=0; i<NUM_PLAY_MODES; i++ )
		iTotal += m_iNumSongsPlayedByPlayMode[i];
	return iTotal;
}

CString Profile::GetProfileDisplayNameFromDir( CString sDir )
{
	Profile pro;
	pro.LoadGeneralDataFromDir( sDir );
	return pro.GetDisplayName();
}

int Profile::GetSongNumTimesPlayed( const Song* pSong ) const
{
	int iTotalNumTimesPlayed = 0;
	vector<Steps*> vpSteps;
	pSong->GetSteps( vpSteps );
	for( unsigned i=0; i<vpSteps.size(); i++ )
	{
		const Steps* pSteps = vpSteps[i];
		iTotalNumTimesPlayed += ((Profile*) this)->GetStepsHighScoreList(pSteps).iNumTimesPlayed;
	}
	return iTotalNumTimesPlayed;
}

//
// Steps high scores
//
void Profile::AddStepsHighScore( const Steps* pSteps, HighScore hs, int &iIndexOut )
{
	std::map<const Steps*,HighScoresForASteps>::iterator iter = m_StepsHighScores.find( pSteps );
	if( iter == m_StepsHighScores.end() )
		m_StepsHighScores[pSteps].hs.AddHighScore( hs, iIndexOut );	// operator[] inserts into map
	else
		iter->second.hs.AddHighScore( hs, iIndexOut );
}

const HighScoreList& Profile::GetStepsHighScoreList( const Steps* pSteps ) const
{
	/* We're const, but insert a blank entry anyway if the requested pointer doesn't exist. */
	return ((Profile *) this)->m_StepsHighScores[pSteps].hs;
}

HighScoreList& Profile::GetStepsHighScoreList( const Steps* pSteps )
{
	std::map<const Steps*,HighScoresForASteps>::iterator iter = m_StepsHighScores.find( pSteps );
	if( iter == m_StepsHighScores.end() )
		return m_StepsHighScores[pSteps].hs;	// operator[] inserts into map
	else
		return iter->second.hs;
}

int Profile::GetStepsNumTimesPlayed( const Steps* pSteps ) const
{
	std::map<const Steps*,HighScoresForASteps>::const_iterator iter = m_StepsHighScores.find( pSteps );
	if( iter == m_StepsHighScores.end() )
	{
		return 0;
	}
	else
	{
		int iTotalNumTimesPlayed = 0;
		for( unsigned st = 0; st < NUM_STEPS_TYPES; ++st )
			iTotalNumTimesPlayed += iter->second.hs.iNumTimesPlayed;
		return iTotalNumTimesPlayed;
	}
}

void Profile::IncrementStepsPlayCount( const Steps* pSteps )
{
	GetStepsHighScoreList(pSteps).iNumTimesPlayed++;
}


//
// Course high scores
//
void Profile::AddCourseHighScore( const Course* pCourse, StepsType st, HighScore hs, int &iIndexOut )
{
	std::map<const Course*,HighScoresForACourse>::iterator iter = m_CourseHighScores.find( pCourse );
	if( iter == m_CourseHighScores.end() )
		m_CourseHighScores[pCourse].hs[st].AddHighScore( hs, iIndexOut );	// operator[] inserts into map
	else
		iter->second.hs[st].AddHighScore( hs, iIndexOut );
}

const HighScoreList& Profile::GetCourseHighScoreList( const Course* pCourse, StepsType st ) const
{
	return ((Profile *)this)->m_CourseHighScores[pCourse].hs[st];
}

HighScoreList& Profile::GetCourseHighScoreList( const Course* pCourse, StepsType st )
{
	std::map<const Course*,HighScoresForACourse>::iterator iter = m_CourseHighScores.find( pCourse );
	if( iter == m_CourseHighScores.end() )
		return m_CourseHighScores[pCourse].hs[st];	// operator[] inserts into map
	else
		return iter->second.hs[st];
}

int Profile::GetCourseNumTimesPlayed( const Course* pCourse ) const
{
	std::map<const Course*,HighScoresForACourse>::const_iterator iter = m_CourseHighScores.find( pCourse );
	if( iter == m_CourseHighScores.end() )
	{
		return 0;
	}
	else
	{
		int iTotalNumTimesPlayed = 0;
		for( unsigned st = 0; st < NUM_STEPS_TYPES; ++st )
			iTotalNumTimesPlayed += iter->second.hs[st].iNumTimesPlayed;
		return iTotalNumTimesPlayed;
	}
}

void Profile::IncrementCoursePlayCount( const Course* pCourse, StepsType st )
{
	GetCourseHighScoreList(pCourse,st).iNumTimesPlayed++;
}

//
// Category high scores
//
void Profile::AddCategoryHighScore( StepsType st, RankingCategory rc, HighScore hs, int &iIndexOut )
{
	m_CategoryHighScores[st][rc].AddHighScore( hs, iIndexOut );
}

const HighScoreList& Profile::GetCategoryHighScoreList( StepsType st, RankingCategory rc ) const
{
	return ((Profile *)this)->m_CategoryHighScores[st][rc];
}

HighScoreList& Profile::GetCategoryHighScoreList( StepsType st, RankingCategory rc )
{
	return m_CategoryHighScores[st][rc];
}

int Profile::GetCategoryNumTimesPlayed( StepsType st ) const
{
	int iNumTimesPlayed = 0;
	FOREACH_RankingCategory( rc )
		iNumTimesPlayed += m_CategoryHighScores[st][rc].iNumTimesPlayed;
	return iNumTimesPlayed;
}

void Profile::IncrementCategoryPlayCount( StepsType st, RankingCategory rc )
{
	m_CategoryHighScores[st][rc].iNumTimesPlayed++;
}


//
// Loading and saving
//

bool Profile::LoadAllFromDir( CString sDir )
{
	InitAll();
	bool bResult = LoadGeneralDataFromDir( sDir );
	if( PREFSMAN->m_bAllowReadOldScoreFormats )
		LoadSongScoresFromDirSM390a12( sDir );
	LoadSongScoresFromDir( sDir );
	if( PREFSMAN->m_bAllowReadOldScoreFormats )
		LoadCourseScoresFromDirSM390a12( sDir );
	LoadCourseScoresFromDir( sDir );
	if( PREFSMAN->m_bAllowReadOldScoreFormats )
		LoadCategoryScoresFromDirSM390a12( sDir );
	LoadCategoryScoresFromDir( sDir );
	return bResult;
}

bool Profile::SaveAllToDir( CString sDir ) const
{
	// Delete old files after saving new ones so we don't try to load old
	// and make duplicate records. 
	// If the save fails, the delete will fail too... probably :-)
	bool bResult = SaveGeneralDataToDir( sDir );
	SaveSongScoresToDir( sDir );
	DeleteSongScoresFromDirSM390a12( sDir );
	SaveCourseScoresToDir( sDir );
	DeleteCourseScoresFromDirSM390a12( sDir );
	SaveCategoryScoresToDir( sDir );
	DeleteCategoryScoresFromDirSM390a12( sDir );
	SaveStatsWebPageToDir( sDir );
	return bResult;
}


#define WARN	LOG->Warn("Error parsing file '%s' at %s:%d",fn.c_str(),__FILE__,__LINE__);
#define WARN_AND_RETURN { WARN; return; }
#define WARN_AND_CONTINUE { WARN; continue; }
#define CRYPT_VERIFY_FILE	\
	if( !CryptManager::VerifyFile(fn) )	{	\
		LOG->Warn("Signature check failed for '%s' at %s:%d",fn.c_str(),__FILE__,__LINE__); return; }
#define CRYPT_VERIFY_FILE_BOOL	\
	if( !CryptManager::VerifyFile(fn) )	{	\
		LOG->Warn("Signature check failed for '%s' at %s:%d",fn.c_str(),__FILE__,__LINE__); return false; }
#define CRYPT_WRITE_SIG		CryptManager::SignFile(fn);

bool Profile::LoadGeneralDataFromDir( CString sDir )
{
	CString fn = sDir + PROFILE_INI;
	InitGeneralData();

	CRYPT_VERIFY_FILE_BOOL;

	//
	// read ini
	//
	IniFile ini( fn );
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

bool Profile::SaveGeneralDataToDir( CString sDir ) const
{
	CString fn = sDir + PROFILE_INI;

	IniFile ini( fn );
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
	
	bool bResult = ini.WriteFile();
	CRYPT_WRITE_SIG;
	return bResult;
}

void Profile::SaveSongScoresToDir( CString sDir ) const
{
	CHECKPOINT;

	const Profile* pProfile = this;
	ASSERT( pProfile );

	CString fn = sDir + SONG_SCORES_XML;

	XNode xml;
	xml.name = "SongScores";

	const vector<Song*> &vpSongs = SONGMAN->GetAllSongs();
	
	for( unsigned s=0; s<vpSongs.size(); s++ )	// foreach song
	{
		Song* pSong = vpSongs[s];
		ASSERT(pSong);

		// skip songs that have never been played
		if( pProfile->GetSongNumTimesPlayed(pSong) == 0 )
			continue;

		LPXNode pSongNode = xml.AppendChild( "Song" );
		pSongNode->AppendChild( "SongDir", pSong->GetSongDir() );

		const vector<Steps*> vSteps = pSong->GetAllSteps();

		for( unsigned n=0; n<vSteps.size(); n++ )
		{
			Steps* pSteps = vSteps[n];

			// skip steps that have never been played
			if( pProfile->GetStepsHighScoreList(pSteps).iNumTimesPlayed == 0 )
				continue;

			LPXNode pStepsNode = pSongNode->AppendChild( "Steps" );

			const HighScoreList &hsl = pProfile->GetStepsHighScoreList( pSteps );
			
			pStepsNode->AppendChild( "StepsType", pSteps->m_StepsType );
			pStepsNode->AppendChild( "Difficulty", pSteps->GetDifficulty() );
			pStepsNode->AppendChild( "Description", pSteps->GetDescription() );

			pStepsNode->AppendChild( hsl.CreateNode() );
		}
	}
	
	xml.SaveToFile( fn );
	CRYPT_WRITE_SIG;
}

void Profile::LoadSongScoresFromDir( CString sDir )
{
	CHECKPOINT;

	CString fn = sDir + SONG_SCORES_XML;

	CRYPT_VERIFY_FILE;

	XNode xml;
	if( !xml.LoadFromFile( fn ) )
	{
		LOG->Warn( "Couldn't open file \"%s\" for writing.", fn.c_str() );
		return;
	}
	
	if( xml.name != "SongScores" )
		WARN_AND_RETURN;

	for( XNodes::iterator song = xml.childs.begin(); 
		song != xml.childs.end(); 
		song++ )
	{
		if( (*song)->name != "Song" )
			continue;

		CString sSongDir;
		if( !(*song)->GetChildValue("SongDir", sSongDir) )
			WARN_AND_CONTINUE;

		Song* pSong = SONGMAN->GetSongFromDir( sSongDir );
		if( pSong == NULL )
			WARN_AND_CONTINUE;

		for( XNodes::iterator steps = (*song)->childs.begin(); 
			steps != (*song)->childs.end(); 
			steps++ )
		{
			if( (*steps)->name != "Steps" )
				continue;

			StepsType st;
			if( !(*steps)->GetChildValue("StepsType", (int&)st) )
				WARN_AND_CONTINUE;

			Difficulty dc;
			if( !(*steps)->GetChildValue("Difficulty", (int&)dc) )
				WARN_AND_CONTINUE;
		
			CString sDescription;
			if( !(*steps)->GetChildValue("Description", sDescription) )
				WARN_AND_CONTINUE;

			// Even if pSong or pSteps is null, we still have to skip over that data.

			Steps* pSteps = NULL;
			if( dc == DIFFICULTY_EDIT )
				pSteps = pSong->GetStepsByDescription( st, sDescription );				
			else
				pSteps = pSong->GetStepsByDifficulty( st, dc );
			if( pSteps == NULL )
				WARN_AND_CONTINUE;

			XNode *pHighScoreListNode = (*steps)->GetChild("HighScoreList");
			if( pHighScoreListNode == NULL )
				WARN_AND_CONTINUE;
			
			HighScoreList &hsl = this->GetStepsHighScoreList( pSteps );
			hsl.LoadFromNode( pHighScoreListNode );
		}
	}
}

void Profile::LoadSongScoresFromDirSM390a12( CString sDir )
{
	Profile* pProfile = this;
	ASSERT( pProfile );

	CString fn = sDir + SM_390A12_SONG_SCORES_DAT;
	if( !IsAFile(fn) )
		return;

	RageFile f;
	if( !f.Open(fn, RageFile::READ) )
	{
		LOG->Warn( "Couldn't open file \"%s\": %s", fn.c_str(), f.GetError().c_str() );
		return;
	}

	int version;
	if( !FileRead(f, version) )
		WARN_AND_RETURN;
	if( version != SM_390A12_SONG_SCORES_VERSION )
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

			// Even if pSong or pSteps is null, we still have to skip over that data.

			Steps* pSteps = NULL;
			if( pSong )
			{
				if( dc==DIFFICULTY_INVALID )
					pSteps = pSong->GetStepsByDescription( st, sDescription );
				else
					pSteps = pSong->GetStepsByDifficulty( st, dc );
			}
			
			int iNumTimesPlayed;
			if( !FileRead(f, iNumTimesPlayed) )
				WARN_AND_RETURN;

			HighScoreList &hsl = pProfile->GetStepsHighScoreList(pSteps);
			
			if( pSteps )
				hsl.iNumTimesPlayed = iNumTimesPlayed;

			int iNumHighScores;
			if( !FileRead(f, iNumHighScores) )
				WARN_AND_RETURN;

			if( pSteps )
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

				if( pSteps == NULL )
					continue;	// ignore this high score
				
				hsl.vHighScores[l].sName = sName;
				hsl.vHighScores[l].grade = grade;
				hsl.vHighScores[l].iScore = iScore;
				hsl.vHighScores[l].fPercentDP = fPercentDP;
			}

			// ignore all high scores that are 0
			for( l=0; l<iNumHighScores; l++ )
			{
				if( pSteps && hsl.vHighScores[l].iScore <= 0 )
				{
					hsl.vHighScores.resize(l);
					break;
				}
			}
		}
	}
}


void Profile::LoadCategoryScoresFromDirSM390a12( CString sDir )
{
	CHECKPOINT;

	Profile* pProfile = this;
	ASSERT( pProfile );

	CString fn = sDir + SM_390A12_CATEGORY_SCORES_DAT;
	if( !IsAFile(fn) )
		return;

	RageFile f;
	if( !f.Open(fn, RageFile::READ) )
	{
		LOG->Warn( "Couldn't open file \"%s\": %s", fn.c_str(), f.GetError().c_str() );
		return;
	}
	
	int version;
	if( !FileRead(f, version) )
		WARN_AND_RETURN;
	if( version != SM_390A12_CATEGORY_RANKING_VERSION )
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

void Profile::LoadCourseScoresFromDirSM390a12( CString sDir )
{
	CHECKPOINT;

	Profile* pProfile = this;
	ASSERT( pProfile );

	CString fn = sDir + SM_390A12_COURSE_SCORES_DAT;
	if( !IsAFile(fn) )
		return;

	RageFile f;
	if( !f.Open(fn, RageFile::READ) )
	{
		LOG->Warn( "Couldn't open file \"%s\": %s", fn.c_str(), f.GetError().c_str() );
		return;
	}
	
	int version;
	if( !FileRead(f, version) )
		WARN_AND_RETURN;
	if( version != SM_390A12_COURSE_SCORES_VERSION )
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

				float fSurviveSeconds;
				if( !FileRead(f, fSurviveSeconds) )
					WARN_AND_RETURN;

				if( pCourse && st < NUM_STEPS_TYPES )
				{
					hsl.vHighScores[l].sName = sName;
					hsl.vHighScores[l].iScore = iScore;
					hsl.vHighScores[l].fPercentDP = fPercentDP;
					hsl.vHighScores[l].fSurviveSeconds = fSurviveSeconds;
				}
			}
		}
	}
}


void Profile::SaveCourseScoresToDir( CString sDir ) const
{
	CHECKPOINT;

	const Profile* pProfile = this;
	ASSERT( pProfile );

	CString fn = sDir + COURSE_SCORES_XML;


	XNode xml;
	xml.name = "CourseScores";

	vector<Course*> vpCourses;
	SONGMAN->GetAllCourses( vpCourses, true );
	
	for( unsigned c=0; c<vpCourses.size(); c++ )	// foreach course
	{
		Course *pCourse = vpCourses[c];

		// skip courses that have never been played
		if( pProfile->GetCourseNumTimesPlayed(pCourse) == 0 )
			continue;

		XNode* pCourseNode = xml.AppendChild( "Course", pCourse->m_bIsAutogen ? pCourse->m_sName : pCourse->m_sPath );

		for( StepsType st=(StepsType)0; st<NUM_STEPS_TYPES; ((int&)st)++ )
		{
			// skip course/steps types that have never been played
			if( pProfile->GetCourseHighScoreList(pCourse, st).iNumTimesPlayed == 0 )
				continue;

			LPXNode pStepsTypeNode = pCourseNode->AppendChild( "StepsType", st );

			const HighScoreList &hsl = pProfile->GetCourseHighScoreList( pCourse, st );
			
			pStepsTypeNode->AppendChild( hsl.CreateNode() );
		}
	}
	
	xml.SaveToFile( fn );
	CRYPT_WRITE_SIG;
}

void Profile::LoadCourseScoresFromDir( CString sDir )
{
	CHECKPOINT;

	CString fn = sDir + COURSE_SCORES_XML;

	CRYPT_VERIFY_FILE;

	XNode xml;
	if( !xml.LoadFromFile( fn ) )
	{
		LOG->Warn( "Couldn't open file \"%s\" for writing.", fn.c_str() );
		return;
	}
	
	if( xml.name != "CourseScores" )
		WARN_AND_RETURN;

	for( XNodes::iterator course = xml.childs.begin(); 
		course != xml.childs.end(); 
		course++ )
	{
		if( (*course)->name != "Course" )
			continue;
		
		CString sCourse;
		(*course)->GetValue(sCourse);
		Course* pCourse = SONGMAN->GetCourseFromPath( sCourse );
		if( pCourse == NULL )
			pCourse = SONGMAN->GetCourseFromName( sCourse );
		
		for( XNodes::iterator stepsType = (*course)->childs.begin(); 
			stepsType != (*course)->childs.end(); 
			stepsType++ )
		{
			if( (*stepsType)->name != "StepsType" )
				continue;
			
			StepsType st;
			(*stepsType)->GetValue((int&)st);

			XNode *pHighScoreListNode = (*stepsType)->GetChild("HighScoreList");
			if( pHighScoreListNode == NULL )
				WARN_AND_CONTINUE;
			
			HighScoreList &hsl = this->GetCourseHighScoreList( pCourse, st );
			hsl.LoadFromNode( pHighScoreListNode );
		}
	}
}

void Profile::SaveCategoryScoresToDir( CString sDir ) const
{
	CHECKPOINT;

	const Profile* pProfile = this;
	ASSERT( pProfile );

	CString fn = sDir + CATEGORY_SCORES_XML;

	XNode xml;
	xml.name = "CategoryScores";

	FOREACH_StepsType( st )
	{
		// skip steps types that have never been played
		if( pProfile->GetCategoryNumTimesPlayed( st ) == 0 )
			continue;

		XNode* pStepsTypeNode = xml.AppendChild( "StepsType", st );

		FOREACH_RankingCategory( rc )
		{
			// skip steps types/categories that have never been played
			if( pProfile->GetCategoryHighScoreList(st,rc).iNumTimesPlayed == 0 )
				continue;

			XNode* pRankingCategoryNode = pStepsTypeNode->AppendChild( "RankingCategory", rc );

			const HighScoreList &hsl = pProfile->GetCategoryHighScoreList( (StepsType)st, (RankingCategory)rc );

			pRankingCategoryNode->AppendChild( hsl.CreateNode() );
		}
	}

	xml.SaveToFile( fn );
	CRYPT_WRITE_SIG;
}

void Profile::LoadCategoryScoresFromDir( CString sDir )
{
	CHECKPOINT;

	CString fn = sDir + CATEGORY_SCORES_XML;

	CRYPT_VERIFY_FILE;

	XNode xml;
	if( !xml.LoadFromFile( fn ) )
	{
		LOG->Warn( "Couldn't open file \"%s\" for writing.", fn.c_str() );
		return;
	}
	
	if( xml.name != "CategoryScores" )
		WARN_AND_RETURN;

	for( XNodes::iterator stepsType = xml.childs.begin(); 
		stepsType != xml.childs.end(); 
		stepsType++ )
	{
		if( (*stepsType)->name != "StepsType" )
			continue;

		StepsType st;
		(*stepsType)->GetValue((int&)st);

		for( XNodes::iterator radarCategory = (*stepsType)->childs.begin(); 
			radarCategory != (*stepsType)->childs.end(); 
			radarCategory++ )
		{
			if( (*radarCategory)->name != "RankingCategory" )
				continue;

			RankingCategory rc;
			(*radarCategory)->GetValue( (int&)rc );

			XNode *pHighScoreListNode = (*radarCategory)->GetChild("HighScoreList");
			if( pHighScoreListNode == NULL )
				WARN_AND_CONTINUE;
			
			HighScoreList &hsl = this->GetCategoryHighScoreList( st, rc );
			hsl.LoadFromNode( pHighScoreListNode );
		}
	}
}

void Profile::DeleteSongScoresFromDirSM390a12( CString sDir ) const
{
	CString fn = sDir + SM_390A12_SONG_SCORES_DAT;
	// FIXME
}

void Profile::DeleteCourseScoresFromDirSM390a12( CString sDir ) const
{
	CString fn = sDir + SM_390A12_COURSE_SCORES_DAT;
	// FIXME
}

void Profile::DeleteCategoryScoresFromDirSM390a12( CString sDir ) const
{
	CString fn = sDir + SM_390A12_CATEGORY_SCORES_DAT;
	// FIXME
}

void Profile::SaveStatsWebPageToDir( CString sDir ) const
{
	const Profile* pProfile = this;

	CString fn = sDir + STATS_HTML;

	LOG->Trace( "Writing %s ...", fn.c_str() );

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
	// Print HTML headers
	//
	{
		f.PutLine( "<html>" );
		f.PutLine( "<head>" );
		f.PutLine( "<META HTTP-EQUIV=\"Content-Type\" Content=\"text/html; charset=UTF-8\">" );
		f.PutLine( ssprintf("<title>%s</title>", STATS_TITLE.c_str() ) );
		f.PutLine( ssprintf("<link rel='stylesheet' type='text/css' href='%s'>",STYLE_CSS) );
		f.PutLine( "</head>" );
		f.PutLine( "<body>" );
	}

#define PRINT_SECTION_START(szName)				f.Write( ssprintf("<h2><a name='%s'>"szName"</a> <a href='#top'>(top)</a></h2>\n", szName) )
#define PRINT_SECTION_END						f.Write( "\n" )
#define PRINT_DIV_START(szName)					f.Write( ssprintf("<div class='section1'>\n" "<h3>%s</h3>\n", szName) )
#define PRINT_DIV_START_ANCHOR(uAnchor,szName)	f.Write( ssprintf("<div class='section1'>\n" "<h3><a name='%u'>%s</a></h3>\n", (unsigned)uAnchor, szName) )
#define PRINT_DIV_END							f.Write( "</div>\n" )
#define PRINT_DIV2_START(szName)				f.Write( ssprintf("<div class='section2'>\n" "<h3>%s</h3>\n", szName) )
#define PRINT_DIV2_START_ANCHOR(uAnchor,szName)	f.Write( ssprintf("<div class='section2'>\n" "<h3><a name='%u'>%s</a></h3>\n", (unsigned)uAnchor, szName) )
#define PRINT_DIV2_END							f.Write( "</div>\n" )
#define PRINT_LINK(szName,szLink)				f.Write( ssprintf("<p><a href='%s'>%s</a></p>\n",szLink,szName) )
#define PRINT_LINE_S(szName,sVal) 				f.Write( ssprintf("<p>%s = <b>%s</b></p>\n",szName,sVal.c_str()) )
#define PRINT_LINE_B(szName,bVal) 				f.Write( ssprintf("<p>%s = <b>%s</b></p>\n",szName,(bVal)?"yes":"no") )
#define PRINT_LINE_I(szName,iVal) 				f.Write( ssprintf("<p>%s = <b>%d</b></p>\n",szName,iVal) )
#define PRINT_LINE_RANK(iRank,sName,iVal)		f.Write( ssprintf("<p><b>%d</b> - %s (%d)</p>\n",iRank,sName.c_str(),iVal) )
#define PRINT_LINE_RANK_LINK(iRank,sName,szLink,iVal)		f.Write( ssprintf("<p><b>%d</b> - <a href='%s'>%s</a> (%d)</p>\n",iRank,szLink,sName.c_str(),iVal) )

	//
	// Print table of Contents
	//
	{
		CString sName = 
			pProfile->m_sLastUsedHighScoreName.empty() ? 
			pProfile->m_sName :
			pProfile->m_sLastUsedHighScoreName;
	    time_t ltime = time( NULL );
		CString sTime = ctime( &ltime );

		f.Write( ssprintf("<h1><a name='top'>%s for %s - %s</a></h1>\n",STATS_TITLE.c_str(), sName.c_str(), sTime.c_str()) );
		PRINT_DIV_START("Table of Contents");
		PRINT_LINK( "Statistics", "#Statistics" );
		PRINT_LINK( "Popularity Lists", "#Popularity Lists" );
		PRINT_LINK( "Difficulty Table", "#Difficulty Table" );
		PRINT_LINK( "High Scores Table", "#High Scores Table" );
		PRINT_LINK( "Song/Steps List", "#Song/Steps List" );
		PRINT_LINK( "Bookkeeping", "#Bookkeeping" );
		PRINT_DIV_END;
	}

	//
	// Print Statistics
	//
	LOG->Trace( "Writing stats ..." );
	{
		PRINT_SECTION_START( "Statistics" );

		// Memory card stats
		{
			PRINT_DIV_START( "This Profile" );
			PRINT_LINE_S( "Name", pProfile->m_sName );
			PRINT_LINE_S( "LastUsedHighScoreName", pProfile->m_sLastUsedHighScoreName );
			PRINT_LINE_B( "UsingProfileDefaultModifiers", pProfile->m_bUsingProfileDefaultModifiers );
			PRINT_LINE_S( "DefaultModifiers", pProfile->m_sDefaultModifiers );
			PRINT_LINE_I( "TotalPlays", pProfile->m_iTotalPlays );
			PRINT_LINE_I( "TotalPlaySeconds", pProfile->m_iTotalPlaySeconds );
			PRINT_LINE_I( "TotalGameplaySeconds", pProfile->m_iTotalGameplaySeconds );
			PRINT_LINE_I( "CurrentCombo", pProfile->m_iCurrentCombo );
			PRINT_DIV_END;
		}
		
		// Num Songs Played by PlayMode
		{
			PRINT_DIV_START( "Num Songs Played by PlayMode" );
			for( int i=0; i<NUM_PLAY_MODES; i++ )
				PRINT_LINE_I( PlayModeToString((PlayMode)i).c_str(), pProfile->m_iNumSongsPlayedByPlayMode[i] );
			PRINT_DIV_END;
		}

		// Num Songs Played by Style
		{
			PRINT_DIV_START( "Num Songs Played by Style" );
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
				PRINT_LINE_I( pStyleDef->m_szName, pProfile->m_iNumSongsPlayedByStyle[i] );
			}
			PRINT_DIV_END;
		}

		// Num Songs Played by Difficulty
		{
			PRINT_DIV_START( "Num Songs Played by Difficulty" );
			for( int i=0; i<NUM_DIFFICULTIES; i++ )
				PRINT_LINE_I( DifficultyToString((Difficulty)i).c_str(), pProfile->m_iNumSongsPlayedByDifficulty[i] );
			PRINT_DIV_END;
		}

		// Num Songs Played by Meter
		{
			PRINT_DIV_START( "Num Songs Played by Meter" );
			for( int i=MAX_METER; i>=MIN_METER; i-- )
				PRINT_LINE_I( ssprintf("%d",i).c_str(), pProfile->m_iNumSongsPlayedByMeter[i] );
			PRINT_DIV_END;
		}

		PRINT_SECTION_END;
	}

	//
	// Print Popularity Lists
	//
	{
		PRINT_SECTION_START( "Popularity Lists" );

		// Songs by popularity
		{
			unsigned uNumToShow = min( vpSongs.size(), (unsigned)100 );

			SortSongPointerArrayByMostPlayed( vpSongs, this );
			PRINT_DIV_START( "Songs by Popularity" );
			for( unsigned i=0; i<uNumToShow; i++ )
			{
				Song* pSong = vpSongs[i];
				PRINT_LINE_RANK_LINK( i+1, pSong->GetFullDisplayTitle(), ssprintf("#%u",(unsigned)pSong).c_str(), this->GetSongNumTimesPlayed(pSong) );
			}
			PRINT_DIV_END;
		}

		// Steps by popularity
		{
			unsigned uNumToShow = min( vpAllSteps.size(), (unsigned)100 );

			SortStepsPointerArrayByMostPlayed( vpAllSteps, this );
			PRINT_DIV_START( "Steps by Popularity" );
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
				PRINT_LINE_RANK_LINK( i+1, s, ssprintf("#%u",(unsigned)pSteps).c_str(), pProfile->GetStepsNumTimesPlayed(pSteps) );
			}
			PRINT_DIV_END;
		}

		// Course by popularity
		{
			unsigned uNumToShow = min( vpCourses.size(), (unsigned)100 );

			SortCoursePointerArrayByMostPlayed( vpCourses, this );
			PRINT_DIV_START( "Courses by Popularity" );
			for( unsigned i=0; i<uNumToShow; i++ )
			{
				Course* pCourse = vpCourses[i];
				PRINT_LINE_RANK_LINK( i+1, pCourse->m_sName, ssprintf("#%u",(unsigned)pCourse).c_str(), pProfile->GetCourseNumTimesPlayed(pCourse) );
			}
			PRINT_DIV_END;
		}

		PRINT_SECTION_END;
	}

	//
	// Print High score tables
	//
	{
		SortSongPointerArrayByGroupAndTitle( vpSongs );

		PRINT_SECTION_START( "High Scores Table" );
		for( unsigned s=0; s<vStepsTypesToShow.size(); s++ )
		{
			StepsType st = vStepsTypesToShow[s];

			unsigned i;

 			PRINT_DIV_START( GAMEMAN->NotesTypeToString(st).c_str() );
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
						const HighScoreList &hsl = pProfile->GetStepsHighScoreList( pSteps );
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
			PRINT_DIV_END;
		}
		PRINT_SECTION_END;
	}

	//
	// Print Difficulty tables
	//
	{
		SortSongPointerArrayByGroupAndTitle( vpSongs );

		PRINT_SECTION_START( "Difficulty Table" );
		for( unsigned s=0; s<vStepsTypesToShow.size(); s++ )
		{
			StepsType st = vStepsTypesToShow[s];

			unsigned i;

 			PRINT_DIV_START( GAMEMAN->NotesTypeToString(st).c_str() );
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
			PRINT_DIV_END;
		}
		PRINT_SECTION_END;
	}

	//
	// Print song list
	//
	LOG->Trace( "Writing song list ..." );
	{
		PRINT_SECTION_START( "Song/Steps List" );
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
			PRINT_DIV_START_ANCHOR( /*Song primary key*/pSong, pSong->GetFullDisplayTitle().c_str() );
			PRINT_LINE_S( "Artist", pSong->GetDisplayArtist() );
			PRINT_LINE_S( "GroupName", pSong->m_sGroupName );
			float fMinBPM, fMaxBPM;
			pSong->GetDisplayBPM( fMinBPM, fMaxBPM );
			CString sBPM = (fMinBPM==fMaxBPM) ? ssprintf("%.1f",fMinBPM) : ssprintf("%.1f - %.1f",fMinBPM,fMaxBPM);
			PRINT_LINE_S( "BPM", sBPM );
			PRINT_LINE_I( "NumTimesPlayed", this->GetSongNumTimesPlayed(pSong) );
			PRINT_LINE_S( "Credit", pSong->m_sCredit );
			PRINT_LINE_S( "MusicLength", SecondsToTime(pSong->m_fMusicLengthSeconds) );
			PRINT_LINE_B( "Lyrics", !pSong->m_sLyricsFile.empty() );
			PRINT_DIV_END;

			//
			// Print Steps list
			//
			for( unsigned j=0; j<vpSteps.size(); j++ )
			{
				Steps* pSteps = vpSteps[j];
				if( pSteps->IsAutogen() )
					continue;	// skip autogen
				const HighScoreList &hsl = pProfile->GetStepsHighScoreList( pSteps );
				CString s = 
					GAMEMAN->NotesTypeToString(pSteps->m_StepsType) + 
					" - " +
					DifficultyToString(pSteps->GetDifficulty());
				PRINT_DIV2_START_ANCHOR( /*Steps primary key*/pSteps, s.c_str() );	// use poister value as the hash
				PRINT_LINE_I( "NumTimesPlayed", hsl.iNumTimesPlayed );
				
				for( unsigned i=0; i<hsl.vHighScores.size(); i++ )
				{
					const HighScore &hs = hsl.vHighScores[i];
					CString sName = ssprintf("#%d",i+1);
					CString sHSName = hs.sName.empty() ? "????" : hs.sName;
					CString sValue = ssprintf("%s, %s, %i, %.2f%%", sHSName.c_str(), GradeToString(hs.grade).c_str(), hs.iScore, hs.fPercentDP*100);
					PRINT_LINE_S( sName.c_str(), sValue );
				}
				f.PutLine( "</div>\n" );
				PRINT_DIV2_END;
			}
		}
		PRINT_SECTION_END;
	}

	//
	// Print Bookkeeping
	//
	{
		PRINT_SECTION_START( "Bookkeeping" );

		// GetCoinsLastDays
		{
			int coins[NUM_LAST_DAYS];
			BOOKKEEPER->GetCoinsLastDays( coins );
			PRINT_DIV_START( ssprintf("Coins for Last %d Days",NUM_LAST_DAYS).c_str() );
			for( int i=0; i<NUM_LAST_DAYS; i++ )
			{
				CString sDay = (i==0) ? "Today" : ssprintf("%d day(s) ago",i);
				PRINT_LINE_I( sDay.c_str(), coins[i] );
			}
			PRINT_DIV_END;
		}

		// GetCoinsLastWeeks
		{
			int coins[NUM_LAST_WEEKS];
			BOOKKEEPER->GetCoinsLastWeeks( coins );
			PRINT_DIV_START( ssprintf("Coins for Last %d Weeks",NUM_LAST_WEEKS).c_str() );
			for( int i=0; i<NUM_LAST_WEEKS; i++ )
			{
				CString sWeek = (i==0) ? "This week" : ssprintf("%d week(s) ago",i);
				PRINT_LINE_I( sWeek.c_str(), coins[i] );
			}
			PRINT_DIV_END;
		}

		// GetCoinsByDayOfWeek
		{
			int coins[DAYS_IN_WEEK];
			BOOKKEEPER->GetCoinsByDayOfWeek( coins );
			PRINT_DIV_START( "Coins by Day of Week" );
			for( int i=0; i<DAYS_IN_WEEK; i++ )
			{
				CString sDay = DAY_OF_WEEK_TO_NAME[i];
				PRINT_LINE_I( sDay.c_str(), coins[i] );
			}
			PRINT_DIV_END;
		}

		// GetCoinsByHour
		{
			int coins[HOURS_PER_DAY];
			BOOKKEEPER->GetCoinsByHour( coins );
			PRINT_DIV_START( ssprintf("Coins for Last %d Hours",HOURS_PER_DAY).c_str() );
			for( int i=0; i<HOURS_PER_DAY; i++ )
			{
				CString sHour = ssprintf("hour %d",i);
				PRINT_LINE_I( sHour.c_str(), coins[i] );
			}
			PRINT_DIV_END;
		}


		PRINT_SECTION_END;
	}
	
	PRINT_SECTION_START( "End of File" );
	PRINT_SECTION_END;

	f.PutLine( "</body>" );
	f.PutLine( "</html>" );

	//
	// Copy CSS file from theme.  If the copy fails, oh well...
	// 
	CString sStyleFile = THEME->GetPathToO("ProfileManager style.css");
	FileCopy( sStyleFile, sDir+STYLE_CSS );
	LOG->Trace( "Done." );		
}
