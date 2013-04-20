#include "global.h"
#include "Song.h"
#include "Steps.h"
#include "RageUtil.h"
#include "RageLog.h"
#include "NoteData.h"
#include "RageSoundReader_FileReader.h"
#include "RageSurface_Load.h"
#include "SongCacheIndex.h"
#include "GameManager.h"
#include "PrefsManager.h"
#include "Style.h"
#include "FontCharAliases.h"
#include "TitleSubstitution.h"
#include "BannerCache.h"
//#include "BackgroundCache.h"
#include "Sprite.h"
#include "RageFileManager.h"
#include "RageSurface.h"
#include "NoteDataUtil.h"
#include "SongUtil.h"
#include "StepsUtil.h"
#include "Foreach.h"
#include "BackgroundUtil.h"
#include "SpecialFiles.h"
#include "NotesLoader.h"
#include "NotesLoaderSM.h"
#include "NotesLoaderSSC.h"
#include "NotesWriterDWI.h"
#include "NotesWriterJson.h"
#include "NotesWriterSM.h"
#include "NotesWriterSSC.h"
#include "UnlockManager.h"
#include "LyricsLoader.h"

#include <time.h>
#include <set>
#include <float.h>


/**
 * @brief The internal version of the cache for StepMania.
 *
 * Increment this value to invalidate the current cache. */
const int FILE_CACHE_VERSION = 208;

/** @brief How long does a song sample last by default? */
const float DEFAULT_MUSIC_SAMPLE_LENGTH = 12.f;

static Preference<float>	g_fLongVerSongSeconds( "LongVerSongSeconds", 60*2.5f );
static Preference<float>	g_fMarathonVerSongSeconds( "MarathonVerSongSeconds", 60*5.f );
static Preference<bool>		g_BackUpAllSongSaves( "BackUpAllSongSaves", false );

static const char *InstrumentTrackNames[] = {
	"Guitar",
	"Rhythm",
	"Bass",
};
XToString( InstrumentTrack );
StringToX( InstrumentTrack );

Song::Song()
{
	FOREACH_BackgroundLayer( i )
		m_BackgroundChanges[i] = AutoPtrCopyOnWrite<VBackgroundChange>(new VBackgroundChange);
	m_ForegroundChanges = AutoPtrCopyOnWrite<VBackgroundChange>(new VBackgroundChange);

	m_LoadedFromProfile = ProfileSlot_Invalid;
	m_fVersion = STEPFILE_VERSION_NUMBER;
	m_fMusicSampleStartSeconds = -1;
	m_fMusicSampleLengthSeconds = DEFAULT_MUSIC_SAMPLE_LENGTH;
	m_fMusicLengthSeconds = 0;
	firstSecond = -1;
	lastSecond = -1;
	specifiedLastSecond = -1;
	m_SelectionDisplay = SHOW_ALWAYS;
	m_bEnabled = true;
	m_DisplayBPMType = DISPLAY_BPM_ACTUAL;
	m_fSpecifiedBPMMin = 0;
	m_fSpecifiedBPMMax = 0;
	m_bIsSymLink = false;
	m_bHasMusic = false;
	m_bHasBanner = false;
	m_bHasBackground = false;
}

Song::~Song()
{
	FOREACH( Steps*, m_vpSteps, s )
		SAFE_DELETE( *s );
	m_vpSteps.clear();
	
	// It's the responsibility of the owner of this Song to make sure
	// that all pointers to this Song and its Steps are invalidated.
}

void Song::DetachSteps()
{
	m_vpSteps.clear();
	FOREACH_ENUM( StepsType, st )
		m_vpStepsByType[st].clear();
}

float Song::GetFirstSecond() const
{
	return this->firstSecond;
}

float Song::GetFirstBeat() const
{
	return this->m_SongTiming.GetBeatFromElapsedTime(this->firstSecond);
}

float Song::GetLastSecond() const
{
	return this->lastSecond;
}

float Song::GetLastBeat() const
{
	return this->m_SongTiming.GetBeatFromElapsedTime(this->lastSecond);
}

float Song::GetSpecifiedLastSecond() const
{
	return this->specifiedLastSecond;
}

float Song::GetSpecifiedLastBeat() const
{
	return this->m_SongTiming.GetBeatFromElapsedTime(this->specifiedLastSecond);
}

void Song::SetFirstSecond(const float f)
{
	this->firstSecond = f;
}

void Song::SetLastSecond(const float f)
{
	this->lastSecond = f;
}

void Song::SetSpecifiedLastSecond(const float f)
{
	this->specifiedLastSecond = f;
}

// Reset to an empty song.
void Song::Reset()
{
	FOREACH( Steps*, m_vpSteps, s )
		SAFE_DELETE( *s );
	m_vpSteps.clear();
	FOREACH_ENUM( StepsType, st )
		m_vpStepsByType[st].clear();

	Song empty;
	*this = empty;

	// It's the responsibility of the owner of this Song to make sure
	// that all pointers to this Song and its Steps are invalidated.
}


void Song::AddBackgroundChange( BackgroundLayer iLayer, BackgroundChange seg )
{
	// Delete old background change at this start beat, if any.
	FOREACH( BackgroundChange, GetBackgroundChanges(iLayer), bgc )
	{
		if( bgc->m_fStartBeat == seg.m_fStartBeat )
		{
			GetBackgroundChanges(iLayer).erase( bgc );
			break;
		}
	}

	ASSERT( iLayer >= 0 && iLayer < NUM_BackgroundLayer );
	BackgroundUtil::AddBackgroundChange( GetBackgroundChanges(iLayer), seg );
}

void Song::AddForegroundChange( BackgroundChange seg )
{
	BackgroundUtil::AddBackgroundChange( GetForegroundChanges(), seg );
}

void Song::AddLyricSegment( LyricSegment seg )
{
	m_LyricSegments.push_back( seg );
}

Steps *Song::CreateSteps()
{
	Steps *pSteps = new Steps(this);
	InitSteps( pSteps );
	return pSteps;
}

void Song::InitSteps(Steps *pSteps)
{
	// TimingData is initially empty (i.e. defaults to song timing)
	pSteps->m_sAttackString = this->m_sAttackString;
	pSteps->m_Attacks = this->m_Attacks;
	pSteps->SetDisplayBPM(this->m_DisplayBPMType);
	pSteps->SetMinBPM(this->m_fSpecifiedBPMMin);
	pSteps->SetMaxBPM(this->m_fSpecifiedBPMMax);
}

void Song::GetDisplayBpms( DisplayBpms &AddTo ) const
{
	if( m_DisplayBPMType == DISPLAY_BPM_SPECIFIED )
	{
		AddTo.Add( m_fSpecifiedBPMMin );
		AddTo.Add( m_fSpecifiedBPMMax );
	}
	else
	{
		float fMinBPM, fMaxBPM;
		m_SongTiming.GetActualBPM( fMinBPM, fMaxBPM );
		AddTo.Add( fMinBPM );
		AddTo.Add( fMaxBPM );
	}
}

const BackgroundChange &Song::GetBackgroundAtBeat( BackgroundLayer iLayer, float fBeat ) const
{
	for( unsigned i=0; i<GetBackgroundChanges(iLayer).size()-1; i++ )
		if( GetBackgroundChanges(iLayer)[i+1].m_fStartBeat > fBeat )
			return GetBackgroundChanges(iLayer)[i];
	return GetBackgroundChanges(iLayer)[0];
}


RString Song::GetCacheFilePath() const
{
	return SongCacheIndex::GetCacheFilePath( "Songs", m_sSongDir );
}

// Get a path to the SM containing data for this song. It might be a cache file.
const RString &Song::GetSongFilePath() const
{
	ASSERT_M( !m_sSongFileName.empty(),
		 ssprintf("The song %s has no filename associated with it!",
			  this->m_sMainTitle.c_str()));
	return m_sSongFileName;
}

/* Hack: This should be a parameter to TidyUpData, but I don't want to pull in
 * <set> into Song.h, which is heavily used. */
static set<RString> BlacklistedImages;

/* If PREFSMAN->m_bFastLoad is true, always load from cache if possible.
 * Don't read the contents of sDir if we can avoid it. That means we can't call
 * HasMusic(), HasBanner() or GetHashForDirectory().
 * If true, check the directory hash and reload the song from scratch if it's changed.
 */
