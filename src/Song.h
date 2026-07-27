#ifndef SONG_H
#define SONG_H

#include "Attack.h"
#include "TimingData.h"
#include "Difficulty.h"
#include "EnumHelper.h"
#include "RageUtil_AutoPtr.h"
#include "RageTypes.h"
#include "Steps.h"

#include <set>
#include <vector>


class Style;
class StepsID;
struct lua_State;
struct BackgroundChange;

void FixupPath( RString &path, const RString &sSongPath );
RString GetSongAssetPath( RString sPath, const RString &sSongPath );

/** @brief The version of the .ssc file format. */
const static float STEPFILE_VERSION_NUMBER = 0.83f;

/** @brief How many edits for this song can each profile have? */
const int MAX_EDITS_PER_SONG_PER_PROFILE = 15;
/** @brief How many edits for this song can be available? */
const int MAX_EDITS_PER_SONG = MAX_EDITS_PER_SONG_PER_PROFILE * NUM_ProfileSlot;

extern const int FILE_CACHE_VERSION;

/** @brief The different background layers available. */
enum BackgroundLayer
{
	BACKGROUND_LAYER_1,
	BACKGROUND_LAYER_2,
	//BACKGROUND_LAYER_3, // StepCollection get
	NUM_BackgroundLayer,
	BACKGROUND_LAYER_Invalid
};

/** @brief A custom foreach loop for the different background layers. */
#define FOREACH_BackgroundLayer( bl ) FOREACH_ENUM( BackgroundLayer, bl )

/** @brief Different instrument tracks for band type games. */
enum InstrumentTrack
{
	InstrumentTrack_Guitar,
	InstrumentTrack_Rhythm,
	InstrumentTrack_Bass,
	NUM_InstrumentTrack,
	InstrumentTrack_Invalid
};
const RString& InstrumentTrackToString( InstrumentTrack it );
InstrumentTrack StringToInstrumentTrack( const RString& s );

/** @brief The collection of lyrics for the Song. */
struct LyricSegment
{
	float	m_fStartTime; /** @brief When does the lyric show up? */
	RString m_sLyric; /** @brief The lyrics themselves. */
	RageColor m_Color; /** @brief The color of the lyrics. */
};

/** @brief Holds all music metadata and steps for one song. */
class Song
{
	RString m_sSongDir;
	RString m_pre_customify_song_dir;
public:
	void SetSongDir( const RString sDir ) { m_sSongDir = sDir; }
	RString GetSongDir() { return m_sSongDir; }
	RString GetPreCustomifyDir() { return m_pre_customify_song_dir; }

	/** @brief When should this song be displayed in the music wheel? */
	enum SelectionDisplay
	{
		SHOW_ALWAYS,	/**< always show on the wheel. */
		SHOW_NEVER	/**< never show on the wheel (unless song hiding is turned off). */
	} m_SelectionDisplay;

	Song();
	~Song();
	void Reset();
	void DetachSteps();

	/**
	 * @brief Load a song from the chosen directory.
	 *
	 * This assumes that there is no song present right now.
	 * @param sDir the song directory from which to load. */
	bool LoadFromSongDir(RString sDir, bool load_autosave= false,
		ProfileSlot from_profile= ProfileSlot_Invalid);
	// This one takes the effort to reuse Steps pointers as best as it can
	bool ReloadFromSongDir( RString sDir );
	bool ReloadFromSongDir() { return ReloadFromSongDir(GetSongDir()); }
	void LoadEditsFromSongDir(RString dir);

	bool HasAutosaveFile();
	bool LoadAutosaveFile();

	/**
	 * @brief Call this after loading a song to clean up invalid data.
	 * @param fromCache was this data loaded from the cache file?
	 * @param duringCache was this data loaded during the cache process? */
	void TidyUpData( bool fromCache = false, bool duringCache = false );

	/**
	 * @brief Get the new radar values, and determine the last second at the same time.
	 * This is called by TidyUpData, after saving the Song.
	 * @param fromCache was this data loaded from the cache file?
	 * @param duringCache was this data loaded during the cache process? */
	void ReCalculateRadarValuesAndLastSecond(bool fromCache = false, bool duringCache = false);
	/**
	 * @brief Translate any titles that aren't in english.
	 * This is called by TidyUpData. */
	void TranslateTitles();

	/**
	 * @brief Save to the new SSC file format.
	 * @param sPath the path where we're saving the file.
	 * @param bSavingCache a flag to determine if we're saving cache data.
	 */
	bool SaveToSSCFile(RString sPath, bool bSavingCache, bool autosave= false);
	/** @brief Save to the SSC and SM files no matter what. */
	void Save(bool autosave= false);
	/**
	  * @brief Save the current Song to a JSON file.
	  * @return its success or failure. */
	bool SaveToJsonFile( RString sPath );
	/**
	  * @brief Save the current Song to a cache file using the preferred format.
	  * @return its success or failure. */
	bool SaveToCacheFile();
	/**
	 * @brief Save the current Song to a SM file.
	 * @return its success or failure. */
	bool SaveToSMFile();
	/**
	 * @brief Save the current Song to a DWI file if possible.
	 * @return its success or failure. */
	bool SaveToDWIFile();

