/* LifeMeterBattery - Battery life meter used in Oni. */

#ifndef LIFEMETERBATTERY_H
#define LIFEMETERBATTERY_H

#include "LifeMeter.h"
#include "Sprite.h"
#include "BitmapText.h"
#include "RageSound.h"
#include "PercentageDisplay.h"


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
	virtual void OnDancePointsChange();	// look in GAMESTATE and update the display
	virtual bool IsInDanger() const;
	virtual bool IsHot() const;
	virtual bool IsFailing() const;
	virtual float GetLife() const;

	void Refresh();

private:
	int			m_iLivesLeft;			// dead when 0
	int			m_iTrailingLivesLeft;	// lags m_iLivesLeft

	float		m_fBatteryBlinkTime;	// if > 0 battery is blinking

	Sprite		m_sprFrame;
	Sprite		m_sprBattery;
	BitmapText	m_textNumLives;

	PercentageDisplay	m_Percent;

	RageSound	m_soundLoseLife;
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
