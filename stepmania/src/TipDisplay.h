/*
-----------------------------------------------------------------------------
 File: TipDisplay.h

 Desc: A graphic displayed in the TipDisplay during Dancing.

 Copyright (c) 2001 Chris Danford.  All rights reserved.
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

	void Update( float fDeltaTime );

protected:
	BitmapText m_textTip;

	CStringArray m_arrayTips;
	int iLastTipShown;
	
	enum TipState { STATE_FADING_IN, STATE_SHOWING, STATE_FADING_OUT };
	TipState m_TipState;
	float m_fTimeLeftInState;
};

#endif