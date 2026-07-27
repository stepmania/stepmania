#include "global.h"
#include "SongManager.h"
#include "arch/LoadingWindow/LoadingWindow.h"
#include "ActorUtil.h"
#include "AnnouncerManager.h"
#include "BackgroundUtil.h"
#include "ImageCache.h"
#include "CommonMetrics.h"
#include "Course.h"
#include "CourseLoaderCRS.h"
#include "CourseUtil.h"
#include "GameManager.h"
#include "GameState.h"
#include "LocalizedString.h"
#include "MemoryCardManager.h"
#include "MsdFile.h"
#include "NoteSkinManager.h"
#include "NotesLoaderDWI.h"
#include "NotesLoaderSSC.h"
#include "NotesLoaderSM.h"
#include "PrefsManager.h"
#include "Profile.h"
#include "ProfileManager.h"
#include "RageFile.h"
#include "RageFileManager.h"
#include "RageLog.h"
#include "Song.h"
#include "SongCacheIndex.h"
#include "SongUtil.h"
#include "Sprite.h"
#include "StatsManager.h"
#include "Steps.h"
#include "StepsUtil.h"
#include "Style.h"
#include "ThemeManager.h"
#include "TitleSubstitution.h"
#include "TrailUtil.h"
#include "UnlockManager.h"
#include "SpecialFiles.h"

#include <cstddef>
#include <tuple>
#include <vector>


SongManager*	SONGMAN = nullptr;	// global and accessible from anywhere in our program

/** @brief The file that contains various random attacks. */
const RString ATTACK_FILE		= "/Data/RandomAttacks.txt";
const RString EDIT_SUBDIR		= "Edits/";

static const ThemeMetric<RageColor>	EXTRA_COLOR			( "SongManager", "ExtraColor" );
static const ThemeMetric<int>		EXTRA_COLOR_METER		( "SongManager", "ExtraColorMeter" );
static const ThemeMetric<bool>		USE_PREFERRED_SORT_COLOR	( "SongManager", "UsePreferredSortColor" );
static const ThemeMetric<bool>		USE_UNLOCK_COLOR		( "SongManager", "UseUnlockColor" );
static const ThemeMetric<RageColor>	UNLOCK_COLOR			( "SongManager", "UnlockColor" );
static const ThemeMetric<bool>		MOVE_UNLOCKS_TO_BOTTOM_OF_PREFERRED_SORT	( "SongManager", "MoveUnlocksToBottomOfPreferredSort" );
static const ThemeMetric<int>		EXTRA_STAGE2_DIFFICULTY_MAX	( "SongManager", "ExtraStage2DifficultyMax" );

static Preference<RString> g_sDisabledSongs( "DisabledSongs", "" );
static Preference<bool> g_bHideIncompleteCourses( "HideIncompleteCourses", false );

RString SONG_GROUP_COLOR_NAME( std::size_t i )   { return ssprintf( "SongGroupColor%i", (int) i+1 ); }
RString COURSE_GROUP_COLOR_NAME( std::size_t i ) { return ssprintf( "CourseGroupColor%i", (int) i+1 ); }
RString profile_song_group_color_name(std::size_t i) { return ssprintf("ProfileSongGroupColor%i", (int)i+1); }

static const float next_loading_window_update= 0.02f;

SongManager::SongManager()
{
	// Register with Lua.
	{
		Lua *L = LUA->Get();
		lua_pushstring( L, "SONGMAN" );
		this->PushSelf( L );
		lua_settable( L, LUA_GLOBALSINDEX );
		LUA->Release( L );
	}

	NUM_SONG_GROUP_COLORS	.Load( "SongManager", "NumSongGroupColors" );
	SONG_GROUP_COLOR	.Load( "SongManager", SONG_GROUP_COLOR_NAME, NUM_SONG_GROUP_COLORS );
	NUM_COURSE_GROUP_COLORS	.Load( "SongManager", "NumCourseGroupColors" );
	COURSE_GROUP_COLOR	.Load( "SongManager", COURSE_GROUP_COLOR_NAME, NUM_COURSE_GROUP_COLORS );
	num_profile_song_group_colors.Load("SongManager", "NumProfileSongGroupColors");
	profile_song_group_colors.Load("SongManager", profile_song_group_color_name, num_profile_song_group_colors);
}

SongManager::~SongManager()
{
	// Unregister with Lua.
	LUA->UnsetGlobal( "SONGMAN" );

	// Courses depend on Songs and Songs don't depend on Courses.
	// So, delete the Courses first.
	FreeCourses();
	FreeSongs();
}

void SongManager::InitAll( LoadingWindow *ld, bool onlyAdditions )
{
	std::vector<RString> never_cache;
	split(PREFSMAN->m_NeverCacheList, ",", never_cache);
	for(std::vector<RString>::iterator group= never_cache.begin();
			group != never_cache.end(); ++group)
	{
		m_GroupsToNeverCache.insert(*group);
	}
	InitSongsFromDisk( ld, onlyAdditions );
	InitCoursesFromDisk( ld, onlyAdditions );
	if (onlyAdditions)
	{
		DeleteAutogenCourses();
	}
	InitAutogenCourses();
	InitRandomAttacks();
}

static LocalizedString RELOADING ( "SongManager", "Reloading..." );
static LocalizedString UNLOADING_SONGS ( "SongManager", "Unloading songs..." );
static LocalizedString UNLOADING_COURSES ( "SongManager", "Unloading courses..." );
static LocalizedString SANITY_CHECKING_GROUPS("SongManager", "Sanity checking groups...");

void SongManager::Reload( bool bAllowFastLoad, LoadingWindow *ld )
{
	FILEMAN->FlushDirCache( SpecialFiles::SONGS_DIR );
	FILEMAN->FlushDirCache( SpecialFiles::COURSES_DIR );
	FILEMAN->FlushDirCache( EDIT_SUBDIR );

	if( ld )
		ld->SetText( RELOADING );

	// save scores before unloading songs, or the scores will be lost
	PROFILEMAN->SaveMachineProfile();
	GAMESTATE->SavePlayerProfiles();

	std::vector<std::tuple<PlayerNumber, RString, bool>> rejoinPlayers;
	FOREACH_HumanPlayer(pn)
	{
		if (GAMESTATE->m_bSideIsJoined[pn])
		{
			if (PROFILEMAN->ProfileWasLoadedFromMemoryCard(pn))
			{
				rejoinPlayers.push_back(std::make_tuple(pn, RString(""), true));
			}
			else if (PROFILEMAN->IsPersistentProfile(pn))
			{
				int numLocalProfiles = PROFILEMAN->GetNumLocalProfiles();
				for (int i = 0; i < numLocalProfiles; i++)
				{
					Profile *profile = PROFILEMAN->GetLocalProfileFromIndex(i);
					if (profile->m_sGuid == PROFILEMAN->GetProfile(pn)->m_sGuid)
					{
						RString profileID = PROFILEMAN->GetLocalProfileIDFromIndex(i);
						rejoinPlayers.push_back(std::make_tuple(pn, profileID, false));
						break;
					}
				}
			}
			else
			{
				rejoinPlayers.push_back(std::make_tuple(pn, RString(""), false));
			}
			GAMESTATE->UnjoinPlayer(pn);
			PROFILEMAN->UnloadProfile(pn);
		}
	}

	if( ld )
		ld->SetText( UNLOADING_COURSES );

	FreeCourses();

	if( ld )
		ld->SetText( UNLOADING_SONGS );

	FreeSongs();

	const bool oldVal = PREFSMAN->m_bFastLoad;
	if( !bAllowFastLoad )
		PREFSMAN->m_bFastLoad.Set( false );

	InitAll( ld, /*onlyAdditions=*/false );

	// reload scores and unlocks afterward
	PROFILEMAN->LoadMachineProfile();
	MEMCARDMAN->WaitForCheckingToComplete();
	for (auto& it : rejoinPlayers)
	{
		PlayerNumber pn;
		RString profileID;
		bool isMemoryCard;
		std::tie(pn, profileID, isMemoryCard) = it;

		GAMESTATE->JoinPlayer(pn);
		if (isMemoryCard)
		{
			bool success = MEMCARDMAN->MountCard(pn);
			if (success)
			{
				PROFILEMAN->LoadProfileFromMemoryCard(pn, true);
				MEMCARDMAN->UnmountCard(pn);
			}
		}
		else
		{
			PROFILEMAN->m_sDefaultLocalProfileID[pn].Set(profileID);
			PROFILEMAN->LoadLocalProfileFromMachine(pn);
		}
		GAMESTATE->LoadCurrentSettingsFromProfile(pn);
	}
	UNLOCKMAN->Reload();

	if( !bAllowFastLoad )
		PREFSMAN->m_bFastLoad.Set( oldVal );

	UpdatePreferredSort();
}

void SongManager::LoadAdditions( LoadingWindow *ld )
{
	FILEMAN->FlushDirCache( SpecialFiles::SONGS_DIR );
	FILEMAN->FlushDirCache( SpecialFiles::COURSES_DIR );
	FILEMAN->FlushDirCache( EDIT_SUBDIR );

	InitAll( ld, /*onlyAdditions=*/true );

	UNLOCKMAN->Reload();

	UpdatePreferredSort();
}

void SongManager::InitSongsFromDisk( LoadingWindow *ld, bool onlyAdditions )
{
	RageTimer tm;
	// Tell SONGINDEX to not write the cache index file every time a song adds
	// an entry. -Kyz
	SONGINDEX->delay_save_cache = true;
	IMAGECACHE->delay_save_cache = true;
	LoadSongDir( SpecialFiles::SONGS_DIR, ld, onlyAdditions );
	LoadEnabledSongsFromPref();
	SONGINDEX->SaveCacheIndex();
	SONGINDEX->delay_save_cache = false;
	IMAGECACHE->WriteToDisk();
	IMAGECACHE->delay_save_cache = false;

	LOG->Trace( "Found %d songs in %f seconds.", (int)m_pSongs.size(), tm.GetDeltaTime() );
}

static LocalizedString FOLDER_CONTAINS_MUSIC_FILES( "SongManager", "The folder \"%s\" appears to be a song folder.  All song folders must reside in a group folder.  For example, \"Songs/Originals/My Song\"." );
void SongManager::SanityCheckGroupDir( RString sDir ) const
{
	// Check to see if they put a song directly inside the group folder.
	std::vector<RString> arrayFiles;
	GetDirListing( sDir + "/*", arrayFiles );
	const std::vector<RString>& audio_exts= ActorUtil::GetTypeExtensionList(FT_Sound);
	for (RString &fname : arrayFiles)
	{
		const RString ext= GetExtension(fname);
		for (RString const &aud : audio_exts)
		{
			if(ext == aud)
			{
				RageException::Throw(
					FOLDER_CONTAINS_MUSIC_FILES.GetValue(), sDir.c_str());
			}
		}
	}
}

