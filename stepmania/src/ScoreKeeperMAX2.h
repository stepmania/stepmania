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
#include "NoteDataWithScoring.h"
struct Notes;

class ScoreKeeperMAX2: public ScoreKeeper
{
	long			m_lScore;
	float			m_fScoreMultiplier;
	int				m_iTapNotesHit;	// number of notes judged so far, needed by scoring

	int				m_iCurToastyCombo;

	void AddScore( TapNoteScore score );

public:
	ScoreKeeperMAX2(Notes *notes, PlayerNumber pn);

	void HandleTapRowScore( TapNoteScore scoreOfLastTap, int iNumTapsInRow, Inventory* pInventory );
	void HandleHoldScore( HoldNoteScore holdScore, TapNoteScore tapScore );

	int TapNoteScoreToDancePoints( TapNoteScore tns );
	int HoldNoteScoreToDancePoints( HoldNoteScore hns );
	int	GetPossibleDancePoints( const NoteData* pNoteData );
};

#endif

