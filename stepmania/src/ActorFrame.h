#ifndef ACTORFRAME_H
#define ACTORFRAME_H
/*
-----------------------------------------------------------------------------
 Class: ActorFrame

 Desc: A container for other actors.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "Actor.h"

class ActorFrame : public Actor
{
public:
	virtual void AddChild( Actor* pActor);
	virtual ~ActorFrame() { }

	virtual void Update( float fDeltaTime );
	virtual void DrawPrimitives();

	virtual void SetDiffuse( RageColor c );

protected:
	CArray<Actor*,Actor*>	m_SubActors;
};

#endif
