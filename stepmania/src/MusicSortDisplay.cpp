#include "stdafx.h"
/*
-----------------------------------------------------------------------------
 Class: MusicSortDisplay

 Desc: A graphic displayed in the MusicSortDisplay during Dancing.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
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
	Load( THEME->GetPathTo("Graphics","music sort icons 1x4") );
	StopAnimating();
}

void MusicSortDisplay::Set( SongSortOrder so ) 
{ 
	switch( so )
	{
	case SORT_GROUP:
	case SORT_TITLE:
	case SORT_BPM:
	case SORT_MOST_PLAYED:
		SetState( so );
		break;
	default:
		ASSERT(0);		// unimplemented MusicSortOrder
	}
}
