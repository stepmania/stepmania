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
#include "ImageCache.h"
#include "ProfileManager.h"
#include "Sprite.h"
#include "RageFile.h"
#include "RageFileManager.h"
#include "RageSurface.h"
#include "RageTextureManager.h"
#include "NoteDataUtil.h"
#include "SongUtil.h"
#include "SongManager.h"
#include "StepsUtil.h"
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
#include "ActorUtil.h"
#include "CommonMetrics.h"

#include <cfloat>
#include <cmath>
#include <cstddef>
#include <ctime>
#include <set>
#include <vector>

//-Nick12 Used for song file hashing
#include <CryptManager.h>

/**
 * @brief The internal version of the cache for StepMania.
 *
 * Increment this value to invalidate the current cache. */
const int FILE_CACHE_VERSION = 227;

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
	m_loaded_from_autosave= false;
	split( CommonMetrics::IMAGES_TO_CACHE, ",", ImageDir );
}

Song::~Song()
{
	for (Steps *s : m_vpSteps)
	{
		SAFE_DELETE( s );
	}
	m_vpSteps.clear();
	for (Steps *s : m_UnknownStyleSteps)
	{
		SAFE_DELETE(s);
	}
	m_UnknownStyleSteps.clear();

	// It's the responsibility of the owner of this Song to make sure
	// that all pointers to this Song and its Steps are invalidated.
}

void Song::DetachSteps()
{
	m_vpSteps.clear();
	FOREACH_ENUM( StepsType, st )
		m_vpStepsByType[st].clear();
	m_UnknownStyleSteps.clear();
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
	for (Steps *s : m_vpSteps)
	{
		SAFE_DELETE( s );
	}
	m_vpSteps.clear();
	FOREACH_ENUM( StepsType, st )
		m_vpStepsByType[st].clear();
	for (Steps *s : m_UnknownStyleSteps)
	{
		SAFE_DELETE(s);
	}
	m_UnknownStyleSteps.clear();

	Song empty;
	*this = empty;

	// It's the responsibility of the owner of this Song to make sure
	// that all pointers to this Song and its Steps are invalidated.
}


