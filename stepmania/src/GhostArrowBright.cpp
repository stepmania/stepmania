#include "stdafx.h"
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


#define SHOW_SECONDS		THEME->GetMetricF("GhostArrowBright","ShowSeconds")
#define ZOOM_START			THEME->GetMetricF("GhostArrowBright","ZoomStart")
#define ZOOM_END			THEME->GetMetricF("GhostArrowBright","ZoomEnd")
#define COLOR_PERFECT_START	THEME->GetMetricC("GhostArrowBright","ColorPerfectStart")
#define COLOR_PERFECT_END	THEME->GetMetricC("GhostArrowBright","ColorPerfectEnd")
#define COLOR_GREAT_START	THEME->GetMetricC("GhostArrowBright","ColorGreatStart")
#define COLOR_GREAT_END		THEME->GetMetricC("GhostArrowBright","ColorGreatEnd")
#define COLOR_GOOD_START	THEME->GetMetricC("GhostArrowBright","ColorGoodStart")
#define COLOR_GOOD_END		THEME->GetMetricC("GhostArrowBright","ColorGoodEnd")
#define COLOR_BOO_START		THEME->GetMetricC("GhostArrowBright","ColorBooStart")
#define COLOR_BOO_END		THEME->GetMetricC("GhostArrowBright","ColorBooEnd")

// "2" appended so the names won't conflict with the GhostArrow
float g_fShowSeconds2;
float g_fZoomStart2, g_fZoomEnd2;
D3DXCOLOR 
	g_colorPerfectStart2, g_colorPerfectEnd2, 
	g_colorGreatStart2, g_colorGreatEnd2, 
	g_colorGoodStart2, g_colorGoodEnd2, 
	g_colorBooStart2, g_colorBooEnd2;


GhostArrowBright::GhostArrowBright()
{
	g_fShowSeconds2		= SHOW_SECONDS;
	g_fZoomStart2		= ZOOM_START;
	g_fZoomEnd2			= ZOOM_END;
	g_colorPerfectStart2= COLOR_PERFECT_START;
	g_colorPerfectEnd2	= COLOR_PERFECT_END;
	g_colorGreatStart2	= COLOR_GREAT_START;
	g_colorGreatEnd2	= COLOR_GREAT_END;
	g_colorGoodStart2	= COLOR_GOOD_START;
	g_colorGoodEnd2		= COLOR_GOOD_END;
	g_colorBooStart2	= COLOR_BOO_START;
	g_colorBooEnd2		= COLOR_BOO_END;


	SetDiffuseColor( D3DXCOLOR(1,1,1,0) );
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
		Sprite::m_fDelay[i] = g_fShowSeconds2 / (float)Sprite::GetNumStates();

	D3DXCOLOR colorStart, colorEnd;
	switch( score )
	{
	case TNS_PERFECT:	colorStart = g_colorPerfectStart2;	colorEnd = g_colorPerfectEnd2;	break;
	case TNS_GREAT:		colorStart = g_colorGreatStart2;	colorEnd = g_colorGreatEnd2;	break;
	case TNS_GOOD:		colorStart = g_colorGoodStart2;		colorEnd = g_colorGoodEnd2;		break;
	case TNS_BOO:		colorStart = g_colorBooStart2;		colorEnd = g_colorBooEnd2;		break;
	case TNS_MISS:		// miss should never be passed in here
	default:
		ASSERT(0);
	}

	StopTweening();
	SetDiffuseColor( colorStart );
	SetState( 0 );
	SetZoom( g_fZoomStart2 );
	
	BeginTweeningQueued( g_fShowSeconds2 );
	SetTweenZoom( g_fZoomEnd2 );
	SetTweenDiffuseColor( colorEnd );

	BeginTweeningQueued( 0.0001f );		// snap to invisible
	SetTweenDiffuseColor( D3DXCOLOR(1,1,1,0) );
}
