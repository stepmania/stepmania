#pragma once
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
#include "RageSoundSample.h"


class LifeMeterBattery : public LifeMeter
{
public:
	LifeMeterBattery();

	virtual void Load( PlayerNumber p, const PlayerOptions &po );

	virtual void Update( float fDeltaTime );

	virtual void NextSong( Song* pSong );
	virtual void ChangeLife( TapNoteScore score );
	virtual bool IsInDanger();
	virtual bool IsHot();
	virtual bool IsFailing();
	virtual bool FailedEarlier();

	void Refresh();

private:
	int			m_iLivesLeft;
	int			m_iMaxLives;
	bool		m_bFailedEarlier;

	float		m_fBatteryBlinkTime;	// if > 0 battery is blinking

	Sprite		m_sprFrame;
	Sprite		m_sprBattery;
	BitmapText	m_textNumLives;
	BitmapText	m_textPercent;

	RageSoundSample m_soundLoseLife;
	RageSoundSample m_soundGainLife;
};

