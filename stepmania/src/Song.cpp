#include "stdafx.h"
/*
-----------------------------------------------------------------------------
 File: Song.h

 Desc: Holds metadata for a song and the song's step data.

 Copyright (c) 2001-2002 by the persons listed below.  All rights reserved.
-----------------------------------------------------------------------------
*/


#include "NoteMetadata.h"
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
	m_dwMusicBytes = 0;
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


// This is a super hack, but it's only called from WindowEdit, so it's OK :-)
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

bool Song::LoadFromSongDir( CString sDir )
{
	LOG->WriteLine( "Song::LoadFromSongDir(%s)", sDir );

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

	CStringArray arraySongFileNames;
	GetDirListing( sDir + CString("*.song"), arraySongFileNames );
	int iNumSongFiles = arraySongFileNames.GetSize();


	if( iNumSongFiles > 1 )
		FatalError( "There is more than one DWI file in '%s'.  Which one should I use?", sDir );
	else if( iNumDWIFiles > 1 )
		FatalError( "There is more than one DWI file in '%s'.  Which one should I use?", sDir );
	else if( iNumSongFiles == 1 )
		LoadFromSMDir( sDir );
	else if( iNumDWIFiles == 1 )
		LoadFromDWIFile( sDir + arrayDWIFileNames[0] );
	else if( iNumBMSFiles > 0 )
		LoadFromBMSDir( sDir );
	else 
		FatalError( "Couldn't find any Song, BMS, or MSD files in '%s'", sDir );

	return TRUE;
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

	// load the NoteMetadata from the rest of the BMS files
	for( int i=0; i<arrayBMSFileNames.GetSize(); i++ ) 
	{
		m_arrayNoteMetadatas.SetSize( m_arrayNoteMetadatas.GetSize()+1 );
		NoteMetadata &new_steps = m_arrayNoteMetadatas[ m_arrayNoteMetadatas.GetSize()-1 ];
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
			// strip NoteMetadata type out of description leaving only song title (looks like 'B4U <BASIC>')
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
						fBeatOffset = NoteIndexToBeat( (float)iStepIndex );
						float fBPS;
						fBPS = m_BPMSegments[0].m_fBPM/60.0f;
						m_fOffsetInSeconds = fBeatOffset / fBPS;
						//LOG->WriteLine( "Found offset to be index %d, beat %f", iStepIndex, NoteIndexToBeat(iStepIndex) );
						break;
					case 03:	// bpm
						BPMSegment new_seg;
						new_seg.m_fStartBeat = NoteIndexToBeat( (float)iStepIndex );
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



	TidyUpData();

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
	if( !file.Open( GetSongFilePath(), CFile::modeRead|CFile::shareDenyNone ) )
		FatalError( ssprintf("Error opening DWI file '%s'.", GetSongFilePath()) );


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

		if( arrayValueTokens.GetSize() == 0 )
			continue;

		CString sValueName = arrayValueTokens.GetAt( 0 );
		sValueName.TrimLeft();

		// handle the data
		if( sValueName == "#FILE" )
			m_sMusicPath = CString("DWI Support\\") + arrayValueTokens[1];

		else if( sValueName == "#TITLE" )
		{
			GetMainAndSubTitlesFromFullTitle( arrayValueTokens[1], m_sMainTitle, m_sSubTitle );
		}
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
				float fFreezeBeat = NoteIndexToBeat( fIndex );
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
				float fBeat = NoteIndexToBeat( fIndex );
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
			m_arrayNoteMetadatas.SetSize( m_arrayNoteMetadatas.GetSize()+1, 1 );
			NoteMetadata &new_NoteMetadata = m_arrayNoteMetadatas[ m_arrayNoteMetadatas.GetSize()-1 ];
			new_NoteMetadata.LoadFromDWITokens( 
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

	TidyUpData();


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


	CStringArray arrayNoteMetadataFileNames;
	GetDirListing( sDir + CString("*.notes"), arrayNoteMetadataFileNames );

	// load the NoteMetadata from the rest of the BMS files
	for( i=0; i<arrayNoteMetadataFileNames.GetSize(); i++ ) 
	{
		m_arrayNoteMetadatas.SetSize( m_arrayNoteMetadatas.GetSize()+1 );
		NoteMetadata &new_steps = m_arrayNoteMetadatas[ m_arrayNoteMetadatas.GetSize()-1 ];
		new_steps.LoadFromNotesFile( m_sSongDir + arrayNoteMetadataFileNames[i] );
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
			m_dwMusicBytes = atoi( arrayValueTokens[1] );

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

	TidyUpData();

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
}


void Song::GetNoteMetadatasThatMatchGameAndStyle( CString sGame, CString sStyle, CArray<NoteMetadata*, NoteMetadata*>& arrayAddTo )
{
	for( int i=0; i<m_arrayNoteMetadatas.GetSize(); i++ )	// for each of the Song's NoteMetadata
	{
		NoteMetadata* pCurNoteMetadata = &m_arrayNoteMetadatas[i];
		
		if( sGame == pCurNoteMetadata->m_sGame && 
			( sStyle == pCurNoteMetadata->m_sStyle || (sStyle == "versus" && pCurNoteMetadata->m_sStyle == "single") ) )
		{
			arrayAddTo.Add( pCurNoteMetadata );
		}
	}

}


/*
void Song::SaveOffsetChangeToDisk()
{
	CString sDir, sFName, sExt;
	splitrelpath( GetSongFilePath(), sDir, sFName, sExt );
	sExt.MakeLower();

	if( sExt == "bms" )
	{
		for( int i=0; i<m_arrayNoteMetadatas.GetSize(); i++ )		// for each NoteMetadata
		{
			CString sNoteMetadataFileName = m_arrayNoteMetadatas[i].m_sNoteMetadataFilePath;

			CStringArray arrayLines;
			CStdioFile fileIn;
			if( !fileIn.Open( sNoteMetadataFileName, CFile::modeRead|CFile::shareDenyNone) )
				FatalError( ssprintf("Failed to open '%s' for reading", sNoteMetadataFileName) );

			// read the file into sFileContents
			CString sLine;
			while( fileIn.ReadString(sLine) )
			{
				arrayLines.Add( sLine+"\n" );
			}
			fileIn.Close();

			// write the file back to disk with the changes
			CStdioFile fileOut;
			if( !fileOut.Open( sNoteMetadataFileName, CFile::modeWrite|CFile::shareDenyWrite) )
				FatalError( ssprintf("Failed to open '%s' for writing", sNoteMetadataFileName) );
			for( int j=0; j<arrayLines.GetSize(); j++ )
			{
				CString sLine = arrayLines[j];
				if( -1 != sLine.Find("01:") )
				{
					float fBPM = m_BPMSegments[0].m_fBPM;
					float fBPS = fBPM/60;
					float fOffsetInBeats = m_fOffsetInSeconds * fBPS;
					float fOffsetInMeasures = fOffsetInBeats / BEATS_PER_MEASURE;
					int   iMeasure = (int)fOffsetInMeasures;
					float fPercentIntoMeasure = fmodf( fOffsetInMeasures, 1 );
					CString sBMSMeasureString;
					for( int k=0; k<64; k++ )
					{
						if( k == int(fPercentIntoMeasure*64) )
							sBMSMeasureString += "99";
						else
							sBMSMeasureString += "00";
					}

					fileOut.WriteString( ssprintf("#%03d01:%s;\n", iMeasure, sBMSMeasureString) );
				}
				else
				{
					fileOut.WriteString( sLine );
				}
			}
			fileOut.Close();
		}
		
	}
	else if( sExt == "dwi" )
	{
		CStringArray arrayLines;
		CStdioFile fileIn;
		if( !fileIn.Open( GetSongFilePath(), CFile::modeRead|CFile::shareDenyNone) )
			FatalError( ssprintf("Failed to open '%s' for reading", GetSongFilePath()) );

		// read the file into sFileContents
		CString sLine;
		while( fileIn.ReadString(sLine) )
		{
			arrayLines.Add( sLine+"\n" );
		}
		fileIn.Close();


		// write the file back to disk with the changes
		CStdioFile fileOut;
		if( !fileOut.Open( GetSongFilePath(), CFile::modeWrite|CFile::shareDenyWrite) )
			FatalError( ssprintf("Failed to open '%s' for writing", GetSongFilePath()) );
		for( int i=0; i<arrayLines.GetSize(); i++ )
		{
			CString sLine = arrayLines[i];
			if( -1 != sLine.Find("#GAP") )
			{
				// Discover whether the GAP line has a semicolon at the end.  
				// If it doesn't then the semicolon is probably on the next line
				// and we don't want to write a second semicolon!
				bool bHasSemiColon = -1 != sLine.Find(";");

				// replace with new offset
				fileOut.WriteString( ssprintf("#GAP:%d%s\n", roundf(GetBeatOffsetInSeconds()*-1*1000), bHasSemiColon ? ";" : "") );
			}
			else
			{
				fileOut.WriteString( sLine );
			}
		}
		fileOut.Close();


	}
	else
	{
		FatalError( ssprintf("Unrecognized extension '%s' on song file '%s'", sExt, GetSongFilePath()) );
	}


	m_bChangedSinceSave = false;
}
*/
void Song::Save()
{
	LOG->WriteLine( "Song::Save()" );

	int i;

	if( -1 != m_sSongDir.Find("DWI Support") )
	{
		LOG->WriteLine("WARNING:  Can't save to DWI support folder!");
		return;
	}

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

	file.WriteString( ssprintf("#MUSICBYTES:%u;\n", m_dwMusicBytes) );
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
	// Save all NoteMetadatas for this file
	//
	for( i=0; i<m_arrayNoteMetadatas.GetSize(); i++ ) 
	{
		m_arrayNoteMetadatas[i].Save( m_sSongDir );
	}

}

Grade Song::GetGradeForDifficultyClass( CString sGame, CString sStyle, DifficultyClass dc )
{
	CArray<NoteMetadata*, NoteMetadata*> arrayNoteMetadatas;
	this->GetNoteMetadatasThatMatchGameAndStyle( sGame, sStyle, arrayNoteMetadatas );
	SortNoteMetadataArrayByDifficultyClass( arrayNoteMetadatas );

	for( int i=0; i<arrayNoteMetadatas.GetSize(); i++ )
	{
		NoteMetadata* pNoteMetadata = arrayNoteMetadatas[i];
		if( pNoteMetadata->m_DifficultyClass == dc )
			return pNoteMetadata->m_TopGrade;
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

