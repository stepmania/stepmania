/*
-----------------------------------------------------------------------------
 Class: ScreenCompany

 Desc: Base class for all attraction screens.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "ScreenAttract.h"


class ScreenCompany : public ScreenAttract
{
public:

private:
	virtual CString	GetMetricName() { return "Company"; };	// used to look up theme metrics
	virtual CString	GetElementName() { return "company"; };	// used to look up theme elements

};



