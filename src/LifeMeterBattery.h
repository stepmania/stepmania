#ifndef LIFEMETERBATTERY_H
#define LIFEMETERBATTERY_H

#include "LifeMeter.h"
#include "Sprite.h"
#include "BitmapText.h"
#include "RageSound.h"
#include "PercentageDisplay.h"
#include "ThemeMetric.h"
#include "AutoActor.h"

/** @brief Battery life meter used in Oni mode. */
class LifeMeterBattery : public LifeMeter
{
public:
	LifeMeterBattery();

	virtual void Load( const PlayerState *pPlayerState, PlayerStageStats *pPlayerStageStats );

	virtual void Update( float fDeltaTime );

	virtual void OnSongEnded();
	virtual void ChangeLife( TapNoteScore score );
	virtual void ChangeLife( HoldNoteScore score, TapNoteScore tscore );
	virtual void ChangeLife( float fDeltaLifePercent );
	virtual void SetLife(float value);
	virtual void HandleTapScoreNone();
	virtual bool IsInDanger() const;
	virtual bool IsHot() const;
	virtual bool IsFailing() const;
	virtual float GetLife() const;
	virtual int GetRemainingLives() const;

	virtual void BroadcastLifeChanged(bool lost_life);

	void Refresh();
	int GetLivesLeft() { return m_iLivesLeft; }
	int GetTotalLives();
	void ChangeLives(int iLifeDiff);

	// Lua
	virtual void PushSelf( lua_State *L );

private:
	void SubtractLives( int iLives );
	void AddLives( int iLives );

	int			m_iLivesLeft;			// dead when 0
	int			m_iTrailingLivesLeft;	// lags m_iLivesLeft

	ThemeMetric<float> BATTERY_BLINK_TIME;
	ThemeMetric<TapNoteScore>	MIN_SCORE_TO_KEEP_LIFE;
	ThemeMetric<int>	DANGER_THRESHOLD;
	ThemeMetric<int>	MAX_LIVES;
	ThemeMetric<int>	SUBTRACT_LIVES;
	ThemeMetric<int>	MINES_SUBTRACT_LIVES;
	ThemeMetric<int>	HELD_ADD_LIVES;
	ThemeMetric<int>	LET_GO_SUBTRACT_LIVES;
	ThemeMetric<LuaReference> COURSE_SONG_REWARD_LIVES;
	ThemeMetric<RString> LIVES_FORMAT;

	AutoActor	m_sprFrame;
	AutoActor	m_sprBattery;
	BitmapText	m_textNumLives;

	PercentageDisplay	m_Percent;

	/** @brief The sound played when a Player loses a life. */
	RageSound	m_soundLoseLife;
	/** @brief The sound played when a Player gains a life. */
	RageSound	m_soundGainLife;
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
