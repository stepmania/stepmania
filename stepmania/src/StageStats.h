/* StageStats - Contains statistics for one stage of play - either one song, or a whole course. */

#ifndef StageStats_H
#define StageStats_H

#include "PlayerNumber.h"
#include "PlayerStageStats.h"
class Song;
class Style;
struct lua_State;


class StageStats
{
public:
	StageStats();
	void Init();

	void AssertValid( PlayerNumber pn ) const;
	void AssertValid( MultiPlayer mp ) const;

	void AddStats( const StageStats& other );		// accumulate

	bool OnePassed() const;
	bool AllFailed() const;
	bool AllFailedEarlier() const;

	int		GetAverageMeter( PlayerNumber pn ) const;

	PlayMode	playMode;
	const Style*	pStyle;
	vector<Song*>	vpPlayedSongs;
	vector<Song*>	vpPossibleSongs;
	enum { Stage_Invalid, STAGE_NORMAL, STAGE_EXTRA, STAGE_EXTRA2 } StageType;

	bool	bGaveUp;	// exited gameplay by giving up
	bool	bUsedAutoplay;	// used autoplay at any point during gameplay

	// TODO: These are updated in ScreenGameplay::Update based on fDelta.  
	// They should be made more accurate.
	float	fGameplaySeconds;		// how many seconds before gameplay ended.  Updated by Gameplay, not scaled by music rate.
	float	fStepsSeconds;		// this is <= fGameplaySeconds unless the song has steps past the end
	float	fMusicRate;

	// Total number of seconds between first beat and last beat for every song.
	float GetTotalPossibleStepsSeconds() const;

	PlayerStageStats m_player[NUM_PLAYERS];
	PlayerStageStats m_multiPlayer[NUM_MultiPlayer];

	void CommitScores( bool bSummary );

	// Lua
	void PushSelf( lua_State *L );
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
