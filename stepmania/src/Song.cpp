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
#include "NoteData.h"
#include "MsdFile.h"
#include "RageSoundStream.h"
#include "RageException.h"
#include "SongCacheIndex.h"


const int FILE_CACHE_VERSION = 61;	// increment this when Song or Notes changes to invalidate cache


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

int CompareStopSegments(const void *arg1, const void *arg2)
{
	// arg1 and arg2 are of type Step**
	StopSegment* seg1 = (StopSegment*)arg1;
	StopSegment* seg2 = (StopSegment*)arg2;

	float score1 = seg1->m_fStartBeat;
	float score2 = seg2->m_fStartBeat;

	if( score1 == score2 )
		return 0;
	else if( score1 < score2 )
		return -1;
	else
		return 1;
}

void SortStopSegmentsArray( CArray<StopSegment,StopSegment&> &arrayStopSegments )
{
	qsort( arrayStopSegments.GetData(), arrayStopSegments.GetSize(), sizeof(StopSegment), CompareStopSegments );
}

int CompareAnimationSegments(const void *arg1, const void *arg2)
{
	// arg1 and arg2 are of type Step**
	const AnimationSegment* seg1 = (const AnimationSegment*)arg1;
	const AnimationSegment* seg2 = (const AnimationSegment*)arg2;

	float score1 = seg1->m_fStartBeat;
	float score2 = seg2->m_fStartBeat;

	if( score1 == score2 )
		return 0;
	else if( score1 < score2 )
		return -1;
	else
		return 1;
}

void SortAnimationSegmentsArray( CArray<AnimationSegment,AnimationSegment&> &arrayAnimationSegments )
{
	qsort( arrayAnimationSegments.GetData(), arrayAnimationSegments.GetSize(), sizeof(AnimationSegment), CompareAnimationSegments );
}


//////////////////////////////
// Song
//////////////////////////////
Song::Song()
{
	m_bChangedSinceSave = false;
	m_fBeat0OffsetInSeconds = 0;
	m_fMusicSampleStartSeconds = 0;
	m_fMusicSampleLengthSeconds = 16;	// start fading out at m_fMusicSampleLengthSeconds-1 seconds
	m_iMusicBytes = 0;
	m_fMusicLengthSeconds = 0;
	m_fFirstBeat = -1;
	m_fLastBeat = -1;
}

Song::~Song()
{
	for( int i=0; i<m_apNotes.GetSize(); i++ )
		SAFE_DELETE( m_apNotes[i] );

	m_apNotes.RemoveAll();
}


void Song::AddBPMSegment( BPMSegment seg )
{
	m_BPMSegments.Add( seg );
	SortBPMSegmentsArray( m_BPMSegments );
}


void Song::AddStopSegment( StopSegment seg )
{
	m_StopSegments.Add( seg );
	SortStopSegmentsArray( m_StopSegments );
}


void Song::AddAnimationSegment( AnimationSegment seg )
{
	m_AnimationSegments.Add( seg );
	SortAnimationSegmentsArray( m_AnimationSegments );
}

float Song::GetMusicStartBeat() const
{
	float fBPS = m_BPMSegments[0].m_fBPM / 60.0f; 
	return -m_fBeat0OffsetInSeconds*fBPS; 
};

void Song::GetBeatAndBPSFromElapsedTime( float fElapsedTime, float &fBeatOut, float &fBPSOut, bool &bFreezeOut ) const
{
//	LOG->Trace( "GetBeatAndBPSFromElapsedTime( fElapsedTime = %f )", fElapsedTime );
	// This function is a nightmare.  Don't even try to understand it. :-)

	fElapsedTime += m_fBeat0OffsetInSeconds;


	for( int i=0; i<m_BPMSegments.GetSize(); i++ ) // foreach BPMSegment
	{
		const BPMSegment &this_seg = m_BPMSegments[i];

		float fStartBeatThisSegment = m_BPMSegments[i].m_fStartBeat;
		bool bIsLastBPMSegment = i==m_BPMSegments.GetSize()-1;
		float fStartBeatNextSegment = bIsLastBPMSegment ? 40000/*inf*/ : m_BPMSegments[i+1].m_fStartBeat; 
		float fBeatsInThisSegment = fStartBeatNextSegment - fStartBeatThisSegment;
		float fBPM = m_BPMSegments[i].m_fBPM;
		float fBPS = fBPM / 60.0f;

		// calculate the number of seconds in this segment
		float fSecondsInThisSegment =  fBeatsInThisSegment / fBPS;
		int j;
		for( j=0; j<m_StopSegments.GetSize(); j++ )	// foreach freeze
		{
			if( fStartBeatThisSegment <= m_StopSegments[j].m_fStartBeat  &&  m_StopSegments[j].m_fStartBeat < fStartBeatNextSegment )	
			{
				// this freeze lies within this BPMSegment
				fSecondsInThisSegment += m_StopSegments[j].m_fStopSeconds;
			}
		}


		if( fElapsedTime > fSecondsInThisSegment )
		{
			// this BPMSegement is NOT the current segment
			fElapsedTime -= fSecondsInThisSegment;
			continue;
		}

		// this BPMSegment IS the current segment

		float fBeatEstimate = fStartBeatThisSegment + fElapsedTime*fBPS;

		for( j=0; j<m_StopSegments.GetSize(); j++ )	// foreach freeze
		{
			if( fStartBeatThisSegment > m_StopSegments[j].m_fStartBeat  ||
				m_StopSegments[j].m_fStartBeat > fStartBeatNextSegment )	
				continue;

			// this freeze lies within this BPMSegment

			if( m_StopSegments[j].m_fStartBeat > fBeatEstimate )
				break;

			fElapsedTime -= m_StopSegments[j].m_fStopSeconds;
			// re-estimate
			fBeatEstimate = fStartBeatThisSegment + fElapsedTime*fBPS;
			if( fBeatEstimate < m_StopSegments[j].m_fStartBeat )
			{
				fBeatOut = m_StopSegments[j].m_fStartBeat;
				fBPSOut = fBPS;
				bFreezeOut = true;
				return;
			}
		}

		fBeatOut = fBeatEstimate;
		fBPSOut = fBPS;
		bFreezeOut = false;
		return;
	}
}


