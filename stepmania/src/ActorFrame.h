#pragma once
/*
-----------------------------------------------------------------------------
 Class: ActorFrame

 Desc: A container for other actors.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "RageUtil.h"

#include <d3dx8math.h>
#include "Actor.h"



class ActorFrame : public Actor
{
public:
	virtual void AddChild( Actor* pActor);
	virtual ~ActorFrame() { }

	virtual void Update( float fDeltaTime );
	virtual void DrawPrimitives();

	virtual void SetDiffuse( D3DXCOLOR c );

protected:
	CArray<Actor*,Actor*>	m_SubActors;
};
