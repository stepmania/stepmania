#include "stdafx.h"
//-----------------------------------------------------------------------------
// File: Song.cpp
//
// Desc: Holds metadata for a song and the song's step data.
//
// Copyright (c) 2001 Chris Danford.  All rights reserved.
//-----------------------------------------------------------------------------


#include "Util.h"
#include "IniFile.h"
#include "BmsFile.h"
#include "Steps.h"
#include "RageUtil.h"

#include "Song.h"
#include <assert.h>


//////////////////////////////
// Song
//////////////////////////////

bool Song::LoadFromSongDir( CString sDir )
{
	RageLog( "Song::LoadFromSongDir(%s)", sDir );

	// make sure there is a trailing '\\' at the end of sDir
	if( sDir.Right(1) != "\\" )
		sDir += "\\";

	// save song dir
	m_sSongDir = sDir;

	// We are going to load from either a .song or .msd file.  
	// If no .song files exist, then look for an .msd.

	CStringArray arrayBMSFileNames;
	GetDirListing( sDir + CString("*.bms"), arrayBMSFileNames );
	int iNumBMSFiles = arrayBMSFileNames.GetSize();

	CStringArray arrayMSDFileNames;
	GetDirListing( sDir + CString("*.msd"), arrayMSDFileNames );
	int iNumMSDFiles = arrayMSDFileNames.GetSize();

	/*
	int j;
	RageLog( "Found the following .bms files:" );
	for( j=0; j<iNumBMSFiles; j++ )
		RageLog( arrayBMSFileNames.GetAt(j) );
	
	RageLog( "Found the following .msd files:" );
	for( j=0; j<iNumMSDFiles; j++ )
		RageLog( arrayMSDFileNames.GetAt(j) );
	*/

	if( iNumBMSFiles > 0 )
	{
		for( int i=0; i<iNumBMSFiles; i++ )
			LoadFromBMSFile( sDir + arrayBMSFileNames.GetAt(i) );
	}
	else if( iNumMSDFiles == 1 )
	{
		//m_sSongFile = arrayMSDFileNames.GetAt( 0 );
		//RageLog( "Found '%s'.  Let's use it...", m_sSongFile );
		//LoadFromMSDFile( sDir + m_sSongFile );
	}
	else if( iNumMSDFiles > 0 )
		RageError( ssprintf("Found more than one MSD file in '%s'.  Which should I use?", sDir) );
	else 
		RageError( ssprintf("Couldn't find any BMS or MSD files in '%s'", sDir) );


	return TRUE;

}