// This is a super hack, but it's only called from ScreenEdit, so it's OK :-)
// Writing an inverse function of GetBeatAndBPSFromElapsedTime() is impossible,
// so do a binary search to get close to the correct elapsed time.
float Song::GetElapsedTimeFromBeat( float fBeat ) const
{
	float fElapsedTimeBestGuess = 100;	//  seconds
	float fSecondsToMove = fElapsedTimeBestGuess;	//  seconds
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

void Song::GetMainAndSubTitlesFromFullTitle( const CString sFullTitle, CString &sMainTitleOut, CString &sSubTitleOut )
{
	const CString sLeftSeps[]  = { " -", " ~", " (", " [" };
	int iNumSeps = sizeof(sLeftSeps)/sizeof(CString);
	for( int i=0; i<iNumSeps; i++ )
	{
		int iBeginIndex = sFullTitle.Find( sLeftSeps[i] );
		if( iBeginIndex == -1 )
			continue;
		sMainTitleOut = sFullTitle.Left( iBeginIndex );
		sSubTitleOut = sFullTitle.Mid( iBeginIndex+1, sFullTitle.GetLength()-iBeginIndex+1 );
		return;
	}
	sMainTitleOut = sFullTitle; 
	sSubTitleOut = ""; 
};	

CString Song::GetCacheFilePath() const
{
	return ssprintf( "Cache\\%u", (UINT)GetHashForString(m_sSongDir) );
}

/* Get a path to the SM containing data for this song.  It might
 * be a cache file. */
const CString &Song::GetSongFilePath() const
{
	return m_sSongFileName;
}

bool Song::LoadFromSongDir( CString sDir )
{
	LOG->Trace( "Song::LoadFromSongDir(%s)", sDir );

	// make sure there is a trailing '\\' at the end of sDir
	if( sDir.Right(1) != "\\" )
		sDir += "\\";

	// save song dir
	m_sSongDir = sDir;

	// save group name
	CStringArray sDirectoryParts;
	split( m_sSongDir, "\\", sDirectoryParts, false );
	m_sGroupName = sDirectoryParts[sDirectoryParts.GetSize()-3];	// second from last item

	{
		/* Generated filename; this doesn't always point to a loadable file,
		 * but instead points to the place we should write changed files to.
		 * If we have an SM file in the directory, this should aways point to it. */
		CStringArray asFileNames;
		GetDirListing( m_sSongDir+"*.sm", asFileNames );
		if( asFileNames.GetSize() > 0 )
			m_sSongFileName = m_sSongDir + asFileNames[0];
		else
			m_sSongFileName = m_sSongDir + GetFullTitle() + ".sm";
	}

	//
	// First look in the cache for this song (without loading NoteData)
	//
	int iDirHash = SONGINDEX->GetCacheHash(m_sSongDir);
	if( GetHashForDirectory(m_sSongDir) == iDirHash )	// this cache is up to date
	{
		if( !DoesFileExist(GetCacheFilePath()) )
			goto load_without_cache;
		LoadFromSMFile( GetCacheFilePath() );

		LOG->Trace( "Loading '%s' from cache file '%s'.", m_sSongDir, GetCacheFilePath() );
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

	CStringArray arrayKSFFileNames;
	GetDirListing( sDir + CString("*.ksf"), arrayKSFFileNames );
	int iNumKSFFiles = arrayKSFFileNames.GetSize();

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
		LoadFromSMFile( sDir + arraySMFileNames[0] );	// do load note data
	else if( iNumDWIFiles == 1 )
		LoadFromDWIFile( sDir + arrayDWIFileNames[0] );
	else if( iNumBMSFiles > 0 )
		LoadFromBMSDir( sDir );
	else if( iNumKSFFiles > 0 )
		LoadFromKSFDir( sDir );
	else 
	{
		LOG->Warn( "Couldn't find any SM, DWI, BMS, or KSF files in '%s'.  This is not a valid song directory.", sDir );
		return false;
	}

	TidyUpData();
	
	// save a cache file so we don't have to parse it all over again next time
	SaveToCacheFile();

	return true;
}



bool Song::LoadFromBMSDir( CString sDir )
{
	LOG->Trace( "Song::LoadFromBMSDir(%s)", sDir );

	CStringArray arrayBMSFileNames;
	GetDirListing( sDir + CString("*.bms"), arrayBMSFileNames );

	if( arrayBMSFileNames.GetSize() == 0 )
		throw RageException( "Couldn't find any BMS files in '%s'", sDir );

	// load the Notes from the rest of the BMS files
	for( int i=0; i<arrayBMSFileNames.GetSize(); i++ ) 
	{
		Notes* pNewNotes = new Notes;
		pNewNotes->LoadFromBMSFile( m_sSongDir + arrayBMSFileNames[i] );
		m_apNotes.Add( pNewNotes );
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
		
			LOG->Trace( "Inserting new BPM change at beat %f, BPM %f", newSeg.m_fStartBeat, newSeg.m_fBPM );
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
			//LOG->Trace( "%s:%s: iMeasureNo = %d, iBMSTrackNo = %d, iNumNotesInThisMeasure = %d", 
			//	valuename, sNoteData, iMeasureNo, iBMSTrackNo, iNumNotesInThisMeasure );
			for( int j=0; j<iNumNotesInThisMeasure; j++ )
			{
				if( arrayNotes[j] == 0 )
					continue;

				float fPercentThroughMeasure = (float)j/(float)iNumNotesInThisMeasure;

				// index is in quarter beats starting at beat 0
				int iStepIndex = (int) ( (iMeasureNo + fPercentThroughMeasure)
								 * BEATS_PER_MEASURE * ELEMENTS_PER_BEAT );

				switch( iBMSTrackNo )
				{
				case 1:	// background music track
					{
						float fBeatOffset = fBeatOffset = NoteRowToBeat( (float)iStepIndex );
						if( fBeatOffset > 10 )	// some BPMs's play the music again at the end.  Why?  Who knows...
							break;
						float fBPS;
						fBPS = m_BPMSegments[0].m_fBPM/60.0f;
						m_fBeat0OffsetInSeconds = fBeatOffset / fBPS;
						//LOG->Trace( "Found offset to be index %d, beat %f", iStepIndex, NoteRowToBeat(iStepIndex) );
					}
					break;
				case 3:	// bpm change
					{
						BPMSegment newSeg( NoteRowToBeat(iStepIndex), (float)arrayNotes[j] );
						AddBPMSegment( newSeg );
						LOG->Trace( "Inserting new BPM change at beat %f, BPM %f", newSeg.m_fStartBeat, newSeg.m_fBPM );
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

							if( 0==stricmp(value_name, sTagToLookFor) )
							{
								fBPM = (float)atof( value_data );
								break;
							}
						}

						if( fBPM == -1 )	// we didn't find the line we were looking for
						{
							LOG->Trace( "WARNING:  Couldn't find tag '%s' in '%s'.", sTagToLookFor, sPath );
						}
						else
						{
							BPMSegment newSeg( NoteRowToBeat(iStepIndex), fBPM );
							AddBPMSegment( newSeg );
							LOG->Trace( "Inserting new BPM change at beat %f, BPM %f", newSeg.m_fStartBeat, newSeg.m_fBPM );
						}

						file.Close();
					}
					break;
				case 9:	// stop
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

							if( 0==stricmp(value_name, sTagToLookFor) )
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
							LOG->Trace( "WARNING:  Couldn't find tag '%s' in '%s'.", sTagToLookFor, sPath );
						}
						else
						{
							StopSegment newSeg( fFreezeStartBeat, fFreezeSecs );
							AddStopSegment( newSeg );
							LOG->Trace( "Inserting new Freeze at beat %f, secs %f", newSeg.m_fStartBeat, newSeg.m_fStopSeconds );
						}

						file.Close();
					}
					break;
				}
			}
		}
	}

	for( i=0; i<m_BPMSegments.GetSize(); i++ )
		LOG->Trace( "There is a BPM change at beat %f, BPM %f, index %d", 
					m_BPMSegments[i].m_fStartBeat, m_BPMSegments[i].m_fBPM, i );

	file.Close();

	return TRUE;
}


