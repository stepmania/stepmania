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


const CString g_sStatisticsFileName = "statistics.ini";

#define NUM_GROUP_COLORS	THEME->GetMetricI("SongManager","NumGroupColors")
#define GROUP_COLOR( i )	THEME->GetMetricC("SongManager",ssprintf("GroupColor%d",i+1))
#define EXTRA_COLOR			THEME->GetMetricC("SongManager","ExtraColor")

RageColor g_GroupColors[30];
RageColor g_ExtraColor;


SongManager::SongManager( LoadingWindow *ld )
{
	for( int i=0; i<NUM_GROUP_COLORS; i++ )
		g_GroupColors[i] = GROUP_COLOR( i );
	g_ExtraColor = EXTRA_COLOR;

	InitSongArrayFromDisk( ld );
	ReadStatisticsFromDisk();

	InitCoursesFromDisk();
}

SongManager::~SongManager()
{
	SaveStatisticsToDisk();
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
	GetDirListing( ssprintf("%s\\%s\\*.png", sDir.GetString(), sGroupDirName.GetString()), arrayGroupBanners );
	GetDirListing( ssprintf("%s\\%s\\*.jpg", sDir.GetString(), sGroupDirName.GetString()), arrayGroupBanners );
	GetDirListing( ssprintf("%s\\%s\\*.gif", sDir.GetString(), sGroupDirName.GetString()), arrayGroupBanners );
	GetDirListing( ssprintf("%s\\%s\\*.bmp", sDir.GetString(), sGroupDirName.GetString()), arrayGroupBanners );
	CString sBannerPath;

	if( !arrayGroupBanners.empty() )
	{
		sBannerPath = ssprintf("%s\\%s\\%s", sDir.GetString(), sGroupDirName.GetString(), arrayGroupBanners[0].GetString() );
		LOG->Trace( "Group banner for '%s' is '%s'.", sGroupDirName.GetString(), sBannerPath.GetString() );
	}

	m_arrayGroupNames.push_back( sGroupDirName );
	m_GroupBannerPaths.push_back(sBannerPath);
}

