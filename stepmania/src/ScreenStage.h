/*
-----------------------------------------------------------------------------
 Class: ScreenStage

 Desc: Shows the stage number.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "Screen.h"
#include "Transition.h"

class ScreenStage : public Screen
{
public:
	ScreenStage();

	virtual void Update( float fDeltaTime );
	virtual void DrawPrimitives();
	virtual void HandleScreenMessage( const ScreenMessage SM );
	
	virtual void MenuBack( PlayerNumber pn );

private:


	// Ez2 and Pump Stage screens should be re-implemented as BGAnimations

/*
	// Common Elements
	Sprite			m_sprNumbers[4];	// up to 3 numbers and suffix
	Sprite			m_sprStage;			// "Stage", "Final Stage", etc.
	// A frame for m_sprNumbers and m_sprStage, so they can be
	// manipulated as a unit.
	ActorFrame		m_frameStage;

	// Ez2 Elements

	Sprite			m_sprbg[3];
	Sprite			m_sprbgxtra;
	Sprite			m_sprScrollingBlobs[2][20];
	BitmapText		m_ez2ukm[2];
	BitmapText		m_stagename;
	BitmapText		m_stagedesc[2];

	// Pump Elements

	Sprite m_sprSongBackground;

	Quad			m_quadMask;		// write this into ZBuffer as a mask
*/
	Transition	m_In, m_Out, m_Back;
	BGAnimation				m_Background;

protected:

};