void SongManager::AddGroup( RString sDir, RString sGroupDirName )
{
	unsigned j;
	for(j = 0; j < m_sSongGroupNames.size(); ++j)
		if( sGroupDirName == m_sSongGroupNames[j] )
			break;

	if( j != m_sSongGroupNames.size() )
		return; // the group is already added

	// Look for a group banner in this group folder
	std::vector<RString> arrayGroupBanners;
	GetDirListing( sDir+sGroupDirName+"/*.png", arrayGroupBanners );
	GetDirListing( sDir+sGroupDirName+"/*.jpg", arrayGroupBanners );
	GetDirListing( sDir+sGroupDirName+"/*.jpeg", arrayGroupBanners );
	GetDirListing( sDir+sGroupDirName+"/*.gif", arrayGroupBanners );
	GetDirListing( sDir+sGroupDirName+"/*.bmp", arrayGroupBanners );

	RString sBannerPath;
	if( !arrayGroupBanners.empty() )
		sBannerPath = sDir+sGroupDirName+"/"+arrayGroupBanners[0] ;
	else
	{
		// Look for a group banner in the parent folder
		GetDirListing( sDir+sGroupDirName+".png", arrayGroupBanners );
		GetDirListing( sDir+sGroupDirName+".jpg", arrayGroupBanners );
		GetDirListing( sDir+sGroupDirName+".jpeg", arrayGroupBanners );
		GetDirListing( sDir+sGroupDirName+".gif", arrayGroupBanners );
		GetDirListing( sDir+sGroupDirName+".bmp", arrayGroupBanners );
		if( !arrayGroupBanners.empty() )
			sBannerPath = sDir+arrayGroupBanners[0];
	}

	/* Other group graphics are a bit trickier, and usually don't exist.
	 * A themer has a few options, namely checking the aspect ratio and
	 * operating on it. -aj
	 * TODO: Once the files are implemented in Song, bring the extensions
	 * from there into here. -aj */
	// Group background

	//vector<RString> arrayGroupBackgrounds;
	//GetDirListing( sDir+sGroupDirName+"/*-bg.png", arrayGroupBanners );
	//GetDirListing( sDir+sGroupDirName+"/*-bg.jpg", arrayGroupBanners );
	//GetDirListing( sDir+sGroupDirName+"/*-bg.jpeg", arrayGroupBanners );
	//GetDirListing( sDir+sGroupDirName+"/*-bg.gif", arrayGroupBanners );
	//GetDirListing( sDir+sGroupDirName+"/*-bg.bmp", arrayGroupBanners );
/*
	RString sBackgroundPath;
	if( !arrayGroupBackgrounds.empty() )
		sBackgroundPath = sDir+sGroupDirName+"/"+arrayGroupBackgrounds[0];
	else
	{
		// Look for a group background in the parent folder
		GetDirListing( sDir+sGroupDirName+"-bg.png", arrayGroupBackgrounds );
		GetDirListing( sDir+sGroupDirName+"-bg.jpg", arrayGroupBackgrounds );
		GetDirListing( sDir+sGroupDirName+"-bg.jpeg", arrayGroupBackgrounds );
		GetDirListing( sDir+sGroupDirName+"-bg.gif", arrayGroupBackgrounds );
		GetDirListing( sDir+sGroupDirName+"-bg.bmp", arrayGroupBackgrounds );
		if( !arrayGroupBackgrounds.empty() )
			sBackgroundPath = sDir+arrayGroupBackgrounds[0];
	}
*/
	/*
	LOG->Trace( "Group banner for '%s' is '%s'.", sGroupDirName.c_str(),
				sBannerPath != ""? sBannerPath.c_str():"(none)" );
	*/
	m_sSongGroupNames.push_back( sGroupDirName );
	m_sSongGroupBannerPaths.push_back( sBannerPath );
	//m_sSongGroupBackgroundPaths.push_back( sBackgroundPath );
}

static LocalizedString LOADING_SONGS ( "SongManager", "Loading songs..." );
void SongManager::LoadSongDir( RString sDir, LoadingWindow *ld, bool onlyAdditions )
{
	if( ld )
		ld->SetText( LOADING_SONGS );

	// Compositors and other stuff can impose some overhead on updating the
	// loading window, which slows down startup time for some people.
	// loading_window_last_update_time provides a timer so the loading window
	// isn't updated after every song and course. -Kyz
	RageTimer loading_window_last_update_time;
	loading_window_last_update_time.Touch();
	// Make sure sDir has a trailing slash.
	if( sDir.Right(1) != "/" )
		sDir += "/";

	// Find all group directories in "Songs" folder
	std::vector<RString> arrayGroupDirs;
	GetDirListing( sDir+"*", arrayGroupDirs, true );
	SortRStringArray( arrayGroupDirs );
	StripCvsAndSvn( arrayGroupDirs );
	StripMacResourceForks( arrayGroupDirs );

	std::vector<std::vector<RString>> arrayGroupSongDirs;
	int groupIndex, songCount, songIndex;

	groupIndex = 0;
	songCount = 0;
	if(ld)
	{
		ld->SetIndeterminate(false);
		ld->SetTotalWork(arrayGroupDirs.size());
	}
	int sanity_index= 0;
	for (RString const &sGroupDirName : arrayGroupDirs)	// foreach dir in /Songs/
	{
		if(ld && loading_window_last_update_time.Ago() > next_loading_window_update)
		{
			loading_window_last_update_time.Touch();
			ld->SetProgress(sanity_index);
			ld->SetText(SANITY_CHECKING_GROUPS.GetValue() + ssprintf("\n%s",
					Basename(sGroupDirName).c_str()));
		}
		// TODO: If this check fails, log a warning instead of crashing.
		SanityCheckGroupDir(sDir+sGroupDirName);

		// Find all Song folders in this group directory
		std::vector<RString> arraySongDirs;
		GetDirListing( sDir+sGroupDirName + "/*", arraySongDirs, true, true );
		StripCvsAndSvn( arraySongDirs );
		StripMacResourceForks( arraySongDirs );
		SortRStringArray( arraySongDirs );

		arrayGroupSongDirs.push_back(arraySongDirs);
		songCount += arraySongDirs.size();

	}

	if( songCount==0 ) return;

	if( ld ) {
		ld->SetIndeterminate( false );
		ld->SetTotalWork( songCount );
	}

	groupIndex = 0;
	songIndex = 0;
	for (RString const &sGroupDirName : arrayGroupDirs)	// foreach dir in /Songs/
	{
		std::vector<RString> &arraySongDirs = arrayGroupSongDirs[groupIndex++];

		LOG->Trace("Attempting to load %i songs from \"%s\"", int(arraySongDirs.size()),
				   (sDir+sGroupDirName).c_str() );
		int loaded = 0;

		SongPointerVector& index_entry = m_mapSongGroupIndex[sGroupDirName];
		RString group_base_name= Basename(sGroupDirName);
		for( unsigned j=0; j< arraySongDirs.size(); ++j )	// for each song dir
		{
			RString sSongDirName = arraySongDirs[j];

			// Skip already loaded songs if onlyAdditions is set.
			if (onlyAdditions)
			{
				SongID songID;
				songID.FromString(sSongDirName);
				if (songID.ToSong() != nullptr)
					continue;
			}

			// this is a song directory. Load a new song.
			if(ld && loading_window_last_update_time.Ago() > next_loading_window_update)
			{
				loading_window_last_update_time.Touch();
				ld->SetProgress(songIndex);
				ld->SetText( LOADING_SONGS.GetValue() +
					ssprintf("\n%s\n%s",
						group_base_name.c_str(),
						Basename(sSongDirName).c_str()
					)
				);
			}

			Song* pNewSong = new Song;
			if( !pNewSong->LoadFromSongDir( sSongDirName ) )
			{
				// The song failed to load.
				delete pNewSong;
				continue;
			}
			AddSongToList(pNewSong);

			index_entry.push_back( pNewSong );
			loaded++;
			songIndex++;
		}

		LOG->Trace("Loaded %i songs from \"%s\"", loaded, (sDir+sGroupDirName).c_str() );

		// Don't add the group name if we didn't load any songs in this group.
		if(!loaded) continue;

		// Add this group to the group array.
		AddGroup(sDir, sGroupDirName);

		// Cache and load the group banner. (and background if it has one -aj)
		IMAGECACHE->CacheImage( "Banner", GetSongGroupBannerPath(sGroupDirName) );

		// Load the group sym links (if any)
		LoadGroupSymLinks(sDir, sGroupDirName);
	}

	if( ld ) {
		ld->SetIndeterminate( true );
	}
}

// Instead of "symlinks", songs should have membership in multiple groups. -Chris
void SongManager::LoadGroupSymLinks(RString sDir, RString sGroupFolder)
{
	// Find all symlink files in this folder
	std::vector<RString> arraySymLinks;
	GetDirListing( sDir+sGroupFolder+"/*.include", arraySymLinks, false );
	SortRStringArray( arraySymLinks );
	SongPointerVector& index_entry = m_mapSongGroupIndex[sGroupFolder];
	for( unsigned s=0; s< arraySymLinks.size(); s++ )	// for each symlink in this dir, add it in as a song.
	{
		MsdFile msdF;
		msdF.ReadFile( sDir+sGroupFolder+"/"+arraySymLinks[s].c_str(), false );  // don't unescape
		RString	sSymDestination = msdF.GetParam(0,1); // Should only be 1 value & param...period.

		Song* pNewSong = new Song;
		if( !pNewSong->LoadFromSongDir( sSymDestination ) )
		{
			delete pNewSong; // The song failed to load.
		}
		else
		{
			const std::vector<Steps*>& vpSteps = pNewSong->GetAllSteps();
			while( vpSteps.size() )
				pNewSong->DeleteSteps( vpSteps[0] );

			FOREACH_BackgroundLayer( i )
				pNewSong->GetBackgroundChanges(i).clear();

			pNewSong->m_bIsSymLink = true;	// Very important so we don't double-parse later
			pNewSong->m_sGroupName = sGroupFolder;
			AddSongToList(pNewSong);
			index_entry.push_back( pNewSong );
		}
	}
}

void SongManager::PreloadSongImages()
{
	if( PREFSMAN->m_ImageCache != IMGCACHE_FULL )
		return;

	/* Load textures before unloading old ones, so we don't reload textures
	 * that we don't need to. */
	RageTexturePreloader preload;

	const std::vector<Song*> &songs = GetAllSongs();
	for( unsigned i = 0; i < songs.size(); ++i )
	{
		if( !songs[i]->HasBanner() )
			continue;

		const RageTextureID ID = Sprite::SongBannerTexture( songs[i]->GetBannerPath() );
		preload.Load( ID );
	}

	std::vector<Course*> courses;
	GetAllCourses( courses, false );
	for( unsigned i = 0; i < courses.size(); ++i )
	{
		if( !courses[i]->HasBanner() )
			continue;

		const RageTextureID ID = Sprite::SongBannerTexture( courses[i]->GetBannerPath() );
		preload.Load( ID );
	}

	preload.Swap( m_TexturePreload );
}

