/* StageStats - Contains statistics for one stage of play - either one song, or a whole course. */

#ifndef STAGE_STATS_H
#define STAGE_STATS_H

#include "PlayerNumber.h"
#include "GameConstantsAndTypes.h"
#include "Grade.h"
#include "RadarValues.h"
#include <map>
class Song;
class Steps;
class Style;

struct StageStats
{
	StageStats() { Init(); }
	void Init();

	void AssertValid( PlayerNumber pn ) const;
	
	void AddStats( const StageStats& other );		// accumulate
	Grade GetGrade( PlayerNumber pn ) const;
	bool OnePassed() const;
	bool AllFailed() const;
	bool AllFailedEarlier() const;
	float GetPercentDancePoints( PlayerNumber pn ) const;

	PlayMode	playMode;
	const Style*	pStyle;
	vector<Song*>	vpSongs;
	enum { STAGE_INVALID, STAGE_NORMAL, STAGE_EXTRA, STAGE_EXTRA2 } StageType;
	vector<Steps*>  vpSteps[NUM_PLAYERS];
	int		GetAverageMeter( PlayerNumber pn ) const;
	float	fAliveSeconds[NUM_PLAYERS];		// how far into the music did they last before failing?  Updated by Gameplay, scaled by music rate.
	float	fGameplaySeconds;				// how many seconds before gameplay ended.  Updated by Gameplay, not scaled by music rate.

	/* Set if the player actually failed at any point during the song.  This is always
	 * false in FAIL_OFF.  If recovery is enabled and two players are playing,
	 * this is only set if both players were failing at the same time. */
	bool	bFailed[NUM_PLAYERS];

	/* This indicates whether the player bottomed out his bar/ran out of lives at some
	 * point during the song.  It's set in all fail modes. */
	bool	bFailedEarlier[NUM_PLAYERS];
	int		iPossibleDancePoints[NUM_PLAYERS];
	int		iActualDancePoints[NUM_PLAYERS];
	int		iTapNoteScores[NUM_PLAYERS][NUM_TAP_NOTE_SCORES];
	int		iHoldNoteScores[NUM_PLAYERS][NUM_HOLD_NOTE_SCORES];
	int		iCurCombo[NUM_PLAYERS];
	int		iMaxCombo[NUM_PLAYERS];
	int		iCurMissCombo[NUM_PLAYERS];
	int		iScore[NUM_PLAYERS];
	int		iBonus[NUM_PLAYERS];  // bonus to be added on screeneval
	RadarValues	radarPossible[NUM_PLAYERS];	// filled in by ScreenGameplay on start of notes
	RadarValues radarActual[NUM_PLAYERS];
	float	fSecondsBeforeFail[NUM_PLAYERS];				// -1 means didn't/hasn't failed
	/* The number of songs played and passed, respectively. */
	int		iSongsPassed[NUM_PLAYERS];
	int		iSongsPlayed[NUM_PLAYERS];
	int		iTotalError[NUM_PLAYERS];

	map<float,float>	fLifeRecord[NUM_PLAYERS];
	void	SetLifeRecordAt( PlayerNumber pn, float fLife, float fSecond );
	void	GetLifeRecord( PlayerNumber pn, float *fLifeOut, int iNumSamples ) const;
	float	GetLifeRecordAt( PlayerNumber pn, float fSecond ) const;
	float	GetLifeRecordLerpAt( PlayerNumber pn, float fSecond ) const;

	struct Combo_t
	{
		/* Start and size of this combo, in the same scale as the combo list mapping and
		 * the life record. */
		float fStartSecond, fSizeSeconds;

		/* Combo size, in steps. */
		int cnt;

		/* Size of the combo that didn't come from this stage (rollover from the last song). 
		 * (This is a subset of cnt.) */
		int rollover;

		/* Get the size of the combo that came from this song. */
		int GetStageCnt() const { return cnt - rollover; }

		Combo_t(): fStartSecond(0), fSizeSeconds(0), cnt(0), rollover(0) { }
		bool IsZero() const { return fStartSecond < 0; }
	};
	vector<Combo_t> ComboList[NUM_PLAYERS];
	float fFirstSecond[NUM_PLAYERS], fLastSecond[NUM_PLAYERS];

	int		GetComboAtStartOfStage( PlayerNumber pn ) const;
	bool	FullComboOfScore( PlayerNumber pn, TapNoteScore tnsAllGreaterOrEqual ) const;
	bool	FullCombo( PlayerNumber pn ) const { return FullComboOfScore(pn,TNS_GREAT); }
	bool	SingleDigitsOfScore( PlayerNumber pn, TapNoteScore tnsAllGreaterOrEqual ) const;
	bool	OneOfScore( PlayerNumber pn, TapNoteScore tnsAllGreaterOrEqual ) const;
	int		GetTotalTaps( PlayerNumber pn ) const;
	float	GetPercentageOfTaps( PlayerNumber pn, TapNoteScore tns ) const;
	void	UpdateComboList( PlayerNumber pn, float fSecond, bool rollover );
	Combo_t GetMaxCombo( PlayerNumber pn ) const;
};

/*
 * This was in GameState, but GameState.h is used by tons of files, and this object
 * is only used by 20 or so.
 *
 * Stage Statistics: 
 * Arcade:	for the current stage (one song).  
 * Nonstop/Oni/Endless:	 for current course (which usually contains multiple songs)
 */
extern StageStats	g_CurStageStats;				// current stage (not necessarily passed if Extra Stage)
extern vector<StageStats>	g_vPlayedStageStats;


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
