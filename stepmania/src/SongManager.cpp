#include "stdafx.h"
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

SongManager*	SONGMAN = NULL;	// global and accessable from anywhere in our program


const CString CATEGORY_TOP_SCORE_FILE = "CategoryTopScores.dat";
const CString COURSE_TOP_SCORE_FILE = "CourseTopScores.dat";
const int CATEGORY_TOP_SCORE_VERSION = 1;
const int COURSE_TOP_SCORE_VERSION = 1;


#define NUM_GROUP_COLORS	THEME->GetMetricI("SongManager","NumGroupColors")
#define GROUP_COLOR( i )	THEME->GetMetricC("SongManager",ssprintf("GroupColor%d",i+1))
#define EXTRA_COLOR			THEME->GetMetricC("SongManager","ExtraColor")

vector<RageColor> g_vGroupColors;
RageColor g_ExtraColor;


SongManager::SongManager( LoadingWindow *ld )
{
	/* We initialize things that assume they can get at SONGMAN; we only
	 * init one of these, so hook us up to it immediately. */
	SONGMAN = this;

	g_vGroupColors.clear();
	for( int i=0; i<NUM_GROUP_COLORS; i++ )
		g_vGroupColors.push_back( GROUP_COLOR(i) );
	g_ExtraColor = EXTRA_COLOR;

	InitSongArrayFromDisk( ld );
	InitMachineScoresFromDisk();

	InitCoursesFromDisk();
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
		LoadStepManiaSongDir( PREFSMAN->m_DWIPath + "\\Songs", ld );

	LOG->Trace( "Found %d Songs.", m_pSongs.size() );
}

void SongManager::SanityCheckGroupDir( CString sDir ) const
{
	// Check to see if they put a song directly inside the group folder.
	CStringArray arrayFiles;
	GetDirListing( sDir + "\\*.mp3", arrayFiles );
	GetDirListing( sDir + "\\*.ogg", arrayFiles );
	GetDirListing( sDir + "\\*.wav", arrayFiles );
	if( !arrayFiles.empty() )
		RageException::Throw( 
			"The folder '%s' contains music files.\n\n"
			"This means that you have a music outside of a song folder.\n"
			"All song folders must reside in a group folder.  For example, 'Songs\\DDR 4th Mix\\B4U'.\n"
			"See the StepMania readme for more info.",
			sDir.GetString()
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
		LOG->Trace( "Group banner for '%s' is '%s'.", sGroupDirName.GetString(), sBannerPath.GetString() );
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
					Basename(sGroupDirName).GetString(),
					Basename(sSongDirName).GetString()));
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
	
	// Init category top scores
	{
		for( int i=0; i<NUM_NOTES_TYPES; i++ )
			for( int j=0; j<NUM_RANKING_CATEGORIES; j++ )
				for( int k=0; k<NUM_RANKING_LINES; k++ )
				{
					m_MachineScores[i][j][k].fScore = 573000;
					m_MachineScores[i][j][k].sName = "STEP";
				}
	}

	// Read category top scores
	{
		FILE* fp = fopen( CATEGORY_TOP_SCORE_FILE, "r" );
		if( fp )
		{
			int version;
			fscanf(fp, "%d\n", &version );
			if( version == CATEGORY_TOP_SCORE_VERSION )
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

	// Read course top scores
	{
		FILE* fp = fopen( COURSE_TOP_SCORE_FILE, "r" );

		if( fp )
		{
			int version;
			fscanf(fp, "%d\n", &version );
			if( version == COURSE_TOP_SCORE_VERSION )
			{			
				while( fp && !feof(fp) )
				{
						char szPath[256];
						fscanf(fp, "%s\n", szPath);
						Course* pCourse = GetCourseFromPath( szPath );
						
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
										pCourse->m_MachineScores[i][j].iDancePoints = iDancePoints;
										pCourse->m_MachineScores[i][j].fSurviveTime = fSurviveTime;
										pCourse->m_MachineScores[i][j].sName = szName;
									}
								}
				}
			}
			fclose(fp);
		}
	}
}

