/*
-----------------------------------------------------------------------------
 Class: ScreenMemoryCard

 Desc: .

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "ScreenAttract.h"


class ScreenMemoryCard : public ScreenAttract
{
public:

private:
	virtual CString	GetMetricName() { return "MemoryCard"; };	// used to look up theme metrics
	virtual CString	GetElementName() { return "memory card"; };	// used to look up theme elements

};



