/*
-----------------------------------------------------------------------------
 Class: ScreenGraphicOptions

 Desc: Change video-related options.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "Screen.h"
#include "ScreenOptions.h"
#include "Sprite.h"
#include "BitmapText.h"
#include "RandomSample.h"
#include "TransitionFade.h"
#include "Quad.h"


class ScreenGraphicOptions : public ScreenOptions
{
public:
	ScreenGraphicOptions();

private:
	void ImportOptions();
	void ExportOptions();

	void GoToNextState();
	void GoToPrevState();
};
