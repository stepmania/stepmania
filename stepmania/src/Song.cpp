#include "stdafx.h"
/*
-----------------------------------------------------------------------------
 File: Song.h

 Desc: Holds metadata for a song and the song's step data.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
-----------------------------------------------------------------------------
*/

#include "Notes.h"
#include "RageUtil.h"

#include "Song.h"
#include <math.h>	// for fmod
#include "RageLog.h"
#include "ErrorCatcher/ErrorCatcher.h"



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
	m_fMusicSampleStartSeconds = m_fMusicSampleLengthSeconds = -1;
	m_iMusicBytes = 0;
	m_fMusicLength = 0;
}


void Song::GetBeatAndBPSFromElapsedTime( float fElapsedTime, float &fBeatOut, float &fBPSOut )
{
//	LOG->WriteLine( "GetBeatAndBPSFromElapsedTime( fElapsedTime = %f )", fElapsedTime );
	// This function is a nightmare.  Don't even try to understand it. :-)

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


// This is a super hack, but it's only called from ScreenEdit, so it's OK :-)
// Writing an inverse function of GetBeatAndBPSFromElapsedTime() is impossible,
// so do a binary search to get close to the correct elapsed time.
float Song::GetElapsedTimeFromBeat( float fBeat )
{
	float fElapsedTimeBestGuess = 100;	//  seconds
	float fSecondsToMove = fElapsedTimeBestGuess/2;	//  seconds
	float fBeatOut, fBPSOut;

	while( fSecondsToMove > 0.1f )
	{
		GetBeatAndBPSFromElapsedTime( fElapsedTimeBestGuess, fBeatOut, fBPSOut );
		if( fBeatOut > fBeat )
			fElapsedTimeBestGuess -= fSecondsToMove;
		else
			fElapsedTimeBestGuess += fSecondsToMove;

		fSecondsToMove /= 2;
	}

	return fElapsedTimeBestGuess;
}

void Song::GetMainAndSubTitlesFromFullTitle( CString sFullTitle, CString &sMainTitleOut, CString &sSubTitleOut )
{
	char szSeps[] = { '-', '~' };
	for( int i=0; i<sizeof(szSeps); i++ )
	{
		const char c = szSeps[i];
		int iBeginIndex = sFullTitle.Find( c );
		if( iBeginIndex == -1 )
			continue;
		int iEndIndex = sFullTitle.Find( c, iBeginIndex+1 );	
		if( iEndIndex == -1 )
			continue;
		sMainTitleOut = sFullTitle.Left( iBeginIndex-1 );
		sSubTitleOut = sFullTitle.Mid( iBeginIndex, iEndIndex-iBeginIndex+1 );
		return;
	}
	sMainTitleOut = sFullTitle; 
	sSubTitleOut = ""; 
};	

CString Song::GetCacheFilePath()
{
	ULONG hash = GetHashForString( m_sSongDir );
	return ssprintf( "Cache\\%u.song", hash );
}

bool Song::LoadFromSongDir( CString sDir )
{
	LOG->WriteLine( "Song::LoadFromSongDir(%s)", sDir );

	// make sure there is a trailing '\\' at the end of sDir
	if( sDir.Right(1) != "\\" )
		sDir += "\\";

	// save song dir
	m_sSongDir = sDir;


	//
	// First look in the cache for this song (without loading NoteData)
	//
	//if( LoadFromCacheFile(false) )
	//	return true;


	//
	// There was no entry in the cache for this song.
	// Let's load it from a file, then write a cache entry.
	//
	CStringArray arrayBMSFileNames;
	GetDirListing( sDir + CString("*.bms"), arrayBMSFileNames );
	int iNumBMSFiles = arrayBMSFileNames.GetSize();

	CStringArray arrayDWIFileNames;
	GetDirListing( sDir + CString("*.dwi"), arrayDWIFileNames );
	int iNumDWIFiles = arrayDWIFileNames.GetSize();

	CStringArray arraySongFileNames;
	GetDirListing( sDir + CString("*.song"), arraySongFileNames );
	int iNumSongFiles = arraySongFileNames.GetSize();


	if( iNumSongFiles > 1 )
		FatalError( "There is more than one .song file in '%s'.  There should be only one!", sDir );
	else if( iNumDWIFiles > 1 )
		FatalError( "There is more than one DWI file in '%s'.  There should be only one!", sDir );
	else if( iNumSongFiles == 1 )
		LoadFromSMDir( sDir );
	else if( iNumDWIFiles == 1 )
		LoadFromDWIFile( sDir + arrayDWIFileNames[0] );
	else if( iNumBMSFiles > 0 )
		LoadFromBMSDir( sDir );
	else 
		FatalError( "Couldn't find any .song, BMS, or DWI files in '%s'.  This is not a valid song directory.", sDir );

	TidyUpData();

	//
	// In order to save memory, we're going to save the file back to the cache, 
	// then unload all the large NoteData. 
	//
	//SaveToCacheFile();
	//LoadFromCacheFile( false );

	return true;
}



bool Song::LoadFromBMSDir( CString sDir )
{
	LOG->WriteLine( "Song::LoadFromBMSDir(%s)", sDir );

	// make sure there is a trailing '\\' at the end of sDir
	if( sDir.Right(1) != "\\" )
		sDir += "\\";

	// save song dir
	m_sSongDir = sDir;

	// get group name
	CStringArray sDirectoryParts;
	split( m_sSongDir, "\\", sDirectoryParts, true );
	m_sGroupName = sDirectoryParts[1];


	CStringArray arrayBMSFileNames;
	GetDirListing( sDir + CString("*.bms"), arrayBMSFileNames );

	if( arrayBMSFileNames.GetSize() == 0 )
		FatalError( ssprintf("Couldn't find any BMS files in '%s'", sDir) );


	// Load the Song info from the first BMS file.  Silly BMS duplicates the song info in every
	// file.  So, we read the song data from only the first BMS file and assume that the info 
	// is identical for every BMS file in the directory.
	m_sSongFilePath = m_sSongDir + arrayBMSFileNames[0];

	// load the Notes from the rest of the BMS files
	for( int i=0; i<arrayBMSFileNames.GetSize(); i++ ) 
	{
		m_arrayNotes.SetSize( m_arrayNotes.GetSize()+1 );
		Notes &new_steps = m_arrayNotes[ m_arrayNotes.GetSize()-1 ];
		new_steps.LoadFromBMSFile( m_sSongDir + arrayBMSFileNames[i] );
	}


	CStdioFile file;	
	if( !file.Open( m_sSongFilePath, CFile::modeRead|CFile::shareDenyNone ) )
	{
		FatalError( ssprintf("Failed to open %s.", m_sSongFilePath) );
		return false;
	}

	CString line;
	while( file.ReadString(line) )	// foreach line
	{
		CString value_name;		// fill these in
		CString value_data;	

		// BMS value names can be separated by a space or a colon.
		int iIndexOfFirstColon = line.Find( ":" );
		int iIndexOfFirstSpace = line.Find( " " );

		if( iIndexOfFirstColon == -1 )
			iIndexOfFirstColon = 10000;
		if( iIndexOfFirstSpace == -1 )
			iIndexOfFirstSpace = 10000;
		
		int iIndexOfSeparator = min( iIndexOfFirstSpace, iIndexOfFirstColon );

		if( iIndexOfSeparator != 10000 )
		{
			value_name = line.Mid( 0, iIndexOfSeparator );
			value_data = line;	// the rest
			value_data.Delete(0,iIndexOfSeparator+1);
		}
		else	// no separator
		{
			value_name = line;
		}


		value_name.MakeLower();


		// handle the data
		if( -1 != value_name.Find("#title") ) 
		{
			// strip Notes type out of description leaving only song title (looks like 'B4U <BASIC>')
			value_data.Replace( "(ANOTHER)", "" );
			value_data.Replace( "(BASIC)", "" );
			value_data.Replace( "(MANIAC)", "" );
			value_data.Replace( "<ANOTHER>", "" );
			value_data.Replace( "<BASIC>", "" );
			value_data.Replace( "<MANIAC>", "" );

			GetMainAndSubTitlesFromFullTitle( value_data, m_sMainTitle, m_sSubTitle );
		}
		else if( -1 != value_name.Find("#artist") ) 
		{
			m_sArtist = value_data;
		}
		else if( -1 != value_name.Find("#bpm") ) 
		{
			BPMSegment new_seg;
			new_seg.m_fStartBeat = 0;
			new_seg.m_fBPM = (float)atof( value_data );

			// add, then sort
			m_BPMSegments.Add( new_seg );
			SortBPMSegmentsArray( m_BPMSegments );
			LOG->WriteLine( "Inserting new BPM change at beat %f, BPM %f", new_seg.m_fStartBeat, new_seg.m_fBPM );
		}
		else if( -1 != value_name.Find("#backbmp") ) 
		{
			m_sBackgroundPath = m_sSongDir + value_data;
		}
		else if( -1 != value_name.Find("#wav") ) 
		{
			m_sMusicPath = m_sSongDir + value_data;
		}
		else if( value_name.Left(1) == "#"  
			 && IsAnInt( value_name.Mid(1,3) )
			 && IsAnInt( value_name.Mid(4,2) ) )	// this is step or offset data.  Looks like "#00705"
		{
			int iMeasureNo	= atoi( value_name.Mid(1,3) );
			int iTrackNum	= atoi( value_name.Mid(4,2) );

			CString sNoteData = value_data;
			CArray<int, int> arrayNotes;

			for( int i=0; i<sNoteData.GetLength(); i+=2 )
			{
				CString sNote = sNoteData.Mid(i,2);
				int iNote;
				sscanf( sNote, "%x", &iNote );	// data is in hexadecimal
				arrayNotes.Add( iNote );
			}

			const int iNumNotesInThisMeasure = arrayNotes.GetSize();
			//LOG->WriteLine( "%s:%s: iMeasureNo = %d, iTrackNum = %d, iNumNotesInThisMeasure = %d", 
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
						fBeatOffset = NoteRowToBeat( (float)iStepIndex );
						float fBPS;
						fBPS = m_BPMSegments[0].m_fBPM/60.0f;
						m_fOffsetInSeconds = fBeatOffset / fBPS;
						//LOG->WriteLine( "Found offset to be index %d, beat %f", iStepIndex, NoteRowToBeat(iStepIndex) );
						break;
					case 03:	// bpm
						BPMSegment new_seg;
						new_seg.m_fStartBeat = NoteRowToBeat( (float)iStepIndex );
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
		LOG->WriteLine( "There is a BPM change at beat %f, BPM %f, index %d", 
					m_BPMSegments[i].m_fStartBeat, m_BPMSegments[i].m_fBPM, i );


	return TRUE;
}


bool Song::LoadFromDWIFile( CString sPath )
{
	LOG->WriteLine( "Song::LoadFromDWIFile(%s)", sPath );
	
	// save song file path
	m_sSongFilePath = sPath;

	// save song dir
	CString sDir, sFName, sExt;
	splitrelpath(sPath, sDir, sFName, sExt);
	m_sSongDir = sDir;

	// get group name
	sDir.MakeLower();
	CStringArray sDirectoryParts;
	split( m_sSongDir, "\\", sDirectoryParts, true );
	m_sGroupName = sDirectoryParts[1];


	CStdioFile file;	
	if( !file.Open( GetSongFilePath(), CFile::modeRead|CFile::shareDenyNone ) )
		FatalError( ssprintf("Error opening DWI file '%s'.", GetSongFilePath()) );


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

		if( arrayValueTokens.GetSize() == 0 )
			continue;

		CString sValueName = arrayValueTokens.GetAt( 0 );
		sValueName.TrimLeft();

		// handle the data
		if( sValueName == "#FILE" )
			m_sMusicPath = arrayValueTokens[1];

		else if( sValueName == "#TITLE" )
			GetMainAndSubTitlesFromFullTitle( arrayValueTokens[1], m_sMainTitle, m_sSubTitle );

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

		else if( sValueName == "#SAMPLESTART" )
			m_fMusicSampleStartSeconds = TimeToSeconds( arrayValueTokens[1] );

		else if( sValueName == "#SAMPLELENGTH" )
			m_fMusicSampleLengthSeconds = TimeToSeconds(  arrayValueTokens[1] );

		else if( sValueName == "#FREEZE" )
		{
			CStringArray arrayFreezeExpressions;
			split( arrayValueTokens[1], ",", arrayFreezeExpressions );

			for( int f=0; f<arrayFreezeExpressions.GetSize(); f++ )
			{
				CStringArray arrayFreezeValues;
				split( arrayFreezeExpressions[f], "=", arrayFreezeValues );
				float fIndex = atoi( arrayFreezeValues[0] ) * ELEMENTS_PER_BEAT / 4.0f;
				float fFreezeBeat = NoteRowToBeat( fIndex );
				float fFreezeSeconds = (float)atof( arrayFreezeValues[1] ) / 1000.0f;
				
				FreezeSegment new_seg;
				new_seg.m_fStartBeat = fFreezeBeat;
				new_seg.m_fFreezeSeconds = fFreezeSeconds;

				LOG->WriteLine( "Adding a freeze segment: beat: %f, seconds = %f", new_seg.m_fStartBeat, new_seg.m_fFreezeSeconds );

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
				float fIndex = atoi( arrayBPMChangeValues[0] ) * ELEMENTS_PER_BEAT / 4.0f;
				float fBeat = NoteRowToBeat( fIndex );
				float fNewBPM = (float)atoi( arrayBPMChangeValues[1] );
				
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
			m_arrayNotes.SetSize( m_arrayNotes.GetSize()+1, 1 );
			Notes &new_Notes = m_arrayNotes[ m_arrayNotes.GetSize()-1 ];
			new_Notes.LoadFromDWITokens( 
				arrayValueTokens[0], 
				arrayValueTokens[1], 
				arrayValueTokens[2], 
				arrayValueTokens[3], 
				arrayValueTokens.GetSize() == 5 ? arrayValueTokens[4] : "" 
				);
		}
		else
			// do nothing.  We don't care about this value name
			;
	}

	return TRUE;
}


bool Song::LoadFromSMDir( CString sDir )
{
	LOG->WriteLine( "Song::LoadFromSMDir(%s)", sDir );

	int i;

	// make sure there is a trailing '\\' at the end of sDir
	if( sDir.Right(1) != "\\" )
		sDir += "\\";

	// save song dir
	m_sSongDir = sDir;

	// get group name
	CStringArray sDirectoryParts;
	split( m_sSongDir, "\\", sDirectoryParts, true );
	m_sGroupName = sDirectoryParts[1];


	CStringArray arraySongFileNames;
	GetDirListing( sDir + "*.song", arraySongFileNames );
	if( arraySongFileNames.GetSize() == 0 )
		FatalError( "Couldn't find any SM Song files in '%s'", sDir );


	// Load the Song info from the first BMS file.  Silly BMS duplicates the song info in every
	// file.  So, we read the song data from only the first BMS file and assume that the info 
	// is identical for every BMS file in the directory.
	m_sSongFilePath = m_sSongDir + arraySongFileNames[0];


	CStringArray arrayNotesFileNames;
	GetDirListing( sDir + CString("*.notes"), arrayNotesFileNames );

	// load the Notes from the rest of the BMS files
	for( i=0; i<arrayNotesFileNames.GetSize(); i++ ) 
	{
		m_arrayNotes.SetSize( m_arrayNotes.GetSize()+1 );
		Notes &new_steps = m_arrayNotes[ m_arrayNotes.GetSize()-1 ];
		new_steps.LoadFromNotesFile( m_sSongDir + arrayNotesFileNames[i] );
	}


	CStdioFile file;	
	if( !file.Open( GetSongFilePath(), CFile::modeRead|CFile::shareDenyNone ) )
		FatalError( "Error opening DWI file '%s'.", GetSongFilePath() );


	// read the whole file into a sFileText
	CString sFileText;
	CString buffer;
	while( file.ReadString(buffer) )
		sFileText += buffer;
	file.Close();

	// split sFileText into strings containing each value expression
	CStringArray arrayValueStrings;
	split( sFileText, ";", arrayValueStrings, false );


	// for each value expression string, parse it into a value name and data
	for( i=0; i < arrayValueStrings.GetSize(); i++ )
	{
		CString sValueString = arrayValueStrings[i];

		// split the value string into tokens
		CStringArray arrayValueTokens;
		split( sValueString, ":", arrayValueTokens, false );

		if( arrayValueTokens.GetSize() == 0 )
			continue;

		CString sValueName = arrayValueTokens.GetAt( 0 );
		sValueName.TrimLeft();

		// handle the data
		if( sValueName == "#TITLE" )
			GetMainAndSubTitlesFromFullTitle( arrayValueTokens[1], m_sMainTitle, m_sSubTitle );

		if( sValueName == "#MAINTITLE" )
			m_sMainTitle = arrayValueTokens[1];

		if( sValueName == "#SUBTITLE" )
			m_sSubTitle = arrayValueTokens[1];

		else if( sValueName == "#ARTIST" )
			m_sArtist = arrayValueTokens[1];

		else if( sValueName == "#CREDIT" )
			m_sCredit = arrayValueTokens[1];

		else if( sValueName == "#BANNER" )
			m_sBannerPath = sDir + "\\" + arrayValueTokens[1];

		else if( sValueName == "#BACKGROUND" )
			m_sBackgroundPath = sDir + "\\" + arrayValueTokens[1];

		else if( sValueName == "#BACKGROUNDMOVIE" )
		{
			if( arrayValueTokens[1] != "" )
				m_sBackgroundMoviePath = sDir + "\\" + arrayValueTokens[1];
		}
		else if( sValueName == "#CDTITLE" )
		{
			if( arrayValueTokens[1] != "" )
				m_sCDTitlePath = sDir + "\\" + arrayValueTokens[1];
		}
		else if( sValueName == "#MUSIC" )
			m_sMusicPath = sDir + "\\" + arrayValueTokens[1];

		else if( sValueName == "#MUSICBYTES" )
			m_iMusicBytes = atoi( arrayValueTokens[1] );

		else if( sValueName == "#SAMPLESTART" )
			m_fMusicSampleStartSeconds = TimeToSeconds( arrayValueTokens[1] );

		else if( sValueName == "#SAMPLELENGTH" )
			m_fMusicSampleLengthSeconds = TimeToSeconds( arrayValueTokens[1] );

		else if( sValueName == "#OFFSET" )
			m_fOffsetInSeconds = (float)atof( arrayValueTokens[1] );

		else if( sValueName == "#FREEZES" )
		{
			CStringArray arrayFreezeExpressions;
			split( arrayValueTokens[1], ",", arrayFreezeExpressions );

			for( int f=0; f<arrayFreezeExpressions.GetSize(); f++ )
			{
				CStringArray arrayFreezeValues;
				split( arrayFreezeExpressions[f], "=", arrayFreezeValues );
				float fFreezeBeat = (float)atof( arrayFreezeValues[0] );
				float fFreezeSeconds = (float)atof( arrayFreezeValues[1] );
				
				FreezeSegment new_seg;
				new_seg.m_fStartBeat = fFreezeBeat;
				new_seg.m_fFreezeSeconds = fFreezeSeconds;

				LOG->WriteLine( "Adding a freeze segment: beat: %f, seconds = %f", new_seg.m_fStartBeat, new_seg.m_fFreezeSeconds );

				m_FreezeSegments.Add( new_seg );	// add to back for now (we'll sort later)
				SortFreezeSegmentsArray( m_FreezeSegments );
			}
		}

		else if( sValueName == "#BPMS" )
		{
			CStringArray arrayBPMChangeExpressions;
			split( arrayValueTokens[1], ",", arrayBPMChangeExpressions );

			for( int b=0; b<arrayBPMChangeExpressions.GetSize(); b++ )
			{
				CStringArray arrayBPMChangeValues;
				split( arrayBPMChangeExpressions[b], "=", arrayBPMChangeValues );
				float fBeat = (float)atof( arrayBPMChangeValues[0] );
				float fNewBPM = (float)atoi( arrayBPMChangeValues[1] );
				
				BPMSegment new_seg;
				new_seg.m_fStartBeat = fBeat;
				new_seg.m_fBPM = fNewBPM;
				
				// add and sort
				m_BPMSegments.Add( new_seg );
				SortBPMSegmentsArray( m_BPMSegments );
			}
		}

		else
			LOG->WriteLine( "Unexpected value named '%s'", sValueName );
	}

	return TRUE;
}


void Song::TidyUpData()
{
	m_sMainTitle.TrimRight();
	if( m_sMainTitle == "" )	m_sMainTitle = "Untitled song";
	m_sSubTitle.TrimRight();
	if( m_sArtist == "" )		m_sArtist = "Unknown artist";
	if( m_BPMSegments.GetSize() == 0 )
		FatalError( "No #BPM specified in '%s.'", GetSongFilePath() );

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
		//	FatalError( ssprintf("Music could not be found.  Please check the Song file '%s' and verify the specified #MUSIC exists.", GetSongFilePath()) );
	}

	// Save length of music
	if( GetMusicPath() != "" )
	{
		RageSoundStream sound;
		sound.Load( GetMusicPath() );
		m_fMusicLength = sound.GetLengthSeconds();
	}

	if( !DoesFileExist(GetBannerPath()) )
	{
		m_sBannerPath = "";
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

		//FatalError( ssprintf("Banner could not be found.  Please check the Song file '%s' and verify the specified #BANNER exists.", GetSongFilePath()) );
	}

	if( !DoesFileExist(GetBackgroundPath()) )
	{
		m_sBackgroundPath = "";
		// find the largest image in the directory

		CStringArray arrayPossibleBackgrounds;
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
	}

	if( !DoesFileExist(GetBackgroundMoviePath()) )
	{
		m_sBackgroundMoviePath = "";

		CStringArray arrayPossibleBackgroundMovies;
		GetDirListing( m_sSongDir + CString("*.avi"), arrayPossibleBackgroundMovies );
		GetDirListing( m_sSongDir + CString("*.mpg"), arrayPossibleBackgroundMovies );
		GetDirListing( m_sSongDir + CString("*.mpeg"), arrayPossibleBackgroundMovies );

		if( arrayPossibleBackgroundMovies.GetSize() > 0 )		// we found a match
			m_sBackgroundMoviePath = arrayPossibleBackgroundMovies[0];
	}

	if( !DoesFileExist(GetCDTitlePath()) )
	{
		m_sCDTitlePath = "";
	}

	for( int i=0; i<m_arrayNotes.GetSize(); i++ )
	{
		Notes* pNM = &m_arrayNotes[i];

		float fMusicLength = m_fMusicLength;
		if( fMusicLength == 0 )
			fMusicLength = 100;

		pNM->m_fRadarValues[RADAR_STREAM]	= pNM->GetNoteData()->GetStreamRadarValue( fMusicLength );
		pNM->m_fRadarValues[RADAR_VOLTAGE]	= pNM->GetNoteData()->GetVoltageRadarValue( fMusicLength );
		pNM->m_fRadarValues[RADAR_AIR]		= pNM->GetNoteData()->GetAirRadarValue( fMusicLength );
		pNM->m_fRadarValues[RADAR_CHAOS]	= pNM->GetNoteData()->GetChaosRadarValue( fMusicLength );
		pNM->m_fRadarValues[RADAR_FREEZE]	= pNM->GetNoteData()->GetFreezeRadarValue( fMusicLength );
	}
}


void Song::GetNotesThatMatch( NotesType nt, CArray<Notes*, Notes*>& arrayAddTo )
{
	for( int i=0; i<m_arrayNotes.GetSize(); i++ )	// for each of the Song's Notes
		if( m_arrayNotes[i].m_NotesType == nt )
			arrayAddTo.Add( &m_arrayNotes[i] );
}


const int FILE_CACHE_VERSION = 10;	// increment this when the cache file format changes

void Song::SaveToCacheFile()
{
	LOG->WriteLine( "Song::SaveToCacheFile()" );

	int i;
	CString sCacheFilePath = GetCacheFilePath();

	FILE* file = fopen( sCacheFilePath, "w" );
	ASSERT( file != NULL );
	if( file == NULL )
		return;

	fprintf( file, "%d\n", FILE_CACHE_VERSION );
	fprintf( file, "%u\n", GetHashForDirectory(m_sSongDir) );
	WriteStringToFile( file, m_sSongDir );
	WriteStringToFile( file, m_sSongFilePath );
	WriteStringToFile( file, m_sGroupName );

	WriteStringToFile( file, m_sMainTitle );
	WriteStringToFile( file, m_sSubTitle );
	WriteStringToFile( file, m_sArtist );
	WriteStringToFile( file, m_sCredit );
	fprintf( file, "%f\n", m_fOffsetInSeconds );

	WriteStringToFile( file, m_sMusicPath );
	fprintf( file, "%d\n", m_iMusicBytes );
	fprintf( file, "%f\n", m_fMusicLength );
	fprintf( file, "%f\n", m_fMusicSampleStartSeconds );
	fprintf( file, "%f\n", m_fMusicSampleLengthSeconds );
	WriteStringToFile( file, m_sBannerPath );
	WriteStringToFile( file, m_sBackgroundPath );
	WriteStringToFile( file, m_sBackgroundMoviePath );
	WriteStringToFile( file, m_sCDTitlePath );

	fprintf( file, "%d\n", m_BPMSegments.GetSize() );
	for( i=0; i<m_BPMSegments.GetSize(); i++ )
		fprintf( file, "%f,%f\n", m_BPMSegments[i].m_fStartBeat, m_BPMSegments[i].m_fBPM );

	fprintf( file, "%d\n", m_FreezeSegments.GetSize() );
	for( i=0; i<m_FreezeSegments.GetSize(); i++ )
		fprintf( file, "%f,%f\n", m_FreezeSegments[i].m_fStartBeat, m_FreezeSegments[i].m_fFreezeSeconds );

	fprintf( file, "%d\n", m_arrayNotes.GetSize() );
	for( i=0; i<m_arrayNotes.GetSize(); i++ )
		m_arrayNotes[i].WriteToCacheFile( file );

	fclose( file );
}

bool Song::LoadFromCacheFile( bool bLoadNoteData )
{
	LOG->WriteLine( "Song::LoadFromCacheFile( %i )", bLoadNoteData );

	int i;
	CString sCacheFilePath = GetCacheFilePath();

	LOG->WriteLine( "cache file is '%s'.", sCacheFilePath );

	FILE* file = fopen( sCacheFilePath, "r" );
	if( file == NULL )
		return false;

	int iCacheVersion;
	fscanf( file, "%d\n", &iCacheVersion );
	if( iCacheVersion != FILE_CACHE_VERSION )
	{
		LOG->WriteLine( "Cache file versions don't match '%s'.", sCacheFilePath );
		fclose( file );
		DeleteCacheFile();
		return false;
	}

	ULONG hash;
	fscanf( file, "%u\n", &hash );
	if( hash != GetHashForDirectory(m_sSongDir) )
	{
		LOG->WriteLine( "Cache file is out of date.", sCacheFilePath );
		fclose( file );
		return false;
	}

	ReadStringFromFile( file, m_sSongDir );
	ReadStringFromFile( file, m_sSongFilePath );
	ReadStringFromFile( file, m_sGroupName );

	ReadStringFromFile( file, m_sMainTitle );
	ReadStringFromFile( file, m_sSubTitle );
	ReadStringFromFile( file, m_sArtist );
	ReadStringFromFile( file, m_sCredit );
	fscanf( file, "%f\n", &m_fOffsetInSeconds );

	ReadStringFromFile( file, m_sMusicPath );
	fscanf( file, "%d\n", &m_iMusicBytes );
	fscanf( file, "%f\n", &m_fMusicLength );
	fscanf( file, "%f\n", &m_fMusicSampleStartSeconds );
	fscanf( file, "%f\n", &m_fMusicSampleLengthSeconds );
	ReadStringFromFile( file, m_sBannerPath );
	ReadStringFromFile( file, m_sBackgroundPath );
	ReadStringFromFile( file, m_sBackgroundMoviePath );
	ReadStringFromFile( file, m_sCDTitlePath );

	int iNumBPMSegments;
	fscanf( file, "%d\n", &iNumBPMSegments );
	m_BPMSegments.SetSize( iNumBPMSegments );
	for( i=0; i<iNumBPMSegments; i++ )
		fscanf( file, "%f,%f\n", &m_BPMSegments[i].m_fStartBeat, &m_BPMSegments[i].m_fBPM );

	int iNumFreezeSegments;
	fscanf( file, "%d\n", &iNumFreezeSegments );
	m_FreezeSegments.SetSize( iNumFreezeSegments );
	for( i=0; i<iNumFreezeSegments; i++ )
		fscanf( file, "%f,%f\n", &m_FreezeSegments[i].m_fStartBeat, &m_FreezeSegments[i].m_fFreezeSeconds );

	int iNumNotes;
	fscanf( file, "%d\n", &iNumNotes );
	m_arrayNotes.SetSize( iNumNotes );
	for( i=0; i<iNumNotes; i++ )
		m_arrayNotes[i].ReadFromCacheFile( file, bLoadNoteData );

	fclose( file );
	return true;
}

void Song::DeleteCacheFile()
{
	DeleteFile( GetCacheFilePath() );
}


void Song::SaveToSMDir()
{
	LOG->WriteLine( "Song::SaveToSMDir()" );

	int i;
	//
	// rename all old files to avoid confusion
	//
	CStringArray arrayOldFileNames;
	GetDirListing( m_sSongDir + CString("*.bms"), arrayOldFileNames );
	GetDirListing( m_sSongDir + CString("*.dwi"), arrayOldFileNames );
	
	for( i=0; i<arrayOldFileNames.GetSize(); i++ )
	{
		CString sOldPath = m_sSongDir + arrayOldFileNames[i];
		CString sNewPath = sOldPath + ".old";
		MoveFile( sOldPath, sNewPath );
	}

	CString sDir, sFName, sExt;
	splitrelpath( GetSongFilePath(), sDir, sFName, sExt );
	m_sSongFilePath = m_sSongDir + sFName + ".song";

	CStdioFile file;	
	if( !file.Open( m_sSongFilePath, CFile::modeWrite | CFile::modeCreate ) )
		FatalError( "Error opening song file '%s' for writing.", m_sSongFilePath );

	file.WriteString( ssprintf("#MAINTITLE:%s;\n", m_sMainTitle) );
	file.WriteString( ssprintf("#SUBTITLE:%s;\n", m_sSubTitle) );
	file.WriteString( ssprintf("#ARTIST:%s;\n", m_sArtist) );
	file.WriteString( ssprintf("#CREDIT:%s;\n", m_sCredit) );
	
	if( m_sBannerPath != "" )
	{
		splitrelpath( m_sBannerPath, sDir, sFName, sExt );
		file.WriteString( ssprintf("#BANNER:%s;\n", sFName+"."+sExt) );
	}
	else
		file.WriteString( "#BANNER:;\n" );

	if( m_sBackgroundPath != "" )
	{
		splitrelpath( m_sBackgroundPath, sDir, sFName, sExt );
		file.WriteString( ssprintf("#BACKGROUND:%s;\n", sFName+"."+sExt) );
	}
	else
		file.WriteString( "#BACKGROUND:;\n" );

	if( m_sBackgroundMoviePath != "" )
	{
		splitrelpath( m_sBackgroundMoviePath, sDir, sFName, sExt );
		file.WriteString( ssprintf("#BACKGROUNDMOVIE:%s;\n", sFName+"."+sExt) );
	}
	else
		file.WriteString( "#BACKGROUNDMOVIE:;\n" );

	if( m_sCDTitlePath != "" )
	{
		splitrelpath( m_sCDTitlePath, sDir, sFName, sExt );
		file.WriteString( ssprintf("#CDTITLE:%s;\n", sFName+"."+sExt) );
	}
	else
		file.WriteString( "#CDTITLE:;\n" );

	if( m_sMusicPath != "" )
	{
		splitrelpath( m_sMusicPath, sDir, sFName, sExt );
		file.WriteString( ssprintf("#MUSIC:%s;\n", sFName+"."+sExt) );
	}
	else
		file.WriteString( "#MUSIC:;\n" );

	file.WriteString( ssprintf("#MUSICBYTES:%u;\n", m_iMusicBytes) );
	file.WriteString( ssprintf("#OFFSET:%f;\n", m_fOffsetInSeconds) );

	file.WriteString( ssprintf("#SAMPLESTART:%f;\n", m_fMusicSampleStartSeconds) );
	file.WriteString( ssprintf("#SAMPLELENGTH:%f;\n", m_fMusicSampleLengthSeconds) );

	file.WriteString( "#FREEZES:" );
	for( i=0; i<m_FreezeSegments.GetSize(); i++ )
	{
		FreezeSegment &fs = m_FreezeSegments[i];

		file.WriteString( ssprintf("%f=%f", fs.m_fStartBeat, fs.m_fFreezeSeconds) );
		if( i != m_FreezeSegments.GetSize()-1 )
			file.WriteString( "," );
	}
	file.WriteString( ";\n" );
	
	file.WriteString( "#BPMS:" );
	for( i=0; i<m_BPMSegments.GetSize(); i++ )
	{
		BPMSegment &bs = m_BPMSegments[i];

		file.WriteString( ssprintf("%f=%f", bs.m_fStartBeat, bs.m_fBPM) );
		if( i != m_BPMSegments.GetSize()-1 )
			file.WriteString( "," );
	}
	file.WriteString( ";\n" );
	
	file.Close();
			
	//
	// Save all Notess for this file
	//
	for( i=0; i<m_arrayNotes.GetSize(); i++ ) 
	{
		m_arrayNotes[i].SaveToSMDir( m_sSongDir );
	}

}

Grade Song::GetGradeForDifficultyClass( NotesType nt, DifficultyClass dc )
{
	CArray<Notes*, Notes*> arrayNotess;
	this->GetNotesThatMatch( nt, arrayNotess );
	SortNotesArrayByDifficultyClass( arrayNotess );

	for( int i=0; i<arrayNotess.GetSize(); i++ )
	{
		Notes* pNotes = arrayNotess[i];
		if( pNotes->m_DifficultyClass == dc )
			return pNotes->m_TopGrade;
	}
	return GRADE_NO_DATA;
}




/////////////////////////////////////
// Sorting
/////////////////////////////////////

int CompareSongPointersByTitle(const void *arg1, const void *arg2)
{
	Song* pSong1 = *(Song**)arg1;
	Song* pSong2 = *(Song**)arg2;
	
	CString sTitle1 = pSong1->GetMainTitle();
	CString sTitle2 = pSong2->GetMainTitle();

	CString sFilePath1 = pSong1->GetSongFilePath();		// this is unique among songs
	CString sFilePath2 = pSong2->GetSongFilePath();

	CString sCompareString1 = sTitle1 + sFilePath1;
	CString sCompareString2 = sTitle2 + sFilePath2;

	return CompareCStringsAsc( (void*)&sCompareString1, (void*)&sCompareString2 );
}

void SortSongPointerArrayByTitle( CArray<Song*, Song*> &arraySongPointers )
{
	qsort( arraySongPointers.GetData(), arraySongPointers.GetSize(), sizeof(Song*), CompareSongPointersByTitle );
}

int CompareSongPointersByBPM(const void *arg1, const void *arg2)
{
	Song* pSong1 = *(Song**)arg1;
	Song* pSong2 = *(Song**)arg2;
		
	float fMinBPM1, fMaxBPM1, fMinBPM2, fMaxBPM2;
	pSong1->GetMinMaxBPM( fMinBPM1, fMaxBPM1 );
	pSong2->GetMinMaxBPM( fMinBPM2, fMaxBPM2 );

	CString sFilePath1 = pSong1->GetSongFilePath();		// this is unique among songs
	CString sFilePath2 = pSong2->GetSongFilePath();

	if( fMaxBPM1 < fMaxBPM2 )
		return -1;
	else if( fMaxBPM1 == fMaxBPM2 )
		return CompareCStringsAsc( (void*)&sFilePath1, (void*)&sFilePath2 );
	else
		return 1;
}

void SortSongPointerArrayByBPM( CArray<Song*, Song*> &arraySongPointers )
{
	qsort( arraySongPointers.GetData(), arraySongPointers.GetSize(), sizeof(Song*), CompareSongPointersByBPM );
}


int CompareSongPointersByArtist(const void *arg1, const void *arg2)
{
	Song* pSong1 = *(Song**)arg1;
	Song* pSong2 = *(Song**)arg2;
		
	CString sArtist1 = pSong1->GetArtist();
	CString sArtist2 = pSong2->GetArtist();

	if( sArtist1 < sArtist2 )
		return -1;
	else if( sArtist1 == sArtist2 )
		return CompareSongPointersByTitle( arg1, arg2 );
	else
		return 1;
}

void SortSongPointerArrayByArtist( CArray<Song*, Song*> &arraySongPointers )
{
	qsort( arraySongPointers.GetData(), arraySongPointers.GetSize(), sizeof(Song*), CompareSongPointersByArtist );
}

int CompareSongPointersByGroup(const void *arg1, const void *arg2)
{
	Song* pSong1 = *(Song**)arg1;
	Song* pSong2 = *(Song**)arg2;
		
	CString sGroup1 = pSong1->GetGroupName();
	CString sGroup2 = pSong2->GetGroupName();

	if( sGroup1 < sGroup2 )
		return -1;
	else if( sGroup1 == sGroup2 )
		return CompareSongPointersByTitle( arg1, arg2 );
	else
		return 1;
}

void SortSongPointerArrayByGroup( CArray<Song*, Song*> &arraySongPointers )
{
	qsort( arraySongPointers.GetData(), arraySongPointers.GetSize(), sizeof(Song*), CompareSongPointersByGroup );
}

int CompareSongPointersByMostPlayed(const void *arg1, const void *arg2)
{
	Song* pSong1 = *(Song**)arg1;
	Song* pSong2 = *(Song**)arg2;
		
	int iNumTimesPlayed1 = pSong1->GetNumTimesPlayed();
	int iNumTimesPlayed2 = pSong2->GetNumTimesPlayed();

	if( iNumTimesPlayed1 < iNumTimesPlayed2 )
		return -1;
	else if( iNumTimesPlayed1 == iNumTimesPlayed2 )
		return CompareSongPointersByTitle( arg1, arg2 );
	else
		return 1;
}

void SortSongPointerArrayByMostPlayed( CArray<Song*, Song*> &arraySongPointers )
{
	qsort( arraySongPointers.GetData(), arraySongPointers.GetSize(), sizeof(Song*), CompareSongPointersByMostPlayed );
}

