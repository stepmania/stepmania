/* Song - Holds all music metadata and steps for one song. */

#ifndef SONG_H
#define SONG_H

#include "TimingData.h"
#include "Difficulty.h"
#include "EnumHelper.h"
#include "RageUtil_AutoPtr.h"
#include "RageTypes.h"

class Steps;
class Style;
class NotesLoader;
class LyricsLoader;
class Profile;
class StepsID;
struct lua_State;
struct BackgroundChange;

const int MAX_EDITS_PER_SONG_PER_PROFILE	= 5;
const int MAX_EDITS_PER_SONG			= 5*NUM_ProfileSlot;

extern const int FILE_CACHE_VERSION;

enum BackgroundLayer
{
	BACKGROUND_LAYER_1,
	BACKGROUND_LAYER_2,
	NUM_BackgroundLayer,
	BACKGROUND_LAYER_INVALID
};
#define FOREACH_BackgroundLayer( bl ) FOREACH_ENUM( BackgroundLayer, NUM_BackgroundLayer, bl )

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

	/* Set when this song should be displayed in the music wheel: */
	enum SelectionDisplay
	{ 
		SHOW_ALWAYS,	/* all the time */
		SHOW_ROULETTE,	/* only when rouletting */
		SHOW_NEVER		/* never (unless song hiding is turned off) */
	} m_SelectionDisplay;

	Song();
	~Song();
	void Reset();
	void DetachSteps();

	bool LoadFromSongDir( RString sDir );

	void TidyUpData();	// call after loading to clean up invalid data
	void ReCalculateRadarValuesAndLastBeat();	// called by TidyUpData, and after saving
	void TranslateTitles();	// called by TidyUpData

	void SaveToSMFile( RString sPath, bool bSavingCache );
	void Save();	// saves SM and DWI
	void SaveToCacheFile();
	void SaveToDWIFile();

	const RString &GetSongFilePath() const;
	RString GetCacheFilePath() const;

	void AddAutoGenNotes();
	void AutoGen( StepsType ntTo, StepsType ntFrom );	// create Steps of type ntTo from Steps of type ntFrom
	void RemoveAutoGenNotes();

	/* Directory this song data came from: */
	const RString &GetSongDir() const { return m_sSongDir; }

	/* Filename associated with this file.  This will always have
	 * an .SM extension.  If we loaded an .SM, this will point to 
	 * it, but if we loaded any other type, this will point to a
	 * generated .SM filename. */
	RString m_sSongFileName;

	RString m_sGroupName;

	ProfileSlot	m_LoadedFromProfile;	// ProfileSlot_INVALID if wasn't loaded from a profile
	bool	m_bIsSymLink;

	RString	m_sMainTitle, m_sSubTitle, m_sArtist;
	RString m_sMainTitleTranslit, m_sSubTitleTranslit, m_sArtistTranslit;

	/* If PREFSMAN->m_bShowNative is off, these are the same as GetTranslit* below.
	 * Otherwise, they return the main titles. */
	RString GetDisplayMainTitle() const;
	RString GetDisplaySubTitle() const;
	RString GetDisplayArtist() const;

	/* Returns the transliterated titles, if any; otherwise returns the main titles. */
	RString GetTranslitMainTitle() const { return m_sMainTitleTranslit.size()? m_sMainTitleTranslit: m_sMainTitle; }
	RString GetTranslitSubTitle() const { return m_sSubTitleTranslit.size()? m_sSubTitleTranslit: m_sSubTitle; }
	RString GetTranslitArtist() const { return m_sArtistTranslit.size()? m_sArtistTranslit:m_sArtist; }

	/* "title subtitle" */
	RString GetDisplayFullTitle() const;
	RString GetTranslitFullTitle() const;

	RString m_sGenre;

	/* This is read and saved, but never actually used. */
	RString	m_sCredit;

	RString	m_sMusicFile;
	float	m_fMusicLengthSeconds;
	float	m_fFirstBeat;	// beat of first note
	float	m_fLastBeat;	// beat of last note
	float	m_fSpecifiedLastBeat;	// specified last beat of the song
	float	m_fMusicSampleStartSeconds;
	float	m_fMusicSampleLengthSeconds;
	enum { DISPLAY_ACTUAL, DISPLAY_SPECIFIED, DISPLAY_RANDOM } m_DisplayBPMType;
	float	m_fSpecifiedBPMMin;
	float	m_fSpecifiedBPMMax;	// if a range, then Min != Max

	RString	m_sBannerFile;
	RString m_sLyricsFile;
	RString	m_sBackgroundFile;
	RString	m_sCDTitleFile;

	RString GetMusicPath() const;
	RString GetBannerPath() const;
	RString	GetLyricsPath() const;
	RString GetBackgroundPath() const;
	RString GetCDTitlePath() const;

	/* For loading only: */
	bool m_bHasMusic, m_bHasBanner;

	bool HasMusic() const;
	bool HasBanner() const;
	bool HasBackground() const;
	bool HasCDTitle() const;
	bool HasMovieBackground() const;
	bool HasBGChanges() const;
	bool HasLyrics() const;

	bool Matches(RString sGroup, RString sSong) const;

	TimingData				m_Timing;

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
	SelectionDisplay GetDisplayed() const;
	bool NormallyDisplayed() const;
	bool NeverDisplayed() const;
	bool RouletteDisplayed() const;
	bool ShowInDemonstrationAndRanking() const;

	void AddSteps( Steps* pSteps );		// we are responsible for deleting the memory pointed to by pSteps!
	void DeleteSteps( const Steps* pSteps, bool bReAutoGen = true );

	void FreeAllLoadedFromProfile( ProfileSlot slot = ProfileSlot_INVALID );
	bool WasLoadedFromProfile() const { return m_LoadedFromProfile != ProfileSlot_INVALID; }
	void GetStepsLoadedFromProfile( ProfileSlot slot, vector<Steps*> &vpStepsOut ) const;
	int GetNumStepsLoadedFromProfile( ProfileSlot slot ) const;
	bool IsEditAlreadyLoaded( Steps* pSteps ) const;

	// An array of keysound file names (e.g. "beep.wav").
	// The index in this array corresponds to the index in TapNote.  If you 
	// change the index in here, you must change all NoteData too.
	// Any note that doesn't have a value in the range of this array
	// means "this note doens't have a keysound".
	vector<RString> m_vsKeysoundFile;

	// Lua
	void PushSelf( lua_State *L );

private:

	vector<Steps*> m_vpSteps;
	vector<Steps*> m_vpStepsByType[NUM_StepsType];
};

#endif

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
