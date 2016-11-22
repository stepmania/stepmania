#include "global.h"
#include "SongManager.h"
#include "arch/LoadingWindow/LoadingWindow.h"
#include "ActorUtil.h"
#include "AnnouncerManager.h"
#include "BackgroundUtil.h"
#include "BannerCache.h"
#include "CommonMetrics.h"
#include "Course.h"
#include "CourseLoaderCRS.h"
#include "CourseUtil.h"
#include "GameManager.h"
#include "GameState.h"
#include "LocalizedString.h"
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

#include <unordered_map>

using std::vector;

SongManager*	SONGMAN = nullptr;	// global and accessible from anywhere in our program

const std::string ADDITIONAL_SONGS_DIR	= "/AdditionalSongs/";
const std::string ADDITIONAL_COURSES_DIR	= "/AdditionalCourses/";
const std::string EDIT_SUBDIR		= "Edits/";

/** @brief The file that contains various random attacks. */
const std::string ATTACK_FILE		= "/Data/RandomAttacks.txt";

static const ThemeMetric<Rage::Color>	EXTRA_COLOR			( "SongManager", "ExtraColor" );
static const ThemeMetric<int>		EXTRA_COLOR_METER		( "SongManager", "ExtraColorMeter" );
static const ThemeMetric<bool>		USE_PREFERRED_SORT_COLOR	( "SongManager", "UsePreferredSortColor" );
static const ThemeMetric<bool>		USE_UNLOCK_COLOR		( "SongManager", "UseUnlockColor" );
static const ThemeMetric<Rage::Color>	UNLOCK_COLOR			( "SongManager", "UnlockColor" );
static const ThemeMetric<bool>		MOVE_UNLOCKS_TO_BOTTOM_OF_PREFERRED_SORT	( "SongManager", "MoveUnlocksToBottomOfPreferredSort" );
static const ThemeMetric<int>		EXTRA_STAGE2_DIFFICULTY_MAX	( "SongManager", "ExtraStage2DifficultyMax" );

static Preference<std::string> g_sDisabledSongs( "DisabledSongs", "" );
static Preference<bool> g_bHideIncompleteCourses( "HideIncompleteCourses", false );

std::string SONG_GROUP_COLOR_NAME( size_t i )   { return fmt::sprintf( "SongGroupColor%i", (int) i+1 ); }
std::string COURSE_GROUP_COLOR_NAME( size_t i ) { return fmt::sprintf( "CourseGroupColor%i", (int) i+1 ); }
std::string profile_song_group_color_name(size_t i) { return fmt::sprintf("ProfileSongGroupColor%i", (int)i+1); }

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
	SONG_GROUP_COLOR		.Load( "SongManager", SONG_GROUP_COLOR_NAME, NUM_SONG_GROUP_COLORS );
	NUM_COURSE_GROUP_COLORS	.Load( "SongManager", "NumCourseGroupColors" );
	COURSE_GROUP_COLOR		.Load( "SongManager", COURSE_GROUP_COLOR_NAME, NUM_COURSE_GROUP_COLORS );
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

void SongManager::InitAll( LoadingWindow *ld )
{
	auto never_cache = Rage::split(PREFSMAN->m_NeverCacheList.Get(), ",");
	for (auto &group: never_cache)
	{
		m_GroupsToNeverCache.insert(group);
	}
	InitSongsFromDisk( ld );
	InitCoursesFromDisk( ld );
	InitAutogenCourses();
	InitRandomAttacks();
}

static LocalizedString RELOADING ( "SongManager", "Reloading..." );
static LocalizedString UNLOADING_SONGS ( "SongManager", "Unloading songs..." );
static LocalizedString UNLOADING_COURSES ( "SongManager", "Unloading courses..." );

void SongManager::Reload( bool bAllowFastLoad, LoadingWindow *ld )
{
	FILEMAN->FlushDirCache( SpecialFiles::SONGS_DIR );
	FILEMAN->FlushDirCache( ADDITIONAL_SONGS_DIR );
	FILEMAN->FlushDirCache( SpecialFiles::COURSES_DIR );
	FILEMAN->FlushDirCache( ADDITIONAL_COURSES_DIR );
	FILEMAN->FlushDirCache( EDIT_SUBDIR );

	if( ld )
		ld->SetText( RELOADING.GetValue() );

	// save scores before unloading songs, or the scores will be lost
	PROFILEMAN->SaveMachineProfile();

	if( ld )
		ld->SetText( UNLOADING_COURSES.GetValue() );

	FreeCourses();

	if( ld )
		ld->SetText( UNLOADING_SONGS.GetValue() );

	FreeSongs();

	const bool OldVal = PREFSMAN->m_bFastLoad;
	if( !bAllowFastLoad )
		PREFSMAN->m_bFastLoad.Set( false );

	InitAll( ld );

	// reload scores and unlocks afterward
	PROFILEMAN->LoadMachineProfile();
	UNLOCKMAN->Reload();

	if( !bAllowFastLoad )
		PREFSMAN->m_bFastLoad.Set( OldVal );

	UpdatePreferredSort();
}

void SongManager::InitSongsFromDisk( LoadingWindow *ld )
{
	RageTimer tm;
	// Tell SONGINDEX to not write the cache index file every time a song adds
	// an entry. -Kyz
	SONGINDEX->delay_save_cache= true;
	LoadStepManiaSongDir( SpecialFiles::SONGS_DIR, ld );

	const bool bOldVal = PREFSMAN->m_bFastLoad;
	PREFSMAN->m_bFastLoad.Set( PREFSMAN->m_bFastLoadAdditionalSongs );
	LoadStepManiaSongDir( ADDITIONAL_SONGS_DIR, ld );
	PREFSMAN->m_bFastLoad.Set( bOldVal );
	LoadEnabledSongsFromPref();
	SONGINDEX->SaveCacheIndex();
	SONGINDEX->delay_save_cache= false;

	LOG->Trace( "Found %d songs in %f seconds.", (int)m_pSongs.size(), tm.GetDeltaTime() );
}

static LocalizedString FOLDER_CONTAINS_MUSIC_FILES( "SongManager", "The folder \"%s\" appears to be a song folder.  All song folders must reside in a group folder.  For example, \"Songs/Originals/My Song\"." );

