#include "global.h"
/*
-----------------------------------------------------------------------------
 Class: GhostArrow

 Desc: See header.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Ben Nordstrom
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "GhostArrow.h"
#include "PrefsManager.h"
#include "ThemeManager.h"


CachedThemeMetricF	GA_SHOW_SECONDS			("GhostArrow","ShowSeconds");
CachedThemeMetricF	GA_ZOOM_START			("GhostArrow","ZoomStart");
CachedThemeMetricF	GA_ZOOM_END				("GhostArrow","ZoomEnd");
CachedThemeMetricC	GA_COLOR_MARVELOUS		("GhostArrow","ColorMarvelous");
CachedThemeMetricC	GA_COLOR_PERFECT		("GhostArrow","ColorPerfect");
CachedThemeMetricC	GA_COLOR_GREAT			("GhostArrow","ColorGreat");
CachedThemeMetricC	GA_COLOR_GOOD			("GhostArrow","ColorGood");
CachedThemeMetricC	GA_COLOR_BOO			("GhostArrow","ColorBoo");


GhostArrow::GhostArrow()
{
	GA_SHOW_SECONDS.Refresh();
	GA_ZOOM_START.Refresh();
	GA_ZOOM_END.Refresh();
	GA_COLOR_MARVELOUS.Refresh();
	GA_COLOR_PERFECT.Refresh();
	GA_COLOR_GREAT.Refresh();
	GA_COLOR_GOOD.Refresh();
	GA_COLOR_BOO.Refresh();

	SetDiffuse( RageColor(1,1,1,0) );
}

void GhostArrow::Update( float fDeltaTime )
{
	Sprite::Update( fDeltaTime );
}

void GhostArrow::Step( TapNoteScore score )
{
	// HACK: set the length of each frame so the animation plays in exactly 1 pop up time.
	//    We can't do this in the constructor because the image hasn't been loaded yet
	for( int i=0; i<Sprite::GetNumStates(); i++ )
		Sprite::m_States[i].fDelay = GA_SHOW_SECONDS / Sprite::GetNumStates();

	RageColor colorStart;
	switch( score )
	{
	case TNS_MARVELOUS:	colorStart = GA_COLOR_MARVELOUS;	break;
	case TNS_PERFECT:	colorStart = GA_COLOR_PERFECT;		break;
	case TNS_GREAT:		colorStart = GA_COLOR_GREAT;		break;
	case TNS_GOOD:		colorStart = GA_COLOR_GOOD;			break;
	case TNS_BOO:		colorStart = GA_COLOR_BOO;			break;
	case TNS_MISS:		// miss should never be passed in here
	default:
		ASSERT(0);
	}
	RageColor colorEnd = colorStart;
	colorEnd.a = 0;

	StopTweening();
	SetDiffuse( colorStart );
	SetState( 0 );
	SetZoom( GA_ZOOM_START );
	
	BeginTweening( (float)GA_SHOW_SECONDS/2.0f );
	SetZoom( ((float)GA_ZOOM_START+(float)GA_ZOOM_END)/2.0f );
	BeginTweening( (float)GA_SHOW_SECONDS/2.0f );
	SetZoom( GA_ZOOM_END );
	SetDiffuse( colorEnd );
}
