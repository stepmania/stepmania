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
#include "RageSoundReader_FileReader.h"
#include "RageException.h"
#include "SongManager.h"
#include "SongCacheIndex.h"
#include "GameManager.h"
#include "PrefsManager.h"
#include "StyleDef.h"
#include "GameState.h"
#include "FontCharAliases.h"
#include "TitleSubstitution.h"
#include "BannerCache.h"
#include "Sprite.h"
#include "arch/arch.h"
#include "RageFile.h"
#include "RageFileManager.h"
#include "NoteDataUtil.h"
#include "SDL_utils.h"
#include "ProfileManager.h"
#include "StageStats.h"

#include "NotesLoaderSM.h"
#include "NotesLoaderDWI.h"
#include "NotesLoaderBMS.h"
#include "NotesLoaderKSF.h"
#include "NotesWriterDWI.h"
#include "NotesWriterSM.h"

#include "LyricsLoader.h"

#include "SDL.h"

#include <set>

#define CACHE_DIR "Cache/"

const int FILE_CACHE_VERSION = 134;	// increment this when Song or Steps changes to invalidate cache
// also increment it on the rare occasion where we split modes

const float DEFAULT_MUSIC_SAMPLE_LENGTH = 12.f;



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
	m_LoadedFromProfile = PROFILE_SLOT_INVALID;
	m_bChangedSinceSave = false;
	m_fMusicSampleStartSeconds = -1;
	m_fMusicSampleLengthSeconds = DEFAULT_MUSIC_SAMPLE_LENGTH;
	m_fMusicLengthSeconds = 0;
	m_fFirstBeat = -1;
	m_fLastBeat = -1;
	m_SelectionDisplay = SHOW_ALWAYS;
	m_DisplayBPMType = DISPLAY_ACTUAL;
	m_fSpecifiedBPMMin = 0;
	m_fSpecifiedBPMMax = 0;
	m_bIsSymLink = false;
	m_bHasMusic = false;
	m_bHasBanner = false;
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

	/* Courses cache Notes* pointers.  On the off chance that this isn't the last
	 * thing this screen does, clear that cache. */
	SONGMAN->Cleanup();
}


void Song::AddBackgroundChange( BackgroundChange seg )
{
	m_BackgroundChanges.push_back( seg );
	SortBackgroundChangesArray( m_BackgroundChanges );
}

void Song::AddForegroundChange( BackgroundChange seg )
{
	m_ForegroundChanges.push_back( seg );
	SortBackgroundChangesArray( m_ForegroundChanges );
}

void Song::AddLyricSegment( LyricSegment seg )
{
	m_LyricSegments.push_back( seg );
}

void Song::GetDisplayBPM( float &fMinBPMOut, float &fMaxBPMOut ) const
{
	if( m_DisplayBPMType == DISPLAY_SPECIFIED )
	{
		fMinBPMOut = m_fSpecifiedBPMMin;
		fMaxBPMOut = m_fSpecifiedBPMMax;
	}
	else
	{
		m_Timing.GetActualBPM( fMinBPMOut, fMaxBPMOut );
	}
}

CString Song::GetBackgroundAtBeat( float fBeat ) const
{
	unsigned i;
	for( i=0; i<m_BackgroundChanges.size()-1; i++ )
		if( m_BackgroundChanges[i+1].m_fStartBeat > fBeat )
			break;
	return m_BackgroundChanges[i].m_sBGName;
}


CString Song::GetCacheFilePath() const
{
	return ssprintf( CACHE_DIR "Songs/%u", GetHashForString(m_sSongDir) );
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

/*
 * If PREFSMAN->m_bFastLoad is true, always load from cache if possible. Don't read
 * the contents of sDir if we can avoid it.  That means we can't call HasMusic(),
 * HasBanner() or GetHashForDirectory().
 *
 * If true, check the directory hash and reload the song from scratch if it's changed.
 */
bool Song::LoadFromSongDir( CString sDir )
{
//	LOG->Trace( "Song::LoadFromSongDir(%s)", sDir.c_str() );
	ASSERT( sDir != "" );

	// make sure there is a trailing slash at the end of sDir
	if( sDir.Right(1) != "/" )
		sDir += "/";

	// save song dir
	m_sSongDir = sDir;

	// save group name
	CStringArray sDirectoryParts;
	split( m_sSongDir, "/", sDirectoryParts, false );
	ASSERT( sDirectoryParts.size() >= 4 ); /* Songs/Slow/Taps/ */
	m_sGroupName = sDirectoryParts[sDirectoryParts.size()-3];	// second from last item
	ASSERT( m_sGroupName != "" );

	//
	// First look in the cache for this song (without loading NoteData)
	//
	unsigned uDirHash = SONGINDEX->GetCacheHash(m_sSongDir);
	bool bUseCache = true;
	if( !DoesFileExist(GetCacheFilePath()) )
		bUseCache = false;
	if( !PREFSMAN->m_bFastLoad && GetHashForDirectory(m_sSongDir) != uDirHash )
		bUseCache = false; // this cache is out of date 

	if( bUseCache )
	{
//		LOG->Trace( "Loading '%s' from cache file '%s'.", m_sSongDir.c_str(), GetCacheFilePath().c_str() );
		SMLoader ld;
		ld.LoadFromSMFile( GetCacheFilePath(), *this, true );
	}
	else
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
	}

	/* Load the cached banners, if it's not loaded already. */
	if( m_bHasBanner )
		BANNERCACHE->LoadBanner( GetBannerPath() );

	/* Add AutoGen pointers.  (These aren't cached.) */
	AddAutoGenNotes();

	if( !m_bHasMusic )
		return false;	// don't load this song
	else
		return true;	// do load this song
}

