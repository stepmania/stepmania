#include "stdafx.h"
/*
-----------------------------------------------------------------------------
 Class: Song

 Desc: Holds metadata for a song and the song's step data.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "Notes.h"
#include "RageUtil.h"
#include <math.h>	// for fmod
#include "RageLog.h"
#include "IniFile.h"
#include "Song.h"


const int FILE_CACHE_VERSION = 25;	// increment this when the SM file format changes


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
	m_fMusicSampleStartSeconds = 0;
	m_fMusicSampleLengthSeconds = 16;	// start fading out at m_fMusicSampleLengthSeconds-1 seconds
	m_iMusicBytes = 0;
	m_fMusicLengthSeconds = 0;
}

Song::~Song()
{
	for( int i=0; i<m_arrayNotes.GetSize(); i++ )
		SAFE_DELETE( m_arrayNotes[i] );

	m_arrayNotes.RemoveAll();
}


void Song::AddBPMSegment( BPMSegment seg )
{
	m_BPMSegments.Add( seg );
	SortBPMSegmentsArray( m_BPMSegments );
}


void Song::AddFreezeSegment( FreezeSegment seg )
{
	m_FreezeSegments.Add( seg );
	SortFreezeSegmentsArray( m_FreezeSegments );
}


void Song::GetBeatAndBPSFromElapsedTime( float fElapsedTime, float &fBeatOut, float &fBPSOut, bool &bFreezeOut )
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
			// this BPMSegement is NOT the current segment
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
							bFreezeOut = true;
							return;
						}
					}
					else
						break;
				}
			}

			fBeatOut = fBeatEstimate;
			fBPSOut = fBPS;
			bFreezeOut = false;
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
	bool bFreezeOut;

	while( fSecondsToMove > 0.1f )
	{
		GetBeatAndBPSFromElapsedTime( fElapsedTimeBestGuess, fBeatOut, fBPSOut, bFreezeOut );
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
	char sLeftSeps[]  = { '-', '~', '(', '[' };
	char sRightSeps[] = { '-', '~', ')', ']' };
	ASSERT( sizeof(sLeftSeps) == sizeof(sRightSeps) );
	for( int i=0; i<sizeof(sLeftSeps); i++ )
	{
		int iBeginIndex = sFullTitle.Find( sLeftSeps[i] );
		if( iBeginIndex == -1 )
			continue;
		int iEndIndex = sFullTitle.Find( sRightSeps[i], iBeginIndex+1 );	
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
	return ssprintf( "Cache\\%d", GetHashForString(m_sSongDir) );
}

CString Song::GetSongFilePath()
{
	CStringArray asFileNames;
	GetDirListing( m_sSongDir+"*.sm", asFileNames );
	if( asFileNames.GetSize() > 0 )
		return m_sSongDir + asFileNames[0];
	else
		return m_sSongDir + GetFullTitle() + ".sm";
}

bool Song::LoadFromSongDir( CString sDir )
{
	LOG->WriteLine( "Song::LoadFromSongDir(%s)", sDir );

	// make sure there is a trailing '\\' at the end of sDir
	if( sDir.Right(1) != "\\" )
		sDir += "\\";

	// save song dir
	m_sSongDir = sDir;

	// save group name
	CStringArray sDirectoryParts;
	split( m_sSongDir, "\\", sDirectoryParts, true );
	m_sGroupName = sDirectoryParts[1];


	//
	// First look in the cache for this song (without loading NoteData)
	//
	IniFile ini;
	ini.SetPath( "Cache\\index.cache" );
	if( !ini.ReadFile() )
		goto load_without_cache;

	int iCacheVersion;
	ini.GetValueI( "Cache", "CacheVersion", iCacheVersion );
	if( iCacheVersion != FILE_CACHE_VERSION )
	{
		LOG->WriteLine( "Cache format is out of date.  Deleting all cache files" );
		CStringArray asCacheFileNames;
		GetDirListing( "Cache\\*.*", asCacheFileNames );
		for( int i=0; i<asCacheFileNames.GetSize(); i++ )
			DeleteFile( "Cache\\" + asCacheFileNames[i] );
		goto load_without_cache;
	}

	int iDirHash;
	ini.GetValueI( "Cache", m_sSongDir, iDirHash );
	if( GetHashForDirectory(m_sSongDir) == iDirHash )	// this cache is up to date
	{
		if( !DoesFileExist(GetCacheFilePath()) )
			goto load_without_cache;
		LoadFromSMFile( GetCacheFilePath(), false );	// don't load NoteData

		// Save length of music
		if( GetMusicPath() != "" )
		{
			RageSoundStream sound;
			sound.Load( GetMusicPath() );
			m_fMusicLengthSeconds = sound.GetLengthSeconds();
		}

		LOG->WriteLine( "Loading '%s' from cache file '%s'.", m_sSongDir, GetCacheFilePath() );
		return true;
	}

load_without_cache:
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

	CStringArray arraySMFileNames;
	GetDirListing( sDir + CString("*.sm"), arraySMFileNames );
	int iNumSMFiles = arraySMFileNames.GetSize();


	if( iNumSMFiles > 1 )
		throw RageException( "There is more than one SM file in '%s'.  There should be only one!", sDir );
	else if( iNumDWIFiles > 1 )
		throw RageException( "There is more than one DWI file in '%s'.  There should be only one!", sDir );
	else if( iNumSMFiles == 1 )
		LoadFromSMFile( sDir + arraySMFileNames[0], true );	// do load note data
	else if( iNumDWIFiles == 1 )
		LoadFromDWIFile( sDir + arrayDWIFileNames[0] );
	else if( iNumBMSFiles > 0 )
		LoadFromBMSDir( sDir );
	else 
		throw RageException( "Couldn't find any SM, BMS, or DWI files in '%s'.  This is not a valid song directory.", sDir );

	TidyUpData();

	//
	// In order to save memory, we're going to save the file back to the cache, 
	// then unload all the large NoteData. 
	//
	SaveToCacheFile();
	LoadFromSMFile( GetCacheFilePath(), false );	// don't load NoteData

	return true;
}



bool Song::LoadFromBMSDir( CString sDir )
{
	LOG->WriteLine( "Song::LoadFromBMSDir(%s)", sDir );

	CStringArray arrayBMSFileNames;
	GetDirListing( sDir + CString("*.bms"), arrayBMSFileNames );

	if( arrayBMSFileNames.GetSize() == 0 )
		throw RageException( "Couldn't find any BMS files in '%s'", sDir );

	// load the Notes from the rest of the BMS files
	for( int i=0; i<arrayBMSFileNames.GetSize(); i++ ) 
	{
		Notes* pNewNotes = new Notes;
		pNewNotes->LoadFromBMSFile( m_sSongDir + arrayBMSFileNames[i] );
		m_arrayNotes.Add( pNewNotes );
	}

	CString sPath = m_sSongDir + arrayBMSFileNames[0];

	CStdioFile file;	
	if( !file.Open( sPath, CFile::modeRead|CFile::shareDenyNone ) )
	{
		throw RageException( "Failed to open %s.", sPath );
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
		if( value_name == "#title" ) 
		{
			// strip Notes type out of description leaving only song title - looks like 'B4U <BASIC>'
			int iIndex = value_data.ReverseFind('<');
			if( iIndex == -1 )
				iIndex = value_data.ReverseFind('(');
			if( iIndex != -1 )
			{
				value_data = value_data.Left( iIndex );
				GetMainAndSubTitlesFromFullTitle( value_data, m_sMainTitle, m_sSubTitle );
			}
			else
				m_sMainTitle = value_data;
		}
		else if( value_name == "#artist" ) 
		{
			m_sArtist = value_data;
		}
		else if( value_name == "#bpm" ) 
		{
			BPMSegment newSeg( 0, (float)atof(value_data) );
			AddBPMSegment( newSeg );
		
			LOG->WriteLine( "Inserting new BPM change at beat %f, BPM %f", newSeg.m_fStartBeat, newSeg.m_fBPM );
		}
		else if( value_name == "#backbmp" ) 
		{
			m_sBackgroundFile = value_data;
		}
		else if( value_name == "#wav" ) 
		{
			m_sMusicFile = value_data;
		}
		else if( value_name.Left(1) == "#"  
			 && IsAnInt( value_name.Mid(1,3) )
			 && IsAnInt( value_name.Mid(4,2) ) )	// this is step or offset data.  Looks like "#00705"
		{
			int iMeasureNo	= atoi( value_name.Mid(1,3) );
			int iBMSTrackNo	= atoi( value_name.Mid(4,2) );

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
			//LOG->WriteLine( "%s:%s: iMeasureNo = %d, iBMSTrackNo = %d, iNumNotesInThisMeasure = %d", 
			//	valuename, sNoteData, iMeasureNo, iBMSTrackNo, iNumNotesInThisMeasure );
			for( int j=0; j<iNumNotesInThisMeasure; j++ )
			{
				if( arrayNotes[j] != 0 )
				{
					float fPercentThroughMeasure = (float)j/(float)iNumNotesInThisMeasure;

					// index is in quarter beats starting at beat 0
					int iStepIndex = (int) ( (iMeasureNo + fPercentThroughMeasure)
									 * BEATS_PER_MEASURE * ELEMENTS_PER_BEAT );

					switch( iBMSTrackNo )
					{
					case 1:	// background music track
						float fBeatOffset;
						fBeatOffset = NoteRowToBeat( (float)iStepIndex );
						float fBPS;
						fBPS = m_BPMSegments[0].m_fBPM/60.0f;
						m_fOffsetInSeconds = fBeatOffset / fBPS;
						//LOG->WriteLine( "Found offset to be index %d, beat %f", iStepIndex, NoteRowToBeat(iStepIndex) );
						break;
					case 3:	// bpm change
						{
							BPMSegment newSeg( NoteRowToBeat(iStepIndex), (float)arrayNotes[j] );
							AddBPMSegment( newSeg );
							LOG->WriteLine( "Inserting new BPM change at beat %f, BPM %f", newSeg.m_fStartBeat, newSeg.m_fBPM );
						}
						break;

						// Let me just take a moment to express how frustrated I am with the new, 
						// poorly-designed changes to the BMS format.
						//
						//
						// AAAAAAAAAAAAAAAAAAAAAAAAAAAAAaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaahhhhhhhhhhhhhhhhhhhhhhhh!!!!!!!!!!!!!!!
						//
						// Thank you.

					case 8:	// indirect bpm
						{
							// This is a very inefficient way to parse, but it doesn't matter much
							// because this is only parsed on the first run after the song is installed.
							CString sTagToLookFor = ssprintf( "#BPM%02x", arrayNotes[j] );
							float fBPM = -1;


							// open the song file again and and look for this tag's value
							CStdioFile file;	
							if( !file.Open( sPath, CFile::modeRead|CFile::shareDenyNone ) )
							{
								throw RageException( "Failed to open %s.", sPath );
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

								if( stricmp(value_name, sTagToLookFor) == 0 )
								{
									fBPM = (float)atof( value_data );
									break;
								}
							}

							if( fBPM == -1 )	// we didn't find the line we were looking for
							{
								LOG->WriteLine( "WARNING:  Couldn't find tag '%s' in '%s'.", sTagToLookFor, sPath );
							}
							else
							{
								BPMSegment newSeg( NoteRowToBeat(iStepIndex), fBPM );
								AddBPMSegment( newSeg );
								LOG->WriteLine( "Inserting new BPM change at beat %f, BPM %f", newSeg.m_fStartBeat, newSeg.m_fBPM );
							}

							file.Close();
						}
						break;
					case 9:	// freeze
						{
							// This is a very inefficient way to parse, but it doesn't 
							// matter much because this is only parsed on the first run after the song is installed.
							CString sTagToLookFor = ssprintf( "#STOP%02x", arrayNotes[j] );
							float fFreezeStartBeat = NoteRowToBeat(iStepIndex);
							float fFreezeSecs = -1;


							// open the song file again and and look for this tag's value
							CStdioFile file;	
							if( !file.Open( sPath, CFile::modeRead|CFile::shareDenyNone ) )
								throw RageException( "Failed to open %s.", sPath );

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

								if( stricmp(value_name, sTagToLookFor) == 0 )
								{
									// find the BPM at the time of this freeze
									float fBPM = -1;
									for( int i=0; i<m_BPMSegments.GetSize()-1; i++ )
									{
										if( m_BPMSegments[i].m_fStartBeat <= fFreezeStartBeat && 
											m_BPMSegments[i+1].m_fStartBeat > fFreezeStartBeat )
										{
											fBPM = m_BPMSegments[i].m_fBPM;
											break;
										}
									}
									// the BPM segment of this beat is the last BPM segment
									if( fBPM == -1 )
										fBPM = m_BPMSegments[m_BPMSegments.GetSize()-1].m_fBPM;

									fFreezeSecs = (float)atof(value_data)/(fBPM*0.81f);	// I have no idea what units these are in, so I experimented until finding this factor.
									break;
								}
							}

							if( fFreezeSecs == -1 )	// we didn't find the line we were looking for
							{
								LOG->WriteLine( "WARNING:  Couldn't find tag '%s' in '%s'.", sTagToLookFor, sPath );
							}
							else
							{
								FreezeSegment newSeg( fFreezeStartBeat, fFreezeSecs );
								AddFreezeSegment( newSeg );
								LOG->WriteLine( "Inserting new Freeze at beat %f, secs %f", newSeg.m_fStartBeat, newSeg.m_fFreezeSeconds );
							}

							file.Close();
						}
						break;
					}
				}
			}
		}
	}

	for( i=0; i<m_BPMSegments.GetSize(); i++ )
		LOG->WriteLine( "There is a BPM change at beat %f, BPM %f, index %d", 
					m_BPMSegments[i].m_fStartBeat, m_BPMSegments[i].m_fBPM, i );

	file.Close();

	return TRUE;
}


bool Song::LoadFromDWIFile( CString sPath )
{
	LOG->WriteLine( "Song::LoadFromDWIFile(%s)", sPath );
	

	CStdioFile file;	
	if( !file.Open( sPath, CFile::modeRead|CFile::shareDenyNone ) )
		throw RageException( "Error opening DWI file '%s'.", sPath );


	// read the whole file into a sFileText
	CString sFileText;
	CString buffer;
	while( file.ReadString(buffer) )
		sFileText += buffer + "\n";
	file.Close();

	// strip comments out of sFileText
	while( sFileText.Find("//") != -1 )
	{
		int iIndexCommentStart = sFileText.Find("//");
		int iIndexCommentEnd = sFileText.Find("\n", iIndexCommentStart);
		if( iIndexCommentEnd == -1 )	// comment doesn't have an end?
			sFileText.Delete( iIndexCommentStart, 2 );
		else
			sFileText.Delete( iIndexCommentStart, iIndexCommentEnd-iIndexCommentStart );
	}


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
		for( int j=0; j<arrayValueTokens.GetSize(); j++ )
		{
			arrayValueTokens[j].TrimLeft();
			arrayValueTokens[j].TrimRight();
		}
		if( arrayValueTokens.GetSize() == 0 )
			continue;

		CString sValueName = arrayValueTokens.GetAt( 0 );

		// handle the data
		if( sValueName == "#FILE" )
			m_sMusicFile = arrayValueTokens[1];

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
				
				AddFreezeSegment( FreezeSegment(fFreezeBeat, fFreezeSeconds) );
				LOG->WriteLine( "Adding a freeze segment: beat: %f, seconds = %f", fFreezeBeat, fFreezeSeconds );
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
				
				AddBPMSegment( BPMSegment(fBeat, fNewBPM) );
			}
		}

		else if( sValueName == "#SINGLE" || sValueName == "#DOUBLE" || sValueName == "#COUPLE" )
		{
			Notes* pNewNotes = new Notes;
			pNewNotes->LoadFromDWITokens( 
				arrayValueTokens[0], 
				arrayValueTokens[1], 
				atoi(arrayValueTokens[2]), 
				arrayValueTokens[3], 
				arrayValueTokens.GetSize() == 5 ? arrayValueTokens[4] : "" 
				);
			m_arrayNotes.Add( pNewNotes );
		}
		else
			// do nothing.  We don't care about this value name
			;
	}

	return TRUE;
}


bool Song::LoadFromSMFile( CString sPath, bool bLoadNoteData )
{
	LOG->WriteLine( "Song::LoadFromSMDir(%s, %d)", sPath, bLoadNoteData );

	m_BPMSegments.RemoveAll();
	m_FreezeSegments.RemoveAll();

	int i;

	// get group name
	CStringArray sDirectoryParts;
	split( m_sSongDir, "\\", sDirectoryParts, true );
	m_sGroupName = sDirectoryParts[1];


	CStdioFile file;	
	if( !file.Open( sPath, CFile::modeRead|CFile::shareDenyNone ) )
		throw RageException( "Error opening SM file '%s'.", sPath );


	// read the whole file into a sFileText
	CString sFileText;
	CString buffer;
	while( file.ReadString(buffer) )
		sFileText += buffer + "\n";
	file.Close();

	// strip comments out of sFileText
	while( sFileText.Find("//") != -1 )
	{
		int iIndexCommentStart = sFileText.Find("//");
		int iIndexCommentEnd = sFileText.Find("\n", iIndexCommentStart);
		if( iIndexCommentEnd == -1 )	// comment doesn't have an end?
			sFileText.Delete( iIndexCommentStart, 2 );
		else
			sFileText.Delete( iIndexCommentStart, iIndexCommentEnd-iIndexCommentStart );
	}

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
		for( int j=0; j<arrayValueTokens.GetSize(); j++ )
		{
			arrayValueTokens[j].TrimLeft();
			arrayValueTokens[j].TrimRight();
		}
		if( arrayValueTokens.GetSize() == 0 )
			continue;

		CString sValueName = arrayValueTokens.GetAt( 0 );

		// handle the data
		if( sValueName == "#TITLE" )
			GetMainAndSubTitlesFromFullTitle( arrayValueTokens[1], m_sMainTitle, m_sSubTitle );

		else if( sValueName == "#SUBTITLE" )
			m_sSubTitle = arrayValueTokens[1];

		else if( sValueName == "#ARTIST" )
			m_sArtist = arrayValueTokens[1];

		else if( sValueName == "#CREDIT" )
			m_sCredit = arrayValueTokens[1];

		else if( sValueName == "#BANNER" )
			m_sBannerFile = arrayValueTokens[1];

		else if( sValueName == "#BACKGROUND" )
			m_sBackgroundFile = arrayValueTokens[1];

		else if( sValueName == "#BACKGROUNDMOVIE" )
			m_sBackgroundMovieFile = arrayValueTokens[1];

		else if( sValueName == "#CDTITLE" )
			m_sCDTitleFile = arrayValueTokens[1];

		else if( sValueName == "#MUSIC" )
			m_sMusicFile = arrayValueTokens[1];

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

				AddFreezeSegment( new_seg );
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
				
				AddBPMSegment( new_seg );
			}
		}

		else if( sValueName == "#NOTES" )
		{
			Notes* pNewNotes = NULL;
			// Check to see if this notes already has been allocated
			for( int j=0; j<m_arrayNotes.GetSize(); j++ )
				if( m_arrayNotes[j]->m_NotesType == StringToNotesType(arrayValueTokens[1])  &&
					m_arrayNotes[j]->m_sDescription == arrayValueTokens[2] )
					pNewNotes = m_arrayNotes[j];

			if( pNewNotes == NULL )	// we didn't find a match that was already loaded
			{
				pNewNotes = new Notes;
				m_arrayNotes.Add( pNewNotes );
			}

			if( arrayValueTokens.GetSize() != 8 )
				throw RageException( "The song file '%s' is has %d in a #NOTES tag, but should have %d.", sPath, arrayValueTokens.GetSize(), 8 );

			pNewNotes->LoadFromSMTokens( 
				arrayValueTokens[1], 
				arrayValueTokens[2], 
				arrayValueTokens[3], 
				arrayValueTokens[4], 
				arrayValueTokens[5], 
				arrayValueTokens[6], 
				arrayValueTokens[7], 
 				bLoadNoteData );
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
		throw RageException( "No #BPM specified in '%s.'", GetSongFilePath() );

	if( m_sMusicFile == "" || !DoesFileExist(GetMusicPath()) )
	{
		CStringArray arrayPossibleMusic;
		GetDirListing( m_sSongDir + CString("*.mp3"), arrayPossibleMusic );
		GetDirListing( m_sSongDir + CString("*.ogg"), arrayPossibleMusic );
		GetDirListing( m_sSongDir + CString("*.wav"), arrayPossibleMusic );

		if( arrayPossibleMusic.GetSize() != 0 )		// we found a match
			m_sMusicFile = arrayPossibleMusic[0];
		else
			throw RageException( "The song in '%s' is missing a music file.  You must place a music file in the song folder or remove the song", m_sSongDir );
	}

	// Save length of music
	if( GetMusicPath() != "" )
	{
		RageSoundStream sound;
		sound.Load( GetMusicPath() );
		m_fMusicLengthSeconds = sound.GetLengthSeconds();
	}

	if( !DoesFileExist(GetBannerPath()) )
	{
		m_sBannerFile = "";
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
				sSmallestFileSoFar = arrayPossibleBanners[i];
			}
		}

		if( sSmallestFileSoFar != "" )		// we found a match
			m_sBannerFile = sSmallestFileSoFar;
		else
			m_sBannerFile = "";

		//throw RageException( ssprintf("Banner could not be found.  Please check the Song file '%s' and verify the specified #BANNER exists.", GetSongFilePath()) );
	}

	if( !DoesFileExist(GetBackgroundPath()) )
	{
		m_sBackgroundFile = "";
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
				sLargestFileSoFar = arrayPossibleBackgrounds[i];
			}
		}
		
		if( sLargestFileSoFar != "" )		// we found a match
			m_sBackgroundFile = sLargestFileSoFar;
	}

	if( !DoesFileExist(GetBackgroundMoviePath()) )
	{
		m_sBackgroundMovieFile = "";

		CStringArray arrayPossibleBackgroundMovies;
		GetDirListing( m_sSongDir + CString("*.avi"), arrayPossibleBackgroundMovies );
		GetDirListing( m_sSongDir + CString("*.mpg"), arrayPossibleBackgroundMovies );
		GetDirListing( m_sSongDir + CString("*.mpeg"), arrayPossibleBackgroundMovies );

		if( arrayPossibleBackgroundMovies.GetSize() > 0 )		// we found a match
			m_sBackgroundMovieFile = arrayPossibleBackgroundMovies[0];
	}

	if( !DoesFileExist(GetCDTitlePath()) )
	{
		m_sCDTitleFile = "";
	}

	for( int i=0; i<m_arrayNotes.GetSize(); i++ )
	{
		Notes* pNotes = m_arrayNotes[i];
		ASSERT( pNotes->m_pNoteData != NULL );

		for( int r=0; r<NUM_RADAR_VALUES; r++ )
			pNotes->m_fRadarValues[r] = pNotes->GetNoteData()->GetRadarValue( (RadarCategory)r, m_fMusicLengthSeconds );
	}
}


void Song::GetNotesThatMatch( NotesType nt, CArray<Notes*, Notes*>& arrayAddTo )
{
	for( int i=0; i<m_arrayNotes.GetSize(); i++ )	// for each of the Song's Notes
		if( m_arrayNotes[i]->m_NotesType == nt )
			arrayAddTo.Add( m_arrayNotes[i] );
}

void Song::SaveToCacheFile()
{
	LOG->WriteLine( "Song::SaveToCacheFile()" );

	//
	// First look in the cache for this song (without loading NoteData)
	//
	IniFile ini;
	ini.SetPath( "Cache\\index.cache" );
	ini.ReadFile();	// don't care if this fails

	ini.SetValueI( "Cache", "CacheVersion", FILE_CACHE_VERSION );
	ini.SetValueI( "Cache", m_sSongDir, GetHashForDirectory(m_sSongDir) );
	ini.WriteFile();
	SaveToSMFile( GetCacheFilePath() );
}


void Song::SaveToSMFile( CString sPath )
{
	if( sPath == "" )
	{
		sPath = GetSongFilePath();

		//
		// rename all old files to avoid confusion
		//
		CStringArray arrayOldFileNames;
		GetDirListing( m_sSongDir + "*.bms", arrayOldFileNames );
		GetDirListing( m_sSongDir + "*.dwi", arrayOldFileNames );
		
		for( int i=0; i<arrayOldFileNames.GetSize(); i++ )
		{
			CString sOldPath = m_sSongDir + arrayOldFileNames[i];
			CString sNewPath = sOldPath + ".old";
			MoveFile( sOldPath, sNewPath );
		}
	}


	LOG->WriteLine( "Song::SaveToSMDir('%s')", sPath );

	int i;

	FILE* fp = fopen( sPath, "w" );	
	if( fp == NULL )
		throw RageException( "Error opening song file '%s' for writing.", sPath );

	fprintf( fp, "#TITLE:%s;\n", m_sMainTitle );
	fprintf( fp, "#SUBTITLE:%s;\n", m_sSubTitle );
	fprintf( fp, "#ARTIST:%s;\n", m_sArtist );
	fprintf( fp, "#CREDIT:%s;\n", m_sCredit );
	fprintf( fp, "#BANNER:%s;\n", m_sBannerFile );
	fprintf( fp, "#BACKGROUND:%s;\n", m_sBackgroundFile );
	fprintf( fp, "#BACKGROUNDMOVIE:%s;\n", m_sBackgroundMovieFile );
	fprintf( fp, "#CDTITLE:%s;\n", m_sCDTitleFile );
	fprintf( fp, "#MUSIC:%s;\n", m_sMusicFile );
	fprintf( fp, "#MUSICBYTES:%u;\n", m_iMusicBytes );
	fprintf( fp, "#OFFSET:%.2f;\n", m_fOffsetInSeconds );
	fprintf( fp, "#SAMPLESTART:%.2f;\n", m_fMusicSampleStartSeconds );
	fprintf( fp, "#SAMPLELENGTH:%.2f;\n", m_fMusicSampleLengthSeconds );

	fprintf( fp, "#FREEZES:" );
	for( i=0; i<m_FreezeSegments.GetSize(); i++ )
	{
		FreezeSegment &fs = m_FreezeSegments[i];

		fprintf( fp, "%.2f=%.2f", fs.m_fStartBeat, fs.m_fFreezeSeconds );
		if( i != m_FreezeSegments.GetSize()-1 )
			fprintf( fp, "," );
	}
	fprintf( fp, ";\n" );
	
	fprintf( fp, "#BPMS:" );
	for( i=0; i<m_BPMSegments.GetSize(); i++ )
	{
		BPMSegment &bs = m_BPMSegments[i];

		fprintf( fp, "%.2f=%.2f", bs.m_fStartBeat, bs.m_fBPM );
		if( i != m_BPMSegments.GetSize()-1 )
			fprintf( fp, "," );
	}
	fprintf( fp, ";\n" );
	
	//
	// Save all Notes for this file
	//
	for( i=0; i<m_arrayNotes.GetSize(); i++ ) 
		m_arrayNotes[i]->WriteSMNotesTag( fp );

	fclose( fp );
}

void Song::LoadNoteData()
{
	if( m_arrayNotes.GetSize() > 0 )
	{
		if( m_arrayNotes[0]->m_pNoteData != NULL )	// if already loaded
			return;
		LoadFromSMFile( GetCacheFilePath(), true );
	}
}

Grade Song::GetGradeForDifficultyClass( NotesType nt, DifficultyClass dc )
{
	CArray<Notes*, Notes*> aNotes;
	this->GetNotesThatMatch( nt, aNotes );
	SortNotesArrayByDifficultyClass( aNotes );

	for( int i=0; i<aNotes.GetSize(); i++ )
	{
		Notes* pNotes = aNotes[i];
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
	
	CString sTitle1 = pSong1->GetFullTitle();
	CString sTitle2 = pSong2->GetFullTitle();

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
		
	CString sArtist1 = pSong1->m_sArtist;
	CString sArtist2 = pSong2->m_sArtist;

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
		
	CString sGroup1 = pSong1->m_sGroupName;
	CString sGroup2 = pSong2->m_sGroupName;

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
