#ifndef _BPMDisplay_H_
#define _BPMDisplay_H_
/*
-----------------------------------------------------------------------------
 File: BPMDisplay.h

 Desc: A graphic displayed in the BPMDisplay during Dancing.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "Sprite.h"
#include "song.h"
#include "ActorFrame.h"
#include "BitmapText.h"
#include "Quad.h"


class BPMDisplay : public ActorFrame
{
public:
	BPMDisplay();
	virtual void Update( float fDeltaTime ); 
	void SetBPMRange( float fLowBPM, float fHighBPM );
	void CycleRandomly();
	void NoBPM();

protected:
	BitmapText m_textBPM;
	Sprite m_sprLabel;

	float m_fCurrentBPM;
	float m_fLowBPM, m_fHighBPM;
	enum CountingState {
		counting_up, 
		holding_up, 
		counting_down, 
		holding_down,
		cycle_randomly,
		no_bpm	// show dashes
	} m_CountingState;
	float m_fTimeLeftInState;
};

#endif