void SongManager::LoadStepManiaSongDir( CString sDir, LoadingWindow *ld )
{
	// trim off the trailing slash if any
	TrimRight( sDir, "/\\" );

	// Find all group directories in "Songs" folder
	CStringArray arrayGroupDirs;
	GetDirListing( sDir+"\\*.*", arrayGroupDirs, true );
	SortCStringArray( arrayGroupDirs );
	
	for( unsigned i=0; i< arrayGroupDirs.size(); i++ )	// for each dir in /Songs/
	{
		CString sGroupDirName = arrayGroupDirs[i];

		if( 0 == stricmp( sGroupDirName, "cvs" ) )	// the directory called "CVS"
			continue;		// ignore it

		SanityCheckGroupDir(sDir+"\\"+sGroupDirName);

		// Find all Song folders in this group directory
		CStringArray arraySongDirs;
		GetDirListing( ssprintf("%s\\%s\\*.*", sDir.GetString(), sGroupDirName.GetString()), arraySongDirs, true );
		SortCStringArray( arraySongDirs );

		unsigned j;
		int loaded = 0;

		for( j=0; j< arraySongDirs.size(); j++ )	// for each song dir
		{
			CString sSongDirName = arraySongDirs[j];

			if( 0 == stricmp( sSongDirName, "cvs" ) )	// the directory called "CVS"
				continue;		// ignore it

			// this is a song directory.  Load a new song!
			if( ld ) {
				ld->SetText( ssprintf("Loading songs...\n%s\n%s", sGroupDirName.GetString(), sSongDirName.GetString()));
				ld->Paint();
			}
			Song* pNewSong = new Song;
			if( !pNewSong->LoadFromSongDir( ssprintf("%s\\%s\\%s", sDir.GetString(), sGroupDirName.GetString(), sSongDirName.GetString()) ) ) {
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

/*

void SongManager::LoadDWISongDir( CString DWIHome )
{
	// trim off the trailing slash if any
	TrimRight( DWIHome, "/\\" );

	// this has to be fixed, DWI doesn't put files
	// in it's DWI folder.  It puts them in Songs/<MIX>/<SONGNAME>
	// so what we have to do, is go to the DWI directory (which will
	// be changeable by the user so they don't have to copy everything
	// over and have two copies of everything
	// Find all directories in "DWIs" folder
	CStringArray arrayDirs;
	CStringArray MixDirs;
	// now we've got the listing of the mix directories
	// and we need to use THOSE directories to find our
	// dwis
	GetDirListing( ssprintf("%s\\Songs\\*.*", DWIHome ), MixDirs.GetString(), true );
	SortCStringArray( MixDirs );
	
	for( unsigned i=0; i< MixDirs.size(); i++ )	// for each dir in /Songs/
	{
		// the dir name will most likely be something like
		// Dance Dance Revolution 4th Mix, etc.
		CString sDirName = MixDirs[i];
		sDirName.MakeLower();
		if( sDirName == "cvs" )	// ignore the directory called "CVS"
			continue;
		GetDirListing( ssprintf("%s\\Songs\\%s\\*.*", DWIHome.GetString(), MixDirs[i].GetString()), arrayDirs,  true);
		SortCStringArray(arrayDirs, true);

		for( unsigned b = 0; b < arrayDirs.size(); b++)
		{
			// Find all DWIs in this directory
			CStringArray arrayDWIFiles;
			GetDirListing( ssprintf("%s\\Songs\\%s\\%s\\*.dwi", DWIHome.GetString(), MixDirs[i].GetString(), arrayDirs[b].GetString()), arrayDWIFiles, false);
			SortCStringArray( arrayDWIFiles );

			for( unsigned j=0; j< arrayDWIFiles.size(); j++ )	// for each DWI file
			{
				CString sDWIFileName = arrayDWIFiles[j];
				sDWIFileName.MakeLower();

				// load DWIs from the sub dirs
				DWILoader ld;
				Song* pNewSong = new Song;
				ld.LoadFromDWIFile(
					ssprintf("%s\\Songs\\%s\\%s\\%s", DWIHome.GetString(), MixDirs[i].GetString(), arrayDirs[b].GetString(), sDWIFileName.GetString()),
					*pNewSong);
				m_pSongs.push_back( pNewSong );
			}
		}
	}
}
*/


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



void SongManager::ReadStatisticsFromDisk()
{
	IniFile ini;
	ini.SetPath( g_sStatisticsFileName );
	if( !ini.ReadFile() ) {
		LOG->Trace( "WARNING: Could not read config file '%s'.", g_sStatisticsFileName.GetString() );
		return;		// load nothing
	}


	// load song statistics
	const IniFile::key *pSongsKey = ini.GetKey( "Statistics" );
	if( pSongsKey )
	{
		for( IniFile::key::const_iterator i = pSongsKey->begin(); i != pSongsKey->end(); ++i )
		{
			CString name = i->first;
			CString value = i->second;

			// Each value has the format "SongName::StepsName=TimesPlayed::TopGrade::TopScore::MaxCombo".
			char szSongDir[256];
			char szNotesType[256];
			char szNotesDescription[256];
			int iRetVal;
			unsigned i;

			// Parse for Song name and Notes name
			iRetVal = sscanf( name.c_str(), "%[^:]::%[^:]::%[^\n]", szSongDir, szNotesType, szNotesDescription );
			if( iRetVal != 3 )
				continue;	// this line doesn't match what is expected
	
			NotesType notesType = GameManager::StringToNotesType( szNotesType );
			
			// Search for the corresponding Song pointer.
			Song* pSong = NULL;
			for( i=0; i<m_pSongs.size(); i++ )
			{
				if( m_pSongs[i]->GetSongDir() == szSongDir )	// match!
				{
					pSong = m_pSongs[i];
					break;
				}
			}
			if( pSong == NULL )	// didn't find a match
				continue;	// skip this entry


			// Search for the corresponding Notes pointer.
			Notes* pNotes = NULL;
			for( i=0; i<pSong->m_apNotes.size(); i++ )
			{
				if( pSong->m_apNotes[i]->m_NotesType == notesType  &&
					pSong->m_apNotes[i]->GetDescription() == szNotesDescription )	// match!
				{
					pNotes = pSong->m_apNotes[i];
					break;
				}
			}
			if( pNotes == NULL )	// didn't find a match
				continue;	// skip this entry


			// Parse the Notes statistics.
			char szGradeLetters[10];	// longest possible string is "AAA"

			iRetVal = sscanf( 
				value.c_str(), 
				"%d::%[^:]::%d::%d", 
				&pNotes->m_iNumTimesPlayed,
				szGradeLetters,
				&pNotes->m_iTopScore,
				&pNotes->m_iMaxCombo
			);
			if( iRetVal != 4 )
				continue;

			pNotes->m_TopGrade = StringToGrade( szGradeLetters );
		}
	}

	// load course statistics
	const IniFile::key *pCoursesKey = ini.GetKey( "Courses" );
	if( pCoursesKey )
	{
		for( IniFile::key::const_iterator i = pCoursesKey->begin(); i != pCoursesKey->end(); ++i )
		{
			CString name = i->first;
			CString value = i->second;

			// Each value has the format "CoursePath,Difficulty,Slot=DancePoints,SurviveTime,sName"

			// Parse left hand side
			char szCoursePath[256];
			Difficulty difficulty;
			int iSlot;
			int iDancePoints;
			float fSurviveTime;
			char szName[256];

			int iRetVal;
			iRetVal = sscanf( name.c_str(), "%[^,],%d,%d", szCoursePath, &difficulty, &iSlot );
			if( iRetVal != 3 )
				continue;	// line doesn't match what is expected
			iRetVal = sscanf( value.c_str(), "%d,%f,%d[^\n]", &iDancePoints, &fSurviveTime, &szName );
			if( iRetVal != 3 )
				continue;	// line doesn't match what is expected

			
			// Search for the corresponding Course pointer.
			Course* pCourse = SONGMAN->GetCourseFromPath( szCoursePath );
			if( pCourse == NULL )	// didn't find a match
				continue;	// skip this entry

//			pCourse->m_TopGrade = StringToGrade( szGradeLetters );
		}
	}


}

void SongManager::SaveStatisticsToDisk()
{
	IniFile ini;
	ini.SetPath( g_sStatisticsFileName );

	// save song statistics
	for( unsigned i=0; i<m_pSongs.size(); i++ )		// for each Song
	{
		Song* pSong = m_pSongs[i];

		for( unsigned j=0; j<pSong->m_apNotes.size(); j++ )		// for each Notes
		{
			Notes* pNotes = pSong->m_apNotes[j];

			if( pNotes->m_TopGrade == GRADE_NO_DATA )
				continue;		// skip

			// Each value has the format "SongName::NotesName=TimesPlayed::TopGrade::TopScore::MaxCombo".

			CString sName = ssprintf( "%s::%s::%s", pSong->GetSongDir().GetString(), GameManager::NotesTypeToString(pNotes->m_NotesType).GetString(), pNotes->GetDescription().GetString() );
			CString sValue = ssprintf( 
				"%d::%s::%d::%d",
				pNotes->m_iNumTimesPlayed,
				GradeToString( pNotes->m_TopGrade ).GetString(),
				pNotes->m_iTopScore, 
				pNotes->m_iMaxCombo
			);

			ini.SetValue( "Statistics", sName, sValue );
		}
	}

	ini.WriteFile();
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

	return g_GroupColors[i%NUM_GROUP_COLORS];
}

RageColor SongManager::GetSongColor( const Song* pSong )
{
	ASSERT( pSong );
	for( unsigned i=0; i<pSong->m_apNotes.size(); i++ )
	{
		const Notes* pNotes = pSong->m_apNotes[i];
		if( pNotes->GetMeter() == 10 )
			return EXTRA_COLOR;
	}

	return GetGroupColor( pSong->m_sGroupName );
}


void SongManager::GetSongsInGroup( const CString sGroupName, vector<Song*> &AddTo )
{
	for( unsigned i=0; i<m_pSongs.size(); i++ )
	{
		Song* pSong = m_pSongs[i];
		if( sGroupName == m_pSongs[i]->m_sGroupName )
			AddTo.push_back( pSong );
	}
}

CString SongManager::ShortenGroupName( const CString &sOrigGroupName )
{
	CString sShortName = sOrigGroupName;
	sShortName.Replace( "Dance Dance Revolution", "DDR" );
	sShortName.Replace( "dance dance revolution", "DDR" );
	sShortName.Replace( "DANCE DANCE REVOLUTION", "DDR" );
	sShortName.Replace( "Pump It Up", "PIU" );
	sShortName.Replace( "pump it up", "PIU" );
	sShortName.Replace( "PUMP IT UP", "PIU" );
	sShortName.Replace( "ParaParaParadise", "PPP" );
	sShortName.Replace( "paraparaparadise", "PPP" );
	sShortName.Replace( "PARAPARAPARADUSE", "PPP" );
	sShortName.Replace( "Para Para Paradise", "PPP" );
	sShortName.Replace( "para para paradise", "PPP" );
	sShortName.Replace( "PARA PARA PARADUSE", "PPP" );
	sShortName.Replace( "Dancing Stage", "DS" );
	sShortName.Replace( "Dancing Stage", "DS" );
	sShortName.Replace( "Dancing Stage", "DS" );
	return sShortName;
}

void SongManager::InitCoursesFromDisk()
{
	//
	// Load courses from CRS files
	//
	CStringArray saCourseFiles;
	GetDirListing( "Courses\\*.crs", saCourseFiles );
	for( unsigned i=0; i<saCourseFiles.size(); i++ )
	{
		Course course;
		course.LoadFromCRSFile( "Courses\\" + saCourseFiles[i], m_pSongs );
		if( course.GetNumStages() > 0 )
			m_Courses.push_back( course );
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
		GetSongsInGroup( sGroupName, apGroupSongs );

		for( Difficulty dc=DIFFICULTY_EASY; dc<=DIFFICULTY_HARD; dc=Difficulty(dc+1) )	// foreach Difficulty
		{
			Course course;
			course.CreateEndlessCourseFromGroupAndDifficulty( sGroupName, dc, apGroupSongs );

			if( course.GetNumStages() > 0 )
				m_Courses.push_back( course );
		}
	}
}

void SongManager::ReloadCourses()
{

}

/* Called periodically to wipe out cached NoteData.  This is called when we change
 * screens. */
void SongManager::CleanCourses()
{
	for( unsigned i=0; i<m_pSongs.size(); i++ )
	{
		for( unsigned n=0; n<m_pSongs[i]->m_apNotes.size(); n++ )
		{	
			m_pSongs[i]->m_apNotes[n]->Compress();
		}
	}
}

void SongManager::GetNonstopCourses( vector<Course*> AddTo )
{
	PlayerOptions po;
	for( unsigned i=0; i<m_Courses.size(); i++ )
	{
		if( m_Courses[i].m_bRepeat )
			continue;	// skip
		if( m_Courses[i].m_iLives > 0 )	// use battery life meter
			continue;
		
		AddTo.push_back( &m_Courses[i] );
	}
}

void SongManager::GetOniCourses( vector<Course*> AddTo )
{
	PlayerOptions po;
	for( unsigned i=0; i<m_Courses.size(); i++ )
	{
		if( m_Courses[i].m_bRepeat )
			continue;	// skip
		if( m_Courses[i].m_iLives <= 0 )	// use bar life meter
			continue;
		
		AddTo.push_back( &m_Courses[i] );
	}
}

void SongManager::GetEndlessCourses( vector<Course*> AddTo )
{
	for( unsigned i=0; i<m_Courses.size(); i++ )
	{
		if( m_Courses[i].m_bRepeat )
			AddTo.push_back( &m_Courses[i] );
	}	
}


bool SongManager::GetExtraStageInfoFromCourse( bool bExtra2, CString sPreferredGroup,
								   Song*& pSongOut, Notes*& pNotesOut, PlayerOptions& po_out, SongOptions& so_out )
{
	CString sCoursePath = "Songs\\" + sPreferredGroup + "\\" + (bExtra2 ? "extra2" : "extra1") + ".crs";
	if( !DoesFileExist(sCoursePath) ) return false;

	Course course;
	course.LoadFromCRSFile( sCoursePath, m_pSongs );
	if( course.GetNumStages() <= 0 ) return false;

	pSongOut = course.GetSong(0);
	pNotesOut = course.GetNotesForStage( 0 );
	if( pNotesOut == NULL ) return false;
	
	course.GetPlayerOptions( 0, &po_out );
	course.GetSongOptions( &so_out );

	return true;
}

void SongManager::GetExtraStageInfo( bool bExtra2, CString sPreferredGroup, const StyleDef *sd, 
								   Song*& pSongOut, Notes*& pNotesOut, PlayerOptions& po_out, SongOptions& so_out )
{
	if(GetExtraStageInfoFromCourse(bExtra2, sPreferredGroup,
			pSongOut, pNotesOut, po_out, so_out))
		return;
	
	// Choose a hard song for the extra stage
	Song*	pExtra1Song = NULL;		// the absolute hardest Song and Notes.  Use this for extra stage 1.
	Notes*	pExtra1Notes = NULL;
	Song*	pExtra2Song = NULL;		// a medium-hard Song and Notes.  Use this for extra stage 2.
	Notes*	pExtra2Notes = NULL;
	
	vector<Song*> apSongs;
	SONGMAN->GetSongsInGroup( GAMESTATE->m_sPreferredGroup, apSongs );
	for( unsigned s=0; s<apSongs.size(); s++ )	// foreach song
	{
		Song* pSong = apSongs[s];

		vector<Notes*> apNotes;
		pSong->GetNotesThatMatch( sd->m_NotesType, apNotes );
		for( unsigned n=0; n<apNotes.size(); n++ )	// foreach Notes
		{
			Notes* pNotes = apNotes[n];

			if( pExtra1Notes == NULL  ||  CompareNotesPointersByDifficulty(pExtra1Notes,pNotes) )	// pNotes is harder than pHardestNotes
			{
				pExtra1Song = pSong;
				pExtra1Notes = pNotes;
			}

			// for extra 2, we don't want to choose the hardest notes possible.  So, we'll disgard Notes with meter > 8
			if(	bExtra2  &&  pNotes->GetMeter() > 8 )	
				continue;	// skip
			if( pExtra2Notes == NULL  ||  CompareNotesPointersByDifficulty(pExtra2Notes,pNotes) )	// pNotes is harder than pHardestNotes
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
	po_out.m_fArrowScrollSpeed = 1.5f;
	so_out.m_DrainType = (bExtra2 ? SongOptions::DRAIN_SUDDEN_DEATH : SongOptions::DRAIN_NO_RECOVER);

	// should we do something fancy here, like turn on different effects?
	int iSongHash = GetHashForString( pSongOut->GetSongDir() );
	switch( ((UINT)iSongHash) % 6 )
	{
	case 0:	po_out.m_bEffects[PlayerOptions::EFFECT_DIZZY] = true;	break;
	case 1:	po_out.m_bDark = true;									break;
	case 2:	po_out.m_bEffects[PlayerOptions::EFFECT_DRUNK] = true;	break;
	case 3:	po_out.m_bEffects[PlayerOptions::EFFECT_MINI] = true;	break;
	case 4:	po_out.m_bEffects[PlayerOptions::EFFECT_SPACE] = true;	break;
	case 5:	po_out.m_bEffects[PlayerOptions::EFFECT_WAVE] = true;	break;
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
	for( unsigned int i=0; i<m_Courses.size(); i++ )
		if( sPath.CompareNoCase(m_Courses[i].m_sPath) == 0 )
			return &m_Courses[i];

	return NULL;
}