BOOL Song::LoadFromBMSFile( CString sPath )
{
	RageLog( "Song::LoadFromBMSFile(%s)", sPath );

	BmsFile bms( sPath );
	bms.ReadFile();

	Steps new_steps;	// just assume one steps per file for now

	CMapStringToString &mapValues = bms.mapValues;

	POSITION pos = mapValues.GetStartPosition();
	while( pos != NULL )
	{
		CString valuename;
		CString data_string;
		CStringArray data_array;
		mapValues.GetNextAssoc( pos, valuename, data_string );
		split(data_string, ":", data_array);

		// handle the data
		if( valuename == "#PLAYER" )
			;

		else if( valuename == "#GENRE" )
			m_sCreator = data_array[0];

		else if( valuename == "#TITLE" )
			m_sTitle = data_array[0];

		else if( valuename == "#ARTIST" )
			m_sArtist = data_array[0];

		else if( valuename == "#BPM" )
			m_fBPM = (FLOAT)atof( data_array[0] );

		else if( valuename == "#PLAYLEVEL" )
			new_steps.iDifficulty = atoi( data_array[0] );

		else if( valuename == "#BackBMP"  ||  valuename == "#backBMP")
			m_sBackground = data_array[0];

		else if( valuename == "#WAV99" )
			m_sMusic = data_array[0];

		else if( valuename.GetLength() == 6 )	// this is probably step or offset data.  Looks like "#00705"
		{
			int iMeasureNo	= atoi( valuename.Mid(1,3) );
			int iNoteNum	= atoi( valuename.Mid(4,2) );
			CString sNoteData = data_array[0];
			CArray<BOOL, BOOL&> arrayNotes;

			for( int i=0; i<sNoteData.GetLength(); i+=2 )
			{
				BOOL bThisIsANote = sNoteData.Mid(i,2) == "01"  // step
								 || sNoteData.Mid(i,2) == "99"; // offset
				arrayNotes.Add( bThisIsANote );
			}

			const int iNumNotesInThisMeasure = arrayNotes.GetSize();
			//RageLog( "%s:%s: iMeasureNo = %d, iNoteNum = %d, iNumNotesInThisMeasure = %d", 
			//	valuename, sNoteData, iMeasureNo, iNoteNum, iNumNotesInThisMeasure );
			for( int j=0; j<iNumNotesInThisMeasure; j++ )
			{
				if( arrayNotes.GetAt(j) == TRUE )
				{
					FLOAT fPercentThroughMeasure = (FLOAT)j/(FLOAT)iNumNotesInThisMeasure;

					// index is in quarter beats starting at beat 0
					int iStepIndex = (int) ( (iMeasureNo + fPercentThroughMeasure)
									 * BEATS_IN_MEASURE * ELEMENTS_PER_BEAT );
// BMS encoding:
// 4&8panel: Player1  Player2
//    Left		11		21
//    Down		13		23
//    Up		15		25
//    Right		16		26
//	6panel:	 Player1  Player2
//    Left		11		21
//    Left+Up	12		22
//    Down		13		23
//    Up		14		24
//    Up+Right	15		25
//    Right		16		26
//
//	Notice that 15 and 25 have two different meanings!  What were they thinking???
//	While reading in, use the 6 panel mapping.  After reading in, detect if only 4 notes
//	are filled in.  If so, shift the Up+Right column back to the Up column
//
// Our internal encoding:
// 0x0001 left		player 1
// 0x0002 left+up	player 1
// 0x0004 down		player 1
// 0x0008 up		player 1
// 0x0010 up+right	player 1
// 0x0020 right		player 1
// 0x0040 left		player 2
// 0x0080 left+up	player 2
// 0x0100 down		player 2
// 0x0200 up		player 2
// 0x0400 up+right	player 2
// 0x0800 right		player 2
CMap<int, int, Step, Step>  mapBMSNumberToOurStepBit;
mapBMSNumberToOurStepBit[11] = 0x0001;
mapBMSNumberToOurStepBit[12] = 0x0002;
mapBMSNumberToOurStepBit[13] = 0x0004;
mapBMSNumberToOurStepBit[14] = 0x0008;
mapBMSNumberToOurStepBit[15] = 0x0010;
mapBMSNumberToOurStepBit[16] = 0x0020;
mapBMSNumberToOurStepBit[21] = 0x0040;
mapBMSNumberToOurStepBit[22] = 0x0080;
mapBMSNumberToOurStepBit[23] = 0x0100;
mapBMSNumberToOurStepBit[24] = 0x0200;
mapBMSNumberToOurStepBit[25] = 0x0400;
mapBMSNumberToOurStepBit[26] = 0x0800;

					new_steps.StepArray[iStepIndex] |= mapBMSNumberToOurStepBit[iNoteNum];
					new_steps.StatusArray[iStepIndex] = not_stepped_on;

				}
			}
		}
	}

	int iNumSteps = 0;
	for( int i=0; i<new_steps.StatusArray.GetSize(); i++ ) {
		if( new_steps.StatusArray[i] == not_stepped_on ) {
			iNumSteps++;
		}
	}

	RageLog( "%d of %d steps elements are filled", iNumSteps, new_steps.StatusArray.GetSize() );
	// we're done reading in all of the BMS values


	if(		 m_sTitle.Find("<BASIC>")			>0 )	new_steps.sDescription = "BASIC";
	else if( m_sTitle.Find("<ANOTHER>")			>0 )	new_steps.sDescription = "ANOTHER";
	else if( m_sTitle.Find("<TRICK>")			>0 )	new_steps.sDescription = "TRICK";
	else if( m_sTitle.Find("<MANIAC>")			>0 )	new_steps.sDescription = "MANIAC";
	else if( m_sTitle.Find("<SSR>")				>0 )	new_steps.sDescription = "SSR";
	else if( m_sTitle.Find("<6PANELS BASIC>")	>0 )	new_steps.sDescription = "BASIC";
	else if( m_sTitle.Find("<6PANELS ANOTHER>")	>0 )	new_steps.sDescription = "ANOTHER";
	else if( m_sTitle.Find("<6PANELS TRICK>")	>0 )	new_steps.sDescription = "TRICK";
	else if( m_sTitle.Find("<6PANELS MANIAC>")	>0 )	new_steps.sDescription = "MANIAC";
	else if( m_sTitle.Find("<6PANELS SSR>")		>0 )	new_steps.sDescription = "SSR";
	else if( m_sTitle.Find("<BASIC DOUBLE>")	>0 )	new_steps.sDescription = "BASIC DOUBLE";
	else if( m_sTitle.Find("<ANOTHER DOUBLE>")	>0 )	new_steps.sDescription = "ANOTHER DOUBLE";
	else if( m_sTitle.Find("<TRICK DOUBLE>")	>0 )	new_steps.sDescription = "TRICK DOUBLE";
	else if( m_sTitle.Find("<MANIAC DOUBLE>")	>0 )	new_steps.sDescription = "MANIAC DOUBLE";
	else if( m_sTitle.Find("<SSR DOUBLE>")		>0 )	new_steps.sDescription = "SSR DOUBLE";
	else if( m_sTitle.Find("<COUPLE>")			>0 )	new_steps.sDescription = "COUPLE";
	else if( m_sTitle.Find("<BATTLE>")			>0 )	new_steps.sDescription = "BATTLE";
	else												new_steps.sDescription = "UNKNOWN";

	if(		 new_steps.sDescription.Find("COUPLE") >0 )	new_steps.type = st_couple;
	else if( new_steps.sDescription.Find("BATTLE") >0 )	new_steps.type = st_couple;
	else if( new_steps.sDescription.Find("DOUBLE") >0 )	new_steps.type = st_double;
	else												new_steps.type = st_single;

	arraySteps.Add( new_steps );
	
	// strip steps type out of description
	m_sTitle.Replace( " <" + new_steps.sDescription + ">", "" );

	FillEmptyValuesWithDefaults();

	return TRUE;
}