void SongManager::FreeSongs()
{
	m_sSongGroupNames.clear();
	m_sSongGroupBannerPaths.clear();
	//m_sSongGroupBackgroundPaths.clear();

	for (Song *song : m_pSongs)
	{
		SAFE_DELETE( song );
	}
	m_pSongs.clear();
	m_SongsByDir.clear();

	// also free the songs that have been deleted from disk
	for ( unsigned i=0; i<m_pDeletedSongs.size(); ++i )
		SAFE_DELETE( m_pDeletedSongs[i] );
	m_pDeletedSongs.clear();

	m_mapSongGroupIndex.clear();
	m_sSongGroupBannerPaths.clear();

	m_pPopularSongs.clear();
	m_pShuffledSongs.clear();
}

void SongManager::UnlistSong(Song *song)
{
	// cannot immediately free song data, as it is needed temporarily for smooth audio transitions, etc.
	// Instead, remove it from the m_pSongs list and store it in a special place where it can safely be deleted later.
	m_pDeletedSongs.push_back(song);

	// remove all occurences of the song in each of our song vectors
	std::vector<Song*>* songVectors[3] = { &m_pSongs, &m_pPopularSongs, &m_pShuffledSongs };
	for (int songVecIdx=0; songVecIdx<3; ++songVecIdx) {
		std::vector<Song*>& v = *songVectors[songVecIdx];
		for (std::size_t i=0; i<v.size(); ++i) {
			if (v[i] == song) {
				v.erase(v.begin()+i);
				--i;
			}
		}
	}
}

bool SongManager::IsGroupNeverCached(const RString& group) const
{
	return m_GroupsToNeverCache.find(group) != m_GroupsToNeverCache.end();
}

RString SongManager::GetSongGroupBannerPath( RString sSongGroup ) const
{
	for( unsigned i = 0; i < m_sSongGroupNames.size(); ++i )
	{
		if( sSongGroup == m_sSongGroupNames[i] )
			return m_sSongGroupBannerPaths[i];
	}

	return RString();
}
/*
RString SongManager::GetSongGroupBackgroundPath( RString sSongGroup ) const
{
	for( unsigned i = 0; i < m_sSongGroupNames.size(); ++i )
	{
		if( sSongGroup == m_sSongGroupNames[i] )
			return m_sSongGroupBackgroundPaths[i];
	}

	return RString();
}
*/
void SongManager::GetSongGroupNames( std::vector<RString> &AddTo ) const
{
	AddTo.insert(AddTo.end(), m_sSongGroupNames.begin(), m_sSongGroupNames.end() );
}

bool SongManager::DoesSongGroupExist( RString sSongGroup ) const
{
	return find( m_sSongGroupNames.begin(), m_sSongGroupNames.end(), sSongGroup ) != m_sSongGroupNames.end();
}

RageColor SongManager::GetSongGroupColor( const RString &sSongGroup ) const
{
	for( unsigned i=0; i<m_sSongGroupNames.size(); i++ )
	{
		if( m_sSongGroupNames[i] == sSongGroup )
		{
			return SONG_GROUP_COLOR.GetValue( i%NUM_SONG_GROUP_COLORS );
		}
	}
	FOREACH_EnabledPlayer(pn)
	{
		Profile* prof= PROFILEMAN->GetProfile(pn);
		if(prof != nullptr)
		{
			if(prof->GetDisplayNameOrHighScoreName() == sSongGroup)
			{
				return profile_song_group_colors.GetValue(pn % num_profile_song_group_colors);
			}
		}
	}

	LuaHelpers::ReportScriptErrorFmt("requested color for song group '%s' that doesn't exist",sSongGroup.c_str());
	return RageColor(1,1,1,1);
}

RageColor SongManager::GetSongColor( const Song* pSong ) const
{
	ASSERT( pSong != nullptr );

	// protected by royal freem corporation. any modification/removal of
	// this code will result in prosecution.
	if( pSong->m_sMainTitle == "DVNO")
		return RageColor(1.0f,0.8f,0.0f,1.0f);
	// end royal freem protection

	// Use unlock color if applicable
	const UnlockEntry *pUE = UNLOCKMAN->FindSong( pSong );
	if( pUE && USE_UNLOCK_COLOR.GetValue() )
		return UNLOCK_COLOR.GetValue();

	if( USE_PREFERRED_SORT_COLOR )
	{
		int sortIndex = 0;
		for (PreferredSortSection const &v : m_vPreferredSongSort)
		{
			if (std::any_of(v.vpSongs.begin(), v.vpSongs.end(), [&](Song const *s) { return s == pSong; }))
			{
				return SONG_GROUP_COLOR.GetValue( sortIndex % NUM_SONG_GROUP_COLORS );
			}

			sortIndex += 1;
		}

		int i = m_vPreferredSongSort.size();
		return SONG_GROUP_COLOR.GetValue( i%NUM_SONG_GROUP_COLORS );
	}
	else // TODO: Have a better fallback plan with colors?
	{
		/* XXX: Previously, this matched all notes, which set a song to "extra"
		 * if it had any 10-foot steps at all, even edits or doubles.
		 *
		 * For now, only look at notes for the current note type. This means
		 * that if a song has 10-foot steps on Doubles, it'll only show up red
		 * in Doubles. That's not too bad, I think. This will also change it
		 * in the song scroll, which is a little odd but harmless.
		 *
		 * XXX: Ack. This means this function can only be called when we have
		 * a style set up, which is too restrictive. How to handle this? */
		//const StepsType st = GAMESTATE->GetCurrentStyle()->m_StepsType;
		const std::vector<Steps*>& vpSteps = pSong->GetAllSteps();
		for( unsigned i=0; i<vpSteps.size(); i++ )
		{
			const Steps* pSteps = vpSteps[i];
			switch( pSteps->GetDifficulty() )
			{
				case Difficulty_Challenge:
				case Difficulty_Edit:
					continue;
				default: break;
			}

			//if(pSteps->m_StepsType != st)
			//	continue;

			if( pSteps->GetMeter() >= EXTRA_COLOR_METER )
				return (RageColor)EXTRA_COLOR;
		}

		return GetSongGroupColor( pSong->m_sGroupName );
	}
}

RString SongManager::GetCourseGroupBannerPath( const RString &sCourseGroup ) const
{
	std::map<RString, CourseGroupInfo>::const_iterator iter = m_mapCourseGroupToInfo.find( sCourseGroup );
	if( iter == m_mapCourseGroupToInfo.end() )
	{
		ASSERT_M( 0, ssprintf("requested banner for course group '%s' that doesn't exist",sCourseGroup.c_str()) );
		return RString();
	}
	else
	{
		return iter->second.m_sBannerPath;
	}
}

void SongManager::GetCourseGroupNames( std::vector<RString> &AddTo ) const
{
	for (std::pair<RString const, CourseGroupInfo> const &iter : m_mapCourseGroupToInfo)
		AddTo.push_back( iter.first );
}

bool SongManager::DoesCourseGroupExist( const RString &sCourseGroup ) const
{
	return m_mapCourseGroupToInfo.find( sCourseGroup ) != m_mapCourseGroupToInfo.end();
}

RageColor SongManager::GetCourseGroupColor( const RString &sCourseGroup ) const
{
	int iIndex = 0;
	for (std::pair<RString const, CourseGroupInfo> const &iter : m_mapCourseGroupToInfo)
	{
		if( iter.first == sCourseGroup )
			return SONG_GROUP_COLOR.GetValue( iIndex%NUM_SONG_GROUP_COLORS );
		iIndex++;
	}

	ASSERT_M( 0, ssprintf("requested color for course group '%s' that doesn't exist",sCourseGroup.c_str()) );
	return RageColor(1,1,1,1);
}

RageColor SongManager::GetCourseColor( const Course* pCourse ) const
{
	// Use unlock color if applicable
	const UnlockEntry *pUE = UNLOCKMAN->FindCourse( pCourse );
	if( pUE  &&  USE_UNLOCK_COLOR.GetValue() )
		return UNLOCK_COLOR.GetValue();

	if( USE_PREFERRED_SORT_COLOR )
	{
		int courseIndex = 0;
		for (CoursePointerVector const &v : m_vPreferredCourseSort)
		{
			if (std::any_of(v.begin(), v.end(), [&](Course const *s) { return s == pCourse; }))
			{
				CHECKPOINT_M( ssprintf( "%i, NUM_COURSE_GROUP_COLORS = %i", courseIndex, NUM_COURSE_GROUP_COLORS.GetValue()) );
				return COURSE_GROUP_COLOR.GetValue( courseIndex % NUM_COURSE_GROUP_COLORS );
			}
			courseIndex += 1;
		}

		int i = m_vPreferredCourseSort.size();
		CHECKPOINT_M( ssprintf( "%i, NUM_COURSE_GROUP_COLORS = %i", i, NUM_COURSE_GROUP_COLORS.GetValue()) );
		return COURSE_GROUP_COLOR.GetValue( i % NUM_COURSE_GROUP_COLORS );
	}
	else
	{
		return GetCourseGroupColor( pCourse->m_sGroupName );
	}
}

void SongManager::ResetGroupColors()
{
	// Reload song/course group colors to prevent a crash when switching
	// themes in-game. (apparently not, though.) -aj
	SONG_GROUP_COLOR.Clear();
	COURSE_GROUP_COLOR.Clear();

	NUM_SONG_GROUP_COLORS	.Load( "SongManager", "NumSongGroupColors" );
	SONG_GROUP_COLOR	.Load( "SongManager", SONG_GROUP_COLOR_NAME, NUM_SONG_GROUP_COLORS );
	NUM_COURSE_GROUP_COLORS .Load( "SongManager", "NumCourseGroupColors" );
	COURSE_GROUP_COLOR	.Load( "SongManager", COURSE_GROUP_COLOR_NAME, NUM_COURSE_GROUP_COLORS );
}

const std::vector<Song*> &SongManager::GetSongs( const RString &sGroupName ) const
{
	static const std::vector<Song*> vEmpty;

	if( sGroupName == GROUP_ALL )
		return m_pSongs;
	std::map<RString, SongPointerVector, Comp>::const_iterator iter = m_mapSongGroupIndex.find( sGroupName );
	if ( iter != m_mapSongGroupIndex.end() )
		return iter->second;
	FOREACH_EnabledPlayer(pn)
	{
		Profile* prof= PROFILEMAN->GetProfile(pn);
		if(prof != nullptr)
		{
			if(prof->GetDisplayNameOrHighScoreName() == sGroupName)
			{
				return prof->m_songs;
			}
		}
	}
	return vEmpty;
}

void SongManager::GetPreferredSortSongs( std::vector<Song*> &AddTo ) const
{
	if( m_vPreferredSongSort.empty() )
	{
		AddTo.insert( AddTo.end(), m_pSongs.begin(), m_pSongs.end() );
		return;
	}
	for (PreferredSortSection const &v : m_vPreferredSongSort)
		AddTo.insert( AddTo.end(), v.vpSongs.begin(), v.vpSongs.end() );
}

RString SongManager::SongToPreferredSortSectionName( const Song *pSong ) const
{
	for (PreferredSortSection const &v : m_vPreferredSongSort)
	{
		if (std::any_of(v.vpSongs.begin(), v.vpSongs.end(), [&](Song const *s) { return s == pSong; }))
		{
			return v.sName;
		}
	}
	return RString();
}


