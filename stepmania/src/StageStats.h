#ifndef StageStats_H
#define StageStats_H
/*
-----------------------------------------------------------------------------
 Class: StageStats

 Desc: Contains statistics for one stage of play - either one song, or a whole course.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/


#include "PlayerNumber.h"
#include "GameConstantsAndTypes.h"
#include "Grade.h"
#include "Style.h"
#include <map>
class Song;
class Steps;


struct StageStats
{
	StageStats();
	void AddStats( const StageStats& other );		// accumulate
	Grade GetGrade( PlayerNumber pn ) const;
	bool OnePassed() const;
	bool AllFailed() const;
	float GetPercentDancePoints( PlayerNumber pn ) const;

	PlayMode	playMode;
	Style		style;
	Song*	pSong;
	enum { STAGE_INVALID, STAGE_NORMAL, STAGE_EXTRA, STAGE_EXTRA2 } StageType;
	Steps*  pSteps[NUM_PLAYERS];
	int		iMeter[NUM_PLAYERS];
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
	float	fRadarPossible[NUM_PLAYERS][NUM_RADAR_CATEGORIES];	// filled in by ScreenGameplay on start of notes
	float	fRadarActual[NUM_PLAYERS][NUM_RADAR_CATEGORIES];	// filled in by ScreenGameplay on start of notes
	float	fSecondsBeforeFail[NUM_PLAYERS];				// -1 means didn't/hasn't failed
	/* The number of songs played and passed, respectively. */
	int		iSongsPassed[NUM_PLAYERS];
	int		iSongsPlayed[NUM_PLAYERS];
	int		iTotalError[NUM_PLAYERS];

	map<float,float>	fLifeRecord[NUM_PLAYERS];
	void	SetLifeRecord( PlayerNumber pn, float life, float pos );
	void	GetLifeRecord( PlayerNumber pn, float *life, int nout ) const;
	float	GetLifeRecordAt( PlayerNumber pn, float pos ) const;
	float	GetLifeRecordLerpAt( PlayerNumber pn, float pos ) const;

	/* pos,combo */
	struct Combo_t
	{
		/* Start and size of this combo, in the same scale as the combo list mapping and
		 * the life record. */
		float start, size;

		/* Combo size, in steps. */
		int cnt;

		/* Size of the combo that didn't come from this stage (rollover from the last song). 
		 * (This is a subset of cnt.) */
		int rollover;

		/* Get the size of the combo that came from this song. */
		int GetStageCnt() const { return cnt - rollover; }

		Combo_t(): start(0), size(0), cnt(0), rollover(0) { }
		bool IsZero() const { return start < 0; }
	};
	vector<Combo_t> ComboList[NUM_PLAYERS];
	float fFirstPos[NUM_PLAYERS], fLastPos[NUM_PLAYERS];

	bool	FullCombo( PlayerNumber pn ) const;
	void	UpdateComboList( PlayerNumber pn, float pos, bool rollover );
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
