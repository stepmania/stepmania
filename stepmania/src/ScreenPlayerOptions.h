/*
-----------------------------------------------------------------------------
 File: ScreenPlayerOptions.h

 Desc: Select a song.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
-----------------------------------------------------------------------------
*/
#ifndef _WINDOWPLAYEROPTIONS_H_
#define _WINDOWPLAYEROPTIONS_H_


#include "Screen.h"
#include "ScreenOptions.h"
#include "Sprite.h"
#include "BitmapText.h"
#include "RandomSample.h"
#include "TransitionFade.h"
#include "Quad.h"



class ScreenPlayerOptions : public ScreenOptions
{
public:
	ScreenPlayerOptions();

private:
	void ImportOptions();
	void ExportOptions();

	void GoToNextState();
	void GoToPrevState();
};



#endif 