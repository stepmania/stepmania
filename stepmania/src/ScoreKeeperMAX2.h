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
#include "GameState.h"
class Steps;

class ScoreKeeperMAX2: public ScoreKeeper
{
	int				m_iScoreRemainder;
	int				m_iMaxPossiblePoints;
	int				m_iTapNotesHit;	// number of notes judged so far, needed by scoring

	int				m_iNumTapsAndHolds;
	int			    m_iMaxScoreSoFar; // for nonstop scoring
	int				m_iPointBonus; // the difference to award at the end
 	int				m_iCurToastyCombo;
	bool			m_bIsLastSongInCourse;

	const vector<Steps*>& apNotes;

	void AddScore( TapNoteScore score );

	/* Configuration: */
	/* Score after each tap will be rounded to the nearest m_iRoundTo; 1 to do nothing. */
	int				m_iRoundTo;
	int				m_ComboBonusFactor[NUM_TAP_NOTE_SCORES];

public:
	ScoreKeeperMAX2( const vector<Song*>& apSongs, const vector<Steps*>& apNotes, const vector<GameState::AttackArray> &asModifiers, PlayerNumber pn);

	// before a song plays (called multiple times if course)
	void OnNextSong( int iSongInCourseIndex, const Steps* pNotes, const NoteData* pNoteData );

	void HandleTapRowScore( TapNoteScore scoreOfLastTap, int iNumTapsInRow, int iNumAdditions );
	void HandleHoldScore( HoldNoteScore holdScore, TapNoteScore tapScore );

private:
	int TapNoteScoreToDancePoints( TapNoteScore tns );
	int HoldNoteScoreToDancePoints( HoldNoteScore hns );
	int	GetPossibleDancePoints( const NoteData &preNoteData, const NoteData &postNoteData );
};

#endif

