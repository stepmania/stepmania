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
#include "RageFileManager.h"

//
// Old file versions for backward compatibility
//
#define SM_300_STATISTICS_INI	"statistics.ini"

#define SM_390A12_CATEGORY_SCORES_DAT	"CategoryScores.dat"
#define SM_390A12_SONG_SCORES_DAT		"SongScores.dat"
#define SM_390A12_COURSE_SCORES_DAT		"CourseScores.dat"
#define SM_390A12_PROFILE_INI			"Profile.ini"
const int SM_390A12_CATEGORY_RANKING_VERSION = 6;
const int SM_390A12_SONG_SCORES_VERSION = 9;
const int SM_390A12_COURSE_SCORES_VERSION = 8;


#if defined(WIN32)
#pragma warning (disable : 4706) // assignment within conditional expression
#endif

#define FOREACH_Node( Node, Var ) \
	XNodes::const_iterator Var##Iter; \
	const XNode *Var = NULL; \
	for( Var##Iter = Node->childs.begin(); \
		(Var##Iter != Node->childs.end() && (Var = *Var##Iter) ),  Var##Iter != Node->childs.end(); \
		++Var##Iter )


void Profile::InitEditableData()
{
	m_sDisplayName = "";	
	m_sLastUsedHighScoreName = "";
	m_fWeightPounds = 0;
}

void Profile::InitGeneralData()
{
	m_bUsingProfileDefaultModifiers = false;
	m_sDefaultModifiers = "";
	m_iTotalPlays = 0;
	m_iTotalPlaySeconds = 0;
	m_iTotalGameplaySeconds = 0;
	m_iCurrentCombo = 0;
	m_fTotalCaloriesBurned = 0;
	m_iTotalDancePoints = 0;
	m_iNumExtraStagesPassed = 0;
	m_iNumExtraStagesFailed = 0;
	m_iNumToasties = 0;
	m_UnlockedSongs.clear();
	m_sLastPlayedMachineName = "";
	m_iTotalTapsAndHolds = 0;
	m_iTotalJumps = 0;
	m_iTotalHolds = 0;
	m_iTotalMines = 0;
	m_iTotalHands = 0;

	int i;
	for( i=0; i<NUM_PLAY_MODES; i++ )
		m_iNumSongsPlayedByPlayMode[i] = 0;
	for( i=0; i<NUM_STYLES; i++ )
		m_iNumSongsPlayedByStyle[i] = 0;
	for( i=0; i<NUM_DIFFICULTIES; i++ )
		m_iNumSongsPlayedByDifficulty[i] = 0;
	for( i=0; i<MAX_METER+1; i++ )
		m_iNumSongsPlayedByMeter[i] = 0;
	ZERO( m_iNumSongsPassedByPlayMode );
	ZERO( m_iNumSongsPassedByGrade );
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

void Profile::InitScreenshotData()
{
	m_vScreenshots.clear();
}

void Profile::InitCalorieData()
{
	m_mapDayToCaloriesBurned.clear();
}

CString Profile::GetDisplayName() const
{
	if( !m_sDisplayName.empty() )
		return m_sDisplayName;
	else if( !m_sLastUsedHighScoreName.empty() )
		return m_sLastUsedHighScoreName;
	else
		return "NoName";
}

CString Profile::GetDisplayTotalCaloriesBurned() const
{
	if( m_fWeightPounds == 0 )	// weight not entered
		return "N/A";
	else 
		return Commify((int)m_fTotalCaloriesBurned) + " Cal";
}

int Profile::GetTotalNumSongsPlayed() const
{
	int iTotal = 0;
	for( int i=0; i<NUM_PLAY_MODES; i++ )
		iTotal += m_iNumSongsPlayedByPlayMode[i];
	return iTotal;
}

int Profile::GetTotalNumSongsPassed() const
{
	int iTotal = 0;
	for( int i=0; i<NUM_PLAY_MODES; i++ )
		iTotal += m_iNumSongsPassedByPlayMode[i];
	return iTotal;
}

CString Profile::GetProfileDisplayNameFromDir( CString sDir )
{
	Profile profile;
	profile.LoadEditableDataFromDir( sDir );
	return profile.GetDisplayName();
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
void Profile::AddCourseHighScore( const Course* pCourse, StepsType st, CourseDifficulty cd, HighScore hs, int &iIndexOut )
{
	std::map<const Course*,HighScoresForACourse>::iterator iter = m_CourseHighScores.find( pCourse );
	if( iter == m_CourseHighScores.end() )
		m_CourseHighScores[pCourse].hs[st][cd].AddHighScore( hs, iIndexOut );	// operator[] inserts into map
	else
		iter->second.hs[st][cd].AddHighScore( hs, iIndexOut );
}

const HighScoreList& Profile::GetCourseHighScoreList( const Course* pCourse, StepsType st, CourseDifficulty cd ) const
{
	return ((Profile *)this)->m_CourseHighScores[pCourse].hs[st][cd];
}

HighScoreList& Profile::GetCourseHighScoreList( const Course* pCourse, StepsType st, CourseDifficulty cd )
{
	std::map<const Course*,HighScoresForACourse>::iterator iter = m_CourseHighScores.find( pCourse );
	if( iter == m_CourseHighScores.end() )
		return m_CourseHighScores[pCourse].hs[st][cd];	// operator[] inserts into map
	else
		return iter->second.hs[st][cd];
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
			FOREACH_CourseDifficulty( cd )
				iTotalNumTimesPlayed += iter->second.hs[st][cd].iNumTimesPlayed;
		return iTotalNumTimesPlayed;
	}
}

void Profile::IncrementCoursePlayCount( const Course* pCourse, StepsType st, CourseDifficulty cd )
{
	GetCourseHighScoreList(pCourse,st,cd).iNumTimesPlayed++;
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

#define WARN	LOG->Warn("Error parsing file at %s:%d",__FILE__,__LINE__);
#define WARN_AND_RETURN { WARN; return; }
#define WARN_AND_CONTINUE { WARN; continue; }
#define WARN_AND_BREAK { WARN; break; }
#define LOAD_NODE(X)	{ \
	XNode* X = xml.GetChild(#X); \
	if( X==NULL ) LOG->Warn("Failed to read section " #X); \
	else Load##X##FromNode(X); }
int g_iOnceCtr;
#define FOR_ONCE	for(g_iOnceCtr=0;g_iOnceCtr<1;g_iOnceCtr++)


bool Profile::LoadAllFromDir( CString sDir )
{
	CHECKPOINT;

	InitAll();

	// Only try to load old score formats if we're allowing unsigned data.
	if( !PREFSMAN->m_bSignProfileData )
	{
		LoadProfileDataFromDirSM390a12( sDir );
		LoadSongScoresFromDirSM390a12( sDir );
		LoadCourseScoresFromDirSM390a12( sDir );
		LoadCategoryScoresFromDirSM390a12( sDir );
	}

	LoadEditableDataFromDir( sDir );

	
	// Read stats.xml
	FOR_ONCE
	{
		CString fn = sDir + STATS_XML;
		if( !IsAFile(fn) )
			break;

		LOG->Trace( "Reading profile data '%s'", fn.c_str() );

		//
		// Don't unreasonably large stats.xml files.
		//
		int iBytes = FILEMAN->GetFileSizeInBytes( fn );
		if( iBytes > REASONABLE_STATS_XML_SIZE_BYTES )
		{
			LOG->Warn( "The file '%s' is unreasonably large.  It won't be loaded.", fn.c_str() );
			break;
		}

		if( PREFSMAN->m_bSignProfileData )
		{
			CString sStatsXmlSigFile = fn+SIGNATURE_APPEND;
			CString sDontShareFile = sDir + DONT_SHARE_SIG;

			// verify the stats.xml signature with the "don't share" file
			if( !CryptManager::VerifyFileWithFile(sStatsXmlSigFile, sDontShareFile) )
			{
				LOG->Warn( "The don't share check for '%s' failed.  Data will be ignored.", sStatsXmlSigFile.c_str() );
				break;
			}

			// verify stats.xml
			if( !CryptManager::VerifyFileWithFile(fn, sStatsXmlSigFile) )
			{
				LOG->Warn( "The signature check for '%s' failed.  Data will be ignored.", fn.c_str() );
				break;
			}

		}

		XNode xml;
		if( !xml.LoadFromFile( fn ) )
		{
			LOG->Warn( "Couldn't open file '%s' for reading.", fn.c_str() );
			break;
		}

		if( xml.name != "Stats" )
			WARN_AND_BREAK;

		LOAD_NODE( GeneralData );
		LOAD_NODE( SongScores );
		LOAD_NODE( CourseScores );
		LOAD_NODE( CategoryScores );
		LOAD_NODE( ScreenshotData );
		LOAD_NODE( CalorieData );
	}
		
	return true;	// FIXME?  Investigate what happens if we always return true.
}

bool Profile::SaveAllToDir( CString sDir ) const
{
	m_sLastPlayedMachineName = PREFSMAN->m_sMachineName;

	// Save editable.xml
	SaveEditableDataToDir( sDir );

	// Save stats.xml
	{
		CString fn = sDir + STATS_XML;

		XNode xml;
		xml.name = "Stats";
		xml.AppendChild( SaveGeneralDataCreateNode() );
		xml.AppendChild( SaveSongScoresCreateNode() );
		xml.AppendChild( SaveCourseScoresCreateNode() );
		xml.AppendChild( SaveCategoryScoresCreateNode() );
		xml.AppendChild( SaveScreenshotDataCreateNode() );
		xml.AppendChild( SaveCalorieDataCreateNode() );
		bool bSaved = xml.SaveToFile(fn);
		if( bSaved && PREFSMAN->m_bSignProfileData )
		{
			CString sStatsXmlSigFile = fn+SIGNATURE_APPEND;
			CryptManager::SignFileToFile(fn, sStatsXmlSigFile);

			// Save the "don't share" file
			CString sDontShareFile = sDir + DONT_SHARE_SIG;
			CryptManager::SignFileToFile(sStatsXmlSigFile, sDontShareFile);
		}
	}


	// Delete old files after saving new ones so we don't try to load old
	// files the next time and make duplicate records. 
	if( !PREFSMAN->m_bSignProfileData )	// we tried to read the older formats
	{
		DeleteProfileDataFromDirSM390a12( sDir );
		DeleteSongScoresFromDirSM390a12( sDir );
		DeleteCourseScoresFromDirSM390a12( sDir );
		DeleteCategoryScoresFromDirSM390a12( sDir );
	}

	SaveStatsWebPageToDir( sDir );

	return true;
}


void Profile::LoadProfileDataFromDirSM390a12( CString sDir )
{
	CString fn = sDir + SM_390A12_PROFILE_INI;
	InitEditableData();
	InitGeneralData();

	//
	// read ini
	//
	IniFile ini( fn );
	if( !ini.ReadFile() )
		return;

	ini.GetValue( "Profile", "DisplayName",						m_sDisplayName );
	ini.GetValue( "Profile", "LastUsedHighScoreName",			m_sLastUsedHighScoreName );
	ini.GetValue( "Profile", "UsingProfileDefaultModifiers",	m_bUsingProfileDefaultModifiers );
	ini.GetValue( "Profile", "DefaultModifiers",				m_sDefaultModifiers );
	ini.GetValue( "Profile", "TotalPlays",						m_iTotalPlays );
	ini.GetValue( "Profile", "TotalPlaySeconds",				m_iTotalPlaySeconds );
	ini.GetValue( "Profile", "TotalGameplaySeconds",			m_iTotalGameplaySeconds );
	ini.GetValue( "Profile", "CurrentCombo",					m_iCurrentCombo );
	ini.GetValue( "Profile", "WeightPounds",					m_fWeightPounds );
	ini.GetValue( "Profile", "CaloriesBurned",					m_fTotalCaloriesBurned );
	ini.GetValue( "Profile", "LastPlayedMachineName",			m_sLastPlayedMachineName );

	unsigned i;
	for( i=0; i<NUM_PLAY_MODES; i++ )
		ini.GetValue( "Profile", "NumSongsPlayedByPlayMode"+Capitalize(PlayModeToString((PlayMode)i)), m_iNumSongsPlayedByPlayMode[i] );
	for( i=0; i<NUM_STYLES; i++ )
		ini.GetValue( "Profile", "NumSongsPlayedByStyle"+Capitalize(GAMEMAN->GetGameDefForGame(GAMEMAN->GetStyleDefForStyle((Style)i)->m_Game)->m_szName)+Capitalize(GAMEMAN->GetStyleDefForStyle((Style)i)->m_szName), m_iNumSongsPlayedByStyle[i] );
	for( i=0; i<NUM_DIFFICULTIES; i++ )
		ini.GetValue( "Profile", "NumSongsPlayedByDifficulty"+Capitalize(DifficultyToString((Difficulty)i)), m_iNumSongsPlayedByDifficulty[i] );
	for( i=0; i<MAX_METER+1; i++ )
		ini.GetValue( "Profile", "NumSongsPlayedByMeter"+ssprintf("%d",i), m_iNumSongsPlayedByMeter[i] );
}

void Profile::SaveEditableDataToDir( CString sDir ) const
{
	IniFile ini;
	CString fn = sDir + EDITABLE_INI;
	ini.SetPath( fn );

	ini.SetValue( "Editable", "DisplayName",			m_sDisplayName );
	ini.SetValue( "Editable", "LastUsedHighScoreName",	m_sLastUsedHighScoreName );
	ini.SetValue( "Editable", "WeightPounds",			m_fWeightPounds );

	ini.WriteFile();
}

XNode* Profile::SaveGeneralDataCreateNode() const
{
	XNode* pGeneralDataNode = new XNode;
	pGeneralDataNode->name = "GeneralData";
	
	pGeneralDataNode->AppendChild( "UsingProfileDefaultModifiers",	m_bUsingProfileDefaultModifiers );
	pGeneralDataNode->AppendChild( "DefaultModifiers",				m_sDefaultModifiers );
	pGeneralDataNode->AppendChild( "TotalPlays",					m_iTotalPlays );
	pGeneralDataNode->AppendChild( "TotalPlaySeconds",				m_iTotalPlaySeconds );
	pGeneralDataNode->AppendChild( "TotalGameplaySeconds",			m_iTotalGameplaySeconds );
	pGeneralDataNode->AppendChild( "CurrentCombo",					m_iCurrentCombo );
	pGeneralDataNode->AppendChild( "TotalCaloriesBurned",			m_fTotalCaloriesBurned );
	pGeneralDataNode->AppendChild( "LastPlayedMachineName",			m_sLastPlayedMachineName );
	pGeneralDataNode->AppendChild( "TotalDancePoints",				m_iTotalDancePoints );
	pGeneralDataNode->AppendChild( "NumExtraStagesPassed",			m_iNumExtraStagesPassed );
	pGeneralDataNode->AppendChild( "NumExtraStagesFailed",			m_iNumExtraStagesFailed );
	pGeneralDataNode->AppendChild( "NumToasties",					m_iNumToasties );
	pGeneralDataNode->AppendChild( "TotalTapsAndHolds",				m_iTotalTapsAndHolds );
	pGeneralDataNode->AppendChild( "TotalJumps",					m_iTotalJumps );
	pGeneralDataNode->AppendChild( "TotalHolds",					m_iTotalHolds );
	pGeneralDataNode->AppendChild( "TotalMines",					m_iTotalMines );
	pGeneralDataNode->AppendChild( "TotalHands",					m_iTotalHands );

	{
		XNode* pUnlockedSongs = pGeneralDataNode->AppendChild("UnlockedSongs");
		for( set<int>::const_iterator it = m_UnlockedSongs.begin(); it != m_UnlockedSongs.end(); ++it )
			pUnlockedSongs->AppendChild( ssprintf("%i", *it) );
	}

	{
		XNode* pNumSongsPlayedByPlayMode = pGeneralDataNode->AppendChild("NumSongsPlayedByPlayMode");
		FOREACH_PlayMode( pm )
		{
			/* Don't save unplayed PlayModes. */
			if( !m_iNumSongsPlayedByPlayMode[pm] )
				continue;
			pNumSongsPlayedByPlayMode->AppendChild( PlayModeToString(pm), m_iNumSongsPlayedByPlayMode[pm] );
		}
	}

	{
		XNode* pNumSongsPlayedByStyle = pGeneralDataNode->AppendChild("NumSongsPlayedByStyle");
		for( int i=0; i<NUM_STYLES; i++ )
		{
			/* Don't save unplayed styles. */
			if( !m_iNumSongsPlayedByStyle[i] )
				continue;
			
			XNode *pStyleNode = pNumSongsPlayedByStyle->AppendChild( "Style", m_iNumSongsPlayedByStyle[i] );

			const StyleDef *pStyle = GAMEMAN->GetStyleDefForStyle((Style)i);
			const GameDef *g = GAMEMAN->GetGameDefForGame( pStyle->m_Game );
			ASSERT( g );
			pStyleNode->AppendAttr( "Game", g->m_szName );
			pStyleNode->AppendAttr( "Style", pStyle->m_szName );
		}
	}

	{
		XNode* pNumSongsPlayedByDifficulty = pGeneralDataNode->AppendChild("NumSongsPlayedByDifficulty");
		FOREACH_Difficulty( dc )
		{
			if( !m_iNumSongsPlayedByDifficulty[dc] )
				continue;
			pNumSongsPlayedByDifficulty->AppendChild( DifficultyToString(dc), m_iNumSongsPlayedByDifficulty[dc] );
		}
	}

	{
		XNode* pNumSongsPlayedByMeter = pGeneralDataNode->AppendChild("NumSongsPlayedByMeter");
		for( int i=0; i<MAX_METER+1; i++ )
		{
			if( !m_iNumSongsPlayedByMeter[i] )
				continue;
			pNumSongsPlayedByMeter->AppendChild( ssprintf("Meter%d",i), m_iNumSongsPlayedByMeter[i] );
		}
	}

	{
		XNode* pNumSongsPassedByGrade = pGeneralDataNode->AppendChild("NumSongsPassedByGrade");
		FOREACH_Grade( g )
		{
			if( !m_iNumSongsPassedByGrade[g] )
				continue;
			pNumSongsPassedByGrade->AppendChild( GradeToString(g), m_iNumSongsPassedByGrade[g] );
		}
	}

	{
		XNode* pNumSongsPassedByPlayMode = pGeneralDataNode->AppendChild("NumSongsPassedByPlayMode");
		FOREACH_PlayMode( pm )
		{
			/* Don't save unplayed PlayModes. */
			if( !m_iNumSongsPassedByPlayMode[pm] )
				continue;
			pNumSongsPassedByPlayMode->AppendChild( PlayModeToString(pm), m_iNumSongsPassedByPlayMode[pm] );
		}
	}

	return pGeneralDataNode;
}

void Profile::LoadEditableDataFromDir( CString sDir )
{
	CString fn = sDir + EDITABLE_INI;

	//
	// Don't load unreasonably large editable.xml files.
	//
	int iBytes = FILEMAN->GetFileSizeInBytes( fn );
	if( iBytes > REASONABLE_EDITABLE_INI_SIZE_BYTES )
	{
		LOG->Warn( "The file '%s' is unreasonably large.  It won't be loaded.", fn.c_str() );
		return;
	}


	IniFile ini;
	ini.SetPath( fn );
	ini.ReadFile();

	ini.GetValue("Editable","DisplayName",				m_sDisplayName);
	ini.GetValue("Editable","LastUsedHighScoreName",	m_sLastUsedHighScoreName);
	ini.GetValue("Editable","WeightPounds",				m_fWeightPounds);

	// This is data that the user can change, so we have to validate it.
	wstring wstr = CStringToWstring(m_sDisplayName);
	if( wstr.size() > 12 )
		wstr = wstr.substr(0, 12);
	m_sDisplayName = WStringToCString(wstr);
	// TODO: strip invalid chars?
	if( m_fWeightPounds != 0 )
		CLAMP( m_fWeightPounds, 50, 500 );
}

void Profile::LoadGeneralDataFromNode( const XNode* pNode )
{
	ASSERT( pNode->name == "GeneralData" );

	pNode->GetChildValue( "UsingProfileDefaultModifiers",	m_bUsingProfileDefaultModifiers );
	pNode->GetChildValue( "DefaultModifiers",				m_sDefaultModifiers );
	pNode->GetChildValue( "TotalPlays",						m_iTotalPlays );
	pNode->GetChildValue( "TotalPlaySeconds",				m_iTotalPlaySeconds );
	pNode->GetChildValue( "TotalGameplaySeconds",			m_iTotalGameplaySeconds );
	pNode->GetChildValue( "CurrentCombo",					m_iCurrentCombo );
	pNode->GetChildValue( "TotalCaloriesBurned",			m_fTotalCaloriesBurned );
	pNode->GetChildValue( "LastPlayedMachineName",			m_sLastPlayedMachineName );
	pNode->GetChildValue( "TotalDancePoints",				m_iTotalDancePoints );
	pNode->GetChildValue( "NumExtraStagesPassed",			m_iNumExtraStagesPassed );
	pNode->GetChildValue( "NumExtraStagesFailed",			m_iNumExtraStagesFailed );
	pNode->GetChildValue( "NumToasties",					m_iNumToasties );
	pNode->GetChildValue( "TotalTapsAndHolds",				m_iTotalTapsAndHolds );
	pNode->GetChildValue( "TotalJumps",						m_iTotalJumps );
	pNode->GetChildValue( "TotalHolds",						m_iTotalHolds );
	pNode->GetChildValue( "TotalMines",						m_iTotalMines );
	pNode->GetChildValue( "TotalHands",						m_iTotalHands );

	{
		XNode* pUnlockedSongs = pNode->GetChild("UnlockedSongs");
		if( pUnlockedSongs )
		{
			FOREACH_Node( pUnlockedSongs, song )
				m_UnlockedSongs.insert( atoi(song->name) );
		}
	}

	{
		XNode* pNumSongsPlayedByPlayMode = pNode->GetChild("NumSongsPlayedByPlayMode");
		if( pNumSongsPlayedByPlayMode )
			FOREACH_PlayMode( pm )
				pNumSongsPlayedByPlayMode->GetChildValue( PlayModeToString(pm), m_iNumSongsPlayedByPlayMode[pm] );
	}

	{
		XNode* pNumSongsPlayedByStyle = pNode->GetChild("NumSongsPlayedByStyle");
		if( pNumSongsPlayedByStyle )
		{
			FOREACH_Node( pNumSongsPlayedByStyle, style )
			{
				if( style->name != "Style" )
					continue;

				CString sGame;
				if( !style->GetAttrValue( "Game", sGame ) )
					WARN_AND_CONTINUE;
				Game g = GAMEMAN->StringToGameType( sGame );
				if( g == GAME_INVALID )
					WARN_AND_CONTINUE;

				CString sStyle;
				if( !style->GetAttrValue( "Style", sStyle ) )
					WARN_AND_CONTINUE;
				Style s = GAMEMAN->GameAndStringToStyle( g, sStyle );
				if( s == STYLE_INVALID )
					WARN_AND_CONTINUE;

				style->GetValue( m_iNumSongsPlayedByStyle[s] );
			}
		}
	}

	{
		XNode* pNumSongsPlayedByDifficulty = pNode->GetChild("NumSongsPlayedByDifficulty");
		if( pNumSongsPlayedByDifficulty )
			FOREACH_Difficulty( dc )
				pNumSongsPlayedByDifficulty->GetChildValue( DifficultyToString(dc), m_iNumSongsPlayedByDifficulty[dc] );
	}

	{
		XNode* pNumSongsPlayedByMeter = pNode->GetChild("NumSongsPlayedByMeter");
		if( pNumSongsPlayedByMeter )
			for( int i=0; i<MAX_METER+1; i++ )
				pNumSongsPlayedByMeter->GetChildValue( ssprintf("Meter%d",i), m_iNumSongsPlayedByMeter[i] );
	}

	{
		XNode* pNumSongsPassedByGrade = pNode->GetChild("NumSongsPassedByGrade");
		if( pNumSongsPassedByGrade )
			FOREACH_Grade( g )
				pNumSongsPassedByGrade->GetChildValue( GradeToString(g), m_iNumSongsPassedByGrade[g] );
	}

	{
		XNode* pNumSongsPassedByPlayMode = pNode->GetChild("NumSongsPassedByPlayMode");
		if( pNumSongsPassedByPlayMode )
			FOREACH_PlayMode( pm )
				pNumSongsPassedByPlayMode->GetChildValue( PlayModeToString(pm), m_iNumSongsPassedByPlayMode[pm] );
	
	}

}

void Profile::AddStepTotals( int iTotalTapsAndHolds, int iTotalJumps, int iTotalHolds, int iTotalMines, int iTotalHands )
{
	m_iTotalTapsAndHolds += iTotalTapsAndHolds;
	m_iTotalJumps += iTotalJumps;
	m_iTotalHolds += iTotalHolds;
	m_iTotalMines += iTotalMines;
	m_iTotalHands += iTotalHands;

	if( m_fWeightPounds != 0 )
	{
		float fCals = 
			SCALE( m_fWeightPounds, 100.f, 200.f, 0.029f, 0.052f ) * iTotalTapsAndHolds +
			SCALE( m_fWeightPounds, 100.f, 200.f, 0.111f, 0.193f ) * iTotalJumps +
			SCALE( m_fWeightPounds, 100.f, 200.f, 0.029f, 0.052f ) * iTotalHolds +
			SCALE( m_fWeightPounds, 100.f, 200.f, 0.000f, 0.000f ) * iTotalMines +
			SCALE( m_fWeightPounds, 100.f, 200.f, 0.222f, 0.386f ) * iTotalHands;
		m_fTotalCaloriesBurned += fCals;

		tm cur_tm = GetLocalTime();
		Day day = { cur_tm.tm_yday, cur_tm.tm_year+1900 };
		m_mapDayToCaloriesBurned[day] += fCals;
	}
}

XNode* Profile::SaveSongScoresCreateNode() const
{
	CHECKPOINT;

	const Profile* pProfile = this;
	ASSERT( pProfile );

	XNode* pNode = new XNode;
	pNode->name = "SongScores";

	const vector<Song*> &vpSongs = SONGMAN->GetAllSongs();
	
	for( unsigned s=0; s<vpSongs.size(); s++ )	// foreach song
	{
		Song* pSong = vpSongs[s];
		ASSERT(pSong);

		// skip songs that have never been played
		if( pProfile->GetSongNumTimesPlayed(pSong) == 0 )
			continue;

		LPXNode pSongNode = pNode->AppendChild( "Song" );
		pSongNode->AppendAttr( "Dir", pSong->GetSongDir() );

		const vector<Steps*> vSteps = pSong->GetAllSteps();

		for( unsigned n=0; n<vSteps.size(); n++ )
		{
			Steps* pSteps = vSteps[n];

			// skip steps that have never been played
			if( pProfile->GetStepsHighScoreList(pSteps).iNumTimesPlayed == 0 )
				continue;

			LPXNode pStepsNode = pSongNode->AppendChild( "Steps" );

			const HighScoreList &hsl = pProfile->GetStepsHighScoreList( pSteps );
			
			pStepsNode->AppendAttr( "StepsType", GameManager::NotesTypeToString(pSteps->m_StepsType) );
			pStepsNode->AppendAttr( "Difficulty", DifficultyToString(pSteps->GetDifficulty()) );
			if( pSteps->GetDifficulty() == DIFFICULTY_EDIT )
				pStepsNode->AppendChild( "Description", pSteps->GetDescription() );

			pStepsNode->AppendChild( hsl.CreateNode() );
		}
	}
	
	return pNode;
}

void Profile::LoadSongScoresFromNode( const XNode* pNode )
{
	ASSERT( pNode->name == "SongScores" );

	for( XNodes::const_iterator song = pNode->childs.begin(); 
		song != pNode->childs.end(); 
		song++ )
	{
		if( (*song)->name != "Song" )
			continue;

		CString sSongDir;
		if( !(*song)->GetAttrValue( "Dir", sSongDir ) )
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

			CString str;
			if( !(*steps)->GetAttrValue("StepsType", str) )
				WARN_AND_CONTINUE;
			const StepsType st = GameManager::StringToNotesType( str );
			if( st == STEPS_TYPE_INVALID )
				WARN_AND_CONTINUE;

			if( !(*steps)->GetAttrValue("Difficulty", str) )
				WARN_AND_CONTINUE;
			const Difficulty dc = StringToDifficulty( str );
			if( dc == DIFFICULTY_INVALID )
				WARN_AND_CONTINUE;
		
			Steps* pSteps = NULL;
			if( dc == DIFFICULTY_EDIT )
			{
				CString sDescription;
				if( !(*steps)->GetChildValue("Description", sDescription) )
					WARN_AND_CONTINUE;

				pSteps = pSong->GetStepsByDescription( st, sDescription );				
			}
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

			HighScoreList &hsl = pProfile->GetCourseHighScoreList( pCourse, (StepsType)st, COURSE_DIFFICULTY_REGULAR );

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


XNode* Profile::SaveCourseScoresCreateNode() const
{
	CHECKPOINT;

	const Profile* pProfile = this;
	ASSERT( pProfile );

	XNode* pNode = new XNode;
	pNode->name = "CourseScores";

	vector<Course*> vpCourses;
	SONGMAN->GetAllCourses( vpCourses, true );
	
	for( unsigned c=0; c<vpCourses.size(); c++ )	// foreach course
	{
		Course *pCourse = vpCourses[c];

		// skip courses that have never been played
		if( pProfile->GetCourseNumTimesPlayed(pCourse) == 0 )
			continue;

		XNode* pCourseNode = pNode->AppendChild( "Course" );
		pCourseNode->AppendAttr( "Name", pCourse->m_bIsAutogen ? pCourse->m_sName : pCourse->m_sPath );

		for( StepsType st=(StepsType)0; st<NUM_STEPS_TYPES; ((int&)st)++ )
		{
			// TRICKY:  Only append this node if one we have at least one child.
			// There's no point to saving a bunch of empty StepsType nodes.
			LPXNode pStepsTypeNode = new XNode;
			pStepsTypeNode->name = "StepsType";
			pStepsTypeNode->AppendAttr( "Type", GameManager::NotesTypeToString(st) );
			
			FOREACH_CourseDifficulty( cd )
			{
				// skip course/steps types that have never been played
				if( pProfile->GetCourseHighScoreList(pCourse, st, cd).iNumTimesPlayed == 0 )
					continue;

				LPXNode pCourseDifficultyNode = pStepsTypeNode->AppendChild( "CourseDifficulty" );
				pCourseDifficultyNode->AppendAttr( "Type", CourseDifficultyToString(cd) );

				const HighScoreList &hsl = pProfile->GetCourseHighScoreList( pCourse, st, cd );
				
				pCourseDifficultyNode->AppendChild( hsl.CreateNode() );
			}

			if( !pStepsTypeNode->childs.empty() )
				pCourseNode->AppendChild( pStepsTypeNode );
		}
	}

	return pNode;
}

void Profile::LoadCourseScoresFromNode( const XNode* pNode )
{
	CHECKPOINT;

	ASSERT( pNode->name == "CourseScores" );

	for( XNodes::const_iterator course = pNode->childs.begin(); 
		course != pNode->childs.end(); 
		course++ )
	{
		if( (*course)->name != "Course" )
			continue;

		CString sCourse;
		if( !(*course)->GetAttrValue( "Name", sCourse ) )
			WARN_AND_CONTINUE;

		Course* pCourse = SONGMAN->GetCourseFromPath( sCourse );
		if( pCourse == NULL )
			pCourse = SONGMAN->GetCourseFromName( sCourse );
		if( pCourse == NULL )
			WARN_AND_CONTINUE;
		
		for( XNodes::iterator stepsType = (*course)->childs.begin(); 
			stepsType != (*course)->childs.end(); 
			stepsType++ )
		{
			if( (*stepsType)->name != "StepsType" )
				continue;
			
			CString str;
			if( !(*stepsType)->GetAttrValue( "Type", str ) )
				WARN_AND_CONTINUE;
			StepsType st = GameManager::StringToNotesType( str );
			if( st == STEPS_TYPE_INVALID )
				WARN_AND_CONTINUE;

			for( XNodes::iterator courseDifficulty = (*stepsType)->childs.begin(); 
				courseDifficulty != (*stepsType)->childs.end(); 
				courseDifficulty++ )
			{

				const LPXAttr TypeAttr = (*courseDifficulty)->GetAttr( "Type" );
				if( TypeAttr == NULL )
					WARN_AND_CONTINUE;
				CourseDifficulty cd = StringToCourseDifficulty( TypeAttr->value );
				if( cd == COURSE_DIFFICULTY_INVALID )
					WARN_AND_CONTINUE;

				XNode *pHighScoreListNode = (*courseDifficulty)->GetChild("HighScoreList");
				if( pHighScoreListNode == NULL )
					WARN_AND_CONTINUE;
				
				HighScoreList &hsl = this->GetCourseHighScoreList( pCourse, st, cd );
				hsl.LoadFromNode( pHighScoreListNode );
			}
		}
	}
}

XNode* Profile::SaveCategoryScoresCreateNode() const
{
	CHECKPOINT;

	const Profile* pProfile = this;
	ASSERT( pProfile );

	XNode* pNode = new XNode;
	pNode->name = "CategoryScores";

	FOREACH_StepsType( st )
	{
		// skip steps types that have never been played
		if( pProfile->GetCategoryNumTimesPlayed( st ) == 0 )
			continue;

		XNode* pStepsTypeNode = pNode->AppendChild( "StepsType" );
		pStepsTypeNode->AppendAttr( "Type", GameManager::NotesTypeToString(st) );

		FOREACH_RankingCategory( rc )
		{
			// skip steps types/categories that have never been played
			if( pProfile->GetCategoryHighScoreList(st,rc).iNumTimesPlayed == 0 )
				continue;

			XNode* pRankingCategoryNode = pStepsTypeNode->AppendChild( "RankingCategory" );
			pRankingCategoryNode->AppendAttr( "Type", RankingCategoryToString(rc) );

			const HighScoreList &hsl = pProfile->GetCategoryHighScoreList( (StepsType)st, (RankingCategory)rc );

			pRankingCategoryNode->AppendChild( hsl.CreateNode() );
		}
	}

	return pNode;
}

void Profile::LoadCategoryScoresFromNode( const XNode* pNode )
{
	CHECKPOINT;

	ASSERT( pNode->name == "CategoryScores" );

	for( XNodes::const_iterator stepsType = pNode->childs.begin(); 
		stepsType != pNode->childs.end(); 
		stepsType++ )
	{
		if( (*stepsType)->name != "StepsType" )
			continue;

		CString str;
		if( !(*stepsType)->GetAttrValue( "Type", str ) )
			WARN_AND_CONTINUE;
		StepsType st = GameManager::StringToNotesType( str );
		if( st == STEPS_TYPE_INVALID )
			WARN_AND_CONTINUE;

		for( XNodes::iterator radarCategory = (*stepsType)->childs.begin(); 
			radarCategory != (*stepsType)->childs.end(); 
			radarCategory++ )
		{
			if( (*radarCategory)->name != "RankingCategory" )
				continue;

			if( !(*radarCategory)->GetAttrValue( "Type", str ) )
				WARN_AND_CONTINUE;
			RankingCategory rc = StringToRankingCategory( str );
			if( rc == RANKING_INVALID )
				WARN_AND_CONTINUE;

			XNode *pHighScoreListNode = (*radarCategory)->GetChild("HighScoreList");
			if( pHighScoreListNode == NULL )
				WARN_AND_CONTINUE;
			
			HighScoreList &hsl = this->GetCategoryHighScoreList( st, rc );
			hsl.LoadFromNode( pHighScoreListNode );
		}
	}
}

void Profile::DeleteProfileDataFromDirSM390a12( CString sDir ) const
{
	CString fn = sDir + SM_390A12_PROFILE_INI;
	FILEMAN->Remove( fn );
}

void Profile::DeleteSongScoresFromDirSM390a12( CString sDir ) const
{
	CString fn = sDir + SM_390A12_SONG_SCORES_DAT;
	FILEMAN->Remove( fn );
}

void Profile::DeleteCourseScoresFromDirSM390a12( CString sDir ) const
{
	CString fn = sDir + SM_390A12_COURSE_SCORES_DAT;
	FILEMAN->Remove( fn );
}

void Profile::DeleteCategoryScoresFromDirSM390a12( CString sDir ) const
{
	CString fn = sDir + SM_390A12_CATEGORY_SCORES_DAT;
	FILEMAN->Remove( fn );
}

void Profile::SaveStatsWebPageToDir( CString sDir ) const
{
	ASSERT( PROFILEMAN );

	// UGLY...
	bool bThisIsMachineProfile = (this == PROFILEMAN->GetMachineProfile());
	// Profile* pMachineProfile = PROFILEMAN->GetMachineProfile();

	SaveStatsWebPage( 
		sDir,
		this,
		PROFILEMAN->GetMachineProfile(),
		bThisIsMachineProfile ? HTML_TYPE_MACHINE : HTML_TYPE_PLAYER
		);
	if( bThisIsMachineProfile )
		SaveStatsWebPage( 
			sDir+"temp/",
			this,
			PROFILEMAN->GetMachineProfile(),
			HTML_TYPE_PLAYER
			);
}

void Profile::SaveMachinePublicKeyToDir( CString sDir ) const
{
	if( PREFSMAN->m_bSignProfileData && IsAFile(CRYPTMAN->GetPublicKeyFileName()) )
		FileCopy( CRYPTMAN->GetPublicKeyFileName(), PUBLIC_KEY_FILE );
}

void Profile::AddScreenshot( Screenshot screenshot )
{
	m_vScreenshots.push_back( screenshot );
}

void Profile::LoadScreenshotDataFromNode( const XNode* pNode )
{
	CHECKPOINT;

	ASSERT( pNode->name == "ScreenshotData" );
	for( XNodes::const_iterator screenshot = pNode->childs.begin(); 
		screenshot != pNode->childs.end(); 
		screenshot++ )
	{
		if( (*screenshot)->name != "Screenshot" )
			WARN_AND_CONTINUE;

		Screenshot ss;
		
		if( !(*screenshot)->GetChildValue("FileName",ss.sFileName) )
			WARN;

		if( !(*screenshot)->GetChildValue("MD5",ss.sMD5) )
			WARN;

		if( !(*screenshot)->GetChildValue("Time",(int&)ss.time) )	// time_t is a signed long on Win32.  Is this ok on other platforms?
			WARN;

		if( !(*screenshot)->GetChildValue("MachineName",ss.sMachineName) )
			WARN;

		m_vScreenshots.push_back( ss );
	}	
}

XNode* Profile::SaveScreenshotDataCreateNode() const
{
	CHECKPOINT;

	const Profile* pProfile = this;
	ASSERT( pProfile );

	XNode* pNode = new XNode;
	pNode->name = "ScreenshotData";

	for( unsigned i=0; i<m_vScreenshots.size(); i++ )
	{
		const Screenshot &ss = m_vScreenshots[i];

		XNode* pScreenshotNode = pNode->AppendChild( "Screenshot" );

		pScreenshotNode->AppendChild( "FileName", ss.sFileName );
		pScreenshotNode->AppendChild( "MD5", ss.sMD5 );
		pScreenshotNode->AppendChild( "Time", (int) ss.time );
		pScreenshotNode->AppendChild( "MachineName", ss.sMachineName );
	}

	return pNode;
}

void Profile::LoadCalorieDataFromNode( const XNode* pNode )
{
	CHECKPOINT;

	ASSERT( pNode->name == "CalorieData" );
	for( XNodes::const_iterator pDay = pNode->childs.begin(); 
		pDay != pNode->childs.end(); 
		pDay++ )
	{
		if( (*pDay)->name != "Day" )
			WARN_AND_CONTINUE;

		Day day;
		
		if( !(*pDay)->GetAttrValue("DayInYear",day.iDayInYear) )
			WARN_AND_CONTINUE;

		if( !(*pDay)->GetAttrValue("Year",day.iYear) )
			WARN_AND_CONTINUE;

		float fCaloriesBurned = 0;

		if( !(*pDay)->GetChildValue("CaloriesBurned",fCaloriesBurned) )
			WARN_AND_CONTINUE;

		m_mapDayToCaloriesBurned[day] = fCaloriesBurned;
	}	
}

XNode* Profile::SaveCalorieDataCreateNode() const
{
	CHECKPOINT;

	const Profile* pProfile = this;
	ASSERT( pProfile );

	XNode* pNode = new XNode;
	pNode->name = "CalorieData";

	for( map<Day,float>::const_iterator i = m_mapDayToCaloriesBurned.begin();
		i != m_mapDayToCaloriesBurned.end();
		i++ )
	{
		XNode* pDay = pNode->AppendChild( "Day" );

		pDay->AppendAttr( "DayInYear", i->first.iDayInYear );
		pDay->AppendAttr( "Year", i->first.iYear );

		pDay->AppendChild( "CaloriesBurned", i->second );
	}

	return pNode;
}

float Profile::GetCaloriesBurnedForDay( Day day ) const
{
	map<Day,float>::const_iterator i = m_mapDayToCaloriesBurned.find( day );
	if( i == m_mapDayToCaloriesBurned.end() )
		return 0;
	else
		return i->second;
}
