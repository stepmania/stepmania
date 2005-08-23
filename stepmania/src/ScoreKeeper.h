/* ScoreKeeper - Abstract class to handle scorekeeping, stat-taking, etc. */

#ifndef SCOREKEEPER_H
#define SCOREKEEPER_H

/*
 * Stat handling is in here because that can differ between games, too; for
 * example, some games count double taps as a single note in scoring and
 * some count per-tap.
 *
 * Results are injected directly into GameState.
 */

#include "Attack.h"
#include "GameConstantsAndTypes.h"	// for TapNoteScore and HoldNoteScore
class NoteData;
class Inventory;
class Song;
class Steps;
class PlayerState;
class PlayerStageStats;


class ScoreKeeper
{
protected:
	PlayerState* m_pPlayerState;
	PlayerStageStats* m_pPlayerStageStats;

	/* Common toggles that this class handles directly: */

	/* If true, doubles count as 2+ in stat counts; if false, doubles count as
	 * only one. */ /* (not yet) */
//	bool Stats_DoublesCount;

public:
	ScoreKeeper(PlayerState* pPlayerState,PlayerStageStats* pPlayerStageStats) { m_pPlayerState=pPlayerState; m_pPlayerStageStats=pPlayerStageStats; }
	virtual ~ScoreKeeper() { }
	virtual void Load(
		const vector<Song*>& apSongs,
		const vector<Steps*>& apSteps,
		const vector<AttackArray> &asModifiers ) { }

	virtual void DrawPrimitives() { }
	virtual void Update( float fDelta ) { }

	/* Note that pNoteData will include any transformations due to modifiers. */
	virtual void OnNextSong( int iSongInCourseIndex, const Steps* pSteps, const NoteData* pNoteData ) = 0;	// before a song plays (called multiple times if course)

	virtual void HandleTapScore( TapNoteScore score ) = 0;
	virtual void HandleTapRowScore( TapNoteScore scoreOfLastTap, int iNumTapsInRow ) = 0;
	virtual void HandleHoldScore( HoldNoteScore holdScore, TapNoteScore tapScore ) = 0;
};

#endif

/*
 * (c) 2001-2004 Chris Danford, Glenn Maynard
 * All rights reserved.
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, and/or sell copies of the Software, and to permit persons to
 * whom the Software is furnished to do so, provided that the above
 * copyright notice(s) and this permission notice appear in all copies of
 * the Software and that both the above copyright notice(s) and this
 * permission notice appear in supporting documentation.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT OF
 * THIRD PARTY RIGHTS. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR HOLDERS
 * INCLUDED IN THIS NOTICE BE LIABLE FOR ANY CLAIM, OR ANY SPECIAL INDIRECT
 * OR CONSEQUENTIAL DAMAGES, OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS
 * OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
 * OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */
