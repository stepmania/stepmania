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
#include "GameManager.h"
#include "PrefsManager.h"
#include "StyleDef.h"
#include "Notes.h"

#include "NotesLoaderSM.h"
#include "NotesLoaderDWI.h"
#include "NotesLoaderBMS.h"
#include "NotesLoaderKSF.h"

const int FILE_CACHE_VERSION = 65;	// increment this when Song or Notes changes to invalidate cache


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

int CompareBackgroundChanges(const void *arg1, const void *arg2)
{
	// arg1 and arg2 are of type Step**
	const BackgroundChange* seg1 = (const BackgroundChange*)arg1;
	const BackgroundChange* seg2 = (const BackgroundChange*)arg2;

	float score1 = seg1->m_fStartBeat;
	float score2 = seg2->m_fStartBeat;

	if( score1 == score2 )
		return 0;
	else if( score1 < score2 )
		return -1;
	else
		return 1;
}

void SortBackgroundChangesArray( CArray<BackgroundChange,BackgroundChange&> &arrayBackgroundChanges )
{
	qsort( arrayBackgroundChanges.GetData(), arrayBackgroundChanges.GetSize(), sizeof(BackgroundChange), CompareBackgroundChanges );
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
	m_SelectionDisplay = SHOW_ALWAYS;
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


void Song::AddBackgroundChange( BackgroundChange seg )
{
	m_BackgroundChanges.Add( seg );
	SortBackgroundChangesArray( m_BackgroundChanges );
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


// This is a super hack, but it's only called from ScreenEdit, so it's OK.
// Writing an inverse function of GetBeatAndBPSFromElapsedTime() uber difficult,
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
	return ssprintf( "Cache\\%u", GetHashForString(m_sSongDir) );
}

/* Get a path to the SM containing data for this song.  It might
 * be a cache file. */
const CString &Song::GetSongFilePath() const
{
	ASSERT ( m_sSongFileName.GetLength() != 0 );
	return m_sSongFileName;
}

bool Song::LoadWithoutCache( CString sDir )
{
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
	if( iNumDWIFiles > 1 )
		throw RageException( "There is more than one DWI file in '%s'.  There should be only one!", sDir );
	
	if( iNumSMFiles == 1 )
	{
		SMLoader ld;
		ld.LoadFromSMFile( sDir + arraySMFileNames[0], *this );
	}
	else if( iNumDWIFiles == 1 )
	{
		DWILoader ld;
		ld.LoadFromDWIFile( sDir + arrayDWIFileNames[0], *this );
	}
	else if( iNumBMSFiles > 0 )
	{
		BMSLoader ld;
		ld.LoadFromBMSDir( sDir, *this );
	}
	else if( iNumKSFFiles > 0 )
	{
		KSFLoader ld;
		ld.LoadFromKSFDir( sDir, *this );
	}
	else 
	{
		LOG->Warn( "Couldn't find any SM, DWI, BMS, or KSF files in '%s'.  This is not a valid song directory.", sDir );
		return false;
	}

	AddAutoGenNotes();

	TidyUpData();
	
	// save a cache file so we don't have to parse it all over again next time
	SaveToCacheFile();
	return true;
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

	//
	// First look in the cache for this song (without loading NoteData)
	//
	unsigned uDirHash = SONGINDEX->GetCacheHash(m_sSongDir);
	if( GetHashForDirectory(m_sSongDir) == uDirHash && // this cache is up to date 
		DoesFileExist(GetCacheFilePath()))	
	{
		LOG->Trace( "Loading '%s' from cache file '%s'.", m_sSongDir, GetCacheFilePath() );
		SMLoader ld;
		ld.LoadFromSMFile( GetCacheFilePath(), *this );
	}
	else
	{
		if(!LoadWithoutCache(sDir))
			return false;
	}

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

	return true;
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

/* Get Notes that match a given style and player. */
void Song::GetNotesThatMatch( const StyleDef *s, int p, CArray<Notes*, Notes*>& arrayAddTo ) const
{
	for( int i=0; i<m_apNotes.GetSize(); i++ )	// for each of the Song's Notes
	{
		if( m_apNotes[i]->m_NotesType == s->m_NotesTypes[p] ||
			m_apNotes[i]->m_NotesType == s->m_FallbackNotesType )
			arrayAddTo.Add( m_apNotes[i] );
	}
}

/* Return whether the song is playable in the given style. */
bool Song::SongCompleteForStyle( const StyleDef *st ) const
{
	for( int pn = 0; pn < NUM_PLAYERS; ++pn )
	{
		if(st->m_NotesTypes[pn] == NOTES_TYPE_INVALID) continue; /* unused */
		if(!SongHasNotesType(st->m_NotesTypes[pn]) &&
		   !SongHasNotesType(st->m_FallbackNotesType))
			return false;
	}
	return true;
}

bool Song::SongHasNotesType( NotesType nt ) const
{
	for( int i=0; i < m_apNotes.GetSize(); i++ ) // foreach Notes
		if( m_apNotes[i]->m_NotesType == nt )
			return true;
	return false;
}

bool Song::SongHasNotesTypeAndDifficulty( NotesType nt, DifficultyClass dc ) const
{
	for( int i=0; i < m_apNotes.GetSize(); i++ ) // foreach Notes
		if( m_apNotes[i]->m_NotesType == nt  &&  m_apNotes[i]->m_DifficultyClass == dc )
			return true;
	return false;
}

void Song::SaveToCacheFile()
{
	LOG->Trace( "Song::SaveToCacheFile()" );

	SONGINDEX->AddCacheIndex(m_sSongDir, GetHashForDirectory(m_sSongDir));
	SaveToSMFile( GetCacheFilePath(), true );
}

void Song::SaveToSongFile()
{
	LOG->Trace( "Song::SaveToSongFile()" );

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

	SaveToSMFile( GetSongFilePath(), false );
}


void Song::SaveToSMFile( CString sPath, bool bSavingCache )
{
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
	fprintf( fp, "#SELECTABLE:" );
	switch(m_SelectionDisplay) {
	default: ASSERT(0);  /* fallthrough */
	case SHOW_ALWAYS:
		fprintf( fp, "YES" ); break;
	case SHOW_NEVER:
		fprintf( fp, "NO" ); break;
	case SHOW_ROULETTE:
		fprintf( fp, "ROULETTE" ); break;
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

	fprintf( fp, "#STOPS:" );
	for( i=0; i<m_StopSegments.GetSize(); i++ )
	{
		StopSegment &fs = m_StopSegments[i];

		fprintf( fp, "%.2f=%.2f", fs.m_fStartBeat, fs.m_fStopSeconds );
		if( i != m_StopSegments.GetSize()-1 )
			fprintf( fp, "," );
	}
	fprintf( fp, ";\n" );
	
	fprintf( fp, "#BGCHANGES:" );
	for( i=0; i<m_BackgroundChanges.GetSize(); i++ )
	{
		BackgroundChange &seg = m_BackgroundChanges[i];

		fprintf( fp, "%.2f=%s", seg.m_fStartBeat, seg.m_sBGName );
		if( i != m_BackgroundChanges.GetSize()-1 )
			fprintf( fp, "," );
	}
	fprintf( fp, ";\n" );
	
	//
	// Save all Notes for this file
	//
	for( i=0; i<m_apNotes.GetSize(); i++ ) 
	{
		Notes* pNotes = m_apNotes[i];
		if( bSavingCache  ||  pNotes->m_sDescription.Find("(autogen)") == -1 )	// If notes aren't autogen
			m_apNotes[i]->WriteSMNotesTag( fp );
	}

	fclose( fp );
}

void Song::SaveToSongFileAndDWI()
{
	LOG->Trace( "Song::SaveToSongFileAndDWI()" );

	SaveToSongFile();

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
		fprintf( fp, "%.2f=%.2f", BeatToNoteRow( fs.m_fStartBeat ) / ROWS_PER_BEAT * 4.0f, roundf(fs.m_fStopSeconds*1000) );
		if( i != m_StopSegments.GetSize()-1 )
			fprintf( fp, "," );
	}
	fprintf( fp, ";\n" );

	fprintf( fp, "#CHANGEBPM:" );
	for( i=1; i<m_BPMSegments.GetSize(); i++ )
	{
		BPMSegment &bs = m_BPMSegments[i];
		fprintf( fp, "%.2f=%.2f", BeatToNoteRow( bs.m_fStartBeat ) / ROWS_PER_BEAT * 4.0f, bs.m_fBPM );
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

struct AutoGenMapping
{
	NotesType ntMissing;
	NotesType ntGenerateFrom;
	int iOriginalTrackToTakeFrom[MAX_NOTE_TRACKS];
};
const AutoGenMapping AUTO_GEN_MAPPINGS[] = {
	{ NOTES_TYPE_PUMP_SINGLE,	NOTES_TYPE_DANCE_SINGLE,	{ 0, 1, -1, 2, 3 }					},
	{ NOTES_TYPE_DANCE_SINGLE,	NOTES_TYPE_PUMP_SINGLE,		{ 0, 1, 3, 4 }						},
	{ NOTES_TYPE_PUMP_DOUBLE,	NOTES_TYPE_DANCE_DOUBLE,	{ 0, 1, -1, 2, 3, 4, 5, -1, 6, 7 }	},
	{ NOTES_TYPE_DANCE_DOUBLE,	NOTES_TYPE_PUMP_DOUBLE,		{ 0, 1, 3, 4, 5, 6, 8, 9 }			},
};
const NUM_AUTOGEN_MAPPINGS = sizeof(AUTO_GEN_MAPPINGS) / sizeof(AutoGenMapping);

void Song::AddAutoGenNotes()
{
	for( int i=0; i<NUM_AUTOGEN_MAPPINGS; i++ )
	{
		const AutoGenMapping& mapping = AUTO_GEN_MAPPINGS[i];

		if( !SongHasNotesType(mapping.ntMissing))
		{
			for( int i=0; i<m_apNotes.GetSize(); i++ )
			{
				Notes* pOriginalNotes = m_apNotes[i];
				if( pOriginalNotes->m_NotesType != mapping.ntGenerateFrom )
					continue;

				Notes* pNewNotes = new Notes;
				pNewNotes->m_DifficultyClass	= pOriginalNotes->m_DifficultyClass;
				pNewNotes->m_iMeter				= pOriginalNotes->m_iMeter;
				pNewNotes->m_sDescription		= pOriginalNotes->m_sDescription + " (autogen)";
				pNewNotes->m_NotesType			= mapping.ntMissing;

				NoteData originalNoteData, newNoteData;
				pOriginalNotes->GetNoteData( &originalNoteData );
				newNoteData.LoadTransformed( &originalNoteData, GAMEMAN->NotesTypeToNumTracks(mapping.ntMissing), mapping.iOriginalTrackToTakeFrom );
				pNewNotes->SetNoteData( &newNoteData );
				this->m_apNotes.Add( pNewNotes );
			}
		}
	}
}

Grade Song::GetGradeForDifficultyClass( const StyleDef *st, int p, DifficultyClass dc ) const
{
	CArray<Notes*, Notes*> aNotes;
	this->GetNotesThatMatch( st, p, aNotes );
	SortNotesArrayByDifficulty( aNotes );

	for( int i=0; i<aNotes.GetSize(); i++ )
	{
		const Notes* pNotes = aNotes[i];
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

bool Song::NormallyDisplayed() const 
{
	if(!PREFSMAN->m_bHiddenSongs) return true;
	return m_SelectionDisplay == SHOW_ALWAYS;
}

bool Song::RouletteDisplayed() const
{
	if(!PREFSMAN->m_bHiddenSongs) return true;
	return m_SelectionDisplay != SHOW_NEVER;
}

bool Song::HasMusic() const 		{return m_sMusicFile != ""			&&	IsAFile(GetMusicPath()); };
bool Song::HasBanner() const 		{return m_sBannerFile != ""			&&  IsAFile(GetBannerPath()); };
bool Song::HasBackground() const 	{return m_sBackgroundFile != ""		&&  IsAFile(GetBackgroundPath()); };
bool Song::HasCDTitle() const 		{return m_sCDTitleFile != ""		&&  IsAFile(GetCDTitlePath()); };
bool Song::HasMovieBackground() const {return m_sMovieBackgroundFile != ""&&  IsAFile(GetMovieBackgroundPath()); };
bool Song::HasBGChanges() const 	{return m_BackgroundChanges.GetSize() > 0; };

int Song::GetNumTimesPlayed() const
{
	int iTotalNumTimesPlayed = 0;
	for( int i=0; i<m_apNotes.GetSize(); i++ )
	{
		iTotalNumTimesPlayed += m_apNotes[i]->m_iNumTimesPlayed;
	}
	return iTotalNumTimesPlayed;
}