bool Song::LoadFromDWIFile( CString sPath )
{
	LOG->Trace( "Song::LoadFromDWIFile(%s)", sPath );
	

	MsdFile msd;
	bool bResult = msd.ReadFile( sPath );
	if( !bResult )
		throw RageException( "Error opening file '%s'.", sPath );

	for( int i=0; i<msd.m_iNumValues; i++ )
	{
		int iNumParams = msd.m_iNumParams[i];
		CString* sParams = msd.m_sValuesAndParams[i];
		CString sValueName = sParams[0];

		// handle the data
		if( 0==stricmp(sValueName,"FILE") )
			m_sMusicFile = sParams[1];

		else if( 0==stricmp(sValueName,"TITLE") )
			GetMainAndSubTitlesFromFullTitle( sParams[1], m_sMainTitle, m_sSubTitle );

		else if( 0==stricmp(sValueName,"ARTIST") )
			m_sArtist = sParams[1];

		else if( 0==stricmp(sValueName,"CDTITLE") )
			m_sCDTitleFile = sParams[1];

		else if( 0==stricmp(sValueName,"BPM") )
			AddBPMSegment( BPMSegment(0, (float)atof(sParams[1])) );		

		else if( 0==stricmp(sValueName,"GAP") )
			// the units of GAP is 1/1000 second
			m_fBeat0OffsetInSeconds = -atoi( sParams[1] ) / 1000.0f;

		else if( 0==stricmp(sValueName,"SAMPLESTART") )
			m_fMusicSampleStartSeconds = TimeToSeconds( sParams[1] );

		else if( 0==stricmp(sValueName,"SAMPLELENGTH") )
			m_fMusicSampleLengthSeconds = TimeToSeconds(  sParams[1] );

		else if( 0==stricmp(sValueName,"FREEZE") )
		{
			CStringArray arrayFreezeExpressions;
			split( sParams[1], ",", arrayFreezeExpressions );

			for( int f=0; f<arrayFreezeExpressions.GetSize(); f++ )
			{
				CStringArray arrayFreezeValues;
				split( arrayFreezeExpressions[f], "=", arrayFreezeValues );
				float fIndex = atoi( arrayFreezeValues[0] ) * ELEMENTS_PER_BEAT / 4.0f;
				float fFreezeBeat = NoteRowToBeat( fIndex );
				float fFreezeSeconds = (float)atof( arrayFreezeValues[1] ) / 1000.0f;
				
				AddStopSegment( StopSegment(fFreezeBeat, fFreezeSeconds) );
				LOG->Trace( "Adding a freeze segment: beat: %f, seconds = %f", fFreezeBeat, fFreezeSeconds );
			}
		}

		else if( 0==stricmp(sValueName,"CHANGEBPM")  || 0==stricmp(sValueName,"BPMCHANGE") )
		{
			CStringArray arrayBPMChangeExpressions;
			split( sParams[1], ",", arrayBPMChangeExpressions );

			for( int b=0; b<arrayBPMChangeExpressions.GetSize(); b++ )
			{
				CStringArray arrayBPMChangeValues;
				split( arrayBPMChangeExpressions[b], "=", arrayBPMChangeValues );
				float fIndex = atoi( arrayBPMChangeValues[0] ) * ELEMENTS_PER_BEAT / 4.0f;
				float fBeat = NoteRowToBeat( fIndex );
				float fNewBPM = (float)atof( arrayBPMChangeValues[1] );
				
				AddBPMSegment( BPMSegment(fBeat, fNewBPM) );
			}
		}

		else if( 0==stricmp(sValueName,"SINGLE")  || 
			     0==stricmp(sValueName,"DOUBLE")  ||
				 0==stricmp(sValueName,"COUPLE")  || 
				 0==stricmp(sValueName,"SOLO"))
		{
			Notes* pNewNotes = new Notes;
			pNewNotes->LoadFromDWITokens( 
				sParams[0], 
				sParams[1], 
				sParams[2], 
				sParams[3], 
				(iNumParams==5) ? sParams[4] : "" 
				);
			m_apNotes.Add( pNewNotes );
		}
		else
			// do nothing.  We don't care about this value name
			;
	}

	return TRUE;
}


