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
	virtual void AddChild( Actor* pActor );
	virtual void MoveToTail( Actor* pActor );
	virtual void MoveToHead( Actor* pActor );
	virtual void MoveToFront( Actor* pActor ) { MoveToHead( pActor ); } // hack! so many things call this... when they really mean move to head.
	virtual ~ActorFrame() { }

	virtual void Update( float fDeltaTime );
	virtual void DrawPrimitives();

	virtual void SetDiffuse( RageColor c );

	/* Amount of time until all tweens (and all children's tweens) have stopped: */
	virtual float TweenTime() const;

protected:
	vector<Actor*>	m_SubActors;
};

#endif