void SongManager::AddGroup(std::string dir, std::string group_dir_name)
{
	if(m_song_groups.find(dir) != m_song_groups.end())
	{
		return; // the group is already added
	}

	// Look for a group banner in this group folder
	vector<std::string> images;
	std::string slash_group_name= dir + group_dir_name + "/";
	FILEMAN->GetDirListingWithMultipleExtensions(slash_group_name,
		ActorUtil::GetTypeExtensionList(FT_Bitmap), images);
	std::string banner_path;
	if(!images.empty())
	{
		banner_path= slash_group_name + images[0];
	}
	else
	{
		banner_path= get_possible_group_banner(group_dir_name);
	}

	/* Other group graphics are a bit trickier, and usually don't exist.
	 * A themer has a few options, namely checking the aspect ratio and
	 * operating on it. -aj
	 * TODO: Once the files are implemented in Song, bring the extensions
	 * from there into here. -aj */
	// Group background

	//vector<std::string> arrayGroupBackgrounds;
	//GetDirListing( sDir+sGroupDirName+"/*-bg.png", arrayGroupBanners );
	//GetDirListing( sDir+sGroupDirName+"/*-bg.jpg", arrayGroupBanners );
	//GetDirListing( sDir+sGroupDirName+"/*-bg.jpeg", arrayGroupBanners );
	//GetDirListing( sDir+sGroupDirName+"/*-bg.gif", arrayGroupBanners );
	//GetDirListing( sDir+sGroupDirName+"/*-bg.bmp", arrayGroupBanners );
/*
	std::string sBackgroundPath;
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
	GroupEntry entry= {m_song_groups.size(), banner_path};
	m_song_groups.insert(make_pair(group_dir_name, entry));
	m_song_group_names.push_back(group_dir_name);
	//m_sSongGroupBackgroundPaths.push_back( sBackgroundPath );
}

static LocalizedString LOADING_SONGS ( "SongManager", "Loading songs..." );
void SongManager::LoadStepManiaSongDir( std::string sDir, LoadingWindow *ld )
{
	// Compositors and other stuff can impose some overhead on updating the
	// loading window, which slows down startup time for some people.
	// loading_window_last_update_time provides a timer so the loading window
	// isn't updated after every song and course. -Kyz
	RageTimer loading_window_last_update_time;
	loading_window_last_update_time.Touch();
	// Make sure sDir has a trailing slash.
	if (!Rage::ends_with(sDir, "/"))
	{
		sDir += "/";
	}
	// Find all group directories in "Songs" folder
	vector<std::string> arrayGroupDirs;
	GetDirListing( sDir+"*", arrayGroupDirs, true );
	StripCvsAndSvn( arrayGroupDirs );
	StripMacResourceForks( arrayGroupDirs );
	SortStringArray( arrayGroupDirs );

	// TODO: Add a function to FILEMAN for getting dirs and files in separate
	// vectors, and use that to combine the above GetDirListing and this one,
	// so the list is only fetched once. -Kyz
	m_possible_group_banners.clear();
	vector<std::string> images_in_dir;
	FILEMAN->GetDirListingWithMultipleExtensions(sDir,
		ActorUtil::GetTypeExtensionList(FT_Bitmap), images_in_dir);
	for(auto&& image : images_in_dir)
	{
		m_possible_group_banners.insert(make_pair(
				GetFileNameWithoutExtension(image), sDir + image));
	}

	int const updates_per_group= 50;
	if(ld)
	{
		ld->SetIndeterminate(false);
		ld->SetTotalWork(arrayGroupDirs.size() * updates_per_group);
	}
	auto const & audio_exts= ActorUtil::GetTypeExtensionList(FT_Sound);
	for(size_t group_index= 0; group_index < arrayGroupDirs.size(); ++group_index)
	{
		auto& group_name= arrayGroupDirs[group_index];
		vector<std::string> song_folders;
		GetDirListing(sDir + group_name + "/*", song_folders, true, true);
		StripCvsAndSvn(song_folders);
		StripMacResourceForks(song_folders);
		// Song Select sorts songs by name, already, doesn't it?  Why do the song
		// names need to be sorted during loading? -Kyz
		// SortStringArray(song_folders);
		LOG->Trace("Attempting to load %i songs from \"%s\"",
			int(song_folders.size()), sDir+group_name);
		int loaded= 0;
		SongPointerVector& index_entry = m_mapSongGroupIndex[group_name];
		auto group_base_name= Rage::base_name(group_name);

		for(size_t song_index= 0; song_index < song_folders.size(); ++song_index)
		{
			auto& song_dir_name= song_folders[song_index];
			// Check to see if they put a song directly inside the group folder.
			// TODO: If this check fails, log a warning instead of crashing.
			// -Unknown
			// I think crashing is the only way to get the message across so people
			// put songs in the right place. -Kyz
			std::string const ext= GetExtension(song_dir_name);
			if(!ext.empty())
			{
				for(auto&& aud : audio_exts)
				{
					if(ext == aud)
					{
						RageException::Throw(
							FOLDER_CONTAINS_MUSIC_FILES.GetValue(), sDir.c_str());
					}
				}
			}
			// this is a song directory. Load a new song.
			if(ld && loading_window_last_update_time.Ago() > next_loading_window_update)
			{
				loading_window_last_update_time.Touch();
				ld->SetProgress((group_index * updates_per_group) +
					(Rage::scale(int(song_index), 0, int(song_folders.size()), 0, updates_per_group)));
				ld->SetText(LOADING_SONGS.GetValue() +
					fmt::sprintf("\n%s\n%s",
						group_base_name.c_str(),
						Rage::base_name(song_dir_name).c_str()));
			}
			Song* new_song= new Song;
			if(!new_song->LoadFromSongDir(song_dir_name))
			{
				// The song failed to load.
				delete new_song;
				continue;
			}
			AddSongToList(new_song);
			index_entry.push_back(new_song);
			++loaded;
		}

		LOG->Trace("Loaded %i songs from \"%s\"", loaded, (sDir+group_name).c_str());

		// Don't add the group name if we didn't load any songs in this group.
		if(loaded <= 0) { continue; }

		// Add this group to the group array.
		AddGroup(sDir, group_name);

		// Cache and load the group banner. (and background if it has one -aj)
		BANNERCACHE->CacheBanner(GetSongGroupBannerPath(group_name));

		// Load the group sym links (if any)
		LoadGroupSymLinks(sDir, group_name);
	}

	m_possible_group_banners.clear();
	if( ld ) {
		ld->SetIndeterminate( true );
	}
}

// Instead of "symlinks", songs should have membership in multiple groups. -Chris
void SongManager::LoadGroupSymLinks(std::string sDir, std::string sGroupFolder)
{
	// Find all symlink files in this folder
	vector<std::string> arraySymLinks;
	GetDirListing( sDir+sGroupFolder+"/*.include", arraySymLinks, false );
	SortStringArray( arraySymLinks );
	SongPointerVector& index_entry = m_mapSongGroupIndex[sGroupFolder];
	for (auto &symlink: arraySymLinks)
	{
		MsdFile msdF;
		msdF.ReadFile( sDir+sGroupFolder+"/"+symlink.c_str(), false );  // don't unescape
		std::string	sSymDestination = msdF.GetParam(0,1); // Should only be 1 value & param...period.

		Song* pNewSong = new Song;
		if( !pNewSong->LoadFromSongDir( sSymDestination ) )
		{
			delete pNewSong; // The song failed to load.
		}
		else
		{
			const vector<Steps*>& vpSteps = pNewSong->GetAllSteps();
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
	if( PREFSMAN->m_BannerCache != BNCACHE_FULL )
		return;

	/* Load textures before unloading old ones, so we don't reload textures
	 * that we don't need to. */
	RageTexturePreloader preload;

	const vector<Song*> &songs = GetAllSongs();
	for (auto const *song: songs)
	{
		if( !song->HasBanner() )
		{
			continue;
		}
		const RageTextureID ID = Sprite::SongBannerTexture( song->GetBannerPath() );
		preload.Load( ID );
	}

	vector<Course*> courses;
	GetAllCourses( courses, false );
	for (auto const *course: courses)
	{
		if( !course->HasBanner() )
		{
			continue;
		}
		const RageTextureID ID = Sprite::SongBannerTexture( course->GetBannerPath() );
		preload.Load( ID );
	}

	preload.Swap( m_TexturePreload );
}

