#include "global.h"
/*
-----------------------------------------------------------------------------
 Class: SongManager

 Desc: See header.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "SongManager.h"
#include "IniFile.h"
#include "RageLog.h"
#include "MsdFile.h"
#include "NotesLoaderDWI.h"
#include "BannerCache.h"
#include "arch/arch.h"

#include "GameState.h"
#include "PrefsManager.h"
#include "RageException.h"
#include "arch/LoadingWindow/LoadingWindow.h"
#include "Course.h"

#include "AnnouncerManager.h"
#include "ThemeManager.h"
#include "GameManager.h"
#include "RageFile.h"
#include "ProductInfo.h"
#include "RageTextureManager.h"
#include "Banner.h"

SongManager*	SONGMAN = NULL;	// global and accessable from anywhere in our program

#define SM_300_STATISTICS_FILE	BASE_PATH "statistics.ini"
#define SONGS_DIR				BASE_PATH "Songs" SLASH
#define COURSES_DIR				BASE_PATH "Courses" SLASH
#define STATS_PATH				BASE_PATH "stats.html"
const CString CATEGORY_RANKING_FILE =			BASE_PATH "Data" SLASH "CategoryRanking.dat";
const CString MACHINE_STEPS_MEM_CARD_DATA =		BASE_PATH "Data" SLASH "MachineStepsMemCardData.dat";
const CString MACHINE_COURSE_MEM_CARD_DATA =	BASE_PATH "Data" SLASH "MachineCourseMemCardData.dat";
const int CATEGORY_RANKING_VERSION = 3;
const int STEPS_MEM_CARD_DATA_VERSION = 7;
const int COURSE_MEM_CARD_DATA_VERSION = 5;


#define NUM_GROUP_COLORS	THEME->GetMetricI("SongManager","NumGroupColors")
#define GROUP_COLOR( i )	THEME->GetMetricC("SongManager",ssprintf("GroupColor%d",i+1))
#define BEGINNER_COLOR		THEME->GetMetricC("SongManager","BeginnerColor")
#define EASY_COLOR			THEME->GetMetricC("SongManager","EasyColor")
#define MEDIUM_COLOR		THEME->GetMetricC("SongManager","MediumColor")
#define HARD_COLOR			THEME->GetMetricC("SongManager","HardColor")
#define CHALLENGE_COLOR		THEME->GetMetricC("SongManager","ChallengeColor")
#define EXTRA_COLOR			THEME->GetMetricC("SongManager","ExtraColor")

vector<RageColor> g_vGroupColors;
RageTimer g_LastMetricUpdate; /* can't use RageTimer globally */

static void UpdateMetrics()
{
	if( g_LastMetricUpdate.PeekDeltaTime() < 1 )
		return;

	g_LastMetricUpdate.Touch();
	g_vGroupColors.clear();
	for( int i=0; i<NUM_GROUP_COLORS; i++ )
		g_vGroupColors.push_back( GROUP_COLOR(i) );
}

SongManager::SongManager( LoadingWindow *ld )
{
	/* We initialize things that assume they can get at SONGMAN; we only
	 * init one of these, so hook us up to it immediately. */
	SONGMAN = this;
	try
	{
		InitSongsFromDisk( ld );
		InitCoursesFromDisk( ld );
		InitAutogenCourses();
		InitMachineScoresFromDisk();

	} catch(...) {
		SONGMAN = NULL;
		throw;
	}
	
	g_LastMetricUpdate.SetZero();
	UpdateMetrics();
}

SongManager::~SongManager()
{
	SaveMachineScoresToDisk();

	WriteStatsWebPage();

	FreeSongs();
}

void SongManager::CategoryData::AddHighScore( HighScore hs, int &iIndexOut )
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


void SongManager::Reload()
{
	FlushDirCache();

	SaveMachineScoresToDisk();
	FreeSongs();
	FreeCourses();

	m_sGroupNames.clear();
	m_sGroupBannerPaths.clear();

	InitSongsFromDisk(NULL);
	InitCoursesFromDisk(NULL);
	InitAutogenCourses();
	InitMachineScoresFromDisk();
}


void SongManager::SaveMachineScoresToDisk()
{
	SaveCategoryRankingsToFile( CATEGORY_RANKING_FILE );
	SaveStepsMemCardDataToFile( MACHINE_STEPS_MEM_CARD_DATA, MEMORY_CARD_MACHINE );
	SaveCourseMemCardDataToFile( MACHINE_COURSE_MEM_CARD_DATA, MEMORY_CARD_MACHINE );
}

void SongManager::InitSongsFromDisk( LoadingWindow *ld )
{
	RageTimer tm;
	LoadStepManiaSongDir( SONGS_DIR, ld );

	for( unsigned i=0; i<PREFSMAN->m_asAdditionalSongFolders.size(); i++ )
        LoadStepManiaSongDir( PREFSMAN->m_asAdditionalSongFolders[i], ld );

	if( PREFSMAN->m_DWIPath != "" )
		LoadStepManiaSongDir( PREFSMAN->m_DWIPath + "/Songs", ld );

	LOG->Trace( "Found %d songs in %f seconds.", (int)m_pSongs.size(), tm.GetDeltaTime() );
}

void SongManager::SanityCheckGroupDir( CString sDir ) const
{
	// Check to see if they put a song directly inside the group folder.
	CStringArray arrayFiles;
	GetDirListing( sDir + "/*.mp3", arrayFiles );
	GetDirListing( sDir + "/*.ogg", arrayFiles );
	GetDirListing( sDir + "/*.wav", arrayFiles );
	if( !arrayFiles.empty() )
		RageException::Throw( 
			"The folder '%s' contains music files.\n\n"
			"This means that you have a music outside of a song folder.\n"
			"All song folders must reside in a group folder.  For example, 'Songs/DDR 4th Mix/B4U'.\n"
			"See the StepMania readme for more info.",
			sDir.c_str()
		);
	
}