void Song::AddBackgroundChange( BackgroundLayer iLayer, BackgroundChange seg )
{
	// Delete old background change at this start beat, if any.
	auto &changes = GetBackgroundChanges(iLayer);
	for (std::vector<BackgroundChange>::iterator bgc = changes.begin(); bgc != changes.end(); ++bgc)
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
static std::set<RString> BlacklistedImages;

/* If PREFSMAN->m_bFastLoad is true, always load from cache if possible.
 * Don't read the contents of sDir if we can avoid it. That means we can't call
 * HasMusic(), HasBanner() or GetHashForDirectory().
 * If true, check the directory hash and reload the song from scratch if it's changed.
 */
bool Song::LoadFromSongDir(RString sDir, bool load_autosave, ProfileSlot from_profile)
{
//	LOG->Trace( "Song::LoadFromSongDir(%s)", sDir.c_str() );
	ASSERT_M( sDir != "", "Songs can't be loaded from an empty directory!" );

	// make sure there is a trailing slash at the end of sDir
	if( sDir.Right(1) != "/" )
		sDir += "/";

	// save song dir
	m_sSongDir = sDir;

	bool use_cache = true;

	std::vector<RString> sDirectoryParts;
	split( m_sSongDir, "/", sDirectoryParts, false );
	m_sSongName = sDirectoryParts[sDirectoryParts.size() - 2];
	ASSERT(m_sSongName != "");
	// save group name
	if(from_profile == ProfileSlot_Invalid)
	{
		ASSERT( sDirectoryParts.size() >= 4 ); /* e.g. "/Songs/Slow/Taps/" */
		m_sGroupName = sDirectoryParts[sDirectoryParts.size()-3];	// second from last item
		ASSERT( m_sGroupName != "" );
	}
	else
	{
		m_LoadedFromProfile= from_profile;
		m_sGroupName= PROFILEMAN->GetProfile(from_profile)->m_sDisplayName;
		use_cache= false;
	}

	RString cache_file_path;
	if(m_LoadedFromProfile == ProfileSlot_Invalid)
	{
		// First, look in the cache for this song (without loading NoteData)
		unsigned uCacheHash = SONGINDEX->GetCacheHash(m_sSongDir);
		cache_file_path = GetCacheFilePath();

		if( !DoesFileExist(cache_file_path) )
		{ use_cache = false; }
		else if(!PREFSMAN->m_bFastLoad && GetHashForDirectory(m_sSongDir) != uCacheHash)
		{ use_cache = false; } // this cache is out of date
		else if(load_autosave)
		{ use_cache= false; }
	}

	if(use_cache)
	{
		/*
		LOG->Trace("Loading '%s' from cache file '%s'.",
				   m_sSongDir.c_str(),
				   GetCacheFilePath().c_str());
		*/
		SSCLoader loaderSSC;
		bool bLoadedFromSSC = loaderSSC.LoadFromSimfile( cache_file_path, *this, true );
		if( !bLoadedFromSSC )
		{
			// load from .sm
			SMLoader loaderSM;
			loaderSM.LoadFromSimfile( cache_file_path, *this, true );
			loaderSM.TidyUpData( *this, true );
		}
		if(m_sMainTitle == "" || (m_sMusicFile == "" && m_vsKeysoundFile.empty()))
		{
			LOG->Warn("Main title or music file for '%s' came up blank, forced to fall back on TidyUpData to fix title and paths.  Do not use # or ; in a song title.", m_sSongDir.c_str());
			// Tell TidyUpData that it's not loaded from the cache because it needs
			// to hit the song folder to find the files that weren't found. -Kyz
			TidyUpData(false, false);
		}
	}
	else
	{
		// There was no entry in the cache for this song, or it was out of date.
		// Let's load it from a file, then write a cache entry.

		if(!NotesLoader::LoadFromDir(sDir, *this, BlacklistedImages, load_autosave))
		{
			LOG->UserLog( "Song", sDir, "has no SSC, SM, SMA, DWI, BMS, or KSF files." );

			std::vector<RString> audios;
			FILEMAN->GetDirListingWithMultipleExtensions(sDir,
				ActorUtil::GetTypeExtensionList(FT_Sound), audios);
			bool has_music = !audios.empty();

			if(!has_music)
			{
				LOG->UserLog( "Song", sDir, "has no music file either. Ignoring this song directory." );
				return false;
			}
			// Make sure we have a future filename figured out.
			std::vector<RString> folders;
			split(sDir, "/", folders);
			RString songName = folders[2] + ".ssc";
			this->m_sSongFileName = sDir + songName;
			// Continue on with a blank Song so that people can make adjustments using the editor.
		}
		// If edits are not cached, looking for them causes a substantial hit to
		// loading time. -Kyz
		LoadEditsFromSongDir(sDir);

		TidyUpData(false, true);

		// Don't save a cache file if the autosave is being loaded, because the
		// cache file would contain the autosave filename. -Kyz
		// Songs loaded from removable profile are never cached, on the
		// assumption that they'll change frequently and have invalid cache
		// entries. -Kyz
		if(!load_autosave && m_LoadedFromProfile == ProfileSlot_Invalid)
		{
			// save a cache file so we don't have to parse it all over again next time
			if(!SaveToCacheFile())
			{ cache_file_path = RString(); }
		}
	}

	if(m_LoadedFromProfile != ProfileSlot_Invalid)
	{
		if(m_fMusicLengthSeconds > PREFSMAN->m_custom_songs_max_seconds)
		{
			LOG->Trace("Custom song %s is too long.", m_sSongDir.c_str());
			return false;
		}
		RageFile music;
		if(!music.Open(GetMusicPath(), RageFile::READ))
		{
			LOG->Trace("Custom song %s could not open music.", m_sSongDir.c_str());
			return false;
		}
		if(music.GetFileSize() > PREFSMAN->m_custom_songs_max_megabytes * 1000000)
		{
			LOG->Trace("Custom song %s music file is too big.", m_sSongDir.c_str());
			return false;
		}
		m_pre_customify_song_dir= m_sSongDir;
		m_sSongDir= custom_songify_path(m_sSongDir);
		m_sBannerFile.clear();
		m_sJacketFile.clear();
		m_sCDFile.clear();
		m_sDiscFile.clear();
		m_sLyricsFile.clear();
		m_sBackgroundFile.clear();
		m_sCDTitleFile.clear();
	}

	for (Steps *s : m_vpSteps)
	{
		if(m_LoadedFromProfile != ProfileSlot_Invalid)
		{
			s->ChangeFilenamesForCustomSong();
		}
		/* Compress all Steps. During initial caching, this will remove cached
		 * NoteData; during cached loads, this will just remove cached SMData. */
		s->Compress();
	}

	// Load the cached Images, if it's not loaded already.
	if( PREFSMAN->m_ImageCache == IMGCACHE_LOW_RES_PRELOAD )
	{
		for( RString Image : ImageDir )
		{
			IMAGECACHE->LoadImage( Image, GetCacheFile( Image ) );
		}
	}

	// Add AutoGen pointers. (These aren't cached.)
	AddAutoGenNotes();

	if( !m_bHasMusic )
	{
		LOG->UserLog( "Song", sDir, "has no music; ignored." );
		return false;	// don't load this song
	}
	return true;	// do load this song
}

/* This function feels EXTREMELY hacky - copying things on top of pointers so
 * they don't break elsewhere.  Maybe it could be rewritten to politely ask the
 * Song/Steps objects to reload themselves. -- djpohly */
bool Song::ReloadFromSongDir( RString sDir )
{
	// Remove the cache file to force the song to reload from its dir instead
	// of loading from the cache. -Kyz
	FILEMAN->Remove(GetCacheFilePath());

	RemoveAutoGenNotes();
	std::vector<Steps*> vOldSteps = m_vpSteps;

	Song copy;
	if( !copy.LoadFromSongDir( sDir ) )
		return false;
	copy.RemoveAutoGenNotes();
	*this = copy;

	/* Go through the steps, first setting their Song pointer to this song
	 * (instead of the copy used above), and constructing a map to let us
	 * easily find the new steps. */
	std::map<StepsID, Steps*> mNewSteps;
	for( std::vector<Steps*>::const_iterator it = m_vpSteps.begin(); it != m_vpSteps.end(); ++it )
	{
		(*it)->m_pSong = this;

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
	for( std::vector<Steps*>::const_iterator itOld = vOldSteps.begin(); itOld != vOldSteps.end(); ++itOld )
	{
		StepsID id;
		id.FromSteps( *itOld );
		std::map<StepsID, Steps*>::iterator itNew = mNewSteps.find( id );
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
	for( std::map<StepsID, Steps*>::const_iterator it = mNewSteps.begin(); it != mNewSteps.end(); ++it )
	{
		Steps *NewSteps = new Steps(this);
		*NewSteps = *(it->second);
		AddSteps( NewSteps );
	}

	AddAutoGenNotes();
	// Reload any images associated with the song. -Kyz
	std::vector<RString> to_reload;
	to_reload.reserve(7);
	to_reload.push_back(m_sBannerFile);
	to_reload.push_back(m_sJacketFile);
	to_reload.push_back(m_sCDFile);
	to_reload.push_back(m_sDiscFile);
	to_reload.push_back(m_sBackgroundFile);
	to_reload.push_back(m_sCDTitleFile);
	to_reload.push_back(m_sPreviewVidFile);
	for(std::vector<RString>::iterator file= to_reload.begin(); file != to_reload.end(); ++file)
	{
		RageTextureID id(*file);
		if(TEXTUREMAN->IsTextureRegistered(id))
		{
			RageTexture* tex= TEXTUREMAN->LoadTexture(id);
			if(tex)
			{
				tex->Reload();
			}
		}
	}
	return true;
}

void Song::LoadEditsFromSongDir(RString dir)
{
	// Load any .edit files in the song folder.
	// Doing this BEFORE setting up AutoGen just in case.
	std::vector<RString> vs;
	GetDirListing(dir + "*.edit", vs, false, false);
	// XXX: I'm sure there's a StepMania way of doing this, but familiar with this codebase I am not.
	for(unsigned int i = 0; i < vs.size(); ++i)
	{
		// Try SSCLoader
		SSCLoader ldSSC;
		if(ldSSC.LoadEditFromFile(dir + vs[i], ProfileSlot_Invalid, true, this) != true)
		{
			// No dice? Try SMLoader then. If SMLoader fails too, well whatever.
			// We don't have to do anything to fail gracefully.
			SMLoader ldSM;
			ldSM.LoadEditFromFile(dir + vs[i], ProfileSlot_Invalid, true, this);
		}
	}
	// Note: If vs.empty() then this loop is skipped entirely (vs.size() == 0)
}

bool Song::HasAutosaveFile()
{
	if(m_sSongFileName.empty())
	{
		return false;
	}
	RString autosave_path= SetExtension(m_sSongFileName, "ats");
	return FILEMAN->DoesFileExist(autosave_path);
}

bool Song::LoadAutosaveFile()
{
	if(m_sSongFileName.empty())
	{
		return false;
	}
	// Save these strings because they need to be restored after the reset.
	// The filenames need to point to the original instead of the autosave for
	// things like load from disk to work. -Kyz
	RString dir= GetSongDir();
	RString song_timing_file= m_SongTiming.m_sFile;
	RString song_file= m_sSongFileName;
	// Reset needs to be used to remove all the steps and other things that
	// will be loaded from the autosave. -Kyz
	Reset();
	if(LoadFromSongDir(dir, true))
	{
		m_loaded_from_autosave= true;
		m_sSongFileName= song_file;
		m_SongTiming.m_sFile= song_timing_file;
		return true;
	}
	// Loading the autosave failed, reload the original. -Kyz
	LoadFromSongDir(dir, false);
	return false;
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
void Song::TidyUpData( bool from_cache, bool /* duringCache */ )
{
	// We need to do this before calling any of HasMusic, HasHasCDTitle, etc.
	ASSERT_M(m_sSongDir.Left(3) != "../", m_sSongDir); // meaningless
	FixupPath(m_sSongDir, "");
	FixupPath(m_sMusicFile, m_sSongDir);
	FOREACH_ENUM(InstrumentTrack, i)
	{ if(!m_sInstrumentTrackFile[i].empty())
		{ FixupPath(m_sInstrumentTrackFile[i], m_sSongDir); }	}
	FixupPath(m_sBannerFile, m_sSongDir);
	FixupPath(m_sJacketFile, m_sSongDir);
	FixupPath(m_sCDFile, m_sSongDir);
	FixupPath(m_sDiscFile, m_sSongDir);
	FixupPath(m_sLyricsFile, m_sSongDir);
	FixupPath(m_sBackgroundFile, m_sSongDir);
	FixupPath(m_sCDTitleFile, m_sSongDir);

	CHECKPOINT_M("Looking for images...");

	m_SongTiming.TidyUpData(false);

	for (Steps *s : m_vpSteps)
	{
		s->m_Timing.TidyUpData(true);
	}

	if(!from_cache)
	{
		if (this->m_sArtist == "The Dancing Monkeys Project" && this->m_sMainTitle.find_first_of('-') != std::string::npos)
		{
			// Dancing Monkeys had a bug/feature where the artist was replaced. Restore it.
			std::vector<RString> titleParts;
			split(this->m_sMainTitle, "-", titleParts);
			this->m_sArtist = titleParts.front();
			Trim(this->m_sArtist);
			titleParts.erase(titleParts.begin());
			this->m_sMainTitle = join("-", titleParts);
			Trim(this->m_sMainTitle);
		}

		Trim(m_sMainTitle);
		Trim(m_sSubTitle);
		Trim(m_sArtist);

		// Fall back on the song directory name.
		if(m_sMainTitle == "")
		{
			NotesLoader::GetMainAndSubTitlesFromFullTitle(
				Basename(this->GetSongDir()), m_sMainTitle, m_sSubTitle);
		}

		if(m_sArtist == "")
		{ m_sArtist = "Unknown artist"; }
		TranslateTitles();

		// Set the has flags before tidying so that tidying can check them instead
		// of using the has functions that hit the disk. -Kyz
		// These will be written to cache, for Song::LoadFromSongDir to use later.
		m_bHasMusic = HasMusic();
		m_bHasBanner = HasBanner();
		m_bHasBackground = HasBackground();

		for( RString Image : ImageDir )
		{
			IMAGECACHE->LoadImage( Image, GetCacheFile( Image ) );
		}

		// There are several things that need to find a file from the dir with a
		// particular extension or type of extension.  So fetch a list of all
		// files in the dir once, then split that list into the different things
		// we need. -Kyz
		std::vector<RString> song_dir_listing;
		FILEMAN->GetDirListing(m_sSongDir + "*", song_dir_listing, false, false);
		std::vector<RString> music_list;
		std::vector<RString> image_list;
		std::vector<RString> movie_list;
		std::vector<RString> lyric_list;
		std::vector<RString> lyric_extensions(1, "lrc");
		// Using a pair didn't work, so these two vectors have to be kept in
		// sync instead. -Kyz
		std::vector<std::vector<RString>*> lists_to_fill;
		std::vector<const std::vector<RString>*> fill_exts;
		lists_to_fill.reserve(4);
		fill_exts.reserve(4);
		lists_to_fill.push_back(&music_list);
		fill_exts.push_back(&ActorUtil::GetTypeExtensionList(FT_Sound));
		// Disable bg and banner images and movies for custom songs.  Loading
		// them takes too long. -Kyz
		if(m_LoadedFromProfile == ProfileSlot_Invalid)
		{
			lists_to_fill.push_back(&image_list);
			fill_exts.push_back(&ActorUtil::GetTypeExtensionList(FT_Bitmap));
			lists_to_fill.push_back(&movie_list);
			fill_exts.push_back(&ActorUtil::GetTypeExtensionList(FT_Movie));
		}
		lists_to_fill.push_back(&lyric_list);
		fill_exts.push_back(&lyric_extensions);
		for(std::vector<RString>::iterator filename= song_dir_listing.begin();
				filename != song_dir_listing.end(); ++filename)
		{
			bool matched_something= false;
			RString file_ext= GetExtension(*filename).MakeLower();
			if(!file_ext.empty())
			{
				for(std::size_t tf= 0; tf < lists_to_fill.size(); ++ tf)
				{
					for(std::vector<RString>::const_iterator ext= fill_exts[tf]->begin();
							ext != fill_exts[tf]->end(); ++ext)
					{
						if(file_ext == *ext)
						{
							lists_to_fill[tf]->push_back(*filename);
							matched_something= true;
							break;
						}
					}
					if(matched_something)
					{
						break;
					}
				}
			}
		}

		if(!m_bHasMusic)
		{
			// If the first song is "intro", and we have more than one available,
			// don't use it--it's probably a KSF intro music file, which we don't
			// (yet) support.
			if(!music_list.empty())
			{
				LOG->Trace("Song '%s' points to a music file that doesn't exist, found music file '%s'", m_sSongDir.c_str(), music_list[0].c_str());
				m_bHasMusic= true;
				m_sMusicFile= music_list[0];
				if(music_list.size() > 1 &&
					!m_sMusicFile.Left(5).CompareNoCase("intro"))
				{
					m_sMusicFile= music_list[1];
				}
			}
		}
		// This must be done before radar calculation.
		if(m_bHasMusic)
		{
			RString error;
			RageSoundReader *Sample = RageSoundReader_FileReader::OpenFile(GetMusicPath(), error);
			/* XXX: Checking if the music file exists eliminates a warning
			 * originating from BMS files (which have no music file, per se)
			 * but it's something of a hack. */
			if(Sample == nullptr && m_sMusicFile != "")
			{
				LOG->UserLog("Sound file", GetMusicPath(), "couldn't be opened: %s", error.c_str());

				// Don't use this file.
				m_sMusicFile = "";
			}
			else if(Sample != nullptr)
			{
				m_fMusicLengthSeconds = Sample->GetLength() / 1000.0f;
				delete Sample;

				if(m_fMusicLengthSeconds < 0)
				{
					// It failed; bad file or something. It's already logged a warning.
					m_fMusicLengthSeconds = 100; // guess
				}
				else if(m_fMusicLengthSeconds == 0)
				{
					LOG->UserLog("Sound file", GetMusicPath(), "is empty.");
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
		if(m_fMusicLengthSeconds < 0)
		{
			LOG->UserLog("Sound file",
				GetMusicPath(),
				"has a negative length %f.",
				m_fMusicLengthSeconds);
			m_fMusicLengthSeconds = 0;
		}
		if(!m_PreviewFile.empty() && m_fMusicSampleLengthSeconds <= 0.00f) { // if there's a preview file and sample length isn't specified, set sample length to length of preview file
			RString error;
			RageSoundReader *Sample = RageSoundReader_FileReader::OpenFile(GetPreviewMusicPath(), error);
			if(Sample == nullptr && m_sMusicFile != "")
			{
				LOG->UserLog("Sound file", GetPreviewMusicPath(), "couldn't be opened: %s", error.c_str());

				// Don't use this file.
				m_PreviewFile = "";
				m_fMusicSampleLengthSeconds = DEFAULT_MUSIC_SAMPLE_LENGTH;
			}
			else if(Sample != nullptr)
			{
				m_fMusicSampleLengthSeconds = Sample->GetLength() / 1000.0f;
				delete Sample;

				if(m_fMusicSampleLengthSeconds < 0)
				{
					// It failed; bad file or something. It's already logged a warning.
					m_fMusicSampleLengthSeconds = DEFAULT_MUSIC_SAMPLE_LENGTH;
				}
				else if(m_fMusicSampleLengthSeconds == 0)
				{
					LOG->UserLog("Sound file", GetPreviewMusicPath(), "is empty.");
				}
			}
		} else { // no preview file, calculate sample from music as normal

			if(m_fMusicSampleStartSeconds == -1 ||
				m_fMusicSampleLengthSeconds == 0 ||
				m_fMusicSampleStartSeconds+m_fMusicSampleLengthSeconds > this->m_fMusicLengthSeconds)
			{
				const TimingData &timing = this->m_SongTiming;
				m_fMusicSampleStartSeconds = timing.GetElapsedTimeFromBeat(100);

				if(m_fMusicSampleStartSeconds+m_fMusicSampleLengthSeconds > this->m_fMusicLengthSeconds)
				{
					// Attempt to get a reasonable default.
					int iBeat = std::lrint(this->m_SongTiming.GetBeatFromElapsedTime(this->GetLastSecond())/2);
					iBeat -= iBeat%4;
					m_fMusicSampleStartSeconds = timing.GetElapsedTimeFromBeat((float)iBeat);
				}
			}

			// The old logic meant that you couldn't have sample lengths that go forever,
			// e.g. those in Donkey Konga. I never liked that. -freem
			if(m_fMusicSampleLengthSeconds <= 0.00f)
			{ m_fMusicSampleLengthSeconds = DEFAULT_MUSIC_SAMPLE_LENGTH; }

		}

		if(!HasLyrics())
		{
			// Check if there is a lyric file in here
			if(!lyric_list.empty())
			{
				m_sLyricsFile= lyric_list[0];
			}
		}

		// Since banner, bg, and cdtitle are disabled for custom songs, don't try
		// to find them. -Kyz
		if(m_LoadedFromProfile == ProfileSlot_Invalid)
		{
			// Here's the problem:  We have a directory full of images. We want to
			// determine which image is the banner, which is the background, and
			// which is the CDTitle.

			// For blank args to FindFirstFilenameContaining. -Kyz
			std::vector<RString> empty_list;

			bool has_jacket= HasJacket();
			bool has_cdimage= HasCDImage();
			bool has_disc= HasDisc();
			bool has_cdtitle= HasCDTitle();

			// First, check the file name for hints.
			if(!m_bHasBanner)
			{
				/* If a nonexistant banner file is specified, and we can't find a
				 * replacement, don't wipe out the old value. */
				//m_sBannerFile = "";

				// find an image with "banner" in the file name
				std::vector<RString> contains(1, "banner");
				/* Some people do things differently for the sake of being different.
				 * Don't match eg. abnormal, numbness. */
				std::vector<RString> ends_with(1, " bn");
				m_bHasBanner= FindFirstFilenameContaining(image_list,
					m_sBannerFile, empty_list, contains, ends_with);
			}

			if(!m_bHasBackground)
			{
				//m_sBackgroundFile = "";

				// find an image with "bg" or "background" in the file name
				std::vector<RString> contains(1, "background");
				std::vector<RString> ends_with(1, "bg");
				m_bHasBackground= FindFirstFilenameContaining(image_list,
					m_sBackgroundFile, empty_list, contains, ends_with);
			}

			if(!has_jacket)
			{
				// find an image with "jacket" or "albumart" in the filename.
				std::vector<RString> starts_with(1, "jk_");
				std::vector<RString> contains;
				contains.reserve(2);
				contains.push_back("jacket");
				contains.push_back("albumart");
				has_jacket= FindFirstFilenameContaining(image_list,
					m_sJacketFile, starts_with, contains, empty_list);
			}

			if(!has_cdimage)
			{
				// CD image, a la ddr 1st-3rd (not to be confused with CDTitles)
				// find an image with "-cd" at the end of the filename.
				std::vector<RString> ends_with(1, "-cd");
				has_cdimage= FindFirstFilenameContaining(image_list,
					m_sCDFile, empty_list, empty_list, ends_with);
			}

			if(!has_disc)
			{
				// a rectangular graphic, not to be confused with CDImage above.
				std::vector<RString> ends_with;
				ends_with.reserve(2);
				ends_with.push_back(" disc");
				ends_with.push_back(" title");
				has_disc= FindFirstFilenameContaining(image_list,
					m_sDiscFile, empty_list, empty_list, ends_with);
			}

			if(!has_cdtitle)
			{
				// find an image with "cdtitle" in the file name
				std::vector<RString> contains(1, "cdtitle");
				has_cdtitle= FindFirstFilenameContaining(image_list,
					m_sCDTitleFile, empty_list, contains, empty_list);
			}

			/* Now, For the images we still haven't found,
			 * look at the image dimensions of the remaining unclassified images. */
			for(unsigned int i= 0; i < image_list.size(); ++i) // foreach image
			{
				if(m_bHasBanner && m_bHasBackground && has_cdtitle)
				break; // done

				// ignore DWI "-char" graphics
				RString lower = image_list[i];
				lower.MakeLower();
				if(BlacklistedImages.find(lower) != BlacklistedImages.end())
				continue;	// skip

				// Skip any image that we've already classified

				if(m_bHasBanner && m_sBannerFile.EqualsNoCase(image_list[i]))
				continue;	// skip

				if(m_bHasBackground && m_sBackgroundFile.EqualsNoCase(image_list[i]))
				continue;	// skip

				if(has_cdtitle && m_sCDTitleFile.EqualsNoCase(image_list[i]))
				continue;	// skip

				if(has_jacket && m_sJacketFile.EqualsNoCase(image_list[i]))
				continue;	// skip

				if(has_disc && m_sDiscFile.EqualsNoCase(image_list[i]))
				continue;	// skip

				if(has_cdimage && m_sCDFile.EqualsNoCase(image_list[i]))
				continue;	// skip

				RString sPath = m_sSongDir + image_list[i];

				// We only care about the dimensions.
				RString error;
				RageSurface *img = RageSurfaceUtils::LoadFile(sPath, error, true);
				if(!img)
				{
					LOG->UserLog("Graphic file", sPath, "couldn't be loaded: %s", error.c_str());
					continue;
				}

				const int width = img->w;
				const int height = img->h;
				delete img;

				if(!m_bHasBackground && width >= 320 && height >= 240)
				{
					m_sBackgroundFile = image_list[i];
					m_bHasBackground= true;
					continue;
				}

				if(!m_bHasBanner && 100 <= width && width <= 320 &&
					50 <= height && height <= 240)
				{
					m_sBannerFile = image_list[i];
					m_bHasBanner= true;
					continue;
				}

				/* Some songs have overlarge banners. Check if the ratio is reasonable
				 * (over 2:1; usually over 3:1), and large (not a cdtitle). */
				if(!m_bHasBanner && width > 200 && float(width) / height > 2.0f)
				{
					m_sBannerFile = image_list[i];
					m_bHasBanner= true;
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
				if(!has_cdtitle && width <= 100 && height <= 48)
				{
					m_sCDTitleFile = image_list[i];
					has_cdtitle= true;
					continue;
				}

				// Jacket files typically have the same width and height.
				if(!has_jacket && width == height)
				{
					m_sJacketFile = image_list[i];
					has_jacket= true;
					continue;
				}

				// Disc images are typically rectangular; make sure we have a banner already.
				if(!has_disc && (width > height) && m_bHasBanner)
				{
					if(image_list[i] != m_sBannerFile)
					{
						m_sDiscFile = image_list[i];
						has_disc= true;
					}
					continue;
				}

				// CD images are the same as Jackets, typically the same width and height
				if(!has_cdimage && width == height)
				{
					m_sCDFile = image_list[i];
					has_cdimage= true;
					continue;
				}
			}
			// If no BGChanges are specified and there are movies in the song
			// directory, then assume they are DWI style where the movie begins at
			// beat 0.
			if(!HasBGChanges())
			{
				/* Use this->GetBeatFromElapsedTime(0) instead of 0 to start when the
				 * music starts. */
				if(movie_list.size() == 1)
				{
					this->AddBackgroundChange(BACKGROUND_LAYER_1,
						BackgroundChange(0, movie_list[0], "", 1.f,
							SBE_StretchNoLoop));
				}
			}
			// Clear fields for files that turned out to not exist.
#define CLEAR_NOT_HAS(has_name, field_name) if(!has_name) { field_name= ""; }
			CLEAR_NOT_HAS(m_bHasBanner, m_sBannerFile);
			CLEAR_NOT_HAS(m_bHasBackground, m_sBackgroundFile);
			CLEAR_NOT_HAS(has_jacket, m_sJacketFile);
			CLEAR_NOT_HAS(has_cdimage, m_sCDFile);
			CLEAR_NOT_HAS(has_disc, m_sDiscFile);
			CLEAR_NOT_HAS(has_cdtitle, m_sCDTitleFile);
#undef CLEAR_NOT_HAS
		}
		// Don't allow multiple Steps of the same StepsType and Difficulty
		// (except for edits). We should be able to use difficulty names as
		// unique identifiers for steps. */
		SongUtil::AdjustDuplicateSteps(this);
	}

	/* Generate these before we autogen notes, so the new notes can inherit
	 * their source's values. */
	ReCalculateRadarValuesAndLastSecond(from_cache, true);
	// If the music length is suspiciously shorter than the last second, adjust
	// the length.  This prevents the ogg patch from setting a false length. -Kyz
	if(m_fMusicLengthSeconds < lastSecond - 10.0f)
	{
		m_fMusicLengthSeconds= lastSecond;
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
				localFirst = std::min(localFirst,
					pSteps->GetTimingData()->GetElapsedTimeFromBeat(tempNoteData.GetFirstBeat()));
				localLast = std::max(localLast,
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
	return SongUtil::GetOneSteps( this, st ) != nullptr;
}

bool Song::HasStepsTypeAndDifficulty( StepsType st, Difficulty dc ) const
{
	return SongUtil::GetOneSteps( this, st, dc ) != nullptr;
}

void Song::Save(bool autosave)
{
	LOG->Trace( "Song::SaveToSongFile()" );

	ReCalculateRadarValuesAndLastSecond();
	TranslateTitles();

	// Save the new files. These calls make backups on their own.
	if( !SaveToSSCFile(GetSongFilePath(), false, autosave) )
		return;
	// Skip saving the cache, sm, and .old files if we are autosaving.  The
	// cache file should not contain the autosave filename. -Kyz
	if(autosave)
	{
		return;
	}
	SaveToCacheFile();
	// If one of the charts uses split timing, then it cannot be accurately
	// saved in the .sm format.  So saving the .sm is disabled.
	if(!AnyChartUsesSplitTiming())
	{
		SaveToSMFile();
	}
	//SaveToDWIFile();

	/* We've safely written our files and created backups. Rename non-SM and
	 * non-DWI files to avoid confusion. */
	std::vector<RString> arrayOldFileNames;
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

	std::vector<Steps*> vpStepsToSave;
	for (Steps *pSteps : m_vpSteps)
	{
		if( pSteps->IsAutogen() )
			continue; // don't write autogen notes

		// Only save steps that weren't loaded from a profile.
		if( pSteps->WasLoadedFromProfile() )
			continue;

		vpStepsToSave.push_back( pSteps );
	}
	for (Steps *s : m_UnknownStyleSteps)
	{
		vpStepsToSave.push_back(s);
	}

	return NotesWriterSM::Write( sPath, *this, vpStepsToSave );

}

bool Song::SaveToSSCFile( RString sPath, bool bSavingCache, bool autosave )
{
	RString path = sPath;
	if (!bSavingCache)
		path = SetExtension(sPath, "ssc");
	if(autosave)
	{
		path = SetExtension(sPath, "ats");
	}

	LOG->Trace( "Song::SaveToSSCFile('%s')", path.c_str() );

	// If the file exists, make a backup.
	if(!bSavingCache && !autosave && IsAFile(path))
		FileCopy( path, path + ".old" );

	std::vector<Steps*> vpStepsToSave;
	for (Steps *pSteps : m_vpSteps)
	{
		if( pSteps->IsAutogen() )
			continue; // don't write autogen notes

		// Only save steps that weren't loaded from a profile.
		if( pSteps->WasLoadedFromProfile() )
			continue;

		if (!bSavingCache)
			pSteps->SetFilename(path);
		vpStepsToSave.push_back( pSteps );
	}
	for (Steps *s : m_UnknownStyleSteps)
	{
		vpStepsToSave.push_back(s);
	}

	if(bSavingCache || autosave)
	{
		return NotesWriterSSC::Write(path, *this, vpStepsToSave, bSavingCache);
	}

	if( !NotesWriterSSC::Write(path, *this, vpStepsToSave, bSavingCache) )
		return false;

	RemoveAutosave();

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
	for (Steps *s : vpStepsToSave)
		s->SetSavedToDisk( true );

	return true;
}

bool Song::SaveToJsonFile( RString sPath )
{
	LOG->Trace( "Song::SaveToJsonFile('%s')", sPath.c_str() );
	return NotesWriterJson::WriteSong(sPath, *this, true);
}

bool Song::SaveToCacheFile()
{
	if(SONGMAN->IsGroupNeverCached(m_sGroupName))
	{
		return true;
	}
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

void Song::RemoveAutosave()
{
	RString autosave_path= SetExtension(m_sSongFileName, "ats");
	if(FILEMAN->DoesFileExist(autosave_path))
	{
		// Change all the steps to point to the actual file, not the autosave
		// file.  -Kyz
		RString extension= GetExtension(m_sSongFileName);
		for(std::size_t i= 0; i < m_vpSteps.size(); ++i)
		{
			if(!m_vpSteps[i]->IsAutogen())
			{
				m_vpSteps[i]->SetFilename(
					SetExtension(m_vpSteps[i]->GetFilename(), extension));
			}
		}
		FILEMAN->Remove(autosave_path);
		m_loaded_from_autosave= false;
	}
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
			const int iTrackDifference = std::abs(iNumTracks-iNumTracksOfMissing);
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
	for (Steps const *s : m_vpSteps)
	{
		if( s->m_StepsType == StepsType_lights_cabinet )
			continue; // ignore
		if( s->GetDifficulty() != Difficulty_Beginner )
			return false;
	}

	return true;
}

bool Song::HasEdits( StepsType st ) const
{
	return std::any_of(m_vpSteps.begin(), m_vpSteps.end(),
		[&](Steps const *step) { return step->m_StepsType == st && step->GetDifficulty() == Difficulty_Edit; });
}

bool Song::NormallyDisplayed() const
{
	return UNLOCKMAN == nullptr || !UNLOCKMAN->SongIsLocked(this);
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

const std::vector<BackgroundChange> &Song::GetBackgroundChanges( BackgroundLayer bl ) const
{
	return *(m_BackgroundChanges[bl]);
}
std::vector<BackgroundChange> &Song::GetBackgroundChanges( BackgroundLayer bl )
{
	return *(m_BackgroundChanges[bl].Get());
}

const std::vector<BackgroundChange> &Song::GetForegroundChanges() const
{
	return *m_ForegroundChanges;
}
std::vector<BackgroundChange> &Song::GetForegroundChanges()
{
	return *m_ForegroundChanges.Get();
}

std::vector<RString> Song::GetChangesToVectorString(const std::vector<BackgroundChange> & changes) const
{
	std::vector<RString> ret;
	for (BackgroundChange const &bgc : changes)
	{
		ret.push_back(bgc.ToString());
	}
	return ret;
}

std::vector<RString> Song::GetBGChanges1ToVectorString() const
{
	return this->GetChangesToVectorString(this->GetBackgroundChanges(BACKGROUND_LAYER_1));
}

std::vector<RString> Song::GetBGChanges2ToVectorString() const
{
	return this->GetChangesToVectorString(this->GetBackgroundChanges(BACKGROUND_LAYER_2));
}

std::vector<RString> Song::GetFGChanges1ToVectorString() const
{
	return this->GetChangesToVectorString(this->GetForegroundChanges());
}

// We want to return a filename, We use this function for that.
RString Song::GetCacheFile(RString sType)
{
	// We put the Predefined images into a map.
	std::map<RString, RString> PreDefs;
	PreDefs["Banner"] = GetBannerPath();
	PreDefs["Background"] = GetBackgroundPath();
	PreDefs["CDTitle"] = GetCDTitlePath();
	PreDefs["Jacket"] = GetJacketPath();
	PreDefs["CDImage"] = GetCDImagePath();
	PreDefs["Disc"] = GetDiscPath();

	// Check if Predefined images exist, And return function if they do.
	if(PreDefs[sType.c_str()])
		return PreDefs[sType.c_str()];

	// Get all image files and put them into a vector.
	std::vector<RString> song_dir_listing;
	FILEMAN->GetDirListing(m_sSongDir + "*", song_dir_listing, false, false);
	std::vector<RString> image_list;
	std::vector<RString> fill_exts = ActorUtil::GetTypeExtensionList(FT_Bitmap);
	for( RString Image : song_dir_listing )
	{
		RString FileExt = GetExtension(Image);
		transform(FileExt.begin(), FileExt.end(), FileExt.begin(),::tolower);
		for ( RString FindExt : fill_exts )
		{
			if(FileExt == FindExt)
				image_list.push_back(Image);
		}
	}

	// Create a map that contains all the filenames to search for.
	std::map<RString, std::map<int, RString>> PreSets;
	PreSets["Banner"][1] = "bn";
	PreSets["Banner"][2] = "banner";
	PreSets["Background"][1] = "bg";
	PreSets["Background"][2] = "background";
	PreSets["CDTitle"][1] = "cdtitle";
	PreSets["Jacket"][1] = "jk_";
	PreSets["Jacket"][2] = "jacket";
	PreSets["Jacket"][3] = "albumart";
	PreSets["CDImage"][1] = "-cd";
	PreSets["Disc"][1] = " disc";
	PreSets["Disc"][2] = " title";

	for( RString Image : image_list)
	{
		// We want to make it lower case.
		transform(Image.begin(), Image.end(), Image.begin(),::tolower);
		for( std::pair<const int, RString> PreSet : PreSets[sType.c_str()] )
		{
			// Search for image using PreSets.
			std::size_t Found = Image.find(PreSet.second.c_str());
			if(Found!=RString::npos)
				return GetSongAssetPath( Image, m_sSongDir );
		}
		// Search for the image directly if it doesnt exist in PreSets,
		// Or incase we define our own stuff.
		std::size_t Found = Image.find(sType.c_str());
		if(Found!=RString::npos)
			return GetSongAssetPath( Image, m_sSongDir );
	}
	// Return empty if nothing found.
	return "";
}

RString Song::GetFileHash()
{
	if (m_sFileHash.empty()) {
		RString sPath = SetExtension(GetSongFilePath(), "sm");
		if (!IsAFile(sPath))
			sPath = SetExtension(GetSongFilePath(), "dwi");
		if (!IsAFile(sPath))
			sPath = SetExtension(GetSongFilePath(), "sma");
		if (!IsAFile(sPath))
			sPath = SetExtension(GetSongFilePath(), "bms");
		if (!IsAFile(sPath))
			sPath = SetExtension(GetSongFilePath(), "ksf");
		if (!IsAFile(sPath))
			sPath = SetExtension(GetSongFilePath(), "json");
		if (!IsAFile(sPath))
			sPath = SetExtension(GetSongFilePath(), "jso");
		if (!IsAFile(sPath))
			sPath = SetExtension(GetSongFilePath(), "ssc");
		if (IsAFile(sPath))
			m_sFileHash = BinaryToHex(CRYPTMAN->GetSHA1ForFile(sPath));
		else
			m_sFileHash = "";
	}
	return m_sFileHash;
}

std::vector<RString> Song::GetInstrumentTracksToVectorString() const
{
	std::vector<RString> ret;
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

RString Song::GetSongAssetPath( RString sPath, const RString &sSongPath )
{
	if( sPath == "" )
		return RString();

	RString sRelPath = sSongPath + sPath;
	if( DoesFileExist(sRelPath) )
		return sRelPath;

	/* If there's no path in the file, the file is in the same directory as the
	 * song. (This is the preferred configuration.) */
	if( sPath.find('/') == std::string::npos )
		return sRelPath;

	// The song contains a path; treat it as relative to the top SM directory.
	if( sPath.Left(3) == "../" )
	{
		// The path begins with "../".  Resolve it wrt. the song directory.
		sPath = sRelPath;
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

RString Song::GetPreviewMusicPath() const
{
	if(m_PreviewFile.empty())
	{
		return GetMusicPath();
	}
	return GetSongAssetPath(m_PreviewFile, m_sSongDir);
}

float Song::GetPreviewStartSeconds() const
{
	if(m_PreviewFile.empty())
	{
		return m_fMusicSampleStartSeconds;
	}
	return 0.0f;
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

RString Song::GetMainTitle() const
{
	return m_sMainTitle;
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
	// Songs of unknown stepstype are saved as a forwards compatibility feature
	// so that editing a simfile made by a future version that has a new style
	// won't delete those steps. -Kyz
	if(pSteps->m_StepsType != StepsType_Invalid)
	{
		m_vpSteps.push_back( pSteps );
		ASSERT_M( pSteps->m_StepsType < NUM_StepsType, ssprintf("%i", pSteps->m_StepsType) );
		m_vpStepsByType[pSteps->m_StepsType].push_back( pSteps );
	}
	else
	{
		m_UnknownStyleSteps.push_back(pSteps);
	}
}

void Song::DeleteSteps( const Steps* pSteps, bool bReAutoGen )
{
	ASSERT( !pSteps->IsAutogen() );

	// Avoid any stale Note::parent pointers by removing all AutoGen'd Steps,
	// then adding them again.

	if( bReAutoGen )
		RemoveAutoGenNotes();

	std::vector<Steps*> &vpSteps = m_vpStepsByType[pSteps->m_StepsType];
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

	// match on song dir or title (ala DWI)
	if( !sSong.CompareNoCase(m_sSongName) )
		return true;
	if( !sSong.CompareNoCase(this->GetTranslitFullTitle()) )
		return true;

	return false;
}

/* If apInUse is set, it contains a list of steps which are in use elsewhere,
 * and should not be deleted. */
void Song::FreeAllLoadedFromProfile( ProfileSlot slot, const std::set<Steps*> *setInUse )
{
	/* DeleteSteps will remove and recreate autogen notes, which may reorder
	 * m_vpSteps, so be careful not to skip over entries. */
	std::vector<Steps*> apToRemove;
	for( int s=m_vpSteps.size()-1; s>=0; s-- )
	{
		Steps* pSteps = m_vpSteps[s];
		if( !pSteps->WasLoadedFromProfile() )
			continue;
		if( slot != ProfileSlot_Invalid && pSteps->GetLoadedFromProfileSlot() != slot )
			continue;
		if( setInUse != nullptr && setInUse->find(pSteps) != setInUse->end() )
			continue;
		apToRemove.push_back( pSteps );
	}

	for( unsigned i = 0; i < apToRemove.size(); ++i )
		this->DeleteSteps( apToRemove[i] );
}

void Song::GetStepsLoadedFromProfile( ProfileSlot slot, std::vector<Steps*> &vpStepsOut ) const
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

bool Song::AnyChartUsesSplitTiming() const
{
	for (Steps *s : m_vpSteps)
	{
		if(!s->m_Timing.empty())
		{
			return true;
		}
	}
	return false;
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
	static int GetMainTitle(T* p, lua_State* L)
	{
		lua_pushstring(L, p->GetMainTitle()); return 1;
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
		const std::vector<Steps*> &v = p->GetAllSteps();
		LuaHelpers::CreateTableFromArray<Steps*>( v, L );
		return 1;
	}
	static int GetStepsByStepsType( T* p, lua_State *L )
	{
		StepsType st = Enum::Check<StepsType>(L, 1);
		const std::vector<Steps*> &v = p->GetStepsByStepsType( st );
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
	static int GetPreviewMusicPath(T* p, lua_State* L)
	{
		RString s= p->GetPreviewMusicPath();
		lua_pushstring(L, s);
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
	static int IsCustomSong(T* p, lua_State* L)
	{
		lua_pushboolean(L, p->m_LoadedFromProfile != ProfileSlot_Invalid);
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
		lua_pushnumber(L, p->GetPreviewStartSeconds());
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
	static int GetBGChanges(T* p, lua_State* L)
	{
		const std::vector<BackgroundChange>& changes= p->GetBackgroundChanges(BACKGROUND_LAYER_1);
		lua_createtable(L, changes.size(), 0);
		for(std::size_t c= 0; c < changes.size(); ++c)
		{
			lua_createtable(L, 0, 8);
			lua_pushnumber(L, changes[c].m_fStartBeat);
			lua_setfield(L, -2, "start_beat");
			lua_pushnumber(L, changes[c].m_fRate);
			lua_setfield(L, -2, "rate");
			LuaHelpers::Push(L, changes[c].m_sTransition);
			lua_setfield(L, -2, "transition");
			LuaHelpers::Push(L, changes[c].m_def.m_sEffect);
			lua_setfield(L, -2, "effect");
			LuaHelpers::Push(L, changes[c].m_def.m_sFile1);
			lua_setfield(L, -2, "file1");
			LuaHelpers::Push(L, changes[c].m_def.m_sFile2);
			lua_setfield(L, -2, "file2");
			LuaHelpers::Push(L, changes[c].m_def.m_sColor1);
			lua_setfield(L, -2, "color1");
			LuaHelpers::Push(L, changes[c].m_def.m_sColor2);
			lua_setfield(L, -2, "color2");
			lua_rawseti(L, -2, c+1);
		}
		return 1;
	}
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
		std::vector<float> fBPMs;
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
	static int ReloadFromSongDir(T* p, lua_State* L)
	{
		p->ReloadFromSongDir();
		COMMON_RETURN_SELF;
	}

	LunaSong()
	{
		ADD_METHOD( GetDisplayFullTitle );
		ADD_METHOD( GetTranslitFullTitle );
		ADD_METHOD( GetDisplayMainTitle );
		ADD_METHOD(GetMainTitle);
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
		ADD_METHOD(IsCustomSong);
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
		ADD_METHOD(GetBGChanges);
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
		ADD_METHOD(GetPreviewMusicPath);
		ADD_METHOD(ReloadFromSongDir);
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