	void RemoveAutosave();
	bool WasLoadedFromAutosave() const
	{ return m_loaded_from_autosave; }

	const RString &GetSongFilePath() const;
	RString GetCacheFilePath() const;

	void AddAutoGenNotes();
	/**
	 * @brief Automatically generate steps from one type to another.
	 * @param ntTo the StepsType we're making.
	 * @param ntFrom the StepsType we're generating from.
	 */
	void AutoGen( StepsType ntTo, StepsType ntFrom );
	void RemoveAutoGenNotes();

	// Directory this song data came from:
	const RString &GetSongDir() const { return m_sSongDir; }

	/**
	 * @brief Filename associated with this file.
	 * This will always have a .SSC extension. If we loaded a .SSC, this will
	 * point to it, but if we loaded any other type, this will point to
	 * a generated .SSC filename. */
	RString m_sSongFileName;

	/** @brief The group this Song is in. */
	RString m_sGroupName;
	
	/** @brief The base directory name that this Song is in. */
	RString m_sSongName;

	/**
	 * @brief the Profile this came from.
	 * This is ProfileSlot_Invalid if it wasn't loaded from a profile. */
	ProfileSlot	m_LoadedFromProfile;
	/** @brief Is the song file itself a symlink to another file? */
	bool	m_bIsSymLink;
	bool	m_bEnabled;

	/** @brief The title of the Song. */
	RString	m_sMainTitle;
	/** @brief The subtitle of the Song, if it exists. */
	RString m_sSubTitle;
	/** @brief The artist of the Song, if it exists. */
	RString m_sArtist;
	/** @brief The transliterated title of the Song, if it exists. */
	RString m_sMainTitleTranslit;
	/** @brief The transliterated subtitle of the Song, if it exists. */
	RString m_sSubTitleTranslit;
	/** @brief The transliterated artist of the Song, if it exists. */
	RString m_sArtistTranslit;

	RString m_sFileHash;
	RString GetFileHash();

	/* If PREFSMAN->m_bShowNative is off, these are the same as GetTranslit*
	 * below. Otherwise, they return the main titles. */
	RString GetDisplayMainTitle() const;
	RString GetDisplaySubTitle() const;
	RString GetDisplayArtist() const;
	RString GetMainTitle() const;

	/**
	 * @brief Retrieve the transliterated title, or the main title if there is no translit.
	 * @return the proper title. */
	RString GetTranslitMainTitle() const
	{
		return m_sMainTitleTranslit.size()? m_sMainTitleTranslit: m_sMainTitle;
	}
	/**
	 * @brief Retrieve the transliterated subtitle, or the main subtitle if there is no translit.
	 * @return the proper subtitle. */
	RString GetTranslitSubTitle() const
	{
		return m_sSubTitleTranslit.size()? m_sSubTitleTranslit: m_sSubTitle;
	}
	/**
	 * @brief Retrieve the transliterated artist, or the main artist if there is no translit.
	 * @return the proper artist. */
	RString GetTranslitArtist() const
	{
		return m_sArtistTranslit.size()? m_sArtistTranslit:m_sArtist;
	}

	// "title subtitle"
	RString GetDisplayFullTitle() const;
	RString GetTranslitFullTitle() const;

	/** @brief The version of the song/file. */
	float	m_fVersion;
	/** @brief The genre of the song/file. */
	RString m_sGenre;

	/**
	 * @brief The person who worked with the song file who should be credited.
	 * This is read and saved, but never actually used. */
	RString	m_sCredit;

	RString m_sOrigin; // song origin (for .ssc format)

	RString	m_sMusicFile;
	RString m_PreviewFile;
	RString	m_sInstrumentTrackFile[NUM_InstrumentTrack];

	/** @brief The length of the music file. */
	float	m_fMusicLengthSeconds;
	float	m_fMusicSampleStartSeconds;
	float	m_fMusicSampleLengthSeconds;
	DisplayBPM m_DisplayBPMType;
	float	m_fSpecifiedBPMMin;
	float	m_fSpecifiedBPMMax;	// if a range, then Min != Max

	RString m_sBannerFile;		// typically a 16:5 ratio graphic (e.g. 256x80)
	RString m_sJacketFile;	// typically square (e.g. 192x192, 256x256)
	RString m_sCDFile;		// square (e.g. 128x128 [DDR 1st-3rd])
	RString m_sDiscFile;		// rectangular (e.g. 256x192 [Pump], 200x150 [MGD3])
	/** @brief The location of the lyrics file, if it exists. */
	RString m_sLyricsFile;
	RString m_sBackgroundFile;
	RString m_sCDTitleFile;
	RString m_sPreviewVidFile;
	std::vector<RString> ImageDir;

