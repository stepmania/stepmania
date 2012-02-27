#ifndef STEPS_H
#define STEPS_H

#include "Attack.h"
#include "GameConstantsAndTypes.h"
#include "PlayerNumber.h"
#include "Grade.h"
#include "RadarValues.h"
#include "Difficulty.h"
#include "RageUtil_AutoPtr.h"
#include "RageUtil_CachedObject.h"
#include "TimingData.h"

class Profile;
class NoteData;
struct lua_State;

/** 
 * @brief Enforce a limit on the number of chars for the description.
 *
 * In In The Groove, this limit was 12: we do not need such a limit now.
 */
const int MAX_STEPS_DESCRIPTION_LENGTH = 255;

/** @brief The different ways of displaying the BPM. */
enum DisplayBPM
{
	DISPLAY_BPM_ACTUAL, /**< Display the song's actual BPM. */
	DISPLAY_BPM_SPECIFIED, /**< Display a specified value or values. */
	DISPLAY_BPM_RANDOM, /**< Display a random selection of BPMs. */
	NUM_DisplayBPM,
	DisplayBPM_Invalid
};
const RString& DisplayBPMToString( DisplayBPM dbpm );
LuaDeclareType( DisplayBPM );

/** 
 * @brief Holds note information for a Song.
 *
 * A Song may have one or more Notes. */
class Steps
{
public:
	/** @brief Set up the Steps with initial values. */
	Steps();
	/** @brief Destroy the Steps that are no longer needed. */
	~Steps();

	// initializers
	void AutogenFrom( const Steps *parent, StepsType ntTo );
	void CopyFrom( Steps* pSource, StepsType ntTo, float fMusicLengthSeconds );
	void CreateBlank( StepsType ntTo );

	void Compress() const;
	void Decompress() const;
	void Decompress();
	/** 
	 * @brief Determine if these steps were created by the autogenerator.
	 * @return true if they were, false otherwise.
	 */
	bool IsAutogen() const				{ return parent != NULL; }

	/**
	 * @brief Determine if this set of Steps is an edit.
	 *
	 * Edits have a special value of difficulty to make it easy to determine.
	 * @return true if this is an edit, false otherwise.
	 */
	bool IsAnEdit() const				{ return m_Difficulty == Difficulty_Edit; }
	/**
	 * @brief Determine if this set of Steps is a player edit.
	 *
	 * Player edits also have to be loaded from a player's profile slot, not the machine.
	 * @return true if this is a player edit, false otherwise. */
	bool IsAPlayerEdit() const			{ return IsAnEdit() && GetLoadedFromProfileSlot() < ProfileSlot_Machine; }
	/**
	 * @brief Determine if these steps were loaded from a player's profile.
	 * @return true if they were from a player profile, false otherwise.
	 */
	bool WasLoadedFromProfile() const		{ return m_LoadedFromProfile != ProfileSlot_Invalid; }
	ProfileSlot GetLoadedFromProfileSlot() const	{ return m_LoadedFromProfile; }
	/**
	 * @brief Retrieve the description used for this edit.
	 * @return the description used for this edit.
	 */
	RString GetDescription() const			{ return Real()->m_sDescription; }
	/**
	 * @brief Retrieve the ChartStyle used for this chart.
	 * @return the description used for this chart.
	 */
	RString GetChartStyle() const			{ return Real()->m_sChartStyle; }
	/**
	 * @brief Retrieve the difficulty used for this edit.
	 * @return the difficulty used for this edit.
	 */
	Difficulty GetDifficulty() const		{ return Real()->m_Difficulty; }
	/**
	 * @brief Retrieve the meter used for this edit.
	 * @return the meter used for this edit.
	 */
	int GetMeter() const				{ return Real()->m_iMeter; }
	const RadarValues& GetRadarValues( PlayerNumber pn ) const { return Real()->m_CachedRadarValues[pn]; }
	/**
	 * @brief Retrieve the author credit used for this edit.
	 * @return the author credit used for this edit.
	 */
	RString GetCredit() const			{ return Real()->m_sCredit; }

