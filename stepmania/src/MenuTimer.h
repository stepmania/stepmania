#ifndef MENUTIMER_H
#define MENUTIMER_H
/*
-----------------------------------------------------------------------------
 Class: MenuTimer

 Desc: A timer in the upper right corner of the menu that ticks down.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "Sprite.h"
#include "Song.h"
#include "ActorFrame.h"
#include "BitmapText.h"
#include "RageSound.h"


class MenuTimer : public ActorFrame
{
public:
	MenuTimer();
	
	virtual void Update( float fDeltaTime ); 

	void SetTimer( int iTimerSeconds );
	void StartTimer();
	void StopTimer();
	void StallTimer();

	void StealthTimer(int iActive);

protected:
	float m_fSecondsLeft;
	float m_fStallSeconds;

	bool m_bTimerStopped;

	BitmapText m_textDigit1;
	BitmapText m_textDigit2;

	RageSound	m_soundBeep;
};

#endif
