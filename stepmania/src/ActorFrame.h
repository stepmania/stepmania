#pragma once
/*
-----------------------------------------------------------------------------
 File: ActorFrame.h

 Desc: Base class for all objects that appear on the screen.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
-----------------------------------------------------------------------------
*/

#include "RageUtil.h"

#include <d3dx8math.h>
#include "Actor.h"



class ActorFrame : public Actor
{
protected:
	CArray<Actor*,Actor*>	m_SubActors;


public:
	void AddActor( Actor* pActor);

	virtual void Update( float fDeltaTime );
	virtual void DrawPrimitives();

	virtual void SetDiffuseColor( D3DXCOLOR c );

};
