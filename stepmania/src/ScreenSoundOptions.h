/*
-----------------------------------------------------------------------------
 File: ScreenSoundOptions

 Desc: Adjust sound config

 Copyright (c) 2001-2003 by the person(s) listed below.  All rights reserved.
	Chris Danford
	Kevin Slaughter
-----------------------------------------------------------------------------
*/
#ifndef SCREEN_SOUND_OPTIONS_H
#define SCREEN_SOUND_OPTIONS_H

#include "ScreenOptions.h"

class ScreenSoundOptions : public ScreenOptions
{
public:
	ScreenSoundOptions();

private:
	void ImportOptions();
	void ExportOptions();

	void Input( const DeviceInput& DeviceI, const InputEventType type, const GameInput &GameI, const MenuInput &MenuI, const StyleInput &StyleI );

	void GoToNextState();
	void GoToPrevState();
};

#endif
