/* ScoreKeeperRave - Launches attacks in PLAY_MODE_RAVE. */

#ifndef ScoreKeeperRave_H
#define ScoreKeeperRave_H

#include "ScoreKeeper.h"
#include "GameConstantsAndTypes.h"


class ScoreKeeperRave : public ScoreKeeper
{
public:
	// Overrides
	ScoreKeeperRave(PlayerNumber pn);
	void Update( float fDelta );
	void OnNextSong( int iSongInCourseIndex, const Steps* pSteps, const NoteData* pNoteData );	// before a song plays (called multiple times if course)
	void HandleTapScore( TapNoteScore score );
	void HandleTapRowScore( TapNoteScore scoreOfLastTap, int iNumTapsInRow );
	void HandleHoldScore( HoldNoteScore holdScore, TapNoteScore tapScore );

protected:
	void LaunchAttack( AttackLevel al );
	void AddSuperMeterDelta( float fUnscaledPercentChange );
};

#endif

/*
 * (c) 2001-2004 Chris Danford
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