bool Song::LoadFromSongDir( RString sDir )
{
//	LOG->Trace( "Song::LoadFromSongDir(%s)", sDir.c_str() );
	ASSERT_M( sDir != "", "Songs can't be loaded from an empty directory!" );

	// make sure there is a trailing slash at the end of sDir
	if( sDir.Right(1) != "/" )
		sDir += "/";

	// save song dir
	m_sSongDir = sDir;

	// save group name
	vector<RString> sDirectoryParts;
	split( m_sSongDir, "/", sDirectoryParts, false );
	ASSERT( sDirectoryParts.size() >= 4 ); /* e.g. "/Songs/Slow/Taps/" */
	m_sGroupName = sDirectoryParts[sDirectoryParts.size()-3];	// second from last item
	ASSERT( m_sGroupName != "" );

	// First, look in the cache for this song (without loading NoteData)
	unsigned uCacheHash = SONGINDEX->GetCacheHash(m_sSongDir);
	bool bUseCache = true;
	RString sCacheFilePath = GetCacheFilePath();

	if( !DoesFileExist(sCacheFilePath) )
		bUseCache = false;
	if( !PREFSMAN->m_bFastLoad && GetHashForDirectory(m_sSongDir) != uCacheHash )
		bUseCache = false; // this cache is out of date 

	if( bUseCache )
	{
		/*
		LOG->Trace("Loading '%s' from cache file '%s'.",
				   m_sSongDir.c_str(),
				   GetCacheFilePath().c_str());
		*/
		SSCLoader loaderSSC;
		bool bLoadedFromSSC = loaderSSC.LoadFromSimfile( sCacheFilePath, *this, true );
		if( !bLoadedFromSSC )
		{
			// load from .sm
			SMLoader loaderSM;
			loaderSM.LoadFromSimfile( sCacheFilePath, *this, true );
			loaderSM.TidyUpData( *this, true );
		}
	}
	else
	{
		// There was no entry in the cache for this song, or it was out of date.
		// Let's load it from a file, then write a cache entry.

		if( !NotesLoader::LoadFromDir(sDir, *this, BlacklistedImages) )
		{
			LOG->UserLog( "Song", sDir, "has no SSC, SM, SMA, DWI, BMS, or KSF files." );

			vector<RString> vs;
			GetDirListing( sDir + "*.mp3", vs, false, false ); 
			GetDirListing( sDir + "*.oga", vs, false, false ); 
			GetDirListing( sDir + "*.ogg", vs, false, false ); 
			bool bHasMusic = !vs.empty();

			if( !bHasMusic )
			{
				LOG->UserLog( "Song", sDir, "has no music file either. Ignoring this song directory." );
				return false;
			}
			// Make sure we have a future filename figured out.
			vector<RString> folders;
			split(sDir, "/", folders);
			RString songName = folders[2] + ".ssc";
			this->m_sSongFileName = sDir + songName;
			// Continue on with a blank Song so that people can make adjustments using the editor.
		}
		TidyUpData(false, true);

		// save a cache file so we don't have to parse it all over again next time
		if( !SaveToCacheFile() )
			sCacheFilePath = RString();
	}

	FOREACH( Steps*, m_vpSteps, s )
	{
		/* Compress all Steps. During initial caching, this will remove cached
		 * NoteData; during cached loads, this will just remove cached SMData. */
		(*s)->Compress();
	}

	// Load the cached banners, if it's not loaded already.
	if( PREFSMAN->m_BannerCache == BNCACHE_LOW_RES_PRELOAD && m_bHasBanner )
		BANNERCACHE->LoadBanner( GetBannerPath() );
	// Load the cached background, if it's not loaded already.
	/*
	if( PREFSMAN->m_BackgroundCache == BGCACHE_LOW_RES_PRELOAD && m_bHasBackground )
		BACKGROUNDCACHE->LoadBackground( GetBackgroundPath() );
	*/

	// Add AutoGen pointers. (These aren't cached.)
	AddAutoGenNotes();

	if( !m_bHasMusic )
	{
		LOG->UserLog( "Song", sDir, "has no music; ignored." );
		return false;	// don't load this song
	}

	return true;	// do load this song
}

bool Song::ReloadFromSongDir( RString sDir )
{
	RemoveAutoGenNotes();
	vector<Steps*> vOldSteps = m_vpSteps;

	Song copy;
	if( !copy.LoadFromSongDir( sDir ) )
		return false;
	copy.RemoveAutoGenNotes();
	*this = copy;

	// First we assemble a map to let us easily find the new steps
	map<StepsID, Steps*> mNewSteps;
	for( vector<Steps*>::const_iterator it = m_vpSteps.begin(); it != m_vpSteps.end(); ++it )
	{
		StepsID id;
		id.FromSteps( *it );
		mNewSteps[id] = *it;
	}

	// Now we wipe out the new pointers, which were shallow copied and not deep copied...
	m_vpSteps.clear();
	FOREACH_ENUM( StepsType, i )
		m_vpStepsByType[i].clear();

	/* Then we copy as many Steps as possible on top of the old pointers.
	 * The only pointers that change are pointers to Steps that are not in the
	 * reverted file, which we delete, and pointers to Steps that are in the
	 * reverted file but not the original *this, which we create new copies of.
	 * We have to go through these hoops because many places assume the Steps
	 * pointers don't change - even though there are other ways they can change,
	 * such as deleting a Steps via the editor. */
	for( vector<Steps*>::const_iterator itOld = vOldSteps.begin(); itOld != vOldSteps.end(); ++itOld )
	{
		StepsID id;
		id.FromSteps( *itOld );
		map<StepsID, Steps*>::iterator itNew = mNewSteps.find( id );
		if( itNew == mNewSteps.end() )
		{
			// This stepchart didn't exist in the file we reverted from
			delete *itOld;
		}
		else
		{
			Steps *OldSteps = *itOld;
			*OldSteps = *(itNew->second);
			AddSteps( OldSteps );
			mNewSteps.erase( itNew );
		}
	}
	// The leftovers in the map are steps that didn't exist before we reverted
	for( map<StepsID, Steps*>::const_iterator it = mNewSteps.begin(); it != mNewSteps.end(); ++it )
	{
		Steps *NewSteps = new Steps(this);
		*NewSteps = *(it->second);
		AddSteps( NewSteps );
	}

	AddAutoGenNotes();
	return true;
}

static void GetImageDirListing( RString sPath, vector<RString> &AddTo )
{
	GetDirListing( sPath + ".png", AddTo, false, false );
	GetDirListing( sPath + ".jpg", AddTo, false, false );
	GetDirListing( sPath + ".jpeg", AddTo, false, false );
	GetDirListing( sPath + ".bmp", AddTo, false, false );
	GetDirListing( sPath + ".gif", AddTo, false, false );
}

/* Fix up song paths. If there's a leading "./", be sure to keep it: it's
 * a signal that the path is from the root directory, not the song directory.
 * Other than a leading "./", song paths must never contain "." or "..". */
void FixupPath( RString &path, const RString &sSongPath )
{
	// Replace backslashes with slashes in all paths.
	FixSlashesInPlace( path );

	/* Many imported files contain erroneous whitespace before or after
	 * filenames. Paths usually don't actually start or end with spaces,
	 * so let's just remove it. */
	Trim( path );
}

