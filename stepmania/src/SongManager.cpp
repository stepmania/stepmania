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
#include "NotesLoaderDWI.h"

#include "GameState.h"
#include "PrefsManager.h"
#include "RageException.h"
#include "RageTimer.h"
#include "arch/LoadingWindow/LoadingWindow.h"

#include "AnnouncerManager.h"
#include "ThemeManager.h"
#include "GameManager.h"

#include <fstream>

SongManager*	SONGMAN = NULL;	// global and accessable from anywhere in our program


const CString CATEGORY_RANKING_FILE = "Data/CategoryRanking.dat";
const CString COURSE_RANKING_FILE = "Data/CourseRanking.dat";
const CString NOTES_SCORES_FILE[NUM_MEMORY_CARDS] = { "Data/Player1NotesScores.dat", "Data/Player2NotesScores.dat", "Data/MachineNotesScores.dat" };
const CString COURSE_SCORES_FILE[NUM_MEMORY_CARDS] = { "Data/Player1CourseScores.dat", "Data/Player2CourseScores.dat", "Data/MachineCourseScores.dat" };
const int CATEGORY_RANKING_VERSION = 1;
const int COURSE_RANKING_VERSION = 1;
const int NOTES_SCORES_VERSION = 2;
const int COURSE_SCORES_VERSION = 1;


#define NUM_GROUP_COLORS	THEME->GetMetricI("SongManager","NumGroupColors")
#define GROUP_COLOR( i )	THEME->GetMetricC("SongManager",ssprintf("GroupColor%d",i+1))
#define BEGINNER_COLOR		THEME->GetMetricC("SongManager","BeginnerColor")
#define EASY_COLOR			THEME->GetMetricC("SongManager","EasyColor")
#define MEDIUM_COLOR		THEME->GetMetricC("SongManager","MediumColor")
#define HARD_COLOR			THEME->GetMetricC("SongManager","HardColor")
#define CHALLENGE_COLOR		THEME->GetMetricC("SongManager","ChallengeColor")
#define EXTRA_COLOR			THEME->GetMetricC("SongManager","ExtraColor")

vector<RageColor> g_vGroupColors;
RageColor g_ExtraColor;


SongManager::SongManager( LoadingWindow *ld )
{
	/* We initialize things that assume they can get at SONGMAN; we only
	 * init one of these, so hook us up to it immediately. */
	SONGMAN = this;
	try
	{
		g_vGroupColors.clear();
		for( int i=0; i<NUM_GROUP_COLORS; i++ )
			g_vGroupColors.push_back( GROUP_COLOR(i) );
		g_ExtraColor = EXTRA_COLOR;

		InitSongArrayFromDisk( ld );
		InitCoursesFromDisk();
		InitAutogenCourses();
		InitMachineScoresFromDisk();

	} catch(...) {
		SONGMAN = NULL;
		throw;
	}
}

SongManager::~SongManager()
{
	SaveMachineScoresToDisk();
	FreeSongArray();
}


void SongManager::InitSongArrayFromDisk( LoadingWindow *ld )
{
	LoadStepManiaSongDir( "Songs", ld );

	for( unsigned i=0; i<PREFSMAN->m_asAdditionalSongFolders.size(); i++ )
        LoadStepManiaSongDir( PREFSMAN->m_asAdditionalSongFolders[i], ld );

	if( PREFSMAN->m_DWIPath != "" )
		LoadStepManiaSongDir( PREFSMAN->m_DWIPath + "/Songs", ld );

	LOG->Trace( "Found %d Songs.", m_pSongs.size() );
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
	for(j = 0; j < m_arrayGroupNames.size(); ++j)
		if( sGroupDirName == m_arrayGroupNames[j] ) break;

	if( j != m_arrayGroupNames.size() )
		return; /* the group is already added */

	// Look for a group banner in this group folder
	CStringArray arrayGroupBanners;
	GetDirListing( sDir+sGroupDirName+"/*.png", arrayGroupBanners );
	GetDirListing( sDir+sGroupDirName+"/*.jpg", arrayGroupBanners );
	GetDirListing( sDir+sGroupDirName+"/*.gif", arrayGroupBanners );
	GetDirListing( sDir+sGroupDirName+"/*.bmp", arrayGroupBanners );
	CString sBannerPath;

	if( !arrayGroupBanners.empty() )
	{
		sBannerPath = sDir+sGroupDirName+"/"+arrayGroupBanners[0] ;
		LOG->Trace( "Group banner for '%s' is '%s'.", sGroupDirName.c_str(), sBannerPath.c_str() );
	}

	m_arrayGroupNames.push_back( sGroupDirName );
	m_GroupBannerPaths.push_back(sBannerPath);
}

