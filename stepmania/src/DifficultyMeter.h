#ifndef DifficultyMeter_H
#define DifficultyMeter_H
/*
-----------------------------------------------------------------------------
 Class: DifficultyMeter

 Desc: A meter represention of how hard Steps is.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "BitmapText.h"
#include "PlayerNumber.h"
class Steps;
class Course;


class DifficultyMeter : public BitmapText
{
public:
	DifficultyMeter();

	void SetFromGameState( PlayerNumber pn );
	void SetFromNotes( Steps* pNotes );
	void SetFromCourse( Course* pCourse );
	void Unset();

private:
	void SetMeter( int iMeter );
};

#endif
