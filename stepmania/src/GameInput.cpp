#include "global.h"
/*
-----------------------------------------------------------------------------
 Class: GameInput

 Desc: See header.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "GameInput.h"
#include "RageLog.h"
#include "RageUtil.h"


CString GameInput::toString() 
{
	return ssprintf("%d-%d", controller, button );
}

bool GameInput::fromString( CString s )
{ 
	CStringArray a;
	split( s, "-", a);

	if( a.size() != 2 ) {
		MakeInvalid();
		return false;
	}

	controller = (GameController)atoi( a[0] );
	button = (GameButton)atoi( a[1] );
	return true;
};