// Songs in BlacklistImages will never be autodetected as song images.
void Song::TidyUpData( bool fromCache, bool /* duringCache */ )
{
	// We need to do this before calling any of HasMusic, HasHasCDTitle, etc.
	ASSERT_M( m_sSongDir.Left(3) != "../", m_sSongDir ); // meaningless
	FixupPath( m_sSongDir, "" );
	FixupPath( m_sMusicFile, m_sSongDir );
	FOREACH_ENUM( InstrumentTrack, i )
		if( !m_sInstrumentTrackFile[i].empty() )
			FixupPath( m_sInstrumentTrackFile[i], m_sSongDir );
	FixupPath( m_sBannerFile, m_sSongDir );
	FixupPath( m_sJacketFile, m_sSongDir );
	FixupPath( m_sCDFile, m_sSongDir );
	FixupPath( m_sDiscFile, m_sSongDir );
	FixupPath( m_sLyricsFile, m_sSongDir );
	FixupPath( m_sBackgroundFile, m_sSongDir );
	FixupPath( m_sCDTitleFile, m_sSongDir );

	if( !HasMusic() )
	{
		vector<RString> arrayPossibleMusic;
		GetDirListing( m_sSongDir + RString("*.mp3"), arrayPossibleMusic );
		GetDirListing( m_sSongDir + RString("*.oga"), arrayPossibleMusic );
		GetDirListing( m_sSongDir + RString("*.ogg"), arrayPossibleMusic );
		GetDirListing( m_sSongDir + RString("*.wav"), arrayPossibleMusic );

		if( !arrayPossibleMusic.empty() )
		{
			int idx = 0;
			/* If the first song is "intro", and we have more than one available,
			 * don't use it--it's probably a KSF intro music file, which we don't
			 * (yet) support. */
			if( arrayPossibleMusic.size() > 1 &&
				!arrayPossibleMusic[0].Left(5).CompareNoCase("intro") )
				++idx;

			// we found a match
			m_sMusicFile = arrayPossibleMusic[idx];
		}
	}

	// This must be done before radar calculation.
	if( HasMusic() )
	{
		RString error;
		RageSoundReader *Sample = RageSoundReader_FileReader::OpenFile( GetMusicPath(), error );
		/* XXX: Checking if the music file exists eliminates a warning
		 * originating from BMS files (which have no music file, per se)
		 * but it's something of a hack. */
		if( Sample == NULL && m_sMusicFile != "" )
		{
			LOG->UserLog( "Sound file", GetMusicPath(), "couldn't be opened: %s", error.c_str() );

			// Don't use this file.
			m_sMusicFile = "";
		}
		else if ( Sample != NULL )
		{
			m_fMusicLengthSeconds = Sample->GetLength() / 1000.0f;
			delete Sample;

			if( m_fMusicLengthSeconds < 0 )
			{
				// It failed; bad file or something. It's already logged a warning.
				m_fMusicLengthSeconds = 100; // guess
			}
			else if( m_fMusicLengthSeconds == 0 )
			{
				LOG->UserLog( "Sound file", GetMusicPath(), "is empty." );
			}
		}
	}
	else	// ! HasMusic()
	{
		m_fMusicLengthSeconds = 100; // guess
		LOG->UserLog("Song",
					 GetSongDir(),
					 "has no music file; guessing at %f seconds",
					 m_fMusicLengthSeconds);
	}

	if( m_fMusicLengthSeconds < 0 )
	{
		LOG->UserLog("Sound file",
					 GetMusicPath(),
					 "has a negative length %f.",
					 m_fMusicLengthSeconds);
		m_fMusicLengthSeconds = 0;
	}

	m_SongTiming.TidyUpData( false );
	
	FOREACH( Steps *, m_vpSteps, s )
	{
		(*s)->m_Timing.TidyUpData( true );
	}

	/* Generate these before we autogen notes, so the new notes can inherit
	 * their source's values. */
	ReCalculateRadarValuesAndLastSecond( fromCache, true );

	Trim( m_sMainTitle );
	Trim( m_sSubTitle );
	Trim( m_sArtist );

	// Fall back on the song directory name.
	if( m_sMainTitle == "" )
		NotesLoader::GetMainAndSubTitlesFromFullTitle(Basename(this->GetSongDir()),
													  m_sMainTitle, m_sSubTitle );

	if( m_sArtist == "" )
		m_sArtist = "Unknown artist";
	TranslateTitles();

	if( m_fMusicSampleStartSeconds == -1 ||
		m_fMusicSampleLengthSeconds == 0 ||
		m_fMusicSampleStartSeconds+m_fMusicSampleLengthSeconds > this->m_fMusicLengthSeconds )
	{
		const TimingData &timing = this->m_SongTiming;
		m_fMusicSampleStartSeconds = timing.GetElapsedTimeFromBeat( 100 );

		if( m_fMusicSampleStartSeconds+m_fMusicSampleLengthSeconds > this->m_fMusicLengthSeconds )
		{
			// Attempt to get a reasonable default.
			int iBeat = lrintf(this->m_SongTiming.GetBeatFromElapsedTime(this->GetLastSecond())/2);
			iBeat -= iBeat%4;
			m_fMusicSampleStartSeconds = timing.GetElapsedTimeFromBeat( (float)iBeat );
		}
	}

	// The old logic meant that you couldn't have sample lengths that go forever,
	// e.g. those in Donkey Konga. I never liked that. -freem
	if( m_fMusicSampleLengthSeconds <= 0.00f )
		m_fMusicSampleLengthSeconds = DEFAULT_MUSIC_SAMPLE_LENGTH;

	// Here's the problem:  We have a directory full of images. We want to
	// determine which image is the banner, which is the background, and which
	// is the CDTitle.

	CHECKPOINT_M( "Looking for images..." );

	if( !fromCache )
	{

		// First, check the file name for hints.
		if( !HasBanner() )
		{
			/* If a nonexistant banner file is specified, and we can't find a
			 * replacement, don't wipe out the old value. */
			//m_sBannerFile = "";

			// find an image with "banner" in the file name
			vector<RString> arrayPossibleBanners;
			GetImageDirListing( m_sSongDir + "*banner*", arrayPossibleBanners );

			/* Some people do things differently for the sake of being different.
			 * Don't match eg. abnormal, numbness. */
			GetImageDirListing( m_sSongDir + "* BN", arrayPossibleBanners );

			if( !arrayPossibleBanners.empty() )
				m_sBannerFile = arrayPossibleBanners[0];
		}

		if( !HasBackground() )
		{
			//m_sBackgroundFile = "";

			// find an image with "bg" or "background" in the file name
			vector<RString> arrayPossibleBGs;
			GetImageDirListing( m_sSongDir + "*background*", arrayPossibleBGs );
			// don't match e.g. "subgroup", "hobgoblin", etc.
			GetImageDirListing( m_sSongDir + "*bg", arrayPossibleBGs );
			if( !arrayPossibleBGs.empty() )
				m_sBackgroundFile = arrayPossibleBGs[0];
		}

		if( !HasJacket() )
		{
			// find an image with "jacket" or "albumart" in the filename.
			vector<RString> arrayPossibleJackets;
			GetImageDirListing( m_sSongDir + "jk_*", arrayPossibleJackets );
			GetImageDirListing( m_sSongDir + "*jacket*", arrayPossibleJackets );
			GetImageDirListing( m_sSongDir + "*albumart*", arrayPossibleJackets );
			if( !arrayPossibleJackets.empty() )
				m_sJacketFile = arrayPossibleJackets[0];
		}

		if( !HasCDImage() )
		{
			// CD image, a la ddr 1st-3rd (not to be confused with CDTitles)
			// find an image with "-cd" at the end of the filename.
			vector<RString> arrayPossibleCDImages;
			GetImageDirListing( m_sSongDir + "*-cd", arrayPossibleCDImages );
			if( !arrayPossibleCDImages.empty() )
				m_sCDFile = arrayPossibleCDImages[0];
		}

		if( !HasDisc() )
		{
			// a rectangular graphic, not to be confused with CDImage above.
			vector<RString> arrayPossibleDiscImages;
			GetImageDirListing( m_sSongDir + "* disc", arrayPossibleDiscImages );
			GetImageDirListing( m_sSongDir + "* title", arrayPossibleDiscImages );
			if( !arrayPossibleDiscImages.empty() )
				m_sDiscFile = arrayPossibleDiscImages[0];
		}

		if( !HasCDTitle() )
		{
			// find an image with "cdtitle" in the file name
			vector<RString> arrayPossibleCDTitles;
			GetImageDirListing( m_sSongDir + "*cdtitle*", arrayPossibleCDTitles );
			if( !arrayPossibleCDTitles.empty() )
				m_sCDTitleFile = arrayPossibleCDTitles[0];
		}

		if( !HasLyrics() )
		{
			// Check if there is a lyric file in here
			vector<RString> arrayLyricFiles;
			GetDirListing(m_sSongDir + RString("*.lrc"), arrayLyricFiles );
			if(	!arrayLyricFiles.empty() )
				m_sLyricsFile = arrayLyricFiles[0];
		}

		/* Now, For the images we still haven't found,
		 * look at the image dimensions of the remaining unclassified images. */
		vector<RString> arrayImages;
		GetImageDirListing( m_sSongDir + "*", arrayImages );

		for( unsigned i=0; i<arrayImages.size(); i++ )	// foreach image
		{
			if( HasBanner() && HasCDTitle() && HasBackground() )
				break; // done

			// ignore DWI "-char" graphics
			RString sLower = arrayImages[i];
			sLower.MakeLower();
			if( BlacklistedImages.find(sLower) != BlacklistedImages.end() )
				continue;	// skip

			// Skip any image that we've already classified

			if( HasBanner()  &&  m_sBannerFile.EqualsNoCase(arrayImages[i]) )
				continue;	// skip

			if( HasBackground()  &&  m_sBackgroundFile.EqualsNoCase(arrayImages[i]) )
				continue;	// skip

			if( HasCDTitle()  &&  m_sCDTitleFile.EqualsNoCase(arrayImages[i]) )
				continue;	// skip

			if( HasJacket()  &&  m_sJacketFile.EqualsNoCase(arrayImages[i]) )
				continue;	// skip

			if( HasDisc()  &&  m_sDiscFile.EqualsNoCase(arrayImages[i]) )
				continue;	// skip

			if( HasCDImage()  &&  m_sCDFile.EqualsNoCase(arrayImages[i]) )
				continue;	// skip

			RString sPath = m_sSongDir + arrayImages[i];

			// We only care about the dimensions.
			RString error;
			RageSurface *img = RageSurfaceUtils::LoadFile( sPath, error, true );
			if( !img )
			{
				LOG->UserLog( "Graphic file", sPath, "couldn't be loaded: %s", error.c_str() );
				continue;
			}

			const int width = img->w;
			const int height = img->h;
			delete img;

			if( !HasBackground()  &&  width >= 320  &&  height >= 240 )
			{
				m_sBackgroundFile = arrayImages[i];
				continue;
			}

			if( !HasBanner()  &&  100<=width  &&  width<=320  &&  50<=height  &&  height<=240 )
			{
				m_sBannerFile = arrayImages[i];
				continue;
			}

			/* Some songs have overlarge banners. Check if the ratio is reasonable
			 * (over 2:1; usually over 3:1), and large (not a cdtitle). */
			if( !HasBanner() && width > 200 && float(width) / height > 2.0f )
			{
				m_sBannerFile = arrayImages[i];
				continue;
			}

			/* Agh. DWI's inline title images are triggering this, resulting in
			 * kanji, etc., being used as a CDTitle for songs with none. Some
			 * sample data from random incarnations:
			 *   42x50 35x50 50x50 144x49
			 * It looks like ~50 height is what people use to align to DWI's font.
			 *
			 * My tallest CDTitle is 44. Let's cut off in the middle and hope for
			 * the best. -(who? -aj) */
			/* The proper size of a CDTitle is 64x48 or sometihng. Simfile artists
			 * typically don't give a shit about this (see Cetaka's fucking banner
			 * -sized CDTitle). This is also subverted in certain designs (beta
			 * Mungyodance 3 simfiles, for instance, used the CDTitle to hold
			 * various information about the song in question). As it stands,
			 * I'm keeping this code until I figure out wtf to do -aj
			 */
			if( !HasCDTitle()  &&  width<=100  &&  height<=48 )
			{
				m_sCDTitleFile = arrayImages[i];
				continue;
			}

			// Jacket files typically have the same width and height.
			if( !HasJacket() && width == height )
			{
				m_sJacketFile = arrayImages[i];
				continue;
			}

			// Disc images are typically rectangular; make sure we have a banner already.
			if( !HasDisc() && (width > height) && HasBanner() )
			{
				if( arrayImages[i] != m_sBannerFile )
					m_sDiscFile = arrayImages[i];
				continue;
			}

			// CD images are the same as Jackets, typically the same width and height
			if( !HasCDImage() && width == height )
			{
				m_sCDFile = arrayImages[i];
				continue;
			}
		}

	}
		
	// These will be written to cache, for Song::LoadFromSongDir to use later.
	m_bHasMusic = HasMusic();
	m_bHasBanner = HasBanner();
	m_bHasBackground = HasBackground();

	if( HasBanner() )
		BANNERCACHE->CacheBanner( GetBannerPath() );
	/*
	if( HasBackground() )
		BANNERCACHE->CacheBackground( GetBackgroundPath() );
	*/

	// If no BGChanges are specified and there are movies in the song directory, then assume
	// they are DWI style where the movie begins at beat 0.
	if( (!HasBGChanges() && !fromCache) )
	{
		vector<RString> arrayPossibleMovies;
		GetDirListing( m_sSongDir + RString("*.ogv"), arrayPossibleMovies );
		GetDirListing( m_sSongDir + RString("*.avi"), arrayPossibleMovies );
		GetDirListing( m_sSongDir + RString("*.mpg"), arrayPossibleMovies );
		GetDirListing( m_sSongDir + RString("*.mpeg"), arrayPossibleMovies );
		/* Use this->GetBeatFromElapsedTime(0) instead of 0 to start when the
		 * music starts. */
		if( arrayPossibleMovies.size() == 1 )
			this->AddBackgroundChange(BACKGROUND_LAYER_1,
									BackgroundChange(0,
									arrayPossibleMovies[0],
									"",
									1.f,
									SBE_StretchNoLoop));
	}


	/* Don't allow multiple Steps of the same StepsType and Difficulty (except
	 * for edits). We should be able to use difficulty names as unique
	 * identifiers for steps. */
	if( !fromCache )
	{
		SongUtil::AdjustDuplicateSteps( this );
	}
}