BOOL Song::LoadFromMSDFile( CString sPath )
{
	RageLog( "Song::LoadFromMSDFile(%s)", sPath );

	
	// BROKEN
	/*
	CStdioFile file;	
	if( !file.Open( GetSongFilePath(), CFile::modeRead ) )
		RageError( ssprintf("Error opening Song file '%s'.", GetSongFilePath()) );

	// read the whole file into a sFileText
	CString sFileText;
	CString buffer;
	while( file.ReadString(buffer) )
		sFileText += buffer;
	file.Close();

	// split sFileText into strings containing each value expression
	CStringArray arrayValueStrings;
	split( sFileText, ";", arrayValueStrings );


	// for each value expression string, parse it into a value name and data
	for( int i=0; i < arrayValueStrings.GetSize(); i++ )
	{
		CString sValueString = arrayValueStrings[i];

		// split the value string into tokens
		CStringArray arrayValueTokens;
		split( sValueString, ":", arrayValueTokens );

		CString sValueName = arrayValueTokens.GetAt( 0 );
			
		// handle the data
		if( sValueName == "#TITLE" )
			m_sTitle = arrayValueTokens[1];

		else if( sValueName == "#ARTIST" )
			m_sArtist = arrayValueTokens[1];

		else if( sValueName == "#REMIXER" || sValueName == "#MSD")
			m_sCreator = arrayValueTokens[1];

		else if( sValueName == "#BPM" )
			m_fBPM = (FLOAT)atof( arrayValueTokens[1] );

		else if( sValueName == "#OFFSET" )
			m_fBeatOffset = (FLOAT)atoi( arrayValueTokens[1] );

		else if( sValueName == "#GAP" )
			// the units of GAP is 1/10 second
			m_fBeatOffset = (FLOAT)atof( arrayValueTokens[1] ) / 10.0f;

		else if( sValueName == "#MUSIC" )
			m_sMusic = arrayValueTokens[1];

		else if( sValueName == "#SAMPLE" )
			m_sSample = arrayValueTokens[1];

		else if( sValueName == "#BANNER" )
			m_sBanner = arrayValueTokens[1];

		else if( sValueName == "#BACKGROUND" )
			m_sBackground = arrayValueTokens[1];

		// handle STEPS defined in the file for backwards compatibility with MSD files
		else if( sValueName == "#SINGLE" || sValueName == "#DOUBLE" || sValueName == "#COUPLE" )
		{
			Steps s;
			if( sValueName == "#SINGLE" )
				s.type = st_single;
			else if( sValueName == "#DOUBLE" )
				s.type = st_double;
			else if( sValueName == "#COUPLE" )
				s.type = st_couple;
			else
				RageError( ssprintf("Unknown game type '%s' in '%s'", sValueName, GetSongFilePath()) );

			s.sDescription = arrayValueTokens[1];
			s.iDifficulty = atoi( arrayValueTokens[2] );
			s.sPadDataLeft = arrayValueTokens[3];
			if( sValueName == "#DOUBLE" || sValueName == "#COUPLE" )
				s.sPadDataRight = arrayValueTokens[4];
			
			switch( s.type)
			{
			case st_single:
				arraySingleSteps.Add( s );
				break;
			case st_double:
				arrayDoubleSteps.Add( s );
				break;
			case st_couple:
				arrayCoupleSteps.Add( s );
				break;
			}
		}
		else
			// do nothing.  We don't care about this value name
			;
	}

	FillEmptyValuesWithDefaults();
	*/

	return TRUE;
}


