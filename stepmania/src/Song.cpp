#include "stdafx.h"
/*
-----------------------------------------------------------------------------
 File: Song.h

 Desc: Holds metadata for a song and the song's step data.

 Copyright (c) 2001 Chris Danford.  All rights reserved.
-----------------------------------------------------------------------------
*/


#include "Util.h"
#include "IniFile.h"
#include "BmsFile.h"
#include "Steps.h"
#include "RageUtil.h"

#include "Song.h"
#include <assert.h>


const CString TEXTURE_FALLBACK_BANNER		= "Textures\\Fallback Banner.png";
const CString TEXTURE_FALLBACK_BACKGROUND	= "Textures\\Fallback Background.png";



int CompareBPMSegments(const void *arg1, const void *arg2)
{
	// arg1 and arg2 are of type Step**
	BPMSegment* seg1 = (BPMSegment*)arg1;
	BPMSegment* seg2 = (BPMSegment*)arg2;

	float score1 = seg1->m_fStartBeat;
	float score2 = seg2->m_fStartBeat;

	if( score1 == score2 )
		return 0;
	else if( score1 < score2 )
		return -1;
	else
		return 1;
}

void SortBPMSegmentsArray( CArray<BPMSegment,BPMSegment&> &arrayBPMSegments )
{
	qsort( arrayBPMSegments.GetData(), arrayBPMSegments.GetSize(), sizeof(BPMSegment), CompareBPMSegments );
}

int CompareFreezeSegments(const void *arg1, const void *arg2)
{
	// arg1 and arg2 are of type Step**
	FreezeSegment* seg1 = (FreezeSegment*)arg1;
	FreezeSegment* seg2 = (FreezeSegment*)arg2;

	float score1 = seg1->m_fStartBeat;
	float score2 = seg2->m_fStartBeat;

	if( score1 == score2 )
		return 0;
	else if( score1 < score2 )
		return -1;
	else
		return 1;
}

void SortFreezeSegmentsArray( CArray<FreezeSegment,FreezeSegment&> &arrayFreezeSegments )
{
	qsort( arrayFreezeSegments.GetData(), arrayFreezeSegments.GetSize(), sizeof(FreezeSegment), CompareFreezeSegments );
}


//////////////////////////////
// Song
//////////////////////////////
Song::Song()
{
	m_bChangedSinceSave = false;
	m_fOffsetInSeconds = 0;
	m_iMaxCombo = m_iTopScore = m_iNumTimesPlayed = 0;
	m_bHasBeenLoadedBefore = false;
}


