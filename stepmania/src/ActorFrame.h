/*
-----------------------------------------------------------------------------
 File: ActorFrame.h

 Desc: Base class for all objects that appear on the screen.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
-----------------------------------------------------------------------------
*/


#ifndef _ActorFrame_H_
#define _ActorFrame_H_

#include "RageUtil.h"

#include <d3dx8math.h>
#include "Actor.h"



class ActorFrame : public Actor
{
protected:
	CArray<Actor*,Actor*>	m_SubActors;


public:
	void AddActor( Actor* pActor) { m_SubActors.Add(pActor); };

	virtual void Update( float fDeltaTime );
	virtual void DrawPrimitives();

	virtual void SetDiffuseColor( D3DXCOLOR c );

};



#endif