void SongManager::FreeSongs()
{
	m_song_groups.clear();
	//m_sSongGroupBackgroundPaths.clear();

	for (auto *song: m_pSongs)
	{
		Rage::safe_delete( song );
	}
	m_pSongs.clear();
	m_SongsByDir.clear();

	// also free the songs that have been deleted from disk
	for ( unsigned i=0; i<m_pDeletedSongs.size(); ++i )
		Rage::safe_delete( m_pDeletedSongs[i] );
	m_pDeletedSongs.clear();

	m_mapSongGroupIndex.clear();

	m_pPopularSongs.clear();
	m_pShuffledSongs.clear();
}

void SongManager::UnlistSong(Song *song)
{
	// cannot immediately free song data, as it is needed temporarily for smooth audio transitions, etc.
	// Instead, remove it from the m_pSongs list and store it in a special place where it can safely be deleted later.
	m_pDeletedSongs.push_back(song);

	// remove all occurences of the song in each of our song vectors
	vector<Song*>* songVectors[3] = { &m_pSongs, &m_pPopularSongs, &m_pShuffledSongs };
	for (int songVecIdx=0; songVecIdx<3; ++songVecIdx) {
		vector<Song*>& v = *songVectors[songVecIdx];
		for (size_t i = 0; i < v.size(); ++i) {
			if (v[i] == song) {
				v.erase(v.begin()+i);
				--i;
			}
		}
	}
}

bool SongManager::IsGroupNeverCached(const std::string& group) const
{
	return m_GroupsToNeverCache.find(group) != m_GroupsToNeverCache.end();
}

std::string SongManager::GetSongGroupBannerPath(std::string const& group_name) const
{
	auto entry= m_song_groups.find(group_name);
	if(entry != m_song_groups.end())
	{
		return entry->second.banner_path;
	}
	return std::string();
}
/*
std::string SongManager::GetSongGroupBackgroundPath( std::string sSongGroup ) const
{
	for( unsigned i = 0; i < m_sSongGroupNames.size(); ++i )
	{
		if( sSongGroup == m_sSongGroupNames[i] )
			return m_sSongGroupBackgroundPaths[i];
	}

	return std::string();
}
*/
void SongManager::GetSongGroupNames( vector<std::string> &AddTo ) const
{
	AddTo.reserve(AddTo.size() + m_song_group_names.size());
	for(auto&& entry : m_song_group_names)
	{
		AddTo.push_back(entry);
	}
}

bool SongManager::DoesSongGroupExist(std::string const& group_name) const
{
	return m_song_groups.find(group_name) != m_song_groups.end();
}

Rage::Color SongManager::GetSongGroupColor(std::string const& group_name) const
{
	auto entry= m_song_groups.find(group_name);
	if(entry != m_song_groups.end())
	{
		return SONG_GROUP_COLOR.GetValue(entry->second.id % NUM_SONG_GROUP_COLORS);
	}
	FOREACH_EnabledPlayer(pn)
	{
		Profile* prof= PROFILEMAN->GetProfile(pn);
		if(prof != nullptr)
		{
			if(prof->GetDisplayNameOrHighScoreName() == group_name)
			{
				return profile_song_group_colors.GetValue(pn % num_profile_song_group_colors);
			}
		}
	}

	LuaHelpers::ReportScriptError(fmt::sprintf(
			"requested color for song group '%s' that doesn't exist",
			group_name.c_str()));
	return Rage::Color(1,1,1,1);
}

