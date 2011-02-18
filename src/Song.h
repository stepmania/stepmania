/** @brief Song - Holds all music metadata and steps for one song. */

#ifndef SONG_H
#define SONG_H

#include "Attack.h"
#include "TimingData.h"
#include "Difficulty.h"
#include "EnumHelper.h"
#include "RageUtil_AutoPtr.h"
#include "RageUtil_CachedObject.h"
#include "RageTypes.h"
#include <set>

class Steps;
class Style;
class StepsID;
struct lua_State;
struct BackgroundChange;

const int MAX_EDITS_PER_SONG_PER_PROFILE	= 5;
const int MAX_EDITS_PER_SONG			= 5*NUM_ProfileSlot;

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

struct LyricSegment
{
	float	m_fStartTime;
	RString m_sLyric;
	RageColor m_Color;
};

class Song
{
	RString m_sSongDir;
public:
	void SetSongDir( const RString sDir ) { m_sSongDir = sDir; }

	// Set when this song should be displayed in the music wheel:
	enum SelectionDisplay
	{ 
		SHOW_ALWAYS,	// always
		SHOW_NEVER	// never (unless song hiding is turned off)
	} m_SelectionDisplay;

	Song();
	~Song();
	void Reset();
	void DetachSteps();

	// This one assumes the song is currently empty
	bool LoadFromSongDir( RString sDir );
	// This one takes the effort to reuse Steps pointers as best as it can
	bool ReloadFromSongDir( RString sDir );

	void TidyUpData();	// call after loading to clean up invalid data
	void ReCalculateRadarValuesAndLastBeat();	// called by TidyUpData, and after saving
	void TranslateTitles();	// called by TidyUpData

	bool SaveToSSCFile( RString sPath, bool bSavingCache );
	void Save();	// saves SSC and SM guaranteed.
	bool SaveToCacheFile();
	bool SaveToSMFile();
	bool SaveToDWIFile();

	const RString &GetSongFilePath() const;
	RString GetCacheFilePath() const;

	void AddAutoGenNotes();
	void AutoGen( StepsType ntTo, StepsType ntFrom );	// create Steps of type ntTo from Steps of type ntFrom
	void RemoveAutoGenNotes();

	// Directory this song data came from:
	const RString &GetSongDir() const { return m_sSongDir; }

	/* Filename associated with this file. This will always have a .SSC
	 * extension. If we loaded a .SSC, this will point to it, but if we loaded
	 * any other type, this will point to a generated .SSC filename. */
	RString m_sSongFileName;

	RString m_sGroupName;

	ProfileSlot	m_LoadedFromProfile;	// ProfileSlot_Invalid if not loaded from a profile
	bool	m_bIsSymLink;
	bool	m_bEnabled;

	RString	m_sMainTitle, m_sSubTitle, m_sArtist;
	RString m_sMainTitleTranslit, m_sSubTitleTranslit, m_sArtistTranslit;

	/* If PREFSMAN->m_bShowNative is off, these are the same as GetTranslit*
	 * below. Otherwise, they return the main titles. */
	RString GetDisplayMainTitle() const;
	RString GetDisplaySubTitle() const;
	RString GetDisplayArtist() const;

	// Returns the transliterated titles, if any; otherwise returns the main titles.
	RString GetTranslitMainTitle() const { return m_sMainTitleTranslit.size()? m_sMainTitleTranslit: m_sMainTitle; }
	RString GetTranslitSubTitle() const { return m_sSubTitleTranslit.size()? m_sSubTitleTranslit: m_sSubTitle; }
	RString GetTranslitArtist() const { return m_sArtistTranslit.size()? m_sArtistTranslit:m_sArtist; }

	// "title subtitle"
	RString GetDisplayFullTitle() const;
	RString GetTranslitFullTitle() const;

	// allow versioning with the song.
	float	m_fVersion;

	RString m_sGenre;

	// This is read and saved, but never actually used.
	RString	m_sCredit;

	RString	m_sMusicFile;
	RString	m_sInstrumentTrackFile[NUM_InstrumentTrack];

	float	m_fMusicLengthSeconds;
	float	m_fFirstBeat;	// beat of first note
	float	m_fLastBeat;	// beat of last note
	float	m_fSpecifiedLastBeat;	// specified last beat of the song
	float	m_fMusicSampleStartSeconds;
	float	m_fMusicSampleLengthSeconds;
	enum { DISPLAY_ACTUAL, DISPLAY_SPECIFIED, DISPLAY_RANDOM } m_DisplayBPMType;
	float	m_fSpecifiedBPMMin;
	float	m_fSpecifiedBPMMax;	// if a range, then Min != Max

	RString m_sBannerFile;		// typically a 16:5 ratio graphic (e.g. 256x80)
	//RString m_sJacketFile;	// typically square (e.g. 192x192, 256x256)
	//RString m_sCDFile;		// square (e.g. 128x128 [DDR 1st-3rd])
	//RString m_sDiscFile;		// rectangular (e.g. 256x192 [Pump], 200x150 [MGD3])
	RString m_sLyricsFile;
	RString m_sBackgroundFile;
	RString m_sCDTitleFile;

	AttackArray m_Attacks;
	vector<RString>	m_sAttackString;

