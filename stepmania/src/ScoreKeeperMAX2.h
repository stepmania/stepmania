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
class Steps;

class ScoreKeeperMAX2: public ScoreKeeper
{
	int				m_iScore;
	int				m_iMaxPossiblePoints;
	int				m_iTapNotesHit;	// number of notes judged so far, needed by scoring

	int				m_iNumTapsAndHolds;
	int			    m_iMaxScoreSoFar; // for nonstop scoring
	int				m_iPointBonus; // the difference to award at the end
 	int				m_iCurToastyCombo;
	bool			m_bIsLastSongInCourse;

	const vector<Steps*>& apNotes;

	void AddScore( TapNoteScore score );

public:
	ScoreKeeperMAX2( const vector<Steps*>& apNotes, PlayerNumber pn);

	// before a song plays (called multiple times if course)
	void OnNextSong( int iSongInCourseIndex, Steps* pNotes, NoteData* pNoteData );

	void HandleTapRowScore( TapNoteScore scoreOfLastTap, int iNumTapsInRow );
	void HandleHoldScore( HoldNoteScore holdScore, TapNoteScore tapScore );

	int TapNoteScoreToDancePoints( TapNoteScore tns );
	int HoldNoteScoreToDancePoints( HoldNoteScore hns );
	int	GetPossibleDancePoints( const NoteData* pNoteData );
};

#endif