bool Song::LoadFromSMFile( CString sPath )
{
	LOG->Trace( "Song::LoadFromSMDir(%s)", sPath );

	m_BPMSegments.RemoveAll();
	m_StopSegments.RemoveAll();

	int i;

	MsdFile msd;
	bool bResult = msd.ReadFile( sPath );
	if( !bResult )
		throw RageException( "Error opening file '%s'.", sPath );

	for( i=0; i<msd.m_iNumValues; i++ )
	{
		int iNumParams = msd.m_iNumParams[i];
		CString* sParams = msd.m_sValuesAndParams[i];
		CString sValueName = sParams[0];

		// handle the data
		if( 0==stricmp(sValueName,"TITLE") )
			GetMainAndSubTitlesFromFullTitle( sParams[1], m_sMainTitle, m_sSubTitle );

		else if( 0==stricmp(sValueName,"SUBTITLE") )
			m_sSubTitle = sParams[1];

		else if( 0==stricmp(sValueName,"ARTIST") )
			m_sArtist = sParams[1];

		else if( 0==stricmp(sValueName,"CREDIT") )
			m_sCredit = sParams[1];

		else if( 0==stricmp(sValueName,"BANNER") )
			m_sBannerFile = sParams[1];

		else if( 0==stricmp(sValueName,"BACKGROUND") )
			m_sBackgroundFile = sParams[1];

		else if( 0==stricmp(sValueName,"CDTITLE") )
			m_sCDTitleFile = sParams[1];

		else if( 0==stricmp(sValueName,"MOVIEBACKGROUND") )
			m_sMovieBackgroundFile = sParams[1];

		else if( 0==stricmp(sValueName,"MUSIC") )
			m_sMusicFile = sParams[1];

		else if( 0==stricmp(sValueName,"MUSICBYTES") )
			m_iMusicBytes = atoi( sParams[1] );

		else if( 0==stricmp(sValueName,"MUSICLENGTH") )
			m_fMusicLengthSeconds = (float)atof( sParams[1] );

		else if( 0==stricmp(sValueName,"FIRSTBEAT") )
			m_fFirstBeat = (float)atof( sParams[1] );

		else if( 0==stricmp(sValueName,"LASTBEAT") )
			m_fLastBeat = (float)atof( sParams[1] );

		else if( 0==stricmp(sValueName,"SAMPLESTART") )
			m_fMusicSampleStartSeconds = TimeToSeconds( sParams[1] );

		else if( 0==stricmp(sValueName,"SAMPLELENGTH") )
			m_fMusicSampleLengthSeconds = TimeToSeconds( sParams[1] );

		else if( 0==stricmp(sValueName,"OFFSET") )
			m_fBeat0OffsetInSeconds = (float)atof( sParams[1] );

		else if( 0==stricmp(sValueName,"STOPS") || 0==stricmp(sValueName,"FREEZES") )
		{
			CStringArray arrayFreezeExpressions;
			split( sParams[1], ",", arrayFreezeExpressions );

			for( int f=0; f<arrayFreezeExpressions.GetSize(); f++ )
			{
				CStringArray arrayFreezeValues;
				split( arrayFreezeExpressions[f], "=", arrayFreezeValues );
				float fFreezeBeat = (float)atof( arrayFreezeValues[0] );
				float fFreezeSeconds = (float)atof( arrayFreezeValues[1] );
				
				StopSegment new_seg;
				new_seg.m_fStartBeat = fFreezeBeat;
				new_seg.m_fStopSeconds = fFreezeSeconds;

				LOG->Trace( "Adding a freeze segment: beat: %f, seconds = %f", new_seg.m_fStartBeat, new_seg.m_fStopSeconds );

				AddStopSegment( new_seg );
			}
		}

		else if( 0==stricmp(sValueName,"BPMS") )
		{
			CStringArray arrayBPMChangeExpressions;
			split( sParams[1], ",", arrayBPMChangeExpressions );

			for( int b=0; b<arrayBPMChangeExpressions.GetSize(); b++ )
			{
				CStringArray arrayBPMChangeValues;
				split( arrayBPMChangeExpressions[b], "=", arrayBPMChangeValues );
				float fBeat = (float)atof( arrayBPMChangeValues[0] );
				float fNewBPM = (float)atof( arrayBPMChangeValues[1] );
				
				BPMSegment new_seg;
				new_seg.m_fStartBeat = fBeat;
				new_seg.m_fBPM = fNewBPM;
				
				AddBPMSegment( new_seg );
			}
		}

		else if( 0==stricmp(sValueName,"ANIMATIONS") )
		{
			CStringArray arrayAnimationExpressions;
			split( sParams[1], ",", arrayAnimationExpressions );

			for( int b=0; b<arrayAnimationExpressions.GetSize(); b++ )
			{
				CStringArray arrayAnimationValues;
				split( arrayAnimationExpressions[b], "=", arrayAnimationValues );
				float fBeat = (float)atof( arrayAnimationValues[0] );
				CString sAnimName = arrayAnimationValues[1];
				sAnimName.MakeLower();
				
				AddAnimationSegment( AnimationSegment(fBeat, sAnimName) );
			}
		}

		else if( 0==stricmp(sValueName,"NOTES") )
		{
			Notes* pNewNotes = new Notes;
			ASSERT( pNewNotes );
			m_apNotes.Add( pNewNotes );

			if( iNumParams != 7 )
				throw RageException( "The song file '%s' is has %d fields in a #NOTES tag, but should have %d.", sPath, iNumParams, 7 );

			pNewNotes->LoadFromSMTokens( 
				sParams[1], 
				sParams[2], 
				sParams[3], 
				sParams[4], 
				sParams[5], 
				sParams[6]
				);
		}

		else
			LOG->Trace( "Unexpected value named '%s'", sValueName );
	}

	return TRUE;
}