void Song::GetBeatAndBPSFromElapsedTime( float fElapsedTime, float &fBeatOut, float &fBPSOut )
{
	fElapsedTime += m_fOffsetInSeconds;


	for( int i=0; i<m_BPMSegments.GetSize(); i++ ) // foreach BPMSegment
	{
		BPMSegment &this_seg = m_BPMSegments[i];

		float fStartBeatThisSegment = m_BPMSegments[i].m_fStartBeat;
		bool bIsLastBPMSegment = i==m_BPMSegments.GetSize()-1;
		float fStartBeatNextSegment = bIsLastBPMSegment ? 40000/*inf*/ : m_BPMSegments[i+1].m_fStartBeat; 
		float fBeatsInThisSegment = fStartBeatNextSegment - fStartBeatThisSegment;
		float fBPM = m_BPMSegments[i].m_fBPM;
		float fBPS = fBPM / 60.0f;

		// calculate the number of seconds in this segment
		float fSecondsInThisSegment =  fBeatsInThisSegment / fBPS;
		for( int j=0; j<m_FreezeSegments.GetSize(); j++ )	// foreach freeze
		{
			if( fStartBeatThisSegment <= m_FreezeSegments[j].m_fStartBeat  &&  m_FreezeSegments[j].m_fStartBeat < fStartBeatNextSegment )	
			{
				// this freeze lies within this BPMSegment
				fSecondsInThisSegment += m_FreezeSegments[j].m_fFreezeSeconds;
			}
		}


		if( fElapsedTime > fSecondsInThisSegment )
		{
			// this BPMSegement is not the current segment
			fElapsedTime -= fSecondsInThisSegment;
		}
		else 
		{
			// this BPMSegment IS the current segment

			float fBeatEstimate = fStartBeatThisSegment + fElapsedTime*fBPS;

			for( int j=0; j<m_FreezeSegments.GetSize(); j++ )	// foreach freeze
			{
				if( fStartBeatThisSegment <= m_FreezeSegments[j].m_fStartBeat  &&  m_FreezeSegments[j].m_fStartBeat <= fStartBeatNextSegment )	
				{
					// this freeze lies within this BPMSegment

					if( m_FreezeSegments[j].m_fStartBeat <= fBeatEstimate )
					{
						fElapsedTime -= m_FreezeSegments[j].m_fFreezeSeconds;
						// re-estimate
						fBeatEstimate = fStartBeatThisSegment + fElapsedTime*fBPS;
						if( fBeatEstimate < m_FreezeSegments[j].m_fStartBeat )
						{
							fBeatOut = m_FreezeSegments[j].m_fStartBeat;
							fBPSOut = fBPS;
							return;
						}
					}
					else
						break;
				}
			}

			fBeatOut = fBeatEstimate;
			fBPSOut = fBPS;
			return;
		}

	}
}


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

	CStringArray arrayDWIFileNames;
	GetDirListing( sDir + CString("*.dwi"), arrayDWIFileNames );
	int iNumDWIFiles = arrayDWIFileNames.GetSize();


	if( iNumDWIFiles > 1 )
	{
		RageError( ssprintf("There is more than one DWI file in '%s'.  Which one should I use?", sDir) );
	}
	else if( iNumDWIFiles == 1 )
	{
		LoadSongInfoFromDWIFile( sDir + arrayDWIFileNames[0] );
	}
	else if( iNumBMSFiles > 0 )
	{
		LoadSongInfoFromBMSDir( sDir );
	}
	else 
	{
		RageError( ssprintf("Couldn't find any BMS or MSD files in '%s'", sDir) );
	}


	return TRUE;

}



