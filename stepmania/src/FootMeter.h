#pragma once
/*
-----------------------------------------------------------------------------
 Class: FootMeter

 Desc: A graphic displayed in the FootMeter during Dancing.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "Sprite.h"
#include "Song.h"
#include "BitmapText.h"
#include "ThemeManager.h"


class FootMeter : public BitmapText
{
public:
	FootMeter();

	void SetFromNotes( Notes* pNotes );

private:
	void SetNumFeet( int iNumFeet, const CString &sDescription );
};
