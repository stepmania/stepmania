#ifndef ScreenTest_H
#define ScreenTest_H
/*
-----------------------------------------------------------------------------
 Class: ScreenTest

 Desc: Area for testing.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
	Glenn Maynard
-----------------------------------------------------------------------------
*/

#include "Screen.h"

class ScreenTest : public Screen
{
public:
	ScreenTest();
	~ScreenTest();

	virtual void Input( const DeviceInput& DeviceI, const InputEventType type, const GameInput &GameI, const MenuInput &MenuI, const StyleInput &StyleI );
	virtual void HandleScreenMessage( const ScreenMessage SM );

	Screen *current;
	int cur_screen;
	void SetScreen(int num);

	void Update(float f);
	void Draw();
};


#endif
