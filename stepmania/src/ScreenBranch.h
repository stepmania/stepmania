/*
-----------------------------------------------------------------------------
 Class: ScreenBranch

 Desc: Load one of several screens.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "Screen.h"
#include "MenuElements.h"


class ScreenBranch : public Screen
{
public:
	ScreenBranch( CString sName );

	virtual void HandleScreenMessage( const ScreenMessage SM );

private:
	CString m_sChoice;
};

