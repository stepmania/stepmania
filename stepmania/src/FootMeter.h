#ifndef FOOTMETER_H
#define FOOTMETER_H
/*
-----------------------------------------------------------------------------
 Class: FootMeter

 Desc: A meter represention of how hard Notes is.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "Sprite.h"
#include "song.h"
#include "BitmapText.h"
#include "PrefsManager.h"


class FootMeter : public BitmapText
{
public:
	FootMeter();

	void SetFromNotes( Notes* pNotes );

private:
	void SetNumFeet( int iNumFeet );
};

#endif
