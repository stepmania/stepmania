#ifndef TIPDISPLAY_H
#define TIPDISPLAY_H
/*
-----------------------------------------------------------------------------
 Class: TipDisplay

 Desc: A BitmapText that cycles through messages.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "Sprite.h"
#include "song.h"
#include "ActorFrame.h"
#include "BitmapText.h"


class TipDisplay : public ActorFrame
{
public:
	TipDisplay();
	void SetTips( CStringArray &arrayTips );

	virtual void Update( float fDeltaTime );

protected:
	BitmapText m_textTip;

	CStringArray m_arrayTips;
	int m_iCurTipIndex;
	
	float m_fSecsUntilSwitch;
};

#endif
