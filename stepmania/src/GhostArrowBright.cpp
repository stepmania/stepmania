#include "global.h"
/*
-----------------------------------------------------------------------------
 Class: GhostArrowBright

 Desc: See header.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Ben Nordstrom
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "GhostArrowBright.h"
#include "PrefsManager.h"
#include "ThemeManager.h"


CachedThemeMetric	GAB_SHOW_SECONDS		("GhostArrowBright","ShowSeconds");
CachedThemeMetric	GAB_ZOOM_START			("GhostArrowBright","ZoomStart");
CachedThemeMetric	GAB_ZOOM_END			("GhostArrowBright","ZoomEnd");
CachedThemeMetric	GAB_COLOR_MARVELOUS		("GhostArrowBright","ColorMarvelous");
CachedThemeMetric	GAB_COLOR_PERFECT		("GhostArrowBright","ColorPerfect");
CachedThemeMetric	GAB_COLOR_GREAT			("GhostArrowBright","ColorGreat");
CachedThemeMetric	GAB_COLOR_GOOD			("GhostArrowBright","ColorGood");
CachedThemeMetric	GAB_COLOR_BOO			("GhostArrowBright","ColorBoo");


GhostArrowBright::GhostArrowBright()
{
	GAB_SHOW_SECONDS.Refresh();
	GAB_ZOOM_START.Refresh();
	GAB_ZOOM_END.Refresh();
	GAB_COLOR_MARVELOUS.Refresh();
	GAB_COLOR_PERFECT.Refresh();
	GAB_COLOR_GREAT.Refresh();
	GAB_COLOR_GOOD.Refresh();
	GAB_COLOR_BOO.Refresh();

	SetDiffuse( RageColor(1,1,1,0) );
}

void GhostArrowBright::Update( float fDeltaTime )
{
	Sprite::Update( fDeltaTime );
}

void GhostArrowBright::Step( TapNoteScore score )
{
	// HACK: set the length of each frame so the animation plays in exactly 1 pop up time.
	//    We can't do this in the constructor because the image hasn't been loaded yet
	for( int i=0; i<Sprite::GetNumStates(); i++ )
		Sprite::m_fDelay[i] = (float)GAB_SHOW_SECONDS / (float)Sprite::GetNumStates();

	RageColor colorStart;
	switch( score )
	{
	case TNS_MARVELOUS:	colorStart = GAB_COLOR_MARVELOUS;	break;
	case TNS_PERFECT:	colorStart = GAB_COLOR_PERFECT;		break;
	case TNS_GREAT:		colorStart = GAB_COLOR_GREAT;		break;
	case TNS_GOOD:		colorStart = GAB_COLOR_GOOD;			break;
	case TNS_BOO:		colorStart = GAB_COLOR_BOO;			break;
	case TNS_MISS:		// miss should never be passed in here
	default:
		ASSERT(0);
	}
	RageColor colorEnd = colorStart;
	colorEnd.a = 0;

	StopTweening();
	SetDiffuse( colorStart );
	SetState( 0 );
	SetZoom( GAB_ZOOM_START );
	
	BeginTweening( (float)GAB_SHOW_SECONDS/2 );
	SetTweenZoom( ((float)GAB_ZOOM_START+(float)GAB_ZOOM_END)/2 );
	BeginTweening( (float)GAB_SHOW_SECONDS/2 );
	SetTweenZoom( GAB_ZOOM_END );
	SetTweenDiffuse( colorEnd );
}
