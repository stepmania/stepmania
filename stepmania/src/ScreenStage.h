/*
-----------------------------------------------------------------------------
 Class: ScreenStage

 Desc: Shows the stage number.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "Screen.h"
#include "ActorFrame.h"
#include "Actor.h"
#include "TransitionFade.h"


class ScreenStage : public Screen
{
public:
	ScreenStage();

	virtual void Update( float fDeltaTime );
	virtual void DrawPrimitives();
	virtual void HandleScreenMessage( const ScreenMessage SM );

private:

	ActorFrame		m_frameStage;
	Sprite			m_sprNumbers[4];	// up to 3 numbers and suffix
	Sprite			m_sprStage;			// "Stage"

	Quad			m_quadMask;		// write this into ZBuffer as a mask

	TransitionFade	m_Fade;

	Screen*			m_pNextScreen;
};


