#ifndef SCOREKEEPER_H
#define SCOREKEEPER_H 1
/*
-----------------------------------------------------------------------------
 Class: ScoreKeeper

 Abstract class to handle scorekeeping, stat-taking, etc.

 Stat handling is in here because that can differ between games, too; for
 example, some games count double taps as a single note in scoring and
 some count per-tap.

 Results are injected directly into GameState.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
	Glenn Maynard
-----------------------------------------------------------------------------
*/

#include "Actor.h"
#include "PlayerNumber.h"
#include "GameConstantsAndTypes.h"

class ScoreKeeper: public Actor 
{
protected:
	PlayerNumber m_PlayerNumber;

	/* Common toggles that this class handles directly: */

	/* If true, doubles count as 2+ in stat counts; if false, doubles count as
	 * only one. */ /* (not yet) */
//	bool Stats_DoublesCount;

public:
	ScoreKeeper(PlayerNumber pn) { m_PlayerNumber=pn; }
	virtual void DrawPrimitives() { }

	virtual void HandleTapScore( TapNoteScore score, int iNumTapsInRow ) { }
	virtual void HandleHoldScore( HoldNoteScore score, TapNoteScore TapNoteScore ) { }
};

#endif
