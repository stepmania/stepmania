#ifndef LIFEMETERBAR_H
#define LIFEMETERBAR_H

#include "LifeMeter.h"
#include "Sprite.h"
#include "AutoActor.h"
#include "Quad.h"
#include "ThemeMetric.h"
class StreamDisplay;

/** @brief The player's life represented as a bar. */
class LifeMeterBar : public LifeMeter
{
public:
	LifeMeterBar();
	~LifeMeterBar();

	virtual void Load( const PlayerState *pPlayerState, PlayerStageStats *pPlayerStageStats );

	virtual void Update( float fDeltaTime );

	virtual void ChangeLife( TapNoteScore score );
	virtual void ChangeLife( HoldNoteScore score, TapNoteScore tscore  );
	virtual void ChangeLife( float fDeltaLifePercent );
	virtual void SetLife(float value);
	virtual void HandleTapScoreNone();
	virtual void AfterLifeChanged();
	virtual bool IsInDanger() const;
	virtual bool IsHot() const;
	virtual bool IsFailing() const;
	virtual float GetLife() const { return m_fLifePercentage; }

	void UpdateNonstopLifebar();
	void FillForHowToPlay(int NumT2s, int NumMisses);
	// this function is solely for HowToPlay

private:
	ThemeMetric<float> DANGER_THRESHOLD;
	ThemeMetric<float> INITIAL_VALUE;
	ThemeMetric<float> HOT_VALUE;
	ThemeMetric<float> LIFE_MULTIPLIER;
	ThemeMetric<bool> FORCE_LIFE_DIFFICULTY_ON_EXTRA_STAGE;
	ThemeMetric<TapNoteScore>   MIN_STAY_ALIVE;
	ThemeMetric<float>	EXTRA_STAGE_LIFE_DIFFICULTY;

	ThemeMetric1D<float> m_fLifePercentChange;

	AutoActor		m_sprUnder;
	AutoActor		m_sprDanger;
	StreamDisplay*	m_pStream;
	AutoActor		m_sprOver;

	float		m_fLifePercentage;

	float		m_fPassingAlpha;
	float		m_fHotAlpha;

	bool		m_bMercifulBeginnerInEffect;
	float		m_fBaseLifeDifficulty;
	float		m_fLifeDifficulty;		// essentially same as pref

	int			m_iProgressiveLifebar;		// cached from prefs
	/** @brief The current number of progressive W5/miss rankings. */
	int			m_iMissCombo;
	/** @brief The combo needed before the life bar starts to fill up after a Player failed. */
	int			m_iComboToRegainLife;
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
 * 
 * (c) 2016- Electromuis, Anton Grootes
 * This branch of https://github.com/stepmania/stepmania
 * will from here on out be released as GPL v3 (wich converts from the previous MIT license)
 */
