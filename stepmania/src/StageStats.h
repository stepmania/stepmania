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


#include "GameConstantsAndTypes.h"	// for NUM_PLAYERS
#include "Grade.h"

class Song;


struct StageStats
{
	StageStats();
	void operator+=( const StageStats& other );		// accumulate
	Grade GetGrade( PlayerNumber pn );

	Song*	pSong;
	int		iMeter[NUM_PLAYERS];
	float	fAliveSeconds[NUM_PLAYERS];				// how far into the music did they last before failing?  Updated by Gameplay.
	bool	bFailed[NUM_PLAYERS];					// true if they have failed at any point during the song
	int		iPossibleDancePoints[NUM_PLAYERS];
	int		iActualDancePoints[NUM_PLAYERS];
	int		iTapNoteScores[NUM_PLAYERS][NUM_TAP_NOTE_SCORES];
	int		iHoldNoteScores[NUM_PLAYERS][NUM_HOLD_NOTE_SCORES];
	int		iMaxCombo[NUM_PLAYERS];
	float	fScore[NUM_PLAYERS];
	float	fRadarPossible[NUM_PLAYERS][NUM_RADAR_CATEGORIES];	// filled in by ScreenGameplay on start of notes
	float	fRadarActual[NUM_PLAYERS][NUM_RADAR_CATEGORIES];	// filled in by ScreenGameplay on start of notes
	float	fSecondsBeforeFail[NUM_PLAYERS];				// -1 means didn't/hasn't failed
	/* The number of songs played and passed, respectively. */
	int		iSongsPassed[NUM_PLAYERS];
	int		iSongsPlayed[NUM_PLAYERS];
};


#endif
