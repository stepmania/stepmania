#ifndef SONGMANAGER_H
#define SONGMANAGER_H

class LoadingWindow;
class Song;
class Style;
class Steps;
class PlayerOptions;
struct lua_State;

#include "RageTypes.h"
#include "GameConstantsAndTypes.h"
#include "SongOptions.h"
#include "PlayerOptions.h"
#include "PlayerNumber.h"
#include "Difficulty.h"
#include "Course.h"
#include "ThemeMetric.h"
#include "RageTexturePreloader.h"
#include "RageUtil.h"

std::string SONG_GROUP_COLOR_NAME( size_t i );
std::string COURSE_GROUP_COLOR_NAME( size_t i );
bool CompareNotesPointersForExtra(const Steps *n1, const Steps *n2);

/** @brief The max number of edit steps a profile can have. */
const int MAX_EDIT_STEPS_PER_PROFILE	= 200;
/** @brief The max number of edit courses a profile can have. */
const int MAX_EDIT_COURSES_PER_PROFILE	= 20;

/** @brief The holder for the Songs and its Steps. */
class SongManager
{
public:
	SongManager();
	~SongManager();

	void InitSongsFromDisk( LoadingWindow *ld );
	void FreeSongs();
	void UnlistSong(Song *song);
	void Cleanup();

	void Invalidate( const Song *pStaleSong );

	void RegenerateNonFixedCourses();
	void SetPreferences();
	void SaveEnabledSongsToPref();
	void LoadEnabledSongsFromPref();

	void LoadStepEditsFromProfileDir( const std::string &sProfileDir, ProfileSlot slot );
	void LoadCourseEditsFromProfileDir( const std::string &sProfileDir, ProfileSlot slot );
	int GetNumStepsLoadedFromProfile();
	void FreeAllLoadedFromProfile( ProfileSlot slot = ProfileSlot_Invalid );

	void LoadGroupSymLinks( std::string sDir, std::string sGroupFolder );

	void InitCoursesFromDisk( LoadingWindow *ld );
	void InitAutogenCourses();
	void InitRandomAttacks();
	void FreeCourses();
	void AddCourse( Course *pCourse );	// transfers ownership of pCourse
	void DeleteCourse( Course *pCourse );	// transfers ownership of pCourse
	/** @brief Remove all of the auto generated courses. */
	void DeleteAutogenCourses();
	void InvalidateCachedTrails();

	void InitAll( LoadingWindow *ld );	// songs, courses, groups - everything.
	void Reload( bool bAllowFastLoad, LoadingWindow *ld=nullptr );	// songs, courses, groups - everything.
	void PreloadSongImages();

	bool IsGroupNeverCached(const std::string& group) const;

	std::string GetSongGroupBannerPath(std::string const& group_name) const;
	//std::string GetSongGroupBackgroundPath( std::string sSongGroup ) const;
	void GetSongGroupNames( std::vector<std::string> &AddTo ) const;
	bool DoesSongGroupExist(std::string const& group_name) const;
	Rage::Color GetSongGroupColor(std::string const& group_name) const;
	Rage::Color GetSongColor( const Song* pSong ) const;

	std::string GetCourseGroupBannerPath( const std::string &sCourseGroup ) const;
	//std::string GetCourseGroupBackgroundPath( const std::string &sCourseGroup ) const;
	void GetCourseGroupNames( std::vector<std::string> &AddTo ) const;
	bool DoesCourseGroupExist( const std::string &sCourseGroup ) const;
	Rage::Color GetCourseGroupColor( const std::string &sCourseGroupName ) const;
	Rage::Color GetCourseColor( const Course* pCourse ) const;

	void ResetGroupColors();

	static std::string ShortenGroupName( std::string sLongGroupName );

	// Lookup
	/**
	 * @brief Retrieve all of the songs that belong to a particular group.
	 * @param sGroupName the name of the group.
	 * @return the songs that belong in the group. */
	const std::vector<Song*> &GetSongs( const std::string &sGroupName ) const;
	/**
	 * @brief Retrieve all of the songs in the game.
	 * @return all of the songs. */
	const std::vector<Song*> &GetAllSongs() const { return GetSongs(GROUP_ALL); }
	/**
	 * @brief Retrieve all of the popular songs.
	 *
	 * Popularity is determined specifically by the number of times
	 * a song is chosen.
	 * @return all of the popular songs. */
	const std::vector<Song*> &GetPopularSongs() const { return m_pPopularSongs; }

	/**
	 * @brief Retrieve all of the songs in a group that have at least one
	 * valid step for the current gametype.
	 * @param sGroupName the name of the group.
	 * @return the songs within the group that have at least one valid Step. */
	const std::vector<Song *> &GetSongsOfCurrentGame( const std::string &sGroupName ) const;
	/**
	 * @brief Retrieve all of the songs in the game that have at least one
	 * valid step for the current gametype.
	 * @return the songs within the game that have at least one valid Step. */
	const std::vector<Song *> &GetAllSongsOfCurrentGame() const;

