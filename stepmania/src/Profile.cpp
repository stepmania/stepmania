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
#include "CryptManager.h"
#include "PrefsManager.h"
#include "ProfileHtml.h"
#include "ProfileManager.h"

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


#define DEFAULT_PROFILE_NAME	""


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
	m_sLastMachinePlayed = "";

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

CString Profile::GetDisplayName() const
{
	if( !m_sName.empty() )
		return m_sName;
	else if( !m_sLastUsedHighScoreName.empty() )
		return m_sLastUsedHighScoreName;
	else
		return "NoName";
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
		return 0;
	else
		return iter->second.hs.iNumTimesPlayed;
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
	
	// Only try to load old score formats if we're allowing unsigned data.
	if( !PREFSMAN->m_bSignProfileData )
	{
		LoadSongScoresFromDirSM390a12( sDir );
		LoadCourseScoresFromDirSM390a12( sDir );
		LoadCategoryScoresFromDirSM390a12( sDir );
	}
	
	LoadSongScoresFromDir( sDir );
	LoadCourseScoresFromDir( sDir );
	LoadCategoryScoresFromDir( sDir );
	return bResult;
}

bool Profile::SaveAllToDir( CString sDir ) const
{
	m_sLastMachinePlayed = PREFSMAN->m_sMachineName;

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
	SaveMachinePublicKeyToDir( sDir );
	return bResult;
}


#define WARN	LOG->Warn("Error parsing file '%s' at %s:%d",fn.c_str(),__FILE__,__LINE__);
#define WARN_AND_RETURN { WARN; return; }
#define WARN_AND_CONTINUE { WARN; continue; }
#define CRYPT_VERIFY_FILE	\
	if( PREFSMAN->m_bSignProfileData && !CryptManager::VerifyFile(fn) )	{	\
		LOG->Warn("Signature check failed for '%s' at %s:%d",fn.c_str(),__FILE__,__LINE__); return; }
#define CRYPT_VERIFY_FILE_BOOL	\
	if( PREFSMAN->m_bSignProfileData && !CryptManager::VerifyFile(fn) )	{	\
		LOG->Warn("Signature check failed for '%s' at %s:%d",fn.c_str(),__FILE__,__LINE__); return false; }
#define CRYPT_WRITE_SIG		if( PREFSMAN->m_bSignProfileData ) { CryptManager::SignFile(fn); }

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
	ini.GetValue( "Profile", "WeightPounds",					m_fWeightPounds );
	ini.GetValue( "Profile", "CaloriesBurned",					m_fCaloriesBurned );
	ini.GetValue( "Profile", "LastMachinePlayed",				m_sLastMachinePlayed );

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
	ini.SetValue( "Profile", "WeightPounds",					m_fWeightPounds );
	ini.SetValue( "Profile", "CaloriesBurned",					m_fCaloriesBurned );
	ini.SetValue( "Profile", "LastMachinePlayed",				m_sLastMachinePlayed );

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
	// UGLY...
	bool bThisIsMachineProfile = this == PROFILEMAN->GetMachineProfile();

	if( bThisIsMachineProfile )
	{
		SaveMachineHtmlToDir( sDir, this );
		SavePlayerHtmlToDir( sDir+"temp/", this, PROFILEMAN->GetMachineProfile() );	// remove this when done debugging
	}
	else
	{
		SavePlayerHtmlToDir( sDir, this, PROFILEMAN->GetMachineProfile() );
	}
}

void Profile::SaveMachinePublicKeyToDir( CString sDir ) const
{
	FileCopy( CRYPTMAN->GetPublicKeyFileName(), "public.key.rsa" );
}