bool Song::LoadFromKSFDir( CString sDir )
{
	LOG->Trace( "Song::LoadFromKSFDir(%s)", sDir );

	CStringArray arrayKSFFileNames;
	GetDirListing( sDir + CString("*.ksf"), arrayKSFFileNames );

	if( arrayKSFFileNames.GetSize() == 0 )
		throw RageException( "Couldn't find any KSF files in '%s'", sDir );

	// load the Notes from the rest of the KSF files
	for( int i=0; i<arrayKSFFileNames.GetSize(); i++ ) 
	{
		if( arrayKSFFileNames[i].Find("_2") != -1 )
			continue;	// any "right-side" files should be loaded by Notes
		Notes* pNewNotes = new Notes;
		pNewNotes->LoadFromKSFFile( m_sSongDir + arrayKSFFileNames[i] );
		m_apNotes.Add( pNewNotes );
	}

	CString sPath = m_sSongDir + arrayKSFFileNames[0];

	MsdFile msd;
	bool bResult = msd.ReadFile( sPath );
	if( !bResult )
		throw RageException( "Error opening file '%s'.", sPath );

	for( i=0; i<msd.m_iNumValues; i++ )
	{
		int iNumParams = msd.m_iNumParams[i];
		CString* sParams = msd.m_sValuesAndParams[i];
		CString sValueName = sParams[0];

		// handle the data
		if( 0==stricmp(sValueName,"TITLE") )
		{
			//title is usually in format "artist - songtitle"
			CStringArray asBits;
			split( sParams[1], " - ", asBits, false );

			/* It's often "artist - songtitle - difficulty".  Ignore
			 * the difficulty, since we get that from the filename. */
			if( asBits.GetSize() == 3 &&
				(!stricmp(asBits[2], "double") ||
				 !stricmp(asBits[2], "easy") ||
				 !stricmp(asBits[2], "normal") ||
				 !stricmp(asBits[2], "hard") ||
				 !stricmp(asBits[2], "crazy")) )
			{
				asBits.RemoveAt(2);
			}

			if( asBits.GetSize() == 2 )
			{
				m_sArtist = asBits[0];
				m_sMainTitle = asBits[1];
			}
			else
			{
				m_sMainTitle = asBits[0];
			}

			for( int j=0; j<m_sMainTitle.GetLength(); j++ )
			{
				char c = m_sMainTitle[j];
				if( c < 0 )	// this title has a foreign char
				{
					CStringArray asBits;
					split( sDir, "\\", asBits, true);
					CString sSongFolderName = asBits[ asBits.GetSize()-1 ];
					asBits.RemoveAll();

					split( sSongFolderName, " - ", asBits, false );
					if( asBits.GetSize() == 2 )
					{
						m_sArtist = asBits[0];
						m_sMainTitle = asBits[1];
					}
					else
					{
						m_sMainTitle = asBits[0];
					}
					break;
				}
			}
		}

		else if( 0==stricmp(sValueName,"BPM") )
			AddBPMSegment( BPMSegment(0, (float)atof(sParams[1])) );		

		else if( 0==stricmp(sValueName,"STARTTIME") )
			m_fBeat0OffsetInSeconds = -(float)atof(sParams[1])/100;		
		else if( 0==stricmp(sValueName,"TICKCOUNT") ||
				 0==stricmp(sValueName,"STEP") ||
				 0==stricmp(sValueName,"DIFFICULTY"))
			; /* Handled in Notes::LoadFromKSFFile; don't warn. */
		else
			LOG->Trace( "Unexpected value named '%s'", sValueName );
	}

	// search for music with song in the file name
	CStringArray arrayPossibleMusic;
	GetDirListing( m_sSongDir + CString("song.mp3"), arrayPossibleMusic );
	GetDirListing( m_sSongDir + CString("song.ogg"), arrayPossibleMusic );
	GetDirListing( m_sSongDir + CString("song.wav"), arrayPossibleMusic );

	if( arrayPossibleMusic.GetSize() > 0 )		// we found a match
		m_sMusicFile = arrayPossibleMusic[0];

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

	if( !HasMusic() )
	{
		CStringArray arrayPossibleMusic;
		GetDirListing( m_sSongDir + CString("*.mp3"), arrayPossibleMusic );
		GetDirListing( m_sSongDir + CString("*.ogg"), arrayPossibleMusic );
		GetDirListing( m_sSongDir + CString("*.wav"), arrayPossibleMusic );

		if( arrayPossibleMusic.GetSize() > 0 )		// we found a match
			m_sMusicFile = arrayPossibleMusic[0];
		else
			throw RageException( "The song in '%s' is missing a music file.  You must place a music file in the song folder or remove the song", m_sSongDir );
	}

	RageSoundStream sound;
	sound.Load( GetMusicPath() );
	m_fMusicLengthSeconds = sound.GetLengthSeconds();



	//
	// Here's the problem:  We have a directory full of images.  We want to determine which 
	// image is the banner, which is the background, and which is the CDTitle.
	//

	//
	// First, check the file name for hints.
	//
	if( !HasBanner() )
	{
		m_sBannerFile = "";

		// find an image with "banner" in the file name
		CStringArray arrayPossibleBanners;
		GetDirListing( m_sSongDir + CString("*banner*.png"), arrayPossibleBanners );
		GetDirListing( m_sSongDir + CString("*banner*.jpg"), arrayPossibleBanners );
		GetDirListing( m_sSongDir + CString("*banner*.bmp"), arrayPossibleBanners );
		GetDirListing( m_sSongDir + CString("*banner*.gif"), arrayPossibleBanners );
		if( arrayPossibleBanners.GetSize() > 0 )
			m_sBannerFile = arrayPossibleBanners[0];
	}

	if( !HasBackground() )
	{
		m_sBackgroundFile = "";

		// find an image with "bg" or "background" in the file name
		CStringArray arrayPossibleBGs;
		GetDirListing( m_sSongDir + CString("*bg*.png"), arrayPossibleBGs );
		GetDirListing( m_sSongDir + CString("*bg*.jpg"), arrayPossibleBGs );
		GetDirListing( m_sSongDir + CString("*bg*.bmp"), arrayPossibleBGs );
		GetDirListing( m_sSongDir + CString("*bg*.gif"), arrayPossibleBGs );
		GetDirListing( m_sSongDir + CString("*background*.png"), arrayPossibleBGs );
		GetDirListing( m_sSongDir + CString("*background*.jpg"), arrayPossibleBGs );
		GetDirListing( m_sSongDir + CString("*background*.bmp"), arrayPossibleBGs );
		GetDirListing( m_sSongDir + CString("*background*.gif"), arrayPossibleBGs );
		if( arrayPossibleBGs.GetSize() > 0 )
			m_sBackgroundFile = arrayPossibleBGs[0];
	}

	if( !HasCDTitle() )
	{
		m_sCDTitleFile = "";

		// find an image with "cdtitle" in the file name
		CStringArray arrayPossibleCDTitles;
		GetDirListing( m_sSongDir + CString("*cdtitle*.png"), arrayPossibleCDTitles );
		GetDirListing( m_sSongDir + CString("*cdtitle*.jpg"), arrayPossibleCDTitles );
		GetDirListing( m_sSongDir + CString("*cdtitle*.bmp"), arrayPossibleCDTitles );
		GetDirListing( m_sSongDir + CString("*cdtitle*.gif"), arrayPossibleCDTitles );
		if( arrayPossibleCDTitles.GetSize() > 0 )
			m_sCDTitleFile = arrayPossibleCDTitles[0];
	}


	//
	// Now, For the images we still haven't found, look at the image dimensions of the remaining unclassified images.
	//
	CStringArray arrayImages;
	GetDirListing( m_sSongDir + CString("*.png"), arrayImages );
	GetDirListing( m_sSongDir + CString("*.jpg"), arrayImages );
	GetDirListing( m_sSongDir + CString("*.bmp"), arrayImages );
	GetDirListing( m_sSongDir + CString("*.gif"), arrayImages );

	for( int i=0; i<arrayImages.GetSize(); i++ )	// foreach image
	{
		// Skip any image that we've already classified

		if( HasBanner()  &&  stricmp(m_sBannerFile, arrayImages[i])==0 )
			continue;	// skip

		if( HasBackground()  &&  stricmp(m_sBackgroundFile, arrayImages[i])==0 )
			continue;	// skip

		if( HasCDTitle()  &&  stricmp(m_sCDTitleFile, arrayImages[i])==0 )
			continue;	// skip

		D3DXIMAGE_INFO ddii;
		if( !FAILED( D3DXGetImageInfoFromFile( m_sSongDir + arrayImages[i], &ddii ) ) )
		{
			if( !HasBackground()  &&  ddii.Width >= 320  &&  ddii.Height >= 240 )
			{
				m_sBackgroundFile = arrayImages[i];
				continue;
			}
			if( !HasBanner()  &&  100 < ddii.Width &&  ddii.Width < 320  &&  50 < ddii.Height  &&  ddii.Height < 240 )
			{
				m_sBannerFile = arrayImages[i];
				continue;
			}
			if( !HasCDTitle()  &&  ddii.Width <= 100  &&  ddii.Height <= 50 )
			{
				m_sCDTitleFile = arrayImages[i];
				continue;
			}
		}
	}



	if( !HasMovieBackground() )
	{
		CStringArray arrayPossibleMovies;
		GetDirListing( m_sSongDir + CString("*movie*.avi"), arrayPossibleMovies );
		GetDirListing( m_sSongDir + CString("*movie*.mpg"), arrayPossibleMovies );
		GetDirListing( m_sSongDir + CString("*movie*.mpeg"), arrayPossibleMovies );
		GetDirListing( m_sSongDir + CString("*.avi"), arrayPossibleMovies );
		GetDirListing( m_sSongDir + CString("*.mpg"), arrayPossibleMovies );
		GetDirListing( m_sSongDir + CString("*.mpeg"), arrayPossibleMovies );
		if( arrayPossibleMovies.GetSize() > 0 )
			m_sMovieBackgroundFile = arrayPossibleMovies[0];
	}

	//
	// calculate radar values and first/last beat
	//
	for( i=0; i<m_apNotes.GetSize(); i++ )
	{
		Notes* pNotes = m_apNotes[i];
		NoteData tempNoteData;
		pNotes->GetNoteData( &tempNoteData );

		for( int r=0; r<NUM_RADAR_VALUES; r++ )
			pNotes->m_fRadarValues[r] = tempNoteData.GetRadarValue( (RadarCategory)r, m_fMusicLengthSeconds );

		float fFirstBeat = tempNoteData.GetFirstBeat();
		float fLastBeat = tempNoteData.GetLastBeat();
		if( m_fFirstBeat == -1 )
			m_fFirstBeat = fFirstBeat;
		else
			m_fFirstBeat = min( m_fFirstBeat, fFirstBeat );
		if( m_fLastBeat == -1 )
			m_fLastBeat = fLastBeat;
		else
			m_fLastBeat = max( m_fLastBeat, fLastBeat );
	}
}