bool Song::LoadSongInfoFromBMSDir( CString sDir )
{
	RageLog( "Song::LoadSongInfoFromBMSDir(%s)", sDir );

	// make sure there is a trailing '\\' at the end of sDir
	if( sDir.Right(1) != "\\" )
		sDir += "\\";

	// save song dir
	m_sSongDir = sDir;

	// get group name
	CStringArray sDirectoryParts;
	split( m_sSongDir, "\\", sDirectoryParts, true );
	for( int p=0; p<sDirectoryParts.GetSize(); p++ )
		RageLog( "sDirectoryParts[p] = '%s'", sDirectoryParts[p] );
	m_sGroupName = sDirectoryParts[1];


	CStringArray arrayBMSFileNames;
	GetDirListing( sDir + CString("*.bms"), arrayBMSFileNames );

	if( arrayBMSFileNames.GetSize() == 0 )
		RageError( ssprintf("Couldn't find any BMS files in '%s'", sDir) );


	// Load the Song info from the first BMS file.  Silly BMS duplicates the song info in every
	// file, so this method assumes that the song info is identical for every BMS file in
	// the directory.
	CString sBMSFilePath = m_sSongDir + arrayBMSFileNames[0];

	// load the Steps from the rest of the BMS files
	for( int i=0; i<arrayBMSFileNames.GetSize(); i++ ) 
	{
		arraySteps.SetSize( arraySteps.GetSize()+1 );
		Steps &new_steps = arraySteps[ arraySteps.GetSize()-1 ];
		new_steps.LoadStepsFromBMSFile( m_sSongDir + arrayBMSFileNames[i] );
	}



	BmsFile bms( sBMSFilePath );
	bms.ReadFile();

	CMapStringToString &mapValues = bms.mapValues;

	POSITION pos = mapValues.GetStartPosition();
	while( pos != NULL )
	{
		CString value_name;
		CString data_string;
		CStringArray data_array;
		mapValues.GetNextAssoc( pos, value_name, data_string );
		split(data_string, ":", data_array);

		// handle the data
		if( value_name == "#GENRE" )
			m_sCreator = data_array[0];
		else if( value_name == "#TITLE" ) {
			m_sTitle = data_array[0];
			// strip steps type out of description leaving only song title (looks like 'Music <BASIC>')
			int iPosBracket = m_sTitle.Find( " <" );
			if( iPosBracket != -1 )
				m_sTitle = m_sTitle.Left( iPosBracket );
		}
		else if( value_name == "#ARTIST" )
			m_sArtist = data_array[0];
		else if( value_name == "#BPM" ) {
			BPMSegment new_seg;
			new_seg.m_fStartBeat = 0;
			new_seg.m_fBPM = (float)atof( data_array[0] );

			// add, then sort
			m_BPMSegments.Add( new_seg );
			SortBPMSegmentsArray( m_BPMSegments );
			RageLog( "Inserting new BPM change at beat %f, BPM %f", new_seg.m_fStartBeat, new_seg.m_fBPM );
		}
		else if( value_name == "#BackBMP"  ||  value_name == "#backBMP")
			m_sBackgroundPath = m_sSongDir + data_array[0];
		else if( value_name == "#WAV99" )
			m_sMusicPath = m_sSongDir + data_array[0];
		else if( value_name.GetLength() == 6 )	// this is probably step or offset data.  Looks like "#00705"
		{
			int iMeasureNo	= atoi( value_name.Mid(1,3) );
			int iTrackNum	= atoi( value_name.Mid(4,2) );

			CString sNoteData = data_array[0];
			CArray<int, int> arrayNotes;

			for( int i=0; i<sNoteData.GetLength(); i+=2 )
			{
				CString sNote = sNoteData.Mid(i,2);
				int iNote;
				sscanf( sNote, "%x", &iNote );	// data is in hexadecimal
				arrayNotes.Add( iNote );
			}

			const int iNumNotesInThisMeasure = arrayNotes.GetSize();
			//RageLog( "%s:%s: iMeasureNo = %d, iTrackNum = %d, iNumNotesInThisMeasure = %d", 
			//	valuename, sNoteData, iMeasureNo, iTrackNum, iNumNotesInThisMeasure );
			for( int j=0; j<iNumNotesInThisMeasure; j++ )
			{
				if( arrayNotes[j] != 0 )
				{
					float fPercentThroughMeasure = (float)j/(float)iNumNotesInThisMeasure;

					// index is in quarter beats starting at beat 0
					int iStepIndex = (int) ( (iMeasureNo + fPercentThroughMeasure)
									 * BEATS_PER_MEASURE * ELEMENTS_PER_BEAT );

					switch( iTrackNum )
					{
					case 01:	// background music track
						float fBeatOffset;
						fBeatOffset = StepIndexToBeat(iStepIndex);
						float fBPS;
						fBPS = m_BPMSegments[0].m_fBPM/60.0f;
						m_fOffsetInSeconds = fBeatOffset / fBPS;
						//RageLog( "Found offset to be index %d, beat %f", iStepIndex, StepIndexToBeat(iStepIndex) );
						break;
					case 03:	// bpm
						BPMSegment new_seg;
						new_seg.m_fStartBeat = StepIndexToBeat( iStepIndex );
						new_seg.m_fBPM = (float)arrayNotes[j];

						m_BPMSegments.Add( new_seg );	// add to back for now (we'll sort later)
						SortBPMSegmentsArray( m_BPMSegments );
						break;
					}
				}
			}
		}
	}

	for( i=0; i<m_BPMSegments.GetSize(); i++ )
		RageLog( "There is a BPM change at beat fd, BPM %f, index %d", 
					m_BPMSegments[i].m_fStartBeat, m_BPMSegments[i].m_fBPM, i );



	TidyUpData();

	return TRUE;
}


