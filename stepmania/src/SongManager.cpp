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


SongManager*	SONGS = NULL;	// global and accessable from anywhere in our program


const CString g_sStatisticsFileName = "statistics.ini";


SongManager::SongManager()
{
	m_pCurSong = NULL;
	for( int p=0; p<NUM_PLAYERS; p++ )
		m_pStepsPlayer[p] = NULL;

	InitSongArrayFromDisk();
	ReadStatisticsFromDisk();
}


SongManager::~SongManager()
{
	CleanUpSongArray();
	SaveStatisticsToDisk();
}


void SongManager::InitSongArrayFromDisk()
{
	/////////////////////////////
	// Handle DWI support
	/////////////////////////////

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


	/////////////////////////////
	// Load songs from StepMania's directory structure in Songs
	/////////////////////////////

	// Find all group directories in "Songs" folder
	CStringArray arrayGroupDirs;
	GetDirListing( "Songs\\*.*", arrayGroupDirs, true );
	SortCStringArray( arrayGroupDirs );
	
	for( i=0; i< arrayGroupDirs.GetSize(); i++ )	// for each dir in /Songs/
	{
		CString sGroupDirName = arrayGroupDirs[i];
		sGroupDirName.MakeLower();

		if( sGroupDirName == "cvs" )	// ignore the directory called "CVS"
			continue;
		
		// Find all Song folders in this group directory
		CStringArray arraySongDirs;
		GetDirListing( ssprintf("Songs\\%s\\*.*", sGroupDirName, true), arraySongDirs );
		SortCStringArray( arraySongDirs );

		for( int j=0; j< arraySongDirs.GetSize(); j++ )	// for each song dir
		{
			CString sSongDirName = arraySongDirs[j];
			sSongDirName.MakeLower();

			if( sSongDirName == "cvs" )	// ignore the directory called "CVS"
				continue;

			// load DWIs from the sub dirs
			Song* pNewSong = new Song;
			pNewSong->LoadFromSongDir( ssprintf("Songs\\%s\\%s", sGroupDirName, sSongDirName) );
			m_pSongs.Add( pNewSong );
		}
	}



	RageLog( "Found %d Songs.", m_pSongs.GetSize() );
}


void SongManager::CleanUpSongArray()
{
	for( int i=0; i<m_pSongs.GetSize(); i++ )
	{
		SAFE_DELETE( m_pSongs[i] );
	}

	m_pSongs.RemoveAll();
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
		return;		// load nothing
		//RageError( "could not read config file" );
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

			// Parse for Song name and Steps name
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


			// Search for the corresponding Steps pointer.
			Steps* pSteps = NULL;
			for( i=0; i<pSong->arraySteps.GetSize(); i++ )
			{
				if( pSong->arraySteps[i].m_sDescription == szStepsName )	// match!
				{
					pSteps = &pSong->arraySteps[i];
					break;
				}
			}
			if( pSteps == NULL )	// didn't find a match
				continue;	// skip this entry


			// Parse the Steps statistics.
			char szGradeLetters[10];	// longest possible string is "AAA"

			iRetVal = sscanf( 
				value_string, 
				"%d::%[^:]::%d::%d", 
				&pSteps->m_iNumTimesPlayed,
				szGradeLetters,
				&pSteps->m_iTopScore,
				&pSteps->m_iMaxCombo
			);
			if( iRetVal != 4 )
				continue;

			pSteps->m_TopGrade = StringToGrade( szGradeLetters );
		}
	}
}

void SongManager::SaveStatisticsToDisk()
{
	IniFile ini;
	ini.SetPath( g_sStatisticsFileName );
	if( !ini.ReadFile() ) {
		return;		// load nothing
		//RageError( "could not read config file" );
	}

	// save song statistics
	for( int i=0; i<m_pSongs.GetSize(); i++ )		// for each Song
	{
		Song* pSong = m_pSongs[i];

		for( int j=0; j<pSong->arraySteps.GetSize(); j++ )		// for each Steps
		{
			Steps* pSteps = &pSong->arraySteps[j];

			if( pSteps->m_TopGrade == GRADE_NO_DATA )
				continue;

			// Each value has the format "SongName::StepsName=TimesPlayed::TopGrade::TopScore::MaxCombo".

			CString sName = ssprintf( "%s::%s", pSong->GetTitle(), pSteps->m_sDescription );
			CString sValue = ssprintf( 
				"%d::%s::%d::%d",
				pSteps->m_iNumTimesPlayed,
				GradeToString( pSteps->m_TopGrade ),
				pSteps->m_iTopScore, 
				pSteps->m_iMaxCombo
			);

			ini.SetValue( "Statistics", sName, sValue );
		}
	}

	ini.WriteFile();
}