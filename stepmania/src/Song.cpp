#include "global.h"
/*
-----------------------------------------------------------------------------
 Class: Song

 Desc: Holds metadata for a song and the song's step data.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
	Glenn Maynard
-----------------------------------------------------------------------------
*/

#include "song.h"
#include "Steps.h"
#include "RageUtil.h"
#include "RageLog.h"
#include "IniFile.h"
#include "NoteData.h"
#include "RageSound.h"
#include "RageException.h"
#include "SongCacheIndex.h"
#include "GameManager.h"
#include "PrefsManager.h"
#include "StyleDef.h"
#include "GameState.h"
#include "FontCharAliases.h"
#include "TitleSubstitution.h"
#include "BannerCache.h"
#include "Sprite.h"
#include "PrefsManager.h"
#include "arch/arch.h"
#include "RageFile.h"

#include "NotesLoaderSM.h"
#include "NotesLoaderDWI.h"
#include "NotesLoaderBMS.h"
#include "NotesLoaderKSF.h"
#include "NotesWriterDWI.h"
#include "NotesWriterSM.h"

#include "LyricsLoader.h"

#include "SDL.h"
#include "SDL_image.h"

#include <set>

#define CACHE_DIR BASE_PATH "Cache" SLASH

const int FILE_CACHE_VERSION = 127;	// increment this when Song or Steps changes to invalidate cache

const float DEFAULT_MUSIC_SAMPLE_LENGTH = 12.f;


static int CompareBPMSegments(const BPMSegment &seg1, const BPMSegment &seg2)
{
	return seg1.m_fStartBeat < seg2.m_fStartBeat;
}

void SortBPMSegmentsArray( vector<BPMSegment> &arrayBPMSegments )
{
	sort( arrayBPMSegments.begin(), arrayBPMSegments.end(), CompareBPMSegments );
}

static int CompareStopSegments(const StopSegment &seg1, const StopSegment &seg2)
{
	return seg1.m_fStartBeat < seg2.m_fStartBeat;
}

void SortStopSegmentsArray( vector<StopSegment> &arrayStopSegments )
{
	sort( arrayStopSegments.begin(), arrayStopSegments.end(), CompareStopSegments );
}

int CompareBackgroundChanges(const BackgroundChange &seg1, const BackgroundChange &seg2)
{
	return seg1.m_fStartBeat < seg2.m_fStartBeat;
}

void SortBackgroundChangesArray( vector<BackgroundChange> &arrayBackgroundChanges )
{
	sort( arrayBackgroundChanges.begin(), arrayBackgroundChanges.end(), CompareBackgroundChanges );
}


//////////////////////////////
// Song
//////////////////////////////
Song::Song()
{
	m_bChangedSinceSave = false;
	m_fBeat0OffsetInSeconds = 0;
	m_fMusicSampleStartSeconds = -1;
	m_fMusicSampleLengthSeconds = DEFAULT_MUSIC_SAMPLE_LENGTH;
	m_iMusicBytes = 0;
	m_fMusicLengthSeconds = 0;
	m_fFirstBeat = -1;
	m_fLastBeat = -1;
	m_SelectionDisplay = SHOW_ALWAYS;
	m_DisplayBPMType = DISPLAY_ACTUAL;
	m_fSpecifiedBPMMin = 0;
	m_fSpecifiedBPMMax = 0;
}

Song::~Song()
{
	for( unsigned i=0; i<m_apNotes.size(); i++ )
		SAFE_DELETE( m_apNotes[i] );

	m_apNotes.clear();
}

/* Reset to an empty song. */
void Song::Reset()
{
	for( unsigned i=0; i<m_apNotes.size(); i++ )
		SAFE_DELETE( m_apNotes[i] );
	m_apNotes.clear();

	Song empty;
	*this = empty;
}


void Song::AddBPMSegment( BPMSegment seg )
{
	m_BPMSegments.push_back( seg );
	SortBPMSegmentsArray( m_BPMSegments );
}


void Song::AddStopSegment( StopSegment seg )
{
	m_StopSegments.push_back( seg );
	SortStopSegmentsArray( m_StopSegments );
}


void Song::AddBackgroundChange( BackgroundChange seg )
{
	m_BackgroundChanges.push_back( seg );
	SortBackgroundChangesArray( m_BackgroundChanges );
}


void Song::AddLyricSegment( LyricSegment seg )
{
	m_LyricSegments.push_back( seg );
}


float Song::GetMusicStartBeat() const
{
	float fBPS = m_BPMSegments[0].m_fBPM / 60.0f; 
	return -(PREFSMAN->m_fGlobalOffsetSeconds+m_fBeat0OffsetInSeconds)*fBPS; 
};

