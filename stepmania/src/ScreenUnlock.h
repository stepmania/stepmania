/*
-----------------------------------------------------------------------------
 Class: ScreenUnlock

 Desc: Base class for all attraction screens.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "ScreenAttract.h"


class ScreenUnlock : public ScreenAttract
{
public:

private:
	virtual CString	GetMetricName() { return "Unlock"; };	// used to look up theme metrics
	virtual CString	GetElementName() { return "unlock"; };	// used to look up theme elements

};