Rage::Color SongManager::GetSongColor( const Song* pSong ) const
{
	ASSERT( pSong != nullptr );

	// protected by royal freem corporation. any modification/removal of
	// this code will result in prosecution.
	if( pSong->m_sMainTitle == "DVNO")
		return Rage::Color(1.0f,0.8f,0.0f,1.0f);
	// end royal freem protection

	// Use unlock color if applicable
	const UnlockEntry *pUE = UNLOCKMAN->FindSong( pSong );
	if( pUE && USE_UNLOCK_COLOR.GetValue() )
		return UNLOCK_COLOR.GetValue();

	if( USE_PREFERRED_SORT_COLOR )
	{
		for (auto v = m_vPreferredSongSort.begin(); v != m_vPreferredSongSort.end(); ++v)
		{
			for (auto const *s: v->vpSongs)
			{
				if( s == pSong )
				{
					int i = v - m_vPreferredSongSort.begin();
					return SONG_GROUP_COLOR.GetValue( i%NUM_SONG_GROUP_COLORS );
				}
			}
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
		const vector<Steps*>& vpSteps = pSong->GetAllSteps();
		for (auto const *pSteps: vpSteps)
		{
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
				return (Rage::Color)EXTRA_COLOR;
		}

		return GetSongGroupColor( pSong->m_sGroupName );
	}
}

std::string SongManager::GetCourseGroupBannerPath( const std::string &sCourseGroup ) const
{
	auto iter = m_mapCourseGroupToInfo.find(sCourseGroup);
	if( iter == m_mapCourseGroupToInfo.end() )
	{
		LuaHelpers::ReportScriptError(fmt::sprintf("requested banner for course group '%s' that doesn't exist",sCourseGroup.c_str()));
		return std::string();
	}
	else
	{
		return iter->second.m_sBannerPath;
	}
}

void SongManager::GetCourseGroupNames( vector<std::string> &AddTo ) const
{
	for (auto const &iter: m_mapCourseGroupToInfo)
	{
		AddTo.push_back( iter.first );
	}
}

bool SongManager::DoesCourseGroupExist( const std::string &sCourseGroup ) const
{
	return m_mapCourseGroupToInfo.find(sCourseGroup) != m_mapCourseGroupToInfo.end();
}

Rage::Color SongManager::GetCourseGroupColor( const std::string &sCourseGroup ) const
{
	int iIndex = 0;
	for (auto const &iter: m_mapCourseGroupToInfo)
	{
		if( iter.first == sCourseGroup )
		{
			return SONG_GROUP_COLOR.GetValue( iIndex%NUM_SONG_GROUP_COLORS );
		}
		iIndex++;
	}

	LuaHelpers::ReportScriptError(fmt::sprintf("requested color for course group '%s' that doesn't exist",sCourseGroup.c_str()));
	return Rage::Color(1,1,1,1);
}

Rage::Color SongManager::GetCourseColor( const Course* pCourse ) const
{
	// Use unlock color if applicable
	const UnlockEntry *pUE = UNLOCKMAN->FindCourse( pCourse );
	if( pUE  &&  USE_UNLOCK_COLOR.GetValue() )
		return UNLOCK_COLOR.GetValue();

	if( USE_PREFERRED_SORT_COLOR )
	{
		for (auto v = m_vPreferredCourseSort.begin(); v != m_vPreferredCourseSort.end(); ++v)
		{
			for (auto const *s: *v)
			{
				if( s == pCourse )
				{
					int i = v - m_vPreferredCourseSort.begin();
					CHECKPOINT_M( fmt::sprintf( "%i, NUM_COURSE_GROUP_COLORS = %i", i, NUM_COURSE_GROUP_COLORS.GetValue()) );
					return COURSE_GROUP_COLOR.GetValue( i % NUM_COURSE_GROUP_COLORS );
				}
			}
		}

		int i = m_vPreferredCourseSort.size();
		CHECKPOINT_M( fmt::sprintf( "%i, NUM_COURSE_GROUP_COLORS = %i", i, NUM_COURSE_GROUP_COLORS.GetValue()) );
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
	SONG_GROUP_COLOR		.Load( "SongManager", SONG_GROUP_COLOR_NAME, NUM_SONG_GROUP_COLORS );
	NUM_COURSE_GROUP_COLORS .Load( "SongManager", "NumCourseGroupColors" );
	COURSE_GROUP_COLOR		.Load( "SongManager", COURSE_GROUP_COLOR_NAME, NUM_COURSE_GROUP_COLORS );
}

const vector<Song*> &SongManager::GetSongs( const std::string &sGroupName ) const
{
	static const vector<Song*> vEmpty;

	if( sGroupName == GROUP_ALL )
		return m_pSongs;
	auto iter = m_mapSongGroupIndex.find(sGroupName);
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

void SongManager::GetPreferredSortSongs( vector<Song*> &AddTo ) const
{
	if( m_vPreferredSongSort.empty() )
	{
		AddTo.insert( AddTo.end(), m_pSongs.begin(), m_pSongs.end() );
		return;
	}

	for (auto const &v: m_vPreferredSongSort)
	{
		AddTo.insert( AddTo.end(), v.vpSongs.begin(), v.vpSongs.end() );
	}
}

std::string SongManager::SongToPreferredSortSectionName( const Song *pSong ) const
{
	for (auto const &v: m_vPreferredSongSort)
	{
		for (auto const *s: v.vpSongs)
		{
			if( s == pSong )
			{
				return v.sName;
			}
		}
	}
	return std::string();
}

void SongManager::GetPreferredSortCourses( CourseType ct, vector<Course*> &AddTo, bool bIncludeAutogen ) const
{
	if( m_vPreferredCourseSort.empty() )
	{
		GetCourses( ct, AddTo, bIncludeAutogen );
		return;
	}

	for (auto const &v: m_vPreferredCourseSort)
	{
		for (auto *pCourse: v)
		{
			if( pCourse->GetCourseType() == ct )
			{
				AddTo.push_back( pCourse );
			}
		}
	}
}

int SongManager::GetNumSongs() const
{
	return m_pSongs.size();
}

int SongManager::GetNumLockedSongs() const
{
	auto isSongLocked = [](Song const *s) {
		return UNLOCKMAN->SongIsLocked(s);
	};
	return std::count_if(m_pSongs.begin(), m_pSongs.end(), isSongLocked);
}

int SongManager::GetNumUnlockedSongs() const
{
	auto isSongLocked = [](Song const *s) {
		return UNLOCKMAN->SongIsLocked(s) & ~LOCKED_LOCK;
	};
	return std::count_if(m_pSongs.begin(), m_pSongs.end(), isSongLocked);
}

int SongManager::GetNumSelectableAndUnlockedSongs() const
{
	auto isSongLocked = [](Song const *s) {
		return UNLOCKMAN->SongIsLocked(s) & ~(LOCKED_LOCK | LOCKED_SELECTABLE);
	};
	return std::count_if(m_pSongs.begin(), m_pSongs.end(), isSongLocked);
}

int SongManager::GetNumAdditionalSongs() const
{
	auto wasLoadedFromAdditional = [this](Song const *s) {
		return WasLoadedFromAdditionalSongs(s);
	};
	return std::count_if(m_pSongs.begin(), m_pSongs.end(), wasLoadedFromAdditional);
}

int SongManager::GetNumSongGroups() const
{
	return m_song_groups.size();
}

int SongManager::GetNumCourses() const
{
	return m_pCourses.size();
}

int SongManager::GetNumAdditionalCourses() const
{
	auto wasLoadedFromAdditional = [this](Course const *c) {
		return WasLoadedFromAdditionalCourses(c);
	};
	return std::count_if(m_pCourses.begin(), m_pCourses.end(), wasLoadedFromAdditional);
}

int SongManager::GetNumCourseGroups() const
{
	return m_mapCourseGroupToInfo.size();
}

std::string SongManager::ShortenGroupName( std::string sLongGroupName )
{
	static TitleSubst tsub("Groups");

	TitleFields title;
	title.Title = sLongGroupName;
	tsub.Subst( title );
	return title.Title;
}

static LocalizedString LOADING_COURSES ( "SongManager", "Loading courses..." );
void SongManager::InitCoursesFromDisk( LoadingWindow *ld )
{
	LOG->Trace( "Loading courses." );
	RageTimer loading_window_last_update_time;
	loading_window_last_update_time.Touch();

	vector<std::string> vsCourseDirs;
	vsCourseDirs.push_back( SpecialFiles::COURSES_DIR );
	vsCourseDirs.push_back( ADDITIONAL_COURSES_DIR );

	vector<std::string> vsCourseGroupNames;
	for (auto const &sDir: vsCourseDirs)
	{
		// Find all group directories in Courses dir
		GetDirListing( sDir + "*", vsCourseGroupNames, true, true );
		StripCvsAndSvn( vsCourseGroupNames );
		StripMacResourceForks( vsCourseGroupNames );
	}

	// Search for courses both in COURSES_DIR and in subdirectories
	vsCourseGroupNames.push_back( SpecialFiles::COURSES_DIR );
	SortStringArray( vsCourseGroupNames );

	int courseIndex = 0;
	for (auto const &sCourseGroup: vsCourseGroupNames)
	{
		// Find all CRS files in this group directory
		vector<std::string> vsCoursePaths;
		GetDirListing( sCourseGroup + "/*.crs", vsCoursePaths, false, true );
		SortStringArray( vsCoursePaths );

		if( ld )
		{
			ld->SetIndeterminate( false );
			ld->SetTotalWork( vsCoursePaths.size() );
		}

		auto base_course_group= Rage::base_name(sCourseGroup);
		for (auto const &sCoursePath: vsCoursePaths)
		{
			if(ld && loading_window_last_update_time.Ago() > next_loading_window_update)
			{
				loading_window_last_update_time.Touch();
				ld->SetProgress(courseIndex);
				ld->SetText( LOADING_COURSES.GetValue()+fmt::sprintf("\n%s\n%s",
					base_course_group.c_str(),
					Rage::base_name(sCoursePath).c_str()));
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
	vector<std::string> saGroupNames;
	this->GetSongGroupNames( saGroupNames );
	Course* pCourse;
	for (auto const &sGroupName: saGroupNames)
	{
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

	vector<Song*> apCourseSongs = GetAllSongs();

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
		vector<Song*> apSongs = this->GetAllSongs();
		SongUtil::SortSongPointerArrayByDisplayArtist( apSongs );

		std::string sCurArtist = "";
		std::string sCurArtistTranslit = "";
		int iCurArtistCount = 0;

		vector<Song *> aSongs;
		unsigned i = 0;
		Rage::ci_ascii_string unknownArtist{ "Unknown artist" };
		do {
			std::string sArtist = i >= apSongs.size()? std::string(""): apSongs[i]->GetDisplayArtist();
			std::string sTranslitArtist = i >= apSongs.size()? std::string(""): apSongs[i]->GetTranslitArtist();
			Rage::ci_ascii_string ciArtist{ sArtist.c_str() };
			if( i < apSongs.size() && ciArtist == sCurArtist )
			{
				aSongs.push_back( apSongs[i] );
				++iCurArtistCount;
				continue;
			}

			/* Different artist, or we're at the end. If we have enough entries for
			 * the last artist, add it. Skip blanks and "Unknown artist". */
			if( iCurArtistCount >= 3 && sCurArtistTranslit != "" &&
				unknownArtist != sCurArtist && unknownArtist != sCurArtistTranslit )
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
				std::string sType = sParams[0];
				std::string sAttack = sParams[1];

				if( iNumParams > 2 )
				{
					LuaHelpers::ReportScriptErrorFmt( "Got \"%s:%s\" tag with too many parameters", sType.c_str(), sAttack.c_str() );
					continue;
				}

				if (Rage::ci_ascii_string{ sType.c_str() } != "ATTACK")
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
	for (auto *course: m_pCourses)
	{
		delete course;
	}
	m_pCourses.clear();

	FOREACH_CourseType( ct )
	{
		m_pPopularCourses[ct].clear();
	}
	m_pShuffledCourses.clear();

	m_mapCourseGroupToInfo.clear();
}

void SongManager::DeleteAutogenCourses()
{
	vector<Course*> vNewCourses;
	for( auto it = m_pCourses.begin(); it != m_pCourses.end(); ++it )
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
	vector<Course*>::iterator iter = find( m_pCourses.begin(), m_pCourses.end(), pCourse );
	ASSERT( iter != m_pCourses.end() );
	m_pCourses.erase( iter );
	UpdatePopular();
	UpdateShuffled();
	RefreshCourseGroupInfo();
}

void SongManager::InvalidateCachedTrails()
{
	for (auto const *pCourse: m_pCourses)
	{
		if( pCourse->IsAnEdit() )
		{
			pCourse->m_TrailCache.clear();
		}
	}
}

/* Called periodically to wipe out cached NoteData. This is called when we
 * change screens. */
void SongManager::Cleanup()
{
	for (auto *pSong: m_pSongs)
	{
		if (pSong)
		{
			auto &vpSteps = pSong->GetAllSteps();
			for (auto *pSteps: vpSteps)
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

	for (auto *pCourse: this->m_pCourses)
	{
		pCourse->Invalidate( pStaleSong );
	}

	InitAutogenCourses();

	UpdatePopular();
	UpdateShuffled();
	RefreshCourseGroupInfo();
}

void SongManager::RegenerateNonFixedCourses()
{
	for (auto *course: m_pCourses)
	{
		course->RegenerateNonFixedTrails();
	}
}

void SongManager::SetPreferences()
{
	for (auto *song: m_pSongs)
	{
		// PREFSMAN->m_bAutogenSteps may have changed.
		song->RemoveAutoGenNotes();
		song->AddAutoGenNotes();
	}
}

void SongManager::SaveEnabledSongsToPref()
{
	vector<std::string> vsDisabledSongs;

	// Intentionally drop disabled song entries for songs that aren't currently loaded.

	auto &apSongs = SONGMAN->GetAllSongs();
	for (auto const *pSong: apSongs)
	{
		SongID sid;
		sid.FromSong( pSong );
		if( !pSong->GetEnabled() )
		{
			vsDisabledSongs.push_back( sid.ToString() );
		}
	}
	g_sDisabledSongs.Set( Rage::join(";", vsDisabledSongs) );
}

void SongManager::LoadEnabledSongsFromPref()
{
	auto asDisabledSongs = Rage::split(g_sDisabledSongs.Get(), ";", Rage::EmptyEntries::skip);

	for (auto &s: asDisabledSongs)
	{
		SongID sid;
		sid.FromString( s );
		Song *pSong = sid.ToSong();
		if( pSong )
		{
			pSong->SetEnabled( false );
		}
	}
}

void SongManager::GetStepsLoadedFromProfile( vector<Steps*> &AddTo, ProfileSlot slot ) const
{
	auto &vSongs = GetAllSongs();
	for (auto const *song: vSongs)
	{
		song->GetStepsLoadedFromProfile( slot, AddTo );
	}
}

void SongManager::DeleteSteps( Steps *pSteps )
{
	pSteps->m_pSong->DeleteSteps( pSteps );
}

bool SongManager::WasLoadedFromAdditionalSongs( const Song *pSong ) const
{
	std::string sDir = pSong->GetSongDir();
	return Rage::starts_with( sDir, ADDITIONAL_SONGS_DIR );
}

bool SongManager::WasLoadedFromAdditionalCourses( const Course *pCourse ) const
{
	std::string sDir = pCourse->m_sPath;
	return Rage::starts_with( sDir, ADDITIONAL_COURSES_DIR );
}

void SongManager::GetAllCourses( vector<Course*> &AddTo, bool bIncludeAutogen ) const
{
	for (auto *course: m_pCourses)
	{
		if( bIncludeAutogen || !course->m_bIsAutogen )
		{
			AddTo.push_back( course );
		}
	}
}

void SongManager::GetCourses( CourseType ct, vector<Course*> &AddTo, bool bIncludeAutogen ) const
{
	for (auto *course: m_pCourses)
	{
		if( course->GetCourseType() == ct )
		{
			if( bIncludeAutogen || !course->m_bIsAutogen )
			{
				AddTo.push_back( course );
			}
		}
	}
}

void SongManager::GetCoursesInGroup( vector<Course*> &AddTo, const std::string &sCourseGroup, bool bIncludeAutogen ) const
{
	for (auto *course: m_pCourses)
	{
		if( course->m_sGroupName == sCourseGroup )
		{
			if( bIncludeAutogen || !course->m_bIsAutogen )
			{
				AddTo.push_back( course );
			}
		}
	}
}

bool SongManager::GetExtraStageInfoFromCourse( bool bExtra2, std::string sPreferredGroup, Song*& pSongOut, Steps*& pStepsOut, StepsType stype )
{
	const std::string sCourseSuffix = sPreferredGroup + (bExtra2 ? "/extra2.crs" : "/extra1.crs");
	std::string sCoursePath = SpecialFiles::SONGS_DIR + sCourseSuffix;

	// Couldn't find course in DWI path or alternative song folders
	if( !DoesFileExist(sCoursePath) )
	{
		sCoursePath = ADDITIONAL_SONGS_DIR + sCourseSuffix;
		if( !DoesFileExist(sCoursePath) )
			return false;
	}

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
	using std::min;
	// Equate CHALLENGE to HARD.
	Difficulty d1 = min(n1->GetDifficulty(), Difficulty_Hard);
	Difficulty d2 = min(n2->GetDifficulty(), Difficulty_Hard);

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
	std::string sGroup = GAMESTATE->m_sPreferredSongGroup;
	if( sGroup == GROUP_ALL )
	{
		if( GAMESTATE->get_curr_song() == nullptr )
		{
			// This normally shouldn't happen, but it's helpful to permit it for testing.
			LuaHelpers::ReportScriptErrorFmt( "GetExtraStageInfo() called in GROUP_ALL, but GAMESTATE->get_curr_song() == nullptr" );
			GAMESTATE->set_curr_song(GetRandomSong());
		}
		sGroup = GAMESTATE->get_curr_song()->m_sGroupName;
	}

	ASSERT_M( sGroup != "", fmt::sprintf("%p '%s' '%s'",
		static_cast<void *>(GAMESTATE->get_curr_song()),
		GAMESTATE->get_curr_song()? GAMESTATE->get_curr_song()->GetSongDir().c_str():"",
		GAMESTATE->get_curr_song()? GAMESTATE->get_curr_song()->m_sGroupName.c_str():"") );

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

	auto &apSongs = GetSongs( sGroup );
	for (auto *pSong: apSongs)
	{
		vector<Steps*> apSteps;
		SongUtil::GetSteps( pSong, apSteps, sd->m_StepsType );
		for (auto *pSteps: apSteps)
		{
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

std::string SongManager::GetSongGroupByIndex(unsigned index)
{
	return m_song_group_names[index];
}

Song* SongManager::GetSongFromDir(std::string dir) const
{
	if (!Rage::ends_with(dir, "/"))
	{
		dir += "/"; 
	}

	Rage::replace(dir, '\\', '/');
	dir = Rage::make_lower(dir);
	auto entry = m_SongsByDir.find(dir);
	if(entry != m_SongsByDir.end())
	{
		return entry->second;
	}
	return nullptr;
}

Course* SongManager::GetCourseFromPath( std::string sPath ) const
{
	if (sPath == "")
	{
		return nullptr;
	}
	Rage::ci_ascii_string ciPath{ sPath.c_str() };
	for (auto *c : m_pCourses)
	{
		if (ciPath == c->m_sPath)
		{
			return c;
		}
	}

	return nullptr;
}

Course* SongManager::GetCourseFromName( std::string sName ) const
{
	if (sName == "")
	{
		return nullptr;
	}
	Rage::ci_ascii_string ciName{ sName.c_str() };
	for (auto *course: m_pCourses)
	{
		if (ciName == course->GetDisplayFullTitle())
		{
			return course;
		}
	}

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

Song *SongManager::FindSong( std::string sPath ) const
{
	Rage::replace(sPath, '\\', '/' );
	auto bits = Rage::split( sPath, "/" );

	if( bits.size() == 1 )
	{
		return FindSong( "", bits[0] );
	}
	if( bits.size() == 2 )
	{
		return FindSong( bits[0], bits[1] );
	}
	return nullptr;
}

Song *SongManager::FindSong( std::string sGroup, std::string sSong ) const
{
	// foreach song
	const vector<Song *> &vSongs = GetSongs( sGroup.empty()? GROUP_ALL:sGroup );
	for (auto *s: vSongs)
	{
		if( s->Matches(sGroup, sSong) )
		{
			return s;
		}
	}

	return nullptr;
}

Course *SongManager::FindCourse( std::string sPath ) const
{
	Rage::replace(sPath, '\\', '/' );
	auto bits = Rage::split( sPath, "/" );

	if( bits.size() == 1 )
	{
		return FindCourse( "", bits[0] );
	}
	if( bits.size() == 2 )
	{
		return FindCourse( bits[0], bits[1] );
	}
	return nullptr;
}

Course *SongManager::FindCourse( std::string sGroup, std::string sName ) const
{
	for (auto *c: m_pCourses)
	{
		if( c->Matches(sGroup, sName) )
		{
			return c;
		}
	}

	return nullptr;
}

void SongManager::UpdatePopular()
{
	// update players best
	vector<Song*> apBestSongs = m_pSongs;
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

	vector<Course*> apBestCourses[NUM_CourseType];
	FOREACH_ENUM( CourseType, ct )
	{
		GetCourses( ct, apBestCourses[ct], PREFSMAN->m_bAutogenGroupCourses );
		CourseUtil::SortCoursePointerArrayByTitle( apBestCourses[ct] );
	}

	m_pPopularSongs = apBestSongs;
	SongUtil::SortSongPointerArrayByNumPlays( m_pPopularSongs, ProfileSlot_Machine, true );

	FOREACH_CourseType( ct )
	{
		vector<Course*> &vpCourses = m_pPopularCourses[ct];
		vpCourses = apBestCourses[ct];
		CourseUtil::SortCoursePointerArrayByNumPlays( vpCourses, ProfileSlot_Machine, true );
	}
}

void SongManager::UpdateShuffled()
{
	// update shuffled
	m_pShuffledSongs = m_pSongs;
	std::shuffle( m_pShuffledSongs.begin(), m_pShuffledSongs.end(), g_RandomNumberGenerator );

	m_pShuffledCourses = m_pCourses;
	std::shuffle( m_pShuffledCourses.begin(), m_pShuffledCourses.end(), g_RandomNumberGenerator );
}

void SongManager::UpdatePreferredSort(std::string sPreferredSongs, std::string sPreferredCourses)
{
	ASSERT( UNLOCKMAN != nullptr );

	{
		m_vPreferredSongSort.clear();

		vector<std::string> asLines;
		std::string sFile = THEME->GetPathO( "SongManager", sPreferredSongs );
		GetFileContents( sFile, asLines );
		if( asLines.empty() )
			return;

		PreferredSortSection section;

		for (auto &s: asLines)
		{
			std::string sLine = s;

			bool bSectionDivider = Rage::starts_with(sLine, "---");
			if( bSectionDivider )
			{
				if( !section.vpSongs.empty() )
				{
					m_vPreferredSongSort.push_back( section );
					section = PreferredSortSection();
				}

				section.sName = Rage::trim(Rage::tail(sLine, -3));
			}
			else
			{
				/* if the line ends in slash-star, check if the section exists,
				 * and if it does, add all the songs in that group to the list. */
				if( Rage::ends_with(sLine,"/*") )
				{
					std::string group = Rage::head(sLine, sLine.size() - 2);
					if( DoesSongGroupExist(group) )
					{
						// add all songs in group
						auto const &vSongs = GetSongs( group );
						for (auto *song: vSongs)
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
			section = PreferredSortSection();
		}

		if( MOVE_UNLOCKS_TO_BOTTOM_OF_PREFERRED_SORT.GetValue() )
		{
			// move all unlock songs to a group at the bottom
			PreferredSortSection PFSection;
			PFSection.sName = "Unlocks";
			for (auto const &ue: UNLOCKMAN->m_UnlockEntries)
			{
				if( ue.m_Type == UnlockRewardType_Song )
				{
					Song *pSong = ue.m_Song.ToSong();
					if( pSong )
						PFSection.vpSongs.push_back( pSong );
				}
			}

			for (auto &v: m_vPreferredSongSort)
			{
				for( int i=v.vpSongs.size()-1; i>=0; i-- )
				{
					Song *pSong = v.vpSongs[i];
					if( find(PFSection.vpSongs.begin(),PFSection.vpSongs.end(),pSong) != PFSection.vpSongs.end() )
					{
						v.vpSongs.erase( v.vpSongs.begin()+i );
					}
				}
			}

			m_vPreferredSongSort.push_back( PFSection );
		}

		// prune empty groups
		for( int i=m_vPreferredSongSort.size()-1; i>=0; i-- )
			if( m_vPreferredSongSort[i].vpSongs.empty() )
				m_vPreferredSongSort.erase( m_vPreferredSongSort.begin()+i );

		for (auto const &i: m_vPreferredSongSort)
		{
			for (auto const *j: i.vpSongs)
			{
				ASSERT( j != nullptr );
			}
		}
	}

	{
		m_vPreferredCourseSort.clear();

		vector<std::string> asLines;
		std::string sFile = THEME->GetPathO( "SongManager", sPreferredCourses );
		if( !GetFileContents(sFile, asLines) )
			return;

		vector<Course*> vpCourses;

		for (auto &s: asLines)
		{
			std::string sLine = s;
			bool bSectionDivider = Rage::starts_with( sLine, "---" );
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
			vector<Course*> vpUnlockCourses;
			for (auto const &ue: UNLOCKMAN->m_UnlockEntries)
			{
				if( ue.m_Type == UnlockRewardType_Course )
				{
					if( ue.m_Course.IsValid() )
					{
						vpUnlockCourses.push_back( ue.m_Course.ToCourse() );
					}
				}
			}

			for (auto &v: m_vPreferredCourseSort)
			{
				for( int i=v.size()-1; i>=0; i-- )
				{
					Course *pCourse = v[i];
					if( find(vpUnlockCourses.begin(),vpUnlockCourses.end(),pCourse) != vpUnlockCourses.end() )
					{
						v.erase( v.begin()+i );
					}
				}
			}

			m_vPreferredCourseSort.push_back( vpUnlockCourses );
		}

		// prune empty groups
		for( int i=m_vPreferredCourseSort.size()-1; i>=0; i-- )
			if( m_vPreferredCourseSort[i].empty() )
				m_vPreferredCourseSort.erase( m_vPreferredCourseSort.begin()+i );

		for (auto const &i: m_vPreferredCourseSort)
		{
			for (auto const *j: i)
			{
				ASSERT( j != nullptr );
			}
		}
	}
}

void SongManager::SortSongs()
{
	SongUtil::SortSongPointerArrayByTitle( m_pSongs );
}

void SongManager::UpdateRankingCourses()
{
	/* Updating the ranking courses data is fairly expensive since it involves
	 * comparing strings. Do so sparingly. */
	auto rankingCourses = Rage::split(THEME->GetMetric("ScreenRanking", "CoursesToShow"), ",");

	for (auto *c: m_pCourses)
	{
		bool bLotsOfStages = c->GetEstimatedNumStages() > 7;
		c->m_SortOrder_Ranking = bLotsOfStages? 3 : 2;
		Rage::ci_ascii_string ciPath{ c->m_sPath.c_str() };
		for (auto &course: rankingCourses)
		{
			if (ciPath == course)
			{
				c->m_SortOrder_Ranking = 1;
			}
		}
	}
}

void SongManager::RefreshCourseGroupInfo()
{
	m_mapCourseGroupToInfo.clear();

	for (auto const *c: m_pCourses)
	{
		m_mapCourseGroupToInfo[c->m_sGroupName];	// insert
	}

	// TODO: Search for course group banners
}

void SongManager::LoadStepEditsFromProfileDir( const std::string &sProfileDir, ProfileSlot slot )
{
	using std::min;
	// Load all edit steps
	std::string sDir = sProfileDir + EDIT_STEPS_SUBDIR;
	SSCLoader loaderSSC;
	SMLoader loaderSM;
	int iNumEditsLoaded = GetNumEditsLoadedFromProfile( slot );

	// Pass 1: Flat folder (old style)
	vector<std::string> vsFiles;
	int size = min( static_cast<int>(vsFiles.size()), MAX_EDIT_STEPS_PER_PROFILE - iNumEditsLoaded );
	GetDirListing( sDir+"*.edit", vsFiles, false, true );

	// XXX: If some edits are invalid and they're close to the edit limit, this may erroneously skip some edits, and won't warn.
	for( int i=0; i<size; i++ )
	{
		std::string fn = vsFiles[i];
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
	vector<std::string> vsGroups;
	GetDirListing( sDir+"*", vsGroups, true, false );

	// XXX: Same as above, edits may be skipped in error in some cases
	for (auto &group: vsGroups)
	{
		std::string sGroupDir = group+"/";
		vector<std::string> vsSongs;
		GetDirListing(sDir+sGroupDir+"*", vsSongs, true, false );

		for (auto &song: vsSongs)
		{
			vector<std::string> vsEdits;
			std::string sSongDir = sGroupDir+song+"/";
			// XXX There doesn't appear to be a songdir const?
			Song *given = GetSongFromDir( "/Songs/"+sSongDir );
			// NOTE: We don't have to check the return value of GetSongFromDir here,
			// because if it fails, it returns nullptr, which is then passed to NotesLoader*.LoadEditFromFile(),
			// which will interpret that as "we couldn't infer the song from the path",
			// which is what we want in that case anyway.
			GetDirListing(sDir+sSongDir+"/*.edit", vsEdits, false, true );
			size = min( (int) vsEdits.size(), MAX_EDIT_STEPS_PER_PROFILE - iNumEditsLoaded );

			for( int k=0; k<size; k++ )
			{
				std::string fn = vsEdits[k];
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

void SongManager::LoadCourseEditsFromProfileDir( const std::string &sProfileDir, ProfileSlot slot )
{
	using std::min;
	// Load all edit courses
	std::string sDir = sProfileDir + EDIT_COURSES_SUBDIR;

	vector<std::string> vsFiles;
	GetDirListing( sDir+"*.crs", vsFiles, false, true );

	int iNumEditsLoaded = GetNumEditsLoadedFromProfile( slot );
	int size = min( (int) vsFiles.size(), MAX_EDIT_COURSES_PER_PROFILE - iNumEditsLoaded );

	for( int i=0; i<size; i++ )
	{
		std::string fn = vsFiles[i];

		CourseLoaderCRS::LoadEditFromFile( fn, slot );
	}
}

int SongManager::GetNumEditsLoadedFromProfile( ProfileSlot slot ) const
{
	// TODO: Either use std::accumulate or std::count_if.
	int iCount = 0;
	for (auto const *pSong: m_pSongs)
	{
		vector<Steps*> apSteps;
		SongUtil::GetSteps( pSong, apSteps );

		for (auto const *pSteps: apSteps)
		{
			if( pSteps->WasLoadedFromProfile() && pSteps->GetLoadedFromProfileSlot() == slot )
			{
				++iCount;
			}
		}
	}
	return iCount;
}

std::string SongManager::get_possible_group_banner(std::string group_dir_name)
{
	auto entry= m_possible_group_banners.find(group_dir_name);
	if(entry != m_possible_group_banners.end())
	{
		return entry->second;
	}
	return std::string();
}


void SongManager::AddSongToList(Song* new_song)
{
	new_song->SetEnabled(true);
	m_pSongs.push_back(new_song);
	std::string dir= Rage::make_lower(new_song->GetSongDir());
	m_SongsByDir.insert(std::make_pair(dir, new_song));
}

void SongManager::FreeAllLoadedFromProfile( ProfileSlot slot )
{
	// Profile courses may refer to profile steps, so free profile courses first.
	vector<Course*> apToDelete;
	for (auto *pCourse: m_pCourses)
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
		vector<Course*>::iterator iter = find( m_pCourses.begin(), m_pCourses.end(), apToDelete[i] );
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
	for (auto *s: m_pSongs)
	{
		s->FreeAllLoadedFromProfile( slot, &setInUse );
	}
}

int SongManager::GetNumStepsLoadedFromProfile()
{
	// TODO: Find a way to utilize std::count or std::accumulate.
	int iCount = 0;
	auto isProfileValid = [](Steps const *step) {
		return step->GetLoadedFromProfileSlot() != ProfileSlot_Invalid;
	};
	for (auto const *s: m_pSongs)
	{
		vector<Steps*> vpAllSteps = s->GetAllSteps();

		iCount += std::count_if(vpAllSteps.begin(), vpAllSteps.end(), isProfileValid);
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
		p->UpdatePreferredSort( SArg(1), "PreferredCourses.txt" );
		COMMON_RETURN_SELF;
	}

	static int SetPreferredCourses( T* p, lua_State *L )
	{
		p->UpdatePreferredSort( "PreferredSongs.txt", SArg(1) );
		COMMON_RETURN_SELF;
	}
	static int GetAllSongs( T* p, lua_State *L )
	{
		const vector<Song*> &v = p->GetAllSongs();
		LuaHelpers::CreateTableFromArray<Song*>( v, L );
		return 1;
	}
	static int GetAllCourses( T* p, lua_State *L )
	{
		vector<Course*> v;
		p->GetAllCourses( v, BArg(1) );
		LuaHelpers::CreateTableFromArray<Course*>( v, L );
		return 1;
	}

	static int GetPreferredSortSongs( T* p, lua_State *L )
	{
		vector<Song*> v;
		p->GetPreferredSortSongs(v);
		LuaHelpers::CreateTableFromArray<Song*>( v, L );
		return 1;
	}
	static int GetPreferredSortCourses( T* p, lua_State *L )
	{
		vector<Course*> v;
		CourseType ct = Enum::Check<CourseType>(L,1);
		p->GetPreferredSortCourses( ct, v, BArg(2) );
		LuaHelpers::CreateTableFromArray<Course*>( v, L );
		return 1;
	}

	static int find_song_by_dir(T* p, lua_State* L)
	{
		Song* song= p->GetSongFromDir(SArg(1));
		if(song == nullptr)
		{
			lua_pushnil(L);
		}
		else
		{
			song->PushSelf(L);
		}
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
	static int GetNumAdditionalSongs( T* p, lua_State *L )  { lua_pushnumber( L, p->GetNumAdditionalSongs() ); return 1; }
	static int GetNumSongGroups( T* p, lua_State *L )	{ lua_pushnumber( L, p->GetNumSongGroups() ); return 1; }
	static int GetNumCourses( T* p, lua_State *L )		{ lua_pushnumber( L, p->GetNumCourses() ); return 1; }
	static int GetNumAdditionalCourses( T* p, lua_State *L ){ lua_pushnumber( L, p->GetNumAdditionalCourses() ); return 1; }
	static int GetNumCourseGroups( T* p, lua_State *L )	{ lua_pushnumber( L, p->GetNumCourseGroups() ); return 1; }
	/* Note: this could now be implemented as Luna<Steps>::GetSong */
	static int GetSongFromSteps( T*, lua_State *L )
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
		vector<std::string> v;
		p->GetSongGroupNames( v );
		LuaHelpers::CreateTableFromArray<std::string>( v, L );
		return 1;
	}

	static int GetSongsInGroup( T* p, lua_State *L )
	{
		vector<Song*> v = p->GetSongs(SArg(1));
		LuaHelpers::CreateTableFromArray<Song*>( v, L );
		return 1;
	}

	static int GetCoursesInGroup( T* p, lua_State *L )
	{
		vector<Course*> v;
		p->GetCoursesInGroup(v,SArg(1),BArg(2));
		LuaHelpers::CreateTableFromArray<Course*>( v, L );
		return 1;
	}

	DEFINE_METHOD( ShortenGroupName, ShortenGroupName( SArg(1) ) )

	static int GetCourseGroupNames( T* p, lua_State *L )
	{
		vector<std::string> v;
		p->GetCourseGroupNames( v );
		LuaHelpers::CreateTableFromArray<std::string>( v, L );
		return 1;
	}

	DEFINE_METHOD( GetSongGroupBannerPath, GetSongGroupBannerPath(SArg(1)) );
	DEFINE_METHOD( GetCourseGroupBannerPath, GetCourseGroupBannerPath(SArg(1)) );
	DEFINE_METHOD( DoesSongGroupExist, DoesSongGroupExist(SArg(1)) );
	DEFINE_METHOD( DoesCourseGroupExist, DoesCourseGroupExist(SArg(1)) );

	static int GetPopularSongs( T* p, lua_State *L )
	{
		const vector<Song*> &v = p->GetPopularSongs();
		LuaHelpers::CreateTableFromArray<Song*>( v, L );
		return 1;
	}
	static int GetPopularCourses( T* p, lua_State *L )
	{
		CourseType ct = Enum::Check<CourseType>(L,1);
		const vector<Course*> &v = p->GetPopularCourses(ct);
		LuaHelpers::CreateTableFromArray<Course*>( v, L );
		return 1;
	}
	static int SongToPreferredSortSectionName( T* p, lua_State *L )
	{
		const Song* pSong = Luna<Song>::check(L,1);
		lua_pushstring(L, p->SongToPreferredSortSectionName(pSong).c_str());
		return 1;
	}
	static int WasLoadedFromAdditionalSongs( T* p, lua_State *L )
	{
		const Song* pSong = Luna<Song>::check(L,1);
		lua_pushboolean(L, p->WasLoadedFromAdditionalSongs(pSong));
		return 1;
	}
	static int WasLoadedFromAdditionalCourses( T* p, lua_State *L )
	{
		const Course* pCourse = Luna<Course>::check(L,1);
		lua_pushboolean(L, p->WasLoadedFromAdditionalCourses(pCourse));
		return 1;
	}

	LunaSongManager()
	{
		ADD_METHOD( GetAllSongs );
		ADD_METHOD( GetAllCourses );
		ADD_METHOD(find_song_by_dir);
		ADD_METHOD( FindSong );
		ADD_METHOD( FindCourse );
		ADD_METHOD( GetRandomSong );
		ADD_METHOD( GetRandomCourse );
		ADD_METHOD( GetCourseGroupNames );
		ADD_METHOD( GetNumSongs );
		ADD_METHOD( GetNumLockedSongs );
		ADD_METHOD( GetNumUnlockedSongs );
		ADD_METHOD( GetNumSelectableAndUnlockedSongs );
		ADD_METHOD( GetNumAdditionalSongs );
		ADD_METHOD( GetNumSongGroups );
		ADD_METHOD( GetNumCourses );
		ADD_METHOD( GetNumAdditionalCourses );
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
		ADD_METHOD( WasLoadedFromAdditionalSongs );
		ADD_METHOD( WasLoadedFromAdditionalCourses );
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
