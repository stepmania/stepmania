/*
-----------------------------------------------------------------------------
 File: ScreenSongOptions.h

 Desc: Select a song.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
-----------------------------------------------------------------------------
*/
#ifndef _ScreenSongOptions_H_
#define _ScreenSongOptions_H_



#include "Screen.h"
#include "ScreenOptions.h"
#include "Sprite.h"
#include "BitmapText.h"
#include "RandomSample.h"
#include "TransitionFade.h"
#include "Quad.h"



class ScreenSongOptions : public ScreenOptions
{
public:
	ScreenSongOptions();

private:
	void ImportOptions();
	void ExportOptions();

	void GoToNextState();
	void GoToPrevState();
};





#endif