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
#include "RageTextureManager.h"
#include "Banner.h"
#include "ProfileManager.h"

SongManager*	SONGMAN = NULL;	// global and accessable from anywhere in our program

#define SONGS_DIR				"Songs/"
#define COURSES_DIR				"Courses/"


#define NUM_GROUP_COLORS	THEME->GetMetricI("SongManager","NumGroupColors")
#define GROUP_COLOR( i )	THEME->GetMetricC("SongManager",ssprintf("GroupColor%d",i+1))
CachedThemeMetricC BEGINNER_COLOR		("SongManager","BeginnerColor");
CachedThemeMetricC EASY_COLOR			("SongManager","EasyColor");
CachedThemeMetricC MEDIUM_COLOR			("SongManager","MediumColor");
CachedThemeMetricC HARD_COLOR			("SongManager","HardColor");
CachedThemeMetricC CHALLENGE_COLOR		("SongManager","ChallengeColor");
CachedThemeMetricC EXTRA_COLOR			("SongManager","ExtraColor");
CachedThemeMetricI EXTRA_COLOR_METER	("SongManager","ExtraColorMeter");

vector<RageColor> g_vGroupColors;
RageTimer g_LastMetricUpdate; /* can't use RageTimer globally */

static void UpdateMetrics()
{
	if( !g_LastMetricUpdate.IsZero() && g_LastMetricUpdate.PeekDeltaTime() < 1 )
		return;

	g_LastMetricUpdate.Touch();
	g_vGroupColors.clear();
	for( int i=0; i<NUM_GROUP_COLORS; i++ )
		g_vGroupColors.push_back( GROUP_COLOR(i) );

	BEGINNER_COLOR.Refresh();
	EASY_COLOR.Refresh();
	MEDIUM_COLOR.Refresh();
	HARD_COLOR.Refresh();
	CHALLENGE_COLOR.Refresh();
	EXTRA_COLOR.Refresh();
	EXTRA_COLOR_METER.Refresh();
}

SongManager::SongManager( LoadingWindow *ld )
{
	g_LastMetricUpdate.SetZero();
	UpdateMetrics();

	/* We initialize things that assume they can get at SONGMAN; we only
	 * init one of these, so hook us up to it immediately. */
	SONGMAN = this;
	try
	{
		InitSongsFromDisk( ld );
		InitCoursesFromDisk( ld );
		InitAutogenCourses();

	} catch(...) {
		SONGMAN = NULL;
		throw;
	}
}

SongManager::~SongManager()
{
	FreeSongs();
	FreeCourses();
}

void SongManager::Reload()
{
	FlushDirCache();

	PROFILEMAN->SaveMachineScoresToDisk();
	FreeSongs();
	FreeCourses();

	m_sGroupNames.clear();
	m_sGroupBannerPaths.clear();

	/* Always check songs for changes. */
	const bool OldVal = PREFSMAN->m_bFastLoad;
	PREFSMAN->m_bFastLoad = false;

	InitSongsFromDisk(NULL);
	InitCoursesFromDisk(NULL);
	InitAutogenCourses();
	PROFILEMAN->InitMachineScoresFromDisk();

	PREFSMAN->m_bFastLoad = OldVal;
}

void SongManager::InitSongsFromDisk( LoadingWindow *ld )
{
	RageTimer tm;
	LoadStepManiaSongDir( SONGS_DIR, ld );
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
		sBannerPath = sDir+sGroupDirName+"/"+arrayGroupBanners[0] ;
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

	LOG->Trace( "Group banner for '%s' is '%s'.", sGroupDirName.c_str(), 
		sBannerPath != ""? sBannerPath.c_str():"(none)" );
	m_sGroupNames.push_back( sGroupDirName );
	m_sGroupBannerPaths.push_back(sBannerPath);
}

void SongManager::LoadStepManiaSongDir( CString sDir, LoadingWindow *ld )
{
	/* Make sure sDir has a trailing slash. */
	if( sDir.Right(1) != "/" )
		sDir += "/";

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
		GetDirListing( sDir+sGroupDirName + "/*", arraySongDirs, true, true );
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
	GetDirListing( sDir+sGroupFolder+"/*.include", arraySymLinks, false );
	SortCStringArray( arraySymLinks );
	for( unsigned s=0; s< arraySymLinks.size(); s++ )	// for each symlink in this dir, add it in as a song.
	{
		MsdFile		msdF;
		msdF.ReadFile( sDir+sGroupFolder+"/"+arraySymLinks[s].c_str() );
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
	RAGE_ASSERT_M( i != m_sGroupNames.size(), sGroupName );	// this is not a valid group

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

		if( pNotes->GetMeter() >= EXTRA_COLOR_METER )
			return EXTRA_COLOR;
	}

	return GetGroupColor( pSong->m_sGroupName );
}

RageColor SongManager::GetDifficultyColor( Difficulty dc ) const
{
	switch( dc )
	{
	case DIFFICULTY_BEGINNER:	return BEGINNER_COLOR;
	case DIFFICULTY_EASY:		return EASY_COLOR;
	case DIFFICULTY_MEDIUM:		return MEDIUM_COLOR;
	case DIFFICULTY_HARD:		return HARD_COLOR;
	case DIFFICULTY_CHALLENGE:	return CHALLENGE_COLOR;
	default:	ASSERT(0);		return CHALLENGE_COLOR;
	}
}

CString SongManager::GetDifficultyThemeName( Difficulty dc ) const
{
	CString sMetricName = "Difficulty" + Capitalize( DifficultyToString(dc) );
	return THEME->GetMetric( "Common", sMetricName );
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
		GetDirListing( COURSES_DIR + sGroupDirName + "/*.crs", arrayCoursePaths, false, true );
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
void SongManager::Cleanup()
{
	unsigned i;
	for( i=0; i<m_pSongs.size(); i++ )
	{
		Song* pSong = m_pSongs[i];
		for( unsigned n=0; n<pSong->m_apNotes.size(); n++ )
		{
			Steps* pNotes = pSong->m_apNotes[n];
			pNotes->Compress();
		}
	}

	/* Erase cached course info. */
	for( i=0; i < m_pCourses.size(); i++ )
		m_pCourses[i]->ClearCache();
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
	const CString sCourseSuffix = sPreferredGroup + "/" + (bExtra2 ? "extra2" : "extra1") + ".crs";
	CString sCoursePath = SONGS_DIR + sCourseSuffix;

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
	if( sDir.Right(1) != "/" )
		sDir += "/";

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