void Song::GetNotesThatMatch( NotesType nt, CArray<Notes*, Notes*>& arrayAddTo ) const
{
	for( int i=0; i<m_apNotes.GetSize(); i++ )	// for each of the Song's Notes
		if( m_apNotes[i]->m_NotesType == nt )
			arrayAddTo.Add( m_apNotes[i] );
}

bool Song::SongHasNoteType( NotesType nt ) const
{
	for( int i=0; i < m_apNotes.GetSize(); i++ ) // foreach Notes
	{
		if( m_apNotes[i]->m_NotesType == nt )
			return true;
	}

	return false;
}

void Song::SaveToCacheFile()
{
	LOG->Trace( "Song::SaveToCacheFile()" );

	SONGINDEX->AddCacheIndex(m_sSongDir, GetHashForDirectory(m_sSongDir));
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
		GetDirListing( m_sSongDir + "*.ksf", arrayOldFileNames );
		
		for( int i=0; i<arrayOldFileNames.GetSize(); i++ )
		{
			CString sOldPath = m_sSongDir + arrayOldFileNames[i];
			CString sNewPath = sOldPath + ".old";
			MoveFile( sOldPath, sNewPath );
		}
	}


	LOG->Trace( "Song::SaveToSMDir('%s')", sPath );

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
	fprintf( fp, "#CDTITLE:%s;\n", m_sCDTitleFile );
	fprintf( fp, "#MOVIEBACKGROUND:%s;\n", m_sMovieBackgroundFile );
	fprintf( fp, "#MUSIC:%s;\n", m_sMusicFile );
	fprintf( fp, "#MUSICBYTES:%u;\n", m_iMusicBytes );
	fprintf( fp, "#MUSICLENGTH:%.2f;\n", m_fMusicLengthSeconds );
	fprintf( fp, "#FIRSTBEAT:%.2f;\n", m_fFirstBeat );
	fprintf( fp, "#LASTBEAT:%.2f;\n", m_fLastBeat );
	fprintf( fp, "#OFFSET:%.2f;\n", m_fBeat0OffsetInSeconds );
	fprintf( fp, "#SAMPLESTART:%.2f;\n", m_fMusicSampleStartSeconds );
	fprintf( fp, "#SAMPLELENGTH:%.2f;\n", m_fMusicSampleLengthSeconds );

	fprintf( fp, "#BPMS:" );
	for( i=0; i<m_BPMSegments.GetSize(); i++ )
	{
		BPMSegment &bs = m_BPMSegments[i];

		fprintf( fp, "%.2f=%.2f", bs.m_fStartBeat, bs.m_fBPM );
		if( i != m_BPMSegments.GetSize()-1 )
			fprintf( fp, "," );
	}
	fprintf( fp, ";\n" );

	fprintf( fp, "#STOPS:" );
	for( i=0; i<m_StopSegments.GetSize(); i++ )
	{
		StopSegment &fs = m_StopSegments[i];

		fprintf( fp, "%.2f=%.2f", fs.m_fStartBeat, fs.m_fStopSeconds );
		if( i != m_StopSegments.GetSize()-1 )
			fprintf( fp, "," );
	}
	fprintf( fp, ";\n" );
	
	fprintf( fp, "#ANIMATIONS:" );
	for( i=0; i<m_AnimationSegments.GetSize(); i++ )
	{
		AnimationSegment &seg = m_AnimationSegments[i];

		fprintf( fp, "%.2f=%s", seg.m_fStartBeat, seg.m_sAnimationName );
		if( i != m_AnimationSegments.GetSize()-1 )
			fprintf( fp, "," );
	}
	fprintf( fp, ";\n" );
	
	//
	// Save all Notes for this file
	//
	for( i=0; i<m_apNotes.GetSize(); i++ ) 
		m_apNotes[i]->WriteSMNotesTag( fp );

	fclose( fp );
}