struct SongID
{
	StepsType st;
	Difficulty dc;
	SongID() { SetFrom(NULL); }
	void SetFrom( const Steps *p )
	{
		if( p == NULL )
		{
			st = STEPS_TYPE_INVALID;
			dc = DIFFICULTY_INVALID;
		}
		else
		{
			st = p->m_StepsType;
			dc = p->GetDifficulty();
		}
	}
	Steps *GetSteps( const Song *p ) const
	{
		if( st == STEPS_TYPE_INVALID || dc == DIFFICULTY_INVALID )
			return NULL;
		Steps *ret = p->GetStepsByDifficulty( st, dc, false );
		RAGE_ASSERT_M( ret, ssprintf("%i, %i", st, dc) );	// we had something selected before reloading, so it better still be there after!
		return ret;
	}
};

void Song::RevertFromDisk()
{
	// Ugly:  When we re-load the song, the Steps* will change.
	// Fix GAMESTATE->m_CurNotes, g_CurStageStats, g_vPlayedStageStats[] after reloading.
	/* XXX: This is very brittle.  However, we must know about all globals uses of Steps*,
	 * so we can check to make sure we didn't lose any steps which are referenced ... */
	SongID OldCurNotes[NUM_PLAYERS];
	SongID OldCurStageStats[NUM_PLAYERS];
	vector<SongID> OldPlayedStageStats[NUM_PLAYERS];
	for( int p = 0; p < NUM_PLAYERS; ++p )
	{
		OldCurNotes[p].SetFrom( GAMESTATE->m_pCurNotes[p] );
		OldCurStageStats[p].SetFrom( g_CurStageStats.pSteps[p] );
		for( unsigned i = 0; i < g_vPlayedStageStats.size(); ++i )
		{
			OldPlayedStageStats[p].push_back( SongID() );
			OldPlayedStageStats[p][i].SetFrom( g_vPlayedStageStats[i].pSteps[p] );
		}
	}

	const CString dir = GetSongDir();

	/* Erase all existing data. */
	Reset();

	FILEMAN->FlushDirCache( dir );

	const bool OldVal = PREFSMAN->m_bFastLoad;
	PREFSMAN->m_bFastLoad = false;

	LoadFromSongDir( dir );	

	PREFSMAN->m_bFastLoad = OldVal;

	{
		for( int p = 0; p < NUM_PLAYERS; ++p )
		{
			CHECKPOINT;
			if( GAMESTATE->m_pCurSong == this )
				GAMESTATE->m_pCurNotes[p] = OldCurNotes[p].GetSteps( this );
			CHECKPOINT;
			if( g_CurStageStats.pSong == this )
				g_CurStageStats.pSteps[p] = OldCurStageStats[p].GetSteps( this );
			CHECKPOINT;
			for( unsigned i = 0; i < g_vPlayedStageStats.size(); ++i )
			{
			CHECKPOINT_M(ssprintf("%i", i));
				if( g_vPlayedStageStats[i].pSong == this )
					g_vPlayedStageStats[i].pSteps[p] = OldPlayedStageStats[p][i].GetSteps( this );
			}
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

static CString RemoveInitialWhitespace( CString s )
{
	unsigned i = s.find_first_not_of(" \t\r\n");
	if( i != s.npos )
		s.erase( 0, i );
	return s;
}

/* This is called within TidyUpData, before autogen notes are added. */
static void DeleteDuplicateSteps( Song *song, vector<Steps*> &vSteps )
{
	/* vSteps have the same StepsType and Difficulty.  Delete them if they have the
	 * same m_sDescription, m_iMeter and SMNoteData. */
	CHECKPOINT;
	for( unsigned i=0; i<vSteps.size(); i++ )
	{
		CHECKPOINT;
		const Steps *s1 = vSteps[i];

		for( unsigned j=i+1; j<vSteps.size(); j++ )
		{
			CHECKPOINT;
			const Steps *s2 = vSteps[j];

			if( s1->GetDescription() != s2->GetDescription() )
				continue;
			if( s1->GetMeter() != s2->GetMeter() )
				continue;
			/* Compare, ignoring whitespace. */
			CString sSMNoteData1, sSMAttackData1;
			s1->GetSMNoteData( sSMNoteData1, sSMAttackData1 );
			CString sSMNoteData2, sSMAttackData2;
			s2->GetSMNoteData( sSMNoteData2, sSMAttackData2 );
			if( RemoveInitialWhitespace(sSMNoteData1) != RemoveInitialWhitespace(sSMNoteData2) )
				continue;

			LOG->Trace("Removed %p duplicate steps in song \"%s\" with description \"%s\" and meter \"%i\"",
				s2, song->GetSongDir().c_str(), s1->GetDescription().c_str(), s1->GetMeter() );
				
			/* Don't use RemoveNotes; autogen notes havn't yet been created and it'll
			 * create them. */
			for( int k=song->m_apNotes.size()-1; k>=0; k-- )
			{
				if( song->m_apNotes[k] == s2 )
				{
					delete song->m_apNotes[k];
					song->m_apNotes.erase( song->m_apNotes.begin()+k );
					break;
				}
			}

			vSteps.erase(vSteps.begin()+j);
			--j;
		}
	}
}
/*
static bool ImageIsLoadable( const CString &sPath )
{
	SDL_Surface *img = SDL_LoadImage( sPath );
	if( !img )
	{
		LOG->Warn( "Error loading song image \"%s\": %s", sPath.c_str(), SDL_GetError() );
		return false;
	}

	SDL_FreeSurface( img );
	return true;
}
*/
/* Fix up song paths.  If there's a leading "./", be sure to keep it: it's
 * a signal that the path is from the root directory, not the song directory. */
void FixupPath( CString &path )
{
	/* Replace backslashes with slashes in all paths. */	
	FixSlashesInPlace( path );

	CollapsePath( path );

	/* Many imported files contain erroneous whitespace before or after
	 * filenames.  Paths usually don't actually start or end with spaces,
	 * so let's just remove it. */
	TrimLeft( path );
	TrimRight( path );
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

		if( !arrayPossibleMusic.empty() )
		{
			int idx = 0;
			/* If the first song is "intro", and we have more than one available,
			 * don't use it--it's probably a KSF intro music file, which we don't support. */
			if( arrayPossibleMusic.size() > 1 &&
				!arrayPossibleMusic[0].Left(5).CompareNoCase("intro") )
				++idx;

			// we found a match
			m_sMusicFile = arrayPossibleMusic[idx];
		}
	}

	/* This must be done before radar calculation. */
	if( HasMusic() )
	{
		CString error;
		SoundReader *Sample = SoundReader_FileReader::OpenFile( GetMusicPath(), error );
		if( Sample == NULL )
			RageException::Throw( "Error opening sound '%s': '%s'",
				GetMusicPath().c_str(), error.c_str());

		m_fMusicLengthSeconds = Sample->GetLength() / 1000.0f;
		delete Sample;

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
		LOG->Warn("Song \"%s\" has no music file; guessing at %f seconds", this->GetSongDir().c_str(), m_fMusicLengthSeconds);
	}

	if(m_fMusicLengthSeconds < 0)
	{
		LOG->Warn( "File %s has negative length? (%f)", GetMusicPath().c_str(), m_fMusicLengthSeconds );
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
		split(SongDir, "/", parts);
		ASSERT(parts.size() > 0);

		NotesLoader::GetMainAndSubTitlesFromFullTitle( parts[parts.size()-1], m_sMainTitle, m_sSubTitle );
	}
	TrimRight(m_sSubTitle);
	if( m_sArtist == "" )		m_sArtist = "Unknown artist";
	TranslateTitles();

	if( m_Timing.m_BPMSegments.empty() )
	{
		/* XXX: Once we have a way to display warnings that the user actually
		 * cares about (unlike most warnings), this should be one of them. */
		LOG->Warn( "No BPM segments specified in '%s%s', default provided.",
			m_sSongDir.c_str(), m_sSongFileName.c_str() );

		m_Timing.AddBPMSegment( BPMSegment(0, 60) );
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

	CHECKPOINT_M( "Looking for images..." );

	FixupPath( m_sSongDir  );
	FixupPath( m_sMusicFile  );
	FixupPath( m_sBannerFile );
	FixupPath( m_sLyricsFile );
	FixupPath( m_sBackgroundFile );
	FixupPath( m_sCDTitleFile );

	//
	// First, check the file name for hints.
	//
	if( !HasBanner() )
	{
		/* If a nonexistant banner file is specified, and we can't find a replacement,
		 * don't wipe out the old value. */
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

	/* This takes a long time--sometimes as much as half of load time.  This only matters
	 * if there is both a corrupt image pointed to by the metadata *and* another valid file
	 * for the image, which is very rare.  Let's just tolerate failed images.
	 *
	 * We may still have to poke at the image below, but that's not always needed, and that
	 * can be optimized. */
	/*
	if( HasBanner() && !ImageIsLoadable( GetBannerPath() ) )
		m_sBannerFile = "";
	if( HasBackground() && !ImageIsLoadable( GetBackgroundPath() ) )
		m_sBackgroundFile = "";
	if( HasCDTitle() && !ImageIsLoadable( GetCDTitlePath() ) )
		m_sCDTitleFile = "";
	*/
	//
	// Now, For the images we still haven't found, look at the image dimensions of the remaining unclassified images.
	//
	CStringArray arrayImages;
	GetImageDirListing( m_sSongDir + "*", arrayImages );

	unsigned i;
	for( i=0; i<arrayImages.size(); i++ )	// foreach image
	{
		if( m_sBannerFile != "" && m_sCDTitleFile != "" && m_sBackgroundFile != "" )
			break; /* done */

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
		SDL_Surface *img = SDL_LoadImage( sPath );
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

	/* These will be written to cache, for Song::LoadFromSongDir to use later. */
	m_bHasMusic = HasMusic();
	m_bHasBanner = HasBanner();

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


	/* Don't allow multiple Steps of the same StepsType and Diffiuclty.
	 * This happens a lot reading BMS files because they we have to guess
	 * the Difficulty from the meter. */
	for( i=0; i<NUM_STEPS_TYPES; i++ )
	{
		StepsType st = (StepsType)i;

		for( unsigned j=0; j<NUM_DIFFICULTIES; j++ )
		{
			Difficulty dc = (Difficulty)j;

			vector<Steps*> vSteps;
			this->GetSteps( vSteps, st, dc );

			/* Delete steps that are completely identical.  This happened due to a
			 * bug in an earlier version. */
			DeleteDuplicateSteps( this, vSteps );

			CHECKPOINT;
			SortNotesArrayByDifficulty( vSteps );
			CHECKPOINT;
			for( unsigned k=1; k<vSteps.size(); k++ )
			{
				Steps* pSteps = vSteps[k];
			
				Difficulty dc2 = min( (Difficulty)(dc+1), DIFFICULTY_CHALLENGE );
				pSteps->SetDifficulty( dc2 );
			}
		}
	}

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
				m_sSongFileName += SetExtension( asFileNames[0], "sm" );
			} else {
				m_sSongFileName += Basename(m_sSongDir);
				m_sSongFileName += ".sm";
			}
		}
	}


	// Compress all Steps
	for( i=0; i<m_apNotes.size(); i++ )
	{
		m_apNotes[i]->Compress();
	}
}

void Song::TranslateTitles()
{
	static TitleSubst tsub("songs");

	tsub.Subst(m_sMainTitle, m_sSubTitle, m_sArtist,
				m_sMainTitleTranslit, m_sSubTitleTranslit, m_sArtistTranslit);
}

void Song::ReCalculateRadarValuesAndLastBeat()
{
	for( unsigned i=0; i<m_apNotes.size(); i++ )
	{
		Steps* pNotes = m_apNotes[i];

		/* If it's autogen, radar vals and first/last beat will come from the parent. */
		if( pNotes->IsAutogen() )
			continue;


		//
		// calculate radar values
		//
		NoteData tempNoteData;
		pNotes->GetNoteData( &tempNoteData );

		for( int r=0; r<NUM_RADAR_CATEGORIES; r++ )
		{
			float fVal = NoteDataUtil::GetRadarValue( tempNoteData, (RadarCategory)r, m_fMusicLengthSeconds );
			pNotes->SetRadarValue(r, fVal);
		}


		/* Calculate first/last beat.
		 *
		 * Many songs have stray, empty song patterns.  Ignore them, so
		 * they don't force the first beat of the whole song to 0. */
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

void Song::GetSteps( vector<Steps*>& arrayAddTo, StepsType nt, Difficulty dc, int iMeterLow, int iMeterHigh, const CString &sDescription, bool bIncludeAutoGen, int Max ) const
{
	if( !PREFSMAN->m_bAutogenSteps )
		bIncludeAutoGen = false;

	for( unsigned i=0; i<m_apNotes.size(); i++ )	// for each of the Song's Steps
	{
		if( !Max )
			break;

		if( nt != STEPS_TYPE_INVALID && m_apNotes[i]->m_StepsType != nt ) 
			continue;
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

		if( Max != -1 )
			--Max;

		arrayAddTo.push_back( m_apNotes[i] );
	}
}

Steps* Song::GetStepsByDifficulty( StepsType nt, Difficulty dc, bool bIncludeAutoGen ) const
{
	vector<Steps*> vNotes;
	GetSteps( vNotes, nt, dc, -1, -1, "", bIncludeAutoGen, 1 );
	if( vNotes.size() == 0 )
		return NULL;
	else 
		return vNotes[0];
}

Steps* Song::GetStepsByMeter( StepsType nt, int iMeterLow, int iMeterHigh ) const
{
	vector<Steps*> vNotes;
	GetSteps( vNotes, nt, DIFFICULTY_INVALID, iMeterLow, iMeterHigh, "", true, 1 );
	if( vNotes.size() == 0 )
		return NULL;
	else 
		return vNotes[0];
}

Steps* Song::GetStepsByDescription( StepsType nt, CString sDescription ) const
{
	vector<Steps*> vNotes;
	GetSteps( vNotes, nt, DIFFICULTY_INVALID, -1, -1, sDescription );
	if( vNotes.size() == 0 )
		return NULL;
	else 
		return vNotes[0];
}


Steps* Song::GetClosestNotes( StepsType nt, Difficulty dc ) const
{
	Difficulty newDC = dc;
	Steps* pNotes;
	pNotes = GetStepsByDifficulty( nt, newDC );
	if( pNotes )
		return pNotes;
	newDC = (Difficulty)(dc-1);
	CLAMP( (int&)newDC, 0, NUM_DIFFICULTIES-1 );
	pNotes = GetStepsByDifficulty( nt, newDC );
	if( pNotes )
		return pNotes;
	newDC = (Difficulty)(dc+1);
	CLAMP( (int&)newDC, 0, NUM_DIFFICULTIES-1 );
	pNotes = GetStepsByDifficulty( nt, newDC );
	if( pNotes )
		return pNotes;
	newDC = (Difficulty)(dc-2);
	CLAMP( (int&)newDC, 0, NUM_DIFFICULTIES-1 );
	pNotes = GetStepsByDifficulty( nt, newDC );
	if( pNotes )
		return pNotes;
	newDC = (Difficulty)(dc+2);
	CLAMP( (int&)newDC, 0, NUM_DIFFICULTIES-1 );
	pNotes = GetStepsByDifficulty( nt, newDC );
	return pNotes;
}

void Song::GetEdits( vector<Steps*>& arrayAddTo, StepsType nt ) const
{
}


/* Return whether the song is playable in the given style. */
bool Song::SongCompleteForStyle( const StyleDef *st ) const
{
	return SongHasNotesType( st->m_StepsType );
}

bool Song::SongHasNotesType( StepsType nt ) const
{
	vector<Steps*> add;
	GetSteps( add, nt, DIFFICULTY_INVALID, -1, -1, "", true, 1 );
	return !add.empty();
}

bool Song::SongHasNotesTypeAndDifficulty( StepsType nt, Difficulty dc ) const
{
	vector<Steps*> add;
	GetSteps( add, nt, dc, -1, -1, "", true, 1 );
	return !add.empty();
}

void Song::Save()
{
	LOG->Trace( "Song::SaveToSongFile()" );

	ReCalculateRadarValuesAndLastBeat();
	TranslateTitles();

	SaveToSMFile( GetSongFilePath(), false );
	SaveToDWIFile();
	SaveToCacheFile();

	/* We've safely written our files and created backups.  Rename non-SM and non-DWI
	 * files to avoid confusion. 
	 *
	 * Don't rename anything before writing.  If we do, and we crash before we actually get
	 * the file written, we'll be in a state where we can't automatically find the
	 * data in the future and the user will have to correct it himself.  Also, don't
	 * rename SM or DWI: we just wrote those. */
	CStringArray arrayOldFileNames;
	GetDirListing( m_sSongDir + "*.bms", arrayOldFileNames );
	GetDirListing( m_sSongDir + "*.ksf", arrayOldFileNames );
//	GetDirListing( m_sSongDir + "*.dwi", arrayOldFileNames );
//	GetDirListing( m_sSongDir + "*.sm", arrayOldFileNames );
	
	/* We copy files instead of renaming them, since it's more reliable: if we crash
	 * before the data below is written, we'll be left */
	for( unsigned i=0; i<arrayOldFileNames.size(); i++ )
	{
		const CString sOldPath = m_sSongDir + arrayOldFileNames[i];
		const CString sNewPath = sOldPath + ".old";

		if( !FileCopy( sOldPath, sNewPath ) )
		{
			LOG->Warn( "Backup of \"%s\" failed", sOldPath.c_str() );
			/* Don't remove. */
		} else
			FILEMAN->Remove( sOldPath );
	}

}


void Song::SaveToSMFile( CString sPath, bool bSavingCache )
{
	LOG->Trace( "Song::SaveToSMFile('%s')", sPath.c_str() );

	/* If the file exists, make a backup. */
	if( !bSavingCache && IsAFile(sPath) )
		FileCopy( sPath, sPath + ".old" );

	NotesWriterSM wr;
	wr.Write(sPath, *this, bSavingCache);
}

void Song::SaveToCacheFile()
{
	SONGINDEX->AddCacheIndex(m_sSongDir, GetHashForDirectory(m_sSongDir));
	SaveToSMFile( GetCacheFilePath(), true );
}

void Song::SaveToDWIFile()
{
	const CString sPath = SetExtension( GetSongFilePath(), "dwi" );
	LOG->Trace( "Song::SaveToDWIFile(%s)", sPath.c_str() );

	/* If the file exists, make a backup. */
	if( IsAFile(sPath) )
		FileCopy( sPath, sPath + ".old" );

	NotesWriterDWI wr;
	wr.Write(sPath, *this);
}


void Song::AddAutoGenNotes()
{
	bool HasNotes[NUM_STEPS_TYPES];
	memset( HasNotes, 0, sizeof(HasNotes) );
	for( unsigned i=0; i < m_apNotes.size(); i++ ) // foreach Steps
	{
		if( m_apNotes[i]->IsAutogen() )
			continue;

		StepsType nt = m_apNotes[i]->m_StepsType;
		HasNotes[nt] = true;
	}
		
	for( StepsType ntMissing=(StepsType)0; ntMissing<NUM_STEPS_TYPES; ntMissing=(StepsType)(ntMissing+1) )
	{
		if( HasNotes[ntMissing] )
			continue;

		// missing Steps of this type
		int iNumTracksOfMissing = GAMEMAN->NotesTypeToNumTracks(ntMissing);

		// look for closest match
		StepsType	ntBestMatch = (StepsType)-1;
		int			iBestTrackDifference = 10000;	// inf

		for( StepsType nt=(StepsType)0; nt<NUM_STEPS_TYPES; nt=(StepsType)(nt+1) )
		{
			if( !HasNotes[nt] )
				continue;

			/* has (non-autogen) Steps of this type */
			const int iNumTracks = GAMEMAN->NotesTypeToNumTracks(nt);
			const int iTrackDifference = abs(iNumTracks-iNumTracksOfMissing);
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

void Song::AutoGen( StepsType ntTo, StepsType ntFrom )
{
//	int iNumTracksOfTo = GAMEMAN->NotesTypeToNumTracks(ntTo);

	for( unsigned int j=0; j<m_apNotes.size(); j++ )
	{
		const Steps* pOriginalNotes = m_apNotes[j];
		if( pOriginalNotes->m_StepsType == ntFrom )
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

bool Song::IsEasy( StepsType nt ) const
{
	const Steps* pHardNotes = GetStepsByDifficulty( nt, DIFFICULTY_HARD );

	// HACK:  Looks bizarre to see the easy mark by Legend of MAX.
	if( pHardNotes && pHardNotes->GetMeter() > 9 )
		return false;

	/* The easy marker indicates which songs a beginner, having selected "beginner",
	 * can play and actually get a very easy song: if there are actual beginner
	 * steps, or if the light steps are 1- or 2-foot. */
	const Steps* pBeginnerNotes = GetStepsByDifficulty( nt, DIFFICULTY_BEGINNER );
	if( pBeginnerNotes )
		return true;
	
	const Steps* pEasyNotes = GetStepsByDifficulty( nt, DIFFICULTY_EASY );
	if( pEasyNotes && pEasyNotes->GetMeter() == 1 )
		return true;

	return false;
}

bool Song::HasEdits( StepsType nt ) const
{
	vector<Steps*> vpNotes;
	this->GetEdits( vpNotes, nt );
	return vpNotes.size() > 0;
}

/////////////////////////////////////
// Sorting
/////////////////////////////////////

/* Just calculating GetNumTimesPlayed within the sort is pretty slow, so let's precompute
 * it.  (This could be generalized with a template.) */
map<const Song*, CString> song_sort_val;

bool CompareSongPointersBySortValueAscending(const Song *pSong1, const Song *pSong2)
{
	return song_sort_val[pSong1] < song_sort_val[pSong2];
}

bool CompareSongPointersBySortValueDescending(const Song *pSong1, const Song *pSong2)
{
	return song_sort_val[pSong1] > song_sort_val[pSong2];
}


CString MakeSortString( CString s )
{
	s.MakeUpper();

	// Make sure that non-alphanumeric characters are placed at the very end
	if( s.size() > 0 )
	{
		if( s[0] == '.' )	// ".59"
			s.erase(s.begin());
		if( (s[0] < 'A' || s[0] > 'Z') && (s[0] < '0' || s[0] > '9') )
			s = char(126) + s;
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
	pSong->GetSteps( aNotes, GAMESTATE->GetCurrentStyleDef()->m_StepsType );

	/* Sort by the first difficulty found in the following order: */
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

void SortSongPointerArrayByDifficulty( vector<Song*> &arraySongPointers )
{
	for( unsigned i = 0; i < arraySongPointers.size(); ++i )
		song_sort_val[arraySongPointers[i]] =
			ssprintf("%9i", GetSongSortDifficulty(arraySongPointers[i]));
	stable_sort( arraySongPointers.begin(), arraySongPointers.end(), CompareSongPointersBySortValueAscending );
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

void AppendOctal( int n, int digits, CString &out )
{
	for( int p = digits; p >= 0; --p )
	{
		const int shift = p*3;
		int n2 = (n >> shift) & 0x7;
		out.insert( out.begin(), (char) n2 );
	}
}

bool CompDescending( const pair<Song *, CString> &a, const pair<Song *, CString> &b )
{ return a.second > b.second; }

void SortSongPointerArrayByGrade( vector<Song*> &arraySongPointers )
{
	/* Optimize by pre-writing a string to compare, since doing GetNumNotesWithGrade
	 * inside the sort is too slow. */
	typedef pair< Song *, CString > val;
	vector<val> vals;
	vals.reserve( arraySongPointers.size() );

	unsigned i;
	for( i = 0; i < arraySongPointers.size(); ++i )
	{
		Song *pSong = arraySongPointers[i];

		CString foo;
		foo.reserve(256);
		for( int g=NUM_GRADES-1; g>GRADE_NO_DATA; g-- )
		{
			int n = pSong->GetNumNotesWithGrade( (Grade)g );
			AppendOctal( n, 3, foo );
		}
		vals.push_back( val(pSong, foo) );
	}

	sort( vals.begin(), vals.end(), CompDescending );

	for( i = 0; i < arraySongPointers.size(); ++i )
		arraySongPointers[i] = vals[i].first;
}


void SortSongPointerArrayByArtist( vector<Song*> &arraySongPointers )
{
	for( unsigned i = 0; i < arraySongPointers.size(); ++i )
		song_sort_val[arraySongPointers[i]] = MakeSortString( arraySongPointers[i]->GetTranslitArtist() );
	stable_sort( arraySongPointers.begin(), arraySongPointers.end(), CompareSongPointersBySortValueAscending );
}

static int CompareSongPointersByGroup(const Song *pSong1, const Song *pSong2)
{
	return pSong1->m_sGroupName < pSong2->m_sGroupName;
}

void SortSongPointerArrayByGroupAndDifficulty( vector<Song*> &arraySongPointers )
{
	SortSongPointerArrayByDifficulty( arraySongPointers );
	stable_sort( arraySongPointers.begin(), arraySongPointers.end(), CompareSongPointersByGroup );
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

void SortSongPointerArrayByNumPlays( vector<Song*> &arraySongPointers, ProfileSlot slot, bool bDescending )
{
	Profile* pProfile = PROFILEMAN->GetProfile(slot);
	if( pProfile == NULL )
		return;	// nothing to do since we don't have data
	SortSongPointerArrayByNumPlays( arraySongPointers, pProfile, bDescending );
}

void SortSongPointerArrayByNumPlays( vector<Song*> &arraySongPointers, const Profile* pProfile, bool bDescending )
{
	ASSERT( pProfile );
	for(unsigned i = 0; i < arraySongPointers.size(); ++i)
		song_sort_val[arraySongPointers[i]] = ssprintf("%9i", pProfile->GetSongNumTimesPlayed(arraySongPointers[i]));
	stable_sort( arraySongPointers.begin(), arraySongPointers.end(), bDescending ? CompareSongPointersBySortValueDescending : CompareSongPointersBySortValueAscending );
	song_sort_val.clear();
}

CString GetSectionNameFromSongAndSort( const Song* pSong, SongSortOrder so )
{
	if( pSong == NULL )
		return "";

	switch( so )
	{
	case SORT_PREFERRED:
		return "";
	case SORT_GROUP:	
		return pSong->m_sGroupName;
	case SORT_TITLE:
	case SORT_ARTIST:	
		{
			CString s;
			switch( so )
			{
			case SORT_TITLE:	s = pSong->GetTranslitMainTitle();	break;
			case SORT_ARTIST:	s = pSong->GetTranslitArtist();		break;
			default:	ASSERT(0);
			}
			s = MakeSortString(s);	// resulting string will be uppercase
			
			if( s.empty() )
				return "";
			else if( s[0] >= '0' && s[0] <= '9' )
				return "NUM";
			else if( s[0] < 'A' || s[0] > 'Z')
				return "OTHER";
			else
				return s.Left(1);
		}
	case SORT_BPM:
		{
			const int iBPMGroupSize = 20;
			float fMinBPM, fMaxBPM;
			pSong->GetDisplayBPM( fMinBPM, fMaxBPM );
			int iMaxBPM = (int)fMaxBPM;
			iMaxBPM += iBPMGroupSize - (iMaxBPM%iBPMGroupSize) - 1;
			return ssprintf("%03d-%03d",iMaxBPM-(iBPMGroupSize-1), iMaxBPM);
		}
	case SORT_MOST_PLAYED:
		return "";
	case SORT_GRADE:
		{
			for( int i=NUM_GRADES; i>GRADE_NO_DATA; i-- )
			{
				Grade g = (Grade)i;
				int iCount = pSong->GetNumNotesWithGrade( g );
				if( iCount > 0 )
					return ssprintf( "%4s x %d", GradeToString(g).c_str(), iCount );
			}
			return "NO DATA";
		}
	case SORT_EASY_METER:
		{
			Steps* pNotes = pSong->GetStepsByDifficulty(GAMESTATE->GetCurrentStyleDef()->m_StepsType,DIFFICULTY_EASY);
			if( pNotes )	
				return ssprintf("%02d", pNotes->GetMeter() );
			return "N/A";
		}
	case SORT_MEDIUM_METER:
		{
			Steps* pNotes = pSong->GetStepsByDifficulty(GAMESTATE->GetCurrentStyleDef()->m_StepsType,DIFFICULTY_MEDIUM);
			if( pNotes )	
				return ssprintf("%02d", pNotes->GetMeter() );
			return "N/A";
		}
	case SORT_HARD_METER:
		{
			Steps* pNotes = pSong->GetStepsByDifficulty(GAMESTATE->GetCurrentStyleDef()->m_StepsType,DIFFICULTY_HARD);
			if( pNotes )	
				return ssprintf("%02d", pNotes->GetMeter() );
			return "N/A";
		}
	case SORT_CHALLENGE_METER:
		{
			Steps* pNotes = pSong->GetStepsByDifficulty(GAMESTATE->GetCurrentStyleDef()->m_StepsType,DIFFICULTY_CHALLENGE);
			if( pNotes )	
				return ssprintf("%02d", pNotes->GetMeter() );
			return "N/A";
		}
	case SORT_SORT_MENU:
	case SORT_MODE_MENU:
		return "";
	case SORT_ALL_COURSES:
	case SORT_NONSTOP_COURSES:
	case SORT_ONI_COURSES:
	case SORT_ENDLESS_COURSES:
	default:
		ASSERT(0);
		return "";
	}
}

void SortSongPointerArrayBySectionName( vector<Song*> &arraySongPointers, SongSortOrder so )
{
	for(unsigned i = 0; i < arraySongPointers.size(); ++i)
	{
		CString val = GetSectionNameFromSongAndSort( arraySongPointers[i], so );

		/* Make sure NUM comes first and OTHER comes last. */
		if( val == "NUM" )			val = "0";
		else if( val == "OTHER" )	val = "2";
		else						val = "1" + val;

		song_sort_val[arraySongPointers[i]] = val;
	}

	stable_sort( arraySongPointers.begin(), arraySongPointers.end(), CompareSongPointersBySortValueAscending );
	song_sort_val.clear();
}

void SortSongPointerArrayByMeter( vector<Song*> &arraySongPointers, Difficulty dc )
{
	song_sort_val.clear();
	for(unsigned i = 0; i < arraySongPointers.size(); ++i)
	{
		Steps* pNotes = arraySongPointers[i]->GetStepsByDifficulty( GAMESTATE->GetCurrentStyleDef()->m_StepsType, dc );
		song_sort_val[arraySongPointers[i]] = ssprintf("%i", pNotes ? pNotes->GetMeter() : 0);
	}
	stable_sort( arraySongPointers.begin(), arraySongPointers.end(), CompareSongPointersBySortValueAscending );
}

Song::SelectionDisplay Song::GetDisplayed() const
{
	if( !PREFSMAN->m_bHiddenSongs )
		return SHOW_ALWAYS;
	return m_SelectionDisplay;
}

bool Song::HasMusic() const 		{return m_sMusicFile != ""			&&	IsAFile(GetMusicPath()); }
bool Song::HasBanner() const 		{return m_sBannerFile != ""			&&  IsAFile(GetBannerPath()); }
bool Song::HasLyrics() const		{return m_sLyricsFile != ""			&&	IsAFile(GetLyricsPath()); }
bool Song::HasBackground() const 	{return m_sBackgroundFile != ""		&&  IsAFile(GetBackgroundPath()); }
bool Song::HasCDTitle() const 		{return m_sCDTitleFile != ""		&&  IsAFile(GetCDTitlePath()); }
bool Song::HasBGChanges() const 	{return !m_BackgroundChanges.empty(); }


/* Note that supplying a path relative to the top-level directory is only for compatibility
 * with DWI.  We prefer paths relative to the song directory. */
CString Song::GetMusicPath() const
{
	/* If there's no path in the music file, the file is in the same directory
	 * as the song.  (This is the preferred configuration.) */
	if( m_sMusicFile.Find('/') == -1)
		return m_sSongDir+m_sMusicFile;

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
	if( m_sCDTitleFile.Find('/') == -1 )
		return m_sSongDir+m_sCDTitleFile;
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

void Song::RemoveNotes( const Steps* pNotes )
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
	this->GetSteps( vNotes, GAMESTATE->GetCurrentStyleDef()->m_StepsType );
	for( unsigned j=0; j<vNotes.size(); j++ )
	{
		Steps* pSteps = vNotes[j];
		
		if( PROFILEMAN->GetMachineProfile()->GetStepsHighScoreList(pSteps).GetTopScore().grade == g )
			iCount++;
	}
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

void Song::FreeAllLoadedFromProfiles()
{
	for( unsigned s=0; s<m_apNotes.size(); s++ )
	{
		Steps* pSteps = m_apNotes[s];
		if( pSteps->WasLoadedFromProfile() )
			this->RemoveNotes( pSteps );
	}
}
