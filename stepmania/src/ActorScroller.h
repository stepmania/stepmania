#ifndef ActorScroller_H
#define ActorScroller_H
/*
-----------------------------------------------------------------------------
 Class: ActorScroller

 Desc: A container for other actors.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "Actor.h"


class ActorScroller : public Actor
{
public:
	ActorScroller();

	// owner's responsibility to delete vActors!
	void Load( vector<Actor*> vActors, float fBaseX, float fBaseY, float fVelX, float fVelY, float fSpacingX, float fSpacingY );

	void Update( float fDelta );
	void DrawPrimitives();

	float GetTotalSecsToScroll();

protected:
	float		m_fSecsIntoScroll;
	RageVector2	m_vBase;
	RageVector2	m_vVelocity;
	RageVector2	m_vSpacing;
	vector<Actor*> m_vActors;
};


#endif