void Song::SaveToSMAndDWIFile()
{
	LOG->Trace( "Song::SaveToSMAndDWIFile()" );

	SaveToSMFile();

	CString sPath = GetSongFilePath();
	sPath.Replace( ".sm", ".dwi" );


	FILE* fp = fopen( sPath, "w" );	
	if( fp == NULL )
		throw RageException( "Error opening song file '%s' for writing.", sPath );

	fprintf( fp, "#TITLE:%s;\n", GetFullTitle() );
	fprintf( fp, "#ARTIST:%s;\n", m_sArtist );
	ASSERT( m_BPMSegments[0].m_fStartBeat == 0 );
	fprintf( fp, "#BPM:%.2f;\n", m_BPMSegments[0].m_fBPM );
	fprintf( fp, "#GAP:%d;\n", -roundf( m_fBeat0OffsetInSeconds*1000 ) );

	fprintf( fp, "#FREEZE:" );
	for( int i=0; i<m_StopSegments.GetSize(); i++ )
	{
		StopSegment &fs = m_StopSegments[i];
		fprintf( fp, "%.2f=%.2f", BeatToNoteRow( fs.m_fStartBeat ) / ELEMENTS_PER_BEAT * 4.0f, roundf(fs.m_fStopSeconds*1000) );
		if( i != m_StopSegments.GetSize()-1 )
			fprintf( fp, "," );
	}
	fprintf( fp, ";\n" );

	fprintf( fp, "#CHANGEBPM:" );
	for( i=1; i<m_BPMSegments.GetSize(); i++ )
	{
		BPMSegment &bs = m_BPMSegments[i];
		fprintf( fp, "%.2f=%.2f", BeatToNoteRow( bs.m_fStartBeat ) / ELEMENTS_PER_BEAT * 4.0f, bs.m_fBPM );
		if( i != m_BPMSegments.GetSize()-1 )
			fprintf( fp, "," );
	}
	fprintf( fp, ";\n" );
	
	//
	// Save all Notes for this file
	//
	for( i=0; i<m_apNotes.GetSize(); i++ ) 
		m_apNotes[i]->WriteDWINotesTag( fp );

	fclose( fp );
}

