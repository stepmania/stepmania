#include "stdafx.h"
/*
-----------------------------------------------------------------------------
 Class: GrayArrow

 Desc: A gray arrow that "receives" ColorArrows.

 Copyright (c) 2001-2002 by the names listed below.  All rights reserved.
	Ben Nordstrom
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "GrayArrow.h"
#include "ThemeManager.h"

const float GRAY_ARROW_POP_UP_TIME			= 0.3f;


GrayArrow::GrayArrow()
{
	Load( THEME->GetPathTo(GRAPHIC_GRAY_ARROW) );
	StopAnimating();
}

void GrayArrow::SetBeat( const float fSongBeat )
{
	SetState( fmod(fSongBeat,1)<0.25 ? 1 : 0 );
}

void GrayArrow::Step()
{
	SetZoom( 0.50 );
	BeginTweening( GRAY_ARROW_POP_UP_TIME );
	SetTweenZoom( 1.0f );
}
