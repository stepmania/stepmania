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


//////////////////////////////
// Song
//////////////////////////////
Song::Song()
{
	m_bChangedSinceSave = false;
//	m_fBPM = 0;
	m_fOffsetInSeconds = 0;
}


void Song::GetBeatAndBPSFromElapsedTime( float fElapsedTime, float &fBeatOut, float &fBPSOut )
{
	fElapsedTime += m_fOffsetInSeconds;

	for( int i=0; i<m_BPMSegments.GetSize(); i++ ) {
		float fStartBeatThisSegment = m_BPMSegments[i].m_fStartBeat;
		bool bIsLastBPMSegment = i==m_BPMSegments.GetSize()-1;
		float fStartBeatNextSegment = bIsLastBPMSegment ? 40000/*inf*/ : m_BPMSegments[i+1].m_fStartBeat; 
		float fBeatsInThisSegment = fStartBeatNextSegment - fStartBeatThisSegment;
		float fBPM = m_BPMSegments[i].m_fBPM;
		float fBPS = fBPM / 60.0f;
		float fSecondsInThisSegment =  fBeatsInThisSegment / fBPS;
		fElapsedTime -= m_BPMSegments[i].m_fFreezeSeconds;
		if( fElapsedTime < 0 )	fElapsedTime = 0;

		if( fElapsedTime > fSecondsInThisSegment )
			fElapsedTime -= fSecondsInThisSegment;
		else {
			fBeatOut = fStartBeatThisSegment + fElapsedTime*fBPS;
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
		// Load the Song info from the first BMS file.  Silly BMS duplicates the song info in every
		// file, so this method assumes that the song info is identical for every BMS file in
		// the directory.
		LoadSongInfoFromBMSFile( sDir + arrayBMSFileNames.GetAt(0) );
		for( int i=0; i<iNumBMSFiles; i++ ) {
			arraySteps.SetSize( arraySteps.GetSize()+1 );
			Steps &new_steps = arraySteps[ arraySteps.GetSize()-1 ];
			new_steps.LoadStepsFromBMSFile( sDir + arrayBMSFileNames.GetAt(i) );
		}
	}
	else if( iNumDWIFiles == 1 )
	{
		m_sSongFile = arrayDWIFileNames[0];
		RageLog( "Found '%s'.  Let's use it...", m_sSongFile );
		LoadSongInfoFromDWIFile( sDir + m_sSongFile );
	}
	else if( iNumDWIFiles > 0 )
		RageError( ssprintf("Found more than one DWI file in '%s'.  Which should I use?", sDir) );
	else 
		RageError( ssprintf("Couldn't find any BMS or MSD files in '%s'", sDir) );


	return TRUE;

}

bool Song::LoadSongInfoFromBMSFile( CString sPath )
{
	RageLog( "Song::LoadFromBMSFile(%s)", sPath );

	if( sPath == "Songs\\saints\\saint_4basic.bms" )
		int kdgjf = 0;

	BmsFile bms( sPath );
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
			// find insertion point
			m_BPMSegments.Add( new_seg );
			RageLog( "Inserting new BPM change at beat %f, BPM %f", new_seg.m_fStartBeat, new_seg.m_fBPM );
		}
		else if( value_name == "#BackBMP"  ||  value_name == "#backBMP")
			m_sBackground = data_array[0];
		else if( value_name == "#WAV99" )
			m_sMusic = data_array[0];
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
						
						// sort the BPM segments so that they are always in order
						SortBPMSegmentsArray( m_BPMSegments );
						break;
					}
				}
			}
		}
	}

	for( int i=0; i<m_BPMSegments.GetSize(); i++ )
		RageLog( "There is a BPM change at beat fd, BPM %f, index %d", 
					m_BPMSegments[i].m_fStartBeat, m_BPMSegments[i].m_fBPM, i );



	TidyUpData();

	return TRUE;
}


