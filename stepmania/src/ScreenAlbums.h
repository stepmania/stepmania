/*
-----------------------------------------------------------------------------
 Class: ScreenAlbums

 Desc: Base class for all attraction screens.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "ScreenAttract.h"


class ScreenAlbums : public ScreenAttract
{
public:

private:
	virtual CString	GetMetricName() { return "Albums"; };	// used to loop up theme metrics
	virtual CString	GetElementName() { return "albums"; };	// used to loop up theme elements

};