bool Song::LoadSongInfoFromDWIFile( CString sPath )
{
	RageLog( "Song::LoadFromDWIFile(%s)", sPath );
	
	// save song file path
	m_sSongFilePath = sPath;

	// save song dir
	CString sDir, sFName, sExt;
	splitrelpath(sPath, sDir, sFName, sExt);
	m_sSongDir = sDir;

	// get group name
	sDir.MakeLower();
	if( sDir.Find( "dwi support" ) != -1 )	// loading from DWI support
	{
		int iIndexOfFirstBackslash = sDir.Find('\\');
		int iIndexOfSecondBackslash = sDir.Find('\\', iIndexOfFirstBackslash+1);
		int iIndexOfThirdBackslash = sDir.Find('\\', iIndexOfSecondBackslash+1);
		m_sGroupName = sDir.Mid( iIndexOfSecondBackslash+1, iIndexOfThirdBackslash-iIndexOfSecondBackslash-1 );
	}
	else
	{
		// get group name
		CStringArray sDirectoryParts;
		split( m_sSongDir, "\\", sDirectoryParts, true );
		for( int p=0; p<sDirectoryParts.GetSize(); p++ )
			RageLog( "sDirectoryParts[p] = '%s'", sDirectoryParts[p] );
		m_sGroupName = sDirectoryParts[1];
	}

	// save probable image paths
	m_sBackgroundPath = ssprintf(".\\DWI Support\\Backgrounds\\%s\\%s.avi", m_sGroupName, sFName);
	if( !DoesFileExist( GetBackgroundPath() ) )
		m_sBackgroundPath = ssprintf(".\\DWI Support\\Backgrounds\\%s\\%s.mpg", m_sGroupName, sFName);
	if( !DoesFileExist( GetBackgroundPath() ) )
		m_sBackgroundPath = ssprintf(".\\DWI Support\\Backgrounds\\%s\\%s.mpeg", m_sGroupName, sFName);
	if( !DoesFileExist( GetBackgroundPath() ) )
		m_sBackgroundPath = ssprintf(".\\DWI Support\\Backgrounds\\%s\\%s.png", m_sGroupName, sFName);
	
	m_sBannerPath = ssprintf(".\\DWI Support\\Banners\\%s\\%s.png", m_sGroupName, sFName);

	
	CStdioFile file;	
	if( !file.Open( GetSongFilePath(), CFile::modeRead ) )
		RageError( ssprintf("Error opening DWI file '%s'.", GetSongFilePath()) );


//	MessageBox( NULL, sFName, sFName, MB_OK );


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
		if( sValueName == "#FILE" )
			m_sMusicPath = CString("DWI Support\\") + arrayValueTokens[1];

		else if( sValueName == "#TITLE" )
			m_sTitle = arrayValueTokens[1];

		else if( sValueName == "#ARTIST" )
			m_sArtist = arrayValueTokens[1];

		else if( sValueName == "#BPM" )
		{
			BPMSegment new_seg;
			new_seg.m_fStartBeat = 0;
			new_seg.m_fBPM = (float)atof( arrayValueTokens[1] );

			m_BPMSegments.Add( new_seg );
			SortBPMSegmentsArray( m_BPMSegments );
		}
		else if( sValueName == "#GAP" )
			// the units of GAP is 1/1000 second
			m_fOffsetInSeconds = -atoi( arrayValueTokens[1] ) / 1000.0f;

		else if( sValueName == "#FREEZE" )
		{
			CStringArray arrayFreezeExpressions;
			split( arrayValueTokens[1], ",", arrayFreezeExpressions );

			for( int f=0; f<arrayFreezeExpressions.GetSize(); f++ )
			{
				CStringArray arrayFreezeValues;
				split( arrayFreezeExpressions[f], "=", arrayFreezeValues );
				int fIndex = atoi( arrayFreezeValues[0] ) * 2;
				float fFreezeBeat = StepIndexToBeat( fIndex );
				float fFreezeSeconds = atof( arrayFreezeValues[1] ) / 1000.0f;
				
				FreezeSegment new_seg;
				new_seg.m_fStartBeat = fFreezeBeat;
				new_seg.m_fFreezeSeconds = fFreezeSeconds;

				RageLog( "Adding a freeze segment: beat: %f, seconds = %f", new_seg.m_fStartBeat, new_seg.m_fFreezeSeconds );

				m_FreezeSegments.Add( new_seg );	// add to back for now (we'll sort later)
				SortFreezeSegmentsArray( m_FreezeSegments );

			}
		}

		else if( sValueName == "#CHANGEBPM"  || sValueName == "#BPMCHANGE" )
		{
			CStringArray arrayBPMChangeExpressions;
			split( arrayValueTokens[1], ",", arrayBPMChangeExpressions );

			for( int b=0; b<arrayBPMChangeExpressions.GetSize(); b++ )
			{
				CStringArray arrayBPMChangeValues;
				split( arrayBPMChangeExpressions[b], "=", arrayBPMChangeValues );
				int fIndex = atoi( arrayBPMChangeValues[0] ) * 2;
				float fBeat = StepIndexToBeat( fIndex );
				float fNewBPM = atoi( arrayBPMChangeValues[1] );
				
				BPMSegment new_seg;
				new_seg.m_fStartBeat = fBeat;
				new_seg.m_fBPM = fNewBPM;
				
				// add and sort
				m_BPMSegments.Add( new_seg );
				SortBPMSegmentsArray( m_BPMSegments );
			}
		}

		else if( sValueName == "#SINGLE" || sValueName == "#DOUBLE" || sValueName == "#COUPLE" )
		{
			arraySteps.SetSize( arraySteps.GetSize()+1 );
			Steps &new_steps = arraySteps[ arraySteps.GetSize()-1 ];
			new_steps.LoadFromDWIValueTokens( arrayValueTokens );
		}
		else
			// do nothing.  We don't care about this value name
			;
	}

	TidyUpData();


	return TRUE;
}


