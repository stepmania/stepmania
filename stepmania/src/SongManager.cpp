#include "stdafx.h"
/*
-----------------------------------------------------------------------------
 Class: SongManager

 Desc: See header.

 Copyright (c) 2001-2002 by the names listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "SongManager.h"
#include "IniFile.h"


SongManager*	SONG = NULL;	// global and accessable from anywhere in our program


const CString g_sStatisticsFileName = "statistics.ini";


SongManager::SongManager()
{
	m_pCurSong = NULL;
	for( int p=0; p<NUM_PLAYERS; p++ )
		m_pCurPattern[p] = NULL;

	InitSongArrayFromDisk();
	ReadStatisticsFromDisk();
}


SongManager::~SongManager()
{
	SaveStatisticsToDisk();
	CleanUpSongArray();
}


void SongManager::InitSongArrayFromDisk()
{
	LoadStepManiaSongDir( "Songs" );
	LoadDWISongDir( "DWI Support" );
	RageLog( "Found %d Songs.", m_pSongs.GetSize() );
}

void SongManager::LoadStepManiaSongDir( CString sDir )
{
	// trim off the trailing slash if any
	sDir.TrimRight( "/\\" );

	// Find all group directories in "Songs" folder
	CStringArray arrayGroupDirs;
	GetDirListing( sDir+"\\*.*", arrayGroupDirs, true );
	SortCStringArray( arrayGroupDirs );
	
	for( int i=0; i< arrayGroupDirs.GetSize(); i++ )	// for each dir in /Songs/
	{
		CString sGroupDirName = arrayGroupDirs[i];

		if( 0 == stricmp( sGroupDirName, "cvs" ) )	// the directory called "CVS"
			continue;		// ignore it

		// Check to see if they put a song directly inside of the group folder
		CStringArray arrayFiles;
		GetDirListing( ssprintf("%s\\%s\\*.mp3", sDir, sGroupDirName), arrayFiles );
		GetDirListing( ssprintf("%s\\%s\\*.wav", sDir, sGroupDirName), arrayFiles );
		if( arrayFiles.GetSize() > 0 )
			RageError( 
				ssprintf( "The song folder '%s' must be placed inside of a group folder.\n\n"
					"All song folders must be placed below a group folder.  For example, 'Songs\\DDR 4th Mix\\B4U'.  See the StepMania readme for more info.",
					ssprintf("%s\\%s", sDir, sGroupDirName ) )
			);
		
		// Look for a group banner in this group folder
		CStringArray arrayGroupBanners;
		GetDirListing( ssprintf("%s\\%s\\*.png", sDir, sGroupDirName), arrayGroupBanners );
		GetDirListing( ssprintf("%s\\%s\\*.jpg", sDir, sGroupDirName), arrayGroupBanners );
		GetDirListing( ssprintf("%s\\%s\\*.gif", sDir, sGroupDirName), arrayGroupBanners );
		GetDirListing( ssprintf("%s\\%s\\*.bmp", sDir, sGroupDirName), arrayGroupBanners );
		if( arrayGroupBanners.GetSize() > 0 )
		{
			m_mapGroupToBannerPath[sGroupDirName] = ssprintf("%s\\%s\\%s", sDir, sGroupDirName, arrayGroupBanners[0] );
			RageLog( ssprintf("Group banner for '%s' is '%s'.", sGroupDirName, m_mapGroupToBannerPath[sGroupDirName]) );
		}

		// Find all Song folders in this group directory
		CStringArray arraySongDirs;
		GetDirListing( ssprintf("%s\\%s\\*.*", sDir, sGroupDirName), arraySongDirs, true );
		SortCStringArray( arraySongDirs );

		for( int j=0; j< arraySongDirs.GetSize(); j++ )	// for each song dir
		{
			CString sSongDirName = arraySongDirs[j];

			if( 0 == stricmp( sSongDirName, "cvs" ) )	// the directory called "CVS"
				continue;		// ignore it

			// this is a song directory.  Load a new song!
			Song* pNewSong = new Song;
			pNewSong->LoadFromSongDir( ssprintf("%s\\%s\\%s", sDir, sGroupDirName, sSongDirName) );
			m_pSongs.Add( pNewSong );
		}
	}
}

void SongManager::LoadDWISongDir( CString sDir )
{
	// trim off the trailing slash if any
	sDir.TrimRight( "/\\" );

	// Find all directories in "DWIs" folder
	CStringArray arrayDirs;
	GetDirListing( "DWI Support\\DWIs\\*.*", arrayDirs, true );
	SortCStringArray( arrayDirs );
	
	for( int i=0; i< arrayDirs.GetSize(); i++ )	// for each dir in /DWIs/
	{
		CString sDirName = arrayDirs[i];
		sDirName.MakeLower();
		if( sDirName == "cvs" )	// ignore the directory called "CVS"
			continue;
		
		// Find all DWIs in this directory
		CStringArray arrayDWIFiles;
		GetDirListing( ssprintf("DWI Support\\DWIs\\%s\\*.dwi", sDirName), arrayDWIFiles );
		SortCStringArray( arrayDWIFiles );

		for( int j=0; j< arrayDWIFiles.GetSize(); j++ )	// for each DWI file
		{
			CString sDWIFileName = arrayDWIFiles[j];
			sDWIFileName.MakeLower();

			// load DWIs from the sub dirs
			Song* pNewSong = new Song;
			pNewSong->LoadSongInfoFromDWIFile( ssprintf("DWI Support\\DWIs\\%s\\%s", sDirName, sDWIFileName) );
			m_pSongs.Add( pNewSong );
		}
	}
}



void SongManager::CleanUpSongArray()
{
	for( int i=0; i<m_pSongs.GetSize(); i++ )
	{
		SAFE_DELETE( m_pSongs[i] );
	}

	m_pSongs.RemoveAll();
	m_mapGroupToBannerPath.RemoveAll();
}


void SongManager::ReloadSongArray()
{
	InitSongArrayFromDisk();
	CleanUpSongArray();
}



void SongManager::ReadStatisticsFromDisk()
{
	IniFile ini;
	ini.SetPath( g_sStatisticsFileName );
	if( !ini.ReadFile() ) {
		RageLog( "WARNING: Could not read config file '%s'.", g_sStatisticsFileName );
		return;		// load nothing
	}


	// load song statistics
	CMapStringToString* pKey = ini.GetKeyPointer( "Statistics" );
	if( pKey )
	{
		for( POSITION pos = pKey->GetStartPosition(); pos != NULL; )
		{
			CString name_string, value_string;

			pKey->GetNextAssoc( pos, name_string, value_string );

			// Each value has the format "SongName::StepsName=TimesPlayed::TopGrade::TopScore::MaxCombo".

			char szSongName[256];
			char szStepsName[256];
			int iRetVal;
			int i;

			// Parse for Song name and Pattern name
			iRetVal = sscanf( name_string, "%[^:]::%[^\n]", szSongName, szStepsName );
			if( iRetVal != 2 )
				continue;	// this line doesn't match what is expected
	
			
			// Search for the corresponding Song pointer.
			Song* pSong = NULL;
			for( i=0; i<m_pSongs.GetSize(); i++ )
			{
				if( m_pSongs[i]->GetTitle() == szSongName )	// match!
				{
					pSong = m_pSongs[i];
					break;
				}
			}
			if( pSong == NULL )	// didn't find a match
				continue;	// skip this entry


			// Search for the corresponding Pattern pointer.
			Pattern* pPattern = NULL;
			for( i=0; i<pSong->m_arrayPatterns.GetSize(); i++ )
			{
				if( pSong->m_arrayPatterns[i].m_sDescription == szStepsName )	// match!
				{
					pPattern = &pSong->m_arrayPatterns[i];
					break;
				}
			}
			if( pPattern == NULL )	// didn't find a match
				continue;	// skip this entry


			// Parse the Pattern statistics.
			char szGradeLetters[10];	// longest possible string is "AAA"

			iRetVal = sscanf( 
				value_string, 
				"%d::%[^:]::%d::%d", 
				&pPattern->m_iNumTimesPlayed,
				szGradeLetters,
				&pPattern->m_iTopScore,
				&pPattern->m_iMaxCombo
			);
			if( iRetVal != 4 )
				continue;

			pPattern->m_TopGrade = StringToGrade( szGradeLetters );
		}
	}
}

void SongManager::SaveStatisticsToDisk()
{
	IniFile ini;
	ini.SetPath( g_sStatisticsFileName );

	// save song statistics
	for( int i=0; i<m_pSongs.GetSize(); i++ )		// for each Song
	{
		Song* pSong = m_pSongs[i];

		for( int j=0; j<pSong->m_arrayPatterns.GetSize(); j++ )		// for each Pattern
		{
			Pattern* pPattern = &pSong->m_arrayPatterns[j];

			if( pPattern->m_TopGrade == GRADE_NO_DATA )
				continue;		// skip

			// Each value has the format "SongName::PatternName=TimesPlayed::TopGrade::TopScore::MaxCombo".

			CString sName = ssprintf( "%s::%s", pSong->GetTitle(), pPattern->m_sDescription );
			CString sValue = ssprintf( 
				"%d::%s::%d::%d",
				pPattern->m_iNumTimesPlayed,
				GradeToString( pPattern->m_TopGrade ),
				pPattern->m_iTopScore, 
				pPattern->m_iMaxCombo
			);

			ini.SetValue( "Statistics", sName, sValue );
		}
	}

	ini.WriteFile();
}


CString SongManager::GetGroupBannerPath( CString sGroupName )
{
	CString sPath;

	if( m_mapGroupToBannerPath.Lookup( sGroupName, sPath ) )
		return sPath;
	else
		return "";
}
