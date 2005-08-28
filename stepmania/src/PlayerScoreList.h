#ifndef PlayerScoreList_H
#define PlayerScoreList_H

#include "ActorFrame.h"
#include "AutoActor.h"
#include "PercentageDisplay.h"
class PlayerState;
class PlayerStageStats;

class PlayerScoreItem : public ActorFrame
{
public:
	void Init( const PlayerState *pPlayerState, const PlayerStageStats *pPlayerStageStats );

protected:
	AutoActor			m_sprFrame;
	PercentageDisplay	m_score;
};

class PlayerScoreList : public ActorFrame
{
public:
	PlayerScoreList();

	void Init( vector<const PlayerState*> vpPlayerState, vector<const PlayerStageStats*> vpPlayerStageStats );

	virtual void Update( float fDelta );

	void Refresh();

protected:
	vector<const PlayerState*> m_vpPlayerState;
	vector<const PlayerStageStats*> m_vpPlayerStageStats;

	RageTimer m_timerRefreshCountdown;

	vector<Actor::TweenState> m_vtsPositions;	// size = num joined players
	vector<AutoActor>		m_vsprBullet;	// size = num joined players
	vector<PlayerScoreItem>	m_vScoreItem;	// size = num joined players
};

#endif

/*
 * (c) 2004 Chris Danford
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