void SongManager::AddGroup( CString sDir, CString sGroupDirName )
{
	unsigned j;
	for(j = 0; j < m_sGroupNames.size(); ++j)
		if( sGroupDirName == m_sGroupNames[j] ) break;

	if( j != m_sGroupNames.size() )
		return; /* the group is already added */

	// Look for a group banner in this group folder
	CStringArray arrayGroupBanners;
	GetDirListing( sDir+sGroupDirName+"/*.png", arrayGroupBanners );
	GetDirListing( sDir+sGroupDirName+"/*.jpg", arrayGroupBanners );
	GetDirListing( sDir+sGroupDirName+"/*.gif", arrayGroupBanners );
	GetDirListing( sDir+sGroupDirName+"/*.bmp", arrayGroupBanners );

	CString sBannerPath;
	if( !arrayGroupBanners.empty() )
		sBannerPath = sDir+sGroupDirName+SLASH+arrayGroupBanners[0] ;
	else
	{
		// Look for a group banner in the parent folder
		GetDirListing( sDir+sGroupDirName+".png", arrayGroupBanners );
		GetDirListing( sDir+sGroupDirName+".jpg", arrayGroupBanners );
		GetDirListing( sDir+sGroupDirName+".gif", arrayGroupBanners );
		GetDirListing( sDir+sGroupDirName+".bmp", arrayGroupBanners );
		if( !arrayGroupBanners.empty() )
			sBannerPath = sDir+arrayGroupBanners[0];
	}

	if( sBannerPath != "" )
		LOG->Trace( "Group banner for '%s' is '%s'.", sGroupDirName.c_str(), sBannerPath.c_str() );
	m_sGroupNames.push_back( sGroupDirName );
	m_sGroupBannerPaths.push_back(sBannerPath);
}

void SongManager::LoadStepManiaSongDir( CString sDir, LoadingWindow *ld )
{
	/* Make sure sDir has a trailing slash. */
	TrimRight( sDir, "/\\" );
	sDir += SLASH;

	// Find all group directories in "Songs" folder
	CStringArray arrayGroupDirs;
	GetDirListing( sDir+"*", arrayGroupDirs, true );
	SortCStringArray( arrayGroupDirs );

	for( unsigned i=0; i< arrayGroupDirs.size(); i++ )	// for each dir in /Songs/
	{
		CString sGroupDirName = arrayGroupDirs[i];

		if( 0 == stricmp( sGroupDirName, "cvs" ) )	// the directory called "CVS"
			continue;		// ignore it

		SanityCheckGroupDir(sDir+sGroupDirName);

		// Find all Song folders in this group directory
		CStringArray arraySongDirs;
		GetDirListing( sDir+sGroupDirName + SLASH "*", arraySongDirs, true, true );
		SortCStringArray( arraySongDirs );

		unsigned j;
		int loaded = 0;

		for( j=0; j< arraySongDirs.size(); j++ )	// for each song dir
		{
			CString sSongDirName = arraySongDirs[j];

			if( 0 == stricmp( Basename(sSongDirName), "cvs" ) )	// the directory called "CVS"
				continue;		// ignore it

			// this is a song directory.  Load a new song!
			if( ld ) {
				ld->SetText( ssprintf("Loading songs...\n%s\n%s",
					Basename(sGroupDirName).c_str(),
					Basename(sSongDirName).c_str()));
				ld->Paint();
			}
			Song* pNewSong = new Song;
			if( !pNewSong->LoadFromSongDir( sSongDirName ) ) {
				/* The song failed to load. */
				delete pNewSong;
				continue;
			}
			
            m_pSongs.push_back( pNewSong );
			loaded++;
		}

		/* Don't add the group name if we didn't load any songs in this group. */
		if(!loaded) continue;

		/* Add this group to the group array. */
		AddGroup(sDir, sGroupDirName);

		/* Cache and load the group banner. */
		BANNERCACHE->CacheBanner( GetGroupBannerPath(sGroupDirName) );
		
		/* Load the group sym links (if any)*/
		LoadGroupSymLinks(sDir, sGroupDirName);
	}
}

void SongManager::LoadGroupSymLinks(CString sDir, CString sGroupFolder)
{
	// Find all symlink files in this folder
	CStringArray arraySymLinks;
	GetDirListing( sDir+sGroupFolder+SLASH+"*.include", arraySymLinks, false );
	SortCStringArray( arraySymLinks );
	for( unsigned s=0; s< arraySymLinks.size(); s++ )	// for each symlink in this dir, add it in as a song.
	{
		MsdFile		msdF;
		msdF.ReadFile( sDir+sGroupFolder+SLASH+arraySymLinks[s].c_str() );
		CString	sSymDestination = msdF.GetParam(0,1);	// Should only be 1 vale&param...period.
		
		Song* pNewSong = new Song;
		if( !pNewSong->LoadFromSongDir( sSymDestination ) )
			delete pNewSong; // The song failed to load.
		else
		{
			pNewSong->m_apNotes.clear();	// No memory hogs..
			pNewSong->m_BackgroundChanges.clear();

			pNewSong->m_bIsSymLink = true;	// Very important so we don't double-parse later
			pNewSong->m_sGroupName = sGroupFolder;
			m_pSongs.push_back( pNewSong );
		}
	}
}

void SongManager::PreloadSongImages()
{
	ASSERT( TEXTUREMAN );
	if( PREFSMAN->m_BannerCache != PrefsManager::BNCACHE_FULL )
		return;

	const vector<Song*> &songs = SONGMAN->GetAllSongs();
	unsigned i;
	for( i = 0; i < songs.size(); ++i )
	{
		if( !songs[i]->HasBanner() )
			continue;

		const RageTextureID ID = Banner::BannerTex( songs[i]->GetBannerPath() );
		TEXTUREMAN->CacheTexture( ID );
	}

	vector<Course*> courses;
	SONGMAN->GetAllCourses( courses, false );
	for( i = 0; i < courses.size(); ++i )
	{
		if( !courses[i]->HasBanner() )
			continue;

		const RageTextureID ID = Banner::BannerTex( courses[i]->m_sBannerPath );
		TEXTUREMAN->CacheTexture( ID );
	}
}

