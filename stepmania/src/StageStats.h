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
class Song;
class Steps;


struct StageStats
{
	StageStats();
	void AddStats( const StageStats& other );		// accumulate
	Grade GetGrade( PlayerNumber pn );
	bool OnePassed() const;
	bool AllFailed() const;
	float GetPercentDancePoints( PlayerNumber pn ) const;

	Song*	pSong;
	enum { STAGE_INVALID, STAGE_NORMAL, STAGE_EXTRA, STAGE_EXTRA2 } StageType;
	Steps*  pSteps[NUM_PLAYERS];
	int		iMeter[NUM_PLAYERS];
	float	fAliveSeconds[NUM_PLAYERS];				// how far into the music did they last before failing?  Updated by Gameplay.

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

	enum { LIFE_RECORD_RESOLUTION=1000 };
	float	fLifeRecord[NUM_PLAYERS][LIFE_RECORD_RESOLUTION];
	void	SetLifeRecord( PlayerNumber pn, float life, float pos );
	void	GetLifeRecord( PlayerNumber pn, float *life, int nout ) const;

	/* pos,combo */
	struct Combo_t
	{
		/* Start and size of this combo, in the same scale as the combo list mapping and
		 * the life record. */
		float start, size;

		/* Combo size, in steps. */
		int cnt;

		/* Combo rollover from the last song (see UpdateComboList for details). */
		int rollover;

		Combo_t(): start(0), size(0), cnt(0), rollover(0) { }
		bool IsZero() const { return start < 0; }
	};
	float fFirstPos[NUM_PLAYERS], fLastPos[NUM_PLAYERS];

	bool	FullCombo( PlayerNumber pn ) const;
	void	UpdateComboList( PlayerNumber pn, float pos );
	Combo_t GetMaxCombo( PlayerNumber pn ) const;

	void	Finish();
	vector<Combo_t> ComboList[NUM_PLAYERS];
};


#endif