void Song::TidyUpData()
{
	if( m_sTitle == "" )	m_sTitle = "Untitled song";
	if( m_sArtist == "" )	m_sArtist = "Unknown artist";
	if( m_sCreator == "" )	m_sCreator = "Unknown creator";
	if( m_BPMSegments.GetSize() == 0 )
		RageError( ssprintf("No #BPM specified in '%s.'", GetSongFilePath()) );

	if( m_sMusicPath == "" || !DoesFileExist(GetMusicPath()) )
	{
		CStringArray arrayPossibleMusic;
		GetDirListing( m_sSongDir + CString("*.mp3"), arrayPossibleMusic );
		GetDirListing( m_sSongDir + CString("*.ogg"), arrayPossibleMusic );
		GetDirListing( m_sSongDir + CString("*.wav"), arrayPossibleMusic );

		if( arrayPossibleMusic.GetSize() != 0 )		// we found a match
			m_sMusicPath = m_sSongDir + arrayPossibleMusic[0];
		else
			m_sMusicPath = "";
		//	RageError( ssprintf("Music could not be found.  Please check the Song file '%s' and verify the specified #MUSIC exists.", GetSongFilePath()) );
	}

	if( !DoesFileExist(GetBannerPath()) )
		m_sBannerPath = "";
	if( m_sBannerPath == "" )
	{
		// find the smallest image in the directory

		CStringArray arrayPossibleBanners;
		GetDirListing( m_sSongDir + CString("*.png"), arrayPossibleBanners );
		GetDirListing( m_sSongDir + CString("*.jpg"), arrayPossibleBanners );
		GetDirListing( m_sSongDir + CString("*.bmp"), arrayPossibleBanners );

		DWORD dwSmallestFileSoFar = 0xFFFFFFFF;
		CString sSmallestFileSoFar = "";

		for( int i=0; i<arrayPossibleBanners.GetSize(); i++ )
		{
			DWORD this_size = GetFileSizeInBytes( m_sSongDir + arrayPossibleBanners[i] );
			if( this_size < dwSmallestFileSoFar )	// we have a new leader!
			{
				dwSmallestFileSoFar = this_size;
				sSmallestFileSoFar = m_sSongDir + arrayPossibleBanners[i];
			}
		}

		if( sSmallestFileSoFar != "" )		// we found a match
			m_sBannerPath = sSmallestFileSoFar;
		else
			m_sBannerPath = "";

		//RageError( ssprintf("Banner could not be found.  Please check the Song file '%s' and verify the specified #BANNER exists.", GetSongFilePath()) );
	}
	if( !DoesFileExist(GetBackgroundPath()) )
		m_sBackgroundPath = "";
	if( m_sBackgroundPath == "" ) 
	{
		// find the largest image in the directory

		CStringArray arrayPossibleBackgrounds;
		GetDirListing( m_sSongDir + CString("*.avi"), arrayPossibleBackgrounds );
		GetDirListing( m_sSongDir + CString("*.mpg"), arrayPossibleBackgrounds );
		GetDirListing( m_sSongDir + CString("*.mpeg"), arrayPossibleBackgrounds );
		GetDirListing( m_sSongDir + CString("*.png"), arrayPossibleBackgrounds );
		GetDirListing( m_sSongDir + CString("*.jpg"), arrayPossibleBackgrounds );
		GetDirListing( m_sSongDir + CString("*.bmp"), arrayPossibleBackgrounds );

		DWORD dwLargestFileSoFar = 0;
		CString sLargestFileSoFar = "";

		for( int i=0; i<arrayPossibleBackgrounds.GetSize(); i++ )
		{
			DWORD this_size = GetFileSizeInBytes( m_sSongDir + arrayPossibleBackgrounds[i] );
			if( this_size > dwLargestFileSoFar )	// we have a new leader!
			{
				dwLargestFileSoFar = this_size;
				sLargestFileSoFar = m_sSongDir + arrayPossibleBackgrounds[i];
			}
		}

		if( sLargestFileSoFar != "" )		// we found a match
			m_sBackgroundPath = sLargestFileSoFar;
		else
			m_sBackgroundPath = "";
		// RageError( ssprintf("Background could not be found.  Please check the Song file '%s' and verify the specified #BANNER exists.", GetSongFilePath()) );
	}
}