void SongManager::LoadStepManiaSongDir( CString sDir, LoadingWindow *ld )
{
	/* Make sure sDir has a trailing slash. */
	TrimRight( sDir, "/\\" );
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
	}
	
	if( ld ) {
		ld->Paint();
		ld->SetText("Done loading songs.");
	}
}

void SongManager::FreeSongArray()
{
	for( unsigned i=0; i<m_pSongs.size(); i++ )
		SAFE_DELETE( m_pSongs[i] );
	m_pSongs.clear();

	m_GroupBannerPaths.clear();
}


void SongManager::ReloadSongArray()
{
	InitSongArrayFromDisk( NULL );
	FreeSongArray();
}



void SongManager::InitMachineScoresFromDisk()
{
	
	// Init category ranking
	{
		for( int i=0; i<NUM_NOTES_TYPES; i++ )
			for( int j=0; j<NUM_RANKING_CATEGORIES; j++ )
				for( int k=0; k<NUM_RANKING_LINES; k++ )
				{
					m_MachineScores[i][j][k].fScore = 573000;
					m_MachineScores[i][j][k].sName = DEFAULT_RANKING_NAME;
				}
	}

	// category ranking
	{
		FILE* fp = fopen( CATEGORY_RANKING_FILE, "r" );
		if( fp )
		{
			int version;
			fscanf(fp, "%d\n", &version );
			if( version == CATEGORY_RANKING_VERSION )
			{			
				for( int i=0; i<NUM_NOTES_TYPES; i++ )
					for( int j=0; j<NUM_RANKING_CATEGORIES; j++ )
						for( int k=0; k<NUM_RANKING_LINES; k++ )
							if( fp && !feof(fp) )
							{
								char szName[256];
								fscanf(fp, "%f %[^\n]\n", &m_MachineScores[i][j][k].fScore, szName);
								m_MachineScores[i][j][k].sName = szName;
							}
			}
			fclose(fp);
		}
	}

	// course ranking
	{
		FILE* fp = fopen( COURSE_RANKING_FILE, "r" );

		if( fp )
		{
			int version;
			fscanf(fp, "%i\n", &version );
			if( version == COURSE_RANKING_VERSION )
			{			
				while( fp && !feof(fp) )
				{
					char szPath[256];
					fscanf(fp, "%s\n", szPath);
					Course* pCourse = GetCourseFromPath( szPath );
					if( pCourse == NULL )
						pCourse = GetCourseFromName( szPath );
				
						for( int i=0; i<NUM_NOTES_TYPES; i++ )
						for( int j=0; j<NUM_RANKING_LINES; j++ )
							if( fp && !feof(fp) )
							{
								int iDancePoints;
								float fSurviveTime;
								char szName[256];
								fscanf(fp, "%d %f %[^\n]\n", &iDancePoints, &fSurviveTime, szName);
								if( pCourse )
								{
									pCourse->m_RankingScores[i][j].iDancePoints = iDancePoints;
									pCourse->m_RankingScores[i][j].fSurviveTime = fSurviveTime;
									pCourse->m_RankingScores[i][j].sName = szName;
								}
							}
				}
			}
			fclose(fp);
		}
	}

	// notes scores
	for( int c=0; c<NUM_MEMORY_CARDS; c++ )
	{
		ifstream f(NOTES_SCORES_FILE[c]);
		if( f.good() )
		{
			CString line;

			getline(f, line);
			int version;
			sscanf(line.c_str(), "%i", &version);

			if( version == NOTES_SCORES_VERSION )
			{			
				while( f && !f.eof() )
				{
					CString sSongDir;
					getline(f, sSongDir);

					getline(f, line);
					unsigned uNumNotes;
					sscanf(line.c_str(), "%u", &uNumNotes);

					Song* pSong = this->GetSongFromDir( sSongDir );

					for( unsigned i=0; i<uNumNotes; i++ )
					{
						NotesType nt;
						Difficulty dc;
	
						getline(f, line);
						sscanf(line.c_str(), "%d %d", &nt, &dc);

						CString sDescription;
						getline(f, sDescription);

						Notes* pNotes = NULL;
						if( pSong )
						{
							if( dc==DIFFICULTY_INVALID )
								pNotes = pSong->GetNotes( nt, sDescription );
							else
								pNotes = pSong->GetNotes( nt, dc );
						}
						
						getline(f, line);
						
						if( pNotes )
						{
							sscanf(line.c_str(), "%d %d %f\n",
								&pNotes->m_MemCardScores[c].iNumTimesPlayed,
								&pNotes->m_MemCardScores[c].grade,
								&pNotes->m_MemCardScores[c].fScore);
						}
					}
				}
			}
		}
	}

	// course scores
	{
		for( int c=0; c<NUM_MEMORY_CARDS; c++ )
		{
			FILE* fp = fopen( COURSE_SCORES_FILE[c], "r" );

			if( fp )
			{
				int version;
				fscanf(fp, "%d\n", &version );
				if( version == COURSE_SCORES_VERSION )
				{			
					while( fp && !feof(fp) )
					{
						char szPath[256];
						fscanf(fp, "%[^\n]\n", szPath);
						Course* pCourse = GetCourseFromPath( szPath );
						if( pCourse == NULL )
							pCourse = GetCourseFromName( szPath );
					
						for( int i=0; i<NUM_NOTES_TYPES; i++ )
							if( fp && !feof(fp) )
							{
								int iNumTimesPlayed;
								int iDancePoints;
								float fSurviveTime;
								fscanf(fp, "%d %d %f\n", &iNumTimesPlayed, &iDancePoints, &fSurviveTime);
								if( pCourse )
								{
									pCourse->m_MemCardScores[c][i].iNumTimesPlayed = iNumTimesPlayed;
									pCourse->m_MemCardScores[c][i].iDancePoints = iDancePoints;
									pCourse->m_MemCardScores[c][i].fSurviveTime = fSurviveTime;
								}
							}
					}
				}
				fclose(fp);
			}
		}
	}
}