	AttackArray m_Attacks;
	std::vector<RString>	m_sAttackString;

	static RString GetSongAssetPath( RString sPath, const RString &sSongPath );
	RString GetMusicPath() const;
	RString GetInstrumentTrackPath( InstrumentTrack it ) const;
	RString GetBannerPath() const;
	RString GetJacketPath() const;
	RString GetCDImagePath() const;
	RString GetDiscPath() const;
	RString	GetLyricsPath() const;
	RString GetBackgroundPath() const;
	RString GetCDTitlePath() const;
	RString GetPreviewVidPath() const;
	RString GetPreviewMusicPath() const;
	float GetPreviewStartSeconds() const;

	RString GetCacheFile( RString sPath );

	// For loading only:
	bool m_bHasMusic, m_bHasBanner, m_bHasBackground;

	bool HasMusic() const;
	bool HasInstrumentTrack( InstrumentTrack it ) const;
	/**
	 * @brief Does this song have a banner?
	 * @return true if it does, false otherwise. */
	bool HasBanner() const;
	/**
	 * @brief Does this song have a background image?
	 * @return true if it does, false otherwise. */
	bool HasBackground() const;
	bool HasJacket() const;
	bool HasCDImage() const;
	bool HasDisc() const;
	bool HasCDTitle() const;
	//bool HasMovieBackground() const;
	bool HasBGChanges() const;
	bool HasLyrics() const;
	bool HasAttacks() const;
	bool HasPreviewVid() const;

	bool Matches(RString sGroup, RString sSong) const;

	/** @brief The Song's TimingData. */
	TimingData m_SongTiming;

	float GetFirstBeat() const;
	float GetFirstSecond() const;
	float GetLastBeat() const;
	float GetLastSecond() const;
	float GetSpecifiedLastBeat() const;
	float GetSpecifiedLastSecond() const;

	void SetFirstSecond(const float f);
	void SetLastSecond(const float f);
	void SetSpecifiedLastSecond(const float f);

	typedef std::vector<BackgroundChange> 	VBackgroundChange;
private:
	/** @brief The first second that a note is hit. */
	float firstSecond;
	/** @brief The last second that a note is hit. */
	float lastSecond;
	/** @brief The last second of the song for playing purposes. */
	float specifiedLastSecond;
	/**
	 * @brief The background changes (sorted by layer) that are for this Song.
	 * This uses an AutoPtr instead of a raw pointer so that the
	 * auto gen'd copy constructor works correctly.
	 * This must be sorted before gameplay. */
	AutoPtrCopyOnWrite<VBackgroundChange>	m_BackgroundChanges[NUM_BackgroundLayer];
	/**
	 * @brief The foreground changes that are for this Song.
	 * This uses an AutoPtr instead of a raw pointer so that the
	 * auto gen'd copy constructor works correctly.
	 * This must be sorted before gameplay. */
	AutoPtrCopyOnWrite<VBackgroundChange>	m_ForegroundChanges;

	std::vector<RString> GetChangesToVectorString(const std::vector<BackgroundChange> & changes) const;
public:
	const std::vector<BackgroundChange>	&GetBackgroundChanges( BackgroundLayer bl ) const;
	std::vector<BackgroundChange>	&GetBackgroundChanges( BackgroundLayer bl );
	const std::vector<BackgroundChange>	&GetForegroundChanges() const;
	std::vector<BackgroundChange>	&GetForegroundChanges();

	std::vector<RString> GetBGChanges1ToVectorString() const;
	std::vector<RString> GetBGChanges2ToVectorString() const;
	std::vector<RString> GetFGChanges1ToVectorString() const;

	std::vector<RString> GetInstrumentTracksToVectorString() const;

	/**
	 * @brief The list of LyricSegments.
	 * This must be sorted before gameplay. */
	std::vector<LyricSegment>			m_LyricSegments;

/* [splittiming]
	void AddBPMSegment( const BPMSegment &seg ) { m_Timing.AddBPMSegment( seg ); }
	void AddStopSegment( const StopSegment &seg ) { m_Timing.AddStopSegment( seg ); }
	void AddWarpSegment( const WarpSegment &seg ) { m_Timing.AddWarpSegment( seg ); }
*/
	void AddBackgroundChange( BackgroundLayer blLayer, BackgroundChange seg );
	void AddForegroundChange( BackgroundChange seg );
	void AddLyricSegment( LyricSegment seg );