void Song::GetBeatAndBPSFromElapsedTime( float fElapsedTime, float &fBeatOut, float &fBPSOut, bool &bFreezeOut ) const
{
//	LOG->Trace( "GetBeatAndBPSFromElapsedTime( fElapsedTime = %f )", fElapsedTime );
	// This function is a nightmare.  Don't even try to understand it. :-)

	fElapsedTime += PREFSMAN->m_fGlobalOffsetSeconds;

	fElapsedTime += m_fBeat0OffsetInSeconds;


	for( unsigned i=0; i<m_BPMSegments.size(); i++ ) // foreach BPMSegment
	{
		float fStartBeatThisSegment = m_BPMSegments[i].m_fStartBeat;
		bool bIsLastBPMSegment = i==m_BPMSegments.size()-1;
		float fStartBeatNextSegment = bIsLastBPMSegment ? 40000/*inf*/ : m_BPMSegments[i+1].m_fStartBeat; 
		float fBeatsInThisSegment = fStartBeatNextSegment - fStartBeatThisSegment;
		float fBPM = m_BPMSegments[i].m_fBPM;
		float fBPS = fBPM / 60.0f;

		// calculate the number of seconds in this segment
		float fSecondsInThisSegment =  fBeatsInThisSegment / fBPS;
		unsigned j;
		for( j=0; j<m_StopSegments.size(); j++ )	// foreach freeze
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

		for( j=0; j<m_StopSegments.size(); j++ )	// foreach freeze
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


float Song::GetElapsedTimeFromBeat( float fBeat ) const
{
	float fElapsedTime = 0;
	fElapsedTime -= PREFSMAN->m_fGlobalOffsetSeconds;
	fElapsedTime -= m_fBeat0OffsetInSeconds;

	for( unsigned j=0; j<m_StopSegments.size(); j++ )	// foreach freeze
	{
		if( m_StopSegments[j].m_fStartBeat >= fBeat )
			break;
		fElapsedTime += m_StopSegments[j].m_fStopSeconds;
	}

	for( unsigned i=0; i<m_BPMSegments.size(); i++ ) // foreach BPMSegment
	{
		const float fStartBeatThisSegment = m_BPMSegments[i].m_fStartBeat;
		const bool bIsLastBPMSegment = i==m_BPMSegments.size()-1;
		const float fStartBeatNextSegment = bIsLastBPMSegment ? 40000/*inf*/ : m_BPMSegments[i+1].m_fStartBeat; 
		const float fBPS = m_BPMSegments[i].m_fBPM / 60.0f;
		const float fBeatsInThisSegment = fStartBeatNextSegment - fStartBeatThisSegment;

		fElapsedTime += min(fBeat, fBeatsInThisSegment) / fBPS;
		fBeat -= fBeatsInThisSegment;
		
		if( fBeat <= 0 )
			return fElapsedTime;
	}

	ASSERT(0);
	return fElapsedTime;
#if 0
// This is a super hack, but it's only called from ScreenEdit, so it's OK.
// Writing an inverse function of GetBeatAndBPSFromElapsedTime() uber difficult,
// so do a binary search to get close to the correct elapsed time.

	float fElapsedTimeBestGuess = this->m_fMusicLengthSeconds/2;	//  seconds
	float fSecondsToMove = fElapsedTimeBestGuess;	//  seconds
	float fBeatOut, fBPSOut;
	bool bFreezeOut;

	/* 0.001 gives higher precision and takes about 7 more iterations than
	 * 0.100. A 90-second song took about 9 iterations; now it takes about
	 * 16.  -glenn */
	while( fSecondsToMove > 0.001f )
	{
		GetBeatAndBPSFromElapsedTime( fElapsedTimeBestGuess, fBeatOut, fBPSOut, bFreezeOut );
		/* If this is an exact match, we're done.  However, if we're on a
		 * freeze segment, keep moving backwards until we're off it, so we
		 * return the time associated with the beginning of the freeze segment
		 * and not some random place in its middle. */
		if( fBeatOut == fBeat && !bFreezeOut)
			break;

		if( fBeatOut >= fBeat )
			fElapsedTimeBestGuess -= fSecondsToMove;
		else
			fElapsedTimeBestGuess += fSecondsToMove;

		fSecondsToMove /= 2;
	}

	return fElapsedTimeBestGuess;
#endif
}

CString Song::GetCacheFilePath() const
{
	return ssprintf( CACHE_DIR "Songs" SLASH "%u", GetHashForString(m_sSongDir) );
}

/* Get a path to the SM containing data for this song.  It might
 * be a cache file. */
const CString &Song::GetSongFilePath() const
{
	ASSERT ( m_sSongFileName.GetLength() != 0 );
	return m_sSongFileName;
}

NotesLoader *Song::MakeLoader( CString sDir ) const
{
	NotesLoader *ret;

	/* Actually, none of these have any persistant data, so we 
	 * could optimize this, but since they don't have any data,
	 * there's no real point ... */
	ret = new SMLoader;
	if(ret->Loadable( sDir )) return ret;
	delete ret;

	ret = new DWILoader;
	if(ret->Loadable( sDir )) return ret;
	delete ret;

	ret = new BMSLoader;
	if(ret->Loadable( sDir )) return ret;
	delete ret;

	ret = new KSFLoader;
	if(ret->Loadable( sDir )) return ret;
	delete ret;

	return NULL;
}

/* Hack: This should be a parameter to TidyUpData, but I don't want to
 * pull in <set> into Song.h, which is heavily used. */
static set<istring> BlacklistedImages;

bool Song::LoadWithoutCache( CString sDir )
{
	//
	// There was no entry in the cache for this song, or it was out of date.
	// Let's load it from a file, then write a cache entry.
	//
	
	NotesLoader *ld = MakeLoader( sDir );
	if(!ld)
	{
		LOG->Warn( "Couldn't find any SM, DWI, BMS, or KSF files in '%s'.  This is not a valid song directory.", sDir.c_str() );
		return false;
	}

	bool success = ld->LoadFromDir( sDir, *this );
	BlacklistedImages = ld->GetBlacklistedImages();
	delete ld;

	if(!success)
		return false;

	TidyUpData();

	// save a cache file so we don't have to parse it all over again next time
	SaveToCacheFile();
	return true;
}

bool Song::LoadFromSongDir( CString sDir )
{
//	LOG->Trace( "Song::LoadFromSongDir(%s)", sDir.c_str() );
	ASSERT( sDir != "" );

	// make sure there is a trailing slash at the end of sDir
	if( sDir.Right(1) != SLASH )
		sDir += SLASH;

	// save song dir
	m_sSongDir = sDir;

	// save group name
	CStringArray sDirectoryParts;
	split( m_sSongDir, SLASH, sDirectoryParts, false );
	ASSERT( sDirectoryParts.size() >= 4 ); /* Songs/Slow/Taps/ */
	m_sGroupName = sDirectoryParts[sDirectoryParts.size()-3];	// second from last item
	ASSERT( m_sGroupName != "" );

	//
	// First look in the cache for this song (without loading NoteData)
	//
	unsigned uDirHash = SONGINDEX->GetCacheHash(m_sSongDir);
	if( GetHashForDirectory(m_sSongDir) == uDirHash && // this cache is up to date 
		DoesFileExist(GetCacheFilePath()))	
	{
//		LOG->Trace( "Loading '%s' from cache file '%s'.", m_sSongDir.c_str(), GetCacheFilePath().c_str() );
		SMLoader ld;
		ld.LoadFromSMFile( GetCacheFilePath(), *this, true );
	}
	else
	{
		if(!LoadWithoutCache(m_sSongDir))
			return false;
	}

	/* Load the cached banners, if it's not loaded already. */
	if( HasBanner() )
		BANNERCACHE->LoadBanner( GetBannerPath() );

	{
		/* Generated filename; this doesn't always point to a loadable file,
		 * but instead points to the file we should write changed files to,
		 * and will always be an .SM.
		 *
		 * This is a little tricky.  We can't always use the song title directly,
		 * since it might contain characters we can't store in filenames.  Two
		 * easy options: we could manually filter out invalid characters, or we
		 * could use the name of the directory, which is always a valid filename
		 * and should always be the same as the song.  The former might not catch
		 * everything--filename restrictions are platform-specific; we might even
		 * be on an 8.3 filesystem, so let's do the latter.
		 *
		 * We can't rely on searching for other data filenames; it works for DWIs,
		 * but not KSFs and BMSs.
		 *
		 * So, let's do this (by priority):
		 * 1. If there's an .SM file, use that filename.  No reason to use anything
		 *    else; it's the filename in use.
		 * 2. If there's a .DWI, use it with a changed extension.
		 * 3. Otherwise, use the name of the directory, since it's definitely a valid
		 *    filename, and should always be the title of the song (unlike KSFs).
		 */
		m_sSongFileName = m_sSongDir;
		CStringArray asFileNames;
		GetDirListing( m_sSongDir+"*.sm", asFileNames );
		if( !asFileNames.empty() )
			m_sSongFileName += asFileNames[0];
		else {
			GetDirListing( m_sSongDir+"*.dwi", asFileNames );
			if( !asFileNames.empty() ) {
				m_sSongFileName += asFileNames[0];
				/* XXX: This would mess up "vote.for.dwight.d.eisenhower.dwi". */
				m_sSongFileName.Replace( ".dwi", ".sm" );
			} else {
				m_sSongFileName += sDirectoryParts[sDirectoryParts.size()-2];	// last item
				m_sSongFileName += ".sm";
			}
		}
	}

	/* Add AutoGen pointers.  (These aren't cached.) */
	AddAutoGenNotes();

	if( !HasMusic() )
		return false;	// don't load this song
	else
		return true;	// do load this song
}

void Song::RevertFromDisk()
{
	unsigned CurNoteIndex[NUM_PLAYERS];
	memset(CurNoteIndex, 0, sizeof(CurNoteIndex));
	for( int pn = 0; pn < NUM_PLAYERS; ++pn )
		for( unsigned n = 0; n < m_apNotes.size(); ++n )
			if( m_apNotes[n] == GAMESTATE->m_pCurNotes[pn] )
				CurNoteIndex[pn] = n;

	const CString dir = GetSongDir();

	/* Erase all existing data. */
	Reset();
	LoadFromSongDir( dir );

	if( GAMESTATE->m_pCurSong == this )
	{
		/* m_pCurNotes points at notes that we just reloaded; fix it. */
		for( int pn = 0; pn < NUM_PLAYERS; ++pn )
		{
			int ind = min(CurNoteIndex[pn], m_apNotes.size()-1);
			GAMESTATE->m_pCurNotes[pn] = m_apNotes[ind];
		}
	}
}


static void GetImageDirListing( CString sPath, CStringArray &AddTo, bool bReturnPathToo=false )
{
	GetDirListing( sPath + ".png", AddTo, false, bReturnPathToo ); 
	GetDirListing( sPath + ".jpg", AddTo, false, bReturnPathToo ); 
	GetDirListing( sPath + ".bmp", AddTo, false, bReturnPathToo ); 
	GetDirListing( sPath + ".gif", AddTo, false, bReturnPathToo ); 
}

/* Songs in BlacklistImages will never be autodetected as song images. */
void Song::TidyUpData()
{
	if( !HasMusic() )
	{
		CStringArray arrayPossibleMusic;
		GetDirListing( m_sSongDir + CString("*.mp3"), arrayPossibleMusic );
		GetDirListing( m_sSongDir + CString("*.ogg"), arrayPossibleMusic );
		GetDirListing( m_sSongDir + CString("*.wav"), arrayPossibleMusic );

		if( !arrayPossibleMusic.empty() )		// we found a match
			m_sMusicFile = arrayPossibleMusic[0];
//		Don't throw on missing music.  -Chris
//		else
//			RageException::Throw( "The song in '%s' is missing a music file.  You must place a music file in the song folder or remove the song", m_sSongDir.c_str() );
	}

	/* This must be done before radar calculation. */
	if( HasMusic() )
	{
		RageSound sound;
		sound.Load( GetMusicPath(), false ); /* don't pre-cache */

		m_fMusicLengthSeconds = sound.GetLengthSeconds();

		if(m_fMusicLengthSeconds == -1)
		{
			/* It failed; bad file or something.  It's already logged a warning,
			 * so just set the file to 0 seconds. */
			m_fMusicLengthSeconds = 0;
		} else if(m_fMusicLengthSeconds == 0) {
			LOG->Warn("File %s is empty?", GetMusicPath().c_str());
		}
	}
	else	// ! HasMusic()
	{
		m_fMusicLengthSeconds = 100;		// guess
		LOG->Warn("This song has no music file; guessing at %f seconds", m_fMusicLengthSeconds);
	}

	if(m_fMusicLengthSeconds < 0)
	{
		LOG->Warn("File %i has negative length? (%i)", GetMusicPath().c_str(), m_fMusicLengthSeconds);
		m_fMusicLengthSeconds = 0;
	}

	/* Generate these before we autogen notes, so the new notes can inherit
	 * their source's values. */
	ReCalculateRadarValuesAndLastBeat();

	TrimRight(m_sMainTitle);
	if( m_sMainTitle == "" )
	{
		/* Use the song directory name. */
		CString SongDir = this->GetSongDir();
		vector<CString> parts;
		split(SongDir, SLASH, parts);
		ASSERT(parts.size() > 0);

		NotesLoader::GetMainAndSubTitlesFromFullTitle( parts[parts.size()-1], m_sMainTitle, m_sSubTitle );
	}
	TrimRight(m_sSubTitle);
	if( m_sArtist == "" )		m_sArtist = "Unknown artist";
	TranslateTitles();

	if( m_BPMSegments.empty() )
	{
		/* XXX: Once we have a way to display warnings that the user actually
		 * cares about (unlike most warnings), this should be one of them. */
		LOG->Warn( "No BPM segments specified in '%s%s', default provided.",
			m_sSongDir.c_str(), m_sSongFileName.c_str() );

		AddBPMSegment( BPMSegment(0, 60) );
	}

	/* Only automatically set the sample time if there was no sample length
	 * (m_fMusicSampleStartSeconds == -1). */
	/* We don't want to test if m_fMusicSampleStartSeconds == 0, since some 
	 * people really do want the sample to start at the very beginning of the song. */
	
	/* Having a sample start of 0 sounds terrible for most songs because because
	 * of the silence at the beginning.  Many of my files have a 0 for the 
	 * sample start that was not manually set, and I assume other people have the same.
	 * If there are complaints atou manually-set sample start at 0 is being ignored, 
	 * then change this back.  -Chris */
	if( m_fMusicSampleStartSeconds == -1 ||
		m_fMusicSampleStartSeconds == 0 ||
		m_fMusicSampleStartSeconds+m_fMusicSampleLengthSeconds > this->m_fMusicLengthSeconds )
	{
		m_fMusicSampleStartSeconds = this->GetElapsedTimeFromBeat( 100 );

		if( m_fMusicSampleStartSeconds+m_fMusicSampleLengthSeconds > this->m_fMusicLengthSeconds )
		{
			// fix for BAG and other slow songs
			int iBeat = (int)(m_fLastBeat/2);
			/* Er.  I see that this truncates the beat down to a multiple
			 * of 10, but what's the logic behind doing that?  (It'd make
			 * sense to use a multiple of 4, so we try to line up to a
			 * measure ...) -glenn */
			iBeat = iBeat - iBeat%10;
			m_fMusicSampleStartSeconds = this->GetElapsedTimeFromBeat( (float)iBeat );
		}
	}
	

	/* Some DWIs have lengths in ms when they meant seconds, eg. #SAMPLELENGTH:10;.
	 * If the sample length is way too short, change it. */
	if( m_fMusicSampleLengthSeconds < 3 || m_fMusicSampleLengthSeconds > 30 )
		m_fMusicSampleLengthSeconds = DEFAULT_MUSIC_SAMPLE_LENGTH;

	//
	// Here's the problem:  We have a directory full of images.  We want to determine which 
	// image is the banner, which is the background, and which is the CDTitle.
	//

	LOG->Trace("Looking for images...");

	/* Replace backslashes with slashes in all paths. */	
	FixSlashesInPlace( m_sMusicFile );
	FixSlashesInPlace( m_sBannerFile );
	FixSlashesInPlace( m_sBackgroundFile );
	FixSlashesInPlace( m_sCDTitleFile );
	FixSlashesInPlace( m_sLyricsFile );

	/* Many imported files contain erroneous whitespace before or after
	 * filenames.  Paths usually don't actually start or end with spaces,
	 * so let's just remove it. */
	TrimLeft(m_sBannerFile);
	TrimRight(m_sBannerFile);
	TrimLeft(m_sBackgroundFile);
	TrimRight(m_sBackgroundFile);
	TrimLeft(m_sCDTitleFile);
	TrimRight(m_sCDTitleFile);
	TrimLeft(m_sLyricsFile);
	TrimRight(m_sLyricsFile);

	//
	// First, check the file name for hints.
	//
	if( !HasBanner() )
	{
//		m_sBannerFile = "";

		// find an image with "banner" in the file name
		CStringArray arrayPossibleBanners;
		GetImageDirListing( m_sSongDir + "*banner*", arrayPossibleBanners );

		/* Some people do things differently for the sake of being different.  Don't
		 * match eg. abnormal, numbness. */
		GetImageDirListing( m_sSongDir + "* BN", arrayPossibleBanners );

		if( !arrayPossibleBanners.empty() )
			m_sBannerFile = arrayPossibleBanners[0];
	}

	if( !HasBackground() )
	{
//		m_sBackgroundFile = "";

		// find an image with "bg" or "background" in the file name
		CStringArray arrayPossibleBGs;
		GetImageDirListing( m_sSongDir + "*bg*", arrayPossibleBGs );
		GetImageDirListing( m_sSongDir + "*background*", arrayPossibleBGs );
		if( !arrayPossibleBGs.empty() )
			m_sBackgroundFile = arrayPossibleBGs[0];
	}

	if( !HasCDTitle() )
	{
		// find an image with "cdtitle" in the file name
		CStringArray arrayPossibleCDTitles;
		GetImageDirListing( m_sSongDir + "*cdtitle*", arrayPossibleCDTitles );
		if( !arrayPossibleCDTitles.empty() )
			m_sCDTitleFile = arrayPossibleCDTitles[0];
	}

	if( !HasLyrics() )
	{
		// Check if there is a lyric file in here
		CStringArray arrayLyricFiles;
		GetDirListing(m_sSongDir + CString("*.lrc"), arrayLyricFiles );
		if(	!arrayLyricFiles.empty() )
			m_sLyricsFile = arrayLyricFiles[0];
	}



	//
	// Now, For the images we still haven't found, look at the image dimensions of the remaining unclassified images.
	//
	CStringArray arrayImages;
	GetImageDirListing( m_sSongDir + "*", arrayImages );

	unsigned i;
	for( i=0; i<arrayImages.size(); i++ )	// foreach image
	{
		// ignore DWI "-char" graphics
		if( BlacklistedImages.find( arrayImages[i].c_str() ) != BlacklistedImages.end() )
			continue;	// skip
		
		// Skip any image that we've already classified

		if( HasBanner()  &&  stricmp(m_sBannerFile, arrayImages[i])==0 )
			continue;	// skip

		if( HasBackground()  &&  stricmp(m_sBackgroundFile, arrayImages[i])==0 )
			continue;	// skip

		if( HasCDTitle()  &&  stricmp(m_sCDTitleFile, arrayImages[i])==0 )
			continue;	// skip

		CString sPath = m_sSongDir + arrayImages[i];
		SDL_Surface *img = IMG_Load( sPath );
		if( !img )
		{
			LOG->Trace("Couldn't load '%s': %s", sPath.c_str(), SDL_GetError());
			continue;
		}

		const int width = img->w;
		const int height = img->h;
		SDL_FreeSurface( img );

		if( !HasBackground()  &&  width >= 320  &&  height >= 240 )
		{
			m_sBackgroundFile = arrayImages[i];
			continue;
		}

		if( !HasBanner() && Sprite::IsDiagonalBanner(width, height) )
		{
			m_sBannerFile = arrayImages[i];
			continue;
		}

		if( !HasBanner()  &&  100<=width  &&  width<=320  &&  50<=height  &&  height<=240 )
		{
			m_sBannerFile = arrayImages[i];
			continue;
		}

		/* Some songs have overlarge banners.  Check if the ratio is reasonable (over
		 * 2:1; usually over 3:1), and large (not a cdtitle). */
		if( !HasBanner() && width > 200 && float(width) / height > 2.0f )
		{
			m_sBannerFile = arrayImages[i];
			continue;
		}

		/* Agh.  DWI's inline title images are triggering this, resulting in kanji,
		 * etc., being used as a CDTitle for songs with none.  Some sample data
		 * from random incarnations:
		 *   42x50 35x50 50x50 144x49
		 * It looks like ~50 height is what people use to align to DWI's font.
		 *
		 * My tallest CDTitle is 44.  Let's cut off in the middle and hope for
		 * the best. */
		if( !HasCDTitle()  &&  width<=100  &&  height<=48 )
		{
			m_sCDTitleFile = arrayImages[i];
			continue;
		}
	}

	if( HasBanner() )
		BANNERCACHE->CacheBanner( GetBannerPath() );


	// If no BGChanges are specified and there are movies in the song directory, then assume
	// they are DWI style where the movie begins at beat 0.
	if( !HasBGChanges() )
	{
		CStringArray arrayPossibleMovies;
		GetDirListing( m_sSongDir + CString("*movie*.avi"), arrayPossibleMovies );
		GetDirListing( m_sSongDir + CString("*movie*.mpg"), arrayPossibleMovies );
		GetDirListing( m_sSongDir + CString("*movie*.mpeg"), arrayPossibleMovies );
		GetDirListing( m_sSongDir + CString("*.avi"), arrayPossibleMovies );
		GetDirListing( m_sSongDir + CString("*.mpg"), arrayPossibleMovies );
		GetDirListing( m_sSongDir + CString("*.mpeg"), arrayPossibleMovies );
		/* Use this->GetBeatFromElapsedTime(0) instead of 0 to start when the
		 * music starts. */
		if( arrayPossibleMovies.size() == 1 )
			this->AddBackgroundChange( BackgroundChange(0,arrayPossibleMovies[0],1.f,true,true,false) );
	}

	for( i=0; i<m_apNotes.size(); i++ )
		m_apNotes[i]->Compress();
}

void Song::TranslateTitles()
{
	static TitleSubst tsub("songs");

	tsub.Subst(m_sMainTitle, m_sSubTitle, m_sArtist,
				m_sMainTitleTranslit, m_sSubTitleTranslit, m_sArtistTranslit);
}

void Song::ReCalculateRadarValuesAndLastBeat()
{
	//
	// calculate radar values and first/last beat
	//
	for( unsigned i=0; i<m_apNotes.size(); i++ )
	{
		Steps* pNotes = m_apNotes[i];
		NoteData tempNoteData;
		pNotes->GetNoteData( &tempNoteData );

		for( int r=0; r<NUM_RADAR_CATEGORIES; r++ )
		{
			/* If it's autogen, radar vals come from the parent. */
			if(pNotes->IsAutogen())
				continue;

			pNotes->SetRadarValue(r, NoteDataUtil::GetRadarValue( tempNoteData, (RadarCategory)r, m_fMusicLengthSeconds ));
		}

		/* Many songs have stray, empty song patterns.  Ignore them, so
		 * they don't force the first beat of the whole song to 0.  XXX Should
		 * we just delete them, now that new patterns can be created quickly
		 * in the editor? */
		
		if(tempNoteData.GetLastRow() == 0)
			continue;

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

void Song::GetNotes( vector<Steps*>& arrayAddTo, NotesType nt, Difficulty dc, int iMeterLow, int iMeterHigh, CString sDescription, bool bIncludeAutoGen ) const
{
	for( unsigned i=0; i<m_apNotes.size(); i++ )	// for each of the Song's Steps
	{
		if( m_apNotes[i]->m_NotesType != nt ) continue;
		if( dc != DIFFICULTY_INVALID && dc != m_apNotes[i]->GetDifficulty() )
			continue;
		if( iMeterLow != -1 && iMeterLow > m_apNotes[i]->GetMeter() )
			continue;
		if( iMeterHigh != -1 && iMeterHigh < m_apNotes[i]->GetMeter() )
			continue;
		if( sDescription != "" && sDescription != m_apNotes[i]->GetDescription() )
			continue;
		if( !bIncludeAutoGen && m_apNotes[i]->IsAutogen() )
			continue;
		arrayAddTo.push_back( m_apNotes[i] );
	}
}

Steps* Song::GetNotes( NotesType nt, Difficulty dc, bool bIncludeAutoGen ) const
{
	vector<Steps*> vNotes;
	GetNotes( vNotes, nt, dc, -1, -1, "", bIncludeAutoGen );
	if( vNotes.size() == 0 )
		return NULL;
	else 
		return vNotes[0];
}

Steps* Song::GetNotes( NotesType nt, int iMeterLow, int iMeterHigh, bool bIncludeAutoGen ) const
{
	vector<Steps*> vNotes;
	GetNotes( vNotes, nt, DIFFICULTY_INVALID, iMeterLow, iMeterHigh, "", bIncludeAutoGen );
	if( vNotes.size() == 0 )
		return NULL;
	else 
		return vNotes[0];
}

Steps* Song::GetNotes( NotesType nt, CString sDescription, bool bIncludeAutoGen ) const
{
	vector<Steps*> vNotes;
	GetNotes( vNotes, nt, DIFFICULTY_INVALID, -1, -1, sDescription, bIncludeAutoGen );
	if( vNotes.size() == 0 )
		return NULL;
	else 
		return vNotes[0];
}


Steps* Song::GetClosestNotes( NotesType nt, Difficulty dc, bool bIncludeAutoGen ) const
{
	Difficulty newDC = dc;
	Steps* pNotes;
	pNotes = GetNotes( nt, newDC, bIncludeAutoGen );
	if( pNotes )
		return pNotes;
	newDC = (Difficulty)(dc-1);
	CLAMP( (int&)newDC, 0, NUM_DIFFICULTIES-1 );
	pNotes = GetNotes( nt, newDC, bIncludeAutoGen );
	if( pNotes )
		return pNotes;
	newDC = (Difficulty)(dc+1);
	CLAMP( (int&)newDC, 0, NUM_DIFFICULTIES-1 );
	pNotes = GetNotes( nt, newDC, bIncludeAutoGen );
	if( pNotes )
		return pNotes;
	newDC = (Difficulty)(dc-2);
	CLAMP( (int&)newDC, 0, NUM_DIFFICULTIES-1 );
	pNotes = GetNotes( nt, newDC, bIncludeAutoGen );
	if( pNotes )
		return pNotes;
	newDC = (Difficulty)(dc+2);
	CLAMP( (int&)newDC, 0, NUM_DIFFICULTIES-1 );
	pNotes = GetNotes( nt, newDC, bIncludeAutoGen );
	return pNotes;
}

void Song::GetEdits( vector<Steps*>& arrayAddTo, NotesType nt, bool bIncludeAutoGen ) const
{
}


/* Return whether the song is playable in the given style. */
bool Song::SongCompleteForStyle( const StyleDef *st ) const
{
	if(!SongHasNotesType(st->m_NotesType))
		return false;
	return true;
}

bool Song::SongHasNotesType( NotesType nt ) const
{
	for( unsigned i=0; i < m_apNotes.size(); i++ ) // foreach Steps
		if( m_apNotes[i]->m_NotesType == nt )
			return true;
	return false;
}

bool Song::SongHasNotesTypeAndDifficulty( NotesType nt, Difficulty dc ) const
{
	for( unsigned i=0; i < m_apNotes.size(); i++ ) // foreach Steps
		if( m_apNotes[i]->m_NotesType == nt  &&  m_apNotes[i]->GetDifficulty() == dc )
			return true;
	return false;
}

void Song::SaveToCacheFile()
{
	LOG->Trace( "Song::SaveToCacheFile()" );

	SONGINDEX->AddCacheIndex(m_sSongDir, GetHashForDirectory(m_sSongDir));
	SaveToSMFile( GetCacheFilePath(), true );
}

void Song::Save()
{
	LOG->Trace( "Song::SaveToSongFile()" );

	/* rename all old files to avoid confusion.
	 *
	 * This also serves as a backup, so rename .sm's, too.  If we crash when
	 * saving the .sm, we don't want to lose what we had.  But, what we really
	 * should be doing is saving to another file (eg. foo.sm.new), then once we 
	 * know we havn't crashed, move the old .sm to .sm.old and the new one to
	 * the real filename.  That way, if we crash, we don't leave the song in an
	 * unplayable state where the user has to manually un-rename stuff.  XXX -glenn 
	 */
	CStringArray arrayOldFileNames;
	GetDirListing( m_sSongDir + "*.bms", arrayOldFileNames );
	GetDirListing( m_sSongDir + "*.dwi", arrayOldFileNames );
	GetDirListing( m_sSongDir + "*.ksf", arrayOldFileNames );
	GetDirListing( m_sSongDir + "*.sm", arrayOldFileNames );
	
	for( unsigned i=0; i<arrayOldFileNames.size(); i++ )
	{
		CString sOldPath = m_sSongDir + arrayOldFileNames[i];
		CString sNewPath = sOldPath + ".old";
		rename( sOldPath, sNewPath );
	}

	ReCalculateRadarValuesAndLastBeat();
	TranslateTitles();

	SaveToSMFile( GetSongFilePath(), false );
	SaveToDWIFile();
}


void Song::SaveToSMFile( CString sPath, bool bSavingCache )
{
	LOG->Trace( "Song::SaveToSMFile('%s')", sPath.c_str() );

	NotesWriterSM wr;
	wr.Write(sPath, *this, bSavingCache);
}

void Song::SaveToDWIFile()
{
	LOG->Trace( "Song::SaveToSongFileAndDWI()" );

	CString sPath = GetSongFilePath();
	sPath.Replace( ".sm", ".dwi" );

	NotesWriterDWI wr;
	wr.Write(sPath, *this);
}


void Song::AddAutoGenNotes()
{
	for( NotesType ntMissing=(NotesType)0; ntMissing<NUM_NOTES_TYPES; ntMissing=(NotesType)(ntMissing+1) )
	{
		if( SongHasNotesType(ntMissing) )
			continue;


		// missing Steps of this type
		int iNumTracksOfMissing = GAMEMAN->NotesTypeToNumTracks(ntMissing);

		// look for closest match
		NotesType	ntBestMatch = (NotesType)-1;
		int			iBestTrackDifference = 10000;	// inf

		for( NotesType nt=(NotesType)0; nt<NUM_NOTES_TYPES; nt=(NotesType)(nt+1) )
		{
			vector<Steps*> apNotes;
			this->GetNotes( apNotes, nt );

			if(apNotes.empty() || apNotes[0]->IsAutogen())
				continue; /* can't autogen from other autogen */

			int iNumTracks = GAMEMAN->NotesTypeToNumTracks(nt);
			int iTrackDifference = abs(iNumTracks-iNumTracksOfMissing);
			if( iTrackDifference < iBestTrackDifference )
			{
				ntBestMatch = nt;
				iBestTrackDifference = iTrackDifference;
			}
		}

		if( ntBestMatch != -1 )
			AutoGen( ntMissing, ntBestMatch );
	}
}

void Song::AutoGen( NotesType ntTo, NotesType ntFrom )
{
//	int iNumTracksOfTo = GAMEMAN->NotesTypeToNumTracks(ntTo);

	for( unsigned int j=0; j<m_apNotes.size(); j++ )
	{
		Steps* pOriginalNotes = m_apNotes[j];
		if( pOriginalNotes->m_NotesType == ntFrom )
		{
			Steps* pNewNotes = new Steps;
			pNewNotes->AutogenFrom(pOriginalNotes, ntTo);
			this->m_apNotes.push_back( pNewNotes );
		}
	}
}

void Song::RemoveAutoGenNotes()
{
	for( int j=m_apNotes.size()-1; j>=0; j-- )
	{
		if( m_apNotes[j]->IsAutogen() )
		{
			delete m_apNotes[j];
			m_apNotes.erase( m_apNotes.begin()+j );
		}
	}
}


Grade Song::GetGradeForDifficulty( const StyleDef *st, PlayerNumber pn, Difficulty dc ) const
{
	// return max grade of notes in difficulty class
	vector<Steps*> aNotes;
	this->GetNotes( aNotes, st->m_NotesType );
	SortNotesArrayByDifficulty( aNotes );

	Grade grade = GRADE_NO_DATA;

	for( unsigned i=0; i<aNotes.size(); i++ )
	{
		const Steps* pNotes = aNotes[i];
		if( pNotes->GetDifficulty() == dc )
			grade = max( grade, pNotes->m_MemCardScores[pn].grade );
	}
	return grade;
}


bool Song::IsNew() const
{
	return GetNumTimesPlayed()==0;
}

bool Song::IsEasy( NotesType nt ) const
{
	Steps* pBeginnerNotes = GetNotes( nt, DIFFICULTY_BEGINNER );
	Steps* pEasyNotes = GetNotes( nt, DIFFICULTY_EASY );
	Steps* pHardNotes = GetNotes( nt, DIFFICULTY_HARD );

	// HACK:  Looks bizarre to see the easy mark by Legend of MAX.
	if( pHardNotes && pHardNotes->GetMeter() > 9 )
		return false;

	/* Originally, there was only Easy.  Beginners found easy steps by
	 * going to the top of the default sort, since the default sort was
	 * by Easy meter.
	 *
	 * Now, there's Beginner.  We can't very well sort by it; the meter is
	 * almost always 1 or 2.  So, let's mark "easy" songs as those with any
	 * beginner steps, so beginners (playing on beginner) can be told to look
	 * for the easy icon, and other beginners (playing on light) can go by the
	 * sort like they have been.  Don't make IsEasy some kind of mixture of
	 * beginner and easy, since that's too confusing and makes it useless to
	 * beginners. 
	 *
	 * Hmm.  I guess it does make sense to show as easy if it has 1-foot light
	 * steps, too: if the user selected Beginner, then 1-foot light is probably
	 * fine, and if the song has no beginner steps that's what he'll get. */
//	return pBeginnerNotes != NULL;

	if( pBeginnerNotes )
		return true;
	else if( pEasyNotes && pEasyNotes->GetMeter()==1 )
		return true;
	else
		return false;
}

bool Song::HasEdits( NotesType nt ) const
{
	vector<Steps*> vpNotes;
	this->GetEdits( vpNotes, nt );
	return vpNotes.size() > 0;
}

/////////////////////////////////////
// Sorting
/////////////////////////////////////

CString MakeSortString( CString s )
{
	s.MakeUpper();

	// Make sure that non-alphanumeric characters are placed at the very end
	if( s.size()>0 )
	{
		if( s[0] == '.' )	// ".59"
			s.erase(s.begin());
		if( s[0] < 'A' || s[0] > 'Z' )
			if( s[0] < '0' && s[0] > '9' )
				s = char(126) + s;	// prepend with a high ASCII character
	}

	return s;
}

bool CompareSongPointersByTitle(const Song *pSong1, const Song *pSong2)
{
	// Prefer transliterations to full titles
	CString s1 = pSong1->GetTranslitMainTitle();
	CString s2 = pSong2->GetTranslitMainTitle();
	if( s1 == s2 )
	{
		s1 = pSong1->GetTranslitSubTitle();
		s2 = pSong2->GetTranslitSubTitle();
	}

	s1 = MakeSortString(s1);
	s2 = MakeSortString(s2);

	int ret = s1.CompareNoCase( s2 );
	if(ret < 0) return true;
	if(ret > 0) return false;

	/* The titles are the same.  Ensure we get a consistent ordering
	 * by comparing the unique SongFilePaths. */
	return pSong1->GetSongFilePath().CompareNoCase(pSong2->GetSongFilePath()) < 0;
}

void SortSongPointerArrayByTitle( vector<Song*> &arraySongPointers )
{
	sort( arraySongPointers.begin(), arraySongPointers.end(), CompareSongPointersByTitle );
}

static int GetSongSortDifficulty(const Song *pSong)
{
	vector<Steps*> aNotes;
	pSong->GetNotes( aNotes, GAMESTATE->GetCurrentStyleDef()->m_NotesType );

	// sort by anything but beginner
	/* This makes the sort a little odd; songs can end up in unintuitive places because,
	 * for example, of a HARD difficulty with an incorrectly-set meter of 1, or because
	 * of challenge steps. */
/*
	int iEasiestMeter = 1000;	// infinity
	for( unsigned i=0; i<aNotes.size(); i++ )
	{
		if(aNotes[i]->GetDifficulty() == DIFFICULTY_BEGINNER)
			continue;

		iEasiestMeter = min( iEasiestMeter, aNotes[i]->GetMeter() );
	}
	// odd case where there are only beginner steps... what to do? should probably
	// act just like it was a 1-footer, even if the beginner steps aren't	
	if( iEasiestMeter1 == 1000 )
		iEasiestMeter1 = 1;
	if( iEasiestMeter2 == 1000 )
		iEasiestMeter2 = 1;
*/

	/* Instead, always sort by the first difficulty found in the following order: */
	const Difficulty d[] = { DIFFICULTY_EASY, DIFFICULTY_MEDIUM, DIFFICULTY_HARD,
		DIFFICULTY_CHALLENGE, DIFFICULTY_INVALID };

	for(int i = 0; d[i] != DIFFICULTY_INVALID; ++i)
	{
		for( unsigned j = 0; j < aNotes.size(); ++j)
		{
			if(aNotes[j]->GetDifficulty() != d[i])
				continue;

			return aNotes[j]->GetMeter();
		}
	}

	return 1;
}

int CompareSongPointersByDifficulty(const Song *pSong1, const Song *pSong2)
{
	int iEasiestMeter1 = GetSongSortDifficulty(pSong1);
	int iEasiestMeter2 = GetSongSortDifficulty(pSong2);

	if( iEasiestMeter1 < iEasiestMeter2 )
		return true;
	if( iEasiestMeter1 > iEasiestMeter2 )
		return false;
	return CompareSongPointersByTitle( pSong1, pSong2 );
}

void SortSongPointerArrayByDifficulty( vector<Song*> &arraySongPointers )
{
	sort( arraySongPointers.begin(), arraySongPointers.end(), CompareSongPointersByDifficulty );
}

bool CompareSongPointersByBPM(const Song *pSong1, const Song *pSong2)
{
	float fMinBPM1, fMaxBPM1, fMinBPM2, fMaxBPM2;
	pSong1->GetDisplayBPM( fMinBPM1, fMaxBPM1 );
	pSong2->GetDisplayBPM( fMinBPM2, fMaxBPM2 );

	if( fMaxBPM1 < fMaxBPM2 )
		return true;
	if( fMaxBPM1 > fMaxBPM2 )
		return false;
	
	return CompareCStringsAsc( pSong1->GetSongFilePath(), pSong2->GetSongFilePath() );
}

void SortSongPointerArrayByBPM( vector<Song*> &arraySongPointers )
{
	sort( arraySongPointers.begin(), arraySongPointers.end(), CompareSongPointersByBPM );
}


bool CompareSongPointersByGrade(const Song *pSong1, const Song *pSong2)
{
	for( int i=NUM_GRADES; i>GRADE_NO_DATA; i-- )
	{
		Grade g = (Grade)i;
		int iCount1 = pSong1->GetNumNotesWithGrade( g );
		int iCount2 = pSong2->GetNumNotesWithGrade( g );

		if( iCount1 > iCount2 )
			return true;
		if( iCount1 < iCount2 )
			return false;
	}
	
	return CompareSongPointersByTitle( pSong1, pSong2 );
}

void SortSongPointerArrayByGrade( vector<Song*> &arraySongPointers )
{
	sort( arraySongPointers.begin(), arraySongPointers.end(), CompareSongPointersByGrade );
}


int CompareSongPointersByArtist(const Song *pSong1, const Song *pSong2)
{
	CString s1 = pSong1->GetTranslitArtist();
	CString s2 = pSong2->GetTranslitArtist();

	s1 = MakeSortString(s1);
	s2 = MakeSortString(s2);

	if( s1 < s2 )
		return true;
	if( s1 > s2 )
		return false;
	return CompareSongPointersByTitle( pSong1, pSong2 );
}

void SortSongPointerArrayByArtist( vector<Song*> &arraySongPointers )
{
	sort( arraySongPointers.begin(), arraySongPointers.end(), CompareSongPointersByArtist );
}

int CompareSongPointersByGroupAndDifficulty(const Song *pSong1, const Song *pSong2)
{
	const CString &sGroup1 = pSong1->m_sGroupName;
	const CString &sGroup2 = pSong2->m_sGroupName;

	if( sGroup1 < sGroup2 )
		return true;
	if( sGroup1 > sGroup2 )
		return false;

	/* Same group; compare by difficulty. */
	return CompareSongPointersByDifficulty( pSong1, pSong2 );
}

void SortSongPointerArrayByGroupAndDifficulty( vector<Song*> &arraySongPointers )
{
	sort( arraySongPointers.begin(), arraySongPointers.end(), CompareSongPointersByGroupAndDifficulty );
}

int CompareSongPointersByGroupAndTitle(const Song *pSong1, const Song *pSong2)
{
	const CString &sGroup1 = pSong1->m_sGroupName;
	const CString &sGroup2 = pSong2->m_sGroupName;

	if( sGroup1 < sGroup2 )
		return true;
	if( sGroup1 > sGroup2 )
		return false;

	/* Same group; compare by name. */
	return CompareSongPointersByTitle( pSong1, pSong2 );
}

void SortSongPointerArrayByGroupAndTitle( vector<Song*> &arraySongPointers )
{
	sort( arraySongPointers.begin(), arraySongPointers.end(), CompareSongPointersByGroupAndTitle );
}

bool CompareSongPointersByMostPlayed(const Song *pSong1, const Song *pSong2)
{
	return pSong1->GetNumTimesPlayed() < pSong2->GetNumTimesPlayed();
/*
Comparing titles is slow, and this makes course selection choppy.  Turning this
off means we don't get consistent orderings of songs that have been played
the same amount, but that shouldn't be a big problem; the songs should stay
in the same original order during a single load of the game anyway (and will
probably stay in the same general order even across loads).  The number of
plays is more likely to change.

This is mostly a problem with new games, where there are a large number of songs
that have all been played 0 times.

	int iNumTimesPlayed1 = pSong1->GetNumTimesPlayed();
	int iNumTimesPlayed2 = pSong2->GetNumTimesPlayed();
	
	if( iNumTimesPlayed1 > iNumTimesPlayed2 )
		return true;
	if( iNumTimesPlayed1 < iNumTimesPlayed2 )
		return false;
	return CompareSongPointersByTitle( pSong1, pSong2 );
*/
}
/* Actually, just calculating GetNumTimesPlayed within the sort is pretty
 * slow, so let's precompute it.  (This could be generalized with a template.) */
map<const Song*, CString> song_sort_val;

bool CompareSongPointersBySortVal(const Song *pSong1, const Song *pSong2)
{
	return song_sort_val[pSong1] < song_sort_val[pSong2];
}

void SortSongPointerArrayByMostPlayed( vector<Song*> &arraySongPointers )
{
	for(unsigned i = 0; i < arraySongPointers.size(); ++i)
		song_sort_val[arraySongPointers[i]] = ssprintf("%9i", arraySongPointers[i]->GetNumTimesPlayed());
	stable_sort( arraySongPointers.begin(), arraySongPointers.end(), CompareSongPointersBySortVal );
	reverse( arraySongPointers.begin(), arraySongPointers.end() );
	song_sort_val.clear();
}

struct CompareSongByMeter
{
	Difficulty dc;

	CompareSongByMeter(Difficulty d): dc(d) { }
	bool operator() (const Song* pSong1, const Song* pSong2)
	{
		Steps* pNotes1 = pSong1->GetNotes( GAMESTATE->GetCurrentStyleDef()->m_NotesType, dc );
		Steps* pNotes2 = pSong2->GetNotes( GAMESTATE->GetCurrentStyleDef()->m_NotesType, dc );

		const int iMeter1 = pNotes1 ? pNotes1->GetMeter() : 0;
		const int iMeter2 = pNotes2 ? pNotes2->GetMeter() : 0;

		if( iMeter1 < iMeter2 )
			return true;
		if( iMeter1 > iMeter2 )
			return false;
		return CompareSongPointersByTitle( pSong1, pSong2 );
	}
};

void SortSongPointerArrayByMeter( vector<Song*> &arraySongPointers, Difficulty dc )
{
	sort( arraySongPointers.begin(), arraySongPointers.end(), CompareSongByMeter(dc) );
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

bool Song::HasMusic() const 		{return m_sMusicFile != ""			&&	IsAFile(GetMusicPath()); }
bool Song::HasBanner() const 		{return m_sBannerFile != ""			&&  IsAFile(GetBannerPath()); }
bool Song::HasLyrics() const		{return m_sLyricsFile != ""			&&	IsAFile(GetLyricsPath()); }
bool Song::HasBackground() const 	{return m_sBackgroundFile != ""		&&  IsAFile(GetBackgroundPath()); }
bool Song::HasCDTitle() const 		{return m_sCDTitleFile != ""		&&  IsAFile(GetCDTitlePath()); }
bool Song::HasBGChanges() const 	{return !m_BackgroundChanges.empty(); }

int Song::GetNumTimesPlayed() const
{
	int iTotalNumTimesPlayed = 0;
	for( unsigned i=0; i<m_apNotes.size(); i++ )
	{
		iTotalNumTimesPlayed += m_apNotes[i]->m_MemCardScores[MEMORY_CARD_MACHINE].iNumTimesPlayed;
	}
	return iTotalNumTimesPlayed;
}

/* Search semantics for all song files:
 *
 * If the path doesn't have any directory separators, it's a filename in the
 * song directory.
 *
 * If it does, it's relative to the top directory of the tree it was loaded
 * from, eg ".\music\stuff\song.ogg".  Most of the time, that's the SM tree,
 * and that's the PWD, so just return it directly.  If the file was originally
 * loaded from the DWIPath, it's relative to that, so prepend it.
 *
 * Some DWI's do have relative paths that don't start with ".\".
 */

/* We only follow this for song files and cdtitles; it's for compatibility
 * with DWI.  We prefer paths relative to the song directory; only support
 * that for all other paths. */

/* Note: Prepending the dwipath is ugly.  The first impression might be to
 * add a Song::TopFilePath, but don't do that--we have too much stuff in there
 * already.  We don't really need it; this is the only place it'd be used.  What
 * we *should* be doing is prepending this path when we first load the song.
 * However, dwipaths are usually like "c:\games\dwi", and we can't store colons
 * in SM's; they'll get interpreted as delimiters.  So:
 * XXX: Add some kind of escape character to SM's.
 *
 * -glenn */
 
CString Song::GetMusicPath() const
{
	/* If there's no path in the music file, the file is in the same directory
	 * as the song.  (This is the preferred configuration.) */
	if( m_sMusicFile.Find('/') == -1)
		return m_sSongDir+m_sMusicFile;

	/* The file has a path.  If it was loaded from the m_DWIPath, it's relative
	 * to that. */
	if( PREFSMAN->m_DWIPath!="" && m_sSongDir.Left(PREFSMAN->m_DWIPath.GetLength()) == PREFSMAN->m_DWIPath )
		return PREFSMAN->m_DWIPath+SLASH+m_sMusicFile;

	/* Otherwise, it's relative to the top of the SM directory (the CWD), so
	 * return it directly. */
	return m_sMusicFile;
}

CString Song::GetBannerPath() const
{
	if( m_sBannerFile == "" )
		return "";
	return m_sSongDir+m_sBannerFile;
}

CString Song::GetLyricsPath() const
{
	if( m_sLyricsFile == "" )
		return "";
	return m_sSongDir+m_sLyricsFile;
}

CString Song::GetCDTitlePath() const
{
	if( m_sCDTitleFile.Find('/') == -1)
		return m_sSongDir+m_sCDTitleFile;

	if( PREFSMAN->m_DWIPath!="" && m_sSongDir.Left(PREFSMAN->m_DWIPath.GetLength()) == PREFSMAN->m_DWIPath )
		return PREFSMAN->m_DWIPath+SLASH+m_sCDTitleFile;

	return m_sCDTitleFile;
}

CString Song::GetBackgroundPath() const
{
	return m_sSongDir+m_sBackgroundFile;
}

CString Song::GetDisplayMainTitle() const
{
	if(!PREFSMAN->m_bShowNative) return GetTranslitMainTitle();
	return m_sMainTitle;
}

CString Song::GetDisplaySubTitle() const
{
	if(!PREFSMAN->m_bShowNative) return GetTranslitSubTitle();
	return m_sSubTitle;
}

CString Song::GetDisplayArtist() const
{
	if(!PREFSMAN->m_bShowNative) return GetTranslitArtist();
	return m_sArtist;
}


CString Song::GetFullDisplayTitle() const
{
	CString Title = GetDisplayMainTitle();
	CString SubTitle = GetDisplaySubTitle();

	if(!SubTitle.empty()) Title += " " + SubTitle;
	return Title;
}

CString Song::GetFullTranslitTitle() const
{
	CString Title = GetTranslitMainTitle();
	CString SubTitle = GetTranslitSubTitle();

	if(!SubTitle.empty()) Title += " " + SubTitle;
	return Title;
}

void Song::AddNotes( Steps* pNotes )
{
	m_apNotes.push_back( pNotes );
}

void Song::RemoveNotes( Steps* pNotes )
{
	// Avoid any stale Note::parent pointers by removing all AutoGen'd Steps,
	// then adding them again.

	RemoveAutoGenNotes();

	for( int j=m_apNotes.size()-1; j>=0; j-- )
	{
		if( m_apNotes[j] == pNotes )
		{
			delete m_apNotes[j];
			m_apNotes.erase( m_apNotes.begin()+j );
			break;
		}
	}

	AddAutoGenNotes();
}

int	Song::GetNumNotesWithGrade( Grade g ) const
{
	int iCount = 0;
	vector<Steps*> vNotes;
	this->GetNotes( vNotes, GAMESTATE->GetCurrentStyleDef()->m_NotesType );
	for( unsigned j=0; j<vNotes.size(); j++ )
		if( vNotes[j]->m_MemCardScores[MEMORY_CARD_MACHINE].grade == g )
			iCount++;
	return iCount;
}

bool Song::Matches(CString sGroup, CString sSong) const
{
	if( sGroup.size() && sGroup.CompareNoCase(this->m_sGroupName) != 0)
		return false;

	CString sDir = this->GetSongDir();
	sDir.Replace("\\","/");
	CStringArray bits;
	split( sDir, "/", bits );
	ASSERT(bits.size() >= 2); /* should always have at least two parts */
	const CString &sLastBit = bits[bits.size()-1];

	// match on song dir or title (ala DWI)
	if( !sSong.CompareNoCase(sLastBit) )
		return true;
	if( !sSong.CompareNoCase(this->GetFullTranslitTitle()) )
		return true;

	return false;
}