void SongManager::SaveMachineScoresToDisk()
{
	// category ranking
	LOG->Trace("Writing category ranking");
	{
		FILE* fp = fopen( CATEGORY_RANKING_FILE, "w" );
		if( fp )
		{
			fprintf(fp,"%d\n",CATEGORY_RANKING_VERSION);
			for( int i=0; i<NUM_NOTES_TYPES; i++ )
				for( int j=0; j<NUM_RANKING_CATEGORIES; j++ )
					for( int k=0; k<NUM_RANKING_LINES; k++ )
						if( fp )
							fprintf(fp, "%f %s\n", m_MachineScores[i][j][k].fScore, m_MachineScores[i][j][k].sName.c_str());
			fclose(fp);
		}
	}

	// course ranking
	LOG->Trace("Writing course ranking");
	{
		FILE* fp = fopen( COURSE_RANKING_FILE, "w" );

		if( fp )
		{
			fprintf(fp,"%d\n",COURSE_RANKING_VERSION);
			for( unsigned c=0; c<m_pCourses.size(); c++ )	// foreach course
			{
				Course* pCourse = m_pCourses[c];
				ASSERT(pCourse);

				if( pCourse->m_bIsAutogen )
					fprintf(fp, "%s\n", pCourse->m_sName.c_str());
				else
					fprintf(fp, "%s\n", pCourse->m_sPath.c_str());
				
				for( int i=0; i<NUM_NOTES_TYPES; i++ )
					for( int j=0; j<NUM_RANKING_LINES; j++ )
					{
						fprintf(fp, "%d %f %s\n", 
							pCourse->m_RankingScores[i][j].iDancePoints, 
							pCourse->m_RankingScores[i][j].fSurviveTime, 
							pCourse->m_RankingScores[i][j].sName.c_str());
					}
			}

			fclose(fp);
		}
	}

	// notes scores
	LOG->Trace("Writing note scores");
	for( int c=0; c<NUM_MEMORY_CARDS; c++ )
	{
		FILE* fp = fopen( NOTES_SCORES_FILE[c], "w" );
		if( fp )
		{
			fprintf(fp,"%d\n",NOTES_SCORES_VERSION);

			for( unsigned s=0; s<m_pSongs.size(); s++ )	// foreach song
			{
				Song* pSong = m_pSongs[s];
				ASSERT(pSong);

				vector<Notes*> vNotes = pSong->m_apNotes;
/* This prevents the play count from being saved for songs that havn't
 * been passed.  Though, it seems we only increment this when it's
 * passed, anyway ...
				for( int n=(int)vNotes.size()-1; n>=0; n-- )
				{
					if( vNotes[n]->m_MemCardScores[c].grade <= GRADE_E )
						vNotes.erase( vNotes.begin()+n );
				}
*/
				if( vNotes.size() == 0 )
					continue;	// skip	

				fprintf(fp, "%s\n%u\n", 
					pSong->GetSongDir().c_str(),
					vNotes.size() );

				for( unsigned i=0; i<vNotes.size(); i++ )
				{
					Notes* pNotes = vNotes[i];
					ASSERT(pNotes);

					fprintf(fp, "%d %d\n%s\n", 
						pNotes->m_NotesType,
						pNotes->GetDifficulty(),
						pNotes->GetDescription().c_str() );

					fprintf(fp, "%d %d %f\n", 
						pNotes->m_MemCardScores[c].iNumTimesPlayed,
						pNotes->m_MemCardScores[c].grade,
						pNotes->m_MemCardScores[c].fScore);
				}
			}

			fclose(fp);
		}
	}

	// course scores
	LOG->Trace("Writing course scores");
	{
		for( int c=0; c<NUM_MEMORY_CARDS; c++ )
		{
			FILE* fp = fopen( COURSE_SCORES_FILE[c], "w" );
			if( fp )
			{
				fprintf(fp,"%d\n",COURSE_SCORES_VERSION);

				for( unsigned i=0; i<m_pCourses.size(); i++ )	// foreach song
				{
					Course* pCourse = m_pCourses[i];
					ASSERT(pCourse);

					if( pCourse->m_bIsAutogen )
						fprintf(fp, "%s\n", pCourse->m_sName.c_str());
					else
						fprintf(fp, "%s\n", pCourse->m_sPath.c_str());
					
					for( int nt=0; nt<NUM_NOTES_TYPES; nt++ )
						fprintf(fp, "%d %d %f\n", 
							pCourse->m_MemCardScores[c][nt].iNumTimesPlayed, 
							pCourse->m_MemCardScores[c][nt].iDancePoints, 
							pCourse->m_MemCardScores[c][nt].fSurviveTime);
				}

				fclose(fp);
			}
		}
	}
}


