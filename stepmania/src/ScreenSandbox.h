/*
-----------------------------------------------------------------------------
 File: ScreenSandbox.h

 Desc: Area for testing.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
-----------------------------------------------------------------------------
*/

#ifndef _ScreenSandbox_H_
#define _ScreenSandbox_H_


#include "Screen.h"
#include "Sprite.h"
#include "BitmapText.h"
#include "Quad.h"
#include "TransitionStarWipe.h"
#include "MenuElements.h"


class ScreenSandbox : public Screen
{
public:
	ScreenSandbox();

	virtual void Update( float fDeltaTime );
	virtual void DrawPrimitives();
	virtual void Input( const DeviceInput& DeviceI, const InputEventType type, const GameInput &GameI, const MenuInput &MenuI, const StyleInput &StyleI );
	virtual void HandleScreenMessage( const ScreenMessage SM );

	Sprite m_spr;

	RageSoundSample	m_sound;
};


#endif