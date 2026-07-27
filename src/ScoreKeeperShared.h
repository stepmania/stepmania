#ifndef SCORE_KEEPER_SHARED_H
#define SCORE_KEEPER_SHARED_H

#include "ScoreKeeperNormal.h"
#include "PlayerNumber.h"

#include <vector>


/** @brief ScoreKeeper for Routine mode. */
class ScoreKeeperShared : public ScoreKeeperNormal
{
public:
	ScoreKeeperShared( PlayerState *pPlayerState, PlayerStageStats *pPlayerStageStats );

	virtual void Load(
	const std::vector<Song*> &apSongs,
	const std::vector<Steps*> &apSteps,
	const std::vector<AttackArray> &asModifiers );

	virtual void DrawPrimitives();
	virtual void Update( float fDelta );

	virtual void OnNextSong( int iSongInCourseIndex, const Steps* pSteps, const NoteData* pNoteData );
	virtual void HandleTapScore( const TapNote &tn );
	virtual void HandleTapRowScore( const NoteData &nd, int iRow );
	virtual void HandleHoldScore( const TapNote &tn );
	virtual void HandleHoldActiveSeconds( float fMusicSecondsHeld );
	virtual void HandleHoldCheckpointScore( const NoteData &nd, int iRow, int iNumHoldsHeldThisRow, int iNumHoldsMissedThisRow );
	virtual void HandleTapScoreNone();
};
#endif

/*
 * (c) 2006 Steve Checkoway
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