Grade Song::GetGradeForDifficultyClass( NotesType nt, DifficultyClass dc ) const
{
	CArray<Notes*, Notes*> aNotes;
	this->GetNotesThatMatch( nt, aNotes );
	SortNotesArrayByDifficulty( aNotes );

	for( int i=0; i<aNotes.GetSize(); i++ )
	{
		Notes* pNotes = aNotes[i];
		if( pNotes->m_DifficultyClass == dc )
			return pNotes->m_TopGrade;
	}
	return GRADE_NO_DATA;
}


bool Song::IsNew() const
{
	return GetNumTimesPlayed()==0;
}

bool Song::IsEasy( NotesType nt ) const
{
	for( int i=0; i<m_apNotes.GetSize(); i++ )
	{
		Notes* pNotes = m_apNotes[i];
		if( pNotes->m_NotesType != nt )
			continue;
		if( pNotes->m_iMeter <= 2 )
			return true;
	}
	return false;
}

/////////////////////////////////////
// Sorting
/////////////////////////////////////

int CompareSongPointersByTitle(const void *arg1, const void *arg2)
{
	const Song* pSong1 = *(const Song**)arg1;
	const Song* pSong2 = *(const Song**)arg2;
	
	int ret = pSong1->GetFullTitle().CompareNoCase(pSong2->GetFullTitle());
	if(ret != 0) return ret;

	/* The titles are the same.  Ensure we get a consistent ordering
	 * by comparing the unique SongFilePaths. */
	return pSong1->GetSongFilePath().CompareNoCase(pSong2->GetSongFilePath());
}

void SortSongPointerArrayByTitle( CArray<Song*, Song*> &arraySongPointers )
{
	qsort( arraySongPointers.GetData(), arraySongPointers.GetSize(), sizeof(Song*), CompareSongPointersByTitle );
}

int CompareSongPointersByBPM(const void *arg1, const void *arg2)
{
	const Song* pSong1 = *(const Song**)arg1;
	const Song* pSong2 = *(const Song**)arg2;
		
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
	const Song* pSong1 = *(const Song**)arg1;
	const Song* pSong2 = *(const Song**)arg2;
		
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
	const Song* pSong1 = *(const Song**)arg1;
	const Song* pSong2 = *(const Song**)arg2;
		
	const CString &sGroup1 = pSong1->m_sGroupName;
	const CString &sGroup2 = pSong2->m_sGroupName;

	if( sGroup1 < sGroup2 )
		return -1;
	else if( sGroup1 > sGroup2 )
		return 1;

	/* Same group; compare by title. */
	return CompareSongPointersByTitle( arg1, arg2 );
}

void SortSongPointerArrayByGroup( CArray<Song*, Song*> &arraySongPointers )
{
	qsort( arraySongPointers.GetData(), arraySongPointers.GetSize(), sizeof(Song*), CompareSongPointersByGroup );
}

int CompareSongPointersByMostPlayed(const void *arg1, const void *arg2)
{
	const Song* pSong1 = *(const Song**)arg1;
	const Song* pSong2 = *(const Song**)arg2;
		
	int iNumTimesPlayed1 = pSong1->GetNumTimesPlayed();
	int iNumTimesPlayed2 = pSong2->GetNumTimesPlayed();

	if( iNumTimesPlayed1 > iNumTimesPlayed2 )
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