	/** @brief The list of attacks. */
	AttackArray m_Attacks;
	/** @brief The stringified list of attacks. */
	vector<RString> m_sAttackString;

	RString GetChartName() const			{ return parent ? Real()->GetChartName() : this->chartName; }
	void SetChartName(const RString name)	{ this->chartName = name; }
	void SetFilename( RString fn )			{ m_sFilename = fn; }
	RString GetFilename() const			{ return m_sFilename; }
	void SetSavedToDisk( bool b )			{ DeAutogen(); m_bSavedToDisk = b; }
	bool GetSavedToDisk() const			{ return Real()->m_bSavedToDisk; }
	void SetDifficulty( Difficulty dc )		{ SetDifficultyAndDescription( dc, GetDescription() ); }
	void SetDescription( RString sDescription ) 	{ SetDifficultyAndDescription( this->GetDifficulty(), sDescription ); }
	void SetDifficultyAndDescription( Difficulty dc, RString sDescription );
	void SetCredit( RString sCredit );
	void SetChartStyle( RString sChartStyle );
	static bool MakeValidEditDescription( RString &sPreferredDescription );	// return true if was modified

	void SetLoadedFromProfile( ProfileSlot slot )	{ m_LoadedFromProfile = slot; }
	void SetMeter( int meter );
	void SetCachedRadarValues( const RadarValues v[NUM_PLAYERS] );
	float PredictMeter() const;

	unsigned GetHash() const;
	void GetNoteData( NoteData& noteDataOut ) const;
	NoteData GetNoteData() const;
	void SetNoteData( const NoteData& noteDataNew );
	void SetSMNoteData( const RString &notes_comp );
	void GetSMNoteData( RString &notes_comp_out ) const;

	/**
	 * @brief Retrieve the NoteData from the original source.
	 * @return true if successful, false for failure. */
	bool GetNoteDataFromSimfile();

	/**
	 * @brief Determine if we are missing any note data.
	 *
	 * This takes advantage of the fact that we usually compress our data.
	 * @return true if our notedata is empty, false otherwise. */
	bool IsNoteDataEmpty() const;

	void TidyUpData();
	void CalculateRadarValues( float fMusicLengthSeconds );

	/** 
	 * @brief The TimingData used by the Steps.
	 *
	 * This is required to allow Split Timing. */
	TimingData m_Timing;

	/**
	 * @brief Determine if the Steps have any major timing changes during gameplay.
	 * @return true if it does, or false otherwise. */
	bool HasSignificantTimingChanges() const;

	/**
	 * @brief Determine if the Steps have any attacks.
	 * @return true if it does, or false otherwise. */
	bool HasAttacks() const;

	// Lua
	void PushSelf( lua_State *L );

	StepsType			m_StepsType;

	CachedObject<Steps> m_CachedObject;

	/**
	 * @brief Determine if the Steps use Split Timing by comparing the Song it's in.
	 * @return true if the Step and Song use different timings, false otherwise. */
	bool UsesSplitTiming() const;

	void SetDisplayBPM(const DisplayBPM type)	{ this->displayBPMType = type; }
	DisplayBPM GetDisplayBPM() const			{ return this->displayBPMType; }
	void SetMinBPM(const float f)				{ this->specifiedBPMMin = f; }
	float GetMinBPM() const					{ return this->specifiedBPMMin; }
	void SetMaxBPM(const float f)				{ this->specifiedBPMMax = f; }
	float GetMaxBPM() const					{ return this->specifiedBPMMax; }
	void GetDisplayBpms( DisplayBpms &addTo) const;

	RString GetAttackString() const
	{
		return join(":", this->m_sAttackString);
	}

	/**
	 * @brief Quickly get the Steps' StepsType.
	 * @return the StepsType. */
	StepsType GetStepsType() const;

	/**
	 * @brief Quickly get the category for this Steps' style.
	 * @return the appropriate StepsTypeCategory. */
	StepsTypeCategory GetStepsTypeCategory() const;