void SongManager::GetPreferredSortSongsBySectionName( const RString &sSectionName, std::vector<Song*> &AddTo ) const
{
	// Use m_mapPreferredSectionToSongs
	std::map<RString, SongPointerVector>::const_iterator iter = m_mapPreferredSectionToSongs.find( sSectionName );
	if( iter != m_mapPreferredSectionToSongs.end() )
		AddTo.insert( AddTo.end(), iter->second.begin(), iter->second.end() );
}

std::vector<Song*> SongManager::GetPreferredSortSongsBySectionName( const RString &sSectionName ) const
{
	std::vector<Song*> AddTo;
	GetPreferredSortSongsBySectionName(sSectionName, AddTo);
	return AddTo;
}

std::vector<RString> SongManager::GetPreferredSortSectionNames() const
{
	std::vector<RString> sectionNames;
	// Use m_mapPreferredSectionToSongs
	for (std::pair<RString const, SongPointerVector> const &iter : m_mapPreferredSectionToSongs)
		sectionNames.push_back(iter.first);
	return sectionNames;

}
	
void SongManager::GetPreferredSortCourses( CourseType ct, std::vector<Course*> &AddTo, bool bIncludeAutogen ) const
{
	if( m_vPreferredCourseSort.empty() )
	{
		GetCourses( ct, AddTo, bIncludeAutogen );
		return;
	}

	for (CoursePointerVector const &v : m_vPreferredCourseSort)
	{
		for (Course *pCourse : v)
		{
			if( pCourse->GetCourseType() == ct )
				AddTo.push_back( pCourse );
		}
	}
}

std::vector<Song*> SongManager::GetSongsByMeter(const int iMeter) const {
    std::vector<Song*> AddTo;
    auto iter = m_mapSongsByDifficulty.find(iMeter);
    if (iter != m_mapSongsByDifficulty.end()) {
        // Found the level, add the songs to AddTo
        AddTo.insert(AddTo.end(), iter->second.begin(), iter->second.end());
    }
    return AddTo;
}

int SongManager::GetNumSongs() const
{
	return m_pSongs.size();
}

int SongManager::GetNumLockedSongs() const
{
	return std::count_if(m_pSongs.begin(), m_pSongs.end(), [](Song const *s) { return UNLOCKMAN->SongIsLocked(s); });
}

int SongManager::GetNumUnlockedSongs() const
{
	return std::count_if(m_pSongs.begin(), m_pSongs.end(), [](Song const *s) { return UNLOCKMAN->SongIsLocked(s) & ~LOCKED_LOCK; });
}

int SongManager::GetNumSelectableAndUnlockedSongs() const
{
	return std::count_if(m_pSongs.begin(), m_pSongs.end(), [](Song const *s) { return UNLOCKMAN->SongIsLocked(s) & ~(LOCKED_LOCK | LOCKED_SELECTABLE); });
}

int SongManager::GetNumSongGroups() const
{
	return m_sSongGroupNames.size();
}

int SongManager::GetNumCourses() const
{
	return m_pCourses.size();
}

int SongManager::GetNumCourseGroups() const
{
	return m_mapCourseGroupToInfo.size();
}

RString SongManager::ShortenGroupName( RString sLongGroupName )
{
	static TitleSubst tsub("Groups");

	TitleFields title;
	title.Title = sLongGroupName;
	tsub.Subst( title );
	return title.Title;
}

static LocalizedString LOADING_COURSES ( "SongManager", "Loading courses..." );
void SongManager::InitCoursesFromDisk( LoadingWindow *ld, bool onlyAdditions )
{
	LOG->Trace( "Loading courses." );
	if( ld )
		ld->SetText( LOADING_COURSES );

	RageTimer loading_window_last_update_time;
	loading_window_last_update_time.Touch();

	std::vector<RString> vsCourseGroupNames;
	// Find all group directories in Courses dir
	GetDirListing( SpecialFiles::COURSES_DIR + "*", vsCourseGroupNames, true, true );
	StripCvsAndSvn( vsCourseGroupNames );
	StripMacResourceForks( vsCourseGroupNames );

	// Search for courses both in COURSES_DIR and in subdirectories
	vsCourseGroupNames.push_back( SpecialFiles::COURSES_DIR );
	SortRStringArray( vsCourseGroupNames );

	int courseIndex = 0;
	for (RString const &sCourseGroup : vsCourseGroupNames) // for each dir in /Courses/
	{
		// Find all CRS files in this group directory
		std::vector<RString> vsCoursePaths;
		GetDirListing( sCourseGroup + "/*.crs", vsCoursePaths, false, true );
		SortRStringArray( vsCoursePaths );

		if( ld )
		{
			ld->SetIndeterminate( false );
			ld->SetTotalWork( vsCoursePaths.size() );
		}

		RString base_course_group= Basename(sCourseGroup);
		for (RString const &sCoursePath : vsCoursePaths)
		{
			// Skip already loaded courses if onlyAdditions is set.
			if (onlyAdditions)
			{
				CourseID courseID;
				courseID.FromPath(sCoursePath);
				if (courseID.ToCourse() != nullptr)
					continue;
			}

			if(ld && loading_window_last_update_time.Ago() > next_loading_window_update)
			{
				loading_window_last_update_time.Touch();
				ld->SetProgress(courseIndex);
				ld->SetText( LOADING_COURSES.GetValue()+ssprintf("\n%s\n%s",
					base_course_group.c_str(),
					Basename(sCoursePath).c_str()));
			}

			Course* pCourse = new Course;
			CourseLoaderCRS::LoadFromCRSFile( sCoursePath, *pCourse );

			if( g_bHideIncompleteCourses.Get() && pCourse->m_bIncomplete )
			{
				delete pCourse;
				continue;
			}

			m_pCourses.push_back( pCourse );
			courseIndex++;
		}
	}

	if( ld ) {
		ld->SetIndeterminate( true );
	}

	RefreshCourseGroupInfo();
}

void SongManager::InitAutogenCourses()
{
	// Create group courses for Endless and Nonstop
	std::vector<RString> saGroupNames;
	this->GetSongGroupNames( saGroupNames );
	Course* pCourse;
	for( unsigned g=0; g<saGroupNames.size(); g++ )	// foreach Group
	{
		RString sGroupName = saGroupNames[g];

		// Generate random courses from each group.
		pCourse = new Course;
		CourseUtil::AutogenEndlessFromGroup( sGroupName, Difficulty_Medium, *pCourse );
		pCourse->m_sScripter = "Autogen";
		m_pCourses.push_back( pCourse );

		pCourse = new Course;
		CourseUtil::AutogenNonstopFromGroup( sGroupName, Difficulty_Medium, *pCourse );
		pCourse->m_sScripter = "Autogen";
		m_pCourses.push_back( pCourse );
	}

	std::vector<Song*> apCourseSongs = GetAllSongs();

	// Generate "All Songs" endless course.
	pCourse = new Course;
	CourseUtil::AutogenEndlessFromGroup( "", Difficulty_Medium, *pCourse );
	pCourse->m_sScripter = "Autogen";
	m_pCourses.push_back( pCourse );

	/* Generate Oni courses from artists. Only create courses if we have at least
	 * four songs from an artist; create 3- and 4-song courses. */
	{
		/* We normally sort by translit artist. However, display artist is more
		 * consistent. For example, transliterated Japanese names are alternately
		 * spelled given- and family-name first, but display titles are more consistent. */
		std::vector<Song*> apSongs = this->GetAllSongs();
		SongUtil::SortSongPointerArrayByDisplayArtist( apSongs );

		RString sCurArtist = "";
		RString sCurArtistTranslit = "";
		int iCurArtistCount = 0;

		std::vector<Song *> aSongs;
		unsigned i = 0;
		do {
			RString sArtist = i >= apSongs.size()? RString(""): apSongs[i]->GetDisplayArtist();
			RString sTranslitArtist = i >= apSongs.size()? RString(""): apSongs[i]->GetTranslitArtist();
			if( i < apSongs.size() && !sCurArtist.CompareNoCase(sArtist) )
			{
				aSongs.push_back( apSongs[i] );
				++iCurArtistCount;
				continue;
			}

			/* Different artist, or we're at the end. If we have enough entries for
			 * the last artist, add it. Skip blanks and "Unknown artist". */
			if( iCurArtistCount >= 3 && sCurArtistTranslit != "" &&
				sCurArtistTranslit.CompareNoCase("Unknown artist") &&
				sCurArtist.CompareNoCase("Unknown artist") )
			{
				pCourse = new Course;
				CourseUtil::AutogenOniFromArtist( sCurArtist, sCurArtistTranslit, aSongs, Difficulty_Hard, *pCourse );
				pCourse->m_sScripter = "Autogen";
				m_pCourses.push_back( pCourse );
			}

			aSongs.clear();

			if( i < apSongs.size() )
			{
				sCurArtist = sArtist;
				sCurArtistTranslit = sTranslitArtist;
				iCurArtistCount = 1;
				aSongs.push_back( apSongs[i] );
			}
		} while( i++ < apSongs.size() );
	}
}

void SongManager::InitRandomAttacks()
{
	GAMESTATE->m_RandomAttacks.clear();

	if( !IsAFile(ATTACK_FILE) )
		LOG->Trace( "File Data/RandomAttacks.txt was not found" );
	else
	{
		MsdFile msd;

		if( !msd.ReadFile( ATTACK_FILE, true ) )
			LuaHelpers::ReportScriptErrorFmt( "Error opening file '%s' for reading: %s.", ATTACK_FILE.c_str(), msd.GetError().c_str() );
		else
		{
			for( unsigned i=0; i<msd.GetNumValues(); i++ )
			{
				int iNumParams = msd.GetNumParams(i);
				const MsdFile::value_t &sParams = msd.GetValue(i);
				RString sType = sParams[0];
				RString sAttack = sParams[1];

				if( iNumParams > 2 )
				{
					LuaHelpers::ReportScriptErrorFmt( "Got \"%s:%s\" tag with too many parameters", sType.c_str(), sAttack.c_str() );
					continue;
				}

				if( !sType.EqualsNoCase("ATTACK") )
				{
					LuaHelpers::ReportScriptErrorFmt( "Got \"%s:%s\" tag with wrong declaration", sType.c_str(), sAttack.c_str() );
					continue;
				}

				GAMESTATE->m_RandomAttacks.push_back( sAttack );
			}
		}
	}
}

void SongManager::FreeCourses()
{
	for( unsigned i=0; i<m_pCourses.size(); i++ )
		delete m_pCourses[i];
	m_pCourses.clear();

	FOREACH_CourseType( ct )
		m_pPopularCourses[ct].clear();
	m_pShuffledCourses.clear();

	m_mapCourseGroupToInfo.clear();
}

void SongManager::DeleteAutogenCourses()
{
	std::vector<Course*> vNewCourses;
	for( std::vector<Course*>::iterator it = m_pCourses.begin(); it != m_pCourses.end(); ++it )
	{
		if( (*it)->m_bIsAutogen )
		{
			delete *it;
		}
		else
		{
			vNewCourses.push_back( *it );
		}
	}
	m_pCourses.swap( vNewCourses );
	UpdatePopular();
	UpdateShuffled();
	RefreshCourseGroupInfo();
}

