#ifndef DifficultyMeter_H
#define DifficultyMeter_H
/*
-----------------------------------------------------------------------------
 Class: DifficultyMeter

 Desc: A meter represention of how hard Notes is.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "BitmapText.h"
class Notes;


class DifficultyMeter : public BitmapText
{
public:
	DifficultyMeter();

	void SetFromNotes( Notes* pNotes );
	void SetMeter( int iMeter );

private:
};

#endif
