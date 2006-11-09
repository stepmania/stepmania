#ifndef LIFEMETERBAR_H
#define LIFEMETERBAR_H

#include "LifeMeter.h"
#include "Sprite.h"
#include "AutoActor.h"
#include "Quad.h"
class StreamDisplay;


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
	virtual void AfterLifeChanged();
	virtual bool IsInDanger() const;
	virtual bool IsPastPassmark() const;
	virtual bool IsHot() const;
	virtual bool IsFailing() const;
	virtual float GetLife() const { return m_fLifePercentage; }

	void UpdateNonstopLifebar();
	void FillForHowToPlay(int NumT2s, int NumMisses);
	// this function is solely for HowToPlay

private:
	AutoActor		m_sprBackground;
	Quad			m_quadDangerGlow;
	StreamDisplay*		m_pStream;
	AutoActor		m_sprFrame;

	float			m_fLifePercentage;

	float			m_fPassingAlpha;
	float			m_fHotAlpha;

	float			m_fBaseLifeDifficulty;
	float			m_fLifeDifficulty;		// essentially same as pref

	int			m_iProgressiveLifebar;		// cached from prefs
	int			m_iMissCombo;			// current number of progressive W5/miss

	int			m_iComboToRegainLife;		// combo needed before lifebar starts filling up after fail
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
