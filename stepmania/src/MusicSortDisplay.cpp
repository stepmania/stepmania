#include "stdafx.h"
/*
-----------------------------------------------------------------------------
 File: MusicSortDisplay.h

 Desc: A graphic displayed in the MusicSortDisplay during Dancing.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
-----------------------------------------------------------------------------
*/

#include "MusicSortDisplay.h"
#include "RageUtil.h"
#include "GameConstantsAndTypes.h"
#include "MusicWheel.h"
#include "MusicSortDisplay.h"





MusicSortDisplay::MusicSortDisplay()
{
	Load( THEME->GetPathTo(GRAPHIC_MUSIC_SORT_ICONS) );
	StopAnimating();
	TurnShadowOff();
	//SetXY( ON_SCREEN_X, ON_SCREEN_Y );
}


void MusicSortDisplay::Set( SongSortOrder so ) 
{ 

	switch( so )
	{
	case SORT_GROUP:
		SetState( 0 );
		break;
	case SORT_TITLE:
		SetState( 1 );
		break;
	case SORT_BPM:
		SetState( 2 );
		break;
	case SORT_MOST_PLAYED:
		SetState( 3 );
		break;
	default:
		ASSERT( false );		// unimplemented MusicSortOrder
	}
}

