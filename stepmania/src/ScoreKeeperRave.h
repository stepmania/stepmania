#ifndef ScoreKeeperRave_H
#define ScoreKeeperRave_H
/*
-----------------------------------------------------------------------------
 Class: ScoreKeeperRave

 Desc: Launches attacks in PLAY_MODE_RAVE.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "ScoreKeeper.h"
#include "GameConstantsAndTypes.h"


class ScoreKeeperRave : public ScoreKeeper
{
public:
	// Overrides
	ScoreKeeperRave(PlayerNumber pn);
	void Update( float fDelta );
	void OnNextSong( int iSongInCourseIndex, const Steps* pNotes, const NoteData* pNoteData );	// before a song plays (called multiple times if course)
	void HandleTapScore( TapNoteScore score );
	void HandleTapRowScore( TapNoteScore scoreOfLastTap, int iNumTapsInRow );
	void HandleHoldScore( HoldNoteScore holdScore, TapNoteScore tapScore );

protected:
	void LaunchAttack( AttackLevel al );
	void AddSuperMeterDelta( float fUnscaledPercentChange );
};

#endif