void SongManager::AddCourse( Course *pCourse )
{
	m_pCourses.push_back( pCourse );
	UpdatePopular();
	UpdateShuffled();
	m_mapCourseGroupToInfo[ pCourse->m_sGroupName ];	// insert
}

void SongManager::DeleteCourse( Course *pCourse )
{
	std::vector<Course*>::iterator iter = find( m_pCourses.begin(), m_pCourses.end(), pCourse );
	ASSERT( iter != m_pCourses.end() );
	m_pCourses.erase( iter );
	UpdatePopular();
	UpdateShuffled();
	RefreshCourseGroupInfo();
}

void SongManager::InvalidateCachedTrails()
{
	for (Course *pCourse : m_pCourses)
	{
		if( pCourse->IsAnEdit() )
			pCourse->m_TrailCache.clear();
	}
}

/* Called periodically to wipe out cached NoteData. This is called when we
 * change screens. */
void SongManager::Cleanup()
{
	for (Song *pSong : m_pShuffledSongs)
	{
		if (pSong)
		{
			const std::vector<Steps*>& vpSteps = pSong->GetAllSteps();
			for (Steps *pSteps : vpSteps)
			{
				pSteps->Compress();
			}
		}
	}
}

/* Flush all Song*, Steps* and Course* caches. This is when a Song or its Steps
 * are removed or changed. This doesn't touch GAMESTATE and StageStats
 * pointers. Currently, the only time Steps are altered independently of the
 * Courses and Songs is in Edit Mode, which updates the other pointers it needs. */
void SongManager::Invalidate( const Song *pStaleSong )
{
	// TODO: This is unnecessarily expensive.
	// Can we regenerate only the autogen courses that are affected?
	DeleteAutogenCourses();

	for (Course *c : this->m_pCourses)
	{
		c->Invalidate( pStaleSong );
	}

	InitAutogenCourses();

	UpdatePopular();
	UpdateShuffled();
	RefreshCourseGroupInfo();
}

void SongManager::RegenerateNonFixedCourses()
{
	for( unsigned i=0; i < m_pCourses.size(); i++ )
		m_pCourses[i]->RegenerateNonFixedTrails();
}

void SongManager::SetPreferences()
{
	for( unsigned int i=0; i<m_pSongs.size(); i++ )
	{
		// PREFSMAN->m_bAutogenSteps may have changed.
		m_pSongs[i]->RemoveAutoGenNotes();
		m_pSongs[i]->AddAutoGenNotes();
	}
}

void SongManager::SaveEnabledSongsToPref()
{
	std::vector<RString> vsDisabledSongs;

	// Intentionally drop disabled song entries for songs that aren't currently loaded.

	const std::vector<Song*> &apSongs = SONGMAN->GetAllSongs();
	for (Song *pSong : apSongs)
	{
		SongID sid;
		sid.FromSong( pSong );
		if( !pSong->GetEnabled() )
			vsDisabledSongs.push_back( sid.ToString() );
	}
	g_sDisabledSongs.Set( join(";", vsDisabledSongs) );
}

void SongManager::LoadEnabledSongsFromPref()
{
	std::vector<RString> asDisabledSongs;
	split( g_sDisabledSongs, ";", asDisabledSongs, true );

	for (RString const &s : asDisabledSongs)
	{
		SongID sid;
		sid.FromString( s );
		Song *pSong = sid.ToSong();
		if( pSong )
			pSong->SetEnabled( false );
	}
}

void SongManager::GetStepsLoadedFromProfile( std::vector<Steps*> &AddTo, ProfileSlot slot ) const
{
	const std::vector<Song*> &vSongs = GetAllSongs();
	for (Song *song : vSongs)
	{
		song->GetStepsLoadedFromProfile( slot, AddTo );
	}
}

void SongManager::DeleteSteps( Steps *pSteps )
{
	pSteps->m_pSong->DeleteSteps( pSteps );
}

void SongManager::GetAllCourses( std::vector<Course*> &AddTo, bool bIncludeAutogen ) const
{
	for( unsigned i=0; i<m_pCourses.size(); i++ )
		if( bIncludeAutogen || !m_pCourses[i]->m_bIsAutogen )
			AddTo.push_back( m_pCourses[i] );
}

void SongManager::GetCourses( CourseType ct, std::vector<Course*> &AddTo, bool bIncludeAutogen ) const
{
	for( unsigned i=0; i<m_pCourses.size(); i++ )
		if( m_pCourses[i]->GetCourseType() == ct )
			if( bIncludeAutogen || !m_pCourses[i]->m_bIsAutogen )
				AddTo.push_back( m_pCourses[i] );
}

void SongManager::GetCoursesInGroup( std::vector<Course*> &AddTo, const RString &sCourseGroup, bool bIncludeAutogen ) const
{
	for( unsigned i=0; i<m_pCourses.size(); i++ )
		if( m_pCourses[i]->m_sGroupName == sCourseGroup )
			if( bIncludeAutogen || !m_pCourses[i]->m_bIsAutogen )
				AddTo.push_back( m_pCourses[i] );
}

bool SongManager::GetExtraStageInfoFromCourse( bool bExtra2, RString sPreferredGroup, Song*& pSongOut, Steps*& pStepsOut, StepsType stype )
{
	const RString sCourseSuffix = sPreferredGroup + (bExtra2 ? "/extra2.crs" : "/extra1.crs");
	RString sCoursePath = SpecialFiles::SONGS_DIR + sCourseSuffix;

	Course course;
	CourseLoaderCRS::LoadFromCRSFile( sCoursePath, course );
	if( course.GetEstimatedNumStages() <= 0 ) return false;

	Trail *pTrail = course.GetTrail(stype);
	if( pTrail->m_vEntries.empty() )
		return false;

	pSongOut = pTrail->m_vEntries[0].pSong;
	pStepsOut = pTrail->m_vEntries[0].pSteps;
	return true;
}

// Return true if n1 < n2.
bool CompareNotesPointersForExtra(const Steps *n1, const Steps *n2)
{
	// Equate CHALLENGE to HARD.
	Difficulty d1 = std::min(n1->GetDifficulty(), Difficulty_Hard);
	Difficulty d2 = std::min(n2->GetDifficulty(), Difficulty_Hard);

	if(d1 < d2) return true;
	if(d1 > d2) return false;
	// n1 difficulty == n2 difficulty

	if(StepsUtil::CompareNotesPointersByMeter(n1,n2)) return true;
	if(StepsUtil::CompareNotesPointersByMeter(n2,n1)) return false;
	// n1 meter == n2 meter

	return StepsUtil::CompareNotesPointersByRadarValues(n1,n2);
}

void SongManager::GetExtraStageInfo( bool bExtra2, const Style *sd, Song*& pSongOut, Steps*& pStepsOut )
{
	RString sGroup = GAMESTATE->m_sPreferredSongGroup;
	if( sGroup == GROUP_ALL )
	{
		if( GAMESTATE->m_pCurSong == nullptr )
		{
			// This normally shouldn't happen, but it's helpful to permit it for testing.
			LuaHelpers::ReportScriptErrorFmt( "GetExtraStageInfo() called in GROUP_ALL, but GAMESTATE->m_pCurSong == nullptr" );
			GAMESTATE->m_pCurSong.Set( GetRandomSong() );
		}
		sGroup = GAMESTATE->m_pCurSong->m_sGroupName;
	}

	ASSERT_M( sGroup != "", ssprintf("%p '%s' '%s'",
		static_cast<void*>(GAMESTATE->m_pCurSong.Get()),
		GAMESTATE->m_pCurSong? GAMESTATE->m_pCurSong->GetSongDir().c_str():"",
		GAMESTATE->m_pCurSong? GAMESTATE->m_pCurSong->m_sGroupName.c_str():"") );

	// Check preferred group
	if( GetExtraStageInfoFromCourse(bExtra2, sGroup, pSongOut, pStepsOut, sd->m_StepsType) )
		return;

	// Optionally, check the Songs folder for extra1/2.crs files.
	if( GetExtraStageInfoFromCourse(bExtra2, "", pSongOut, pStepsOut, sd->m_StepsType) )
		return;

	// Choose a hard song for the extra stage
	Song*	pExtra1Song = nullptr;		// the absolute hardest Song and Steps.  Use this for extra stage 1.
	Steps*	pExtra1Notes = nullptr;
	Song*	pExtra2Song = nullptr;		// a medium-hard Song and Steps.  Use this for extra stage 2.
	Steps*	pExtra2Notes = nullptr;

	const std::vector<Song*> &apSongs = GetSongs( sGroup );
	for( unsigned s=0; s<apSongs.size(); s++ )	// foreach song
	{
		Song* pSong = apSongs[s];

		std::vector<Steps*> apSteps;
		SongUtil::GetSteps( pSong, apSteps, sd->m_StepsType );
		for( unsigned n=0; n<apSteps.size(); n++ )	// foreach Steps
		{
			Steps* pSteps = apSteps[n];

			if( pExtra1Notes == nullptr || CompareNotesPointersForExtra(pExtra1Notes,pSteps) )	// pSteps is harder than pHardestNotes
			{
				pExtra1Song = pSong;
				pExtra1Notes = pSteps;
			}

			// for extra 2, we don't want to choose the hardest notes possible.  So, we'll disgard Steps with meter > 8 (assuming dance)
			if( bExtra2 && pSteps->GetMeter() > EXTRA_STAGE2_DIFFICULTY_MAX )
				continue;	// skip
			if( pExtra2Notes == nullptr  ||  CompareNotesPointersForExtra(pExtra2Notes,pSteps) )	// pSteps is harder than pHardestNotes
			{
				pExtra2Song = pSong;
				pExtra2Notes = pSteps;
			}
		}
	}

	if( pExtra2Song == nullptr  &&  pExtra1Song != nullptr )
	{
		pExtra2Song = pExtra1Song;
		pExtra2Notes = pExtra1Notes;
	}

	// If there are any notes at all that match this StepsType, everything should be filled out.
	// Also, it's guaranteed that there is at least one Steps that matches the StepsType because the player
	// had to play something before reaching the extra stage!
	ASSERT( pExtra2Song && pExtra1Song && pExtra2Notes && pExtra1Notes );

	pSongOut = (bExtra2 ? pExtra2Song : pExtra1Song);
	pStepsOut = (bExtra2 ? pExtra2Notes : pExtra1Notes);
}

Song* SongManager::GetRandomSong()
{
	if( m_pShuffledSongs.empty() )
		return nullptr;

	static int i = 0;

	for( int iThrowAway=0; iThrowAway<100; iThrowAway++ )
	{
		i++;
		wrap( i, m_pShuffledSongs.size() );
		Song *pSong = m_pShuffledSongs[ i ];
		if( pSong->IsTutorial() )
			continue;
		if( !pSong->NormallyDisplayed() )
			continue;
		return pSong;
	}

	return nullptr;
}

