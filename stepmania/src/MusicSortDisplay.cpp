#include "global.h"
/*
-----------------------------------------------------------------------------
 Class: MusicSortDisplay

 Desc: A graphic displayed in the MusicSortDisplay during Dancing.

 Copyright (c) 2001-2003 by the person(s) listed below.  All rights reserved.
	Chris Danford
	Chris Gomez
-----------------------------------------------------------------------------
*/

#include "MusicSortDisplay.h"
#include "RageUtil.h"
#include "GameConstantsAndTypes.h"
#include "MusicWheel.h"
#include "MusicSortDisplay.h"
#include "ThemeManager.h"


MusicSortDisplay::MusicSortDisplay()
{
}

void MusicSortDisplay::Set( SortOrder so ) 
{ 
	Load( THEME->GetPathToG(ssprintf("MusicSortDisplay %s",SortOrderToString(so).c_str())) );
}