void Song::TranslateTitles()
{
	static TitleSubst tsub("Songs");

	TitleFields title;
	title.LoadFromStrings(m_sMainTitle, m_sSubTitle, m_sArtist,
						  m_sMainTitleTranslit, m_sSubTitleTranslit, m_sArtistTranslit );
	tsub.Subst( title );
	title.SaveToStrings(m_sMainTitle, m_sSubTitle, m_sArtist,
						m_sMainTitleTranslit, m_sSubTitleTranslit, m_sArtistTranslit );
}

void Song::ReCalculateRadarValuesAndLastSecond(bool fromCache, bool duringCache)
{
	if( fromCache && this->GetFirstSecond() >= 0 && this->GetLastSecond() > 0 )
	{
		// this is loaded from cache, then we just have to calculate the radar values.
		for( unsigned i=0; i<m_vpSteps.size(); i++ )
			m_vpSteps[i]->CalculateRadarValues( m_fMusicLengthSeconds );
		return;
	}

	float localFirst = FLT_MAX; // inf
	// Make sure we're at least as long as the specified amount below.
	float localLast = this->specifiedLastSecond;

	for( unsigned i=0; i<m_vpSteps.size(); i++ )
	{
		Steps* pSteps = m_vpSteps[i];

		pSteps->CalculateRadarValues( m_fMusicLengthSeconds );

		// Must initialize before the gotos.
		NoteData tempNoteData;
		pSteps->GetNoteData( tempNoteData );
		
		// calculate lastSecond

		/* 1. If it's autogen, then first/last beat will come from the parent.
		 * 2. Don't calculate with edits unless the song only contains an edit
		 * chart, like those in Mungyodance 3. Otherwise, edits installed on
		 * the machine could extend the length of the song. */
		if( !pSteps->IsAutogen() &&
				!( pSteps->IsAnEdit() && m_vpSteps.size() > 1 ) )
		{
			// Don't set first/last beat based on lights.  They often start very 
			// early and end very late.
			if( pSteps->m_StepsType == StepsType_lights_cabinet )
				continue; // no need to wipe this.

			/* Many songs have stray, empty song patterns. Ignore them, so they
			 * don't force the first beat of the whole song to 0. */
			if( tempNoteData.GetLastRow() != 0 )
			{
				localFirst = min(localFirst,
					pSteps->GetTimingData()->GetElapsedTimeFromBeat(tempNoteData.GetFirstBeat()));
				localLast = max(localLast,
					pSteps->GetTimingData()->GetElapsedTimeFromBeat(tempNoteData.GetLastBeat()));
			}
		}

		// Wipe NoteData
		if (duringCache)
		{
			NoteData dummy;
			dummy.SetNumTracks(tempNoteData.GetNumTracks());
			pSteps->SetNoteData(dummy);
		}
	}

	// Yes, for some reason we can have freaky stuff take place here.
	this->firstSecond = (localFirst < localLast) ? localFirst : 0;
	this->lastSecond = localLast;
}

// Return whether the song is playable in the given style.
bool Song::SongCompleteForStyle( const Style *st ) const
{
	return HasStepsType( st->m_StepsType );
}

bool Song::HasStepsType( StepsType st ) const
{
	return SongUtil::GetOneSteps( this, st ) != NULL;
}

bool Song::HasStepsTypeAndDifficulty( StepsType st, Difficulty dc ) const
{
	return SongUtil::GetOneSteps( this, st, dc ) != NULL;
}

void Song::Save()
{
	LOG->Trace( "Song::SaveToSongFile()" );

	ReCalculateRadarValuesAndLastSecond();
	TranslateTitles();

	// Save the new files. These calls make backups on their own.
	if( !SaveToSSCFile(GetSongFilePath(), false) )
		return;
	SaveToSMFile();
	//SaveToDWIFile();
	SaveToCacheFile();

	/* We've safely written our files and created backups. Rename non-SM and
	 * non-DWI files to avoid confusion. */
	vector<RString> arrayOldFileNames;
	GetDirListing( m_sSongDir + "*.bms", arrayOldFileNames );
	GetDirListing( m_sSongDir + "*.pms", arrayOldFileNames );
	GetDirListing( m_sSongDir + "*.ksf", arrayOldFileNames );

	for( unsigned i=0; i<arrayOldFileNames.size(); i++ )
	{
		const RString sOldPath = m_sSongDir + arrayOldFileNames[i];
		const RString sNewPath = sOldPath + ".old";

		if( !FileCopy( sOldPath, sNewPath ) )
		{
			LOG->UserLog( "Song file", sOldPath, "couldn't be backed up." );
			// Don't remove.
		}
		else
			FILEMAN->Remove( sOldPath );
	}
}

bool Song::SaveToSMFile()
{
	const RString sPath = SetExtension( GetSongFilePath(), "sm" );
	LOG->Trace( "Song::SaveToSMFile(%s)", sPath.c_str() );
	
	// If the file exists, make a backup.
	if( IsAFile(sPath) )
		FileCopy( sPath, sPath + ".old" );
	
	vector<Steps*> vpStepsToSave;
	FOREACH_CONST( Steps*, m_vpSteps, s ) 
	{
		Steps *pSteps = *s;
		if( pSteps->IsAutogen() )
			continue; // don't write autogen notes
		
		// Only save steps that weren't loaded from a profile.
		if( pSteps->WasLoadedFromProfile() )
			continue;
		
		vpStepsToSave.push_back( pSteps );
	}
	
	return NotesWriterSM::Write( sPath, *this, vpStepsToSave );

}