Course* SongManager::GetRandomCourse()
{
	if( m_pShuffledCourses.empty() )
		return nullptr;

	static int i = 0;

	for( int iThrowAway=0; iThrowAway<100; iThrowAway++ )
	{
		i++;
		wrap( i, m_pShuffledCourses.size() );
		Course *pCourse = m_pShuffledCourses[ i ];
		if( pCourse->m_bIsAutogen && !PREFSMAN->m_bAutogenGroupCourses )
			continue;
		if( pCourse->GetCourseType() == COURSE_TYPE_ENDLESS )
			continue;
		if( UNLOCKMAN->CourseIsLocked(pCourse) )
			continue;
		return pCourse;
	}

	return nullptr;
}

Song* SongManager::GetSongFromDir(RString dir) const
{
	if(dir.Right(1) != "/")
	{ dir += "/"; }

	dir.Replace('\\', '/');
	dir.MakeLower();
	std::map<RString, Song*>::const_iterator entry= m_SongsByDir.find(dir);
	if(entry != m_SongsByDir.end())
	{
		return entry->second;
	}
	return nullptr;
}

Course* SongManager::GetCourseFromPath( RString sPath ) const
{
	if( sPath == "" )
		return nullptr;

	for (Course *c : m_pCourses)
	{
		if( sPath.CompareNoCase(c->m_sPath) == 0 )
			return c;
	}

	return nullptr;
}

Course* SongManager::GetCourseFromName( RString sName ) const
{
	if( sName == "" )
		return nullptr;

	for (Course *c : m_pCourses)
		if( sName.CompareNoCase(c->GetDisplayFullTitle()) == 0 )
			return c;

	return nullptr;
}


/* GetSongDir() contains a path to the song, possibly a full path, eg:
 * Songs\Group\SongName                   or
 * My Other Song Folder\Group\SongName    or
 * c:\Corny J-pop\Group\SongName
 *
 * Most course group names are "Group\SongName", so we want to match against the
 * last two elements. Let's also support "SongName" alone, since the group is
 * only important when it's potentially ambiguous.
 *
 * Let's *not* support "Songs\Group\SongName" in course files. That's probably
 * a common error, but that would result in course files floating around that
 * only work for people who put songs in "Songs"; we don't want that. */

Song *SongManager::FindSong( RString sPath ) const
{
	sPath.Replace( '\\', '/' );
	std::vector<RString> bits;
	split( sPath, "/", bits );

	if( bits.size() == 1 )
		return FindSong( "", bits[0] );
	else if( bits.size() == 2 )
		return FindSong( bits[0], bits[1] );

	return nullptr;
}

Song *SongManager::FindSong( RString sGroup, RString sSong ) const
{
	// foreach song
	const std::vector<Song *> &vSongs = GetSongs( sGroup.empty()? GROUP_ALL:sGroup );
	for (Song *s : vSongs)
	{
		if( s->Matches(sGroup, sSong) )
			return s;
	}

	return nullptr;
}

Course *SongManager::FindCourse( RString sPath ) const
{
	sPath.Replace( '\\', '/' );
	std::vector<RString> bits;
	split( sPath, "/", bits );

	if( bits.size() == 1 )
		return FindCourse( "", bits[0] );
	else if( bits.size() == 2 )
		return FindCourse( bits[0], bits[1] );

	return nullptr;
}

Course *SongManager::FindCourse( RString sGroup, RString sName ) const
{
	for (Course *c : m_pCourses)
	{
		if( c->Matches(sGroup, sName) )
			return c;
	}

	return nullptr;
}

void SongManager::UpdatePopular()
{
	// update players best
	std::vector<Song*> apBestSongs = m_pSongs;
	for ( unsigned j=0; j < apBestSongs.size() ; ++j )
	{
		bool bFiltered = false;
		// Filter out locked songs.
		if( !apBestSongs[j]->NormallyDisplayed() )
			bFiltered = true;
		if( !bFiltered )
			continue;

		// Remove it.
		std::swap( apBestSongs[j], apBestSongs.back() );
		apBestSongs.erase( apBestSongs.end()-1 );
		--j;
	}

	SongUtil::SortSongPointerArrayByTitle( apBestSongs );

	std::vector<Course*> apBestCourses[NUM_CourseType];
	FOREACH_ENUM( CourseType, ct )
	{
		GetCourses( ct, apBestCourses[ct], PREFSMAN->m_bAutogenGroupCourses );
		CourseUtil::SortCoursePointerArrayByTitle( apBestCourses[ct] );
	}

	m_pPopularSongs = apBestSongs;
	SongUtil::SortSongPointerArrayByNumPlays( m_pPopularSongs, ProfileSlot_Machine, true );

	FOREACH_CourseType( ct )
	{
		std::vector<Course*> &vpCourses = m_pPopularCourses[ct];
		vpCourses = apBestCourses[ct];
		CourseUtil::SortCoursePointerArrayByNumPlays( vpCourses, ProfileSlot_Machine, true );
	}
}

std::map<int, std::vector<Song*>> SongManager::UpdateMeterSort( std::vector<Song*> songs) {
	// Empty the map
	m_mapSongsByDifficulty.clear();
	std::vector<Song*> apDifficultSongs = songs;
	// For each song, for each step
	for( unsigned i = 0; i < apDifficultSongs.size(); ++i )
	{
		std::vector<Steps*>	vpSteps;
		SongUtil::GetPlayableSteps( apDifficultSongs[i], vpSteps );
		for( unsigned j = 0; j < vpSteps.size(); ++j )
		{
			Steps *pSteps = vpSteps[j];
			// Check if the meter is already in m_mapSongsByDifficulty
			if (std::find(m_mapSongsByDifficulty[pSteps->GetMeter()].begin(), m_mapSongsByDifficulty[pSteps->GetMeter()].end(), apDifficultSongs[i]) != m_mapSongsByDifficulty[pSteps->GetMeter()].end())
				continue;
			else {			
				m_mapSongsByDifficulty[pSteps->GetMeter()].push_back(apDifficultSongs[i]);
			}
		}
	}
	return m_mapSongsByDifficulty;
}


void SongManager::UpdateShuffled()
{
	// update shuffled
	m_pShuffledSongs = m_pSongs;
	std::shuffle( m_pShuffledSongs.begin(), m_pShuffledSongs.end(), g_RandomNumberGenerator );

	m_pShuffledCourses = m_pCourses;
	std::shuffle( m_pShuffledCourses.begin(), m_pShuffledCourses.end(), g_RandomNumberGenerator );
}

void SongManager::SetPreferredSongs(RString sPreferredSongs, bool bIsAbsolute) {
	ASSERT( UNLOCKMAN != nullptr );

	m_vPreferredSongSort.clear();
	m_mapPreferredSectionToSongs.clear();
	std::vector<RString> asLines;
	RString sFile = sPreferredSongs;
	if (!bIsAbsolute)
		sFile = THEME->GetPathO( "SongManager", sPreferredSongs );
	GetFileContents( sFile, asLines );
	if( asLines.empty() )
		return;

	PreferredSortSection section;
	std::map<Song*, float> mapSongToPri;

	for (RString sLine : asLines)
	{
		bool bSectionDivider = BeginsWith(sLine, "---");
		if( bSectionDivider )
		{
			if( !section.vpSongs.empty() )
			{
				m_vPreferredSongSort.push_back( section );
				m_mapPreferredSectionToSongs[section.sName] = section.vpSongs;
				section = PreferredSortSection();
			}

			section.sName = sLine.Right( sLine.length() - RString("---").length() );
			TrimLeft( section.sName );
			TrimRight( section.sName );
		}
		else
		{
			/* if the line ends in slash-star, check if the section exists,
				* and if it does, add all the songs in that group to the list. */
			if( EndsWith(sLine,"/*") )
			{
				RString group = sLine.Left( sLine.length() - RString("/*").length() );
				if( DoesSongGroupExist(group) )
				{
					// add all songs in group
					const std::vector<Song*> &vSongs = GetSongs( group );
					for (Song *song : vSongs)
					{
						if( UNLOCKMAN->SongIsLocked(song) & LOCKED_SELECTABLE )
							continue;
						section.vpSongs.push_back( song );
					}
				}
			}

			Song *pSong = FindSong( sLine );
			if( pSong == nullptr )
				continue;
			if( UNLOCKMAN->SongIsLocked(pSong) & LOCKED_SELECTABLE )
				continue;
			section.vpSongs.push_back( pSong );
		}
	}

	if( !section.vpSongs.empty() )
	{
		m_vPreferredSongSort.push_back( section );
		m_mapPreferredSectionToSongs[section.sName] = section.vpSongs;
		section = PreferredSortSection();
	}

	if( MOVE_UNLOCKS_TO_BOTTOM_OF_PREFERRED_SORT.GetValue() )
	{
		// move all unlock songs to a group at the bottom
		PreferredSortSection PFSection;
		PFSection.sName = "Unlocks";
		for (UnlockEntry const &ue : UNLOCKMAN->m_UnlockEntries)
		{
			if( ue.m_Type == UnlockRewardType_Song )
			{
				Song *pSong = ue.m_Song.ToSong();
				if( pSong )
					PFSection.vpSongs.push_back( pSong );
			}
		}

		// NOTE(crashcringle): This code removed the unlocks from other sections they might have been in.
		// This was needed due to the previous 1:1 relationship between songs and section in order for the song to appear in the Unlocks section correctly.
		// Commented out for now.
		// for (std::vector<PreferredSortSection>::iterator v = m_vPreferredSongSort.begin(); v != m_vPreferredSongSort.end(); ++v)
		// {
		// 	for( int i=v->vpSongs.size()-1; i>=0; i-- )
		// 	{
		// 		Song *pSong = v->vpSongs[i];
		// 		if( find(PFSection.vpSongs.begin(),PFSection.vpSongs.end(),pSong) != PFSection.vpSongs.end() )
		// 		{
		// 			v->vpSongs.erase( v->vpSongs.begin()+i );
		// 		}
		// 	}
		// }
		
		m_vPreferredSongSort.push_back( PFSection );
		m_mapPreferredSectionToSongs[PFSection.sName] = PFSection.vpSongs;
	}

	// prune empty groups
	for( int i=m_vPreferredSongSort.size()-1; i>=0; i-- )
		if( m_vPreferredSongSort[i].vpSongs.empty() ) {
			m_vPreferredSongSort.erase( m_vPreferredSongSort.begin()+i );
			m_mapPreferredSectionToSongs.erase( m_vPreferredSongSort[i].sName );
		}

	for (PreferredSortSection const &i : m_vPreferredSongSort)
	{
		for (Song const *j : i.vpSongs)
		{
			ASSERT( j != nullptr );
		}
	}
}