void Song::GetStepsThatMatchGameMode( GameMode gm, CArray<Steps*, Steps*&>& arrayAddTo )
{
	for( int i=0; i<arraySteps.GetSize(); i++ )	// for each of the Song's Steps
	{
		Steps* pCurrentSteps = &arraySteps[i];
		Steps::StepsMode sm = pCurrentSteps->m_StepsMode;

		switch( gm )	// different logic for different game modes
		{
		case single4:
			if( sm == Steps::SMsingle4 )
				arrayAddTo.Add( pCurrentSteps );
			break;
		case single6:
			if( sm == Steps::SMsingle6 )
				arrayAddTo.Add( pCurrentSteps );
			break;
		case versus4:
			if( sm == Steps::SMsingle4  ||  sm == Steps::SMcouple  ||  sm == Steps::SMbattle )
				arrayAddTo.Add( pCurrentSteps );
			break;
		case double4:
			if( sm == Steps::SMdouble4 )
				arrayAddTo.Add( pCurrentSteps );
			break;
		}
	}

}

void Song::GetNumFeet( GameMode gm, int& iDiffEasyOut, int& iDiffMediumOut, int& iDiffHardOut )
{
	iDiffEasyOut = iDiffMediumOut = iDiffHardOut = -1;		// -1 means not found
	CArray<Steps*, Steps*&> arrayMatchingSteps;
	GetStepsThatMatchGameMode( gm, arrayMatchingSteps );

	for( int i=0; i<arrayMatchingSteps.GetSize(); i++ )
	{
		int iNumFeet = arrayMatchingSteps[i]->m_iNumFeet;

		switch( arrayMatchingSteps[i]->m_difficulty )
		{
		case Steps::easy:
			iDiffEasyOut = iNumFeet;
			break;
		case Steps::medium:
			iDiffMediumOut = iNumFeet;
			break;
		case Steps::hard:
			iDiffHardOut = iNumFeet;
			break;
		case Steps::other:
			// should do something intelligent to fill in the missing spots...
			if( iDiffEasyOut < 0  &&  iNumFeet <= 4 )
				iDiffEasyOut = iNumFeet;
			else if( iDiffHardOut < 0  &&  iNumFeet >= 7 )
				iDiffHardOut = iNumFeet;
			else if( iDiffMediumOut < 0 )
				iDiffMediumOut = iNumFeet;
			break;
		}
	}
}