	/**
	 * @brief Determine whether this Steps' style requires more than one player.
	 * @return true if more than one player is needed for this style, false otherwise. */
	bool IsMultiPlayerStyle() const;

	/**
	 * @brief Determine whether this note is to be a Tap
	 * given the Steps' timing data.
	 * @param tn the TapNote to check.
	 * @param row the row to check.
	 * @return true if this is to be a Tap, false otherwise. */
	bool IsTap(const TapNote &tn, const int row) const;
	/**
	 * @brief Determine whether this note is to be a Mine
	 * given the Steps' timing data.
	 * @param tn the TapNote to check.
	 * @param row the row to check.
	 * @return true if this is to be a Mine, false otherwise. */
	bool IsMine(const TapNote &tn, const int row) const;
	/**
	 * @brief Determine whether this note is to be a Lift
	 * given the Steps' timing data.
	 * @param tn the TapNote to check.
	 * @param row the row to check.
	 * @return true if this is to be a Lift, false otherwise. */
	bool IsLift(const TapNote &tn, const int row) const;
	/**
	 * @brief Determine whether this note is to be a Fake
	 * given the Steps' timing data.
	 * @param tn the TapNote to check.
	 * @param row the row to check.
	 * @return true if this is to be a Fake, false otherwise. */
	bool IsFake(const TapNote &tn, const int row) const;

	/* Many of the functions below are designed to work with any arbitrary number of players.
	 * While we only support two players at once right now for normal gameplay purposes,
	 * it helps to think ahead for future versions of StepMania.
	 *
	 * If there are any situations where we don't have to worry about multiple players or
	 * crazy timing data, we defer to the NoteData calls. They are generally faster. */

	vector<int> GetNumRowsWithTap(int start = 0, int end = MAX_NOTE_ROW) const; // may keep int
	vector<int> GetNumRowsWithTapOrHoldHead(int start = 0, int end = MAX_NOTE_ROW) const; // may keep int
	/** @brief Determine if a Player's row needs a certain number of presses.
	 * @param min the mininum number to watch for.
	 * @param row the row to check for.
	 * @return true if the player needs that many presses at minimum, false otherwise. */
	vector<bool> RowNeedsAtLeastSimultaneousPresses(int min, int row) const;
	/** @brief Get the number of rows that require a minimum number of presses for all players.
	 *
	 * Note that presses are NOT the same as taps. Presses means others can be held, as long as
	 * there is at least one tap involved.
	 * @param min the minimum number to watch for.
	 * @param start the starting row.
	 * @param end the ending row.
	 * @return the number of rows each player must press said number of times. */
	vector<int> GetNumRowsWithSimultaneousPresses(int min, int start = 0, int end = MAX_NOTE_ROW) const;
	/** @brief Get the number of rows that require a minimum number of taps for all players.
	 *
	 * Note that presses are NOT the same as taps. Taps means at the same time.
	 * @param min the minimum number to watch for.
	 * @param start the starting row.
	 * @param end the ending row.
	 * @return the number of rows each player must tap said number of times. */
	vector<int> GetNumRowsWithSimultaneousTaps(int min, int start = 0, int end = MAX_NOTE_ROW) const;
	/** @brief Get the number of tap notes when comparing TimingData for all players.
	 * @param start the starting row.
	 * @param end the ending row.
	 * @return the number of tap notes for each player. */
	vector<int> GetNumTapNotes(int start = 0, int end = MAX_NOTE_ROW) const;

	vector<int> GetNumTapNotesInRow(int row) const; // may keep int
	
