/* ScoreKeeper - Abstract class to handle scorekeeping, stat-taking, etc. */

#ifndef SCORE_KEEPER_H
#define SCORE_KEEPER_H

/*
 * Stat handling is in here because that can differ between games, too; for
 * example, some games count double taps as a single note in scoring and
 * some count per-tap.
 *
 * Results are injected directly into the PlayerStageStats.
 */

#include "GameConstantsAndTypes.h"

class NoteData;
class Inventory;
class Song;
class Steps;
class PlayerState;
class PlayerStageStats;
struct TapNote;
struct AttackArray;

class ScoreKeeper
{
public:
	static ScoreKeeper* MakeScoreKeeper( RString sClassName, PlayerState *pPlayerState, PlayerStageStats *pPlayerStageStats );

protected:
	PlayerState		*m_pPlayerState;
	PlayerStageStats	*m_pPlayerStageStats;

	/* Common toggles that this class handles directly: */

	/* If true, doubles count as 2+ in stat counts; if false, doubles count as
	 * only one. */ /* (not yet) */
//	bool Stats_DoublesCount;

public:
	ScoreKeeper( PlayerState *pPlayerState, PlayerStageStats *pPlayerStageStats );
	virtual ~ScoreKeeper() { }
	virtual void Load(
		const vector<Song*> &apSongs,
		const vector<Steps*> &apSteps,
		const vector<AttackArray> &asModifiers ) { }

	virtual void DrawPrimitives() { }
	virtual void Update( float fDelta ) { }

	/* Note that pNoteData will include any transformations due to modifiers. */
	virtual void OnNextSong( int iSongInCourseIndex, const Steps* pSteps, const NoteData* pNoteData ) = 0;	// before a song plays (called multiple times if course)

	// HandleTap* is called before HandleTapRow*
	virtual void HandleTapScore( const TapNote &tn ) = 0;
	virtual void HandleTapRowScore( const NoteData &nd, int iRow ) = 0;
	virtual void HandleHoldScore( const TapNote &tn ) = 0;
	virtual void HandleHoldActiveSeconds( float fMusicSecondsHeld ) = 0;
	virtual void HandleTapScoreNone() = 0;

protected:
	void GetScoreOfLastTapInRow( const NoteData &nd, int iRow, TapNoteScore &tnsOut, int &iNumTapsInRowOut );
};

#endif

/*
 * (c) 2001-2006 Chris Danford, Glenn Maynard, Steve Checkoway
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