bool Song::SaveToSSCFile( RString sPath, bool bSavingCache )
{
	RString path = sPath;
	if (!bSavingCache)
		path = SetExtension(sPath, "ssc");
	
	LOG->Trace( "Song::SaveToSSCFile('%s')", path.c_str() );

	// If the file exists, make a backup.
	if( !bSavingCache && IsAFile(path) )
		FileCopy( path, path + ".old" );

	vector<Steps*> vpStepsToSave;
	FOREACH_CONST( Steps*, m_vpSteps, s ) 
	{
		Steps *pSteps = *s;
		if( pSteps->IsAutogen() )
			continue; // don't write autogen notes

		// Only save steps that weren't loaded from a profile.
		if( pSteps->WasLoadedFromProfile() )
			continue;

		if (!bSavingCache)
			pSteps->SetFilename(path);
		vpStepsToSave.push_back( pSteps );
	}
	
	if (bSavingCache)
	{
		return NotesWriterSSC::Write(path, *this, vpStepsToSave, bSavingCache);
	}

	if( !NotesWriterSSC::Write(path, *this, vpStepsToSave, bSavingCache) )
		return false;

	if( g_BackUpAllSongSaves.Get() )
	{
		RString sExt = GetExtension( path );
		RString sBackupFile = SetExtension( path, "" );

		time_t cur_time;
		time( &cur_time );
		struct tm now;
		localtime_r( &cur_time, &now );

		sBackupFile += ssprintf( "-%04i-%02i-%02i--%02i-%02i-%02i", 
			1900+now.tm_year, now.tm_mon+1, now.tm_mday, now.tm_hour, now.tm_min, now.tm_sec );
		sBackupFile = SetExtension( sBackupFile, sExt );
		sBackupFile += ssprintf( ".old" );

		if( FileCopy(path, sBackupFile) )
			LOG->Trace( "Backed up %s to %s", path.c_str(), sBackupFile.c_str() );
		else
			LOG->Trace( "Failed to back up %s to %s", path.c_str(), sBackupFile.c_str() );
	}

	// Mark these steps saved to disk.
	FOREACH( Steps*, vpStepsToSave, s )
		(*s)->SetSavedToDisk( true );

	return true;
}

bool Song::SaveToJsonFile( RString sPath )
{
	LOG->Trace( "Song::SaveToJsonFile('%s')", sPath.c_str() );
	return NotesWriterJson::WriteSong(sPath, *this, true);
}

bool Song::SaveToCacheFile()
{
	SONGINDEX->AddCacheIndex(m_sSongDir, GetHashForDirectory(m_sSongDir));
	const RString sPath = GetCacheFilePath();
	return SaveToSSCFile(sPath, true);
}

bool Song::SaveToDWIFile()
{
	const RString sPath = SetExtension( GetSongFilePath(), "dwi" );
	LOG->Trace( "Song::SaveToDWIFile(%s)", sPath.c_str() );

	// If the file exists, make a backup.
	if( IsAFile(sPath) )
		FileCopy( sPath, sPath + ".old" );

	return NotesWriterDWI::Write( sPath, *this );
}


void Song::AddAutoGenNotes()
{
	bool HasNotes[NUM_StepsType];
	memset( HasNotes, 0, sizeof(HasNotes) );
	for( unsigned i=0; i < m_vpSteps.size(); i++ ) // foreach Steps
	{
		if( m_vpSteps[i]->IsAutogen() )
			continue;

		StepsType st = m_vpSteps[i]->m_StepsType;
		HasNotes[st] = true;
	}

	FOREACH_ENUM( StepsType, stMissing )
	{
		if( HasNotes[stMissing] )
			continue;

		// If m_bAutogenSteps is disabled, only autogen lights.
		if( !PREFSMAN->m_bAutogenSteps && stMissing != StepsType_lights_cabinet )
			continue;
		if( !GAMEMAN->GetStepsTypeInfo(stMissing).bAllowAutogen )
			continue;

		// missing Steps of this type
		int iNumTracksOfMissing = GAMEMAN->GetStepsTypeInfo(stMissing).iNumTracks;

		// look for closest match
		StepsType stBestMatch = StepsType_Invalid;
		int			iBestTrackDifference = INT_MAX;

		FOREACH_ENUM( StepsType, st )
		{
			if( !HasNotes[st] )
				continue;

			// has (non-autogen) Steps of this type
			const int iNumTracks = GAMEMAN->GetStepsTypeInfo(st).iNumTracks;
			const int iTrackDifference = abs(iNumTracks-iNumTracksOfMissing);
			if( iTrackDifference < iBestTrackDifference )
			{
				stBestMatch = st;
				iBestTrackDifference = iTrackDifference;
			}
		}

		if( stBestMatch != StepsType_Invalid )
			AutoGen( stMissing, stBestMatch );
	}
}

void Song::AutoGen( StepsType ntTo, StepsType ntFrom )
{
	// int iNumTracksOfTo = GAMEMAN->StepsTypeToNumTracks(ntTo);

	for( unsigned int j=0; j<m_vpSteps.size(); j++ )
	{
		const Steps* pOriginalNotes = m_vpSteps[j];
		if( pOriginalNotes->m_StepsType == ntFrom )
		{
			Steps* pNewNotes = new Steps(this);
			pNewNotes->AutogenFrom( pOriginalNotes, ntTo );
			this->AddSteps( pNewNotes );
		}
	}
}

void Song::RemoveAutoGenNotes()
{
	FOREACH_ENUM( StepsType, st )
	{
		for( int j=m_vpStepsByType[st].size()-1; j>=0; j-- )
		{
			if( m_vpStepsByType[st][j]->IsAutogen() )
			{
				// delete m_vpSteps[j]; // delete below
				m_vpStepsByType[st].erase( m_vpStepsByType[st].begin()+j );
			}
		}
	}

	for( int j=m_vpSteps.size()-1; j>=0; j-- )
	{
		if( m_vpSteps[j]->IsAutogen() )
		{
			delete m_vpSteps[j];
			m_vpSteps.erase( m_vpSteps.begin()+j );
		}
	}
}

bool Song::IsEasy( StepsType st ) const
{
	/* Very fast songs and songs with wide tempo changes are hard for new
	 * players, even if they have beginner steps. */
	DisplayBpms bpms;
	this->GetDisplayBpms(bpms);
	if( bpms.GetMax() >= 250 || bpms.GetMax() - bpms.GetMin() >= 75 )
		return false;

	/* The easy marker indicates which songs a beginner, having selected
	 * "beginner", can play and actually get a very easy song: if there are
	 * actual beginner steps, or if the light steps are 1- or 2-foot. */
	const Steps* pBeginnerNotes = SongUtil::GetStepsByDifficulty( this, st, Difficulty_Beginner );
	if( pBeginnerNotes )
		return true;

	const Steps* pEasyNotes = SongUtil::GetStepsByDifficulty( this, st, Difficulty_Easy );
	if( pEasyNotes && pEasyNotes->GetMeter() == 1 )
		return true;

	return false;
}

bool Song::IsTutorial() const
{
	// A Song is considered a Tutorial if it has only Beginner steps.
	FOREACH_CONST( Steps*, m_vpSteps, s )
	{
		if( (*s)->m_StepsType == StepsType_lights_cabinet )
			continue; // ignore
		if( (*s)->GetDifficulty() != Difficulty_Beginner )
			return false;
	}

	return true;
}

bool Song::HasEdits( StepsType st ) const
{
	for( unsigned i=0; i<m_vpSteps.size(); i++ )
	{
		Steps* pSteps = m_vpSteps[i];
		if( pSteps->m_StepsType == st &&
			pSteps->GetDifficulty() == Difficulty_Edit )
		{
			return true;
		}
	}

	return false;
}

bool Song::NormallyDisplayed() const
{
	return UNLOCKMAN == NULL || !UNLOCKMAN->SongIsLocked(this);
}

bool Song::ShowInDemonstrationAndRanking() const { return !IsTutorial() && NormallyDisplayed(); }


// Hack: see Song::TidyUpData comments.
bool Song::HasMusic() const
{
	// If we have keys, we always have music.
	if( m_vsKeysoundFile.size() != 0 )
		return true;

	return m_sMusicFile != "" && IsAFile(GetMusicPath());
}
bool Song::HasBanner() const
{
	return m_sBannerFile != "" && IsAFile(GetBannerPath());
}
bool Song::HasInstrumentTrack( InstrumentTrack it ) const
{
	return m_sInstrumentTrackFile[it] != "" && IsAFile(GetInstrumentTrackPath(it));
}
bool Song::HasLyrics() const
{
	return m_sLyricsFile != "" && IsAFile(GetLyricsPath());
}
bool Song::HasBackground() const
{
	return m_sBackgroundFile != "" && IsAFile(GetBackgroundPath());
}
bool Song::HasCDTitle() const
{
	return m_sCDTitleFile != ""	&& IsAFile(GetCDTitlePath());
}
bool Song::HasBGChanges() const
{
	FOREACH_BackgroundLayer( i )
	{
		if( !GetBackgroundChanges(i).empty() )
			return true;
	}
	return false;
}
bool Song::HasAttacks() const		{return !m_Attacks.empty(); }
bool Song::HasJacket() const
{
	return m_sJacketFile != ""	&& IsAFile(GetJacketPath());
}
bool Song::HasDisc() const
{
	return m_sDiscFile != ""	&& IsAFile(GetDiscPath());
}
bool Song::HasCDImage() const
{
	return m_sCDFile != ""	&& IsAFile(GetCDImagePath());
}
bool Song::HasPreviewVid() const
{
	return m_sPreviewVidFile != ""	&& IsAFile(GetPreviewVidPath());
}

const vector<BackgroundChange> &Song::GetBackgroundChanges( BackgroundLayer bl ) const
{
	return *(m_BackgroundChanges[bl]);
}
vector<BackgroundChange> &Song::GetBackgroundChanges( BackgroundLayer bl )
{
	return *(m_BackgroundChanges[bl].Get());
}

const vector<BackgroundChange> &Song::GetForegroundChanges() const
{
	return *m_ForegroundChanges;
}
vector<BackgroundChange> &Song::GetForegroundChanges()
{
	return *m_ForegroundChanges.Get();
}

vector<RString> Song::GetChangesToVectorString(const vector<BackgroundChange> & changes) const
{
	vector<RString> ret;
	FOREACH_CONST( BackgroundChange, changes, bgc )
	{
		ret.push_back((*bgc).ToString());
	}
	return ret;
}

vector<RString> Song::GetBGChanges1ToVectorString() const
{
	return this->GetChangesToVectorString(this->GetBackgroundChanges(BACKGROUND_LAYER_1));
}

vector<RString> Song::GetBGChanges2ToVectorString() const
{
	return this->GetChangesToVectorString(this->GetBackgroundChanges(BACKGROUND_LAYER_2));
}

vector<RString> Song::GetFGChanges1ToVectorString() const
{
	return this->GetChangesToVectorString(this->GetForegroundChanges());
}

vector<RString> Song::GetInstrumentTracksToVectorString() const
{
	vector<RString> ret;
	FOREACH_ENUM(InstrumentTrack, it)
	{
		if (this->HasInstrumentTrack(it))
		{
			ret.push_back(InstrumentTrackToString(it)
					  + "="
					  + this->m_sInstrumentTrackFile[it]);
		}
	}
	return ret;
}

RString GetSongAssetPath( RString sPath, const RString &sSongPath )
{
	if( sPath == "" )
		return RString();

	/* If there's no path in the file, the file is in the same directory as the
	 * song. (This is the preferred configuration.) */
	if( sPath.find('/') == string::npos )
		return sSongPath+sPath;

	// The song contains a path; treat it as relative to the top SM directory.
	if( sPath.Left(3) == "../" )
	{
		// The path begins with "../".  Resolve it wrt. the song directory.
		sPath = sSongPath + sPath;
	}

	CollapsePath( sPath );

	/* If the path still begins with "../", then there were an unreasonable number
	 * of them at the beginning of the path. Ignore the path entirely. */
	if( sPath.Left(3) == "../" )
		return RString();

	return sPath;
}

/* Note that supplying a path relative to the top-level directory is only for
 * compatibility with DWI. We prefer paths relative to the song directory. */
RString Song::GetMusicPath() const
{
	return GetSongAssetPath( m_sMusicFile, m_sSongDir );
}

RString Song::GetInstrumentTrackPath( InstrumentTrack it ) const
{
	return GetSongAssetPath( m_sInstrumentTrackFile[it], m_sSongDir );
}

RString Song::GetBannerPath() const
{
	return GetSongAssetPath( m_sBannerFile, m_sSongDir );
}

RString Song::GetLyricsPath() const
{
	return GetSongAssetPath( m_sLyricsFile, m_sSongDir );
}

RString Song::GetCDTitlePath() const
{
	return GetSongAssetPath( m_sCDTitleFile, m_sSongDir );
}

RString Song::GetBackgroundPath() const
{
	return GetSongAssetPath( m_sBackgroundFile, m_sSongDir );
}

RString Song::GetJacketPath() const
{
	return GetSongAssetPath( m_sJacketFile, m_sSongDir );
}

RString Song::GetDiscPath() const
{
	return GetSongAssetPath( m_sDiscFile, m_sSongDir );
}

RString Song::GetCDImagePath() const
{
	return GetSongAssetPath( m_sCDFile, m_sSongDir );
}

RString Song::GetPreviewVidPath() const
{
	return GetSongAssetPath( m_sPreviewVidFile, m_sSongDir );
}

RString Song::GetDisplayMainTitle() const
{
	if(!PREFSMAN->m_bShowNativeLanguage) return GetTranslitMainTitle();
	return m_sMainTitle;
}

RString Song::GetDisplaySubTitle() const
{
	if(!PREFSMAN->m_bShowNativeLanguage) return GetTranslitSubTitle();
	return m_sSubTitle;
}

RString Song::GetDisplayArtist() const
{
	if(!PREFSMAN->m_bShowNativeLanguage) return GetTranslitArtist();
	return m_sArtist;
}

RString Song::GetDisplayFullTitle() const
{
	RString Title = GetDisplayMainTitle();
	RString SubTitle = GetDisplaySubTitle();

	if(!SubTitle.empty()) Title += " " + SubTitle;
	return Title;
}

RString Song::GetTranslitFullTitle() const
{
	RString Title = GetTranslitMainTitle();
	RString SubTitle = GetTranslitSubTitle();

	if(!SubTitle.empty()) Title += " " + SubTitle;
	return Title;
}

void Song::AddSteps( Steps* pSteps )
{
	m_vpSteps.push_back( pSteps );
	ASSERT_M( pSteps->m_StepsType < NUM_StepsType, ssprintf("%i", pSteps->m_StepsType) );
	m_vpStepsByType[pSteps->m_StepsType].push_back( pSteps );
}

void Song::DeleteSteps( const Steps* pSteps, bool bReAutoGen )
{
	ASSERT( !pSteps->IsAutogen() );

	// Avoid any stale Note::parent pointers by removing all AutoGen'd Steps,
	// then adding them again.

	if( bReAutoGen )
		RemoveAutoGenNotes();

	vector<Steps*> &vpSteps = m_vpStepsByType[pSteps->m_StepsType];
	for( int j=vpSteps.size()-1; j>=0; j-- )
	{
		if( vpSteps[j] == pSteps )
		{
			//delete vpSteps[j]; // delete below
			vpSteps.erase( vpSteps.begin()+j );
			break;
		}
	}

	for( int j=m_vpSteps.size()-1; j>=0; j-- )
	{
		if( m_vpSteps[j] == pSteps )
		{
			delete m_vpSteps[j];
			m_vpSteps.erase( m_vpSteps.begin()+j );
			break;
		}
	}

	if( bReAutoGen )
		AddAutoGenNotes();
}

bool Song::Matches(RString sGroup, RString sSong) const
{
	if( sGroup.size() && sGroup.CompareNoCase(this->m_sGroupName) != 0)
		return false;

	RString sDir = this->GetSongDir();
	sDir.Replace("\\","/");
	vector<RString> bits;
	split( sDir, "/", bits );
	ASSERT(bits.size() >= 2); // should always have at least two parts
	const RString &sLastBit = bits[bits.size()-1];

	// match on song dir or title (ala DWI)
	if( !sSong.CompareNoCase(sLastBit) )
		return true;
	if( !sSong.CompareNoCase(this->GetTranslitFullTitle()) )
		return true;

	return false;
}

/* If apInUse is set, it contains a list of steps which are in use elsewhere,
 * and should not be deleted. */
void Song::FreeAllLoadedFromProfile( ProfileSlot slot, const set<Steps*> *setInUse )
{
	/* DeleteSteps will remove and recreate autogen notes, which may reorder
	 * m_vpSteps, so be careful not to skip over entries. */
	vector<Steps*> apToRemove;
	for( int s=m_vpSteps.size()-1; s>=0; s-- )
	{
		Steps* pSteps = m_vpSteps[s];
		if( !pSteps->WasLoadedFromProfile() )
			continue;
		if( slot != ProfileSlot_Invalid && pSteps->GetLoadedFromProfileSlot() != slot )
			continue;
		if( setInUse != NULL && setInUse->find(pSteps) != setInUse->end() )
			continue;
		apToRemove.push_back( pSteps );
	}

	for( unsigned i = 0; i < apToRemove.size(); ++i )
		this->DeleteSteps( apToRemove[i] );
}

