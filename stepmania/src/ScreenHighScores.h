/*
-----------------------------------------------------------------------------
 Class: ScreenHighScores

 Desc: Base class for all attraction screens.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "ScreenAttract.h"


class ScreenHighScores : public ScreenAttract
{
public:

private:
	virtual CString	GetMetricName() { return "HighScores"; };	// used to loop up theme metrics
	virtual CString	GetElementName() { return "high scores"; };	// used to loop up theme elements

};



