/*
-----------------------------------------------------------------------------
 Class: ScreenDemonstration

 Desc: Base class for all attraction screens.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "ScreenJukebox.h"


class ScreenDemonstration : public ScreenJukebox
{
public:
	ScreenDemonstration( CString sName );

	virtual void HandleScreenMessage( const ScreenMessage SM );

protected:
	BGAnimation	m_Overlay;
};