	RString GetMusicPath() const;
	RString GetInstrumentTrackPath( InstrumentTrack it ) const;
	RString GetBannerPath() const;
	//RString GetJacketPath() const;
	//RString GetCDImagePath() const;
	//RString GetDiscPath() const;
	RString	GetLyricsPath() const;
	RString GetBackgroundPath() const;
	RString GetCDTitlePath() const;

	// For loading only:
	bool m_bHasMusic, m_bHasBanner, m_bHasBackground;

	bool HasMusic() const;
	bool HasInstrumentTrack( InstrumentTrack it ) const;
	bool HasBanner() const;
	bool HasBackground() const;
	//bool HasJacket() const;
	//bool HasCDImage() const;
	//bool HasDisc() const;
	bool HasCDTitle() const;
	bool HasMovieBackground() const;
	bool HasBGChanges() const;
	bool HasLyrics() const;
	bool HasAttacks() const;

	bool Matches(RString sGroup, RString sSong) const;

	TimingData m_Timing;

	typedef vector<BackgroundChange> 	VBackgroundChange;
private:
	// AutoPtr instead of raw pointer so that the auto gen'd copy constructor works correctly.
	AutoPtrCopyOnWrite<VBackgroundChange>	m_BackgroundChanges[NUM_BackgroundLayer];	// these must be sorted before gameplay
	AutoPtrCopyOnWrite<VBackgroundChange>	m_ForegroundChanges;	// this must be sorted before gameplay
public:
	const vector<BackgroundChange>	&GetBackgroundChanges( BackgroundLayer bl ) const;
	vector<BackgroundChange>	&GetBackgroundChanges( BackgroundLayer bl );
	const vector<BackgroundChange>	&GetForegroundChanges() const;
	vector<BackgroundChange>	&GetForegroundChanges();

	vector<LyricSegment>			m_LyricSegments;	// this must be sorted before gameplay

	void AddBPMSegment( const BPMSegment &seg ) { m_Timing.AddBPMSegment( seg ); }
	void AddStopSegment( const StopSegment &seg ) { m_Timing.AddStopSegment( seg ); }
	//void AddWarpSegment( const WarpSegment &seg ) { m_Timing.AddWarpSegment( seg ); }
	void AddBackgroundChange( BackgroundLayer blLayer, BackgroundChange seg );
	void AddForegroundChange( BackgroundChange seg );
	void AddLyricSegment( LyricSegment seg );

	void GetDisplayBpms( DisplayBpms &AddTo ) const;
	const BackgroundChange &GetBackgroundAtBeat( BackgroundLayer iLayer, float fBeat ) const;

	float GetBPMAtBeat( float fBeat ) const { return m_Timing.GetBPMAtBeat( fBeat ); }
	void SetBPMAtBeat( float fBeat, float fBPM ) { m_Timing.SetBPMAtBeat( fBeat, fBPM ); }
	BPMSegment& GetBPMSegmentAtBeat( float fBeat ) { return m_Timing.GetBPMSegmentAtBeat( fBeat ); }
	float GetBeatFromElapsedTime( float fElapsedTime ) const { return m_Timing.GetBeatFromElapsedTime( fElapsedTime ); }
	float GetElapsedTimeFromBeat( float fBeat ) const { return m_Timing.GetElapsedTimeFromBeat( fBeat ); }
	bool HasSignificantBpmChangesOrStops() const;
	float GetStepsSeconds() const;
	bool IsLong() const;
	bool IsMarathon() const;

	bool SongCompleteForStyle( const Style *st ) const;
	bool HasStepsType( StepsType st ) const;
	bool HasStepsTypeAndDifficulty( StepsType st, Difficulty dc ) const;
	const vector<Steps*>& GetAllSteps() const { return m_vpSteps; }
	const vector<Steps*>& GetStepsByStepsType( StepsType st ) const { return m_vpStepsByType[st]; }
	bool IsEasy( StepsType st ) const;
	bool IsTutorial() const;
	bool HasEdits( StepsType st ) const;

	void SetEnabled( bool b ) { m_bEnabled = b; }
	bool GetEnabled() const { return m_bEnabled; }
	bool NormallyDisplayed() const;
	bool ShowInDemonstrationAndRanking() const;

	void AddSteps( Steps* pSteps ); // we are responsible for deleting the memory pointed to by pSteps!
	void DeleteSteps( const Steps* pSteps, bool bReAutoGen = true );

	void FreeAllLoadedFromProfile( ProfileSlot slot = ProfileSlot_Invalid, const set<Steps*> *setInUse = NULL );
	bool WasLoadedFromProfile() const { return m_LoadedFromProfile != ProfileSlot_Invalid; }
	void GetStepsLoadedFromProfile( ProfileSlot slot, vector<Steps*> &vpStepsOut ) const;
	int GetNumStepsLoadedFromProfile( ProfileSlot slot ) const;
	bool IsEditAlreadyLoaded( Steps* pSteps ) const;

	// An array of keysound file names (e.g. "beep.wav").
	// The index in this array corresponds to the index in TapNote.  If you 
	// change the index in here, you must change all NoteData too.
	// Any note that doesn't have a value in the range of this array
	// means "this note doesn't have a keysound".
	vector<RString> m_vsKeysoundFile;

	CachedObject<Song> m_CachedObject;

	// Lua
	void PushSelf( lua_State *L );

private:
	vector<Steps*> m_vpSteps;
	vector<Steps*> m_vpStepsByType[NUM_StepsType];
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