void Song::GetStepsLoadedFromProfile( ProfileSlot slot, vector<Steps*> &vpStepsOut ) const
{
	for( unsigned s=0; s<m_vpSteps.size(); s++ )
	{
		Steps* pSteps = m_vpSteps[s];
		if( pSteps->GetLoadedFromProfileSlot() == slot )
			vpStepsOut.push_back( pSteps );
	}
}

int Song::GetNumStepsLoadedFromProfile( ProfileSlot slot ) const
{
	int iCount = 0;
	for( unsigned s=0; s<m_vpSteps.size(); s++ )
	{
		Steps* pSteps = m_vpSteps[s];
		if( pSteps->GetLoadedFromProfileSlot() == slot )
			iCount++;
	}
	return iCount;
}

bool Song::IsEditAlreadyLoaded( Steps* pSteps ) const
{
	ASSERT_M( pSteps->GetDifficulty() == Difficulty_Edit,
			 ssprintf("The %s chart for %s is no edit, thus it can't be checked for loading.",
					  DifficultyToString(pSteps->GetDifficulty()).c_str(),
					  this->m_sMainTitle.c_str()));

	for( unsigned i=0; i<m_vpSteps.size(); i++ )
	{
		Steps* pOther = m_vpSteps[i];
		if( pOther->GetDifficulty() == Difficulty_Edit &&
			pOther->m_StepsType == pSteps->m_StepsType &&
			pOther->GetHash() == pSteps->GetHash() )
		{
			return true;
		}
	}

	return false;
}

bool Song::IsStepsUsingDifferentTiming(Steps *pSteps) const
{
	// XXX This no longer depends on Song at all
	return !pSteps->m_Timing.empty();
}

bool Song::HasSignificantBpmChangesOrStops() const
{
	if( m_SongTiming.HasStops() || m_SongTiming.HasDelays() )
		return true;

	// Don't consider BPM changes that only are only for maintaining sync as 
	// a real BpmChange.
	if( m_DisplayBPMType == DISPLAY_BPM_SPECIFIED )
	{
		if( m_fSpecifiedBPMMin != m_fSpecifiedBPMMax )
			return true;
	}
	else if( m_SongTiming.HasBpmChanges() )
	{
		return true;
	}

	return false;
}

float Song::GetStepsSeconds() const
{
	return this->GetLastSecond() - this->GetFirstSecond();
}

bool Song::IsLong() const
{
	return !IsMarathon() && m_fMusicLengthSeconds >= g_fLongVerSongSeconds;
}

bool Song::IsMarathon() const
{
	return m_fMusicLengthSeconds >= g_fMarathonVerSongSeconds;
}

// lua start
#include "LuaBinding.h"

/** @brief Allow Lua to have access to the Song. */ 
class LunaSong: public Luna<Song>
{
public:
	static int GetDisplayFullTitle( T* p, lua_State *L )
	{
		lua_pushstring(L, p->GetDisplayFullTitle() ); return 1;
	}
	static int GetTranslitFullTitle( T* p, lua_State *L )
	{
		lua_pushstring(L, p->GetTranslitFullTitle() ); return 1;
	}
	static int GetDisplayMainTitle( T* p, lua_State *L )
	{
		lua_pushstring(L, p->GetDisplayMainTitle() ); return 1;
	}
	static int GetTranslitMainTitle( T* p, lua_State *L )
	{
		lua_pushstring(L, p->GetTranslitMainTitle() ); return 1;
	}
	static int GetDisplaySubTitle( T* p, lua_State *L )
	{
		lua_pushstring(L, p->GetDisplaySubTitle() ); return 1;
	}
	static int GetTranslitSubTitle( T* p, lua_State *L )
	{
		lua_pushstring(L, p->GetTranslitSubTitle() ); return 1;
	}
	static int GetDisplayArtist( T* p, lua_State *L )
	{
		lua_pushstring(L, p->GetDisplayArtist() ); return 1;
	}
	static int GetTranslitArtist( T* p, lua_State *L )
	{
		lua_pushstring(L, p->GetTranslitArtist() ); return 1;
	}
	static int GetGenre( T* p, lua_State *L )
	{
		lua_pushstring(L, p->m_sGenre ); return 1;
	}
	static int GetOrigin( T* p, lua_State *L )
	{
		lua_pushstring(L, p->m_sOrigin ); return 1;
	}
	static int GetAllSteps( T* p, lua_State *L )
	{
		const vector<Steps*> &v = p->GetAllSteps();
		LuaHelpers::CreateTableFromArray<Steps*>( v, L );
		return 1;
	}
	static int GetStepsByStepsType( T* p, lua_State *L )
	{
		StepsType st = Enum::Check<StepsType>(L, 1);
		const vector<Steps*> &v = p->GetStepsByStepsType( st );
		LuaHelpers::CreateTableFromArray<Steps*>( v, L );
		return 1;
	}
	static int GetSongDir( T* p, lua_State *L )	
	{ 
		lua_pushstring(L, p->GetSongDir() );
		return 1; 
	}
	static int GetMusicPath( T* p, lua_State *L )
	{
		RString s = p->GetMusicPath();
		if( !s.empty() )
			lua_pushstring(L, s);
		else
			lua_pushnil(L);
		return 1;
	}
	static int GetBannerPath( T* p, lua_State *L )
	{
		RString s = p->GetBannerPath();
		if( !s.empty() )
			lua_pushstring(L, s);
		else
			lua_pushnil(L);
		return 1;
	}
	static int GetBackgroundPath( T* p, lua_State *L )
	{
		RString s = p->GetBackgroundPath();
		if( !s.empty() ) 
			lua_pushstring(L, s);
		else
			lua_pushnil(L);
		return 1; 
	}
	static int GetPreviewVidPath( T* p, lua_State *L )
	{
		RString s = p->GetPreviewVidPath();
		if( !s.empty() ) 
			lua_pushstring(L, s);
		else
			lua_pushnil(L);
		return 1; 
	}
	static int GetJacketPath( T* p, lua_State *L )
	{
		RString s = p->GetJacketPath();
		if( !s.empty() ) 
			lua_pushstring(L, s);
		else
			lua_pushnil(L);
		return 1; 
	}
	static int GetCDImagePath( T* p, lua_State *L )
	{
		RString s = p->GetCDImagePath();
		if( !s.empty() ) 
			lua_pushstring(L, s);
		else
			lua_pushnil(L);
		return 1; 
	}
	static int GetDiscPath( T* p, lua_State *L )
	{
		RString s = p->GetDiscPath();
		if( !s.empty() ) 
			lua_pushstring(L, s);
		else
			lua_pushnil(L);
		return 1; 
	}
	static int GetCDTitlePath( T* p, lua_State *L )
	{ 
		RString s = p->GetCDTitlePath();
		if( !s.empty() )
			lua_pushstring(L, s);
		else
			lua_pushnil(L);
		return 1;
	}
	static int GetLyricsPath( T* p, lua_State *L )
	{
		RString s = p->GetLyricsPath();
		if( !s.empty() )
			lua_pushstring(L, s);
		else
			lua_pushnil(L);
		return 1;
	}
	static int GetSongFilePath(  T* p, lua_State *L )
	{
		lua_pushstring(L, p->GetSongFilePath() );
		return 1;
	}
	static int IsTutorial( T* p, lua_State *L )	
	{
		lua_pushboolean(L, p->IsTutorial());
		return 1;
	}
	static int IsEnabled( T* p, lua_State *L )
	{
		lua_pushboolean(L, p->GetEnabled());
		return 1;
	}
	static int GetGroupName( T* p, lua_State *L )
	{
		lua_pushstring(L, p->m_sGroupName);
		return 1;
	}
	static int MusicLengthSeconds( T* p, lua_State *L )
	{
		lua_pushnumber(L, p->m_fMusicLengthSeconds);
		return 1;
	}
	static int GetSampleStart( T* p, lua_State *L )
	{
		lua_pushnumber(L, p->m_fMusicSampleStartSeconds);
		return 1;
	}
	static int GetSampleLength( T* p, lua_State *L )
	{
		lua_pushnumber(L, p->m_fMusicSampleLengthSeconds);
		return 1;
	}
	static int IsLong( T* p, lua_State *L )	
	{
		lua_pushboolean(L, p->IsLong());
		return 1;
	}
	static int IsMarathon( T* p, lua_State *L )
	{
		lua_pushboolean(L, p->IsMarathon());
		return 1;
	}
	static int HasStepsType( T* p, lua_State *L )
	{
		StepsType st = Enum::Check<StepsType>(L, 1);
		lua_pushboolean( L, p->HasStepsType(st) );
		return 1;
	}
	static int HasStepsTypeAndDifficulty( T* p, lua_State *L )
	{
		StepsType st = Enum::Check<StepsType>(L, 1);
		Difficulty dc = Enum::Check<Difficulty>( L, 2 );
		lua_pushboolean( L, p->HasStepsTypeAndDifficulty(st, dc) );
		return 1;
	}
	static int IsStepsUsingDifferentTiming(T* p, lua_State *L)
	{
		lua_pushboolean(L, p->IsStepsUsingDifferentTiming(Luna<Steps>::check( L, 1, true )));
		return 1;
	}
	/* TODO: HasStepsTypeAndDifficulty and GetOneSteps should be in
	 * a SongUtil Lua table and a method of Steps. */
	static int GetOneSteps( T* p, lua_State *L )
	{
		StepsType st = Enum::Check<StepsType>(L, 1);
		Difficulty dc = Enum::Check<Difficulty>( L, 2 );
		Steps *pSteps = SongUtil::GetOneSteps( p, st, dc );
		if( pSteps ) 
			pSteps->PushSelf(L);
		else 
			lua_pushnil(L);
		return 1;
	}
	static int GetTimingData( T* p, lua_State *L ) { p->m_SongTiming.PushSelf(L); return 1; }
	// has functions
	static int HasMusic( T* p, lua_State *L )			{ lua_pushboolean(L, p->HasMusic()); return 1; }
	static int HasBanner( T* p, lua_State *L )		{ lua_pushboolean(L, p->HasBanner()); return 1; }
	static int HasBackground( T* p, lua_State *L )	{ lua_pushboolean(L, p->HasBackground()); return 1; }
	static int HasPreviewVid( T* p, lua_State *L )	{ lua_pushboolean(L, p->HasPreviewVid()); return 1; }
	static int HasJacket( T* p, lua_State *L )		{ lua_pushboolean(L, p->HasJacket()); return 1; }
	static int HasDisc( T* p, lua_State *L )			{ lua_pushboolean(L, p->HasDisc()); return 1; }
	static int HasCDImage( T* p, lua_State *L )		{ lua_pushboolean(L, p->HasCDImage()); return 1; }
	static int HasCDTitle( T* p, lua_State *L )		{ lua_pushboolean(L, p->HasCDTitle()); return 1; }
	static int HasBGChanges( T* p, lua_State *L )		{ lua_pushboolean(L, p->HasBGChanges()); return 1; }
	static int HasLyrics( T* p, lua_State *L )		{ lua_pushboolean(L, p->HasLyrics()); return 1; }
	// functions that AJ loves
	static int HasSignificantBPMChangesOrStops( T* p, lua_State *L )
	{
		lua_pushboolean(L, p->HasSignificantBpmChangesOrStops());
		return 1;
	}
	static int HasEdits( T* p, lua_State *L )
	{
		StepsType st = Enum::Check<StepsType>(L, 1);
		lua_pushboolean(L, p->HasEdits( st ));
		return 1;
	}
	static int IsEasy( T* p, lua_State *L )
	{
		StepsType st = Enum::Check<StepsType>(L, 1);
		lua_pushboolean(L, p->IsEasy( st ));
		return 1;
	}
	static int GetStepsSeconds( T* p, lua_State *L )
	{
		lua_pushnumber(L, p->GetStepsSeconds());
		return 1;
	}
	static int NormallyDisplayed( T* p, lua_State *L )
	{
		lua_pushboolean(L, p->NormallyDisplayed());
		return 1;
	}
	static int ShowInDemonstrationAndRanking( T* p, lua_State *L )
	{
		lua_pushboolean(L, p->ShowInDemonstrationAndRanking());
		return 1;
	}
	static int GetFirstSecond(T* p, lua_State *L)
	{
		lua_pushnumber(L, p->GetFirstSecond());
		return 1;
	}
	static int GetLastSecond(T* p, lua_State *L)
	{
		lua_pushnumber(L, p->GetLastSecond());
		return 1;
	}
	static int GetFirstBeat( T* p, lua_State *L )
	{ 
		lua_pushnumber(L, p->GetFirstBeat());
		return 1;
	}
	static int GetLastBeat( T* p, lua_State *L )
	{
		lua_pushnumber(L, p->GetLastBeat());
		return 1;
	}
	static int HasAttacks( T* p, lua_State *L )	
	{
		lua_pushboolean(L, p->HasAttacks());
		return 1;
	}
	static int GetDisplayBpms( T* p, lua_State *L )
	{
		DisplayBpms temp;
		p->GetDisplayBpms(temp);
		float fMin = temp.GetMin();
		float fMax = temp.GetMax();
		vector<float> fBPMs;
		fBPMs.push_back( fMin );
		fBPMs.push_back( fMax );
		LuaHelpers::CreateTableFromArray(fBPMs, L);
		return 1;
	}
	static int IsDisplayBpmSecret( T* p, lua_State *L )
	{
		DisplayBpms temp;
		p->GetDisplayBpms(temp);
		lua_pushboolean( L, temp.IsSecret() );
		return 1;
	}
	static int IsDisplayBpmConstant( T* p, lua_State *L )
	{
		DisplayBpms temp;
		p->GetDisplayBpms(temp);
		lua_pushboolean( L, temp.BpmIsConstant() );
		return 1;
	}
	static int IsDisplayBpmRandom( T* p, lua_State *L )
	{
		lua_pushboolean( L, p->m_DisplayBPMType == DISPLAY_BPM_RANDOM );
		return 1;
	}