	/** @brief Get the number of jumps when comparing TimingData for all players.
	 * @param start the starting row.
	 * @param end the ending row.
	 * @return the number of jumps for each player. */
	vector<int> GetNumJumps(int start = 0, int end = MAX_NOTE_ROW) const;
	/** @brief Get the number of hands when comparing TimingData for all players.
	 * @param start the starting row.
	 * @param end the ending row.
	 * @return the number of hands for each player. */
	vector<int> GetNumHands(int start = 0, int end = MAX_NOTE_ROW) const;
	/** @brief Get the number of quads when comparing TimingData for all players.
	 * @param start the starting row.
	 * @param end the ending row.
	 * @return the number of quads for each player. */
	vector<int> GetNumQuads(int start = 0, int end = MAX_NOTE_ROW) const;
	/** @brief Get the number of holds when comparing TimingData for all players.
	 * @param start the starting row.
	 * @param end the ending row.
	 * @return the number of holds for each player. */
	vector<int> GetNumHoldNotes(int start = 0, int end = MAX_NOTE_ROW) const;
	/** @brief Get the number of rolls when comparing TimingData for all players.
	 * @param start the starting row.
	 * @param end the ending row.
	 * @return the number of rolls for each player. */
	vector<int> GetNumRolls(int start = 0, int end = MAX_NOTE_ROW) const;
	/** @brief Get the number of mines when comparing TimingData for all players.
	 * @param start the starting row.
	 * @param end the ending row.
	 * @return the number of mines for each player. */
	vector<int> GetNumMines(int start = 0, int end = MAX_NOTE_ROW) const;
	/** @brief Get the number of lifts when comparing TimingData for all players.
	 * @param start the starting row.
	 * @param end the ending row.
	 * @return the number of lifts for each player. */
	vector<int> GetNumLifts(int start = 0, int end = MAX_NOTE_ROW) const;
	/** @brief Get the number of fakes when comparing TimingData for all players.
	 * @param start the starting row.
	 * @param end the ending row.
	 * @return the number of fakes for each player. */
	vector<int> GetNumFakes(int start = 0, int end = MAX_NOTE_ROW) const;

private:
	inline const Steps *Real() const		{ return parent ? parent : this; }
	void DeAutogen( bool bCopyNoteData = true ); /* If this Steps is autogenerated, make it a real Steps. */

	/**
	 * @brief Identify this Steps' parent.
	 *
	 * If this Steps is autogenerated, this will point to the autogen
	 * source.  If this is true, m_sNoteDataCompressed will always be empty. */
	const Steps			*parent;

	/* We can have one or both of these; if we have both, they're always identical.
	 * Call Compress() to force us to only have m_sNoteDataCompressed; otherwise, creation of 
	 * these is transparent. */
	mutable HiddenPtr<NoteData>	m_pNoteData;
	mutable bool			m_bNoteDataIsFilled;
	mutable RString			m_sNoteDataCompressed;

	/** @brief The name of the file where these steps are stored. */
	RString				m_sFilename;
	/** @brief true if these Steps were loaded from or saved to disk. */
	bool				m_bSavedToDisk;	
	/** @brief What profile was used? This is ProfileSlot_Invalid if not from a profile. */
	ProfileSlot			m_LoadedFromProfile;

	/* These values are pulled from the autogen source first, if there is one. */
	/** @brief The hash of the steps. This is used only for Edit Steps. */
	mutable unsigned		m_iHash;
	/** @brief The name of the edit, or some other useful description.
	 This used to also contain the step author's name. */
	RString				m_sDescription;
	/** @brief The style of the chart. (e.g. "Pad", "Keyboard") */
	RString				m_sChartStyle;
	/** @brief The difficulty that these steps are assigned to. */
	Difficulty			m_Difficulty;
	/** @brief The numeric difficulty of the Steps, ranging from MIN_METER to MAX_METER. */
	int				m_iMeter;
	/** @brief The radar values used for each player. */
	RadarValues			m_CachedRadarValues[NUM_PLAYERS];
	bool                m_bAreCachedRadarValuesJustLoaded;
	/** @brief The name of the person who created the Steps. */
	RString				m_sCredit;
	/** @brief The name of the chart. */
	RString chartName;
	/** @brief How is the BPM displayed for this chart? */
	DisplayBPM displayBPMType;
	/** @brief What is the minimum specified BPM? */
	float	specifiedBPMMin;
	/**
	 * @brief What is the maximum specified BPM?
	 * If this is a range, then min should not be equal to max. */
	float	specifiedBPMMax;
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