void Song::FillEmptyValuesWithDefaults()
{
	if( m_sTitle == "" )	m_sTitle = "Untitled song";
	if( m_sArtist == "" )	m_sArtist = "Unknown artist";
	if( m_sCreator == "" )	m_sCreator = "";
	if( m_fBPM == 0.0 )
		RageError( ssprintf("No #BPM specified in '%s.'", GetSongFilePath()) );

	if( m_fBeatOffset == 0.0 )	
		RageLog( "Warning: #OFFSET or #GAP in '%s' is either 0.0, or was missing.", GetSongFilePath() );

	if( m_sMusic == "" )
	{
		m_sMusic = "music.mp3";
		if( !DoesFileExist( m_sSongDir + m_sMusic ) )
		{
			// music.mp3 didn't exist, so search for any mp3 in the same dir
			CStringArray arrayMP3Files;
			GetDirListing( m_sSongDir + CString("*.mp3"), arrayMP3Files );

			if( arrayMP3Files.GetSize() != 0 )		// we found a .mp3 file!
				m_sMusic = arrayMP3Files.GetAt( 0 );
			else
				RageError( ssprintf("Music could not be found.  In Song file '%s' no #MUSIC was specified, music.mp3 doesn't exist, and no other MP3s could be found.", GetSongFilePath()) );
		}
	}
	if( m_sSample == "" )
	{
		m_sSample = "sample.mp3";
		if( !DoesFileExist( m_sSongDir + m_sSample ) )
			m_sSample = m_sMusic;	// we assured above that the m_sMusic file exists
	}
	if( m_sBanner == "" )
	{
		m_sBanner = "banner.bmp";
		if( !DoesFileExist( m_sSongDir + m_sBanner ) )
		{
			m_sBanner = "banner.png";
			if( !DoesFileExist( m_sSongDir + m_sBanner ) )
			{
				// couldn't find a true banner, so search for any bmp in the same dir
				CStringArray arrayBMPFiles;
				GetDirListing( m_sSongDir + CString("*.bmp"), arrayBMPFiles );

				if( arrayBMPFiles.GetSize() != 0 )		// we found a .bmp file!
					m_sBanner = arrayBMPFiles.GetAt( 0 );
				else
					RageError( ssprintf("Banner could not be found.  In Song file '%s' no #BANNER was specified, banner.bmp doesn't exist, and no other BMPs could be found.", GetSongFilePath()) );
			}
		}
	}
	if( m_sBackground == "" ) 
	{
		m_sBackground = "background.bmp";
		if( !DoesFileExist( m_sSongDir + m_sBackground ) )
			m_sBackground = m_sBanner;	// we assured above that the m_sBanner file exists
	}
}