CString SongManager::GetGroupBannerPath( CString sGroupName )
{
	unsigned i;
	for(i = 0; i < m_arrayGroupNames.size(); ++i)
		if( sGroupName == m_arrayGroupNames[i] ) break;

	if( i == m_arrayGroupNames.size() )
		return "";

	return m_GroupBannerPaths[i];
}

void SongManager::GetGroupNames( CStringArray &AddTo )
{
	AddTo.insert(AddTo.end(), m_arrayGroupNames.begin(), m_arrayGroupNames.end() );
}

bool SongManager::DoesGroupExist( CString sGroupName )
{
	return find( m_arrayGroupNames.begin(), m_arrayGroupNames.end(), sGroupName ) != m_arrayGroupNames.end();
}

RageColor SongManager::GetGroupColor( const CString &sGroupName )
{
	// search for the group index
	unsigned i;
	for( i=0; i<m_arrayGroupNames.size(); i++ )
	{
		if( m_arrayGroupNames[i] == sGroupName )
			break;
	}
	ASSERT( i != m_arrayGroupNames.size() );	// this is not a valid group

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
//	const NotesType nt = GAMESTATE->GetCurrentStyleDef()->m_NotesType;
	for( unsigned i=0; i<pSong->m_apNotes.size(); i++ )
	{
		const Notes* pNotes = pSong->m_apNotes[i];

//		if(pNotes->m_NotesType != nt)
//			continue;

		if( pNotes->GetMeter() >= 10 )
			return EXTRA_COLOR;
	}

	return GetGroupColor( pSong->m_sGroupName );
}