bool Song::LoadSongInfoFromDWIFile( CString sPath )
{
	RageLog( "Song::LoadFromDWIFile(%s)", sPath );

	
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
		if( sValueName == "#FILE" )
			m_sMusic = arrayValueTokens[1];

		else if( sValueName == "#TITLE" )
			m_sTitle = arrayValueTokens[1];

		else if( sValueName == "#ARTIST" )
			m_sArtist = arrayValueTokens[1];

		else if( sValueName == "#BPM" )
		{
			BPMSegment new_seg;
			new_seg.m_fStartBeat = 0;
			new_seg.m_fBPM = atof( arrayValueTokens[1] );
			m_BPMSegments.Add( new_seg );	// add to back for now (we'll sort later)
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
				
				// find the BPM segment corresponding to this freeze
				for( int b=0; b<m_BPMSegments.GetSize(); b++ )
				{
					if( m_BPMSegments[b].m_fStartBeat == fFreezeBeat )
					{
						m_BPMSegments[b].m_fFreezeSeconds = fFreezeSeconds*0.8;
						break;
					}
				}

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
				m_BPMSegments.Add( new_seg );	// add to back for now (we'll sort later)
				
				// sort the BPM segments so that they are always in order
				SortBPMSegmentsArray( m_BPMSegments );
			}
		}
		else if( sValueName == "#BANNER" )
			m_sBanner = arrayValueTokens[1];

		else if( sValueName == "#BACKGROUND" )
			m_sBackground = arrayValueTokens[1];

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
	if( m_sCreator == "" )	m_sCreator = "";
	if( m_BPMSegments.GetSize() == 0 )
		RageError( ssprintf("No #BPM specified in '%s.'", GetSongFilePath()) );

	if( m_fOffsetInSeconds == 0.0 )	
		RageLog( "WARNING: #OFFSET or #GAP in '%s' is either 0.0, or was missing.", GetSongFilePath() );

	if( m_sMusic == "" || !DoesFileExist(GetMusicPath()) )
	{
		CStringArray arrayPossibleMusic;
		GetDirListing( m_sSongDir + CString("*.mp3"), arrayPossibleMusic );
		GetDirListing( m_sSongDir + CString("*.wav"), arrayPossibleMusic );

		if( arrayPossibleMusic.GetSize() != 0 )		// we found a match
			m_sMusic = arrayPossibleMusic.GetAt( 0 );
		else
			RageError( ssprintf("Music could not be found.  Please check the Song file '%s' and verify the specified #MUSIC exists.", GetSongFilePath()) );
	}
	if( m_sBanner == "" || !DoesFileExist(GetBannerPath()) )
	{
		CStringArray arrayPossibleBanners;
		GetDirListing( m_sSongDir + CString("*banner*.*"), arrayPossibleBanners );
		GetDirListing( m_sSongDir + CString("*.png"), arrayPossibleBanners );
		GetDirListing( m_sSongDir + CString("*.bmp"), arrayPossibleBanners );

		if( arrayPossibleBanners.GetSize() != 0 )		// we found a match
			m_sBanner = arrayPossibleBanners.GetAt( 0 );
		else
			RageError( ssprintf("Banner could not be found.  Please check the Song file '%s' and verify the specified #BANNER exists.", GetSongFilePath()) );
	}
	if( m_sBackground == "" || !DoesFileExist(GetBackgroundPath()) ) 
	{
		CStringArray arrayPossibleBanners;
		GetDirListing( m_sSongDir + CString("*b*g*.*"), arrayPossibleBanners );
		GetDirListing( m_sSongDir + CString("*640*.*"), arrayPossibleBanners );
		GetDirListing( m_sSongDir + CString("*.png"), arrayPossibleBanners );
		GetDirListing( m_sSongDir + CString("*.bmp"), arrayPossibleBanners );

		if( arrayPossibleBanners.GetSize() != 0 )		// we found a match
			m_sBackground = arrayPossibleBanners.GetAt( 0 );
		else
			RageError( ssprintf("Banner could not be found.  Please check the Song file '%s' and verify the specified #BANNER exists.", GetSongFilePath()) );
	}
}

/*
	D3DXIMAGE_INFO ddii;

	if( FAILED( hr = D3DXGetImageInfoFromFile(m_sFilePath,&ddii) ) ) {
        RageErrorHr( ssprintf("D3DXGetImageInfoFromFile() failed for file '%s'.", m_sFilePath), hr );
	}
*/


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