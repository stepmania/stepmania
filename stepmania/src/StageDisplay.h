#ifndef StageDisplay_H
#define StageDisplay_H
/*
-----------------------------------------------------------------------------
 Class: StageDisplay

 Desc: Shows stage number

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "Sprite.h"
#include "Song.h"
#include "BitmapText.h"
#include "PrefsManager.h"


class StageDisplay : public BitmapText
{
public:
	StageDisplay();

	void Refresh();

private:
};

#endif