	LunaSong()
	{
		ADD_METHOD( GetDisplayFullTitle );
		ADD_METHOD( GetTranslitFullTitle );
		ADD_METHOD( GetDisplayMainTitle );
		ADD_METHOD( GetTranslitMainTitle );
		ADD_METHOD( GetDisplaySubTitle );
		ADD_METHOD( GetTranslitSubTitle );
		ADD_METHOD( GetDisplayArtist );
		ADD_METHOD( GetTranslitArtist );
		ADD_METHOD( GetGenre );
		ADD_METHOD( GetOrigin );
		ADD_METHOD( GetAllSteps );
		ADD_METHOD( GetStepsByStepsType );
		ADD_METHOD( GetSongDir );
		ADD_METHOD( GetMusicPath );
		ADD_METHOD( GetBannerPath );
		ADD_METHOD( GetBackgroundPath );
		ADD_METHOD( GetJacketPath );
		ADD_METHOD( GetCDImagePath );
		ADD_METHOD( GetDiscPath );
		ADD_METHOD( GetCDTitlePath );
		ADD_METHOD( GetLyricsPath );
		ADD_METHOD( GetSongFilePath );
		ADD_METHOD( IsTutorial );
		ADD_METHOD( IsEnabled );
		ADD_METHOD( GetGroupName );
		ADD_METHOD( MusicLengthSeconds );
		ADD_METHOD( GetSampleStart );
		ADD_METHOD( GetSampleLength );
		ADD_METHOD( IsLong );
		ADD_METHOD( IsMarathon );
		ADD_METHOD( HasStepsType );
		ADD_METHOD( HasStepsTypeAndDifficulty );
		ADD_METHOD( GetOneSteps );
		ADD_METHOD( GetTimingData );
		ADD_METHOD( HasMusic );
		ADD_METHOD( HasBanner );
		ADD_METHOD( HasBackground );
		ADD_METHOD( HasJacket );
		ADD_METHOD( HasCDImage );
		ADD_METHOD( HasDisc );
		ADD_METHOD( HasCDTitle );
		ADD_METHOD( HasBGChanges );
		ADD_METHOD( HasLyrics );
		ADD_METHOD( HasSignificantBPMChangesOrStops );
		ADD_METHOD( HasEdits );
		ADD_METHOD( IsEasy );
		ADD_METHOD( GetStepsSeconds );
		ADD_METHOD( NormallyDisplayed );
		ADD_METHOD( GetFirstBeat );
		ADD_METHOD( GetFirstSecond );
		ADD_METHOD( GetLastBeat );
		ADD_METHOD( GetLastSecond );
		ADD_METHOD( HasAttacks );
		ADD_METHOD( GetDisplayBpms );
		ADD_METHOD( IsDisplayBpmSecret );
		ADD_METHOD( IsDisplayBpmConstant );
		ADD_METHOD( IsDisplayBpmRandom );
		ADD_METHOD( IsStepsUsingDifferentTiming );
		ADD_METHOD( ShowInDemonstrationAndRanking );
		ADD_METHOD( HasPreviewVid );
		ADD_METHOD( GetPreviewVidPath );
	}   
};

LUA_REGISTER_CLASS( Song )
// lua end


/*
 * (c) 2001-2004 Chris Danford, Glenn Maynard
 * All rights reserved.
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, and/or sell copies of the Software, and to permit persons to
 * whom the Software is furnished to do so, provided that the above
 * copyright notice(s) and this permission notice appear in all copies of
 * the Software and that both the above copyright notice(s) and this
 * permission notice appear in supporting documentation.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT OF
 * THIRD PARTY RIGHTS. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR HOLDERS
 * INCLUDED IN THIS NOTICE BE LIABLE FOR ANY CLAIM, OR ANY SPECIAL INDIRECT
 * OR CONSEQUENTIAL DAMAGES, OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS
 * OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
 * OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */
