#pragma once
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
#include "RageSoundSample.h"


class MenuTimer : public ActorFrame
{
public:
	MenuTimer();
	
	virtual void Update( float fDeltaTime ); 
	void StopTimer();
	void StallTimer();

protected:
	float m_fSecondsLeft;

	BitmapText m_textDigit1;
	BitmapText m_textDigit2;

	RageSoundSample	m_soundBeep;
};
