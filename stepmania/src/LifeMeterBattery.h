#ifndef LIFEMETERBATTERY_H
#define LIFEMETERBATTERY_H
/*
-----------------------------------------------------------------------------
 Class: LifeMeterBattery

 Desc: The battery life meter used in Oni.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/


#include "LifeMeter.h"
#include "Sprite.h"
#include "BitmapText.h"
#include "RageSound.h"


class LifeMeterBattery : public LifeMeter
{
public:
	LifeMeterBattery();

	virtual void Load( PlayerNumber pn );

	virtual void Update( float fDeltaTime );

	virtual void OnSongEnded();
	virtual void ChangeLife( TapNoteScore score );
	virtual void ChangeLife( HoldNoteScore score, TapNoteScore tscore );
	virtual void OnDancePointsChange();	// look in GAMESTATE and update the display
	virtual bool IsInDanger();
	virtual bool IsHot();
	virtual bool IsFailing();
	virtual bool FailedEarlier();

	void Refresh();

private:
	int			m_iLivesLeft;			// dead when 0
	int			m_iTrailingLivesLeft;	// lags m_iLivesLeft
	bool		m_bFailedEarlier;

	float		m_fBatteryBlinkTime;	// if > 0 battery is blinking

	Sprite		m_sprFrame;
	Sprite		m_sprBattery;
	BitmapText	m_textNumLives;
	BitmapText	m_textPercent;

	RageSound	m_soundLoseLife;
	RageSound	m_soundGainLife;
};


#endif