void SongManager::SetPreferredCourses(RString sPreferredCourses, bool bIsAbsolute)
{
	ASSERT( UNLOCKMAN != nullptr );

	m_vPreferredCourseSort.clear();

	std::vector<RString> asLines;
	RString sFile = sPreferredCourses;
	if (!bIsAbsolute)
		sFile = THEME->GetPathO( "SongManager", sPreferredCourses );
	if( !GetFileContents(sFile, asLines) )
		return;

	std::vector<Course*> vpCourses;

	for (RString sLine : asLines)
	{
		bool bSectionDivider = BeginsWith( sLine, "---" );
		if( bSectionDivider )
		{
			if( !vpCourses.empty() )
			{
				m_vPreferredCourseSort.push_back( vpCourses );
				vpCourses.clear();
			}
			continue;
		}

		Course *pCourse = FindCourse( sLine );
		if( pCourse == nullptr )
			continue;
		if( UNLOCKMAN->CourseIsLocked(pCourse) & LOCKED_SELECTABLE )
			continue;
		vpCourses.push_back( pCourse );
	}

	if( !vpCourses.empty() )
	{
		m_vPreferredCourseSort.push_back( vpCourses );
		vpCourses.clear();
	}

	if( MOVE_UNLOCKS_TO_BOTTOM_OF_PREFERRED_SORT.GetValue() )
	{
		// move all unlock Courses to a group at the bottom
		std::vector<Course*> vpUnlockCourses;
		for (UnlockEntry const &ue : UNLOCKMAN->m_UnlockEntries)
		{
			if( ue.m_Type == UnlockRewardType_Course )
				if( ue.m_Course.IsValid() )
					vpUnlockCourses.push_back( ue.m_Course.ToCourse() );
		}

		for (auto v = m_vPreferredCourseSort.begin(); v != m_vPreferredCourseSort.end(); ++v)
		{
			for( int i=v->size()-1; i>=0; i-- )
			{
				Course *pCourse = (*v)[i];
				if( find(vpUnlockCourses.begin(),vpUnlockCourses.end(),pCourse) != vpUnlockCourses.end() )
				{
					v->erase( v->begin()+i );
				}
			}
		}

		m_vPreferredCourseSort.push_back( vpUnlockCourses );
	}

	// prune empty groups
	for( int i=m_vPreferredCourseSort.size()-1; i>=0; i-- )
		if( m_vPreferredCourseSort[i].empty() )
			m_vPreferredCourseSort.erase( m_vPreferredCourseSort.begin()+i );

	for (CoursePointerVector const &i : m_vPreferredCourseSort)
	{
		for (Course *j : i)
		{
			ASSERT( j != nullptr );
		}
	}
}

void SongManager::UpdatePreferredSort(RString sPreferredSongs, RString sPreferredCourses)
{
	SetPreferredSongs(sPreferredSongs);
	SetPreferredCourses(sPreferredCourses);
}

void SongManager::SortSongs()
{
	SongUtil::SortSongPointerArrayByTitle( m_pSongs );
}

void SongManager::UpdateRankingCourses()
{
	/* Updating the ranking courses data is fairly expensive since it involves
	 * comparing strings. Do so sparingly. */
	std::vector<RString> RankingCourses;
	split( THEME->GetMetric("ScreenRanking","CoursesToShow"),",", RankingCourses);

	for (Course *c : m_pCourses)
	{
		bool bLotsOfStages = c->GetEstimatedNumStages() > 7;
		c->m_SortOrder_Ranking = bLotsOfStages? 3 : 2;

		for( unsigned j = 0; j < RankingCourses.size(); j++ )
			if( !RankingCourses[j].CompareNoCase(c->m_sPath) )
				c->m_SortOrder_Ranking = 1;
	}
}

void SongManager::RefreshCourseGroupInfo()
{
	m_mapCourseGroupToInfo.clear();

	for (Course const * c : m_pCourses)
	{
		m_mapCourseGroupToInfo[c->m_sGroupName];	// insert
	}
}

void SongManager::LoadStepEditsFromProfileDir( const RString &sProfileDir, ProfileSlot slot )
{
	// Load all edit steps
	RString sDir = sProfileDir + EDIT_STEPS_SUBDIR;
	SSCLoader loaderSSC;
	SMLoader loaderSM;
	int iNumEditsLoaded = GetNumEditsLoadedFromProfile( slot );

	// Pass 1: Flat folder (old style)
	std::vector<RString> vsFiles;
	int size = std::min( (int) vsFiles.size(), MAX_EDIT_STEPS_PER_PROFILE - iNumEditsLoaded );
	GetDirListing( sDir+"*.edit", vsFiles, false, true );

	// XXX: If some edits are invalid and they're close to the edit limit, this may erroneously skip some edits, and won't warn.
	for( int i=0; i<size; i++ )
	{
		RString fn = vsFiles[i];
		bool bLoadedFromSSC = loaderSSC.LoadEditFromFile( fn, slot, true );
		// If we don't load the edit from a .ssc-style .edit, then we should
		// also try the .sm-style edit file. -aj
		if( !bLoadedFromSSC )
		{
			loaderSM.LoadEditFromFile( fn, slot, true );
		}
	}

	if( (int) vsFiles.size() > MAX_EDIT_STEPS_PER_PROFILE - iNumEditsLoaded )
	{
		LuaHelpers::ReportScriptErrorFmt("Profile %s has too many edits; some have been skipped.", ProfileSlotToString( slot ).c_str() );
		return;
	}

	// Some .edit files may have been invalid, so re-query instead of just += size.
	iNumEditsLoaded = GetNumEditsLoadedFromProfile( slot );

	// Pass 2: Group and song folders with #SONG inferred from folder (optional new style)
	std::vector<RString> vsGroups;
	GetDirListing( sDir+"*", vsGroups, true, false );

	// XXX: Same as above, edits may be skipped in error in some cases
	for( unsigned i=0; i<vsGroups.size(); i++ )
	{
		RString sGroupDir = vsGroups[i]+"/";
		std::vector<RString> vsSongs;
		GetDirListing(sDir+sGroupDir+"*", vsSongs, true, false );

		for( unsigned j=0; j<vsSongs.size(); j++ )
		{
			std::vector<RString> vsEdits;
			RString sSongDir = sGroupDir+vsSongs[j]+"/";
			// XXX There doesn't appear to be a songdir const?
			Song *given = GetSongFromDir( "/Songs/"+sSongDir );
			// NOTE: We don't have to check the return value of GetSongFromDir here,
			// because if it fails, it returns nullptr, which is then passed to NotesLoader*.LoadEditFromFile(),
			// which will interpret that as "we couldn't infer the song from the path",
			// which is what we want in that case anyway.
			GetDirListing(sDir+sSongDir+"/*.edit", vsEdits, false, true );
			size = std::min( (int) vsEdits.size(), MAX_EDIT_STEPS_PER_PROFILE - iNumEditsLoaded );

			for( int k=0; k<size; k++ )
			{
				RString fn = vsEdits[k];
				bool bLoadedFromSSC = loaderSSC.LoadEditFromFile( fn, slot, true, given );
				// And try again with SM
				if( !bLoadedFromSSC )
					loaderSM.LoadEditFromFile( fn, slot, true, given );
			}

			if( (int) vsEdits.size() > MAX_EDIT_STEPS_PER_PROFILE - iNumEditsLoaded )
			{
				LuaHelpers::ReportScriptErrorFmt("Profile %s has too many edits; some have been skipped.", ProfileSlotToString( slot ).c_str() );
				return;
			}

			// Some .edit files may have been invalid, so re-query instead of just += size.
			iNumEditsLoaded = GetNumEditsLoadedFromProfile( slot );
		}
	}
}

void SongManager::LoadCourseEditsFromProfileDir( const RString &sProfileDir, ProfileSlot slot )
{
	// Load all edit courses
	RString sDir = sProfileDir + EDIT_COURSES_SUBDIR;

	std::vector<RString> vsFiles;
	GetDirListing( sDir+"*.crs", vsFiles, false, true );

	int iNumEditsLoaded = GetNumEditsLoadedFromProfile( slot );
	int size = std::min( (int) vsFiles.size(), MAX_EDIT_COURSES_PER_PROFILE - iNumEditsLoaded );

	for( int i=0; i<size; i++ )
	{
		RString fn = vsFiles[i];

		CourseLoaderCRS::LoadEditFromFile( fn, slot );
	}
}

int SongManager::GetNumEditsLoadedFromProfile( ProfileSlot slot ) const
{
	int iCount = 0;
	for( unsigned s=0; s<m_pSongs.size(); s++ )
	{
		const Song *pSong = m_pSongs[s];
		std::vector<Steps*> apSteps;
		SongUtil::GetSteps( pSong, apSteps );

		for( unsigned i = 0; i < apSteps.size(); ++i )
		{
			const Steps *pSteps = apSteps[i];
			if( pSteps->WasLoadedFromProfile() && pSteps->GetLoadedFromProfileSlot() == slot )
				++iCount;
		}
	}
	return iCount;
}

void SongManager::AddSongToList(Song* new_song)
{
	new_song->SetEnabled(true);
	m_pSongs.push_back(new_song);
	RString dir= new_song->GetSongDir();
	dir.MakeLower();
	m_SongsByDir.insert(make_pair(dir, new_song));
}

void SongManager::FreeAllLoadedFromProfile( ProfileSlot slot )
{
	// Profile courses may refer to profile steps, so free profile courses first.
	std::vector<Course*> apToDelete;
	for (Course *pCourse : m_pCourses)
	{
		if( pCourse->GetLoadedFromProfileSlot() == ProfileSlot_Invalid )
			continue;

		if( slot == ProfileSlot_Invalid || pCourse->GetLoadedFromProfileSlot() == slot )
			apToDelete.push_back( pCourse );
	}

	/* We don't use DeleteCourse here, so we don't UpdatePopular and
	 * UpdateShuffled repeatedly. */
	for( unsigned i = 0; i < apToDelete.size(); ++i )
	{
		std::vector<Course*>::iterator iter = find( m_pCourses.begin(), m_pCourses.end(), apToDelete[i] );
		ASSERT( iter != m_pCourses.end() );
		m_pCourses.erase( iter );
		delete apToDelete[i];
	}

	// Popular and Shuffled may refer to courses that we just freed.
	UpdatePopular();
	UpdateShuffled();
	RefreshCourseGroupInfo();

	// Free profile steps.
	std::set<Steps*> setInUse;
	if( STATSMAN )
		STATSMAN->GetStepsInUse( setInUse );
	for (Song *s : m_pSongs)
		s->FreeAllLoadedFromProfile( slot, &setInUse );
}

int SongManager::GetNumStepsLoadedFromProfile()
{
	int iCount = 0;
	for (Song const *s : m_pSongs)
	{
		std::vector<Steps*> vpAllSteps = s->GetAllSteps();

		iCount += std::count_if(vpAllSteps.begin(), vpAllSteps.end(), [](Steps const *step) {
			return step->GetLoadedFromProfileSlot() != ProfileSlot_Invalid;
		});
	}

	return iCount;
}

template<class T>
int FindCourseIndexOfSameMode( T begin, T end, const Course *p )
{
	const PlayMode pm = p->GetPlayMode();

	int n = 0;
	for( T it = begin; it != end; ++it )
	{
		if( *it == p )
			return n;

		/* If it's not playable in this mode, don't increment. It might result in
		 * different output in different modes, but that's better than having holes. */
		if( !(*it)->IsPlayableIn( GAMESTATE->GetCurrentStyle(GAMESTATE->GetMasterPlayerNumber())->m_StepsType ) )
			continue;
		if( (*it)->GetPlayMode() != pm )
			continue;
		++n;
	}

	return -1;
}

