#ifndef SCORE_DISPLAY_H
#define SCORE_DISPLAY_H

#include "PlayerNumber.h"
#include "ActorFrame.h"
#include "GameConstantsAndTypes.h"

class PlayerState;
class PlayerStageStats;

class ScoreDisplay : public ActorFrame
{
public:
	virtual void Init( const PlayerState* pPlayerState, const PlayerStageStats* pPlayerStageStats );

	virtual void SetScore( int iNewScore ) {}
	virtual void OnLoadSong() {};
	/* Notification of a tap note judgment.  This *is* called for
	 * the head of hold notes. */
	virtual void OnJudgment( TapNoteScore score ) {};
	/* Notification of a hold judgment.  tscore is the score
	 * received for the initial tap note. */
	virtual void OnJudgment( HoldNoteScore score, TapNoteScore tscore ) {};

protected:
	const PlayerState* m_pPlayerState;	// needed to look up stats
	const PlayerStageStats* m_pPlayerStageStats;	// needed to look up stats
};

#endif

/*
 * (c) 2001-2003 Chris Danford
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