	void GetPreferredSortSongs( std::vector<Song*> &AddTo ) const;
	std::string SongToPreferredSortSectionName( const Song *pSong ) const;
	const std::vector<Course*> &GetPopularCourses( CourseType ct ) const { return m_pPopularCourses[ct]; }
	Song *FindSong( std::string sPath ) const;
	Song *FindSong( std::string sGroup, std::string sSong ) const;
	Course *FindCourse( std::string sPath ) const;
	Course *FindCourse( std::string sGroup, std::string sName ) const;
	/**
	 * @brief Retrieve the number of songs in the game.
	 * @return the number of songs. */
	int GetNumSongs() const;
	int GetNumLockedSongs() const;
	int GetNumUnlockedSongs() const;
	int GetNumSelectableAndUnlockedSongs() const;
	int GetNumAdditionalSongs() const;
	int GetNumSongGroups() const;
	/**
	 * @brief Retrieve the number of courses in the game.
	 * @return the number of courses. */
	int GetNumCourses() const;
	int GetNumAdditionalCourses() const;
	int GetNumCourseGroups() const;
	Song* GetRandomSong();
	Course* GetRandomCourse();
	// sm-ssc addition:
	std::string GetSongGroupByIndex(unsigned index);
	int GetSongRank(Song* pSong);

	void GetStepsLoadedFromProfile( std::vector<Steps*> &AddTo, ProfileSlot slot ) const;
	void DeleteSteps( Steps *pSteps );	// transfers ownership of pSteps
	bool WasLoadedFromAdditionalSongs( const Song *pSong ) const;
	bool WasLoadedFromAdditionalCourses( const Course *pCourse ) const;

	void GetAllCourses( std::vector<Course*> &AddTo, bool bIncludeAutogen ) const;
	void GetCourses( CourseType ct, std::vector<Course*> &AddTo, bool bIncludeAutogen ) const;
	void GetCoursesInGroup( std::vector<Course*> &AddTo, const std::string &sCourseGroup, bool bIncludeAutogen ) const;
	void GetPreferredSortCourses( CourseType ct, std::vector<Course*> &AddTo, bool bIncludeAutogen ) const;

	void GetExtraStageInfo( bool bExtra2, const Style *s, Song*& pSongOut, Steps*& pStepsOut );
	Song* GetSongFromDir( std::string sDir ) const;
	Course* GetCourseFromPath( std::string sPath ) const;	// path to .crs file, or path to song group dir
	Course* GetCourseFromName( std::string sName ) const;

	void UpdatePopular();
	void UpdateShuffled();	// re-shuffle songs and courses
	void UpdatePreferredSort(std::string sPreferredSongs = "PreferredSongs.txt", std::string sPreferredCourses = "PreferredCourses.txt");
	void SortSongs();		// sort m_pSongs by CompareSongPointersByTitle

	void UpdateRankingCourses();	// courses shown on the ranking screen
	void RefreshCourseGroupInfo();

	// Lua
	void PushSelf( lua_State *L );

protected:
	void LoadStepManiaSongDir( std::string sDir, LoadingWindow *ld );
	void LoadDWISongDir( std::string sDir );
	bool GetExtraStageInfoFromCourse( bool bExtra2, std::string sPreferredGroup, Song*& pSongOut, Steps*& pStepsOut, StepsType stype );
	void AddGroup(std::string dir, std::string group_dir_name);
	int GetNumEditsLoadedFromProfile( ProfileSlot slot ) const;

	std::string get_possible_group_banner(std::string group_dir_name);

	void AddSongToList(Song* new_song);
	/** @brief All of the songs that can be played. */
	std::vector<Song*>		m_pSongs;
	std::unordered_map<std::string, Song*> m_SongsByDir;
	std::set<std::string> m_GroupsToNeverCache;
	/** @brief Hold pointers to all the songs that have been deleted from disk but must at least be kept temporarily alive for smooth audio transitions. */
	std::vector<Song*>       m_pDeletedSongs;
	/** @brief The most popular songs ranked by number of plays. */
	std::vector<Song*>		m_pPopularSongs;
	//std::vector<Song*>		m_pRecentSongs;	// songs recently played on the machine
	std::vector<Song*>		m_pShuffledSongs;	// used by GetRandomSong
	struct PreferredSortSection
	{
		std::string sName;
		std::vector<Song*> vpSongs;
	};
	std::vector<PreferredSortSection> m_vPreferredSongSort;
	struct GroupEntry
	{
		size_t id; // For GetSongGroupColor.
		std::string banner_path;
		// std::string background_path; // Does someone want to add background support? -Kyz
	};
	std::map<std::string, GroupEntry> m_song_groups;
	std::vector<std::string> m_song_group_names; // For GetSongGroupByIndex
	std::map<std::string, std::string> m_possible_group_banners;

	typedef std::vector<Song*> SongPointerVector;
	std::unordered_map<std::string,SongPointerVector> m_mapSongGroupIndex;

	std::vector<Course*>		m_pCourses;
	std::vector<Course*>		m_pPopularCourses[NUM_CourseType];
	std::vector<Course*>		m_pShuffledCourses;	// used by GetRandomCourse
	struct CourseGroupInfo
	{
		std::string m_sBannerPath;
		//std::string m_sBackgroundPath;
	};
	std::unordered_map<std::string,CourseGroupInfo> m_mapCourseGroupToInfo;
	typedef std::vector<Course*> CoursePointerVector;
	std::vector<CoursePointerVector> m_vPreferredCourseSort;

	RageTexturePreloader m_TexturePreload;

	ThemeMetric<int>		NUM_SONG_GROUP_COLORS;
	ThemeMetric1D<Rage::Color>	SONG_GROUP_COLOR;
	ThemeMetric<int>		NUM_COURSE_GROUP_COLORS;
	ThemeMetric1D<Rage::Color>	COURSE_GROUP_COLOR;
	ThemeMetric<int> num_profile_song_group_colors;
	ThemeMetric1D<Rage::Color> profile_song_group_colors;
};

extern SongManager*	SONGMAN;	// global and accessible from anywhere in our program

#endif

/**
 * @file
 * @author Chris Danford, Glenn Maynard (c) 2001-2004
 * @section LICENSE
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