int SongManager::GetSongRank(Song* pSong)
{
	const int index = FindIndex( m_pPopularSongs.begin(), m_pPopularSongs.end(), pSong );
	return index; // -1 means we didn't find it
}

// lua start
#include "LuaBinding.h"

/** @brief Allow Lua to have access to the SongManager. */
class LunaSongManager: public Luna<SongManager>
{
public:
	static int SetPreferredSongs( T* p, lua_State *L )
	{
		RString sPreferredSongs = SArg(1);
		if ( lua_gettop(L) >= 2 && !lua_isnil(L, 2) )
		{
			p->SetPreferredSongs( sPreferredSongs, BArg(2) );
		}
		else
		{
			p->SetPreferredSongs( sPreferredSongs );
		}

		COMMON_RETURN_SELF;
	}
	static int SetPreferredCourses( T* p, lua_State *L )
	{
		RString sPreferredCourses = SArg(1);
		if ( lua_gettop(L) >= 2 && !lua_isnil(L, 2) )
		{
			p->SetPreferredSongs( sPreferredCourses, BArg(2) );
		}
		else
		{
			p->SetPreferredSongs( sPreferredCourses );
		}
		COMMON_RETURN_SELF;
	}
	static int GetAllSongs( T* p, lua_State *L )
	{
		const std::vector<Song*> &v = p->GetAllSongs();
		LuaHelpers::CreateTableFromArray<Song*>( v, L );
		return 1;
	}
	static int GetAllCourses( T* p, lua_State *L )
	{
		std::vector<Course*> v;
		p->GetAllCourses( v, BArg(1) );
		LuaHelpers::CreateTableFromArray<Course*>( v, L );
		return 1;
	}

	static int GetPreferredSortSongs( T* p, lua_State *L )
	{
		std::vector<Song*> v;
		p->GetPreferredSortSongs(v);
		LuaHelpers::CreateTableFromArray<Song*>( v, L );
		return 1;
	}
	static int GetPreferredSortCourses( T* p, lua_State *L )
	{
		std::vector<Course*> v;
		CourseType ct = Enum::Check<CourseType>(L,1);
		p->GetPreferredSortCourses( ct, v, BArg(2) );
		LuaHelpers::CreateTableFromArray<Course*>( v, L );
		return 1;
	}

	static int FindSong( T* p, lua_State *L )		{ Song *pS = p->FindSong(SArg(1)); if(pS) pS->PushSelf(L); else lua_pushnil(L); return 1; }
	static int FindCourse( T* p, lua_State *L )		{ Course *pC = p->FindCourse(SArg(1)); if(pC) pC->PushSelf(L); else lua_pushnil(L); return 1; }
	static int GetRandomSong( T* p, lua_State *L )		{ Song *pS = p->GetRandomSong(); if(pS) pS->PushSelf(L); else lua_pushnil(L); return 1; }
	static int GetRandomCourse( T* p, lua_State *L )	{ Course *pC = p->GetRandomCourse(); if(pC) pC->PushSelf(L); else lua_pushnil(L); return 1; }
	static int GetNumSongs( T* p, lua_State *L )		{ lua_pushnumber( L, p->GetNumSongs() ); return 1; }
	static int GetNumLockedSongs( T* p, lua_State *L ) { lua_pushnumber( L, p->GetNumLockedSongs() ); return 1; }
	static int GetNumUnlockedSongs( T* p, lua_State *L )    { lua_pushnumber( L, p->GetNumUnlockedSongs() ); return 1; }
	static int GetNumSelectableAndUnlockedSongs( T* p, lua_State *L )    { lua_pushnumber( L, p->GetNumSelectableAndUnlockedSongs() ); return 1; }
	static int GetNumAdditionalSongs( T* p, lua_State *L )  { lua_pushnumber( L, 0 ); return 1; }	// deprecated
	static int GetNumSongGroups( T* p, lua_State *L )	{ lua_pushnumber( L, p->GetNumSongGroups() ); return 1; }
	static int GetNumCourses( T* p, lua_State *L )		{ lua_pushnumber( L, p->GetNumCourses() ); return 1; }
	static int GetNumAdditionalCourses( T* p, lua_State *L ){ lua_pushnumber( L, 0 ); return 1; }	// deprecated
	static int GetNumCourseGroups( T* p, lua_State *L )	{ lua_pushnumber( L, p->GetNumCourseGroups() ); return 1; }

	/* Note: this could now be implemented as Luna<Steps>::GetSong */
	static int GetSongFromSteps( T* p, lua_State *L )
	{
		Song *pSong = nullptr;
		if( lua_isnil(L,1) ) { pSong = nullptr; }
		else { Steps *pSteps = Luna<Steps>::check(L,1); pSong = pSteps->m_pSong; }
		if(pSong) pSong->PushSelf(L);
		else lua_pushnil(L);
		return 1;
	}

	static int GetExtraStageInfo( T* p, lua_State *L )
	{
		bool bExtra2 = BArg( 1 );
		const Style *pStyle = Luna<Style>::check( L, 2 );
		Song *pSong;
		Steps *pSteps;

		p->GetExtraStageInfo( bExtra2, pStyle, pSong, pSteps );
		pSong->PushSelf( L );
		pSteps->PushSelf( L );

		return 2;
	}
	DEFINE_METHOD( GetSongColor, GetSongColor( Luna<Song>::check(L,1) ) )
	DEFINE_METHOD( GetSongGroupColor, GetSongGroupColor( SArg(1) ) )
	DEFINE_METHOD( GetCourseColor, GetCourseColor( Luna<Course>::check(L,1) ) )

	static int GetSongRank( T* p, lua_State *L )
	{
		Song *pSong = Luna<Song>::check(L,1);
		int index = p->GetSongRank(pSong);
		if( index != -1 )
			lua_pushnumber(L, index+1);
		else
			lua_pushnil(L);
		return 1;
	}
	/*
	static int GetSongRankFromProfile( T* p, lua_State *L )
	{
		// it's like the above but also takes in a ProfileSlot as well.
	}
	*/

	static int GetSongGroupNames( T* p, lua_State *L )
	{
		std::vector<RString> v;
		p->GetSongGroupNames( v );
		LuaHelpers::CreateTableFromArray<RString>( v, L );
		return 1;
	}

	static int GetSongsInGroup( T* p, lua_State *L )
	{
		std::vector<Song*> v = p->GetSongs(SArg(1));
		LuaHelpers::CreateTableFromArray<Song*>( v, L );
		return 1;
	}

	static int GetCoursesInGroup( T* p, lua_State *L )
	{
		std::vector<Course*> v;
		p->GetCoursesInGroup(v,SArg(1),BArg(2));
		LuaHelpers::CreateTableFromArray<Course*>( v, L );
		return 1;
	}

	DEFINE_METHOD( ShortenGroupName, ShortenGroupName( SArg(1) ) )

	static int GetCourseGroupNames( T* p, lua_State *L )
	{
		std::vector<RString> v;
		p->GetCourseGroupNames( v );
		LuaHelpers::CreateTableFromArray<RString>( v, L );
		return 1;
	}

	DEFINE_METHOD( GetSongGroupBannerPath, GetSongGroupBannerPath(SArg(1)) );
	DEFINE_METHOD( GetCourseGroupBannerPath, GetCourseGroupBannerPath(SArg(1)) );
	DEFINE_METHOD( DoesSongGroupExist, DoesSongGroupExist(SArg(1)) );
	DEFINE_METHOD( DoesCourseGroupExist, DoesCourseGroupExist(SArg(1)) );

	static int GetPopularSongs( T* p, lua_State *L )
	{
		const std::vector<Song*> &v = p->GetPopularSongs();
		LuaHelpers::CreateTableFromArray<Song*>( v, L );
		return 1;
	}
	static int GetPopularCourses( T* p, lua_State *L )
	{
		CourseType ct = Enum::Check<CourseType>(L,1);
		const std::vector<Course*> &v = p->GetPopularCourses(ct);
		LuaHelpers::CreateTableFromArray<Course*>( v, L );
		return 1;
	}
	static int SongToPreferredSortSectionName( T* p, lua_State *L )
	{
		const Song* pSong = Luna<Song>::check(L,1);
		lua_pushstring(L, p->SongToPreferredSortSectionName(pSong));
		return 1;
	}
	static int GetPreferredSortSongsBySectionName( T* p, lua_State *L )
	{
		std::vector<Song*> v;
		p->GetPreferredSortSongsBySectionName(SArg(1), v);
		LuaHelpers::CreateTableFromArray<Song*>( v, L );
		return 1;
	}

	static int WasLoadedFromAdditionalSongs( T* p, lua_State *L )	{ lua_pushboolean(L, false); return 1; }	// deprecated
	static int WasLoadedFromAdditionalCourses( T* p, lua_State *L )	{ lua_pushboolean(L, false); return 1; }	// deprecated

	LunaSongManager()
	{
		ADD_METHOD( GetAllSongs );
		ADD_METHOD( GetAllCourses );
		ADD_METHOD( FindSong );
		ADD_METHOD( FindCourse );
		ADD_METHOD( GetRandomSong );
		ADD_METHOD( GetRandomCourse );
		ADD_METHOD( GetCourseGroupNames );
		ADD_METHOD( GetNumSongs );
		ADD_METHOD( GetNumLockedSongs );
		ADD_METHOD( GetNumUnlockedSongs );
		ADD_METHOD( GetNumSelectableAndUnlockedSongs );
		ADD_METHOD( GetNumAdditionalSongs );	// deprecated
		ADD_METHOD( GetNumSongGroups );
		ADD_METHOD( GetNumCourses );
		ADD_METHOD( GetNumAdditionalCourses );	// deprecated
		ADD_METHOD( GetNumCourseGroups );
		ADD_METHOD( GetSongFromSteps );
		ADD_METHOD( GetExtraStageInfo );
		ADD_METHOD( GetSongColor );
		ADD_METHOD( GetSongGroupColor );
		ADD_METHOD( GetCourseColor );
		ADD_METHOD( GetSongRank );
		ADD_METHOD( GetSongGroupNames );
		ADD_METHOD( GetSongsInGroup );
		ADD_METHOD( GetCoursesInGroup );
		ADD_METHOD( ShortenGroupName );
		ADD_METHOD( SetPreferredSongs );
		ADD_METHOD( SetPreferredCourses );
		ADD_METHOD( GetPreferredSortSongs );
		ADD_METHOD( GetPreferredSortCourses );
		ADD_METHOD( GetSongGroupBannerPath );
		ADD_METHOD( GetCourseGroupBannerPath );
		ADD_METHOD( DoesSongGroupExist );
		ADD_METHOD( DoesCourseGroupExist );
		ADD_METHOD( GetPopularSongs );
		ADD_METHOD( GetPopularCourses );
		ADD_METHOD( SongToPreferredSortSectionName );
		ADD_METHOD( GetPreferredSortSongsBySectionName );
		ADD_METHOD( WasLoadedFromAdditionalSongs );	// deprecated
		ADD_METHOD( WasLoadedFromAdditionalCourses );	// deprecated
	}
};

LUA_REGISTER_CLASS( SongManager )
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
