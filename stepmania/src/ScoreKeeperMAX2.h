#ifndef SCOREKEEPER_MAX2_H
#define SCOREKEEPER_MAX2_H 1
/*
-----------------------------------------------------------------------------
 Class: ScoreKeeperMAX2

 Class to handle scorekeeping, MAX2-style.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
	Glenn Maynard
-----------------------------------------------------------------------------
*/

#include "ScoreKeeper.h"
#include "Notes.h"
#include "NoteDataWithScoring.h"

class ScoreKeeperMAX2: public ScoreKeeper
{
	long			m_lScore;
	float			m_fScoreMultiplier;
	int				m_iTapNotesHit;	// number of notes judged so far, needed by scoring

	void AddScore( TapNoteScore score );

public:
	ScoreKeeperMAX2(Notes *notes, NoteDataWithScoring &data, PlayerNumber pn);

	void HandleTapRowScore( TapNoteScore scoreOfLastTap, int iNumTapsInRow );
	void HandleHoldScore( HoldNoteScore holdScore, TapNoteScore tapScore );
};

#endif