RageColor SongManager::GetDifficultyColor( Difficulty dc )
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

void SongManager::GetSongs( vector<Song*> &AddTo, CString sGroupName, int iMaxStages ) const
{
	AddTo.clear();

	for( unsigned i=0; i<m_pSongs.size(); i++ )
		if( sGroupName=="" || sGroupName==m_pSongs[i]->m_sGroupName )
			if( GetNumStagesForSong(m_pSongs[i])<=iMaxStages )
				AddTo.push_back( m_pSongs[i] );
}

int SongManager::GetNumSongs() const
{
	return m_pSongs.size();
}

int SongManager::GetNumGroups() const
{
	return m_arrayGroupNames.size();
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

void SongManager::InitCoursesFromDisk()
{
	unsigned i;

	//
	// Load courses from in Courses dir
	//
	CStringArray saCourseFiles;
	GetDirListing( "Courses/*.crs", saCourseFiles, false, true );
	for( i=0; i<saCourseFiles.size(); i++ )
	{
		Course* pCourse = new Course;
		pCourse->LoadFromCRSFile( saCourseFiles[i] );
		m_pCourses.push_back( pCourse );
	}


	// Find all group directories in Courses dir
	CStringArray arrayGroupDirs;
	GetDirListing( "Courses/*", arrayGroupDirs, true );
	SortCStringArray( arrayGroupDirs );
	
	for( i=0; i< arrayGroupDirs.size(); i++ )	// for each dir in /Courses/
	{
		CString sGroupDirName = arrayGroupDirs[i];

		if( 0 == stricmp( sGroupDirName, "cvs" ) )	// the directory called "CVS"
			continue;		// ignore it

		// Find all CRS files in this group directory
		CStringArray arrayCoursePaths;
		GetDirListing( "Courses/"+sGroupDirName + "/*.crs", arrayCoursePaths, false, true );
		SortCStringArray( arrayCoursePaths );

		for( unsigned j=0; j<arrayCoursePaths.size(); j++ )
		{
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
void SongManager::CleanData()
{
	for( unsigned i=0; i<m_pSongs.size(); i++ )
	{
		for( unsigned n=0; n<m_pSongs[i]->m_apNotes.size(); n++ )
		{	
			m_pSongs[i]->m_apNotes[n]->Compress();
		}
	}
}

void SongManager::GetNonstopCourses( vector<Course*> &AddTo, bool bIncludeAutogen )
{
	for( unsigned i=0; i<m_pCourses.size(); i++ )
		if( !m_pCourses[i]->m_bRepeat && m_pCourses[i]->m_iLives <= 0 )	// use bar life meter
			if( bIncludeAutogen || !m_pCourses[i]->m_bIsAutogen )
				AddTo.push_back( m_pCourses[i] );
}

void SongManager::GetOniCourses( vector<Course*> &AddTo, bool bIncludeAutogen )
{
	for( unsigned i=0; i<m_pCourses.size(); i++ )
		if( !m_pCourses[i]->m_bRepeat && m_pCourses[i]->m_iLives > 0 )	// use battery life meter
			if( bIncludeAutogen || !m_pCourses[i]->m_bIsAutogen )
				AddTo.push_back( m_pCourses[i] );
}

void SongManager::GetEndlessCourses( vector<Course*> &AddTo, bool bIncludeAutogen )
{
	for( unsigned i=0; i<m_pCourses.size(); i++ )
		if( m_pCourses[i]->m_bRepeat )
			if( bIncludeAutogen || !m_pCourses[i]->m_bIsAutogen )
				AddTo.push_back( m_pCourses[i] );
}


bool SongManager::GetExtraStageInfoFromCourse( bool bExtra2, CString sPreferredGroup,
								   Song*& pSongOut, Notes*& pNotesOut, PlayerOptions& po_out, SongOptions& so_out )
{
	const CString sCourseSuffix = sPreferredGroup + "/" + (bExtra2 ? "extra2" : "extra1") + ".crs";
	CString sCoursePath = "Songs/" + sCourseSuffix;
	if( !DoesFileExist(sCoursePath) ) 
	{
		/* try alternative song folders */
		for( unsigned i=0; i<PREFSMAN->m_asAdditionalSongFolders.size(); i++ )
		{
			sCoursePath = PREFSMAN->m_asAdditionalSongFolders[i] + "/" + sCourseSuffix;
			if( DoesFileExist(sCoursePath) ) 
				break;
		}
	}

	if( !DoesFileExist(sCoursePath) && PREFSMAN->m_DWIPath.size() )
		sCoursePath = PREFSMAN->m_DWIPath + "/Songs/" + sCourseSuffix;

	/* Couldn't find course in DWI path or alternative song folders */
	if( !DoesFileExist(sCoursePath) )
		return false;

	Course course;
	course.LoadFromCRSFile( sCoursePath );
	if( course.GetEstimatedNumStages() <= 0 ) return false;

	CString sModifiers;
	if( !course.GetFirstStageInfo( pSongOut, pNotesOut, sModifiers, GAMESTATE->GetCurrentStyleDef()->m_NotesType ) )
		return false;

	po_out.Init();
	po_out.FromString( sModifiers );
	so_out.Init();
	so_out.FromString( sModifiers );
	return true;
}

/* Return true if n1 < n2. */
bool CompareNotesPointersForExtra(const Notes *n1, const Notes *n2)
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

void SongManager::GetExtraStageInfo( bool bExtra2, CString sPreferredGroup, const StyleDef *sd, 
								   Song*& pSongOut, Notes*& pNotesOut, PlayerOptions& po_out, SongOptions& so_out )
{
	CString Group = GAMESTATE->m_sPreferredGroup;
	if(Group == GROUP_ALL_MUSIC)
	{
		ASSERT(GAMESTATE->m_pCurSong);
		Group = GAMESTATE->m_pCurSong->m_sGroupName;
	}
	ASSERT(Group != "");

	if(GetExtraStageInfoFromCourse(bExtra2, Group, pSongOut, pNotesOut, po_out, so_out))
		return;
	
	// Choose a hard song for the extra stage
	Song*	pExtra1Song = NULL;		// the absolute hardest Song and Notes.  Use this for extra stage 1.
	Notes*	pExtra1Notes = NULL;
	Song*	pExtra2Song = NULL;		// a medium-hard Song and Notes.  Use this for extra stage 2.
	Notes*	pExtra2Notes = NULL;
	
	vector<Song*> apSongs;
	CString sGroup = GAMESTATE->m_sPreferredGroup==GROUP_ALL_MUSIC ? GAMESTATE->m_pCurSong->m_sGroupName : GAMESTATE->m_sPreferredGroup;
	SONGMAN->GetSongs( apSongs, sGroup );
	for( unsigned s=0; s<apSongs.size(); s++ )	// foreach song
	{
		Song* pSong = apSongs[s];

		vector<Notes*> apNotes;
		pSong->GetNotes( apNotes, sd->m_NotesType );
		for( unsigned n=0; n<apNotes.size(); n++ )	// foreach Notes
		{
			Notes* pNotes = apNotes[n];

			if( pExtra1Notes == NULL || CompareNotesPointersForExtra(pExtra1Notes,pNotes) )	// pNotes is harder than pHardestNotes
			{
				pExtra1Song = pSong;
				pExtra1Notes = pNotes;
			}

			// for extra 2, we don't want to choose the hardest notes possible.  So, we'll disgard Notes with meter > 8
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

	// If there are any notes at all that match this NotesType, everything should be filled out.
	// Also, it's guaranteed that there is at least one Notes that matches the NotesType because the player
	// had to play something before reaching the extra stage!
	ASSERT( pExtra2Song && pExtra1Song && pExtra2Notes && pExtra1Notes );

	pSongOut = (bExtra2 ? pExtra2Song : pExtra1Song);
	pNotesOut = (bExtra2 ? pExtra2Notes : pExtra1Notes);


	po_out.Init();
	so_out.Init();
	po_out.m_fReverseScroll = 1;
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
	if( sDir[sDir.GetLength()-1] != '/' )
		sDir += '/';

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


bool SongManager::IsUsingMemoryCard( PlayerNumber pn )
{
	return true;
}


struct CategoryScoreToInsert
{
	PlayerNumber pn;
	RankingCategory cat;
	float fScore;

	static int CompareDescending( const CategoryScoreToInsert &hs1, const CategoryScoreToInsert &hs2 )
	{
		if( hs1.fScore > hs2.fScore )		return -1;
		else if( hs1.fScore == hs2.fScore )	return 0;
		else								return +1;
	}
	static void SortDescending( vector<CategoryScoreToInsert>& vHSout )
	{ 
		sort( vHSout.begin(), vHSout.end(), CompareDescending ); 
	}
};

void SongManager::AddScores( NotesType nt, bool bPlayerEnabled[NUM_PLAYERS], RankingCategory hsc[NUM_PLAYERS], float fScore[NUM_PLAYERS], int iNewRecordIndexOut[NUM_PLAYERS] )	// set iNewRecordIndex = -1 if not a new record
{
	vector<CategoryScoreToInsert> vHS;
	for( int p=0; p<NUM_PLAYERS; p++ )
	{
		iNewRecordIndexOut[p] = -1;

		if( !bPlayerEnabled[p] )
			continue;	// skip

		CategoryScoreToInsert hs;
		hs.pn = (PlayerNumber)p;
		hs.cat = hsc[p];
		hs.fScore = fScore[p];
		vHS.push_back( hs );
	}

	// Sort descending before inserting.
	// This guarantees that a high score will not switch poitions on us when we later insert scores for the other player
	CategoryScoreToInsert::SortDescending( vHS );

	for( unsigned i=0; i<vHS.size(); i++ )
	{
		CategoryScoreToInsert& newHS = vHS[i];
		MachineScore* machineScores = m_MachineScores[nt][newHS.cat];
		for( int i=0; i<NUM_RANKING_LINES; i++ )
		{
			if( newHS.fScore > machineScores[i].fScore )
			{
				// We found the insert point.  Shift down.
				for( int j=NUM_RANKING_LINES-1; j>i; j-- )
					machineScores[j] = machineScores[j-1];
				// insert
				machineScores[i].fScore = newHS.fScore;
				machineScores[i].sName = DEFAULT_RANKING_NAME;
				iNewRecordIndexOut[newHS.pn] = i;
				break;
			}
		}
	}
}