void SongManager::SaveMachineScoresToDisk()
{
	// Write category top scores
	{
		FILE* fp = fopen( CATEGORY_TOP_SCORE_FILE, "w" );
		if( fp )
		{
			fprintf(fp,"%d\n",CATEGORY_TOP_SCORE_VERSION);
			for( int i=0; i<NUM_NOTES_TYPES; i++ )
				for( int j=0; j<NUM_RANKING_CATEGORIES; j++ )
					for( int k=0; k<NUM_RANKING_LINES; k++ )
						if( fp )
							fprintf(fp, "%f %s\n", m_MachineScores[i][j][k].fScore, m_MachineScores[i][j][k].sName.c_str());
			fclose(fp);
		}
	}

	// Write course top scores
	{
		FILE* fp = fopen( COURSE_TOP_SCORE_FILE, "w" );

		if( fp )
		{
			fprintf(fp,"%d",COURSE_TOP_SCORE_VERSION);
			for( unsigned i=0; i<m_pCourses.size(); i++ )	// foreach course
			{
				Course* pCourse = m_pCourses[i];

				fprintf(fp, "%s\n", pCourse->m_sPath.c_str());
				
				for( int i=0; i<NUM_NOTES_TYPES; i++ )
					for( int j=0; j<NUM_RANKING_LINES; j++ )
						fprintf(fp, "%d %f %s\n", 
							pCourse->m_MachineScores[i][j].iDancePoints, 
							pCourse->m_MachineScores[i][j].fSurviveTime, 
							pCourse->m_MachineScores[i][j].sName.c_str());
			}
		}

		if( fp )
			fclose(fp);
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
	for( unsigned i=0; i<pSong->m_apNotes.size(); i++ )
	{
		const Notes* pNotes = pSong->m_apNotes[i];
		if( pNotes->GetMeter() >= 10 )
			return EXTRA_COLOR;
	}

	return GetGroupColor( pSong->m_sGroupName );
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
	//
	// Load courses from CRS files
	//
	CStringArray saCourseFiles;
	GetDirListing( "Courses/*.crs", saCourseFiles, false, true );
	for( unsigned i=0; i<saCourseFiles.size(); i++ )
	{
		Course* pCourse = new Course;
		pCourse->LoadFromCRSFile( saCourseFiles[i] );
		if( pCourse->GetNumStages() > 0 )
			m_pCourses.push_back( pCourse );
		else
			delete pCourse;
	}
	
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

		for( Difficulty dc=DIFFICULTY_EASY; dc<=DIFFICULTY_HARD; dc=Difficulty(dc+1) )	// foreach Difficulty
		{
			Course* pCourse = new Course;
			pCourse->CreateEndlessCourseFromGroupAndDifficulty( sGroupName, dc, apGroupSongs );

			if( pCourse->GetNumStages() > 0 )
				m_pCourses.push_back( pCourse );
			else
				delete pCourse;
		}
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

void SongManager::GetNonstopCourses( vector<Course*> &AddTo )
{
	PlayerOptions po;
	for( unsigned i=0; i<m_pCourses.size(); i++ )
	{
		if( m_pCourses[i]->m_bRepeat )
			continue;	// skip
		if( m_pCourses[i]->m_iLives > 0 )	// use battery life meter
			continue;
		
		AddTo.push_back( m_pCourses[i] );
	}
}

void SongManager::GetOniCourses( vector<Course*> &AddTo )
{
	PlayerOptions po;
	for( unsigned i=0; i<m_pCourses.size(); i++ )
	{
		if( m_pCourses[i]->m_bRepeat )
			continue;	// skip
		if( m_pCourses[i]->m_iLives <= 0 )	// use bar life meter
			continue;
		
		AddTo.push_back( m_pCourses[i] );
	}
}

void SongManager::GetEndlessCourses( vector<Course*> &AddTo )
{
	for( unsigned i=0; i<m_pCourses.size(); i++ )
	{
		if( m_pCourses[i]->m_bRepeat )
			AddTo.push_back( m_pCourses[i] );
	}	
}


bool SongManager::GetExtraStageInfoFromCourse( bool bExtra2, CString sPreferredGroup,
								   Song*& pSongOut, Notes*& pNotesOut, PlayerOptions& po_out, SongOptions& so_out )
{
	CString sCoursePath = "Songs\\" + sPreferredGroup + "\\" + (bExtra2 ? "extra2" : "extra1") + ".crs";
	if( !DoesFileExist(sCoursePath) ) 
	{
		bool bFound = false;

		/* try alternative song folders */
		for( unsigned i=0; i<PREFSMAN->m_asAdditionalSongFolders.size(); i++ )
		{
			sCoursePath = PREFSMAN->m_asAdditionalSongFolders[i] + "\\" + sPreferredGroup + "\\" + (bExtra2 ? "extra2" : "extra1") + ".crs";
			if( DoesFileExist(sCoursePath) ) 
			{
				bFound = true;
				break;
			}
		}

		if( !bFound && PREFSMAN->m_DWIPath != "" )
		{
			sCoursePath = PREFSMAN->m_DWIPath + "\\Songs\\" + sPreferredGroup + "\\" + (bExtra2 ? "extra2" : "extra1") + ".crs";
			if( DoesFileExist(sCoursePath) )
				bFound = true;
		}

		/*Couldn't find course in DWI path or Alternative Song Folders*/
		if( !bFound )
			return false;
	}	

	Course course;
	course.LoadFromCRSFile( sCoursePath );
	if( course.GetNumStages() <= 0 ) return false;

	pSongOut = course.GetSong(0);
	pNotesOut = course.GetNotesForStage( 0 );
	if( pNotesOut == NULL ) return false;
	
	course.GetPlayerOptions( 0, &po_out );
	course.GetSongOptions( &so_out );

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
	if(GetExtraStageInfoFromCourse(bExtra2, (GAMESTATE->m_sPreferredGroup == "ALL MUSIC" ? GAMESTATE->m_pCurSong->m_sGroupName : GAMESTATE->m_sPreferredGroup), pSongOut, pNotesOut, po_out, so_out))
		return;
	
	// Choose a hard song for the extra stage
	Song*	pExtra1Song = NULL;		// the absolute hardest Song and Notes.  Use this for extra stage 1.
	Notes*	pExtra1Notes = NULL;
	Song*	pExtra2Song = NULL;		// a medium-hard Song and Notes.  Use this for extra stage 2.
	Notes*	pExtra2Notes = NULL;
	
	vector<Song*> apSongs;
	CString sGroup = GAMESTATE->m_sPreferredGroup=="ALL MUSIC" ? GAMESTATE->m_pCurSong->m_sGroupName : GAMESTATE->m_sPreferredGroup;
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
	po_out.m_bReverseScroll = true;
	po_out.m_fScrollSpeed = 1.5f;
	so_out.m_DrainType = (bExtra2 ? SongOptions::DRAIN_SUDDEN_DEATH : SongOptions::DRAIN_NO_RECOVER);

	// should we do something fancy here, like turn on different effects?
	int iSongHash = GetHashForString( pSongOut->GetSongDir() );
	switch( ((unsigned int)iSongHash) % 5 )
	{
	case 0:	po_out.m_bEffects[PlayerOptions::EFFECT_DIZZY] = true;	break;
	case 1:	po_out.m_bDark = true;									break;
	case 2:	po_out.m_bEffects[PlayerOptions::EFFECT_DRUNK] = true;	break;
	case 3:	po_out.m_bEffects[PlayerOptions::EFFECT_MINI] = true;	break;
	case 4:	po_out.m_bEffects[PlayerOptions::EFFECT_SPACE] = true;	break;
	default:	ASSERT(0);
	}
}

Song* SongManager::GetRandomSong()
{
	if( m_pSongs.empty() )
		return NULL;

	return SONGMAN->m_pSongs[ rand()%m_pSongs.size() ];
}

Song* SongManager::GetSongFromDir( CString sDir )
{
	if( sDir[sDir.GetLength()-1] != '\\' )
		sDir += '\\';

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

void SongManager::AddMachineRecords( NotesType nt, RankingCategory hsc[NUM_PLAYERS], float fScore[NUM_PLAYERS], int iNewRecordIndexOut[NUM_PLAYERS] )	// set iNewRecordIndex = -1 if not a new record
{
	vector<CategoryScoreToInsert> vHS;
	for( int p=0; p<NUM_PLAYERS; p++ )
	{
		iNewRecordIndexOut[p] = -1;

		if( !GAMESTATE->IsPlayerEnabled(p) )
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
				machineScores[i].sName = "STEP";
				iNewRecordIndexOut[newHS.pn] = i;
				break;
			}
		}
	}
}