void SongManager::FreeSongs()
{
	for( unsigned i=0; i<m_pSongs.size(); i++ )
		SAFE_DELETE( m_pSongs[i] );
	m_pSongs.clear();

	m_sGroupBannerPaths.clear();
}


//
// Helper function for reading/writing scores
//
bool FileRead(RageFile& f, CString& sOut)
{
	if (f.AtEOF())
		return false;
	sOut = f.GetLine();
	return true;
}

bool FileRead(RageFile& f, int& iOut)
{
	CString s;
	if (!FileRead(f, s))
		return false;
	iOut = atoi(s);
	return true;
}

bool FileRead(RageFile& f, unsigned& uOut)
{
	CString s;
	if (!FileRead(f, s))
		return false;
	uOut = atoi(s);
	return true;
}

bool FileRead(RageFile& f, float& fOut)
{
	CString s;
	if (!FileRead(f, s))
		return false;
	fOut = (float)atof(s);
	return true;
}

void FileWrite(RageFile& f, const CString& sWrite)
{
	f.PutLine( sWrite );
}

void FileWrite(RageFile& f, int iWrite)
{
	f.PutLine( ssprintf("%d", iWrite) );
}

void FileWrite(RageFile& f, size_t uWrite)
{
	f.PutLine( ssprintf("%lu", uWrite) );
}

void FileWrite(RageFile& f, float fWrite)
{
	f.PutLine( ssprintf("%f", fWrite) );
}

#define WARN_AND_RETURN { LOG->Warn("Error parsing file '%s' at %s:%d",fn.c_str(),__FILE__,__LINE__); return; }

