#ifndef SCREENSANDBOX_H
#define SCREENSANDBOX_H
/*
-----------------------------------------------------------------------------
 Class: ScreenSandbox

 Desc: Area for testing.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "Screen.h"
#include "Sprite.h"
#include "BitmapText.h"
#include "Quad.h"
#include "MenuElements.h"
#include "RageSound.h"
#include "Sample3dObject.h"

const int nsounds = 3;

class ScreenSandbox : public Screen
{
public:
	ScreenSandbox();

	virtual void Input( const DeviceInput& DeviceI, const InputEventType type, const GameInput &GameI, const MenuInput &MenuI, const StyleInput &StyleI );
	virtual void HandleScreenMessage( const ScreenMessage SM );

	void Update(float f);
	void UpdateText(int n);

	struct Sound {
		RageSound s;
		BitmapText txt;
	};
	Sound s[nsounds];
	BitmapText HEEEEEEEEELP;
	
	int selected;

	Quad m_quad;
	Sprite m_sprite;
	Sample3dObject obj;
};


#endif
