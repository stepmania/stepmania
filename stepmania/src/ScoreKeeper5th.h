#ifndef SCOREKEEPER_5TH_H
#define SCOREKEEPER_5TH_H 1
/*
-----------------------------------------------------------------------------
 Class: ScoreKeeper5th

 Class to handle scorekeeping, 5th-style.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
	Glenn Maynard
-----------------------------------------------------------------------------
*/

#include "ScoreKeeper.h"
#include "NoteDataWithScoring.h"
class Steps;

class ScoreKeeper5th: public ScoreKeeper
{
	int				m_iScore;
	int				m_iMaxPossiblePoints;
	int				m_iTapNotesHit;	// number of notes judged so far, needed by scoring

	int				m_iNumTapsAndHolds;
	int			    m_iMaxScoreSoFar; // for nonstop scoring
	int				m_iComboBonus;  // 5th-mix combo bonus at end
	int				m_iPointBonus; // the difference to award at the end
 	int				m_iCurToastyCombo;
	bool			m_bIsLastSongInCourse;

	const vector<Steps*>& apNotes;

	void AddScore( TapNoteScore score );

public:
	ScoreKeeper5th( const vector<Steps*>& apNotes_, const CStringArray &asModifiers, PlayerNumber pn_ );

	// before a song plays (called multiple times if course)
	void OnNextSong( int iSongInCourseIndex, Steps* pNotes, NoteData* pNoteData );

	void HandleTapRowScore( TapNoteScore scoreOfLastTap, int iNumTapsInRow, int iNumAdditions );
	void HandleHoldScore( HoldNoteScore holdScore, TapNoteScore tapScore );

	int TapNoteScoreToDancePoints( TapNoteScore tns );
	int HoldNoteScoreToDancePoints( HoldNoteScore hns );
	int	GetPossibleDancePoints( const NoteData &preNoteData, const NoteData &postNoteData );
};

#endif

