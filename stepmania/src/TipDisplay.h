/*
-----------------------------------------------------------------------------
 File: TipDisplay.h

 Desc: A graphic displayed in the TipDisplay during Dancing.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
-----------------------------------------------------------------------------
*/


#ifndef _TipDisplay_H_
#define _TipDisplay_H_


#include "Sprite.h"
#include "Song.h"
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