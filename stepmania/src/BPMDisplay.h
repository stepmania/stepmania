/*
-----------------------------------------------------------------------------
 File: BPMDisplay.h

 Desc: A graphic displayed in the BPMDisplay during Dancing.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
-----------------------------------------------------------------------------
*/


#ifndef _BPMDisplay_H_
#define _BPMDisplay_H_


#include "Sprite.h"
#include "Song.h"
#include "ActorFrame.h"
#include "BitmapText.h"
#include "Quad.h"


class BPMDisplay : public ActorFrame
{
public:
	BPMDisplay();
	virtual void Update( float fDeltaTime ); 
	void SetBPMRange( float fLowBPM, float fHighBPM );

protected:
	BitmapText m_textBPM;
	BitmapText m_textLabel;
	Quad m_rectFrame;

	float m_fCurrentBPM;
	float m_fLowBPM, m_fHighBPM;
	enum CountingState{ counting_up, holding_up, counting_down, holding_down };
	CountingState m_CountingState;
	float m_fTimeLeftInState;

	bool m_bExtraStage;		// cycle BPM through random values if extra stage
};

#endif