void SongManager::ReadStepsMemCardDataFromFile( CString fn, int mc )
{
	RageFile f(fn);
	if (!f.IsOpen() || f.GetError() != 0)
		return; /* don't warn if it just doesn't exist */

	int version;
	if( !FileRead(f, version) )
		WARN_AND_RETURN;
	if( version != STEPS_MEM_CARD_DATA_VERSION )
		WARN_AND_RETURN;

	int iNumSongs;
	if( !FileRead(f, iNumSongs) )
		WARN_AND_RETURN;

	for( int s=0; s<iNumSongs; s++ )
	{
		CString sSongDir;
		if( !FileRead(f, sSongDir) )
			WARN_AND_RETURN;

		Song* pSong = this->GetSongFromDir( sSongDir );

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


void SongManager::ReadCategoryRankingsFromFile( CString fn )
{
	RageFile f(fn);
	if (!f.IsOpen() || f.GetError() != 0)
		return; /* don't warn if it just doesn't exist */
	
	int version;
	if( !FileRead(f, version) )
		WARN_AND_RETURN;
	if( version != CATEGORY_RANKING_VERSION )
		WARN_AND_RETURN;

	for( int st=0; st<NUM_STEPS_TYPES; st++ )
	{
		for( int rc=0; rc<NUM_RANKING_CATEGORIES; rc++ )
		{
			m_CategoryDatas[st][rc].vHighScores.resize(NUM_RANKING_LINES);
			for( int l=0; l<NUM_RANKING_LINES; l++ )
			{
				CString sName;
				if( !FileRead(f, sName) )
					WARN_AND_RETURN;
				int iScore;
				if( !FileRead(f, iScore) )
					WARN_AND_RETURN;
				m_CategoryDatas[st][rc].vHighScores[l].sName = sName;
				m_CategoryDatas[st][rc].vHighScores[l].iScore = iScore;
			}
		}
	}
}

void SongManager::ReadCourseMemCardDataFromFile( CString fn, int mc )
{
	RageFile f(fn);
	if (!f.IsOpen() || f.GetError() != 0)
		return; /* don't warn if it just doesn't exist */
	
	int version;
	if( !FileRead(f, version) )
		WARN_AND_RETURN;
	if( version != COURSE_MEM_CARD_DATA_VERSION )
		WARN_AND_RETURN;

	int iNumCourses;
	if( !FileRead(f, iNumCourses) )
		WARN_AND_RETURN;

	for( int c=0; c<iNumCourses; c++ )
	{
		CString sPath;
		if( !FileRead(f, sPath) )
			WARN_AND_RETURN;

		Course* pCourse = GetCourseFromPath( sPath );
		if( pCourse == NULL )
			pCourse = GetCourseFromName( sPath );
		
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

void SongManager::InitMachineScoresFromDisk()
{
	// read old style notes scores
	ReadSM300NoteScores();

	// category ranking
	ReadCategoryRankingsFromFile( CATEGORY_RANKING_FILE );
	ReadCourseMemCardDataFromFile( MACHINE_COURSE_MEM_CARD_DATA, MEMORY_CARD_MACHINE );
	ReadStepsMemCardDataFromFile( MACHINE_STEPS_MEM_CARD_DATA, MEMORY_CARD_MACHINE );
}

void SongManager::ReadSM300NoteScores()
{
	IniFile ini;
	ini.SetPath( SM_300_STATISTICS_FILE );
	if( !ini.ReadFile() ) {
		LOG->Trace( "WARNING: Could not read SM 3.0 final statistics '%s'.", SM_300_STATISTICS_FILE );
		return;		// load nothing
	}

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
			Song* pSong = GetSongFromDir( sSongDir );
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


void SongManager::SaveCategoryRankingsToFile( CString fn )
{
	LOG->Trace("SongManager::SaveCategoryRankingsToFile");

	RageFile f(fn);
	if (!f.IsOpen() || f.GetError() != 0)
		return;

	FileWrite( f, CATEGORY_RANKING_VERSION );

	for( int st=0; st<NUM_STEPS_TYPES; st++ )
	{
		for( int rc=0; rc<NUM_RANKING_CATEGORIES; rc++ )
		{
			m_CategoryDatas[st][rc].vHighScores.resize(NUM_RANKING_LINES);
			for( int l=0; l<NUM_RANKING_LINES; l++ )
			{
				// tricky:  wipe out "name to fill in" markers
				if( IsRankingToFillIn(m_CategoryDatas[st][rc].vHighScores[l].sName) )
					m_CategoryDatas[st][rc].vHighScores[l].sName = "";

				FileWrite( f, m_CategoryDatas[st][rc].vHighScores[l].sName );
				FileWrite( f, m_CategoryDatas[st][rc].vHighScores[l].iScore );
			}
		}
	}
}

void SongManager::SaveCourseMemCardDataToFile( CString fn, int mc )
{
	LOG->Trace("SongManager::SaveCourseMemCardDataToFile");

	RageFile f(fn);
	if (!f.IsOpen() || f.GetError() != 0)
		return;
	
	FileWrite( f, COURSE_MEM_CARD_DATA_VERSION );

	FileWrite( f, m_pCourses.size() );

	for( unsigned c=0; c<m_pCourses.size(); c++ )	// foreach course
	{
		Course* pCourse = m_pCourses[c];
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

void SongManager::SaveStepsMemCardDataToFile( CString fn, int mc )
{
	LOG->Trace("SongManager::SaveStepsMemCardDataToFile %s", fn.c_str());

	RageFile f(fn);
	if (!f.IsOpen() || f.GetError() != 0)
		return;
	
	FileWrite( f, STEPS_MEM_CARD_DATA_VERSION );
	
	FileWrite( f, m_pSongs.size() );

	for( unsigned s=0; s<m_pSongs.size(); s++ )	// foreach song
	{
		Song* pSong = m_pSongs[s];
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


CString SongManager::GetGroupBannerPath( CString sGroupName )
{
	unsigned i;
	for(i = 0; i < m_sGroupNames.size(); ++i)
		if( sGroupName == m_sGroupNames[i] ) break;

	if( i == m_sGroupNames.size() )
		return "";

	return m_sGroupBannerPaths[i];
}

void SongManager::GetGroupNames( CStringArray &AddTo )
{
	AddTo.insert(AddTo.end(), m_sGroupNames.begin(), m_sGroupNames.end() );
}

bool SongManager::DoesGroupExist( CString sGroupName )
{
	for( unsigned i = 0; i < m_sGroupNames.size(); ++i )
		if( !m_sGroupNames[i].CompareNoCase(sGroupName) )
			return true;

	return false;
}

RageColor SongManager::GetGroupColor( const CString &sGroupName )
{
	UpdateMetrics();

	// search for the group index
	unsigned i;
	for( i=0; i<m_sGroupNames.size(); i++ )
	{
		if( m_sGroupNames[i] == sGroupName )
			break;
	}
	ASSERT( i != m_sGroupNames.size() );	// this is not a valid group

	return g_vGroupColors[i%g_vGroupColors.size()];
}

RageColor SongManager::GetSongColor( const Song* pSong )
{
	ASSERT( pSong );

	/* XXX:
	 * Previously, this matched all notes, which set a song to "extra" if it
	 * had any 10-foot steps at all, even edits or doubles.
	 *
	 * For now, only look at notes for the current note type.  This means that
	 * if a song has 10-foot steps on Doubles, it'll only show up red in Doubles.
	 * That's not too bad, I think.  This will also change it in the song scroll,
	 * which is a little odd but harmless. 
	 *
	 * XXX: Ack.  This means this function can only be called when we have a style
	 * set up, which is too restrictive.  How to handle this?
	 *
	 * XXX: Once we support edits, ignore them, too. */
//	const StepsType nt = GAMESTATE->GetCurrentStyleDef()->m_StepsType;
	for( unsigned i=0; i<pSong->m_apNotes.size(); i++ )
	{
		const Steps* pNotes = pSong->m_apNotes[i];
		if( pNotes->GetDifficulty() == DIFFICULTY_CHALLENGE )
			continue;

//		if(pNotes->m_StepsType != nt)
//			continue;

		if( pNotes->GetMeter() >= 10 && PREFSMAN->m_bTenFooterInRed )
			return EXTRA_COLOR;
	}

	return GetGroupColor( pSong->m_sGroupName );
}

#define BEGINNER_DESCRIPTION	THEME->GetMetric ("Common","Beginner")
#define EASY_DESCRIPTION		THEME->GetMetric ("Common","Easy")
#define MEDIUM_DESCRIPTION		THEME->GetMetric ("Common","Medium")
#define HARD_DESCRIPTION		THEME->GetMetric ("Common","Hard")
#define CHALLENGE_DESCRIPTION	THEME->GetMetric ("Common","Challenge")

RageColor SongManager::GetDifficultyColor( Difficulty dc ) const
{
	switch( dc )
	{
	case DIFFICULTY_BEGINNER:	return BEGINNER_COLOR;
	case DIFFICULTY_EASY:		return EASY_COLOR;
	case DIFFICULTY_MEDIUM:		return MEDIUM_COLOR;
	case DIFFICULTY_HARD:		return HARD_COLOR;
	case DIFFICULTY_CHALLENGE:	return CHALLENGE_COLOR;
	default:	ASSERT(0);	return CHALLENGE_COLOR;
	}
}

CString SongManager::GetDifficultyThemeName( Difficulty dc ) const
{
	switch( dc )
	{
	case DIFFICULTY_BEGINNER:	return BEGINNER_DESCRIPTION;
	case DIFFICULTY_EASY:		return EASY_DESCRIPTION;
	case DIFFICULTY_MEDIUM:		return MEDIUM_DESCRIPTION;
	case DIFFICULTY_HARD:		return HARD_DESCRIPTION;
	case DIFFICULTY_CHALLENGE:	return CHALLENGE_DESCRIPTION;
	default:	ASSERT(0);		return "";  // something else
	}
}

void SongManager::GetSongs( vector<Song*> &AddTo, CString sGroupName, int iMaxStages ) const
{
	AddTo.clear();

	for( unsigned i=0; i<m_pSongs.size(); i++ )
		if( sGroupName==GROUP_ALL_MUSIC || sGroupName==m_pSongs[i]->m_sGroupName )
			if( GetNumStagesForSong(m_pSongs[i])<=iMaxStages )
				AddTo.push_back( m_pSongs[i] );
}

int SongManager::GetNumSongs() const
{
	return m_pSongs.size();
}

int SongManager::GetNumGroups() const
{
	return m_sGroupNames.size();
}

int SongManager::GetNumCourses() const
{
	return m_pCourses.size();
}

CString SongManager::ShortenGroupName( CString sLongGroupName )
{
	sLongGroupName.Replace( "Dance Dance Revolution", "DDR" );
	sLongGroupName.Replace( "dance dance revolution", "DDR" );
	sLongGroupName.Replace( "DANCE DANCE REVOLUTION", "DDR" );
	sLongGroupName.Replace( "Pump It Up", "PIU" );
	sLongGroupName.Replace( "pump it up", "PIU" );
	sLongGroupName.Replace( "PUMP IT UP", "PIU" );
	sLongGroupName.Replace( "ParaParaParadise", "PPP" );
	sLongGroupName.Replace( "paraparaparadise", "PPP" );
	sLongGroupName.Replace( "PARAPARAPARADISE", "PPP" );
	sLongGroupName.Replace( "Para Para Paradise", "PPP" );
	sLongGroupName.Replace( "para para paradise", "PPP" );
	sLongGroupName.Replace( "PARA PARA PARADISE", "PPP" );
	sLongGroupName.Replace( "Dancing Stage", "DS" );
	sLongGroupName.Replace( "Dancing Stage", "DS" );
	sLongGroupName.Replace( "Dancing Stage", "DS" );
	sLongGroupName.Replace( "Ez2dancer", "EZ2" );
	sLongGroupName.Replace( "Ez 2 Dancer", "EZ2");
	sLongGroupName.Replace( "Technomotion", "TM");
	sLongGroupName.Replace( "Dance Station 3DDX", "3DDX");
	sLongGroupName.Replace( "DS3DDX", "3DDX");
	sLongGroupName.Replace( "BeatMania", "BM");
	sLongGroupName.Replace( "Beatmania", "BM");
	sLongGroupName.Replace( "BEATMANIA", "BM");
	sLongGroupName.Replace( "beatmania", "BM");
	return sLongGroupName;
}

int SongManager::GetNumStagesForSong( const Song* pSong )
{
	ASSERT( pSong );
	if( pSong->m_fMusicLengthSeconds > PREFSMAN->m_fMarathonVerSongSeconds )
		return 3;
	if( pSong->m_fMusicLengthSeconds > PREFSMAN->m_fLongVerSongSeconds )
		return 2;
	else
		return 1;
}

void SongManager::InitCoursesFromDisk( LoadingWindow *ld )
{
	unsigned i;

	LOG->Trace( "Loading courses." );

	//
	// Load courses from in Courses dir
	//
	CStringArray saCourseFiles;
	GetDirListing( COURSES_DIR "*.crs", saCourseFiles, false, true );
	for( i=0; i<saCourseFiles.size(); i++ )
	{
		Course* pCourse = new Course;
		pCourse->LoadFromCRSFile( saCourseFiles[i] );
		m_pCourses.push_back( pCourse );

		if( ld )
		{
			ld->SetText( ssprintf("Loading courses...\n%s\n%s",
				"Courses",
				Basename(saCourseFiles[i]).c_str()));
			ld->Paint();
		}

	}


	// Find all group directories in Courses dir
	CStringArray arrayGroupDirs;
	GetDirListing( COURSES_DIR "*", arrayGroupDirs, true );
	SortCStringArray( arrayGroupDirs );
	
	for( i=0; i< arrayGroupDirs.size(); i++ )	// for each dir in /Courses/
	{
		CString sGroupDirName = arrayGroupDirs[i];

		if( 0 == stricmp( sGroupDirName, "cvs" ) )	// the directory called "CVS"
			continue;		// ignore it

		// Find all CRS files in this group directory
		CStringArray arrayCoursePaths;
		GetDirListing( COURSES_DIR + sGroupDirName + SLASH + "*.crs", arrayCoursePaths, false, true );
		SortCStringArray( arrayCoursePaths );

		for( unsigned j=0; j<arrayCoursePaths.size(); j++ )
		{
			if( ld )
			{
				ld->SetText( ssprintf("Loading courses...\n%s\n%s",
					Basename(arrayGroupDirs[i]).c_str(),
					Basename(arrayCoursePaths[j]).c_str()));
				ld->Paint();
			}

			Course* pCourse = new Course;
			pCourse->LoadFromCRSFile( arrayCoursePaths[j] );
			m_pCourses.push_back( pCourse );
		}
	}
}
	
void SongManager::InitAutogenCourses()
{
	//
	// Create group courses for Endless and Nonstop
	//
	CStringArray saGroupNames;
	this->GetGroupNames( saGroupNames );
	unsigned g;
	for( g=0; g<saGroupNames.size(); g++ )	// foreach Group
	{
		CString sGroupName = saGroupNames[g];
		vector<Song*> apGroupSongs;
		GetSongs( apGroupSongs, sGroupName );

		Course* pCourse;

		pCourse = new Course;
		pCourse->AutogenEndlessFromGroup( sGroupName, apGroupSongs );
		m_pCourses.push_back( pCourse );

		pCourse = new Course;
		pCourse->AutogenNonstopFromGroup( sGroupName, apGroupSongs );
		m_pCourses.push_back( pCourse );
	}
}


void SongManager::FreeCourses()
{
	for( unsigned i=0; i<m_pCourses.size(); i++ )
		delete m_pCourses[i];
	m_pCourses.clear();
}

/* Called periodically to wipe out cached NoteData.  This is called when we change
 * screens. */
void SongManager::CompressSongs()
{
	for( unsigned i=0; i<m_pSongs.size(); i++ )
	{
		Song* pSong = m_pSongs[i];
		for( unsigned n=0; n<pSong->m_apNotes.size(); n++ )
		{
			Steps* pNotes = pSong->m_apNotes[n];
			pNotes->Compress();
		}
	}
}

void SongManager::GetAllCourses( vector<Course*> &AddTo, bool bIncludeAutogen )
{
	for( unsigned i=0; i<m_pCourses.size(); i++ )
		if( bIncludeAutogen || !m_pCourses[i]->m_bIsAutogen )
			AddTo.push_back( m_pCourses[i] );
}

void SongManager::GetNonstopCourses( vector<Course*> &AddTo, bool bIncludeAutogen )
{
	for( unsigned i=0; i<m_pCourses.size(); i++ )
		if( m_pCourses[i]->IsNonstop() )
			if( bIncludeAutogen || !m_pCourses[i]->m_bIsAutogen )
				AddTo.push_back( m_pCourses[i] );
}

void SongManager::GetOniCourses( vector<Course*> &AddTo, bool bIncludeAutogen )
{
	for( unsigned i=0; i<m_pCourses.size(); i++ )
		if( m_pCourses[i]->IsOni() )
			if( bIncludeAutogen || !m_pCourses[i]->m_bIsAutogen )
				AddTo.push_back( m_pCourses[i] );
}

void SongManager::GetEndlessCourses( vector<Course*> &AddTo, bool bIncludeAutogen )
{
	for( unsigned i=0; i<m_pCourses.size(); i++ )
		if( m_pCourses[i]->IsEndless() )
			if( bIncludeAutogen || !m_pCourses[i]->m_bIsAutogen )
				AddTo.push_back( m_pCourses[i] );
}


bool SongManager::GetExtraStageInfoFromCourse( bool bExtra2, CString sPreferredGroup,
								   Song*& pSongOut, Steps*& pNotesOut, PlayerOptions& po_out, SongOptions& so_out )
{
	const CString sCourseSuffix = sPreferredGroup + SLASH + (bExtra2 ? "extra2" : "extra1") + ".crs";
	CString sCoursePath = SONGS_DIR + sCourseSuffix;
	if( !DoesFileExist(sCoursePath) ) 
	{
		/* try alternative song folders */
		for( unsigned i=0; i<PREFSMAN->m_asAdditionalSongFolders.size(); i++ )
		{
			sCoursePath = PREFSMAN->m_asAdditionalSongFolders[i] + SLASH + sCourseSuffix;
			if( DoesFileExist(sCoursePath) ) 
				break;
		}
	}

	if( !DoesFileExist(sCoursePath) && PREFSMAN->m_DWIPath.size() )
		sCoursePath = PREFSMAN->m_DWIPath + SLASH "Songs" SLASH + sCourseSuffix;

	/* Couldn't find course in DWI path or alternative song folders */
	if( !DoesFileExist(sCoursePath) )
		return false;

	Course course;
	course.LoadFromCRSFile( sCoursePath );
	if( course.GetEstimatedNumStages() <= 0 ) return false;

	vector<Course::Info> ci;
	course.GetCourseInfo( GAMESTATE->GetCurrentStyleDef()->m_StepsType, ci );
	if( ci.empty() )
		return false;

	po_out.Init();
	po_out.FromString( ci[0].Modifiers );
	so_out.Init();
	so_out.FromString( ci[0].Modifiers );

	pSongOut = ci[0].pSong;
	pNotesOut = ci[0].pNotes;
	return true;
}

/* Return true if n1 < n2. */
bool CompareNotesPointersForExtra(const Steps *n1, const Steps *n2)
{
	/* Equate CHALLENGE to HARD. */
	Difficulty d1 = min(n1->GetDifficulty(), DIFFICULTY_HARD);
	Difficulty d2 = min(n2->GetDifficulty(), DIFFICULTY_HARD);

	if(d1 < d2) return true;
	if(d1 > d2) return false;
	/* n1 difficulty == n2 difficulty */

	if(CompareNotesPointersByMeter(n1,n2)) return true;
	if(CompareNotesPointersByMeter(n2,n1)) return false;
	/* n1 meter == n2 meter */

	return CompareNotesPointersByRadarValues(n1,n2);
}

void SongManager::GetExtraStageInfo( bool bExtra2, const StyleDef *sd, 
								   Song*& pSongOut, Steps*& pNotesOut, PlayerOptions& po_out, SongOptions& so_out )
{
	CString sGroup = GAMESTATE->m_sPreferredGroup;
	if(sGroup == GROUP_ALL_MUSIC)
	{
		ASSERT(GAMESTATE->m_pCurSong);
		sGroup = GAMESTATE->m_pCurSong->m_sGroupName;
	}
	/* XXX: Temporary extra info: someone reported an odd assertion failure here. */
//	ASSERT(sGroup != "");
	if( sGroup == "" )
	{
		LOG->Warn("GetExtraStageInfo error: sGroup == \"\", m_pCurSong %p '%s' '%s'",
		GAMESTATE->m_pCurSong,
		GAMESTATE->m_pCurSong? GAMESTATE->m_pCurSong->GetSongDir().c_str():"",
		GAMESTATE->m_pCurSong? GAMESTATE->m_pCurSong->m_sGroupName.c_str():"");
		ASSERT(0); /* get a backtrace */
	}

	if(GetExtraStageInfoFromCourse(bExtra2, sGroup, pSongOut, pNotesOut, po_out, so_out))
		return;
	
	// Choose a hard song for the extra stage
	Song*	pExtra1Song = NULL;		// the absolute hardest Song and Steps.  Use this for extra stage 1.
	Steps*	pExtra1Notes = NULL;
	Song*	pExtra2Song = NULL;		// a medium-hard Song and Steps.  Use this for extra stage 2.
	Steps*	pExtra2Notes = NULL;
	
	vector<Song*> apSongs;
	SONGMAN->GetSongs( apSongs, sGroup );
	for( unsigned s=0; s<apSongs.size(); s++ )	// foreach song
	{
		Song* pSong = apSongs[s];

		vector<Steps*> apNotes;
		pSong->GetSteps( apNotes, sd->m_StepsType );
		for( unsigned n=0; n<apNotes.size(); n++ )	// foreach Steps
		{
			Steps* pNotes = apNotes[n];

			if( pExtra1Notes == NULL || CompareNotesPointersForExtra(pExtra1Notes,pNotes) )	// pNotes is harder than pHardestNotes
			{
				pExtra1Song = pSong;
				pExtra1Notes = pNotes;
			}

			// for extra 2, we don't want to choose the hardest notes possible.  So, we'll disgard Steps with meter > 8
			if(	bExtra2  &&  pNotes->GetMeter() > 8 )	
				continue;	// skip
			if( pExtra2Notes == NULL  ||  CompareNotesPointersForExtra(pExtra2Notes,pNotes) )	// pNotes is harder than pHardestNotes
			{
				pExtra2Song = pSong;
				pExtra2Notes = pNotes;
			}
		}
	}

	if( pExtra2Song == NULL  &&  pExtra1Song != NULL )
	{
		pExtra2Song = pExtra1Song;
		pExtra2Notes = pExtra1Notes;
	}

	// If there are any notes at all that match this StepsType, everything should be filled out.
	// Also, it's guaranteed that there is at least one Steps that matches the StepsType because the player
	// had to play something before reaching the extra stage!
	ASSERT( pExtra2Song && pExtra1Song && pExtra2Notes && pExtra1Notes );

	pSongOut = (bExtra2 ? pExtra2Song : pExtra1Song);
	pNotesOut = (bExtra2 ? pExtra2Notes : pExtra1Notes);


	po_out.Init();
	so_out.Init();
	po_out.m_fScrolls[PlayerOptions::SCROLL_REVERSE] = 1;
	po_out.m_fScrollSpeed = 1.5f;
	so_out.m_DrainType = (bExtra2 ? SongOptions::DRAIN_SUDDEN_DEATH : SongOptions::DRAIN_NO_RECOVER);
	po_out.m_fDark = 1;
}

Song* SongManager::GetRandomSong()
{
	if( m_pSongs.empty() )
		return NULL;

	return SONGMAN->m_pSongs[ rand()%m_pSongs.size() ];
}

Song* SongManager::GetSongFromDir( CString sDir )
{
	if( sDir.Right(1) != SLASH )
		sDir += SLASH;

	for( unsigned int i=0; i<m_pSongs.size(); i++ )
		if( sDir.CompareNoCase(m_pSongs[i]->GetSongDir()) == 0 )
			return m_pSongs[i];

	return NULL;
}

Course* SongManager::GetCourseFromPath( CString sPath )
{
	for( unsigned int i=0; i<m_pCourses.size(); i++ )
		if( sPath.CompareNoCase(m_pCourses[i]->m_sPath) == 0 )
			return m_pCourses[i];

	return NULL;
}

Course* SongManager::GetCourseFromName( CString sName )
{
	for( unsigned int i=0; i<m_pCourses.size(); i++ )
		if( sName.CompareNoCase(m_pCourses[i]->m_sName) == 0 )
			return m_pCourses[i];

	return NULL;
}


/*
 * GetSongDir() contains a path to the song, possibly a full path, eg:
 * Songs\Group\SongName                   or 
 * My Other Song Folder\Group\SongName    or
 * c:\Corny J-pop\Group\SongName
 *
 * Most course group names are "Group\SongName", so we want to
 * match against the last two elements. Let's also support
 * "SongName" alone, since the group is only important when it's
 * potentially ambiguous.
 *
 * Let's *not* support "Songs\Group\SongName" in course files.
 * That's probably a common error, but that would result in
 * course files floating around that only work for people who put
 * songs in "Songs"; we don't want that.
 */

Song *SongManager::FindSong( CString sPath )
{
	sPath.Replace( "\\", "/" );
	CStringArray bits;
	split( sPath, "/", bits );

	if( bits.size() == 1 )
		return FindSong( "", bits[0] );
	else if( bits.size() == 2 )
		return FindSong( bits[0], bits[1] );

	return NULL;	
}

Song *SongManager::FindSong( CString sGroup, CString sSong )
{
	// foreach song
	for( unsigned i = 0; i < m_pSongs.size(); i++ )
	{
		if( m_pSongs[i]->Matches(sGroup, sSong) )
			return m_pSongs[i];
	}

	return NULL;	
}

Course *SongManager::FindCourse( CString sName )
{
	for( unsigned i = 0; i < m_pCourses.size(); i++ )
	{
		if( !sName.CompareNoCase(m_pCourses[i]->m_sName) )
			return m_pCourses[i];
	}

	return NULL;
}

void SongManager::UpdateBest()
{
	for( int i = 0; i < NUM_MEMORY_CARDS; ++i )
	{
		m_pBestSongs[i] = m_pSongs;
		SortSongPointerArrayByMostPlayed( m_pBestSongs[i], (MemoryCard) i );

		m_pBestCourses[i] = m_pCourses;
		SortCoursePointerArrayByMostPlayed( m_pBestCourses[i], (MemoryCard) i );
	}
}

void SongManager::UpdateRankingCourses()
{
	/*  Updating the ranking courses data is fairly expensive
	 *  since it involves comparing strings.  Do so sparingly.
	 */
	CStringArray RankingCourses;

	split( THEME->GetMetric("ScreenRanking","CoursesToShow"),",", RankingCourses);

	for(unsigned i=0; i < m_pCourses.size(); i++)
	{
		if (m_pCourses[i]->GetEstimatedNumStages() > 7)
			m_pCourses[i]->SortOrder_Ranking = 3;
		else
			m_pCourses[i]->SortOrder_Ranking = 2;
		
		for(unsigned j = 0; j < RankingCourses.size(); j++)
			if (!RankingCourses[j].CompareNoCase(m_pCourses[i]->m_sPath))
				m_pCourses[i]->SortOrder_Ranking = 1;
	}
}

static CString HTMLQuoteDoubleQuotes( CString str )
{
	str.Replace( "\"", "&quot;" );
	return str;
}

static bool CompareStepsPointersByTypeAndDifficulty(const Steps *pStep1, const Steps *pStep2)
{
	if( pStep1->m_StepsType < pStep2->m_StepsType )
		return true;
	if( pStep1->m_StepsType > pStep2->m_StepsType )
		return false;
	return pStep1->GetDifficulty() < pStep2->GetDifficulty();
}

static void SortStepsByTypeAndDifficulty( vector<Steps*> &arraySongPointers )
{
	sort( arraySongPointers.begin(), arraySongPointers.end(), CompareStepsPointersByTypeAndDifficulty );
}

static void HTMLWritePerGameHeader( RageFile &f, Game game )
{
	const GameDef* pGameDef = GAMEMAN->GetGameDefForGame(game);

	vector<StepsType> aStepsTypes;
	GAMEMAN->GetNotesTypesForGame( game, aStepsTypes );

	f.PutLine( ssprintf("<h1>%s</h1>", pGameDef->m_szName) );

	f.PutLine( "<table border='1'>" );
	f.Write( "<tr><td>title</td>" );

	unsigned j;
	for( j=0; j<aStepsTypes.size(); j++ )
	{
		StepsType st = aStepsTypes[j];

		f.PutLine( ssprintf("<td colspan='%d'>%s</td>", NUM_DIFFICULTIES, GAMEMAN->NotesTypeToString(st).c_str()) );
	}
	f.PutLine( "</tr>" );

	f.Write( "<tr><td>&nbsp;</td>" );
	for( j=0; j<aStepsTypes.size(); j++ )
	{
		for( unsigned k=0; k<NUM_DIFFICULTIES; k++ )
		{
			Difficulty d = (Difficulty)k;
			f.PutLine( ssprintf("<td>%s</td>", Capitalize(DifficultyToString(d).Left(3)).c_str()) );
		}
	}

	f.PutLine( "</tr>" );
}

// TODO: Move this to a different file.  No need to clutter SongManager.
void SongManager::WriteStatsWebPage()
{
	RageFile f;
	if( !f.Open( STATS_PATH, RageFile::WRITE ) )
		return;

	f.PutLine( "<html>" );
	f.PutLine( "<head>" );
	f.PutLine( "<META HTTP-EQUIV=\"Content-Type\" CONTENT=\"text/html; charset=UTF-8\">" );
	f.PutLine( ssprintf("<title>%s</title>", PRODUCT_NAME_VER) );
	f.PutLine( "</head>" );
	f.PutLine( "<body>" );

	vector<Song*> vSongs = m_pSongs;
	SortSongPointerArrayByGroupAndTitle( vSongs );

	//
	// Print song list
	//
	f.PutLine( "<table border='1'>" );
	for( unsigned i=0; i<vSongs.size(); i++ )
	{
		Song* pSong = m_pSongs[i];
		f.Write( "<tr>" );
		/* XXX: We can't call pSong->HasBanner on every song; it'll effectively re-traverse the entire
		 * song directory tree checking if each banner file really exists.
		 *
		 * (Note for testing this: remember that we'll cache directories for a time; this is only slow if
		 * the directory cache expires before we get here.) */
		//CString sImagePath = pSong->HasBanner() ? pSong->GetBannerPath() : (pSong->HasBackground() ? pSong->GetBackgroundPath() : "" );
		CString sImagePath = pSong->GetBannerPath();
		if( sImagePath.empty() )
			f.Write( "<td> </td>" );
		else
			f.Write( ssprintf("<td><img src=\"%s\" width='120'></td>", HTMLQuoteDoubleQuotes(sImagePath).c_str()) );
		
		f.Write( ssprintf("<td>%s<br>", pSong->GetTranslitMainTitle().c_str()) );
		f.Write( ssprintf("<font size='-1'>%s</font><br>", pSong->GetTranslitSubTitle().c_str()) );
		f.Write( ssprintf("<font size='-1'><i>%s</i></font></td>", pSong->GetTranslitArtist().c_str()) );
		f.PutLine( "</tr>" );
	}
	f.PutLine( "</table>\n<br>" );


	//
	// Print steps tables
	//
	for( int g=0; g<NUM_GAMES; g++ )
	{
		Game game = (Game)g;
		
		vector<StepsType> aStepsTypes;
		GAMEMAN->GetNotesTypesForGame( game, aStepsTypes );

		bool WroteHeader = false;
		for( unsigned i=0; i<vSongs.size(); i++ )
		{
			/* Get the steps for this game type. */
			Song* pSong = m_pSongs[i];
			vector<Steps*> Steps;
			unsigned j;
			for( j=0; j < aStepsTypes.size(); j++ )
				pSong->GetSteps( Steps, (StepsType) aStepsTypes[j], DIFFICULTY_INVALID, -1, -1, "", false );

			/* Don't write anything for songs that have no steps at all for this
			 * game.  Otherwise, we'll write pages and pages of empty fields for
			 * all of the less-used game types. */
			if( Steps.size() == 0 )
				continue;	// skip	

			/* We have some steps for this game.  Make sure we've written the game header. */
			if( !WroteHeader )
			{
				HTMLWritePerGameHeader( f, game );
				WroteHeader = true;
			}


			f.PutLine( "<tr>" );
			
			f.Write( ssprintf("<td>%s</td>", pSong->GetTranslitMainTitle().c_str()) );

			SortStepsByTypeAndDifficulty( Steps );

			unsigned CurSteps = 0;
			for( j=0; j<aStepsTypes.size(); j++ )
			{
				for( int k=0; k<NUM_DIFFICULTIES; k++ )
				{
					if( CurSteps < Steps.size() &&
						Steps[CurSteps]->m_StepsType == aStepsTypes[j] &&
						Steps[CurSteps]->GetDifficulty() == k )
					{
						f.PutLine( ssprintf("<td>%d</td>", Steps[CurSteps]->GetMeter()) );
						++CurSteps;
					}
					else
						f.PutLine( "<td>&nbsp;</td>" );
				}
			}

			f.Write( "</tr>" );
		}
		if( WroteHeader )
			f.PutLine( "</table>\n<br>" ); // footer
	}

	f.PutLine( "</body>" );
	f.PutLine( "</html>" );
}
