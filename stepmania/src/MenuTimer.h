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
#include "song.h"
#include "ActorFrame.h"
#include "BitmapText.h"
#include "RageSound.h"


class MenuTimer : public ActorFrame
{
public:
	MenuTimer();
	
	virtual void Update( float fDeltaTime ); 

	void SetSeconds( int iTimerSeconds );
	void Start();		// resume countdown from paused
	void Pause();		// don't count down
	void Stop();		// set to "00" and pause
	void Disable();		// set to "99" and pause
	void Stall();		// pause countdown for a sec
	void EnableStealth( bool bStealth );	// make timer invisible and silent

protected:
	float m_fSecondsLeft;
	float m_fStallSeconds;
	bool m_bPaused;

	void SetText( int iSeconds );

	BitmapText m_textDigit1;
	BitmapText m_textDigit2;

	RageSound	m_soundBeep;
};

#endif