	void GetDisplayBpms( DisplayBpms &AddTo ) const;
	const BackgroundChange &GetBackgroundAtBeat( BackgroundLayer iLayer, float fBeat ) const;

/* [splittiming]
	float GetBPMAtBeat( float fBeat ) const { return m_Timing.GetBPMAtBeat( fBeat ); }
	void SetBPMAtBeat( float fBeat, float fBPM ) { m_Timing.SetBPMAtBeat( fBeat, fBPM ); }
	BPMSegment& GetBPMSegmentAtBeat( float fBeat ) { return m_Timing.GetBPMSegmentAtBeat( fBeat ); }
*/

	Steps *CreateSteps();
	void InitSteps(Steps *pSteps);

	/* [splittiming]
	float SongGetBeatFromElapsedTime( float fElapsedTime ) const
	{
		return m_SongTiming.GetBeatFromElapsedTime( fElapsedTime );
	}
	float StepsGetBeatFromElapsedTime( float fElapsedTime, const Steps &steps ) const
	{
		return steps.m_Timing.GetBeatFromElapsedTime( fElapsedTime );
	}

	float SongGetElapsedTimeFromBeat( float fBeat ) const
	{
		return m_SongTiming.GetElapsedTimeFromBeat( fBeat );
	}
	float StepsGetElapsedTimeFromBeat( float fBeat, const Steps &steps ) const
	{
		return steps.m_Timing.GetElapsedTimeFromBeat( fBeat );
	}
	*/

	/* [splittiming]
	float GetBeatFromElapsedTime( float fElapsedTime ) const
	{
		return m_Timing.GetBeatFromElapsedTime( fElapsedTime );
	}
	float GetElapsedTimeFromBeat( float fBeat ) const { return m_Timing.GetElapsedTimeFromBeat( fBeat ); }
	*/

	bool HasSignificantBpmChangesOrStops() const;
	float GetStepsSeconds() const;
	bool IsLong() const;
	bool IsMarathon() const;

	bool SongCompleteForStyle( const Style *st ) const;
	bool HasStepsType( StepsType st ) const;
	bool HasStepsTypeAndDifficulty( StepsType st, Difficulty dc ) const;
	// TODO: Allow for a non const version.
	const std::vector<Steps*>& GetAllSteps() const { return m_vpSteps; }
	const std::vector<Steps*>& GetStepsByStepsType( StepsType st ) const { return m_vpStepsByType[st]; }
	bool IsEasy( StepsType st ) const;
	bool IsTutorial() const;
	bool HasEdits( StepsType st ) const;

	void SetEnabled( bool b ) { m_bEnabled = b; }
	bool GetEnabled() const { return m_bEnabled; }
	/**
	 * @brief Determine if the song should be shown on the MusicWheel normally.
	 * Songs that are not displayed normally may still be available during
	 * random selection, extra stages, or other special conditions.
	 * @return true if displayed normally, false otherwise. */
	bool NormallyDisplayed() const;
	bool ShowInDemonstrationAndRanking() const;

	/**
	 * @brief Add the chosen Steps to the Song.
	 * We are responsible for deleting the memory pointed to by pSteps!
	 * @param pSteps the new steps. */
	void AddSteps( Steps* pSteps );
	void DeleteSteps( const Steps* pSteps, bool bReAutoGen = true );

	void FreeAllLoadedFromProfile( ProfileSlot slot = ProfileSlot_Invalid, const std::set<Steps*> *setInUse = nullptr );
	bool WasLoadedFromProfile() const { return m_LoadedFromProfile != ProfileSlot_Invalid; }
	void GetStepsLoadedFromProfile( ProfileSlot slot, std::vector<Steps*> &vpStepsOut ) const;
	int GetNumStepsLoadedFromProfile( ProfileSlot slot ) const;
	bool IsEditAlreadyLoaded( Steps* pSteps ) const;

	bool IsStepsUsingDifferentTiming(Steps *pSteps ) const;
	bool AnyChartUsesSplitTiming() const;

	/**
	 * @brief An array of keysound file names (e.g. "beep.wav").
	 * The index in this array corresponds to the index in TapNote.
	 * If you  change the index in here, you must change all NoteData too.
	 * Any note that doesn't have a value in the range of this array
	 * means "this note doesn't have a keysound". */
	std::vector<RString> m_vsKeysoundFile;

	RString GetAttackString() const
	{
		return join(":", this->m_sAttackString);
	}

	// Lua
	void PushSelf( lua_State *L );

private:
	bool m_loaded_from_autosave;
	/** @brief the Steps that belong to this Song. */
	std::vector<Steps*> m_vpSteps;
	/** @brief the Steps of a particular StepsType that belong to this Song. */
	std::array<std::vector<Steps *>, NUM_StepsType> m_vpStepsByType;
	/** @brief the Steps that are of unrecognized Styles. */
	std::vector<Steps*> m_UnknownStyleSteps;
};

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
