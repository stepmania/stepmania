#ifndef SCREENTESTSOUND_H
#define SCREENTESTSOUND_H
/*
-----------------------------------------------------------------------------
 Class: ScreenSandbox

 Desc: Area for testing.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "Screen.h"
#include "BitmapText.h"
#include "RageSound.h"

const int nsounds = 5;

class ScreenTestSound : public Screen
{
public:
	ScreenTestSound();

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
};


#endif
